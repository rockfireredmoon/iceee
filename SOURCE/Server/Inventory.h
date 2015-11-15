#pragma once


#define MAXCONTAINER     10

#ifndef INVENTORY_H
#define INVENTORY_H

#include "Item.h"
#include "Stats.h"
#include <vector>
#include <map>

struct InventorySlot
{
	unsigned int CCSID;      //Combined Container/Slot ID
	int IID;                 //Item ID.
	ItemDef *dataPtr;        //Points back to the source item.
	int count;               //Number of items in this stack.
	int customLook;          //If refashioned, this is the Item ID of the new look.
	char bindStatus;         //If true, item is bound to the player.
	long secondsRemaining;    //If nonzero, this is the active play time left until the item is destroyed (offline doesn't count)
	unsigned long timeLoaded; // In order the calculate the time remaining the time the item was last loaded (or saved) is stored.

	InventorySlot()
	{
		CCSID = 0;
		IID = 0;
		dataPtr = NULL;
		count = 0;
		customLook = 0;
		bindStatus = 0;
		secondsRemaining = -1;
		timeLoaded = 0;
	}

	void ApplyFromItemDef(ItemDef *def);
	long AdjustTimes();
	int GetStackCount(void);
	int GetMaxStack(void);
	void CopyFrom(InventorySlot &source, bool copyCCSID);
	void CopyWithoutCount(InventorySlot &source, bool copyCCSID);
	ItemDef * ResolveItemPtr(void);
	ItemDef * ResolveSafeItemPtr(void);
	int GetLookID(void);
	unsigned short GetContainer(void);
	unsigned short GetSlot(void);
	bool IsExpired();
	bool VerifyItemExist(void);
	bool TestEquivalent(InventorySlot &other);
	void ApplyItemIntegerType(int IvType, int IvMax);
	int GetTimeRemaining(void);
};

struct InventoryQuery
{
	unsigned int CCSID;
	int count;
	char type;
	InventorySlot *ptr;
	static const int TYPE_NONE = 0;
	static const int TYPE_REMOVE = 1;
	static const int TYPE_MODIFIED = 2;
};

class InventoryManager
{
public:
	InventoryManager();
	~InventoryManager();
	std::vector<InventorySlot> containerList[MAXCONTAINER];
	unsigned int MaxContainerSlot[MAXCONTAINER];

	int buybackSlot;

	static const int ERROR_NONE   = 0;   //No error.
	static const int ERROR_ITEM   = 1;   //Item could not be found in the server item list.
	static const int ERROR_SPACE  = 2;   //Not enough space for the item.
	static const int ERROR_LIMIT  = 3;   //The limit of this item type

	static const int EQ_ERROR_NONE       	 =  0;  //No error.
	static const int EQ_ERROR_ITEM       	 = -1;  //The item does not exist.
	static const int EQ_ERROR_LEVEL      	 = -2;  //The item cannot be worn by the given character level.
	static const int EQ_ERROR_PROFESSION 	 = -3;  //The item cannot be worn by the given character class.
	static const int EQ_ERROR_SLOT       	 = -4;  //The item cannot be equipped in the given slot.


	unsigned char LastError;

	ItemDef * GetBestSpecialItem(int invID, char specialItemType);
	void SetError(int value);
	InventorySlot * GetExistingPartialStack(int containerID, ItemDef *itemDef);
	int AddItem(int containerID, InventorySlot &item);
	InventorySlot * AddItem_Ex(int containerID, int itemID, int count);
	int ScanRemoveItems(int containerID, int itemID, int count, std::vector<InventoryQuery> &resultList);
	int RemoveItemsAndUpdate(int container, int itemID, int itemCost, char *packetBuffer);
	int FindNextItem(int containerID, int itemID, int start);
	int RemoveItems(int containerID, std::vector<InventoryQuery> &resultList);

	int RemItem(unsigned int editCCSID);
	int GetItemByCCSID(unsigned int findCCSID);
	InventorySlot * GetItemPtrByCCSID(unsigned int findCCSID);
	InventorySlot * GetItemPtrByID(int itemID);
	int GetItemBySlot(int containerID, unsigned int slot);
	void ClearAll(void);
	unsigned int GetCCSID(unsigned short container, unsigned short slot);
	unsigned int GetCCSIDFromHexID(const char *hexStr);

	int GetFreeSlot(int containerID);
	int CountUsedSlots(int containerID);
	int CountFreeSlots(int containerID);
	void CountInventorySlots(void);
	InventorySlot * PickRandomItem(int containerID);
	int GetItemCount(int containerID, int itemID);
	InventorySlot * GetFirstItem(int containerID, int itemID);

	int ItemMove(char *buffer, char *convBuf, CharacterStatSet *css, bool localCharacterVault, int origContainer, int origSlot, InventoryManager *destInv, int destContainer, int destSlot, bool updateDest);
	int AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
	int RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
	int SendItemIDUpdate(char *buffer, char *convBuf, InventorySlot *oldSlot, InventorySlot *newSlot);

	void FixBuyBack(void);
	int AddBuyBack(InventorySlot *item, char *buffer);

	bool VerifyContainerSlotBoundary(int container, int slot);
	int VerifyEquipItem(ItemDef *itemDef, int destSlot, int charLevel, int charProfession);
	static const char * GetEqErrorString(int code);
};


int CheckSection_Inventory(FileReader &fr, InventoryManager &cd, const char *debugFilename, const char *debugName, const char *debugType);
int AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
int RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
int PrepExt_TradeItemOffer(char *buffer, char *convBuf, int offeringPlayerID, std::vector<InventorySlot>& itemList);

#endif //INVENTORY_H
