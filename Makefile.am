ACLOCAL_AMFLAGS = -I m4
SUBDIRS = asset SOURCE AIScript Data Instance ItemMod Loot Packages QuestScripts SpawnPackages Scenery Etc
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
	\
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd ; \
	ln -sf $(DESTDIR)$(sysconfdir)/tawd/ServerConfig.txt $(DESTDIR)$(localstatedir)/lib/tawd/ServerConfig.txt ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/AIScript $(DESTDIR)$(localstatedir)/lib/tawd/AIScript ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/Data $(DESTDIR)$(localstatedir)/lib/tawd/Data ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/ItemMod $(DESTDIR)$(localstatedir)/lib/tawd/ItemMod ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/Loot $(DESTDIR)$(localstatedir)/lib/tawd/Loot ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/Packages $(DESTDIR)$(localstatedir)/lib/tawd/Packages ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/QuestScripts $(DESTDIR)$(localstatedir)/lib/tawd/QuestScripts ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/Scenery $(DESTDIR)$(localstatedir)/lib/tawd/Scenery ; \
	ln -sf $(DESTDIR)$(datarootdir)/tawd/SpawnPackages $(DESTDIR)$(localstatedir)/lib/tawd/SpawnPackages ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Accounts ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Audit ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/AuctionHouse ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Characters ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Clan ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/CreditShop ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Dynamic ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Grove ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/IGForum ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/Petitions ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/VirtualItems ; \
	mkdir -p $(DESTDIR)$(localstatedir)/lib/tawd/ZoneDef ; \
	mkdir -p $(DESTDIR)$(localstatedir)/log/tawd ; \
	touch $(DESTDIR)$(localstatedir)/log/tawd/CheatLog.log ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd/CheatLog.log $(DESTDIR)$(localstatedir)/lib/tawd/CheatLog.log ; \
	touch $(DESTDIR)$(localstatedir)/log/tawd/ErrorLog.log ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd/ErrorLog.log $(DESTDIR)$(localstatedir)/lib/tawd/ErrorLog.log ; \
	touch $(DESTDIR)$(localstatedir)/log/tawd/EventLog.log ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd/EventLog.log $(DESTDIR)$(localstatedir)/lib/tawd/EventLog.log ; \
	touch $(DESTDIR)$(localstatedir)/log/tawd/RegionChat.log ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd/RegionChat.log $(DESTDIR)$(localstatedir)/lib/tawd/RegionChat.log ; \
	touch $(DESTDIR)$(localstatedir)/log/tawd/ServerLog.log ; \
	ln -sf $(DESTDIR)$(localstatedir)/log/tawd/ServerLog.log $(DESTDIR)$(localstatedir)/lib/tawd/ServerLog.log ; \
	echo Done ;
	
