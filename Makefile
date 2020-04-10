# APXS=/usr/bin/apxs2

builddir=.
top_srcdir=/usr/share/apache2
top_builddir=/usr/share/apache2
include /usr/share/apache2/build/special.mk

all: local-shared-build

# install: install-modules-yes


install:
	install --directory $(DESTDIR)/usr/lib/apache2/modules/
	/usr/share/apr-1.0/build/libtool --no-silent --mode=install install mod_languagesubdomain.la $(DESTDIR)/usr/lib/apache2/modules/

	install --directory $(DESTDIR)/etc/apache2/mods-available/
	install languagesubdomain.load $(DESTDIR)/etc/apache2/mods-available/

# sudo apxs2 -n languagesubdomain -i mod_languagesubdomain.la
clean:
