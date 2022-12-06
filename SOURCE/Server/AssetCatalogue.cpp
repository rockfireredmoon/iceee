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

#include "Config.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "ByteBuffer.h"

#include "Simulator.h"
#include "StringUtil.h"
#include <string.h>
#include "AssetCatalogue.h"
#include <algorithm>

AssetCatelogueManager g_AssetCatalogueManager;

int WriteAssetCatalogueItem(char *buffer, AssetCatalogueItem *item) {
	int wpos = 0;
	// 1
	wpos += PutStringUTF(&buffer[wpos], item->mName);
	// 2
	wpos += PutStringUTF(&buffer[wpos], item->GetAsset());
	// 3
	wpos += PutStringUTF(&buffer[wpos], item->mDescription);
	// 4
	STRINGLIST l;
	for(auto it = item->mParents.begin(); it != item->mParents.end(); ++it) {
		l.push_back((*it)->mName);
	}
	std::string p;
	Util::Join(l, ",", p);
	wpos += PutStringUTF(&buffer[wpos], p);
	//5
	wpos += PutStringUTF(&buffer[wpos],
			StringUtil::Format("%d", item->mType));
	//6
	wpos += PutStringUTF(&buffer[wpos], item->GetDisplayName());
	return wpos;
}

//
// PropItem
//

AssetCatalogueItem::AssetCatalogueItem() {

	mAsset = "";
	mName = "";
	mDescription = "";
	mType = AssetCatalogueItemType::OTHER;
}

AssetCatalogueItem::~AssetCatalogueItem() {
}

std::string AssetCatalogueItem::GetDisplayName() {
	return mDisplayName == "" ? mName : mDisplayName;
}

std::string AssetCatalogueItem::GetAsset() {
	return mAsset == "" ? (mType == AssetCatalogueItemType::PROP || mType == AssetCatalogueItemType::VARIANT ? mName : "") : mAsset;
}

//
// AuctionHouseManager
//

AssetCatelogueManager::AssetCatelogueManager() {

	mFavourites = new AssetCatalogueItem();
	mFavourites->mName = "Favourites";
	mFavourites->mDescription = "Your favourite scenery.";
}

AssetCatelogueManager::~AssetCatelogueManager() {
}

void AssetCatelogueManager::Search(AssetCatalogueSearch search, std::vector<AssetCatalogueItem*> *results) {
	SearchProps(results, search, mItems[""]);
}

