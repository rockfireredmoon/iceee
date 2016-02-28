
	
install-exec-hook:
	mkdir -p $(DESTDIR)$(datarootdir)/tawd/Instance ; \
	for INSTANCE in * ; do \
		if [ -d "$$INSTANCE" ] ; then \
			echo cp -pR $$INSTANCE $(DESTDIR)$(datarootdir)/tawd/Instance ; \
			cp -pR $$INSTANCE $(DESTDIR)$(datarootdir)/tawd/Instance ; \
		fi ; \
	done;
	
uninstall-hook:
	rm -fr $(DESTDIR)$(datarootdir)/tawd/Instance ;
