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

#include "AuctionHouse.h"
#include "Config.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "ByteBuffer.h"

#include "Simulator.h"
#include "Item.h"
#include "Instance.h"
#include "Chat.h"
#include "Cluster.h"
#include "StringUtil.h"
#include <string.h>
#include <algorithm>

AuctionHouseManager g_AuctionHouseManager;

static inline std::string FormatAuctionItemProto(AuctionHouseItem *item) {
	char buf[32];
	Util::SafeFormat(buf, sizeof(buf), "item%d:%d:%d:%d", item->mItemId,
			item->mLookId, item->mCount, 0);
	return buf;
}

int WriteAuctionItem(char *buffer,
		AuctionHouseItem *item, std::string sellerName) {
	int wpos = 0;
	// 1
	wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%d", item->mId).c_str());
	// 2
	wpos += PutStringUTF(&buffer[wpos], FormatAuctionItemProto(item).c_str());
	// 3
	wpos += PutStringUTF(&buffer[wpos], sellerName.c_str());
	// 4
	wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%lu", item->GetSecondsRemaining()).c_str());
	// 5
	wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%lu", item->mBuyItNowCopper).c_str());
	// 6
	wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%lu", item->mBuyItNowCredits).c_str());
	// 7
	wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%d", item->mBids.size()).c_str());
	if (item->mBids.size() > 0) {
		AuctionHouseBid highBid = item->mBids[item->mBids.size() - 1];
		// 8
		wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%lu", highBid.mCopper).c_str());
		// 9
		wpos += PutStringUTF(&buffer[wpos], StringUtil::Format("%lu", highBid.mCredits).c_str());
	} else {
		// 8
		wpos += PutStringUTF(&buffer[wpos], "0");
		// 9
		wpos += PutStringUTF(&buffer[wpos], "0");
	}
	return wpos;
}

namespace AuctionHouseError {
std::string GetDescription(int id) {
	switch (id) {
	case NONE:
		return "No error";
	case SOLD_OUT:
		return "Sold out!";
	case NOT_ENOUGH_COPPER:
		return "You do not have enough copper!";
	case NOT_ENOUGH_CREDITS:
		return "You do not have enough credits!";
	case NOT_YET_AVAILABLE:
		return "This item is not yet available!";
	case NO_LONGER_AVAILABLE:
		return "This item is no longer available!";
	case NOT_ENOUGH_FREE_SLOTS:
		return "You do not have enough slots in your inventory!";
	case SERVER_ERROR:
		return "Internal error.";
	}
	return "<undefined>";
}
}

//
// AuctionHouseBid
//

AuctionHouseBid::AuctionHouseBid() {
	mBidTime = 0;
	mBuyer = 0;
	mCopper = 0;
	mCredits = 0;
}

AuctionHouseBid::~AuctionHouseBid() {
}

void AuctionHouseBid::WriteToJSON(Json::Value &value) {
	value["buyer"] = mBuyer;
	value["copper"] = Json::UInt64(mCopper);
	value["credits"] = Json::UInt64(mCredits);
	value["time"] = Json::UInt64(mBidTime);
}

//
// AuctionHouseItemState
//

AuctionHouseItemState::AuctionHouseItemState() {
	itemDef = NULL;
	mSellerName = "";
	timerTaskID = 0;
}

AuctionHouseItemState::~AuctionHouseItemState() {
}

//
// AuctionHouseItem
//

AuctionHouseItem::AuctionHouseItem() {

	mId = 0;
	mStartDate = 0;
	mEndDate = 0;
	mItemId = 0;
	mLookId = 0;
	mCount = 0;
	mReserveCopper = 0;
	mReserveCredits = 0;
	mBuyItNowCopper = 0;
	mBuyItNowCredits = 0;
	mSeller = 0;
	mAuctioneer = 0;
	mCompleted = false;
	mSecondsRemaining = 0;
}

AuctionHouseItem::~AuctionHouseItem() {
}

bool AuctionHouseItem :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_AUCTION_ITEM, StringUtil::Format("%d", mId));
	writer->Value("BeginDate", mStartDate);
	writer->Value("EndDate", mEndDate);
	writer->Value("Seller", mSeller);
	writer->Value("Auctioneer", mAuctioneer);
	writer->Value("ReserveCopper", mReserveCopper);
	writer->Value("ReserveCredits", mReserveCredits);
	writer->Value("BuyItNowCopper", mBuyItNowCopper);
	writer->Value("BuyItNowCredits", mBuyItNowCredits);
	writer->Value("SecondsRemaining", mSecondsRemaining);
	writer->Value("Completed", mCompleted);
	writer->Value("ItemID", mItemId);
	writer->Value("Count", mCount);
	writer->Value("LookId", mLookId);
	STRINGLIST l;
	for (auto it = mBids.begin(); it != mBids.end(); ++it) {
		l.push_back(StringUtil::Format("%d,%lu,%lu,%lld", it->mBuyer, it->mCopper,
				it->mCredits, (long long) it->mBidTime));
	}
	writer->ListValue("Bid", l);
	return true;
}