int AssetCatelogueManager::LoadFromDirectory(std::string fileName) {


	AssetCatalogueItem *rootItem = new AssetCatalogueItem();
	mItems[""] = rootItem;

	AssetCatalogueItem *newItem = new AssetCatalogueItem();
	int lastType;

	Platform_DirectoryReader r;
	r.SetDirectory(fileName);
	r.ReadFiles();

	vector<std::string>::iterator it;
	int line = 0;
	std::string p;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		p = Platform::JoinPath(fileName, *it);
		line = 0;
		if(Util::HasEnding(p, ".txt")) {
			FileReader lfr;
			lfr.CommentStyle = Comment_Both;
			if (lfr.OpenText(p.c_str()) != Err_OK) {
				g_Logs.data->error("Could not open file [%v]", p);
				return -1;
			}

			while (lfr.FileOpen() == true) {
				int r = lfr.ReadLine();
				if (r > 0) {
					r = lfr.BreakUntil("=", '='); //Don't use SingleBreak since we won't be able to re-split later.
					lfr.BlockToStringC(0, Case_Upper);
					if (strcmp(lfr.SecBuffer, "[CATEGORY]") == 0 || strcmp(lfr.SecBuffer, "[ITEM]") == 0 || strcmp(lfr.SecBuffer, "[VARIANT]") == 0 || strcmp(lfr.SecBuffer, "[SKIN]") == 0) {
						if (newItem->mName.length() != 0) {
							if(newItem->mParents.size() == 0)
								newItem->mParents.push_back(rootItem);
							for(auto it = newItem->mParents.begin() ; it != newItem->mParents.end(); ++it) {
								(*it)->mChildren.push_back(newItem);
							}
							if(mItems.count(newItem->mName) > 0)
								g_Logs.data->warn("Asset catalogue item named [%v] defined again at line [%v] in [%v]", newItem->mName, line, p);
							mItems[newItem->mName] = newItem;
						}
						newItem = new AssetCatalogueItem();
						if(strcmp(lfr.SecBuffer, "[ITEM]") == 0) {
							lastType = AssetCatalogueItemType::PROP;
						}
						else if(strcmp(lfr.SecBuffer, "[CATEGORY]") == 0) {
							lastType = AssetCatalogueItemType::CATEGORY;
						}
						else if(strcmp(lfr.SecBuffer, "[VARIANT]") == 0) {
							lastType = AssetCatalogueItemType::VARIANT;
						}
						else if(strcmp(lfr.SecBuffer, "[SKIN]") == 0) {
							lastType = AssetCatalogueItemType::SKIN;
						}
						else
							lastType = AssetCatalogueItemType::OTHER;
						newItem->mType = lastType;
					} else {
						if (strcmp(lfr.SecBuffer, "ASSET") == 0 || strcmp(lfr.SecBuffer, "ID") == 0 /* <- deprecated */) {
							newItem->mAsset = lfr.BlockToStringC(1, 0);
						} else if (strcmp(lfr.SecBuffer, "NAME") == 0) {
							newItem->mName = lfr.BlockToStringC(1, 0);
							std::string path = Platform::JoinPath(
									Platform::JoinPath(g_Config.ResolveStaticDataPath(),
											"AssetCatalogue"), newItem->mName + ".html");
							if (Platform::FileExists(path)) {
								std::ifstream ifs(path);
								std::string content(
										(std::istreambuf_iterator<char>(ifs)),
										(std::istreambuf_iterator<char>()));
								newItem->mDescription = content;
							}
						} else if (strcmp(lfr.SecBuffer, "PARENT") == 0) {
							std::string pname = lfr.BlockToStringC(1, 0);
							STRINGLIST l;
							Util::Split(pname, ",", l);
							for(auto it = l.begin(); it != l.end(); ++it) {
								std::string aname = *it;
								if (mItems.count(aname) == 0) {
									g_Logs.data->error(
											"Item '%v' specifies the parent '%v', which doesn't exist (parents must be defined in the datafile before any items that use it)",
											newItem->mName, aname);
								} else {
									AssetCatalogueItem *par = mItems[aname];
									if ((lastType == AssetCatalogueItemType::PROP && par->mType == AssetCatalogueItemType::CATEGORY) ||
										(lastType == AssetCatalogueItemType::VARIANT && par->mType == AssetCatalogueItemType::PROP) ||
										(lastType == AssetCatalogueItemType::CATEGORY && par->mType == AssetCatalogueItemType::CATEGORY)){
										newItem->mParents.push_back(par);
									} else {
										g_Logs.data->error(
												"Item '%v' specifies the parent '%v', which is not the expected type.",
												newItem->mName, aname);
									}
								}
							}
						} else if (strcmp(lfr.SecBuffer, "KEYWORDS") == 0) {
							Util::TokenizeByWhitespace(lfr.BlockToStringC(1, 0),
									newItem->mKeywords);
						} else if (strcmp(lfr.SecBuffer, "TYPE") == 0) {
							newItem->mType = lfr.BlockToIntC(1);
						} else if (strcmp(lfr.SecBuffer, "ORDER") == 0) {
							newItem->mOrder = lfr.BlockToIntC(1);
						} else if (strcmp(lfr.SecBuffer, "DISPLAY") == 0) {
							newItem->mDisplayName = lfr.BlockToString(1);
						} else
							g_Logs.server->error(
									"Unknown identifier [%v] while reading from file %v.",
									lfr.SecBuffer, fileName);
					}
				}
				line++;
			}
			lfr.CloseCurrent();
		}
	}

	if (newItem->mName.length() != 0) {
		if(newItem->mParents.size() == 0)
			newItem->mParents.push_back(rootItem);
		if(mItems.count(newItem->mName) > 0)
			g_Logs.data->warn("Asset catalogue item named [%v] defined again at line [%v] in [%v]", newItem->mName, line, p);
		mItems[newItem->mName] = newItem;
		for(auto it = newItem->mParents.begin() ; it != newItem->mParents.end(); ++it) {
			(*it)->mChildren.push_back(newItem);
		}
	}
	rootItem->mChildren.push_back(mFavourites);
	mItems[mFavourites->mName] = mFavourites;
	mFavourites->mParents.push_back(rootItem);
	return 0;
}

