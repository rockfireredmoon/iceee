ACLOCAL_AMFLAGS = -I m4
SUBDIRS = asset SOURCE Etc Local
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
	cp -n Local/ServerConfig.txt.sample $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt ; \
	sed -i -e 's~StaticDataPath=\.\./iceee-data/Static~StaticDataPath=$(DESTDIR)$(datarootdir)/tawd/Static~g' $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt ; \
	sed -i -e 's~VariableDataPath=\.\./iceee-data/Variable~VariableDataPath=$(DESTDIR)$(localstatedir)/lib/tawd/Variable~g' $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt ; \
	sed -i -e 's~HTTPCARFolder=\.\./iceee-assets~HTTPCARFolder=$(DESTDIR)$(datarootdir)/tawd/asset~g' $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt ; \
	cp -n Local/LogConfig.txt.sample $(DESTDIR)$(sysconfdir)/tawd/LogConfig.txt ; \
	cp -n Local/Cluster.txt.sample $(DESTDIR)$(sysconfdir)/tawd/Cluster.txt ; \
	cp -n Local/URL.txt.sample $(DESTDIR)$(sysconfdir)/tawd/URL.txt ; \
	\
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Local ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/LogConfig.txt $(DESTDIR)$(localstatedir)/lib/tawd/Local/LogConfig.txt ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt $(DESTDIR)$(localstatedir)/lib/tawd/Local/ServerConfig.txt ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/URL.txt $(DESTDIR)$(localstatedir)/lib/tawd/Local/URL.txt ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/Cluster.txt $(DESTDIR)$(localstatedir)/lib/tawd/Local/Cluster.txt ; \
	mkdir -p $(DESTDIR)$(localstatedir)/log/tawd ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd $(DESTDIR)$(localstatedir)/lib/tawd/Logs ; \
	echo Done ;
	
