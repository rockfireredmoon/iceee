
	
install-exec-hook:
	mkdir -p $(DESTDIR)$(datarootdir)/tawd/Scenery ; \
	for INSTANCE in * ; do \
		if [ -d "$$INSTANCE" ] ; then \
			cp -pR "$$INSTANCE" $(DESTDIR)$(datarootdir)/tawd/Scenery ; \
		fi ; \
	done; \
	for ZIP in *.zip ; do \
		cp -p "$$ZIP"  $(DESTDIR)$(datarootdir)/tawd/Scenery ; \
	done;
	
uninstall-hook:
	rm -fr $(DESTDIR)$(datarootdir)/tawd/Scenery ;
