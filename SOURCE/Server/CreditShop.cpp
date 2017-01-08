#include "CreditShop.h"
#include "Config.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"

#include "Item.h"
#include <string.h>
#include "util/Log.h"

using namespace CS;

CreditShopManager g_CreditShopManager;

namespace Status {

const char *GetNameByID(int id) {
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

const char *GetNameByID(int id) {
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

const char *GetNameByID(int id) {
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
	nextMarketItemID = 1;
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

	unsigned long nowTimeSec = time(NULL);
	if (csItem->mStartDate != 0 && nowTimeSec < (csItem->mStartDate))
		return CreditShopError::NOT_YET_AVAILABLE;

	if (csItem->mEndDate != 0 && nowTimeSec >= (csItem->mEndDate))
		return CreditShopError::NO_LONGER_AVAILABLE;

	int slot = cd->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return CreditShopError::NOT_ENOUGH_FREE_SLOTS;

	return CreditShopError::NONE;
}

bool CreditShopManager::SaveItem(CreditShopItem * item) {
	std::string path = GetPath(item->mId);
	g_Logs.data->info("Saving credit shop item to %v.", path.c_str());
	FILE *output = fopen(path.c_str(), "wb");
	if (output == NULL) {
		g_Logs.data->error("Saving petition could not open: %v", path.c_str());
		return false;
	}

	fprintf(output, "[ENTRY]\r\n");
	if (item->mTitle.compare("") != 0)
		fprintf(output, "Title=%s\r\n", item->mTitle.c_str());
	if (item->mDescription.compare("") != 0)
		fprintf(output, "Description=%s\r\n", item->mDescription.c_str());
	if (item->mStartDate > 0)
		fprintf(output, "BeginDate=%s\r\n",
				Util::FormatDate(&item->mStartDate).c_str());
	if (item->mEndDate > 0)
		fprintf(output, "EndDate=%s\r\n",
				Util::FormatDate(&item->mEndDate).c_str());
	if (item->mCreatedDate > 0)
			fprintf(output, "CreatedDate=%s\r\n",
					Util::FormatDate(&item->mStartDate).c_str());
	fprintf(output, "PriceCurrency=%s\r\n",
			Currency::GetNameByID(item->mPriceCurrency));
	fprintf(output, "PriceCopper=%lu\r\n", item->mPriceCopper);
	fprintf(output, "PriceCredits=%lu\r\n", item->mPriceCredits);
	fprintf(output, "ItemID=%d\r\n", item->mItemId);
	if (item->mIv1 > 0)
		fprintf(output, "Iv1=%d\r\n", item->mIv1);
	if (item->mIv2 > 0)
		fprintf(output, "Iv2=%d\r\n", item->mIv2);
	if (item->mLookId > 0)
		fprintf(output, "LookId=%d\r\n", item->mLookId);
	fprintf(output, "Category=%s\r\n", Category::GetNameByID(item->mCategory));
	fprintf(output, "Status=%s\r\n", Status::GetNameByID(item->mStatus));
	if (item->mQuantityLimit > 0)
		fprintf(output, "QuantityLimit=%d\r\n", item->mQuantityLimit);
	if (item->mQuantitySold > 0)
		fprintf(output, "QuantitySold=%d\r\n", item->mQuantitySold);

	fprintf(output, "\r\n");

	fflush(output);
	fclose(output);

	return true;
}

CreditShopItem * CreditShopManager::LoadItem(int id) {
	std::string buf = GetPath(id);
	if (!Platform::FileExists(buf.c_str())) {
		g_Logs.data->error("No file for CS item [%v]", buf.c_str());
		return NULL;
	}

	CreditShopItem *item = new CreditShopItem();

	FileReader lfr;
	if (lfr.OpenText(buf.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", buf.c_str());
		return NULL;
	}

//		unsigned long mStartDate;
//		unsigned long mEndDate;

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	long amt = -1;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (item->mId != 0) {
					g_Logs.data->warn(
							"%v contains multiple entries. CS items have one entry per file",
							buf.c_str());
					break;
				}
				item->mId = id;
			} else if (strcmp(lfr.SecBuffer, "TITLE") == 0)
				item->mTitle = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "DESCRIPTION") == 0)
				item->mDescription = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "BEGINDATE") == 0)
				Util::ParseDate(lfr.BlockToStringC(1, 0), item->mStartDate);
			else if (strcmp(lfr.SecBuffer, "ENDDATE") == 0)
				Util::ParseDate(lfr.BlockToStringC(1, 0), item->mEndDate);
			else if (strcmp(lfr.SecBuffer, "CREATEDDATE") == 0)
				Util::ParseDate(lfr.BlockToStringC(1, 0), item->mCreatedDate);
			else if (strcmp(lfr.SecBuffer, "PRICECURRENCY") == 0)
				item->mPriceCurrency = Currency::GetIDByName(
						lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "PRICEAMOUNT") == 0)
				amt = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "PRICECOPPER") == 0)
				item->mPriceCopper = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "PRICECREDITS") == 0)
				item->mPriceCredits = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "ITEMID") == 0)
				item->mItemId = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "ITEMAMOUNT") == 0
					|| strcmp(lfr.SecBuffer, "IV1") == 0)
				item->mIv1 = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "IV2") == 0)
				item->mIv2 = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "LOOKID") == 0)
				item->mLookId = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "CATEGORY") == 0)
				item->mCategory = Category::GetIDByName(
						lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "STATUS") == 0)
				item->mStatus = Status::GetIDByName(lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "QUANTITYLIMIT") == 0)
				item->mQuantityLimit = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "QUANTITYSOLD") == 0)
				item->mQuantitySold = lfr.BlockToIntC(1);
			else
				g_Logs.data->error("Unknown identifier [%v] in file [%v]",
						lfr.SecBuffer, buf.c_str());
		}
	}
	lfr.CloseCurrent();

