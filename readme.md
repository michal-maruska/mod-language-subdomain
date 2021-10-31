# module to simulate user's preference for language (by VHOST)

Image you have have  it.host.com and ru.host.com and want to serve
content in italian on the former, and russion on the latter.


## Activation:
```
sudo ln -s ../mods-available/languagesubdomain.load /etc/apache2/mods-enabled/
```

## Configuration
in the VHOST add this

```
PrioritizedLanguage "it"
```

Don't forget the activate the .... Negotiation.

* create  *.{lang}.html files
* remove  *.html files