bool AuctionHouseItem :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_AUCTION_ITEM, StringUtil::Format("%d", mId), true);
	return true;
}

bool AuctionHouseItem :: ReadEntity(AbstractEntityReader *reader) {
	mStartDate = reader->ValueULong("BeginDate");
	mEndDate = reader->ValueULong("EndDate");
	mSeller = reader->ValueInt("Seller");
	mAuctioneer = reader->ValueInt("Auctioneer");
	mReserveCopper  = reader->ValueULong("ReserveCopper");
	mReserveCredits = reader->ValueULong("ReserveCredits");
	mBuyItNowCopper = reader->ValueULong("BuyItNowCopper");
	mBuyItNowCredits = reader->ValueULong("BuyItNowCredits");
	mSecondsRemaining = reader->ValueULong("SectionsRemaining");
	mCompleted = reader->ValueBool("Completed");
	mItemId = reader->ValueInt("ItemID");
	mCount = reader->ValueInt("Count");
	mLookId = reader->ValueInt("LookID");
	STRINGLIST l = reader->ListValue("Bid");
	for(auto it = l.begin(); it != l.end(); ++it) {
		STRINGLIST a;
		Util::Split(*it, ",", a);
		if(a.size() > 4) {
			AuctionHouseBid bid;
			bid.mBuyer = atoi(l[0].c_str());
			bid.mCopper = strtoul(l[1].c_str(), NULL, 0);
			bid.mCredits = strtoul(l[2].c_str(), NULL, 0);
			bid.mBidTime = strtoul(l[3].c_str(), NULL, 0);
			mBids.push_back(bid);
		}
	}
	return true;
}

bool AuctionHouseItem::IsExpired() {
	return g_ServerTime >= mEndDate;
}

unsigned long AuctionHouseItem::GetSecondsRemaining() {
	long timeRemaining = mEndDate - g_ServerTime;
	if (timeRemaining < 0)
		timeRemaining = 0;
	return timeRemaining / 1000;

}
bool AuctionHouseItem::Bid(int creatureDefID, unsigned long copper,
		unsigned long credits) {
	if (mBids.size() == 0
			|| (copper > mBids[mBids.size() - 1].mCopper
					|| credits > mBids[mBids.size() - 1].mCredits)) {
		if(mBids.size() != 0 && mBids[mBids.size() - 1].mBuyer == creatureDefID)
			/* Silly beast ... bidding against your own already accepted bid! */
			return false;


		for (std::vector<AuctionHouseBid>::iterator it = mBids.begin();
				it != mBids.end(); ++it) {
			if (it->mBuyer == creatureDefID) {
				mBids.erase(it);
				break;
			}
		}

		AuctionHouseBid bid;
		bid.mBuyer = creatureDefID;
		bid.mCopper = copper;
		bid.mCredits = credits;
		bid.mBidTime = time(NULL);
		mBids.push_back(bid);
		return true;
	}
	return false;
}

void AuctionHouseItem::WriteToJSON(Json::Value &value) {
	value["id"] = mId;
	value["begin"] = Json::UInt64(mStartDate);
	value["end"] = Json::UInt64(mEndDate);
	value["remain"] = Json::Int64(mEndDate - time(NULL));
	value["seller"] = mSeller;
	value["auctioneer"] = mAuctioneer;
	value["reserveCopper"] = Json::UInt64(mReserveCopper);
	value["reserveCredits"] = Json::UInt64(mReserveCredits);
	value["buyItNowCopper"] = Json::UInt64(mBuyItNowCopper);
	value["buyItNowCredits"] = Json::UInt64(mBuyItNowCredits);
	value["itemID"] = mItemId;
	value["lookID"] = mLookId;
	value["count"] = mCount;

	Json::Value bids(Json::arrayValue);

	for (std::vector<AuctionHouseBid>::iterator it = mBids.begin();
			it != mBids.end(); ++it) {
		Json::Value v;
		it->WriteToJSON(v);
		bids.append(v);
	}
	value["bids"] = bids;
}

//
// AuctionHouseManager
//

AuctionHouseManager::AuctionHouseManager() {
	session = NULL;
}

AuctionHouseManager::~AuctionHouseManager() {
}

int AuctionHouseManager::ValidateItem(AuctionHouseItem *ahItem,
		AccountData *accPtr, CharacterStatSet *css, CharacterData *cd) {

	if ((unsigned long) css->copper < ahItem->mBuyItNowCopper)
		return AuctionHouseError::NOT_ENOUGH_COPPER;

	if (g_Config.AccountCredits) {
		css->credits = accPtr->Credits;
	}
	if ((unsigned long) css->credits < ahItem->mBuyItNowCredits)
		return AuctionHouseError::NOT_ENOUGH_CREDITS;

	if (ahItem->mStartDate != 0 && g_ServerTime < (ahItem->mStartDate))
		return AuctionHouseError::NOT_YET_AVAILABLE;

	if (ahItem->mEndDate != 0 && g_ServerTime >= (ahItem->mEndDate))
		return AuctionHouseError::NO_LONGER_AVAILABLE;

	int slot = cd->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return AuctionHouseError::NOT_ENOUGH_FREE_SLOTS;

	return AuctionHouseError::NONE;
}

