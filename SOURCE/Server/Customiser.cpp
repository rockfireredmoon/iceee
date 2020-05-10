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

#include "Customiser.h"
#include "Config.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "ByteBuffer.h"

#include "Simulator.h"
#include "StringUtil.h"
#include <string.h>
#include <algorithm>

PropManager g_PropManager;

int WritePropItem(char *buffer, PropItem *item) {
	int wpos = 0;
	// 1
	wpos += PutStringUTF(&buffer[wpos], item->mName.c_str());
	// 2
	wpos += PutStringUTF(&buffer[wpos], item->mID.c_str());
	// 3
	wpos += PutStringUTF(&buffer[wpos], item->mDescription.c_str());
	// 4
	if (item->mParent == NULL)
		wpos += PutStringUTF(&buffer[wpos], "");
	else
		wpos += PutStringUTF(&buffer[wpos], item->mParent->mName.c_str());
	//5
	wpos += PutStringUTF(&buffer[wpos],
			StringUtil::Format("%d", item->mPropTypeID).c_str());
	return wpos;
}

//
// PropItem
//

PropItem::PropItem() {

	mID = "";
	mName = "";
	mDescription = "";
	mParent = NULL;
	mPropTypeID = PropType::OTHER;
}

PropItem::~PropItem() {
}

//
// AuctionHouseManager
//

PropManager::PropManager() {

	mFavourites = new PropItem();
	mFavourites->mName = "Favourites";
	mFavourites->mDescription = "Your favourite scenery.";
}

PropManager::~PropManager() {
}

void PropManager::Search(PropSearch search, std::vector<PropItem*> *results) {
	SearchProps(results, search, mItems[""]);
}

int PropManager::LoadFromFile(std::string fileName) {

	FileReader lfr;
	if (lfr.OpenText(fileName.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", fileName);
		return -1;
	}

	PropItem *rootItem = new PropItem();
	mItems[""] = rootItem;

	lfr.CommentStyle = Comment_Both;
	PropItem *newItem = new PropItem();
	newItem->mParent = rootItem;
	bool item;

	while (lfr.FileOpen() == true) {
		int r = lfr.ReadLine();
		if (r > 0) {
			r = lfr.BreakUntil("=", '='); //Don't use SingleBreak since we won't be able to re-split later.
			lfr.BlockToStringC(0, Case_Upper);
			if (strcmp(lfr.SecBuffer, "[CATEGORY]") == 0 || strcmp(lfr.SecBuffer, "[ITEM]") == 0) {
				if (newItem->mName.length() != 0) {
					newItem->mParent->mChildren.push_back(newItem);
					mItems[newItem->mName] = newItem;
				}
				newItem = new PropItem();
				newItem->mParent = rootItem;
				item = strcmp(lfr.SecBuffer, "[ITEM]") == 0;
				if(!item)
					newItem->mPropTypeID = PropType::CATEGORY;
			} else {
				if (strcmp(lfr.SecBuffer, "ID") == 0) {
					newItem->mID = lfr.BlockToStringC(1, 0);
					std::string path = Platform::JoinPath(
							Platform::JoinPath(g_Config.ResolveStaticDataPath(),
									"PropCatalogue"), newItem->mID + ".html");
					if (Platform::FileExists(path)) {
						std::ifstream ifs(path);
						std::string content(
								(std::istreambuf_iterator<char>(ifs)),
								(std::istreambuf_iterator<char>()));
						newItem->mDescription = content;
					}
				} else if (strcmp(lfr.SecBuffer, "NAME") == 0) {
					newItem->mName = lfr.BlockToStringC(1, 0);
				} else if (strcmp(lfr.SecBuffer, "PARENT") == 0) {
					std::string pname = lfr.BlockToStringC(1, 0);
					if (mItems.count(pname) == 0) {
						g_Logs.data->error(
								"Item '%v' specifies the parent '%v', which doesn't exist (parents must be defined in the datafile before any items that use it)",
								newItem->mName, pname);
					} else {
						PropItem *par = mItems[pname];
						if (item && par->mPropTypeID != PropType::CATEGORY) {
							g_Logs.data->error(
									"Item '%v' specifies the parent '%v', which is not a category, it is an item.",
									newItem->mName, pname);
						} else {
							newItem->mParent = par;
						}
					}
				} else if (strcmp(lfr.SecBuffer, "KEYWORDS") == 0) {
					Util::TokenizeByWhitespace(lfr.BlockToStringC(1, 0),
							newItem->mKeywords);
				} else if (strcmp(lfr.SecBuffer, "TYPE") == 0) {
					newItem->mPropTypeID = lfr.BlockToIntC(1);
				} else
					g_Logs.server->error(
							"Unknown identifier [%v] while reading from file %v.",
							lfr.SecBuffer, fileName);
			}
		}
	}
	lfr.CloseCurrent();
	if (newItem->mName.length() != 0) {
		mItems[newItem->mName] = newItem;
		newItem->mParent->mChildren.push_back(newItem);
	}
	rootItem->mChildren.push_back(mFavourites);
	mItems[mFavourites->mName] = mFavourites;
	mFavourites->mParent = rootItem;
	return 0;
}

PropItem* PropManager::GetItem(std::string name) {
	if(name == "Favourites") {
		return mFavourites;
	}
	else
		return mItems[name];
}


unsigned int PropManager::Count() {
	return mItems.size();
}

bool PropManager::Contains(std::string name) {
	return mItems.count(name) > 0;
}

PropItem* PropManager::GetByID(std::string id) {
	for (auto const& it : mItems) {
		PropItem* item = it.second;
		if(Util::CaseInsensitiveStringCompare(item->mID, id))
			return item;
	}

	return NULL;
}

std::vector<PropItem*> PropManager::GetChildren(std::string name, CharacterServerData *pld) {
	if(name == "Favourites") {
		std::vector<PropItem*> items;
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
							PropItem *item = GetByID(str);
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

void  PropManager::SearchProps(std::vector<PropItem*> *items, PropSearch search, PropItem* mItem) {
	if(search.mMax != 0 && items->size() >= search.mMax)
		return;
	if(Util::CaseInsensitiveStringFind(mItem->mName, search.mSearch) ||
	   ( mItem->mDescription.length() > 0 && Util::CaseInsensitiveStringFind(mItem->mDescription, search.mSearch)) ||
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
PropSearch::PropSearch() {
	mPropTypeID = -1;
	mSearch = "";
	mMax = 99;
}

PropSearch::~PropSearch() {
}
