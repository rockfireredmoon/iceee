#pragma once
#ifndef CREDITSHOP_H
#define CREDITSHOP_H

#include <map>
#include <list>
#include <string>
#include <stdarg.h>
#include "Components.h"
#include "CommonTypes.h"
#include "Account.h"
#include "Character.h"
#include "Item.h"
#include "Stats.h"
#include "json/json.h"

namespace Category
{
	enum
	{
		UNDEFINED = -1,
		CONSUMABLES = 0,
		CHARMS = 1,
		ARMOR = 2,
		BAGS = 3,
		RECIPES = 4,
		PETS = 5,
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
}

namespace Status
{
	enum
	{
		UNDEFINED = -1,
		HOT = 0,
		NEW = 1,
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
}

namespace Currency
{
	enum
	{
		UNDEFINED = -1,
		COPPER = 0,
		CREDITS = 1,
		COPPER_CREDITS = 2,
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
}

namespace CreditShopError
{
	enum
	{
		NONE = 0,
		SOLD_OUT = 1,
		NOT_ENOUGH_COPPER = 2,
		NOT_ENOUGH_CREDITS = 3,
		NOT_YET_AVAILABLE = 4,
		NO_LONGER_AVAILABLE = 5,
		NOT_ENOUGH_FREE_SLOTS = 7,
		SERVER_ERROR = 8
	};
	std::string GetDescription(int eventID);
}

namespace CS
{

class CreditShopItem
{
public:
	CreditShopItem();
	~CreditShopItem();

	int mId;
	std::string mTitle;            //Internal name of the script.
	std::string mDescription;
	int mStatus;
	int mCategory;
	time_t mStartDate;
	time_t mEndDate;
	time_t mCreatedDate;
	unsigned long mPriceCopper;
	unsigned long mPriceCredits;
	int mPriceCurrency;
	int mQuantityLimit;
	int mQuantitySold;
	int mItemId;
	int mLookId;
	int mIv1;
	int mIv2;

	void ParseItemProto(std::string proto);
	void WriteToJSON(Json::Value &value);

//	this.mItemProtoEntry.setText("item" + defId + ":" + (lookId != defId ? lookId : 0) + ":" + itemID.mItemData.mIv1 + ":" + itemID.mItemData.mIv2);

};

class CreditShopManager
{
public:
	int nextMarketItemID;
	Platform_CriticalSection cs;

	CreditShopManager();
	~CreditShopManager();
	std::map<int, CreditShopItem*> mItems;
	CreditShopItem* LoadItem(int id);
	int LoadItems(void);
	CreditShopItem* GetItem(int id);
	int ValidateItem(CreditShopItem *item, AccountData *accPtr, CharacterStatSet *css, CharacterData *cd);
	bool RemoveItem(int id);
	std::string GetPath(int id);
	bool SaveItem(CreditShopItem * item);

};

} //namespace CS


extern CS::CreditShopManager g_CreditShopManager;

#endif //#ifndef CREDITSHOP_H