AssetCatalogueItem* AssetCatelogueManager::GetItem(std::string name) {
	if(name == "Favourites") {
		return mFavourites;
	}
	else
		return mItems[name];
}


unsigned int AssetCatelogueManager::Count() {
	return mItems.size();
}

bool AssetCatelogueManager::Contains(std::string name) {
	return mItems.count(name) > 0;
}

AssetCatalogueItem* AssetCatelogueManager::GetByID(std::string id) {
	for (auto const& it : mItems) {
		AssetCatalogueItem* item = it.second;
		if(Util::CaseInsensitiveStringCompare(item->GetAsset(), id))
			return item;
	}

	return NULL;
}

std::vector<AssetCatalogueItem*> AssetCatelogueManager::GetChildren(std::string name, CharacterServerData *pld) {
	if(name == "Favourites") {
		std::vector<AssetCatalogueItem*> items;
		std::string stash = pld->charPtr->preferenceList.GetPrefValue("worldEditor.propStash");
		if(stash.size() > 0) {
			stash = "this.a <- " + Util::Unescape(Util::StripLeadingTrailing(stash, "\"")) + ";";

			// TODO shared vm
			// TODO do we need to close this? If so ... Creature appearance modifier needs to do the same
			//HSQUIRRELVM vm = sq_open(g_Config.SquirrelVMStackSize);
			HSQUIRRELVM vm = sq_open(g_Config.SquirrelVMStackSize);
			Sqrat::Script script(vm);

			script.CompileString(_SC(stash.c_str()));
			if (Sqrat::Error::Occurred(vm)) {
				g_Logs.server->error("Failed to parse prop stash. %v",
						Sqrat::Error::Message(vm).c_str());
			}
			else {
				script.Run();
				Sqrat::RootTable rootTable = Sqrat::RootTable(vm);
				Sqrat::Object placeholderObject = rootTable.GetSlot(_SC("a"));
				Sqrat::Array arr = placeholderObject.Cast<Sqrat::Array>();
				for (int i = 0; i < arr.GetSize(); i++) {
					Sqrat::Object obj = arr.GetSlot(SQInteger(i));
					if(!obj.IsNull()) {
						if(obj.GetType() == OT_STRING) {
				            std::string str = obj.Cast<std::string>();
							AssetCatalogueItem *item = GetByID(str);
							if(item != NULL) {
								items.push_back(item);
							}
							else
								g_Logs.data->error("No prop for user prop stash. %v (%v)", str, stash);
				        }
				        else
				            // Unexpected type
							g_Logs.data->error("Unexpected type in user prop stash. %v", stash);
				    }
				}
			}
			//sq_close(vm);
		}
		return items;
	}
	else
		return mItems[name]->mChildren;
}

void  AssetCatelogueManager::SearchProps(std::vector<AssetCatalogueItem*> *items, AssetCatalogueSearch search, AssetCatalogueItem* mItem) {
	if(search.mMax != 0 && items->size() >= search.mMax)
		return;
	if(Util::CaseInsensitiveStringFind(mItem->mName, search.mSearch) ||
	   ( mItem->mDescription.length() > 0 && Util::CaseInsensitiveStringFind(mItem->mDescription, search.mSearch)) ||
	   ( mItem->GetDisplayName().length() > 0 && Util::CaseInsensitiveStringFind(mItem->GetDisplayName(), search.mSearch)) ||
	   ( mItem->GetAsset().length() > 0 && Util::CaseInsensitiveStringFind(mItem->GetAsset(), search.mSearch)) ||
	   ( mItem->mName.length() > 0 && Util::CaseInsensitiveStringFind(mItem->mName, search.mSearch))) {
		items->push_back(mItem);
	}
	for (auto it = mItem->mChildren.begin(); it != mItem->mChildren.end(); ++it) {
		SearchProps(items, search, (*it));
	}
}

//
// PropSearch
//
AssetCatalogueSearch::AssetCatalogueSearch() {
	mPropTypeID = -1;
	mSearch = "";
	mMax = 99;
}

AssetCatalogueSearch::~AssetCatalogueSearch() {
}
