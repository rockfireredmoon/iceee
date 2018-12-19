/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#include <CompilerEnvironment.h>

#include <Cluster.h>
#include <Config.h>
#include <Components.h>
#include <Ability2.h>
#include <VirtualItem.h>
#include <util/Log.h>
#include <curl/curl.h>
#include <dirent.h>
#include <Item.h>
#include <VirtualItem.h>
#include <ZoneDef.h>
#include <CreditShop.h>
#include <IGForum.h>
#include <StringUtil.h>
#include <FriendStatus.h>

class VirtualItemPage: public AbstractEntity {
public:
	static const int ITEMS_PER_PAGE = 256;
	static const int AUTOSAVE_TIMER = 60000;
	int pageIndex;
	std::vector<VirtualItemDef> itemList;
	VirtualItemPage();
	VirtualItemPage(int page);
	void Clear(void);

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
};

class IGFCategoryPage: public AbstractEntity {
public:
	typedef std::map<int, IGFCategory> CATEGORYENTRY;
	typedef std::pair<int, IGFCategory> CATEGORYPAGE;
	CATEGORYENTRY mEntries;

	int mPage;
	IGFCategoryPage();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
};

class IGFThreadPage: public AbstractEntity {
public:
	typedef std::map<int, IGFThread> THREADENTRY;
	typedef std::pair<int, IGFThread> THREADPAIR;
	THREADENTRY mEntries;
	int mPage;
	IGFThreadPage();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

};

class IGFPostPage: public AbstractEntity {
public:
	typedef std::map<int, IGFPost> POSTENTRY;
	typedef std::pair<int, IGFPost> POSTPAIR;
	POSTENTRY mEntries;
	int mPage;
	IGFPostPage();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

};



VirtualItemPage :: VirtualItemPage()
{
	Clear();
}

VirtualItemPage :: VirtualItemPage(int page)
{
	Clear();
	pageIndex = page;
}


bool VirtualItemPage::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_VIRTUAL_ITEM, StringUtil::Format("%d", pageIndex), true);
	return true;
}

bool VirtualItemPage::ReadEntity(AbstractEntityReader *reader) {
	if (!reader->Exists())
		return false;

	reader->Index("ENTRY");

	STRINGLIST sections = reader->Sections();
	for(auto a = sections.begin(); a != sections.end(); ++a) {
		reader->PushSection(*a);
		VirtualItemDef vi;
		if(!vi.ReadEntity(reader))
			return false;
		itemList.push_back(vi);
		reader->PopSection();
	}

	return true;
}

bool VirtualItemPage::WriteEntity(AbstractEntityWriter *writer) {
	// NOTE: Not used. VirtualItemDef pages are now deprecated. Individual items
	// are loaded from the cluster
	return true;
}
void VirtualItemPage :: Clear(void)
{
	pageIndex = 0;
	itemList.clear();
}

IGFPostPage :: IGFPostPage()
{
	mPage = 0;
}

bool IGFPostPage::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_POST, StringUtil::Format("%d", mPage), true);
	return true;
}

bool IGFPostPage::ReadEntity(AbstractEntityReader *reader) {
	if (!reader->Exists())
		return false;

	reader->Index("ENTRY");

	STRINGLIST sections = reader->Sections();
	for(auto a = sections.begin(); a != sections.end(); ++a) {
		reader->PushSection(*a);
		IGFPost thread;
		thread.mID = reader->ValueInt("ID");
		if(!thread.ReadEntity(reader))
			return false;
		mEntries[thread.mID] = thread;
		reader->PopSection();
	}

	return true;
}

bool IGFPostPage::WriteEntity(AbstractEntityWriter *writer) {
	// NOTE: Not used. IGFThreadPage pages are now deprecated. Individual items
	// are loaded from the cluster
	return true;
}




IGFCategoryPage :: IGFCategoryPage()
{
	mPage = 0;
}

bool IGFCategoryPage::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_CATEGORY, StringUtil::Format("%d", mPage), true);
	return true;
}

bool IGFCategoryPage::ReadEntity(AbstractEntityReader *reader) {
	if (!reader->Exists())
		return false;

	reader->Index("ENTRY");

	STRINGLIST sections = reader->Sections();
	for(auto a = sections.begin(); a != sections.end(); ++a) {
		reader->PushSection(*a);
		IGFCategory cat;
		cat.mID = reader->ValueInt("ID");
		if(!cat.ReadEntity(reader))
			return false;
		mEntries[cat.mID] = cat;
		reader->PopSection();
	}

	return true;
}