// For backwards compatibility - will be able to remove once all items resaved or removed
	if (amt > 0) {
		if (item->mPriceCurrency == Currency::COPPER)
			item->mPriceCopper = amt;
		else if (item->mPriceCurrency == Currency::CREDITS)
			item->mPriceCredits = amt;
	}

	cs.Enter("CreditShopManager::LoadItem");
	mItems[id] = item;
	cs.Leave();

	return item;
}

bool CreditShopManager::RemoveItem(int id) {
	const char * path = GetPath(id).c_str();
	if (!Platform::FileExists(path)) {
		g_Logs.data->error("No file for CS item [%v] to remove", path);
		return false;
	}
	cs.Enter("CreditShopManager::RemoveItem");
	std::map<int, CreditShopItem*>::iterator it = mItems.find(id);
	if (it != mItems.end()) {
		delete it->second;
		mItems.erase(it);
	}
	cs.Leave();
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "CreditShop/%d.del", id);
	Platform::FixPaths(buf);
	if (!Platform::FileExists(buf) || remove(buf) == 0) {
		if (!rename(path, buf) == 0) {
			g_Logs.data->error("Failed to remove credit shop item %v", id);
			return false;
		}
	}
	return true;
}

std::string CreditShopManager::GetPath(int id) {

	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "CreditShop/%d.txt", id);
	Platform::FixPaths(buf);
	return buf;
}

int CreditShopManager::LoadItems(void) {
	for (std::map<int, CreditShopItem*>::iterator it = mItems.begin();
			it != mItems.end(); ++it)
		delete it->second;
	mItems.clear();

	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory("CreditShop");
	r.ReadFiles();
	r.SetDirectory(dir);

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			CreditShopItem *item = LoadItem(atoi(Platform::Basename(p.c_str()).c_str()));
			g_Logs.data->debug("Credit shop item %v (item ID %v)", item->mId, item->mItemId);
			if(item != NULL) {
				if(item->mId >= nextMarketItemID) {
					nextMarketItemID = item->mId + 1;
					g_Logs.data->warn("Adjusted next market item ID to %v");
				}
			}
		}
	}

	return 0;
}

CreditShopItem * CreditShopManager::GetItem(int id) {
	cs.Enter("CreditShopManager::GetItem");
	std::map<int, CreditShopItem*>::iterator it = mItems.find(id);
	CreditShopItem *item = it == mItems.end() ? NULL : it->second;
	cs.Leave();
	return item;
}
