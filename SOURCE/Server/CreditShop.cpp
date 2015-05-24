#include "CreditShop.h"
#include "Util.h"
#include "FileReader.h"
#include "StringList.h"

using namespace CS;

CreditShopManager g_CSManager;

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
	case NEW:
		return "NEW";
	case RECIPE:
		return "RECIPE";
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
	if (name.compare("NEW") == 0)
		return NEW;
	if (name.compare("RECIPE") == 0)
		return RECIPE;
	return UNDEFINED;
}

}

namespace Currency {

const char *GetNameByID(int id) {
	switch (id) {
	case CREDITS:
		return "CREDITS";
	case COPPER:
		return "COPPER";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (name.compare("COPPER") == 0)
		return COPPER;
	if (name.compare("CREDITS") == 0)
		return CREDITS;
	return UNDEFINED;
}

}

//
CreditShopItem::CreditShopItem() {
	mCategory = Category::UNDEFINED;
	mStatus = Category::UNDEFINED;
	mPriceCurrency = Currency::COPPER;
	mId = 0;
	mStartDate = 0;
	mEndDate = 0;
	mPriceAmount = 0;
	mQuantityLimit = 0;
	mQuantitySold = 0;
	mTitle = "";
	mDescription = "";
	mItemId = 0;
	mItemAmount = 0;
}

CreditShopManager::CreditShopManager() {
}

CreditShopManager::~CreditShopManager() {
}

CreditShopItem * CreditShopManager::LoadItem(int id) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "CreditShop/%d.txt", id);
	Platform::FixPaths(buf);
	if (!Platform::FileExists(buf)) {
		g_Log.AddMessageFormat("No file for CS item [%s]", buf);
		return NULL;
	}

	CreditShopItem *item = new CreditShopItem();
	item->mId = id;

	FileReader lfr;
	if (lfr.OpenText(buf) != Err_OK) {
		g_Log.AddMessageFormat("Could not open file [%s]", buf);
		return NULL;
	}

//		unsigned long mStartDate;
//		unsigned long mEndDate;

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (item->mId != 0)
					g_Log.AddMessageFormat(
							"[WARNING] %s contains multiple entries. CS items have one entry per file",
							buf);
			}
			else if (strcmp(lfr.SecBuffer, "TITLE") == 0)
				item->mTitle = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "DESCRIPTION") == 0)
				item->mDescription = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "BEGINDATE") == 0)
				Util::ParseDate(lfr.BlockToStringC(1, 0), item->mStartDate);
			else if (strcmp(lfr.SecBuffer, "ENDDATE") == 0)
				Util::ParseDate(lfr.BlockToStringC(1, 0), item->mEndDate);
			else if (strcmp(lfr.SecBuffer, "PRICECURRENCY") == 0)
				item->mPriceCurrency =Currency::GetIDByName(lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "PRICEAMOUNT") == 0)
				item->mPriceAmount = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "ITEMID") == 0)
				item->mItemId = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "ITEMAMOUNT") == 0)
				item->mItemAmount = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "CATEGORY") == 0)
				item->mCategory =Category::GetIDByName(lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "STATUS") == 0)
				item->mStatus = Status::GetIDByName(lfr.BlockToStringC(1, 0));
			else if (strcmp(lfr.SecBuffer, "QUANTITYLIMIT") == 0)
				item->mQuantityLimit = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "QUANTITYSOLD") == 0)
				item->mQuantitySold = lfr.BlockToIntC(1);
			else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]",
						lfr.SecBuffer, buf);
		}
	}
	lfr.CloseCurrent();
	mItems[id] = item;

	return item;
}

int CreditShopManager::LoadItems(void) {
	mItems.clear();
	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory("CreditShop");
	r.ReadFiles();
	r.SetDirectory(dir.c_str());

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			LoadItem(atoi(Platform::Basename(p.c_str())));
		}
	}

	return 0;
}

CreditShopItem * CreditShopManager::GetItem(int id) {
	std::map<int, CreditShopItem*>::iterator it = mItems.find(id);
	return it == mItems.end() ? NULL : it->second;
}

