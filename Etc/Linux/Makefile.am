if SYSTEMD
initdir=$(sysconfdir)/init.d
init_DATA = tawd
EXTRA_DIST = \
	$(init_DATA)
	
install-data-hook:
	chmod a+rx $(sysconfdir)/init.d/tawd ; 
else
initdir=$(libdir)/systemd/system
init_DATA = tawd.service
EXTRA_DIST = \
	$(init_DATA)
	
install-data-hook:
	chmod a+rx $(libdir)/systemd/system/tawd.service ;
endif


