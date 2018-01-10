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
#include "ActiveCharacter.h"
#include "Item.h"
#include "Stats.h"
#include "Scheduler.h"
#include "http/SiteClient.h"
#include "Entities.h"

static std::string KEYPREFIX_AUCTION_ITEM = "AuctionItem";
static std::string ID_NEXT_AUCTION_ITEM_ID = "NextAuctionItemID";
static std::string LISTPREFIX_AUCTION_ITEMS = "AuctionItems";

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
	short mLevelStart;
	short mLevelEnd;

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

class AuctionHouseItemState {
public:
	AuctionHouseItemState();
	~AuctionHouseItemState();

	// Transient
	ItemDef *itemDef;
	std::string mSellerName;
	int timerTaskID;
};

class AuctionHouseItem: public AbstractEntity {
public:
	AuctionHouseItem();
	~AuctionHouseItem();

	int mId;
	int mAuctioneer;
	std::vector<std::string> mTags;
	std::vector<AuctionHouseBid> mBids;
	int mSeller;
	unsigned long mStartDate;
	unsigned long mEndDate;
	unsigned long mReserveCopper;
	unsigned long mReserveCredits;
	unsigned long mBuyItNowCopper;
	unsigned long mBuyItNowCredits;
	unsigned long mSecondsRemaining;
	int mItemId;
	int mLookId;
	int mCount;
	bool mCompleted;

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	unsigned long GetSecondsRemaining();
	bool IsExpired();
	bool Bid(int creatureDefID, unsigned long copper, unsigned long credits);
	void WriteToJSON(Json::Value &value);
};

class AuctionHouseManager
{
public:
	Platform_CriticalSection cs;
	HTTPD::SiteSession *session;

	AuctionHouseManager();
	~AuctionHouseManager();
	AuctionHouseItem Auction(CreatureInstance *creatureInstance, CharacterServerData *pld, InventorySlot &slot, unsigned long copperCommision, unsigned long creditsCommision, int auctioneer, unsigned long reserveCopper, unsigned long reserveCredits, unsigned long buyItNowCopper, unsigned long buyItNowCredits,
			int days, int hours);
	AuctionHouseItem LoadItem(int id);
	int LoadItems(void);
	void BroadcastUpdate(int auctioneerCID, AuctionHouseItem &item);
	void BroadcastRemovedItem(int auctioneerCID, int auctionItemID);
	void BroadcastNewItem(int auctionItemID, const std::string &sellerName);
	void CancelAllTimers(void);
	int ValidateItem(AuctionHouseItem *item, AccountData *accPtr, CharacterStatSet *css, CharacterData *cd);
	bool RemoveItem(int id);
	void BroadcastAndSetupTimer(AuctionHouseItem *ahItem, const std::string &sellerName);
	bool SaveItem(AuctionHouseItem item);
	void Search(AuctionHouseSearch &search, std::vector<AuctionHouseItem> &results);
	void ConnectToSite();
	void CompleteAuction(int auctionItemID);
private:
	std::map<int, int> mTimers;
	void ExpireItem(AuctionHouseItem item);
	void CancelItemTimer(int auctionItemID);
};

int WriteAuctionItem(char *buffer,
		AuctionHouseItem *item, std::string sellerName);

extern AuctionHouseManager g_AuctionHouseManager;

#endif
