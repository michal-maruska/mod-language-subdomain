/*
  install:   apxs2   -c -i  mod_languagesubdomain.c

        Copyright 2002 Kevin O'Donnell
        Copyright 2007 Michal Maruska


        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 * Include the core server components.
 */
#include "httpd.h"  // /usr/include/apache2/httpd.h
#include "http_config.h"

/* mmc: /usr/include/apr-1.0 */
#include "apr-1.0/apr_tables.h"
#include "apr-1.0/apr_lib.h"
#include "apr-1.0/apr_strings.h"

/*
 * The default value for the error string.
 */
#ifndef DEFAULT_MODTUT2_STRING
#define DEFAULT_MODTUT2_STRING "apache2_mod_tut2: A request was made."
#endif

#define DEBUG 0


/*
 * This module
 */
module AP_MODULE_DECLARE_DATA languagesubdomain_module;

/*
 * This modules per-server configuration structure.
 */
typedef struct {
        char *string;
} modtut2_config;


/*
 * Record of available info on a media type specified by the client
 * (we also use 'em for encodings and languages)
 */

typedef struct accept_rec {
    char *name;                 /* MUST be lowercase */
    float quality;
    float level;
    char *charset;              /* for content-type only */
} accept_rec;




/*****************************************************************
 *
 * Parsing (lists of) media types and their parameters, as seen in
 * HTTPD header lines and elsewhere.
 */

/*
 * Get a single mime type entry --- one media type and parameters;
 * enter the values we recognize into the argument accept_rec
 */

static
const char *
get_entry(apr_pool_t *p, accept_rec *result,
                             const char *accept_line)
{
    result->quality = 1.0f;
    result->level = 0.0f;
    result->charset = "";

    /*
     * Note that this handles what I gather is the "old format",
     *
     *    Accept: text/html text/plain moo/zot
     *
     * without any compatibility kludges --- if the token after the
     * MIME type begins with a semicolon, we know we're looking at parms,
     * otherwise, we know we aren't.  (So why all the pissing and moaning
     * in the CERN server code?  I must be missing something).
     */

    result->name = ap_get_token(p, &accept_line, 0);
    apr_tolower(result->name);     /* You want case insensitive,
                                       * you'll *get* case insensitive.
                                       */

    /* KLUDGE!!! Default HTML to level 2.0 unless the browser
     * *explicitly* says something else.
     */

    if (!strcmp(result->name, "text/html") && (result->level == 0.0)) {
        result->level = 2.0f;
    }
    else if (!strcmp(result->name, INCLUDES_MAGIC_TYPE)) {
        result->level = 2.0f;
    }
    else if (!strcmp(result->name, INCLUDES_MAGIC_TYPE3)) {
        result->level = 3.0f;
    }

    while (*accept_line == ';') {
        /* Parameters ... */

        char *parm;
        char *cp;
        char *end;

        ++accept_line;
        parm = ap_get_token(p, &accept_line, 1);

        /* Look for 'var = value' --- and make sure the var is in lcase. */

        for (cp = parm; (*cp && !apr_isspace(*cp) && *cp != '='); ++cp) {
            *cp = apr_tolower(*cp);
        }

        if (!*cp) {
            continue;           /* No '='; just ignore it. */
        }

        *cp++ = '\0';           /* Delimit var */
        while (*cp && (apr_isspace(*cp) || *cp == '=')) {
            ++cp;
        }

        if (*cp == '"') {
            ++cp;
            for (end = cp;
                 (*end && *end != '\n' && *end != '\r' && *end != '\"');
                 end++);
        }
        else {
            for (end = cp; (*end && !apr_isspace(*end)); end++);
        }
        if (*end) {
            *end = '\0';        /* strip ending quote or return */
        }
        apr_tolower(cp);

        if (parm[0] == 'q'
            && (parm[1] == '\0' || (parm[1] == 's' && parm[2] == '\0'))) {
            result->quality = (float)atof(cp);
        }
        else if (parm[0] == 'l' && !strcmp(&parm[1], "evel")) {
            result->level = (float)atof(cp);
        }
        else if (!strcmp(parm, "charset")) {
            result->charset = cp;
        }
    }

    if (*accept_line == ',') {
        ++accept_line;
    }

    return accept_line;
}