void AuctionHouseManager::CancelItemTimer(int auctionItemID) {
	auto it = mTimers.find(auctionItemID);
	if(it != mTimers.end()) {
		g_Scheduler.Cancel(it->second);
		mTimers.erase(it);
	}
}

void AuctionHouseManager::CompleteAuction(int auctionItemID) {
	AuctionHouseItem item = LoadItem(auctionItemID);
	if(item.mId == 0) {
		g_Logs.server->error("Request to complete auction ID %v that does not exist", auctionItemID);
		return;
	}

	char buf[1024];
	char buf2[256];

	// Any bids at all?
	bool success = false;
	bool reserveReached = true;

	cs.Enter("AuctionTimerTask::run");

	// Get seller objects

	CreatureInstance *sellerInstance = g_ActiveInstanceManager.GetPlayerCreatureByDefID(item.mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if (sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(item.mSeller, true);
		if (sellerCharacterData == NULL) {
			g_Logs.server->warn("Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
		} else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(
					sellerCharacterData->AccountID);
			if (sellerAccount == NULL) {
				g_Logs.server->warn(
						"Seller account of auction item, no longer exists. Received copper and credits go to /dev/null!");
			}
		}
	} else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(
				sellerCharacterData->AccountID);
	}

	// Auctioneer
	CreatureDefinition *auctioneer = CreatureDef.GetPointerByCDef(item.mAuctioneer);

	// Get the item
	ItemDef *itemDef = g_ItemManager.GetSafePointerByID(item.mItemId);
	if(itemDef != NULL) {

		// Find the best bid and process it
		for (std::vector<AuctionHouseBid>::reverse_iterator it =
				item.mBids.rbegin(); it != item.mBids.rend(); ++it) {

			// Was reserve not reached
			if ((item.mReserveCopper != 0 && it->mCopper < item.mReserveCopper)
					|| (item.mReserveCredits != 0
							&& it->mCredits < item.mReserveCredits)) {
				success = true;
				break;
			} else
				reserveReached = false;

			// Look for an instance of the bidders character stat set or the offline players character stat set
			CreatureInstance *creatureInstance =
					g_ActiveInstanceManager.GetPlayerCreatureByDefID(it->mBuyer);
			CharacterStatSet *css;
			AccountData *account;
			CharacterData *characterData = NULL;
			if (creatureInstance == NULL) {
				characterData = g_CharacterManager.RequestCharacter(it->mBuyer,
						true);
				if (characterData == NULL) {
					// Creature now deleted?
					continue;
				}
				css = &characterData->cdef.css;
				account = g_AccountManager.FetchIndividualAccount(
						characterData->AccountID);
				if (account == NULL)
					// Account now deleted? (shouldn't happen)
					continue;
			} else {
				characterData = creatureInstance->charPtr;
				css = &creatureInstance->css;
				account = g_AccountManager.GetActiveAccountByID(
						characterData->AccountID);
			}

			InventoryManager *inventory = &characterData->inventory;

			// Check the prospective recipient has enough copper/credits/inventory space
			if (g_Config.AccountCredits) {
				css->credits = account->Credits;
			}

			// Check the prospective recipient inventory space
			int slot = inventory->GetFreeSlot(INV_CONTAINER);
			if (slot == -1) {
				Util::SafeFormat(buf, sizeof(buf),
						"Your bid on '%s' failed because you do not have enough inventory space. Item goes to next bidder or back to seller.",
						itemDef->mDisplayName.c_str());
				ChatMessage cm(buf);
				cm.mChannelName = "tc/";
				cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
				cm.mSender = "EEBay";
				cm.mTell = true;
				cm.mRecipient = css->display_name;
				if (!g_ChatManager.SendChatMessage(cm, NULL)) {
					g_ChatManager.SendChatMessageAsOffline(cm,
							session);
				}
				continue;
			}

			// Check the prospective recipient has enough copper
			if (it->mCopper > 0 && it->mCopper > (unsigned long)css->copper) {

				Util::SafeFormat(buf, sizeof(buf),
						"Your bid on '%s' failed because you do not have enough copper. Item goes to next bidder or back to seller.",
						itemDef->mDisplayName.c_str());
				ChatMessage cm(buf);
				cm.mChannelName = "tc/";
				cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
				cm.mSender =
						auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
				cm.mTell = true;
				if (!g_ChatManager.SendChatMessage(cm, NULL)) {
					g_ChatManager.SendChatMessageAsOffline(cm,
							session);
				}
				continue;
			}

			// Check the prospective recipient has enough credits
			if (it->mCredits > 0 && it->mCredits > css->credits) {
				Util::SafeFormat(buf, sizeof(buf),
						"Your bid on '%s' failed because you do not have enough credits. Item goes to next bidder or back to seller.",
						itemDef->mDisplayName.c_str());
				ChatMessage cm(buf);
				cm.mChannelName = "tc/";
				cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
				cm.mSender =
						auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
				cm.mTell = true;
				cm.mRecipient = css->display_name;
				if (!g_ChatManager.SendChatMessage(cm, NULL)) {
					g_ChatManager.SendChatMessageAsOffline(cm,
							session);
				}
				continue;
			}

			// Nearly there! Now try to add item to recipients inventory
			InventorySlot *sendSlot = inventory->AddItem_Ex(INV_CONTAINER,
					item.mItemId, item.mCount + 1);
			if (sendSlot == NULL) {
				int err = creatureInstance->charPtr->inventory.LastError;
				std::string errText = "Server error: undefined error.";
				if (err == InventoryManager::ERROR_ITEM)
					errText = "Server error: item does not exist.";
				else if (err == InventoryManager::ERROR_SPACE)
					errText = "You do not have any free inventory space.";
				else if (err == InventoryManager::ERROR_LIMIT)
					errText = "You already the maximum amount of these items.";
			} else {
				creatureInstance->charPtr->pendingChanges++;
				sendSlot->secondsRemaining = item.mSecondsRemaining;
				sendSlot->customLook = item.mLookId;

				// Update recipients inventory
				if (creatureInstance != NULL
						&& creatureInstance->simulatorPtr != NULL) {
					int wpos = AddItemUpdate(buf, buf2, sendSlot);
					creatureInstance->simulatorPtr->AttemptSend(buf, wpos);
				}

				// Take copper from recipient and give to seller
				if (it->mCopper > 0) {
					css->copper -= it->mCopper;
					characterData->pendingChanges++;
					if (creatureInstance != NULL) {
						creatureInstance->SendStatUpdate(STAT::COPPER);
					}

					if (sellerCss != NULL) {
						sellerCss->copper += it->mCopper;
						if (sellerCharacterData != NULL) {
							sellerCharacterData->pendingChanges++;
						}
						if (sellerInstance != NULL) {
							sellerInstance->SendStatUpdate(STAT::COPPER);
						}
					}
				}

				// Take credits from recipient and give to seller
				if (it->mCredits > 0) {
					css->credits -= it->mCredits;
					if (g_Config.AccountCredits)
						account->PendingMinorUpdates++;
					else
						characterData->pendingChanges++;
					if (creatureInstance != NULL) {
						creatureInstance->SendStatUpdate(STAT::CREDITS);
					}

					if (sellerCss != NULL) {
						sellerCss->credits += it->mCredits;
						if (g_Config.AccountCredits && sellerAccount != NULL)
							sellerAccount->PendingMinorUpdates++;
						else if (sellerCharacterData != NULL) {
							sellerCharacterData->pendingChanges++;
						}
						if (sellerInstance != NULL) {
							sellerInstance->SendStatUpdate(STAT::CREDITS);
						}
					}
				}

				Util::SafeFormat(buf, sizeof(buf),
						"%s is the winning bidder of '%s'!", css->display_name,
						itemDef->mDisplayName.c_str());
				ChatMessage cm(buf);
				cm.mChannelName = "tc/";
				cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
				cm.mSender =
						auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
				if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				}

				success = true;
				break;
			}
		}

	}
	else {
		g_Logs.server->warn("Auction ended for item %v but the item doesn't exist.", item.mItemId);
		CancelItemTimer(item.mId);
		mTimers[item.mId] = g_Scheduler.Submit([this, item](){
			ExpireItem(item);
		});
		cs.Leave();
		return;
	}

	if (!success) {

		Util::SafeFormat(buf, sizeof(buf),
				"Your auction item '%s' failed to attract any bids. You have %d hours to remove it from the auction before it is automatically removed and placed in your vendor buyback inventory.",
				itemDef->mDisplayName.c_str(),
				g_Config.MaxAuctionExpiredHours);
		ChatMessage cm(buf);
		cm.mChannelName = "tc/";
		cm.mRecipient = sellerCss->display_name;
		cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
		cm.mSender =
				auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
		cm.mTell = true;
		if (!g_ChatManager.SendChatMessage(cm, NULL)) {
			g_ChatManager.SendChatMessageAsOffline(cm,
					session);
		}
		item.mCompleted = true;
		SaveItem(item);
		CancelItemTimer(item.mId);
		mTimers[item.mId] = g_Scheduler.Schedule([this, item](){
			ExpireItem(item);
		}, item.mEndDate + (g_Config.MaxAuctionExpiredHours * 3600000));
		cs.Leave();
	} else {
		if (!reserveReached) {
			Util::SafeFormat(buf, sizeof(buf),
					"Your auction item '%s' failed to reach the reserve price. You have %d hours to remove it from the auction before it is automatically removed and placed in your vendor buyback inventory.",
					itemDef->mDisplayName.c_str(),
					g_Config.MaxAuctionExpiredHours);
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mRecipient = sellerCss->display_name;
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			cm.mTell = true;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						session);
			}
			item.mCompleted = true;
			SaveItem(item);

			CancelItemTimer(item.mId);
			mTimers[item.mId] = g_Scheduler.Schedule([this, item](){
				ExpireItem(item);
			}, item.mEndDate + (g_Config.MaxAuctionExpiredHours * 3600000));

			cs.Leave();
		} else {
			CancelItemTimer(item.mId);

			RemoveItem(item.mId);
			cs.Leave();

			/* Find the active auctioneer instance. If there isn't one, then nobody can be standing at an auction house,
			 * so there is no need to broadcast the change
			 */
			CreatureInstance *instance =
					g_ActiveInstanceManager.GetNPCCreatureByDefID(item.mAuctioneer);
			if (instance != NULL) {
				BroadcastRemovedItem(instance->CreatureID, item.mId);
			}
		}
	}
}

