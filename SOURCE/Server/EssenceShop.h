/*
CLIENT REQUEST
  Query: essenceShop.contents
  Args: CreatureInstanceID    (example: 469768021)
*/

/*
SERVER RESPONSE
  ItemCount = 1 + LootCount      (note: first item is the essence, all others are loot)

	foreach(ItemIndex in ItemCount)
	{
	  if(ItemCount == 0)
	  {
		[ItemIndex][0] = Item ID (For first element, it's the essence to buy)
		[ItemIndex][1] = 0
	  }
	  else
	  {
		[ItemIndex][0] = Item String, ex: "item143548:0:0:0"
		[ItemIndex][1] = Essence cost
	  }
	}

  Note: The Item Proto string only seems to use the first parameter.  The string
  can be constructed with
  sprintf(buffer, "item%d:0:0:0", ItemID);
*/

#pragma once
#ifndef ESSENCESHOP_H
#define ESSENCESHOP_H

#include <vector>

struct EssenceShopItem
{
	int ItemID;
	int EssenceCost;

	EssenceShopItem();
	EssenceShopItem(int id, int cost);
	void SetValues(int id, int cost);
};

class EssenceShop
{
public:
	EssenceShop();
	~EssenceShop();

	int CreatureDefID;
	int EssenceID;
	std::vector<EssenceShopItem> EssenceList;
	void AddItem(int id, int cost);
	int WriteQueryEssenceShop(char *buffer, char *convbuf, int QueryID);
	int WriteQueryShop(char *buffer, char *convbuf, int QueryID);
	int GetItemIDFromProto(char *itemProtoStr);
	int GetTradeIndexByID(int itemID);
	void Clear(void);
};




class EssenceShopContainer
{
public:
	EssenceShopContainer();
	~EssenceShopContainer();
	void Clear(void);

	std::vector<EssenceShop> EssenceShopList;
	int GetShopByCDefID(int cdefid);
	int ProcessQueryEssenceShop(char *buffer, char *convbuf, int cdefid, int queryID);
	int ProcessQueryShop(char *buffer, char *convbuf, int cdefid, int queryID);
	EssenceShop * GetEssenceShopPtr(int cdefid);
	void LoadFromFile(char *filename);
	EssenceShop * GetEssenceShopPtr(int cdefid, char *itemproto, EssenceShopItem **chosenItem);

private:
	void AddItem(EssenceShop &newItem);
	bool IsInteger(char *str);
	int ResolveItemIdentifier(char *str);
};



#endif //ESSENCESHOP_H