/*****************************************************************
 *
 * Dealing with header lines ...
 *
 * Accept, Accept-Charset, Accept-Language and Accept-Encoding
 * are handled by do_header_line() - they all have the same
 * basic structure of a list of items of the format
 *    name; q=N; charset=TEXT
 *
 * where charset is only valid in Accept.
 */

static apr_array_header_t *do_header_line(apr_pool_t *p,
                                          const char *accept_line)
{
    apr_array_header_t *accept_recs;

    if (!accept_line) {
        return NULL;
    }

    accept_recs = apr_array_make(p, 40, sizeof(accept_rec));

    while (*accept_line) {
        accept_rec *new = (accept_rec *) apr_array_push(accept_recs);
        accept_line = get_entry(p, new, accept_line);
    }

    return accept_recs;
}



/*
 * This function is registered as a handler for HTTP methods and will
 * therefore be invoked for all GET requests (and others).  Regardless
 * of the request type, this function simply sends a message to
 * STDERR (which httpd redirects to logs/error_log).  A real module
 * would do *alot* more at this point.
 */
static int mod_tut2_method_handler (request_rec *r)
{
  // Get the module configuration
  modtut2_config *s_cfg = ap_get_module_config(r->server->module_config, &languagesubdomain_module);


  if (s_cfg->string)
    {
      apr_table_t *hdrs = r->headers_in;

      const char* line = apr_table_get(hdrs, "Accept-Language");

      if (!line)
        apr_table_set(hdrs, "Accept-Language",
                      /* fixme: should add comma , ? */
                      /* fixme: I should copy it! */
                      s_cfg->string);
      else
        {
#if DEBUG
          fprintf(stderr, "Accept-Language: %s\n", line);
#endif
          /* is there ap_get_module_config  for vhost? */

          apr_array_header_t *langs  = do_header_line(r->pool, line);

#if DEBUG
          fprintf(stderr,"%d languages accepted (%d)! %s\n", langs->nelts, langs->elt_size, s_cfg->string);
#endif

          char new_string[400]; /* X*Y should be enough!   or alloca(langs->nelts * langs->elt_size) */
          char* ns = new_string; /* cursor */
          int i;
          /* int first = 1; */
          for(i= 0; i< langs->nelts;i++)
            {
              /* elt_size */
              accept_rec *lang = (accept_rec*)
                (langs->elts + (i*langs->elt_size));
#if DEBUG
              fprintf(stderr,"%d: %s: %f!\n", i, lang->name, lang->quality);
#endif

              if (strcmp(lang->name, s_cfg->string) == 0)
                {
                  /* skip */
                }
              else
                {

#if 1
                  if (lang->quality < 0.9) {
                    ns+= sprintf(ns, /*400 - (ns - new_string), */
                                 "%s;q=%0.2f", lang->name, lang->quality);
                  } else {
                    ns+= sprintf(ns, /* 400 - (ns - new_string),*/
                                 "%s;q=0.9", lang->name);
                  }

                  (*(ns++)) = ',';
#endif
                }

            }
#if 1
          /* overwrite the last comma! */
          if (ns != new_string)
            (*(ns -1)) ='\0';
          else
            *ns = '\0';


          /* fprintf(stderr, "new string: %s\n", new_string); */
#endif

#if 1
          char* new_langs = apr_pstrcat(r->pool, s_cfg->string, ",", new_string, NULL);
#if DEBUG
          fprintf(stderr,"setting: %s\n", new_langs);
#endif
          apr_table_setn(hdrs, "Accept-Language",
                         /* fixme: should add comma , ? */
                         new_langs);
#endif
        }
    }



  // Send a message to the log file.


  // We need to flush the stream so that the message appears right away.
  // Performing an fflush() in a production system is not good for
  // performance - don't do this for real.
  fflush(stderr);

  // Return DECLINED so that the Apache core will keep looking for
  // other modules to handle this request.  This effectively makes
  // this module completely transparent.
  return DECLINED;
}