void AuctionHouseManager::ExpireItem(AuctionHouseItem item) {
	char buf[1024];
	char buf2[256];

	g_Logs.event->info(
			"Removing auction house item '%v' (sold by %v) from auction list",
			item.mItemId, item.mSeller);

	cs.Enter("AuctionTimerTask::run");

	// Get seller objects

	CreatureInstance *sellerInstance =
			g_ActiveInstanceManager.GetPlayerCreatureByDefID(item.mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if (sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(item.mSeller, true);
		if (sellerCharacterData == NULL) {
			g_Logs.server->warn("Seller of auction item, no longer exists.");
		} else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(
					sellerCharacterData->AccountID);
			if (sellerAccount == NULL) {
				g_Logs.server->warn("Seller account of auction item, no longer exists.");
			}
		}
	} else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(
				sellerCharacterData->AccountID);
	}

	// Auctioneer
	CreatureInstance *auctioneerInstance =
			g_ActiveInstanceManager.GetNPCCreatureByDefID(item.mAuctioneer);
	if(auctioneerInstance == NULL) {
		g_Logs.server->warn("No auctioneer instance for ID %v", item.mAuctioneer);
	}
	CreatureDefinition *auctioneer = auctioneerInstance == NULL ? CreatureDef.GetPointerByCDef(
			item.mAuctioneer) : &auctioneerInstance->charPtr->cdef;

	if(auctioneer == NULL) {
		g_Logs.server->warn("No auctioneer object for ID %v", item.mAuctioneer);
	}

	ItemDef *itemDef = g_ItemManager.GetSafePointerByID(item.mItemId);
	if(itemDef == NULL) {
		g_Logs.server->warn("Auction removal for item %v but the item doesn't exist.", item.mItemId);
	}
	else {
		if (sellerCss != NULL) {
			Util::SafeFormat(buf, sizeof(buf),
					"Your expired auction item '%s' has been in the auction house too long and has been removed. You may retrieve it from your buyback.",
					itemDef->mDisplayName.c_str());
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mRecipient = sellerCss->display_name;
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			cm.mTell = true;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						session);
			}

			if (sellerCharacterData != NULL) {
				int wpos = 0;
				InventorySlot tempItem;
				tempItem.IID = item.mItemId;
				tempItem.dataPtr = itemDef;
				tempItem.count = item.mCount;
				tempItem.secondsRemaining = item.mSecondsRemaining;
				tempItem.customLook = item.mLookId;
				wpos += sellerCharacterData->inventory.AddBuyBack(&tempItem,
						&buf[wpos]);
				sellerCharacterData->pendingChanges++;
				if (sellerInstance != NULL && sellerInstance->simulatorPtr != NULL) {
					sellerInstance->simulatorPtr->AttemptSend(buf, wpos);
				}
			}
		}

	}

	CancelItemTimer(item.mId);

	if (auctioneerInstance != NULL) {
		RemoveItem(item.mId);
		BroadcastRemovedItem(auctioneerInstance->CreatureID, item.mId);
		cs.Leave();
	} else {
		RemoveItem(item.mId);
		cs.Leave();
	}
}

