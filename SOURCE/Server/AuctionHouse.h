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
#ifndef AUCTIONHOUSE_H
#define AUCTIONHOUSE_H

#include <vector>
#include "json/json.h"
#include "Components.h"
#include "Account.h"
#include "Character.h"
#include "Item.h"
#include "Stats.h"
#include "Timer.h"
#include "http/SiteClient.h"


namespace AuctionHouseError
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


namespace AuctionHouseSearchOrder
{
	enum
	{
		REMAINING = 0,
		END_TIME = 1,
		LEVEL = 2,
		QUALITY = 3,
		TYPE = 4,
		WEAPON_TYPE = 5,
		BUY_COPPER = 6,
		BUY_CREDITS = 7,
		BID_COPPER = 8,
		BID_CREDITS= 9
	};
	std::string GetDescription(int eventID);
}

class AuctionHouseSearch
{
public:
	AuctionHouseSearch();
	~AuctionHouseSearch();
	int mAuctioneer;
	int mItemTypeId;
	int mQualityId;
	int mOrder;
	int mMaxRows;
	bool mReverse;
	unsigned long mBuyPriceCopperStart;
	unsigned long mBuyPriceCopperEnd;
	unsigned long mBuyPriceCreditsStart;
	unsigned long mBuyPriceCreditsEnd;
	unsigned long mLevelStart;
	unsigned long mLevelEnd;

	std::string mSearch;
};

class AuctionHouseBid
{
public:
	AuctionHouseBid();
	~AuctionHouseBid();

	int mBuyer;
	unsigned long mCopper;
	unsigned long mCredits;
	time_t mBidTime;
	void WriteToJSON(Json::Value &value);
};

class AuctionHouseItem
{
public:
	AuctionHouseItem();
	~AuctionHouseItem();

	int mId;
	int mAuctioneer;
	std::vector<std::string> mTags;
	std::vector<AuctionHouseBid> mBids;
	int mSeller;
	time_t mStartDate;
	time_t mEndDate;
	unsigned long mReserveCopper;
	unsigned long mReserveCredits;
	unsigned long mBuyItNowCopper;
	unsigned long mBuyItNowCredits;
	unsigned long mSecondsRemaining;
	int mItemId;
	int mLookId;
	int mCount;
	bool mCompleted;

	// Transient
	ItemDef *itemDef;
	std::string mSellerName;
	TimerTask *timerTask;

	unsigned long GetTimeRemaining();
	bool IsExpired();
	bool Bid(int creatureDefID, unsigned long copper, unsigned long credits);
	void WriteToJSON(Json::Value &value);
};

class AuctionHouseManager
{
public:
	int nextAuctionHouseItemID;
	Platform_CriticalSection cs;
	HTTPD::SiteSession *session;

	AuctionHouseManager();
	~AuctionHouseManager();
	std::map<int, AuctionHouseItem*> mItems;
	AuctionHouseItem* LoadItem(int id);
	int LoadItems(void);
	int ValidateItem(AuctionHouseItem *item, AccountData *accPtr, CharacterStatSet *css, CharacterData *cd);
	AuctionHouseItem* GetItem(int id);
	bool RemoveItem(int id);
	std::string GetPath(int id);
	bool SaveItem(AuctionHouseItem * item);
	void Search(AuctionHouseSearch &search, std::vector<AuctionHouseItem*> &results);
	void ConnectToSite();
};


class AuctionTimerTask: public TimerTask {
public:
	AuctionTimerTask(AuctionHouseItem *mItem);
	AuctionHouseItem *mItem;
	void run();
};


class AuctionRemoveTimerTask: public TimerTask {
public:
	AuctionRemoveTimerTask(AuctionHouseItem *mItem);
	AuctionHouseItem *mItem;
	void run();
};

extern AuctionHouseManager g_AuctionHouseManager;

#endif
