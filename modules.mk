mod_languagesubdomain.la: mod_languagesubdomain.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_languagesubdomain.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_languagesubdomain.la
