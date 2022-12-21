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
#include <filesystem>

using namespace std;
namespace fs = filesystem;

static string KEYPREFIX_CUSTOM_PROP_ITEM = "CustomPropItem";
static string LISTPREFIX_CUSTOM_PROP_ITEMS = "CustomPropItems";

namespace AssetCatalogueItemType
{
	enum
	{
		OTHER = 0,
		PROP = 1,
		BUILDING = 2,
		CAVE = 3,
		DUNGEON = 4,
		CATEGORY = 5,
		SKIN = 6,
		VARIANT = 7
	};
	string GetDescription(int propTypeID);
}

class AssetCatalogueSearch
{
public:
	AssetCatalogueSearch();
	~AssetCatalogueSearch();
	int mPropTypeID;
	string mSearch;
	unsigned int mMax;
};

class AssetCatalogueItem {
public:
	AssetCatalogueItem();
	~AssetCatalogueItem();

	string GetDisplayName();
	string GetAsset();

	string mName;
	string mDisplayName;
	string mDescription;
	string mAsset;
	int mOrder;
	int mType;
	STRINGLIST mKeywords;
	vector<AssetCatalogueItem*> mParents;
	vector<AssetCatalogueItem*> mChildren;
};

class AssetCatelogueManager
{
public:
	Platform_CriticalSection cs;

	AssetCatelogueManager();
	~AssetCatelogueManager();
	int LoadFromDirectory(const fs::path &directory);
	void Search(AssetCatalogueSearch search, vector<AssetCatalogueItem*> *results);
	unsigned int Count();
	bool Contains(string name);
	AssetCatalogueItem* GetItem(string name);
	AssetCatalogueItem* GetByID(string id);
	vector<AssetCatalogueItem*> GetChildren(string name, CharacterServerData *pld);
private:
	map<string, AssetCatalogueItem*> mItems;
	AssetCatalogueItem* mFavourites;
	void SearchProps(vector<AssetCatalogueItem*> *items, AssetCatalogueSearch search, AssetCatalogueItem* mItem);
};

int WriteAssetCatalogueItem(char *buffer, AssetCatalogueItem *item);

extern AssetCatelogueManager g_AssetCatalogueManager;

#endif
