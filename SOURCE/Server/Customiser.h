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


#pragma once
#ifndef CUSTOMISER_H
#define CUSTOMISER_H

#include <vector>
#include "json/json.h"
#include "Components.h"
#include "Account.h"
#include "Character.h"
#include "ActiveCharacter.h"
#include "Item.h"
#include "Stats.h"
#include "Scheduler.h"
#include "http/SiteClient.h"
#include "Entities.h"

static std::string KEYPREFIX_CUSTOM_PROP_ITEM = "CustomPropItem";
static std::string LISTPREFIX_CUSTOM_PROP_ITEMS = "CustomPropItems";

namespace PropType
{
	enum
	{
		OTHER = 0,
		PROP = 1,
		BUILDING = 2,
		CAVE = 3,
		DUNGEON = 4,
		CATEGORY = 5
	};
	std::string GetDescription(int propTypeID);
}

class PropSearch
{
public:
	PropSearch();
	~PropSearch();
	int mPropTypeID;
	std::string mSearch;
	unsigned int mMax;
};

class PropItem {
public:
	PropItem();
	~PropItem();

	std::string mName;
	std::string mDescription;
	std::string mID;
	int mPropTypeID;
	STRINGLIST mKeywords;
	PropItem *mParent;
	std::vector<PropItem*> mChildren;
};

class PropManager
{
public:
	Platform_CriticalSection cs;

	PropManager();
	~PropManager();
	int LoadFromFile(std::string fileName);
	void Search(PropSearch search, std::vector<PropItem*> *results);
	unsigned int Count();
	bool Contains(std::string name);
	PropItem* GetItem(std::string name);
	PropItem* GetByID(std::string id);
	std::vector<PropItem*> GetChildren(std::string name, CharacterServerData *pld);
private:
	std::map<std::string, PropItem*> mItems;
	PropItem* mFavourites;
	void SearchProps(std::vector<PropItem*> *items, PropSearch search, PropItem* mItem);
};

int WritePropItem(char *buffer, PropItem *item);

extern PropManager g_PropManager;

#endif
