
APXS=/usr/bin/apxs2

all: mod_languagesubdomain.so

%.so:%.c
	$(APXS) -c $^


clean:
