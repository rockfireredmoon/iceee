ACLOCAL_AMFLAGS = -I m4
SUBDIRS = asset SOURCE Etc
EXTRA_DIST = autogen.sh

if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool
	
install-exec-hook:
	mkdir -p $(DESTDIR)$(sysconfdir)/tawd ; \
	cp -n ServerConfig.txt.sample $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt ; \
	cp -n LogConfig.txt.sample $(DESTDIR)$(sysconfdir)/tawd/LogConfig.txt ; \
	cp -n URL.txt.sample $(DESTDIR)$(sysconfdir)/tawd/URL.txt ; \
	\
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/LogConfig.txt $(DESTDIR)$(localstatedir)/lib/tawd/LogConfig.txt ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt $(DESTDIR)$(localstatedir)/lib/tawd/ServerConfig.txt ; \
	mkdir -p $(DESTDIR)$(localstatedir)/log/tawd ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd $(DESTDIR)$(localstatedir)/lib/tawd/Logs ; \
	echo Done ;
	
