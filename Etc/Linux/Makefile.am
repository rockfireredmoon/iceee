initdir = $(sysconfdir)/init.d
init_DATA = tawd

EXTRA_DIST = \
	$(init_DATA)
	
install-data-hook:
	chmod a+rx $(sysconfdir)/init.d/tawd ;
