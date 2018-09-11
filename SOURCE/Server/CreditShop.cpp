#include "CreditShop.h"
#include "Config.h"
#include "Util.h"
#include "StringUtil.h"
#include "FileReader.h"
#include "Cluster.h"
#include "DirectoryAccess.h"

#include "Item.h"
#include <string.h>
#include "util/Log.h"

using namespace CS;

CreditShopManager g_CreditShopManager;

namespace Status {

std::string GetNameByID(int id) {
	switch (id) {
	case HOT:
		return "HOT";
	case NEW:
		return "NEW";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (name.compare("HOT") == 0)
		return HOT;
	if (name.compare("NEW") == 0)
		return NEW;
	return UNDEFINED;
}

}

namespace Category {

std::string GetNameByID(int id) {
	switch (id) {
	case ARMOR:
		return "ARMOR";
	case BAGS:
		return "BAGS";
	case CHARMS:
		return "CHARMS";
	case CONSUMABLES:
		return "CONSUMABLES";
	case RECIPES:
		return "RECIPES";
	case PETS:
		return "PETS";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (name.compare("ARMOR") == 0)
		return ARMOR;
	if (name.compare("BAGS") == 0)
		return BAGS;
	if (name.compare("CHARMS") == 0)
		return CHARMS;
	if (name.compare("CONSUMABLES") == 0)
		return CONSUMABLES;
	if (name.compare("RECIPES") == 0)
		return RECIPES;
	if (name.compare("PETS") == 0)
		return PETS;
	return UNDEFINED;
}

}

namespace CreditShopError {
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

namespace Currency {

std::string GetNameByID(int id) {
	switch (id) {
	case CREDITS:
		return "CREDITS";
	case COPPER:
		return "COPPER";
	case COPPER_CREDITS:
		return "COPPER+CREDITS";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (name.compare("COPPER") == 0)
		return COPPER;
	if (name.compare("CREDITS") == 0)
		return CREDITS;
	if (name.compare("COPPER+CREDITS") == 0)
		return COPPER_CREDITS;
	return UNDEFINED;
}

}

//
// CreditShopItem
//

CreditShopItem::CreditShopItem() {
	mCategory = Category::UNDEFINED;
	mStatus = Category::UNDEFINED;
	mPriceCurrency = Currency::COPPER;
	mId = 0;
	mStartDate = 0;
	mEndDate = 0;
	mCreatedDate = 0;
	mPriceCopper = 0;
	mPriceCredits = 0;
	mQuantityLimit = 0;
	mQuantitySold = 0;
	mTitle = "";
	mDescription = "";
	mItemId = 0;
	mLookId = 0;
	mIv1 = 0;
	mIv2 = 0;
}
CreditShopItem::~CreditShopItem() {
}


bool CreditShopItem :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_CS_ITEM, StringUtil::Format("%d", mId));
	writer->Value("Title", mTitle);
	writer->Value("Description", mDescription);
	if(mStartDate > 0)
		writer->Value("BeginDate", Util::FormatDate(&mStartDate));
	if(mEndDate > 0)
		writer->Value("EndDate", Util::FormatDate(&mEndDate));
	if(mCreatedDate  > 0)
		writer->Value("CreatedDate", Util::FormatDate(&mCreatedDate));
	writer->Value("PriceCurrency", Currency::GetNameByID(mPriceCurrency));
	writer->Value("PriceCopper", mPriceCopper);
	writer->Value("PriceCredits", mPriceCredits);
	writer->Value("ItemID", mItemId);
	if(mIv1 >0)
		writer->Value("Iv1", mIv1);
	if(mIv2 >0)
		writer->Value("Iv2", mIv2);
	if(mLookId >0)
		writer->Value("LookId", mLookId);
	writer->Value("Category", Category::GetNameByID(mCategory));
	if(mStatus != Status::UNDEFINED)
		writer->Value("Status", Status::GetNameByID(mStatus));
	if(mQuantityLimit > 0)
		writer->Value("QuantityLimit", mQuantityLimit);
	if(mQuantitySold > 0)
		writer->Value("QuantitySold", mQuantitySold);

	return true;
}

bool CreditShopItem :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_CS_ITEM, StringUtil::Format("%d", mId), true);
	return true;
}

bool CreditShopItem :: ReadEntity(AbstractEntityReader *reader) {
	mTitle = reader->Value("Title");
	mDescription = reader->Value("Description");
	std::string s = reader->Value("BeginDate");
	if(s.length() > 0)
		Util::ParseDate(s, mStartDate);
	s = reader->Value("EndDate");
	if(s.length() > 0)
		Util::ParseDate(s, mEndDate);
	s = reader->Value("CreatedDate");
	if(s.length() > 0)
		Util::ParseDate(s, mCreatedDate);
	mPriceCurrency = Currency::GetIDByName(reader->Value("PriceCurrency", Currency::GetNameByID(Currency::COPPER)));
	long amt = reader->ValueULong("PriceAmount", 0);
	mPriceCopper = reader->ValueULong("PriceCopper", 0);
	mPriceCredits = reader->ValueULong("PriceCredits", 0);
	mItemId = reader->ValueInt("ItemID", 0);
	mIv1 = reader->ValueInt("Iv1", reader->ValueULong("ItemAmount", 0));
	mIv2 = reader->ValueInt("Iv2", 0);
	mLookId = reader->ValueInt("LookId", 0);
	s = reader->Value("Category");
	if(s.length() > 0)
		mCategory = Category::GetIDByName(s);
	s = reader->Value("Status");
	if(s.length() > 0)
		mStatus = Status::GetIDByName(s);
	mQuantityLimit = reader->ValueInt("QuantityLimit", 0);
	mQuantitySold = reader->ValueInt("QuantitySold", 0);

	// For backwards compatibility - will be able to remove once all items resaved or removed
	if (amt > 0) {
		if (mPriceCurrency == Currency::COPPER)
			mPriceCopper = amt;
		else if (mPriceCurrency == Currency::CREDITS)
			mPriceCredits = amt;
	}

	return true;
}