bool AuctionHouseManager::SaveItem(AuctionHouseItem item) {
	return g_ClusterManager.WriteEntity(&item);
}

AuctionHouseItem AuctionHouseManager::LoadItem(int id) {
	AuctionHouseItem item;
	item.mId = id;
	if(!g_ClusterManager.ReadEntity(&item)) {
		g_Logs.data->error("Could not load auction house item [%v] from the cluster", id);
		return item;
	}
	return item;
}

bool AuctionHouseManager::RemoveItem(int id) {
	cs.Enter("AuctionHouseManager::RemoveItem");

	AuctionHouseItem ah = LoadItem(id);
	if(ah.mId == 0) {
		g_Logs.data->error("Failed to load auction house item [%v] from cluster for removal", id);
	}
	else {
		CancelItemTimer(ah.mId);
		if(!g_ClusterManager.RemoveEntity(&ah)) {
			g_Logs.data->error("Failed to remove auction house item [%v] from cluster", ah.mId);
		}
		else {
			g_ClusterManager.ListRemove(LISTPREFIX_AUCTION_ITEMS, StringUtil::Format("%d", id));
			g_ClusterManager.AuctionItemRemoved(ah.mId, ah.mAuctioneer);
		}
	}
	cs.Leave();

	g_Logs.event->info("Auction house item %v removed", id);
	return true;
}

bool remainingSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	return l1.mEndDate < g_ServerTime && l2.mEndDate >= g_ServerTime ?
			true :
			(l1.mEndDate >= g_ServerTime && l2.mEndDate < g_ServerTime ?
					false : l1.mEndDate > l2.mEndDate);
}