bool IGFCategoryPage::WriteEntity(AbstractEntityWriter *writer) {
	// NOTE: Not used. IGFCategoryPage pages are now deprecated. Individual items
	// are loaded from the cluster
	return true;
}

IGFThreadPage :: IGFThreadPage()
{
	mPage = 0;
}

bool IGFThreadPage::EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_IGF_THREAD, StringUtil::Format("%d", mPage), true);
	return true;
}

bool IGFThreadPage::ReadEntity(AbstractEntityReader *reader) {
	if (!reader->Exists())
		return false;

	reader->Index("ENTRY");

	STRINGLIST sections = reader->Sections();
	for(auto a = sections.begin(); a != sections.end(); ++a) {
		reader->PushSection(*a);
		IGFThread thread;
		thread.mID = reader->ValueInt("ID");
		if(!thread.ReadEntity(reader))
			return false;
		mEntries[thread.mID] = thread;
		reader->PopSection();
	}

	return true;
}

bool IGFThreadPage::WriteEntity(AbstractEntityWriter *writer) {
	// NOTE: Not used. IGFThreadPage pages are now deprecated. Individual items
	// are loaded from the cluster
	return true;
}

int main(int argc, char *argv[]) {

	if (PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
		return 0;
	}

	std::string userDataPath;
	bool configSet = false;

	el::Level lvl = el::Level::Warning;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			if(!configSet) {
				configSet = true;
				g_Config.LocalConfigurationPath.clear();
			}
			g_Config.LocalConfigurationPath.push_back(argv[++i]);
		} else if (strcmp(argv[i], "-d") == 0) {
			lvl = el::Level::Debug;
		} else if (strcmp(argv[i], "-i") == 0) {
			lvl = el::Level::Info;
		} else if (strcmp(argv[i], "-q") == 0) {
			lvl = el::Level::Unknown;
		} else {
			userDataPath = argv[i];
		}
	}

	g_Logs.Init(lvl, true);
	g_Logs.data->info("Text-To-Redis");

	curl_global_init(CURL_GLOBAL_DEFAULT);
	g_PlatformTime.Init();

	std::vector<std::string> paths = g_Config.ResolveLocalConfigurationPath();
	for (std::vector<std::string>::iterator it = paths.begin();
			it != paths.end(); ++it) {
		std::string dir = *it;
		std::string filename = Platform::JoinPath(dir, "ServerConfig.txt");
		if(!LoadConfig(filename) && it == paths.begin())
			g_Logs.data->error("Could not open server configuration file: %v", filename);
	}

	if(userDataPath.length() == 0)
		userDataPath = "User";

	g_ClusterManager.mNoEvents = true;
	for (std::vector<std::string>::iterator it = paths.begin();
			it != paths.end(); ++it) {
		std::string dir = *it;
		g_ClusterManager.LoadConfiguration(Platform::JoinPath(*it, "Cluster.txt"));
	}

	if(!g_ClusterManager.Init())
		return 1;

	std::string path;
	DIR *dir;
	struct dirent *ent;

	/* We need some static data */
	g_ItemManager.LoadData();
	g_AbilityManager.LoadData();
	g_SceneryManager.LoadData();
	g_ZoneDefManager.LoadData();

	g_Logs.data->info("Reading user data from %v", userDataPath);

	/* Virtual Items */
	path = Platform::JoinPath(userDataPath, "VirtualItems");
	if ((dir = opendir(path.c_str())) != NULL) {
		int maxc = ItemManager::BASE_VIRTUAL_ITEM_ID;
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if (id != 0) {
				std::string fullfile = Platform::JoinPath(path, ent->d_name);
				g_Logs.data->debug("Found virtual item file %v (%v)", id, fullfile);
				TextFileEntityReader r(fullfile, Case_None, Comment_Semi);

				VirtualItemPage vip;
				vip.pageIndex = id;

				if (vip.EntityKeys(&r) && vip.ReadEntity(&r)) {
					g_Logs.data->info("Loaded virtual item page %v", vip.pageIndex);
					for(auto a2 = vip.itemList.begin(); a2 != vip.itemList.end(); ++a2) {
						VirtualItemDef vi = *a2;
						if(vi.mID > maxc)
							maxc = vi.mID;
						if (g_ClusterManager.WriteEntity(&vi)) {
							g_Logs.data->info("    Migrated virtual item %v", vi.mID);
						} else {
							g_Logs.data->error("Failed to write virtual item %v",
									ent->d_name);
							return 1;
						}
					}
				} else {
					g_Logs.data->error("Failed to load virtual item from %v",
							ent->d_name);
					return 1;
				}
				r.PopSection();
			}
		}
		g_ClusterManager.SetKey(ID_NEXT_VIRTUAL_ITEM_ID, StringUtil::Format("%d", maxc));
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open virtual item directory %v, does it exist?", path);
		return 1;
	}

	/* Accounts */

	path = Platform::JoinPath(userDataPath,
			"Accounts");
	if ((dir = opendir(path.c_str())) != NULL) {
		int maxa = 1;
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if (id > 0) {
				AccountData ad;
				ad.ID = id;
				if(id > maxa)
					maxa = id;
				std::string fullfile = Platform::JoinPath(
						Platform::JoinPath(userDataPath,
								"Accounts"), ent->d_name);
				g_Logs.data->debug("Found account %v (%v)", id, fullfile);
				TextFileEntityReader r(fullfile, Case_None, Comment_Semi);
				r.PushSection("ENTRY");
				if (ad.EntityKeys(&r) && ad.ReadEntity(&r)) {
					if (g_ClusterManager.WriteEntity(&ad)) {
						g_AccountManager.AppendQuickData(&ad);
						g_Logs.data->info("Migrated account %v", ent->d_name);
					} else {
						g_Logs.data->error("Failed to write account %v",
								ent->d_name);
						return 1;
					}
				} else {
					g_Logs.data->error("Failed to read account from %v",
							ent->d_name);
					return 1;
				}
				r.PopSection();

				/* Write the bi-directional character name / ID looks ups as we now have the
				 * data from the character caches
				 *
				 * TODO should use real Character data when that is ported?
				 */
				for (auto it = ad.characterCache.cacheData.begin();
						it != ad.characterCache.cacheData.end(); ++it) {
					g_UsedNameDatabase.Add((*it).creatureDefID,
							(*it).display_name);
				}
			}
		}

		g_ClusterManager.SetKey(ID_NEXT_ACCOUNT_ID, StringUtil::Format("%d", maxa));

		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open Accounts directory %v, does it exist?", path);
		return 1;
	}

	/* ZoneDefs */
	path = Platform::JoinPath(userDataPath, "ZoneDef");
	if ((dir = opendir(path.c_str())) != NULL) {
		int maxz = ZoneDefManager::GROVE_ZONE_ID_DEFAULT;
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if (id > 0) {
				ZoneDefInfo zd;
				zd.mID = id;
				if(id > maxz)
					maxz = id;
				std::string fullfile = Platform::JoinPath(
						Platform::JoinPath(userDataPath,
								"ZoneDef"), ent->d_name);
				g_Logs.data->debug("Found ZoneDef %v (%v)", id, fullfile);
				TextFileEntityReader r(fullfile, Case_None, Comment_Semi);
				r.PushSection("ENTRY");
				if (zd.EntityKeys(&r) && zd.ReadEntity(&r)) {
					if (g_ClusterManager.WriteEntity(&zd)) {

						g_ClusterManager.SetKey(
								StringUtil::Format("%s:%s",
										KEYPREFIX_WARP_NAME_TO_ZONE_ID.c_str(),
										zd.mWarpName.c_str()),
								StringUtil::Format("%d", zd.mID));
						g_ClusterManager.ListAdd(
								StringUtil::Format("%s:%s",
										LISTPREFIX_GROVE_NAME_TO_ZONE_ID.c_str(),
										zd.mGroveName.c_str()),
								StringUtil::Format("%d", zd.mID));
						g_ClusterManager.ListAdd(
								StringUtil::Format("%s:%d",
										LISTPREFIX_ACCOUNT_ID_TO_ZONE_ID.c_str(),
										zd.mAccountID),
								StringUtil::Format("%d", zd.mID));

						g_Logs.data->info("Migrated zone def %v", ent->d_name);
					} else {
						g_Logs.data->error("Failed to zone def %v",
								ent->d_name);
						return 1;
					}
				} else {
					g_Logs.data->error("Failed to load zone def from %v",
							ent->d_name);
					return 1;
				}
				r.PopSection();
			}
		}
		g_ClusterManager.SetKey(ID_NEXT_ZONE_ID, StringUtil::Format("%d", maxz));
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open ZoneDefs directory %v, does it exist?", path);
		return 1;
	}

	/* Characters */
	path = Platform::JoinPath(userDataPath, "Characters");
	if ((dir = opendir(path.c_str())) != NULL) {
		int maxc = AccountManager::DEFAULT_CHARACTER_ID;
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if (id != 0) {
				CharacterData zd;
				zd.cdef.CreatureDefID = id;
				if(id < maxc)
					maxc = id;
				std::string fullfile = Platform::JoinPath(
						Platform::JoinPath(userDataPath,
								"Characters"), ent->d_name);
				g_Logs.data->debug("Found character %v (%v)", id, fullfile);
				TextFileEntityReader r(fullfile, Case_None, Comment_Semi);
				r.PushSection("ENTRY");
				if (zd.EntityKeys(&r) && zd.ReadEntity(&r)) {
					if (g_ClusterManager.WriteEntity(&zd)) {

						SocialWindowEntry we;
						we.creatureDefID = zd.cdef.CreatureDefID;
						we.level = zd.cdef.css.level;
						we.name = zd.cdef.css.display_name;
						we.online = false;
						we.profession = zd.cdef.css.profession;
						we.shard = g_ClusterManager.mShardName;
						we.status = zd.StatusText;
						g_ClusterManager.WriteEntity(&we);

						/* Create friend network here rather than converting the existing
						 * file. We can derive the data from the characters friend list
						 * and reset all the state
						 */
						for(auto fr = zd.friendList.begin(); fr != zd.friendList.end(); ++fr)
							g_ClusterManager.ListAdd(StringUtil::Format("%s:%d", LISTPREFIX_FRIEND_NETWORK.c_str(), we.creatureDefID), StringUtil::Format("%d", (*fr).CDefID));

						g_Logs.data->info("Migrated character %v", ent->d_name);
					} else {
						g_Logs.data->error("Failed to migrate character %v",
								ent->d_name);
						return 1;
					}
				} else {
					g_Logs.data->error("Failed to load character from %v",
							ent->d_name);
					return 1;
				}
			}
		}
		g_ClusterManager.SetKey(ID_NEXT_CHARACTER_ID, StringUtil::Format("%d", maxc));
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open Characters directory %v, does it exist?", path);
		return 1;
	}

	/* Grove */
	path = Platform::JoinPath(userDataPath, "Grove");
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			string zonepath = Platform::JoinPath(path, s);
			int sid = atoi(s.c_str());
			if (sid != 0 && Platform::DirExists(zonepath)) {
				DIR *zonedir;
				if ((zonedir = opendir(zonepath.c_str())) != NULL) {
					int maxID = 0;
					while ((ent = readdir(zonedir)) != NULL) {
						string page = string(ent->d_name);
						string tilepath = Platform::JoinPath(zonepath, page);
						if(Util::HasEnding(page, ".txt")) {
							int x = atoi(page.substr(1, 4).c_str());
							int y = atoi(page.substr(5, 8).c_str());

							SceneryPage p;
							p.mTileX = x;
							p.mTileY = y;
							p.mZone = sid;
							g_Logs.data->info("Migrating grove scenery %v:%v:%v (%v objects)", sid, x, y);

							TextFileEntityReader r(tilepath, Case_None, Comment_Semi);
							if(p.EntityKeys(&r) && p.ReadEntity(&r)) {
								g_Logs.data->info("  Read %v OK. Contains %v objects", tilepath, p.mSceneryList.size());
								for(auto a = p.mSceneryList.begin(); a != p.mSceneryList.end(); ++a) {
									SceneryObject o = (*a).second;
									if(o.ID > maxID)
										maxID = o.ID;
									g_Logs.data->info("    Object: %v [%v]", o.ID, o.Asset);
									if(g_ClusterManager.WriteEntity(&o)) {
										if(sid >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
											g_ClusterManager.ListAdd(StringUtil::Format("%s:%d:%d:%d", KEYPREFIX_GROVE.c_str(), sid, x, y), StringUtil::Format("%d", o.ID));
										else
											g_ClusterManager.ListAdd(StringUtil::Format("%s:%d:%d:%d", KEYPREFIX_SCENERY.c_str(), sid, x, y), StringUtil::Format("%d", o.ID));
									}
									else {
										g_Logs.data->error("Failed to write scenery object %v in %v", (*a).first,
												tilepath);
										return 1;
									}
								}
							} else {
								g_Logs.data->error("Failed to load scenery tile from %v",
										tilepath);
								return 1;
							}
						}
					}
					g_ClusterManager.SetKey(StringUtil::Format("%s:%d", ID_NEXT_SCENERY.c_str(), sid), StringUtil::Format("%d", maxID));
				}
			}
		}
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open Characters directory %v, does it exist?", path);
		return 1;
	}

	/* Static zones. We don't write these to redis, but we scan all scenery looking
	 * for the highest prop ID in the zone, and set the zones next ID value to that */
	path = Platform::JoinPath(g_Config.ResolveVariableDataPath(), "Scenery");
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			string zonepath = Platform::JoinPath(path, s);
			int sid = atoi(s.c_str());
			if (sid != 0 && Platform::DirExists(zonepath)) {
				DIR *zonedir;
				if ((zonedir = opendir(zonepath.c_str())) != NULL) {
					int maxID = 0;
					while ((ent = readdir(zonedir)) != NULL) {
						string page = string(ent->d_name);
						string tilepath = Platform::JoinPath(zonepath, page);
						if(Util::HasEnding(page, ".txt")) {
							int x = atoi(page.substr(1, 4).c_str());
							int y = atoi(page.substr(5, 8).c_str());

							SceneryPage p;
							p.mTileX = x;
							p.mTileY = y;
							p.mZone = sid;
							g_Logs.data->info("Reading overworld scenery %v:%v:%v", sid, x, y);

							TextFileEntityReader r(tilepath, Case_None, Comment_Semi);
							if(p.EntityKeys(&r) && p.ReadEntity(&r)) {
								g_Logs.data->info("  Read %v OK. Contains %v objects", tilepath, p.mSceneryList.size());
								for(auto a = p.mSceneryList.begin(); a != p.mSceneryList.end(); ++a) {
									SceneryObject o = (*a).second;
									if(o.ID > maxID)
										maxID = o.ID;
									g_Logs.data->info("    Object: %v [%v]", o.ID, o.Asset);
								}
							} else {
								g_Logs.data->error("Failed to load overworld scenery tile from %v",
										tilepath);
								return 1;
							}
						}
					}
					g_Logs.data->info("Setting current ID for zone %v to %v", sid, maxID);
					g_ClusterManager.SetKey(StringUtil::Format("%s:%d", ID_NEXT_SCENERY.c_str(), sid), StringUtil::Format("%d", maxID));
				}
			}
		}
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open Characters directory %v, does it exist?", path);
		return 1;
	}

	/* IGF Categories */
	path = Platform::JoinPath(Platform::JoinPath(userDataPath, "IGForum"), "Category");
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			if(s == ".." || s == ".")
				continue;

			Util::RemoveStringsFrom(".txt", s);
			string catpath = Platform::JoinPath(path, ent->d_name);
			int id = atoi(s.c_str());

			IGFCategoryPage p;
			p.mPage = id;
			g_Logs.data->info("Migrating IGF category %v", id);

			TextFileEntityReader r(catpath, Case_None, Comment_Semi);
			int maxID = 0;
			if(p.EntityKeys(&r) && p.ReadEntity(&r)) {
				g_Logs.data->info("  Read %v OK. Contains %v objects", catpath, p.mEntries.size());
				for(auto a = p.mEntries.begin(); a != p.mEntries.end(); ++a) {
					IGFCategory o = (*a).second;
					if(o.mID > maxID)
						maxID = o.mID;
					g_Logs.data->info("    Category: %v [%v]", o.mID, o.mTitle);
					if(g_ClusterManager.WriteEntity(&o)) {
						g_ClusterManager.ListAdd(LISTPREFIX_IGF_CATEGORIES, StringUtil::Format("%d", o.mID));
					}
					else {
						g_Logs.data->error("Failed to write scenery object %v in %v", (*a).first,
								catpath);
						return 1;
					}
				}
			} else {
				g_Logs.data->error("Failed to load IGF category tile from %v", catpath);
				return 1;
			}
			g_ClusterManager.SetKey(ID_IGF_CATEGORY_ID, StringUtil::Format("%d", maxID));
		}
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open IGF categories directory %v, does it exist?", path);
		return 1;
	}

	/* IGF Threads */
	path = Platform::JoinPath(Platform::JoinPath(userDataPath, "IGForum"), "Thread");
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			if(s == ".." || s == ".")
				continue;

			Util::RemoveStringsFrom(".txt", s);
			string threadpath = Platform::JoinPath(path, ent->d_name);
			int id = atoi(s.c_str());

			IGFThreadPage p;
			p.mPage = id;
			g_Logs.data->info("Migrating IGF thread %v", id);

			TextFileEntityReader r(threadpath, Case_None, Comment_Semi);
			int maxID = 0;
			if(p.EntityKeys(&r) && p.ReadEntity(&r)) {
				g_Logs.data->info("  Read %v OK. Contains %v objects", threadpath, p.mEntries.size());
				for(auto a = p.mEntries.begin(); a != p.mEntries.end(); ++a) {
					IGFThread o = (*a).second;
					if(o.mID > maxID)
						maxID = o.mID;
					g_Logs.data->info("    Thread: %v [%v]", o.mID, o.mTitle);
					if(!g_ClusterManager.WriteEntity(&o)) {
						g_Logs.data->error("Failed to write thread %v in %v", (*a).first,
								threadpath);
						return 1;
					}
				}
			} else {
				g_Logs.data->error("Failed to load IGF threads tile from %v", threadpath);
				return 1;
			}
			g_ClusterManager.SetKey(ID_IGF_THREAD_ID, StringUtil::Format("%d", maxID));
		}
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open IGF threads directory %v, does it exist?", path);
		return 1;
	}

	/* IGF Posts */
	path = Platform::JoinPath(Platform::JoinPath(userDataPath, "IGForum"), "Post");
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			if(s == ".." || s == ".")
				continue;

			Util::RemoveStringsFrom(".txt", s);
			string postpath = Platform::JoinPath(path, ent->d_name);
			int id = atoi(s.c_str());

			IGFPostPage p;
			p.mPage = id;
			g_Logs.data->info("Migrating IGF post %v", id);

			TextFileEntityReader r(postpath, Case_None, Comment_Semi);
			int maxID = 0;
			if(p.EntityKeys(&r) && p.ReadEntity(&r)) {
				g_Logs.data->info("  Read %v OK. Contains %v objects", postpath, p.mEntries.size());
				for(auto a = p.mEntries.begin(); a != p.mEntries.end(); ++a) {
					IGFPost o = (*a).second;
					if(o.mID > maxID)
						maxID = o.mID;
					g_Logs.data->info("    Post: %v", o.mID);
					if(!g_ClusterManager.WriteEntity(&o)) {
						g_Logs.data->error("Failed to write post %v in %v", (*a).first,
								postpath);
						return 1;
					}
				}
			} else {
				g_Logs.data->error("Failed to load IGF post from %v", postpath);
				return 1;
			}
			g_ClusterManager.SetKey(ID_IGF_POST_ID, StringUtil::Format("%d", maxID));
		}
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open IGF posts directory %v, does it exist?", path);
		return 1;
	}

	/* Credit Shop */
	path = Platform::JoinPath(g_Config.ResolveVariableDataPath(), "CreditShop");
	if ((dir = opendir(path.c_str())) != NULL) {
		int maxc = 0;
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if (id != 0) {
				CS::CreditShopItem zd;
				zd.mId = id;
				if(id > maxc)
					maxc = id;
				std::string fullfile = Platform::JoinPath(path, ent->d_name);
				g_Logs.data->info("Migrating Credit Shop item %v (%v)", id, fullfile);
				TextFileEntityReader r(fullfile, Case_None, Comment_Semi);
				r.PushSection("ENTRY");
				if (zd.EntityKeys(&r) && zd.ReadEntity(&r)) {
					if (!g_ClusterManager.WriteEntity(&zd)) {
						g_Logs.data->error("Failed to migrate credit shop item %v",
								ent->d_name);
						return 1;
					}
					else
						g_ClusterManager.ListAdd(LISTPREFIX_CS_ITEMS, StringUtil::Format("%d", zd.mId));
				} else {
					g_Logs.data->error("Failed to load credit shop item from %v",
							ent->d_name);
					return 1;
				}
			}
		}
		g_ClusterManager.SetKey(ID_CS_ITEM_ID, StringUtil::Format("%d", maxc));
		closedir(dir);
	} else {
		g_Logs.data->error(
				"Failed to open Credit Shop directory %v, does it exist?", path);
		return 1;
	}

	/* Shutdown cleanly so all messags get sent */
	g_ClusterManager.Shutdown(true);
}