/* if I want to stay after a known module! */
static const char* const post_modules[] = { "mod_setenvif.c", NULL };
/* ... and I indeed want to be before mod_negotiation.c  */


/*
 * This function is a callback and it declares what other functions
 * should be called for request processing and configuration requests.
 * This callback function declares the Handlers for other events.
 */
static void register_hooks (apr_pool_t *p)
{
        // I think this is the call to make to register a handler for method calls (GET PUT et. al.).
        // We will ask to be last so that the comment has a higher tendency to
        // go at the end.
#if DEBUG
        fprintf(stderr,"%s:\n", __FUNCTION__);
#endif

#if 0
        ap_hook_handler(mod_tut2_method_handler, NULL, NULL,
                        APR_HOOK_FIRST
                        /*APR_HOOK_LAST */
                        );
#endif
        ap_hook_header_parser(mod_tut2_method_handler, NULL, NULL, APR_HOOK_MIDDLE);
        /* mod_ipenv:
           ap_hook_header_parser */
}

/**
 * This function is called when the "ModTut2String" configuration directive is parsed.
 */
static const char *set_modtut2_string(cmd_parms *parms, void *mconfig, const char *arg)
{
        // get the module configuration (this is the structure created by create_lang_config())
        modtut2_config *s_cfg = ap_get_module_config(parms->server->module_config, &languagesubdomain_module);

        // make a duplicate of the argument's value using the command parameters pool.
        s_cfg->string = (char *) arg;

        // success
        return NULL;
}

/**
 * A declaration of the configuration directives that are supported by this module.
 */
static const command_rec mod_lang_cmds[] =
{
#if 0
        /** method of declaring a directive which takes 1 argument */
# define AP_INIT_TAKE1(directive, func, mconfig, where, help) \
    { directive, { .take1=func }, mconfig, where, TAKE1, help }

#define RSRC_CONF 128	     /**< *.conf outside <Directory> or <Location> */
#endif
        AP_INIT_TAKE1(
                "ModuleTutorialString",
                set_modtut2_string,
                NULL,
                RSRC_CONF,      /* mmc: ?? */
                "ModuleTutorialString <string> -- the string to prepend to Accept-Language header line (for each HTTP request)."
        ),
        {NULL}
};

/**
 * Creates the per-server configuration records.
 */
static void *create_lang_config(apr_pool_t *p, server_rec *s)
{
        modtut2_config *newcfg;

        // allocate space for the configuration structure from the provided pool p.
        newcfg = (modtut2_config *) apr_pcalloc(p, sizeof(modtut2_config));

        // set the default value for the error string.
        newcfg->string = NULL;  /*  DEFAULT_MODTUT2_STRING */

        // return the new server configuration structure.
        return (void *) newcfg;
}

/*
 * Declare and populate the module's data structure.  The
 * name of this structure ('languagesubdomain_module') is important - it
 * must match the name of the module.  This structure is the
 * only "glue" between the httpd core and the module.
 */
module AP_MODULE_DECLARE_DATA languagesubdomain_module =
{
        STANDARD20_MODULE_STUFF, // standard stuff; no need to mess with this.
        NULL, // create per-directory configuration structures - we do not.
        NULL, // merge per-directory - no need to merge if we are not creating anything.
        create_lang_config, // create per-server configuration structures.
        NULL, // merge per-server - hrm - examples I have been reading don't bother with this for trivial cases.
        mod_lang_cmds, // configuration directive handlers
        register_hooks, // request handlers
};