bool endTimeSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	return l1.mEndDate > l2.mEndDate;
}

bool levelSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	ItemDef *itemDef1 = g_ItemManager.GetSafePointerByID(l1.mItemId);
	ItemDef *itemDef2 = g_ItemManager.GetSafePointerByID(l2.mItemId);
	int lev1 = itemDef1 == NULL ? 0 : itemDef1->mLevel;
	int lev2 = itemDef2 == NULL ? 0 : itemDef2->mLevel;
	return lev1 > lev2;
}

bool qualitySort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	ItemDef *itemDef1 = g_ItemManager.GetSafePointerByID(l1.mItemId);
	ItemDef *itemDef2 = g_ItemManager.GetSafePointerByID(l2.mItemId);
	int q1 = itemDef1 == NULL ? 0 : itemDef1->mQualityLevel;
	int q2 = itemDef2 == NULL ? 0 : itemDef2->mQualityLevel;
	return q1 > q2;
}

bool typeSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	ItemDef *itemDef1 = g_ItemManager.GetSafePointerByID(l1.mItemId);
	ItemDef *itemDef2 = g_ItemManager.GetSafePointerByID(l2.mItemId);
	int t1 = itemDef1 == NULL ? 0 : itemDef1->mType;
	int t2 = itemDef2 == NULL ? 0 : itemDef2->mType;
	return t1 > t2;
}

bool weaponTypeSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	ItemDef *itemDef1 = g_ItemManager.GetSafePointerByID(l1.mItemId);
	ItemDef *itemDef2 = g_ItemManager.GetSafePointerByID(l2.mItemId);
	int t1 = itemDef1 == NULL ? 0 : itemDef1->mWeaponType;
	int t2 = itemDef2 == NULL ? 0 : itemDef2->mWeaponType;
	return t1 > t2;
}

bool buyCopperSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	return l1.mBuyItNowCopper > l2.mBuyItNowCopper;
}

bool buyCreditsSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	return l1.mBuyItNowCredits > l2.mBuyItNowCredits;
}

bool bidCopperSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	unsigned long c1 =
			l1.mBids.size() == 0 ? 0 : l1.mBids[l1.mBids.size() - 1].mCopper;
	unsigned long c2 =
			l2.mBids.size() == 0 ? 0 : l2.mBids[l2.mBids.size() - 1].mCopper;
	return c1 > c2;
}

bool bidCreditsSort(const AuctionHouseItem l1, const AuctionHouseItem l2) {
	unsigned long c1 =
			l1.mBids.size() == 0 ?
					0 : l1.mBids[l1.mBids.size() - 1].mCredits;
	unsigned long c2 =
			l2.mBids.size() == 0 ?
					0 : l2.mBids[l2.mBids.size() - 1].mCredits;
	return c1 > c2;
}

AuctionHouseItem AuctionHouseManager::Auction(CreatureInstance *creatureInstance, CharacterServerData *pld, InventorySlot &slot, unsigned long copperCommision, unsigned long creditsCommision, int auctioneer,
		unsigned long reserveCopper, unsigned long reserveCredits, unsigned long buyItNowCopper, unsigned long buyItNowCredits,
		int days, int hours) {
	if (copperCommision) {
		creatureInstance->css.copper -= copperCommision;
		creatureInstance->SendStatUpdate(STAT::COPPER);
	}

	if (creditsCommision > 0) {
		creatureInstance->css.credits -= creditsCommision;
		if (g_Config.AccountCredits) {
			pld->accPtr->Credits = creatureInstance->css.credits;
			pld->accPtr->PendingMinorUpdates++;
		}
		creatureInstance->SendStatUpdate(STAT::CREDITS);
	}

	AuctionHouseItem ahItem;
	ahItem.mId = g_ClusterManager.NextValue(ID_NEXT_AUCTION_ITEM_ID);
	ahItem.mAuctioneer = auctioneer;
	ahItem.mReserveCopper = reserveCopper;
	ahItem.mReserveCredits = reserveCredits;
	ahItem.mBuyItNowCopper = buyItNowCopper;
	ahItem.mBuyItNowCredits = buyItNowCredits;
	ahItem.mStartDate = g_ServerTime;
	ahItem.mEndDate = ahItem.mStartDate + (hours * 60 * 60 * 1000)
			+ (days * 60 * 60 * 24 * 1000);
	ahItem.mSeller = pld->CreatureDefID;
	ahItem.mItemId = slot.IID;
	ahItem.mLookId = slot.customLook;
	ahItem.mCount = slot.count;
	ahItem.mSecondsRemaining = slot.secondsRemaining;

	if(SaveItem(ahItem))
		g_ClusterManager.ListAdd(LISTPREFIX_AUCTION_ITEMS, StringUtil::Format("%d", ahItem.mId));
	// Finish auction

	CharacterData *auctioneerInstance= g_CharacterManager.GetPointerByID(auctioneer);
	ItemDef *item = g_ItemManager.GetSafePointerByID(ahItem.mItemId);
	ChatMessage cm(StringUtil::Format("%s is selling %s at %s's auction house",
			pld->charPtr->cdef.css.display_name, item->mDisplayName.c_str(),
			auctioneerInstance->cdef.css.display_name));
	cm.mChannelName = "tc/";
	cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
	cm.mSender = "EEBay";
	g_ChatManager.SendChatMessage(cm, NULL);

	// Broadcast
	BroadcastAndSetupTimer(&ahItem, pld->charPtr->cdef.css.display_name);
	g_ClusterManager.Auction(ahItem.mBuyItNowCopper, pld->charPtr->cdef.css.display_name);

	return ahItem;
}

