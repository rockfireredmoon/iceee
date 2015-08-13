#include "EssenceShop.h"

#include "ByteBuffer.h"
#include "Util.h"
#include "Item.h"
#include "FileReader.h"
#include "StringList.h"
#include <string.h>

EssenceShopItem :: EssenceShopItem()
{
	SetValues(0, 0);
}
EssenceShopItem :: EssenceShopItem(int id, int cost)
{
	SetValues(id, cost);
}
void EssenceShopItem :: SetValues(int id, int cost)
{
	ItemID = id;
	EssenceCost = cost;
}


EssenceShop :: EssenceShop()
{
	Clear();
	EssenceID = 0;
}

EssenceShop :: ~EssenceShop()
{
	EssenceList.clear();
}

void EssenceShop :: Clear(void)
{
	CreatureDefID = 0;
	EssenceID = 0;
	EssenceList.clear();
}

void EssenceShop :: AddItem(int id, int cost)
{
	EssenceList.push_back(EssenceShopItem(id, cost));
}

int EssenceShop :: WriteQueryEssenceShop(char *buffer, char *convbuf, int QueryID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], QueryID);     //Query response ID

	int rowCount = EssenceList.size();
	wpos += PutShort(&buffer[wpos], 1 + rowCount);

	wpos += PutByte(&buffer[wpos], 2);              //Always two strings.
	sprintf(convbuf, "%d", EssenceID);
	wpos += PutStringUTF(&buffer[wpos], convbuf);
	wpos += PutStringUTF(&buffer[wpos], "0");       //Second parameter not used for essence id?

	for(int index = 0; index < rowCount; index++)
	{
		wpos += PutByte(&buffer[wpos], 2);              //Always two strings.

		GetItemProto(convbuf, EssenceList[index].ItemID, 0);
		wpos += PutStringUTF(&buffer[wpos], convbuf);
		sprintf(convbuf, "%d", EssenceList[index].EssenceCost);
		wpos += PutStringUTF(&buffer[wpos], convbuf);
	}

	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int EssenceShop :: WriteQueryShop(char *buffer, char *convbuf, int QueryID)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], QueryID);     //Query response ID

	int rowCount = EssenceList.size();
	wpos += PutShort(&buffer[wpos], rowCount);

	for(int index = 0; index < rowCount; index++)
	{
		wpos += PutByte(&buffer[wpos], 1);          //Always one string for regular items.
		GetItemProto(convbuf, EssenceList[index].ItemID, 0);
		wpos += PutStringUTF(&buffer[wpos], convbuf);
	}
	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int EssenceShop :: GetItemIDFromProto(char *itemProtoStr)
{
	//The item proto string is formatted like this:
	//  "item%d:0:0:0"

	char *pos = strstr(itemProtoStr, "item");
	if(pos == NULL)
		return 0;

	char *epos = strchr(pos, ':');
	if(epos == NULL)
		return 0;

	*epos = 0;
	int itemID = atoi(pos + 4);
	*epos = ':';
	
	return itemID;
}

int EssenceShop :: GetTradeIndexByID(int itemID)
{
	int a;
	for(a = 0; a < (int)EssenceList.size(); a++)
		if(EssenceList[a].ItemID == itemID)
			return a;

	return -1;
}








EssenceShopContainer :: EssenceShopContainer()
{
}

EssenceShopContainer :: ~EssenceShopContainer()
{
	EssenceShopList.clear();
}

void EssenceShopContainer :: Clear(void)
{
	EssenceShopList.clear();
}

void EssenceShopContainer :: AddItem(EssenceShop &newItem)
{
	EssenceShopList.push_back(newItem);
}

int EssenceShopContainer :: GetShopByCDefID(int cdefid)
{
	int a;
	for(a = 0; a < (int)EssenceShopList.size(); a++)
		if(EssenceShopList[a].CreatureDefID == cdefid)
			return a;
	return -1;
}

int EssenceShopContainer :: ProcessQueryEssenceShop(char *buffer, char *convbuf, int cdefid, int queryID)
{
	int r = GetShopByCDefID(cdefid);
	if(r >= 0)
		return EssenceShopList[r].WriteQueryEssenceShop(buffer, convbuf, queryID);

	return 0;
}

int EssenceShopContainer :: ProcessQueryShop(char *buffer, char *convbuf, int cdefid, int queryID)
{
	int r = GetShopByCDefID(cdefid);
	if(r >= 0)
		return EssenceShopList[r].WriteQueryShop(buffer, convbuf, queryID);

	return 0;
}

EssenceShop * EssenceShopContainer :: GetEssenceShopPtr(int cdefid)
{
	int r = GetShopByCDefID(cdefid);
	if(r >= 0)
		return &EssenceShopList[r];

	return NULL;
}

bool EssenceShopContainer :: IsInteger(char *str)
{
	int len = strlen(str);
	for(int a = 0; a < len; a++)
	{
		if(str[a] < '0')
			return false;
		if(str[a] > '9')
			return false;
	}
	return true;
}

int EssenceShopContainer :: ResolveItemIdentifier(char *str)
{
	bool isInt = IsInteger(str);
	if(isInt == true)
		return atoi(str);

	ItemDef *itemPtr = g_ItemManager.GetSafePointerByExactName(str);
	return itemPtr->mID;
}


void EssenceShopContainer :: LoadFromFile(char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[NOTICE] EssenceShop file [%s] not found.", filename);
		return;
	}

	EssenceShop newItem;

	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, Case_Upper);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.CreatureDefID != 0)
				{
					AddItem(newItem);
					newItem.Clear();
				}
			}
			else if(strcmp(lfr.SecBuffer, "CREATUREDEFID") == 0)
			{
				newItem.CreatureDefID = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "ESSENCEID") == 0)
			{
				newItem.EssenceID = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "ITEM") == 0)
			{
				int id = ResolveItemIdentifier(lfr.BlockToStringC(1, 0));
				int cost = lfr.BlockToIntC(2);
				if(id <= 0)
					g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] EssenceShop item [%s] referred to in file [%s] was not found.", lfr.BlockToStringC(1, 0), filename);
				else
					newItem.AddItem(id, cost);
			}
		}
	}
	lfr.CloseCurrent();
	if(newItem.CreatureDefID != 0)
	{
		AddItem(newItem);
		newItem.Clear();
	}
}


EssenceShop * EssenceShopContainer :: GetEssenceShopPtr(int cdefid, char *itemproto, EssenceShopItem **chosenItem)
{
	//Processes a "trade.essence" query.
	//Return a pointer to the EssenceList object
	int index = GetShopByCDefID(cdefid);
	if(index == -1)
	{
		g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] TradeEssence() failed: no EssenceShop for CDef [%d]", cdefid);
		return NULL;
	}

	EssenceShop *esptr = &EssenceShopList[index];

	if(chosenItem == NULL)
		return esptr;

	int itemid = esptr->GetItemIDFromProto(itemproto);
	if(itemid <= 0)
	{
		g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] TradeEssence() failed: itemproto not valid [%s]", itemproto);
		return NULL;
	}

	int itemindex = esptr->GetTradeIndexByID(itemid);
	if(itemindex == -1)
	{
		g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] TradeEssence() failed: Item [%d] not found in reward list.", itemid);
		return NULL;
	}

	*chosenItem = &esptr->EssenceList[itemindex];
	return esptr;
}
