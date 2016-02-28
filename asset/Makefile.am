assetsdir = $(datarootdir)/tawd/assets
assets_DATA = Account.html \
	main.css \
	md5.js \
	ping.html \
	ping.txt \
	RemoteServerTools.html \
	ResetPassword.html \
	who.js

EXTRA_DIST = \
	$(assets_DATA)
	
	
install-exec-hook:
	mkdir -p $(DESTDIR)$(sysconfdir)/tawd ; \
	cp -n in_game_news loading_announcements tips $(DESTDIR)$(sysconfdir)/tawd ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/asset ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/in_game_news $(DESTDIR)$(localstatedir)/lib/tawd/asset/in_game_news ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/loading_announcements $(DESTDIR)$(localstatedir)/lib/tawd/asset/loading_announcements ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/tips $(DESTDIR)$(localstatedir)/lib/tawd/asset/tips ; \
	for i in $(DESTDIR)$(datarootdir)/tawd/assets/* ; do \
		ln -sf "$$i" $(DESTDIR)$(localstatedir)/lib/tawd/asset ; \
	done ;
	
uninstall-hook:
	rm -f $(DESTDIR)$(localstatedir)/lib/tawd/asset/in_game_news ; \
	rm -f $(DESTDIR)$(localstatedir)/lib/tawd/asset/loading_announcements ; \
	rm -f $(DESTDIR)$(localstatedir)/lib/tawd/asset/tips ; \
	rm -f $(DESTDIR)$(localstatedir)/lib/tawd/asset/in_game_news ; \
	for i in $(DESTDIR)$(datarootdir)/tawd/assets/* ; do \
		rm -f $(DESTDIR)$(localstatedir)/lib/tawd/asset/$(basename $$i) ; \
	done ;