void AuctionHouseManager::BroadcastAndSetupTimer(AuctionHouseItem *ahItem, const std::string &sellerName) {
	BroadcastNewItem(ahItem->mId, sellerName);
	if(g_ClusterManager.IsMaster()) {
		g_Logs.server->info("This is the master shard, setting up auction time for auction item %v", ahItem->mId);
		mTimers[ahItem->mId] = g_Scheduler.Schedule([this, ahItem](){
			CompleteAuction(ahItem->mId);
		}, ahItem->mEndDate);
	}
}

void AuctionHouseManager::BroadcastUpdate(int auctioneerCID, AuctionHouseItem &item) {
	char buf[1024];
	int wpos2 = 0;
	wpos2 += PutByte(&buf[wpos2], 97);
	wpos2 += PutShort(&buf[wpos2], 0);
	wpos2 += PutByte(&buf[wpos2], 2);
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", auctioneerCID).c_str());
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", item.mId).c_str());
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%lu", item.GetSecondsRemaining()).c_str());
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", item.mBids.size()).c_str());
	AuctionHouseBid highBid = item.mBids[item.mBids.size() - 1];
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%lu", highBid.mCopper).c_str());
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%lu", highBid.mCredits).c_str());
	PutShort(&buf[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(buf, wpos2, NULL);
}

void AuctionHouseManager::BroadcastRemovedItem(int auctioneerCID, int auctionItemID) {
	char buf[1024];
	int wpos2 = 0;
	wpos2 += PutByte(&buf[wpos2], 97);
	wpos2 += PutShort(&buf[wpos2], 0);
	wpos2 += PutByte(&buf[wpos2], 3);
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", auctioneerCID).c_str());
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", auctionItemID).c_str());
	PutShort(&buf[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(buf, wpos2, NULL);
}

void AuctionHouseManager::BroadcastNewItem(int auctionItemID, const std::string &sellerName) {
	AuctionHouseItem ahItem = LoadItem(auctionItemID);
	if(ahItem.mId == 0) {
		g_Logs.server->error("Request to broadcast an auction item that doesn't exist [%v]", auctionItemID);
		return;
	}
	CreatureInstance *cinst = g_ActiveInstanceManager.GetNPCCreatureByDefID(ahItem.mAuctioneer);
	char buf[1024];
	int wpos2 = 0;
	wpos2 += PutByte(buf, 97);
	wpos2 += PutShort(&buf[wpos2], 0);
	wpos2 += PutByte(&buf[wpos2], 1);
	/* The client should be given the CID not the CDefID if it is known. Any players that are
	 * interacting with that auctioneer on this shard will handle the update */
	wpos2 += PutStringUTF(&buf[wpos2], StringUtil::Format("%d", cinst == NULL ? 0 : cinst->CreatureID).c_str());
	wpos2 += WriteAuctionItem(&buf[wpos2], &ahItem, sellerName);
	PutShort(&buf[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(buf, wpos2, NULL);
}

void AuctionHouseManager::Search(AuctionHouseSearch &search,
		std::vector<AuctionHouseItem> &results) {

	STRINGLIST ids = g_ClusterManager.GetList(LISTPREFIX_AUCTION_ITEMS);

	for (auto it = ids.begin(); it != ids.end(); ++it) {
		AuctionHouseItem item;
		item.mId = atoi((*it).c_str());
		if(!g_ClusterManager.ReadEntity(&item)) {
			g_Logs.data->error("Could not read auction house item %v",item.mId);
			continue;
		}

		if (search.mMaxRows > -1 && results.size() >= search.mMaxRows)
			break;

		ItemDef *itemDef = g_ItemManager.GetSafePointerByID(item.mItemId);

		if (itemDef == NULL) {
			g_Logs.data->warn("Item in auction house does not link to a valid item. %v",
					item.mItemId);
		} else {
			bool matches = item.mAuctioneer == 0
					|| search.mAuctioneer == item.mAuctioneer;
			if (matches) {
				matches = search.mItemTypeId == -1
						|| search.mItemTypeId == itemDef->mType;
			}
			if (matches) {
				matches = search.mQualityId == -1
						|| search.mQualityId
								== itemDef->mQualityLevel;
			}
			if (matches) {
				matches = itemDef->mLevel >= search.mLevelStart
						&& itemDef->mLevel <= search.mLevelEnd;
			}
			if (matches) {
				matches = item.mBuyItNowCopper
						>= search.mBuyPriceCopperStart
						&& item.mBuyItNowCopper
								<= search.mBuyPriceCopperEnd;
			}
			if (matches) {
				matches = item.mBuyItNowCredits == 0
						|| (item.mBuyItNowCredits
								>= search.mBuyPriceCreditsStart
								&& item.mBuyItNowCredits
										<= search.mBuyPriceCreditsEnd);
			}
//			if (matches) {
//				unsigned long bidCopper = 0;
//				unsigned long bidCredits = 0;
//				if(it->second->mBids.size() > 0) {
//					bidCopper = it->second->mBids[it->second->mBids.size() - 1].mCopper;
//					bidCredits = it->second->mBids[it->second->mBids.size() - 1].mCredits;
//				}
//				matches = bidCopper >= search.mit->second->mBuyItNowCredits == 0
//						|| (it->second->mBuyItNowCredits
//								>= search.mBuyPriceCreditsStart
//								&& it->second->mBuyItNowCredits
//										<= search.mBuyPriceCreditsEnd);
//			}
			if (matches) {
				matches = search.mSearch.length() == 0
						|| Util::CaseInsensitiveStringFind(
								itemDef->mDisplayName,
								search.mSearch);
			}

			if (matches) {
				results.push_back(item);
			}
		}
	}

	switch (search.mOrder) {
	case AuctionHouseSearchOrder::REMAINING:
		sort(results.begin(), results.end(), remainingSort);
		break;
	case AuctionHouseSearchOrder::END_TIME:
		sort(results.begin(), results.end(), endTimeSort);
		break;
	case AuctionHouseSearchOrder::LEVEL:
		sort(results.begin(), results.end(), levelSort);
		break;
	case AuctionHouseSearchOrder::QUALITY:
		sort(results.begin(), results.end(), qualitySort);
		break;
	case AuctionHouseSearchOrder::TYPE:
		sort(results.begin(), results.end(), typeSort);
		break;
	case AuctionHouseSearchOrder::WEAPON_TYPE:
		sort(results.begin(), results.end(), weaponTypeSort);
		break;
	case AuctionHouseSearchOrder::BUY_COPPER:
		sort(results.begin(), results.end(), buyCopperSort);
		break;
	case AuctionHouseSearchOrder::BUY_CREDITS:
		sort(results.begin(), results.end(), buyCreditsSort);
		break;
	case AuctionHouseSearchOrder::BID_COPPER:
		sort(results.begin(), results.end(), bidCopperSort);
		break;
	case AuctionHouseSearchOrder::BID_CREDITS:
		sort(results.begin(), results.end(), bidCreditsSort);
		break;
	}

	if (search.mReverse)
		std::reverse(results.begin(), results.end());
}

void AuctionHouseManager::CancelAllTimers(void) {
	g_Logs.server->info("Cancelling all auction house timers");
	cs.Enter("AuctionHouseManager::CancelAllTimers");
	for(auto it = mTimers.begin(); it != mTimers.end(); ++it) {
		g_Scheduler.Cancel((*it).second);
	}
	mTimers.clear();
	cs.Leave();
}

int AuctionHouseManager::LoadItems(void) {
	/* Do initial load to setup timers */

	STRINGLIST ids = g_ClusterManager.GetList(LISTPREFIX_AUCTION_ITEMS);
	for (auto it = ids.begin(); it != ids.end(); ++it) {
		AuctionHouseItem item = LoadItem(atoi((*it).c_str()));

		cs.Enter("AuctionHouseManager::LoadItem");
		CancelItemTimer(item.mId);
		if(g_ClusterManager.IsMaster()) {
			/* Only the master shard deals with auction house timers */
			if (!item.mCompleted) {
				// Finish auction
				mTimers[item.mId] = g_Scheduler.Schedule([this, item](){
					CompleteAuction(item.mId);
				}, item.mEndDate);
			} else {
				// Remove the item completely
				mTimers[item.mId] = g_Scheduler.Schedule([this, item](){
					ExpireItem(item);
				}, item.mEndDate + (g_Config.MaxAuctionExpiredHours * 3600000));
			}
		}
		cs.Leave();
	}
	g_Logs.server->info("Loaded %v active auction house items", ids.size());
	return 0;
}

void AuctionHouseManager::ConnectToSite() {
	/* We need a session with which to send private message from the auction system. This
	 * may need to be refreshed periodically
	 */
	session = new HTTPD::SiteSession();
	SiteClient sc(g_Config.ServiceAuthURL);
	sc.refreshXCSRF(session);
	sc.login(session, "eebay", "changeme!");
	sc.refreshXCSRF(session);
}

//
// AuctionHouseSearch
//
AuctionHouseSearch::AuctionHouseSearch() {
	mAuctioneer = 0;
	mItemTypeId = -1;
	mQualityId = -1;
	mMaxRows = -1;
	mReverse = false;
	mOrder = AuctionHouseSearchOrder::REMAINING;
	mBuyPriceCopperStart = 0;
	mBuyPriceCopperEnd = 99999999;
	mBuyPriceCreditsStart = 0;
	mBuyPriceCreditsEnd = 9999999;
	mLevelStart = 0;
	mLevelEnd = 99;
}

AuctionHouseSearch::~AuctionHouseSearch() {
}