void CreditShopItem::WriteToJSON(Json::Value &value) {
	value["id"] = mId;
	value["title"] = mTitle;
	value["description"] = mDescription;
	if (mStartDate > 0)
		value["beginDate"] = Util::FormatDate(&mStartDate);
	if (mEndDate > 0)
		value["endDate"] = Util::FormatDate(&mEndDate);
	if (mCreatedDate > 0)
		value["createdDate"] = Util::FormatDate(&mCreatedDate);
	value["currency"] = Currency::GetNameByID(mPriceCurrency);
	value["copper"] = Json::UInt64(mPriceCopper);
	value["credits"] = Json::UInt64(mPriceCredits);
	value["itemID"] = mItemId;
	value["lookID"] = mLookId;
	if (mIv1 > 0)
		value["iv1"] = mIv1;
	if (mIv2 > 0)
		value["iv2"] = mIv2;
	value["category"] = Category::GetNameByID(mCategory);
	value["status"] = Status::GetNameByID(mStatus);
	if (mQuantityLimit > 0)
		value["limit"] = mQuantityLimit;
	if (mQuantitySold > 0)
		value["sold"] = mQuantitySold;
}

void CreditShopItem::ParseItemProto(std::string proto) {

	std::vector<std::string> p;
	if (Util::HasBeginning(proto, "item")) {
		proto = proto.substr(4);
	}
	Util::Split(proto.c_str(), ":", p);
	if (p.size() > 0) {
		mItemId = atoi(p[0].c_str());
		if (p.size() > 1) {
			mLookId = atoi(p[1].c_str());
			if (p.size() > 2) {
				mIv1 = atoi(p[2].c_str());
				if (p.size() > 3) {
					mIv2 = atoi(p[3].c_str());
				}
			}
		}
	}
}

//
// CreditShopManager
//

CreditShopManager::CreditShopManager() {
}

CreditShopManager::~CreditShopManager() {
}

int CreditShopManager::ValidateItem(CreditShopItem *csItem, AccountData *accPtr,
		CharacterStatSet *css, CharacterData *cd) {
	if (csItem->mQuantityLimit != 0
			&& (csItem->mQuantityLimit - csItem->mQuantitySold) < 1)
		return CreditShopError::SOLD_OUT;

	if ((csItem->mPriceCurrency == Currency::COPPER
			|| csItem->mPriceCurrency == Currency::COPPER_CREDITS)
			&& (unsigned long) css->copper < csItem->mPriceCopper)
		return CreditShopError::NOT_ENOUGH_COPPER;

	if (csItem->mPriceCurrency == Currency::CREDITS
			|| csItem->mPriceCurrency == Currency::COPPER_CREDITS) {
		if (g_Config.AccountCredits) {
			css->credits = accPtr->Credits;
		}
		if ((unsigned long) css->credits < csItem->mPriceCredits)
			return CreditShopError::NOT_ENOUGH_CREDITS;
	}

	time_t nowTimeSec = time(NULL);
	if (csItem->mStartDate != 0 && nowTimeSec < (csItem->mStartDate))
		return CreditShopError::NOT_YET_AVAILABLE;

	if (csItem->mEndDate != 0 && nowTimeSec >= (csItem->mEndDate))
		return CreditShopError::NO_LONGER_AVAILABLE;

	int slot = cd->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return CreditShopError::NOT_ENOUGH_FREE_SLOTS;

	return CreditShopError::NONE;
}

bool CreditShopManager::UpdateItem(CreditShopItem * item) {
	g_Logs.data->info("Updating credit shop item %v to cluster.", item->mId);
	return g_ClusterManager.WriteEntity(item);
}

void CreditShopManager::GetItems(std::vector<CreditShopItem> &items) {
	STRINGLIST ids = g_ClusterManager.GetList(LISTPREFIX_CS_ITEMS);
	for(auto it = ids.begin(); it != ids.end(); ++it) {
		items.push_back(GetItem(atoi((*it).c_str())));
	}
}

bool CreditShopManager::CreateItem(CreditShopItem * item) {
	item->mId = g_ClusterManager.NextValue(ID_CS_ITEM_ID);
	item->mCreatedDate = g_ServerTime / 1000;
	g_Logs.data->info("Creating credit shop item %v to cluster.", item->mId);
	if(g_ClusterManager.WriteEntity(item)) {
		return g_ClusterManager.ListAdd(LISTPREFIX_CS_ITEMS, StringUtil::Format("%d", item->mId));
	}
	return false;
}

CreditShopItem  CreditShopManager::GetItem(int id) {
	CreditShopItem item;
	item.mId = id;
	g_ClusterManager.ReadEntity(&item);
	return item;
}

bool CreditShopManager::RemoveItem(int id) {
	CreditShopItem it = GetItem(id);
	if(it.mId == 0) {
		g_Logs.data->error("CS item [%v] could not be found to be removed", id);
		return false;
	}
	if(g_ClusterManager.RemoveEntity(&it)) {
		return g_ClusterManager.ListRemove(LISTPREFIX_CS_ITEMS, StringUtil::Format("%d", id));
	}
	return false;
}

