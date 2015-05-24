#pragma once
#ifndef CREDITSHOP_H
#define CREDITSHOP_H

#include <map>
#include <list>
#include <string>
#include <stdarg.h>
#include "CommonTypes.h"


namespace Category
{
	enum
	{
		UNDEFINED = -1,
		CONSUMABLES = 0,
		CHARMS = 1,
		ARMOR = 2,
		BAGS = 3,
		RECIPE = 4,
		NEW = 5,
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
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const std::string &eventName);
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
	unsigned long mPriceAmount;
	int mPriceCurrency;
	int mQuantityLimit;
	int mQuantitySold;
	int mItemId;
	int mItemAmount;
	
};

class CreditShopManager
{
public:
	CreditShopManager();
	~CreditShopManager();
	std::map<int, CreditShopItem*> mItems;
	CreditShopItem* LoadItem(int id);
	int LoadItems(void);
	CreditShopItem* GetItem(int id);
};

} //namespace CS


extern CS::CreditShopManager g_CSManager;

#endif //#ifndef CREDITSHOP_H
