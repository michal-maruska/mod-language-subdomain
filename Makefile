

APXS=/usr/bin/apxs2

NAME := mod_languagesubdomain
MODULE := $(NAME).so
all: $(MODULE)

# 
$(MODULE): %.so : %.c
	$(APXS) -c $^


# does not support DESTDIR
# https://bz.apache.org/bugzilla/show_bug.cgi?id=32930
# $(APXS) -i $^


install: $(MODULE)
	# don't fail on permission denied :(
	- apxs -i $(NAME).c
	# manually install into destdir:
	install --directory $(DESTDIR)/usr/lib/apache2/modules/
	/usr/share/apr-1.0/build/libtool --mode=install install mod_languagesubdomain.c $(DESTDIR)/usr/lib/apache2/modules/
	# install $(MODULE) $(DESTDIR)/usr/lib/apache2/modules/

# sudo apxs2 -n languagesubdomain -i mod_languagesubdomain.la

clean:
