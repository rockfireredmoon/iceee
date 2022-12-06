#include "Gamble.h"
#include "FileReader.h"
#include "Util.h"
#include "StringList.h"
#include <string.h>

GambleManager g_GambleManager;

GambleItemSelection::GambleItemSelection()
{
	ItemID = 0;
	Shares = 0;
}

void GambleDefinition :: AddItemSelections(const char *itemList)
{
	std::string items = itemList;
	STRINGLIST subitems;
	STRINGLIST entry;

	Util::Split(items, ",", subitems);
	for(size_t i = 0; i < subitems.size(); i++)
	{
		Util::Split(subitems[i], ":", entry);
		if(entry.size() >= 2)
		{
			GambleItemSelection isel;
			isel.ItemID = Util::GetInteger(entry, 0);
			isel.Shares = Util::GetInteger(entry, 1);
			itemSelection.push_back(isel);
		}
	}
}

void GambleDefinition :: Clear(void)
{
	memset(defName, 0, sizeof(defName));
	triggerItemID = 0;
	searchType = 0;
	params.Reset();
	itemSelection.clear();
}

int GambleDefinition :: GetRandomSelection(void)
{
	int maxShares = 0;
	for(size_t i = 0; i < itemSelection.size(); i++)
	{
		maxShares += itemSelection[i].Shares;
	}

	int base = 1;
	int rnd = g_GambleManager.RandInt(1, maxShares);
	for(size_t i = 0; i < itemSelection.size(); i++)
	{
		if(rnd >= base && rnd <= base + itemSelection[i].Shares - 1)
			return itemSelection[i].ItemID;

		base += itemSelection[i].Shares;
	}
	return 0;
}


GambleManager :: GambleManager()
{
}

GambleManager :: ~GambleManager()
{
	defList.clear();
}

int GambleManager :: GetSearchType(const char *name)
{
	static const char *SearchName[5] = {
		"none",
		"linear",
		"equip",
		"query",
		"list",
	};
	static const int SearchRet[5] = {
		SEARCH_NONE,
		SEARCH_LINEARRANGE,
		SEARCH_EQUIP,
		SEARCH_QUERY,
		SEARCH_LIST,
	};
	for(int i = 0; i < 5; i++)
		if(strcmp(SearchName[i], name) == 0)
			return SearchRet[i];

	g_Log.AddMessageFormat("[WARNING] Search type [%s] not found for gamble definition.", name);
	return SEARCH_NONE;
}

void GambleManager :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open file [%s]", filename);
		return;
	}
	GambleDefinition newItem;
	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	while(lfr.FileOpen() == true)
	{
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if(r > 0)
		{
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.searchType != SEARCH_NONE)
				{
					defList.push_back(newItem);
					newItem.Clear();
				}
			}
			else if(strcmp(lfr.SecBuffer, "NAME") == 0)
				Util::SafeCopy(newItem.defName, lfr.BlockToStringC(1, 0), sizeof(newItem.defName));
			else if(strcmp(lfr.SecBuffer, "TRIGGERITEMID") == 0)
				newItem.triggerItemID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "SEARCHTYPE") == 0)
				newItem.searchType = GetSearchType(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "MINID") == 0)
				newItem.params.minID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "MAXID") == 0)
				newItem.params.maxID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "EQUIPTYPE") == 0)
				newItem.params.equipType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "WEAPONTYPE") == 0)
				newItem.params.weaponType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "MINLEVEL") == 0)
				newItem.params.minLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "MAXLEVEL") == 0)
				newItem.params.maxLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ITEMSELECTION") == 0)
				newItem.AddItemSelections(lfr.BlockToStringC(1, 0));
			else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]", lfr.SecBuffer, filename);
		}
	}
	if(newItem.searchType != SEARCH_NONE)
		defList.push_back(newItem);

	lfr.CloseCurrent();
}


int GambleManager::Randint_32bit(int min, int max) {
	// Generate a 32 bit random number.

//		/*
//			Explanation:
//			rand() doesn't work well for larger numbers.
//			RAND_MAX is limited to 32767.
//			There are other quirks, where powers of two seem to generate more even
//			distributions of numbers.
//
//			Since smaller numbers have better distribution, use a sequence
//			of random numbers and use those to fill the bits of a larger number.
//		*/
//
//		// RAND_MAX (as defined with a value of 0x7fff) is only 15 bits wide.
//		if(min == max)
//			return min;
//		unsigned long rand_build = (rand() << 15) | rand();
//		//unsigned long rand_build = ((rand() & 0xFF) << 24) | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | ((rand() & 0xFF));
//		return min + (rand_build % (max - min + 1));

	// TODO above should not be necessary now
	return RandInt(min, max);
}

int GambleManager::RandInt(int min, int max) {
	//return (int) (((double) rand() / ((double)RAND_MAX + 1) * ((max + 1) - min)) + min);
	std::uniform_int_distribution<int> intDistro(min,max);
	return intDistro(randomEngine);
}

int GambleManager::RandMod(int max) {
	if(max < 2)
		return 0;
//	// Max is exclusive, e.g, max of 10 would give numbers between 0 and 9
//	return rand()%max;
	return RandInt(0, max - 1);
}

int GambleManager::RandI(int max) {
	return RandInt(1, max);
}
int GambleManager::RandModRng(int min, int max) {

	if(min == max)
		return min;
	// Min is inclusive, max is exclusive, e.g, min of 3, max of 10 would give numbers between 3 and 9
	//return(rand()%(max-min)+min);

	return RandInt(min, max - 1);
}
double GambleManager::RandDbl(double min, double max) {
	//return ((double)rand() / ((double)RAND_MAX) * (max - min)) + min;
	std::uniform_real_distribution<double> dblDistro(min,max);
	return dblDistro(randomEngine);
}

int GambleManager :: GetStandardCount(void)
{
	return (int)defList.size();
}

GambleDefinition * GambleManager :: GetGambleDefByTriggerItemID(int itemID)
{
	size_t i = 0;
	for(i = 0; i < defList.size(); i++)
	{
		if(defList[i].triggerItemID == itemID)
			return &defList[i];
	}
	return NULL;
}
