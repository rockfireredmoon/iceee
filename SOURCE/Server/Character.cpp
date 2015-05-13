#include <vector>
#include <algorithm>   //For sort
#include "Character.h"
#include "FileReader.h"
#include "Config.h"       //For global variables holding default positions
#include "StringList.h"   //To report errors
#include "Item.h"         //Need to check EQ and INV item lists as they're being loaded
#include "ItemSet.h"
#include "Quest.h"
#include <limits>
#include "Account.h"  //For upgrade conversion only.
#include "DebugTracer.h"
#include "Ability2.h"
#include "Util.h"
#include "Debug.h"
#include "Globals.h"
#include "InstanceScale.h"


const int SERVER_CHARACTER_VERSION = 1;

//vector<CharacterData> CharList;
CharacterManager g_CharacterManager;

AbilityContainer :: AbilityContainer()
{
}

AbilityContainer :: ~AbilityContainer()
{
	AbilityList.clear();
}

int AbilityContainer :: GetAbilityIndex(int value)
{
	int a;
	for(a = 0; a < (int)AbilityList.size(); a++)
		if(AbilityList[a] == value)
			return a;

	return -1;
}

int AbilityContainer :: AddAbility(int value)
{
	int r = GetAbilityIndex(value);
	if(r == -1)
	{
		AbilityList.push_back(value);
		r = AbilityList.size() - 1;
	}
	return r;
}

IntContainer :: IntContainer()
{
}

IntContainer :: ~IntContainer()
{
	List.clear();
}

int IntContainer :: GetIndex(int value)
{
	int a;
	for(a = 0; a < (int)List.size(); a++)
		if(List[a] == value)
			return a;

	return -1;
}

int IntContainer :: Add(int value)
{
	int r = GetIndex(value);
	if(r == -1)
	{
		List.push_back(value);
		r = List.size() - 1;
	}
	return r;
}

int IntContainer :: Remove(int value)
{
	int r = GetIndex(value);
	if(r >= 0)
	{
		List.erase(List.begin() + r);
		return 1;
	}

	return 0;
}

void IntContainer :: Clear(void)
{
	List.clear();
}

PreferenceContainer :: PreferenceContainer()
{
}

PreferenceContainer :: ~PreferenceContainer()
{
	PrefList.clear();
}

int PreferenceContainer :: GetPrefIndex(const char *name)
{
	for(size_t a = 0; a < PrefList.size(); a++)
		if(PrefList[a].name.compare(name) == 0)
			return a;

	return -1;
}

const char * PreferenceContainer :: GetPrefValue(const char *name)
{
	int r = GetPrefIndex(name);
	if(r >= 0)
		return PrefList[r].value.c_str();

	return NULL;
}

int PreferenceContainer :: SetPref(const char *name, const char *value)
{
	int r = GetPrefIndex(name);
	if(r >= 0)
	{
		PrefList[r].value = value;
	}
	else
	{
		_PreferencePair newPref;
		newPref.name = name;
		newPref.value = value;
		PrefList.push_back(newPref);
		r = PrefList.size() - 1;
	}

	return r;
}

int InventorySlot :: GetStackCount(void)
{
	ItemDef *itemPtr = g_ItemManager.GetPointerByID(IID);
	//TODO: Storage assignment should go here to prevent repeated lookups.
	if(itemPtr != NULL)
	{
		if(itemPtr->mIvType1 == ItemIntegerType::STACKING)
			return count + 1;
	}
	else
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] GetStackCount() failed item lookup for ID [%d]", IID);
	}

	return 1;
}

int InventorySlot :: GetMaxStack(void)
{
	//Return the maximum number of items that may be stacked, or -1 if
	//the item cannot be stacked.

	ItemDef *itemPtr = g_ItemManager.GetPointerByID(IID);
	//TODO: Storage assignment should go here to prevent repeated lookups.
	if(itemPtr != NULL)
	{
		if(itemPtr->mIvType1 == ItemIntegerType::STACKING)
			return itemPtr->mIvMax1;
	}
	return -1;
}

void InventorySlot :: CopyFrom(InventorySlot &source, bool copyCCSID)
{
	count = source.count;
	IID = source.IID;
	dataPtr = source.dataPtr;
	customLook = source.customLook;
	if(copyCCSID == true)
		CCSID = source.CCSID;
	bindStatus = source.bindStatus;
	secondsRemaining = source.secondsRemaining;
}

// Trading needed a new copy function because it would overwrite previous stack counts.
void InventorySlot :: CopyWithoutCount(InventorySlot &source, bool copyCCSID)
{
	IID = source.IID;
	dataPtr = source.dataPtr;
	customLook = source.customLook;
	if(copyCCSID == true)
		CCSID = source.CCSID;
	bindStatus = source.bindStatus;
	secondsRemaining = source.secondsRemaining;
}

ItemDef * InventorySlot :: ResolveItemPtr(void)
{
	if(dataPtr == NULL)
		dataPtr = g_ItemManager.GetPointerByID(IID);
	return dataPtr;
}

ItemDef * InventorySlot :: ResolveSafeItemPtr(void)
{
	if(dataPtr == NULL)
	{
		dataPtr = g_ItemManager.GetPointerByID(IID);
		if(dataPtr == NULL)
		{
			g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] ResolveSafeItemPtr() ID not found: %d", IID);
			dataPtr = g_ItemManager.GetDefaultItemPtr();
		}
	}
	if(dataPtr == NULL)
	{
		fprintf(stderr, "ResolveSafeItemPtr() returning NULL\r\n");
		fflush(stderr);
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] ResolveSafeItemPtr() returning with NULL");
	}
	return dataPtr;
}

int InventorySlot :: GetLookID(void)
{
	if(customLook != 0)
		return customLook;
	return IID;
}

unsigned short InventorySlot :: GetContainer(void)
{
	return (CCSID & CONTAINER_ID) >> 16;
}

unsigned short InventorySlot :: GetSlot(void)
{
	return (CCSID & CONTAINER_SLOT);
}

void InventorySlot :: SaveToAccountStream(const char *containerName, FILE *output)
{
	fprintf(output, "%s=%d,%d,%d,%d,%d\r\n", containerName, CCSID, IID, count, customLook, bindStatus);
}

bool InventorySlot :: TestEquivalent(InventorySlot &other)
{
	if(CCSID != other.CCSID) return false;
	if(IID != other.IID) return false;
	if(count != other.count) return false;
	if(customLook != other.customLook) return false;
	if(bindStatus != other.bindStatus) return false;
	return true;
}

void InventorySlot :: ApplyItemIntegerType(int IvType, int IvMax)
{
	switch(IvType)
	{
	case ItemIntegerType::LIFETIME:   //IvMax is number of hours until the item expires.
		secondsRemaining = IvMax * 60 * 60;
		break;
	}
}
int InventorySlot :: GetTimeRemaining(void)
{
	if(secondsRemaining > 0)
		return secondsRemaining;

	return -1;
}

bool InventorySlot :: VerifyItemExist(void)
{
	ItemDef *itemDef = g_ItemManager.GetPointerByID(IID);
	if(itemDef == NULL)
		return false;

	return true;
}

InventoryManager :: InventoryManager()
{
	LastError = ERROR_NONE;
	buybackSlot = 0;
	memset(MaxContainerSlot, 0, sizeof(MaxContainerSlot));
}

InventoryManager :: ~InventoryManager()
{
	ClearAll();
}

int InventoryManager :: AddItem(int containerID, InventorySlot &item)
{
	if(containerID >= 0 && containerID <= MAXCONTAINER)
	{
		//Hack to fix item counts for those that need them.
		/*
		if(item.IIndex > 0)
		{
			if(ItemList[item.IIndex].mIvType1 == ItemIntegerType::STACKING)
				if(item.count == 0)
					item.count = 1;
		}*/
		containerList[containerID].push_back(item);
		return (int)containerList[containerID].size() - 1;
	}
	return -1;
}

void InventoryManager :: SetError(int value)
{
	LastError = value;
}

InventorySlot * InventoryManager :: GetExistingPartialStack(int containerID, ItemDef *itemDef)
{
	//Try to find a stack of an existing stackable item that isn't at full
	//capacity (at least one below max).
	if(itemDef->mIvType1 != ItemIntegerType::STACKING)
		return NULL;

	for(unsigned int a = 0; a < containerList[containerID].size(); a++)
	{
		InventorySlot *slot = &containerList[containerID][a];
		if(slot->IID == itemDef->mID)
		{
			if(slot->GetStackCount() < itemDef->mIvMax1)
				return slot;
		}
	}
	return NULL;
}

InventorySlot * InventoryManager :: AddItem_Ex(int containerID, int itemID, int count)
{
	if(count == 0)
		g_Log.AddMessageFormat("[WARNING] AddItem_Ex() count is zero");

	ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
	if(itemDef == NULL)
	{
		g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] Item [%d] does not exist.", itemID);
		SetError(ERROR_ITEM);
		return NULL;
	}

	//If count is 1, try to increment an existing stack if necessary.
	InventorySlot *slot = GetExistingPartialStack(containerID, itemDef);
	if(slot != NULL)
	{
		if(slot->GetStackCount() + count <= slot->GetMaxStack())
		{
			slot->count += count;
			return slot;
		}
	}

	int freeSlot = GetFreeSlot(containerID);
	if(freeSlot == -1)
	{
		SetError(ERROR_SPACE);
		return NULL;
	}

	InventorySlot newItem;
	newItem.CCSID = GetCCSID(containerID, freeSlot);
	if(count > 1)
		newItem.count = count - 1;
	else
		newItem.count = 0;
	newItem.IID = itemID;
	newItem.dataPtr = itemDef;
	if(itemDef->mBindingType == BIND_ON_PICKUP)
		newItem.bindStatus = 1;
	newItem.ApplyItemIntegerType(itemDef->mIvType1, itemDef->mIvMax1);
	newItem.ApplyItemIntegerType(itemDef->mIvType2, itemDef->mIvMax2);

	AddItem(containerID, newItem);
	return &containerList[containerID].back();
}

int InventoryManager :: FindNextItem(int containerID, int itemID, int start)
{
	//Return the index of the item that next matches the given item ID.

	//We could use an unsigned int, but the calling function is probably
	//assigning the next search index to the return value, which can be
	//negative.
	if(start < 0)
		return -1;

	int size = containerList[containerID].size();
	if(start >= size)
		return -1;

	for(int a = start; a < size; a++)
	{
		if(containerList[containerID][a].IID == itemID)
			return a;
	}
	return -1;
}

int InventoryManager :: ScanRemoveItems(int containerID, int itemID, int count, vector<InventoryQuery> &resultList)
{
	if(resultList.size() > 0)
		resultList.clear();

	//Scans for items that would be removed or modified.  Fills a list with
	//indexes to those items.
	int remain = count;
	int index = 0;
	InventoryQuery iq;
	while(remain > 0 && index >= 0)
	{
		index = FindNextItem(containerID, itemID, index);
		if(index >= 0)
		{
			InventorySlot *slot = &containerList[containerID][index];

			int stackCount = slot->GetStackCount();
			if(remain >= stackCount)
			{
				//Not enough to fulfill the remove count requirements.
				//This stack will be flagged for removal.
				iq.type = iq.TYPE_REMOVE;
				iq.count = stackCount;
			}
			else
			{
				iq.type = iq.TYPE_MODIFIED;
				iq.count = remain;
			}
			iq.CCSID = slot->CCSID;
			iq.ptr = slot;
			resultList.push_back(iq);
			remain -= stackCount;
			index++; //Advance the starting index
		}
	}
	return resultList.size();
}

int InventoryManager :: RemoveItemsAndUpdate(int container, int itemID, int itemCost, char *packetBuffer)
{
	int wpos = 0;
	char ConvBuf[64];
	vector<InventoryQuery> iq;
	ScanRemoveItems(container, itemID, itemCost, iq);
	for(size_t i = 0; i < iq.size(); i++)
	{
		if(iq[i].type == InventoryQuery::TYPE_REMOVE)
			wpos += RemoveItemUpdate(&packetBuffer[wpos], ConvBuf, iq[i].ptr);
		else if(iq[i].type == InventoryQuery::TYPE_MODIFIED)
		{
			iq[i].ptr->count -= iq[i].count;
			wpos += AddItemUpdate(&packetBuffer[wpos], ConvBuf, iq[i].ptr);
		}
	}
	RemoveItems(INV_CONTAINER, iq);
	return wpos;
}

int InventoryManager :: RemoveItems(int containerID, vector<InventoryQuery> &resultList)
{
	for(size_t i = 0; i < resultList.size(); i++)
	{
		if(resultList[i].type == InventoryQuery::TYPE_REMOVE)
		{
			RemItem(resultList[i].CCSID);
		}
	}
	return 0;
}

int InventoryManager :: RemItem(unsigned int editCCSID)
{
	int containerID = (editCCSID & CONTAINER_ID) >> 16;
	int a;
	for(a = 0; a < (int)containerList[containerID].size(); a++)
	{
		if(containerList[containerID][a].CCSID == editCCSID)
		{
			containerList[containerID].erase(containerList[containerID].begin() + a);
			return 1;
		}
	}
	return 0;
}

int InventoryManager :: GetItemByCCSID(unsigned int findCCSID)
{
	int containerID = (findCCSID & CONTAINER_ID) >> 16;
	//Attempt to find an item in the list matching the given CCSID
	for(size_t i = 0; i < containerList[containerID].size(); i++)
		if(containerList[containerID][i].CCSID == findCCSID)
			return i;
	return -1;
}

InventorySlot * InventoryManager :: GetItemPtrByCCSID(unsigned int findCCSID)
{
	//Added because refashioning needed a better way to retrieve item data.
	int containerID = (findCCSID & CONTAINER_ID) >> 16;
	for(size_t i = 0; i < containerList[containerID].size(); i++)
		if(containerList[containerID][i].CCSID == findCCSID)
			return &containerList[containerID][i];
	return NULL;
}

InventorySlot * InventoryManager :: GetItemPtrByID(int itemID)
{
	//Just search backpack inventory for now.
	for(size_t i = 0; i < containerList[INV_CONTAINER].size(); i++)
	{
		if(containerList[INV_CONTAINER][i].IID == itemID)
			return &containerList[INV_CONTAINER][i];
	}
	return NULL;
}

int InventoryManager :: GetItemBySlot(int containerID, uint slot)
{
	if(slot < 0)
		return -1;
	if(containerID >= MAXCONTAINER)
		return -1;

	for(size_t i = 0; i < containerList[containerID].size(); i++)
		if((containerList[containerID][i].CCSID & CONTAINER_SLOT) == slot)
			return i;
	return -1;
}

void InventoryManager :: ClearAll(void)
{
	int a;
	for(a = 0; a < MAXCONTAINER; a++)
		containerList[a].clear();
}

unsigned int InventoryManager :: GetCCSID(unsigned short container, unsigned short slot)
{
	return (container << 16) | slot;
}

unsigned int InventoryManager :: GetCCSIDFromHexID(const char *hexStr)
{
	if(hexStr == NULL)
		return 0;

	return static_cast<unsigned int>(strtol(hexStr, NULL, 16));
}


int InventoryManager :: GetFreeSlot(int containerID)
{
	int size = containerList[containerID].size();
	if(size >= MaxContainerSlot[containerID])
		return -1;

	vector<bool> tempCon;
	tempCon.resize(MaxContainerSlot[containerID]);
	int a;
	for(a = 0; a < size; a++)
	{
		int slot = containerList[containerID][a].CCSID & CONTAINER_SLOT;
		if(slot < MaxContainerSlot[containerID])
			tempCon[slot] = true;
	}

	for(a = 0; a < MaxContainerSlot[containerID]; a++)
		if(tempCon[a] == false)
			return a;

	return -1;
}

int InventoryManager :: CountFreeSlots(int containerID)
{
	int size = containerList[containerID].size();
	if(size >= MaxContainerSlot[containerID])
		return 0;

	vector<bool> tempCon;
	tempCon.resize(MaxContainerSlot[containerID]);
	int a;
	for(a = 0; a < size; a++)
	{
		int slot = containerList[containerID][a].CCSID & CONTAINER_SLOT;
		if(slot < MaxContainerSlot[containerID])
			tempCon[slot] = true;
	}

	int count = 0;
	for(a = 0; a < MaxContainerSlot[containerID]; a++)
		if(tempCon[a] == false)
			count++;

	return count;
}

void InventoryManager :: CountInventorySlots(void)
{
	//Scan through the items in the EQ container for all items with a
	//container type ID.  Count the slots from the equipped containers,
	//then update the final slot count.

	int containerID = EQ_CONTAINER;
	int maxSlot = INV_BASESLOTS;
	for(size_t a = 0; a < containerList[containerID].size(); a++)
	{
		ItemDef *itemDef = containerList[containerID][a].ResolveItemPtr();
		if(itemDef != NULL)
		{
			if(itemDef->mType == ItemType::CONTAINER)
			{
				maxSlot += itemDef->mContainerSlots;
			}
		}
	}
	MaxContainerSlot[INV_CONTAINER] = maxSlot;
}

int InventoryManager :: GetItemCount(int containerID, int itemID)
{
	//Search all objects in this container, returning the total number of items
	//that match the given item ID.  This includes item stacks.
	int rcount = 0;
	for(size_t a = 0; a < containerList[containerID].size(); a++)
	{
		InventorySlot &item = containerList[containerID][a];
		if(item.IID == itemID)
		{
			rcount++;
			ItemDef *itemDef = item.ResolveItemPtr();
			if(itemDef != NULL)
			{
				//Single items don't have a count.
				//  Stack Count = 1 + item.count;
				if(itemDef->mIvType1 == ItemIntegerType::STACKING)
					rcount += item.count;
			}
		}
	}
	return rcount;
}

InventorySlot * InventoryManager :: GetFirstItem(int containerID, int itemID)
{
	// Quick way to get the pointer of the first item.  Added this function
	// for crafting, as a way to retrieve single items from the inventory
	// (disregarding stack size) for deletion.
	for(size_t a = 0; a < containerList[containerID].size(); a++)
		if(containerList[containerID][a].IID == itemID)
			return &containerList[containerID][a];
	return NULL;
}

int InventoryManager :: ItemMove(char *buffer, char *convBuf, CharacterStatSet *css, bool localCharacterVault, int origContainer, int origSlot, int destContainer, int destSlot)
{
	int wpos = 0;
	int origItemIndex = GetItemBySlot(origContainer, origSlot);
	if(origItemIndex == -1)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] ItemMove: item not found (container: %d, slot: %d)", origContainer, origSlot);
		return 0;
	}

	int destItemIndex = GetItemBySlot(destContainer, destSlot);

	//Don't move an item onto itself.  This was causing stacked items to be deleted.
	if(origContainer == destContainer && origSlot == destSlot)
		return -3;

	//Two hacks, one to prevent equipping a new twohand weapon with an existing offhand,
	//and to prevent equipping an offhand with an existing twohand.
	if(destContainer == EQ_CONTAINER && destSlot == ItemEquipSlot::WEAPON_MAIN_HAND)
	{
		InventorySlot &source = containerList[origContainer][origItemIndex];
		ItemDef *sourceItem = source.ResolveSafeItemPtr();
		if(ItemManager::IsWeaponTwoHanded(sourceItem->mEquipType, sourceItem->mWeaponType) == true)
			if(GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_OFF_HAND) >= 0)
				return -1;
	}
	if(destContainer == EQ_CONTAINER && destSlot == ItemEquipSlot::WEAPON_OFF_HAND)
	{
		int index = GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_MAIN_HAND);
		if(index >= 0)
		{
			ItemDef *destItem = containerList[EQ_CONTAINER][index].ResolveSafeItemPtr();
			if(ItemManager::IsWeaponTwoHanded(destItem->mEquipType, destItem->mWeaponType) == true)
				return -2;
		}
	}
	//Repeat for reverse.
	if((origContainer == EQ_CONTAINER) && (origSlot == ItemEquipSlot::WEAPON_MAIN_HAND) && (destItemIndex >= 0))
	{
		InventorySlot &dest = containerList[destContainer][destItemIndex];
		ItemDef *check = dest.ResolveSafeItemPtr();
		if(ItemManager::IsWeaponTwoHanded(check->mEquipType, check->mWeaponType) == true)
			if(GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_OFF_HAND) >= 0)
				return -1;
	}
	if(origContainer == EQ_CONTAINER && origSlot == ItemEquipSlot::WEAPON_OFF_HAND)
	{
		int index = GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_MAIN_HAND);
		if(index >= 0)
		{
			ItemDef *destItem = containerList[EQ_CONTAINER][index].ResolveSafeItemPtr();
			if(ItemManager::IsWeaponTwoHanded(destItem->mEquipType, destItem->mWeaponType) == true)
				return -2;
		}
	}


	if(destContainer == BANK_CONTAINER && localCharacterVault == false)
	{
		if(containerList[origContainer][origItemIndex].bindStatus != 0)
			return -4;
	}

	//If something is in the destination slot, swap the objects.
	//Just swap the data in memory.
	//No need to add or erase slots.
	if(destItemIndex >= 0)
	{
		InventorySlot &source = containerList[origContainer][origItemIndex];
		InventorySlot &dest = containerList[destContainer][destItemIndex];

		//HACK: Very special case! When swapping the item in the destination slot into the equipment
		//slot, it must be verified for equipment restriction checks.
		if(origContainer == EQ_CONTAINER)
		{
			ItemDef *destItem = dest.ResolveItemPtr();
			if(destItem == NULL)
				return 0;
			int res = VerifyEquipItem(destItem, origSlot, css->level, css->profession);
			if(res != EQ_ERROR_NONE)
				return -100 + res;  //errors are negative, but we want the result to be -100 or lower (-101, -102 etc)
		}

		//Check to see if they're stackable, and the same object.
		if(source.IID == dest.IID)
		{
			int dmax = source.GetMaxStack();
			if(dmax >= 0)
			{
				//They can be stacked, try to do so.
				int scount = source.GetStackCount();
				int dcount = dest.GetStackCount();
				int emptyStackSpace = dmax - dcount;
				if(emptyStackSpace >= scount)
				{
					//Add entire count of source to destination.
					//Remove source.
					dest.count += scount;
					wpos += AddItemUpdate(&buffer[wpos], convBuf, &dest);
					wpos += RemoveItemUpdate(&buffer[wpos], convBuf, &source);
					g_Log.AddMessageFormat("[ITEMMOVE] Stack Erasing: %d", source.IID);
					containerList[origContainer].erase(containerList[origContainer].begin() + origItemIndex);
				}
				else
				{
					//Add to count of destination.
					//Subtract count of source.
					dest.count += emptyStackSpace;
					source.count -= emptyStackSpace;
					wpos += AddItemUpdate(&buffer[wpos], convBuf, &source);
					wpos += AddItemUpdate(&buffer[wpos], convBuf, &dest);
				}

				//We're done, nothing left to process.
				return wpos;
			}
		}

		//Order of operations for item.move swap:
		//remove item in source slot
		//remove item in dest slot
		//add item to source slot
		//add item to dest slot
		wpos += RemoveItemUpdate(&buffer[wpos], convBuf, &source);
		wpos += RemoveItemUpdate(&buffer[wpos], convBuf, &dest);

		InventorySlot temp;
		temp.CopyFrom(dest, false);
		dest.CopyFrom(source, false);
		source.CopyFrom(temp, false);

		wpos += AddItemUpdate(&buffer[wpos], convBuf, &dest);
		wpos += AddItemUpdate(&buffer[wpos], convBuf, &source);
		return wpos;
	}

	//Check if moving among a single container.  If so, just update the CCSID.
	//No need to add or erase slots.
	if(destContainer == origContainer)
	{
		InventorySlot temp;
		InventorySlot &source = containerList[origContainer][origItemIndex];
		wpos += RemoveItemUpdate(&buffer[wpos], convBuf, &source);
		temp.CopyFrom(source, true);
		source.CCSID = GetCCSID(destContainer, destSlot);
		wpos += AddItemUpdate(&buffer[wpos], convBuf, &source);
		//wpos = SendItemIDUpdate(&buffer[wpos], convBuf, &temp, &source);
		return wpos;
	}

	//If we get to this point, the items are in different containers.
	//Add to destination, delete origin.
	InventorySlot &source = containerList[origContainer][origItemIndex];
	InventorySlot dest;
	dest.CopyFrom(source, false);
	dest.CCSID = GetCCSID(destContainer, destSlot);
	wpos += RemoveItemUpdate(&buffer[wpos], convBuf, &source);
	wpos += AddItemUpdate(&buffer[wpos], convBuf, &dest);
	containerList[destContainer].push_back(dest);
	g_Log.AddMessageFormat("[ITEMMOVE] Erasing: %d (new dest: %d)", source.IID, dest.IID);
	containerList[origContainer].erase(containerList[origContainer].begin() + origItemIndex);
	return wpos;
}

int InventoryManager :: AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 70);       //_handleItemUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size

	//LogMessageL("  Adding: %d", slot->CCSID);
	sprintf(convBuf, "%X", slot->CCSID);
	wpos += PutStringUTF(&buffer[wpos], convBuf);  //itemID
	//wpos += PutByte(&buffer[wpos], ITEM_DEF | ITEM_LOOK_DEF | ITEM_CONTAINER);       //mask   zero = delete item?
	wpos += PutByte(&buffer[wpos], 255);       //mask
	wpos += PutByte(&buffer[wpos], 255);       //flags

	//for ITEM_DEF
	wpos += PutInteger(&buffer[wpos], slot->IID);

	//for ITEM_LOOK_DEF
	wpos += PutInteger(&buffer[wpos], slot->customLook);

	//for (MASK & ITEM_IV1)
	wpos += PutShort(&buffer[wpos], slot->count);
	//for (MASK & ITEM_IV2)
	wpos += PutShort(&buffer[wpos], 0);

	//if (flags & FLAG_ITEM_BOUND)
	wpos += PutByte(&buffer[wpos], slot->bindStatus);

	//if (flags & FLAG_ITEM_TIME_REMAINING)
	wpos += PutInteger(&buffer[wpos], -1);   //timeRemaining

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}

int InventoryManager :: RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 70);       //_handleItemUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size

	sprintf(convBuf, "%X", slot->CCSID);
	wpos += PutStringUTF(&buffer[wpos], convBuf);  //itemID
	wpos += PutByte(&buffer[wpos], 0);       //mask   zero = delete item
	wpos += PutByte(&buffer[wpos], 0);       //flags

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}

int InventoryManager :: SendItemIDUpdate(char *buffer, char *convBuf, InventorySlot *oldSlot, InventorySlot *newSlot)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 70);       //_handleItemUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size

	sprintf(convBuf, "%X", oldSlot->CCSID);
	//sprintf(convBuf, "%X", newSlot->CCSID);
	wpos += PutStringUTF(&buffer[wpos], convBuf);  //itemID
	wpos += PutByte(&buffer[wpos], ITEM_ID_CHANGE);
	wpos += PutByte(&buffer[wpos], 0);       //flags

	sprintf(convBuf, "%X", newSlot->CCSID);
	//sprintf(convBuf, "%X", oldSlot->CCSID);
	wpos += PutStringUTF(&buffer[wpos], convBuf);  //itemID

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
}

void InventoryManager :: FixBuyBack(void)
{
	//To be called when a character has been loaded.
	//Make sure the buyback slots in use don't exceed the maximum allowed server
	//capacity.  Remove the oldest items if necessary.  Reset the ID order
	//and update the highest buyback slot.

	/*
	//Array index order: [newest] ... [oldest]
	while((int)containerList[BUYBACK_CONTAINER].size() > g_Config.BuybackLimit)
		containerList[BUYBACK_CONTAINER].pop_back();
	*/
	
	//Array index order: [oldest] ... [newest]
	int difference = (int)containerList[BUYBACK_CONTAINER].size() - g_Config.BuybackLimit;
	if(difference > 0)
		containerList[BUYBACK_CONTAINER].erase(containerList[BUYBACK_CONTAINER].begin(), containerList[BUYBACK_CONTAINER].begin() + difference);

	//New items that are added in-game will have the highest slot number.
	//However, when the client asks for the "item.contents" query, the lowest
	//slot number will appear first (top left shop panel)
	size_t len = containerList[BUYBACK_CONTAINER].size();
	for(size_t i = 0; i < len; i++)
		containerList[BUYBACK_CONTAINER][i].CCSID = GetCCSID(BUYBACK_CONTAINER, i);

	//New items added in gameplay will use this incremental counter.
	buybackSlot = len;
}

int InventoryManager :: AddBuyBack(InventorySlot *item, char *buffer)
{
	//Add a new item to the buyback list.  If the buyback list is full, pop
	//the oldest element off the list.

	int wpos = 0;
	char convbuf[32];

	/*
	//Array index order: [newest] ... [oldest]
	if((int)containerList[BUYBACK_CONTAINER].size() >= g_Config.BuybackLimit)
	{
		wpos += RemoveItemUpdate(&buffer[wpos], convbuf, &containerList[BUYBACK_CONTAINER].back());
		containerList[BUYBACK_CONTAINER].pop_back();
	}
	*/
	//Array index order: [oldest] ... [newest]
	if((int)containerList[BUYBACK_CONTAINER].size() >= g_Config.BuybackLimit)
	{
		wpos += RemoveItemUpdate(&buffer[wpos], convbuf, &containerList[BUYBACK_CONTAINER][0]);
		g_ItemManager.NotifyDestroy(containerList[BUYBACK_CONTAINER][0].IID, "addBuyBack");
		containerList[BUYBACK_CONTAINER].erase(containerList[BUYBACK_CONTAINER].begin());
	}


	//Note: the slot number is arbitrary and will be continously incremented.
	//The client seems to auto organize the slots, so it's only used for purposes
	//of unique identification.
	InventorySlot tempItem;
	tempItem.CopyFrom(*item, false);
	tempItem.CCSID = GetCCSID(BUYBACK_CONTAINER, buybackSlot++);
	
	//Array index order: [newest] ... [oldest]
	//containerList[BUYBACK_CONTAINER].insert(containerList[BUYBACK_CONTAINER].begin(), tempItem);

	//Array index order: [oldest] ... [newest]
	containerList[BUYBACK_CONTAINER].push_back(tempItem);

	wpos += AddItemUpdate(&buffer[wpos], convbuf, &tempItem);
	return wpos;
}

/* Determine whether the given container and slot fit within the valid container array or slot
boundaries. */
bool InventoryManager :: VerifyContainerSlotBoundary(int container, int slot)
{
	if(container < 0 || container >= MAXCONTAINER)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] VerifyContainerSlotBoundary: invalid container: %d", container);
		return false;
	}
	if(slot < 0)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] VerifyContainerSlotBoundary: invalid slot: %d", slot);
		return false;
	}
	if(container == INV_CONTAINER && slot >= MaxContainerSlot[container])
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] VerifyContainerSlotBoundary: slot is too high: %d", slot);
		return false;
	}
	return true;
}

/* Determine whether the item is capable of being equipped in a particular slot, if that slot were
empty.  Assumes that the slot index is a valid equipped container slot.  For weapons, the existence
of conflicting offhand equipment must be performed separately.
Note the return error codes, the calling function may need to use them to determine an error message
to send to the client.  */
int InventoryManager :: VerifyEquipItem(ItemDef *itemDef, int destSlot, int charLevel, int charProfession)
{
	if(itemDef == NULL)
		return EQ_ERROR_ITEM;

	if(itemDef->mMinUseLevel > charLevel)
		return EQ_ERROR_LEVEL;
	
	if(itemDef->mWeaponType != WeaponType::NONE)
		if(WeaponTypeClassRestrictions::isValid(itemDef->mWeaponType, charProfession) == false)
			return EQ_ERROR_PROFESSION;

	int targetEquipSlot = 0;
	switch(itemDef->mEquipType)
	{
	case ItemEquipType::NONE: targetEquipSlot = ItemEquipSlot::NONE; break;
	case ItemEquipType::WEAPON_1H:
	case ItemEquipType::WEAPON_1H_UNIQUE:
	case ItemEquipType::WEAPON_1H_MAIN:
	case ItemEquipType::WEAPON_1H_OFF:
		switch(itemDef->mWeaponType)
		{
		case WeaponType::ONE_HAND:
		case WeaponType::SMALL:
		case WeaponType::ARCANE_TOTEM:
			targetEquipSlot = destSlot;
			break;
		default:
			targetEquipSlot = 0;
			break;
		}
		break;
	case ItemEquipType::WEAPON_2H:
		//targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND; break;
		switch(itemDef->mWeaponType)
		{
		case WeaponType::POLE:
		case WeaponType::TWO_HAND:
			targetEquipSlot = destSlot;
			break;
		default:
			targetEquipSlot = 0;
			break;
		}
		break;
	case ItemEquipType::WEAPON_RANGED:
		//targetEquipSlot = ItemEquipSlot::WEAPON_RANGED; break;
		switch(itemDef->mWeaponType)
		{
		case WeaponType::THROWN:
		case WeaponType::WAND:
		case WeaponType::BOW:
			targetEquipSlot = destSlot;
			break;
		default:
			targetEquipSlot = 0;
			break;
		}
		break;
	case ItemEquipType::RED_CHARM: targetEquipSlot = ItemEquipSlot::RED_CHARM; break;
	case ItemEquipType::BLUE_CHARM: targetEquipSlot = ItemEquipSlot::RED_CHARM; break;
	case ItemEquipType::PURPLE_CHARM: targetEquipSlot = ItemEquipSlot::PURPLE_CHARM; break;
	case ItemEquipType::YELLOW_CHARM: targetEquipSlot = ItemEquipSlot::YELLOW_CHARM; break;
	case ItemEquipType::GREEN_CHARM: targetEquipSlot = ItemEquipSlot::GREEN_CHARM; break;
	case ItemEquipType::ORANGE_CHARM: targetEquipSlot = ItemEquipSlot::ORANGE_CHARM; break;
	case ItemEquipType::ARMOR_SHIELD: targetEquipSlot = ItemEquipSlot::WEAPON_OFF_HAND; break;
	case ItemEquipType::ARMOR_HEAD: targetEquipSlot = ItemEquipSlot::ARMOR_HEAD; break;
	case ItemEquipType::ARMOR_NECK: targetEquipSlot = ItemEquipSlot::ARMOR_NECK; break;
	case ItemEquipType::ARMOR_SHOULDER: targetEquipSlot = ItemEquipSlot::ARMOR_SHOULDER; break;
	case ItemEquipType::ARMOR_CHEST: targetEquipSlot = ItemEquipSlot::ARMOR_CHEST; break;
	case ItemEquipType::ARMOR_ARMS: targetEquipSlot = ItemEquipSlot::ARMOR_ARMS; break;
	case ItemEquipType::ARMOR_HANDS: targetEquipSlot = ItemEquipSlot::ARMOR_HANDS; break;
	case ItemEquipType::ARMOR_WAIST: targetEquipSlot = ItemEquipSlot::ARMOR_WAIST; break;
	case ItemEquipType::ARMOR_LEGS: targetEquipSlot = ItemEquipSlot::ARMOR_LEGS; break;
	case ItemEquipType::ARMOR_FEET: targetEquipSlot = ItemEquipSlot::ARMOR_FEET; break;
	case ItemEquipType::ARMOR_RING: targetEquipSlot = destSlot; break;
	case ItemEquipType::ARMOR_RING_UNIQUE: targetEquipSlot = destSlot; break;
	case ItemEquipType::CONTAINER: targetEquipSlot = destSlot; break;
	//case ItemEquipType::ARMOR_RING: targetEquipSlot = ItemEquipSlot::ARMOR_RING_L; break;
	//case ItemEquipType::ARMOR_RING_UNIQUE: targetEquipSlot = ItemEquipSlot::ARMOR_RING_L; break;
	case ItemEquipType::ARMOR_AMULET: targetEquipSlot = ItemEquipSlot::ARMOR_AMULET; break;
	default: 
		targetEquipSlot = 0;
	}
	if(targetEquipSlot != destSlot)
	{
		return EQ_ERROR_SLOT;
	}

	return EQ_ERROR_NONE;
}

const char * InventoryManager :: GetEqErrorString(int code)
{
	switch(code)
	{
	case InventoryManager::EQ_ERROR_ITEM: return "The item does not exist in the server database.";
	case InventoryManager::EQ_ERROR_LEVEL: return "Your level is not high enough to equip that item.";
	case InventoryManager::EQ_ERROR_PROFESSION: return "That item cannot be equipped by your class.";
	case InventoryManager::EQ_ERROR_SLOT: return "That item cannot be equipped in that slot.";
	}
	return NULL;
}

CharacterData :: CharacterData()
{
	ClearAll();
}

CharacterData :: ~CharacterData()
{
	abilityList.AbilityList.clear();
	preferenceList.PrefList.clear();
	SidekickList.clear();
	originalAppearance.clear();
}

void CharacterData :: ClearAll(void)
{
	AccountID = 0;
	characterVersion = 0;
	pendingChanges = 0;
	expireTime = 0;

	cdef.css.Clear();
	memset(&activeData, 0, sizeof(activeData));
	StatusText.clear();
	PrivateChannelName.clear();
	PrivateChannelPassword.clear();
	abilityList.AbilityList.clear();
	preferenceList.PrefList.clear();
	friendList.clear();
	guildList.clear();
	SidekickList.clear();
	MaxSidekicks = MAX_SIDEKICK;

	//eq.slotList.clear();
	//inv.slotList.clear();
	inventory.ClearAll();
	localCharacterVault = true;

	hengeList.clear();

	SecondsLogged = 0;
	SessionsLogged = 0;
	memset(&TimeLogged, 0, sizeof(TimeLogged));
	memset(&LastSession, 0, sizeof(LastSession));
	memset(&LastLogOn, 0, sizeof(LastLogOn));
	memset(&LastLogOff, 0, sizeof(LastLogOff));

	questJournal.Clear();

	InstanceScaler.clear();

	memset(groveReturnPoint, 0, sizeof(groveReturnPoint));
	memset(bindReturnPoint, 0, sizeof(bindReturnPoint));
	memset(pvpReturnPoint, 0, sizeof(pvpReturnPoint));
	LastWarpTime = 0;
	UnstickCount = 0;
	LastUnstickTime = 0;

	memset(PermissionSet, 0, sizeof(PermissionSet));

	CurrentVaultSize = 0;
	CreditsPurchased = 0;
	CreditsSpent = 0;
	ExtraAbilityPoints = 0;
}

void CharacterData :: CopyFrom(CharacterData &source)
{
	//Don't need to copy base stuff, so just set to zero
	activeData.CopyFrom(&source.activeData);
	cdef.Clear();

	MaxSidekicks = source.MaxSidekicks;

	//Assign the vector lists
	abilityList.AbilityList.assign(source.abilityList.AbilityList.begin(), source.abilityList.AbilityList.end());
	preferenceList.PrefList.assign(source.preferenceList.PrefList.begin(), source.preferenceList.PrefList.end());
}

void CharacterData :: EraseExpirationTime(void)
{
	expireTime = 0;
}

void CharacterData :: SetExpireTime(void)
{
	expireTime = g_ServerTime + CharacterManager::TEMP_EXPIRE_TIME;
}

void CharacterData :: ExtendExpireTime(void)
{
	if(expireTime != 0)
		SetExpireTime();
}

void CharacterData :: AddValour(int GuildDefID, int valour)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			guildList[i].Valour += valour;
			return;
		}
	}
}

int CharacterData :: GetValour(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			return guildList[i].Valour;
		}
	}
	return 0;
}

bool CharacterData :: IsInGuildAndHasValour(int GuildDefID, int valour) {
	for(size_t i = 0; i < guildList.size(); i++)
		if(guildList[i].GuildDefID == GuildDefID && guildList[i].Valour >= valour)
			return true;
	return false;
}

void CharacterData :: LeaveGuild(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++) {
		if(guildList[i].GuildDefID == GuildDefID) {
			guildList.erase(guildList.begin() + i);
			break;
		}
	}
	OnRankChange(0);
}

void CharacterData :: JoinGuild(int GuildDefID)
{
	for(size_t i = 0; i < guildList.size(); i++)
		if(guildList[i].GuildDefID == GuildDefID)
			return;

	GuildListObject newObject;
	newObject.GuildDefID = GuildDefID;
	guildList.push_back(newObject);
}

void CharacterData :: AddFriend(int CDefID, const char *name)
{
	for(size_t i = 0; i < friendList.size(); i++)
		if(friendList[i].CDefID == CDefID)
			return;

	FriendListObject newObject;
	newObject.CDefID = CDefID;
	newObject.Name = name;
	friendList.push_back(newObject);
}

int CharacterData :: RemoveFriend(const char *name)
{
	for(size_t i = 0; i < friendList.size(); i++)
	{
		if(friendList[i].Name.compare(name) == 0)
		{
			friendList.erase(friendList.begin() + i);
			return 1;
		}
	}
	return 0;
}

int CharacterData :: GetFriendIndex(int CDefID)
{
	for(size_t a = 0; a < friendList.size(); a++)
		if(friendList[a].CDefID == CDefID)
			return a;

	return -1;
}

void CharacterData :: UpdateBaseStats(CreatureInstance *destData, bool setStats)
{
	//if setStats is true, it communicates back the adjusted base stats to the
	//Creature Instance, allowing external functions to update buffs if necessary,
	//such as when a character levels.

	CharacterStatSet *css;
	if(destData != NULL)
		css = &destData->css;
	else
		css = &cdef.css;

	int level = css->level;
	if(level < 0)
		level = 0;
	else if(level > MAX_LEVEL)
		level = MAX_LEVEL;

	int prof = css->profession;
	if(prof < 0)
		prof = 0;
	else if(prof > MAX_PROFESSION)
		prof = MAX_PROFESSION;

	/*
	equipStat[Stat_Str][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Str]];
	equipStat[Stat_Dex][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Dex]];
	equipStat[Stat_Con][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Con]];
	equipStat[Stat_Psy][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Psy]];
	equipStat[Stat_Spi][Stat_Base] = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Spi]];
	sprintf(css->base_stats, "%d,%d,%d,%d,%d",
		equipStat[Stat_Str][Stat_Base],
		equipStat[Stat_Dex][Stat_Base],
		equipStat[Stat_Con][Stat_Base],
		equipStat[Stat_Psy][Stat_Base],
		equipStat[Stat_Spi][Stat_Base] );
	*/

	if(setStats == true)
	{
		/*
		css->strength = equipStat[Stat_Str][Stat_Base];
		css->dexterity = equipStat[Stat_Dex][Stat_Base];
		css->constitution = equipStat[Stat_Con][Stat_Base];
		css->psyche = equipStat[Stat_Psy][Stat_Base];
		css->spirit = equipStat[Stat_Spi][Stat_Base];
		*/
		short baseStr = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Str]];
		short baseDex = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Dex]];
		short baseCon = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Con]];
		short basePsy = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Psy]];
		short baseSpi = LevelBaseStats[level][ProfBaseStats[prof][BaseStatMap::Spi]];
		cdef.css.strength = baseStr;
		cdef.css.dexterity = baseDex;
		cdef.css.constitution = baseCon;
		cdef.css.psyche = basePsy;
		cdef.css.spirit = baseSpi;
		Util::SafeFormat(css->base_stats, sizeof(css->base_stats), "%d,%d,%d,%d,%d", baseStr, baseDex, baseCon, basePsy, baseSpi);

		if(destData != NULL)
		{
			if(destData->css.strength < baseStr) destData->css.strength = baseStr;
			if(destData->css.dexterity < baseDex) destData->css.dexterity = baseDex;
			if(destData->css.constitution < baseCon) destData->css.constitution = baseCon;
			if(destData->css.psyche < basePsy) destData->css.psyche = basePsy;
			if(destData->css.spirit < baseSpi) destData->css.spirit = baseSpi;

			destData->UpdateBaseStatMinimum(STAT::STRENGTH, baseStr);
			destData->UpdateBaseStatMinimum(STAT::DEXTERITY, baseDex);
			destData->UpdateBaseStatMinimum(STAT::CONSTITUTION, baseCon);
			destData->UpdateBaseStatMinimum(STAT::PSYCHE, basePsy);
			destData->UpdateBaseStatMinimum(STAT::SPIRIT, baseSpi);
		}
	}
}

void CharacterData :: UpdateEquipStats(CreatureInstance *destData)
{
	if(destData != NULL)
	{
		destData->MainDamage[0] = 0;
		destData->MainDamage[1] = 0;
		destData->RangedDamage[0] = 0;
		destData->RangedDamage[1] = 0;
		destData->OffhandDamage[0] = 0;
		destData->OffhandDamage[1] = 0;
		memset(destData->EquippedWeapons, 0, sizeof(destData->EquippedWeapons));
	}

	int armorClassMult = 1;
	switch(cdef.css.profession)
	{
	case Professions::KNIGHT: armorClassMult = 4; break;
	case Professions::ROGUE: armorClassMult = 2; break;
	case Professions::MAGE: armorClassMult = 1; break;
	case Professions::DRUID: armorClassMult = 3; break;
	}

	ItemSetTally itemSetTally;

	bool hasMeleeWeapon = false;
	bool hasShield = false;
	for(size_t a = 0; a < inventory.containerList[EQ_CONTAINER].size(); a++)
	{
		int slot = inventory.containerList[EQ_CONTAINER][a].GetSlot();
		int ID = inventory.containerList[EQ_CONTAINER][a].IID;
		VirtualItem *vitem = NULL;
		if(ID >= ItemManager::BASE_VIRTUAL_ITEM_ID)
			vitem = g_ItemManager.GetVirtualItem(ID);

		ItemDef *itemDef = inventory.containerList[EQ_CONTAINER][a].ResolveSafeItemPtr();

		g_ItemSetManager.CheckItem(ID, itemSetTally);

		if(itemDef->mEquipType == ItemEquipType::ARMOR_SHIELD)
			hasShield = true;
		switch(itemDef->mWeaponType)
		{
		case WeaponType::SMALL:
		case WeaponType::ONE_HAND:
		case WeaponType::TWO_HAND:
		case WeaponType::POLE:
		case WeaponType::WAND:
		case WeaponType::ARCANE_TOTEM:
			hasMeleeWeapon = true;
		}

		if(destData != NULL)
		{
			if(itemDef->mWeaponType != 0)
			{
				int wslot = slot;
				if(slot < 0 || slot >= 3)
				{
					g_Log.AddMessageFormat("[ERROR] UpdateEquipStats unexpected weapon slot: %d", slot);
					wslot = 0;
				}
				destData->EquippedWeapons[wslot] = itemDef->mWeaponType;
			}

			if(itemDef->mBonusStrength > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::STRENGTH, itemDef->mBonusStrength);
			if(itemDef->mBonusDexterity > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DEXTERITY, itemDef->mBonusDexterity);
			if(itemDef->mBonusConstitution > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::CONSTITUTION, itemDef->mBonusConstitution);
			if(itemDef->mBonusPsyche > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::PSYCHE, itemDef->mBonusPsyche);
			if(itemDef->mBonusSpirit > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::SPIRIT, itemDef->mBonusSpirit);

			if(itemDef->mArmorResistMelee > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_MELEE, itemDef->mArmorResistMelee * armorClassMult);
			if(itemDef->mArmorResistFire > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_FIRE, itemDef->mArmorResistFire);
			if(itemDef->mArmorResistFrost > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_FROST, itemDef->mArmorResistFrost);
			if(itemDef->mArmorResistMystic > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_MYSTIC, itemDef->mArmorResistMystic);
			if(itemDef->mArmorResistDeath > 0)
				destData->AddItemStatMod(itemDef->mID, STAT::DAMAGE_RESIST_DEATH, itemDef->mArmorResistDeath);

			if(vitem != NULL)
			{
				STRINGLIST modentry;
				STRINGLIST moddata;
				std::string combine;
				vitem->MergeStats(vitem->mModString, combine);
				Util::Split(combine, "&", modentry);
				for(size_t i = 0; i < modentry.size(); i++)
				{
					Util::Split(modentry[i], "=", moddata);
					if(moddata.size() >= 2)
					{
						int index = GetStatIndexByName(moddata[0].c_str());
						if(index >= 0)
						{
							if(StatList[index].isNumericalType() == true)
							{
								int statID = StatList[index].ID;
								float value = static_cast<float>(atof(moddata[1].c_str()));
								if(vitem->isItemDefStat(statID) == false)
									destData->AddItemStatMod(itemDef->mID, statID, value);
							}
						}
					}
				}
			}
			else if(itemDef->Params.size() > 0)
			{
				destData->ApplyItemStatModFromConfig(itemDef->mID, itemDef->Params);
			}
		}

		if(destData != NULL)
		{
			int slot = inventory.containerList[EQ_CONTAINER][a].CCSID & CONTAINER_SLOT;
			if(slot == ItemEquipSlot::WEAPON_MAIN_HAND)
			{
				destData->MainDamage[0] = itemDef->mWeaponDamageMin;
				destData->MainDamage[1] = itemDef->mWeaponDamageMax;
			}
			else if(slot == ItemEquipSlot::WEAPON_RANGED)
			{
				destData->RangedDamage[0] = itemDef->mWeaponDamageMin;
				destData->RangedDamage[1] = itemDef->mWeaponDamageMax;
			}
			else if(slot == ItemEquipSlot::WEAPON_OFF_HAND)
			{
				destData->OffhandDamage[0] = itemDef->mWeaponDamageMin;
				destData->OffhandDamage[1] = itemDef->mWeaponDamageMax;
			}
		}
	}

	if(destData != NULL)
	{
		g_ItemSetManager.UpdateCreatureBonuses(itemSetTally, destData);

		destData->SetServerFlag(ServerFlags::HasMeleeWeapon, hasMeleeWeapon);
		destData->SetServerFlag(ServerFlags::HasShield, hasShield);

		//g_Log.AddMessageFormat("Melee: %d, Shield: %d", hasMeleeWeapon, hasShield);
	}
}

void CharacterData :: UpdateEqAppearance()
{
	std::string str;
	str.append("{");
	for(size_t a = 0; a < inventory.containerList[EQ_CONTAINER].size(); a++)
	{
		InventorySlot *slot = &inventory.containerList[EQ_CONTAINER][a];
		if(a > 0)
			str.append(",");
		int appearanceID = slot->GetLookID();
		str.append("[");
		Util::StringAppendInt(str, slot->CCSID & CONTAINER_SLOT);
		str.append("]=");
		Util::StringAppendInt(str, appearanceID);
	}
	str.append("}");
	cdef.css.SetEqAppearance(str.c_str());
}

void CharacterData :: BackupAppearance(void)
{
	if(originalAppearance.size() == 0)
		originalAppearance = cdef.css.appearance;
}

void CharacterData :: BuildAvailableQuests(QuestDefinitionContainer &questList)
{
	questJournal.availableQuests.itemList.clear();
	questJournal.availableSoonQuests.itemList.clear();

	//The iterators below would crash the program if attempting to iterate across an empty quest list
	//when trying to set up the default null (zero ID) character.
	if(questList.mQuests.size() == 0)
		return;

	QuestReference qr;
	bool soon = false;
	
	QuestDefinitionContainer::ITERATOR it;
	for(it = questList.mQuests.begin(); it != questList.mQuests.end(); ++it)
	{
		QuestDefinition *qd = &it->second;

		soon = false;
		if(cdef.css.level < qd->levelMin)
		{
			if(cdef.css.level >= qd->levelMin - QuestJournal::QUEST_SOON_TOLERANCE)
				soon = true;
			else
				continue;
		}
		if(qd->levelMax != 0)
			if(cdef.css.level > qd->levelMax)
				continue;
		if(qd->profession != 0)
			if(cdef.css.profession != qd->profession)
				continue;

		// Guild start quest
		if(qd->guildStart && IsInGuildAndHasValour(qd->guildId, 0))
			continue;

		// Guild requirements
		if(!qd->guildStart && qd->guildId != 0 && !IsInGuildAndHasValour(qd->guildId, qd->valourRequired))
			continue;

		//If we get here, the quest is good to add to at least one availability list.
		qr.QuestID = qd->questID;
		qr.DefPtr = qd;
		qr.CreatureDefID = qd->QuestGiverID;

		//Add to the pending list.  This function performs the necessary checks.
		if(soon == false)
		{
			questJournal.AddPendingQuest(qr);
		}
		else
			questJournal.availableSoonQuests.AddItem(qr);
	}

	if(questJournal.availableSoonQuests.itemList.size() > 0)
		questJournal.availableSoonQuests.Sort();
}

void CharacterData :: OnFinishedLoading(void)
{
	//If the character file loaded any items into the vault, then all vault processing must
	//be local only.  Do not overwrite the account vault when logging out.
	localCharacterVault = true;
	
	/*
	localCharacterVault = (inventory.containerList[BANK_CONTAINER].size() > 0);
	g_Log.AddMessageFormat("Local vault data: %d.", localCharacterVault);
	*/

	//Reset the important stats here.  Just in case the character file has incorrect values.
	//Base stats and health will be recalculated after when the equipment bonuses are rechecked.
	cdef.css.strength = 0;
	cdef.css.dexterity = 0;
	cdef.css.constitution = 0;
	cdef.css.psyche = 0;
	cdef.css.spirit = 0;
	cdef.css.damage_resist_melee = 0;
	cdef.css.damage_resist_fire = 0;
	cdef.css.damage_resist_frost = 0;
	cdef.css.damage_resist_mystic = 0;
	cdef.css.damage_resist_death = 0;
	cdef.css.dmg_mod_death = 0;
	cdef.css.dmg_mod_fire = 0;
	cdef.css.dmg_mod_frost = 0;
	cdef.css.dmg_mod_mystic = 0;
	cdef.css.dmg_mod_melee = 0;
	cdef.css.dr_mod_death = 0;
	cdef.css.dr_mod_fire = 0;
	cdef.css.dr_mod_frost = 0;
	cdef.css.dr_mod_mystic = 0;
	cdef.css.dr_mod_melee = 0;
	cdef.css.casting_setback_chance = 500;
	cdef.css.channeling_break_chance = 500;
	cdef.css.offhand_weapon_damage = 500;
	cdef.css.base_healing = 0;
	cdef.css.base_damage_death = 0;
	cdef.css.base_damage_fire = 0;
	cdef.css.base_damage_frost = 0;
	cdef.css.base_damage_mystic = 0;
	cdef.css.base_damage_melee = 0;
	cdef.css.extra_damage_death = 0;
	cdef.css.extra_damage_fire = 0;
	cdef.css.extra_damage_frost = 0;
	cdef.css.extra_damage_mystic = 0;
	cdef.css.melee_attack_speed = 0;
	cdef.css.magic_attack_speed = 0;
	cdef.css.mod_melee_to_crit = 0.0F;
	cdef.css.mod_magic_to_crit = 0.0F;
	cdef.css.mod_attack_speed = 0;
	cdef.css.mod_movement = 0;
	cdef.css.experience_gain_rate = 0;
	cdef.css.base_movement = 0;
	cdef.css.weapon_damage_1h = 0;
	cdef.css.weapon_damage_2h = 0;
	cdef.css.weapon_damage_box = 0;
	cdef.css.weapon_damage_pole = 0;
	cdef.css.weapon_damage_small = 0;
	cdef.css.weapon_damage_thrown = 0;
	cdef.css.weapon_damage_wand = 0;
	cdef.css.base_block = 0;
	cdef.css.base_parry = 0;
	cdef.css.base_dodge = 0;
	cdef.css.mod_attack_speed = ABGlobals::MINIMAL_FLOAT;
	cdef.css.mod_casting_speed = ABGlobals::MINIMAL_FLOAT;
	cdef.css.mod_health_regen = 0.0F;
	cdef.css.might_regen = ABGlobals::DEFAULT_MIGHT_REGEN;
	cdef.css.will_regen = ABGlobals::DEFAULT_WILL_REGEN;
	cdef.css.mod_luck = 0.0F;
	cdef.css.bonus_health = 0;
	cdef.css.damage_shield = 0;

	UpdateBaseStats(NULL, true);

	inventory.FixBuyBack();
	BackupAppearance();
	BuildAvailableQuests(QuestDef);
	questJournal.ResolveLoadedQuests();
	inventory.CountInventorySlots();
	
	cdef.css.total_ability_points = Global::GetAbilityPointsLevelCumulative(cdef.css.level) + ExtraAbilityPoints;
	CheckVersion();

	VersionUpgradeCharacterItems();

	//Check if the scaler is valid since it may have changed through updates.
	if(InstanceScaler.size() > 0)
	{
		if(g_InstanceScaleManager.GetProfile(InstanceScaler) == NULL)
			InstanceScaler.clear();
	}
}

void CharacterData :: CheckVersion(void)
{
	if(characterVersion >= SERVER_CHARACTER_VERSION)
		return;

	if(characterVersion < 2)
		VersionUpgradeCharacterItems();

	characterVersion = SERVER_CHARACTER_VERSION;
	pendingChanges++;
	g_Log.AddMessageFormat("Character Upgraded: %s (%d)", cdef.css.display_name, cdef.CreatureDefID);
}

void CharacterData :: VersionUpgradeCharacterItems(void)
{
	if(g_Config.Upgrade == 0)
		return;

	for(int c = 0; c < MAXCONTAINER; c++)
	{
		for(size_t i = 0; i < inventory.containerList[c].size(); i++)
		{
			ItemDef *itemDef = inventory.containerList[c][i].ResolveItemPtr();
			if(itemDef != NULL)
			{
				if(itemDef->mBindingType == BIND_ON_PICKUP && inventory.containerList[c][i].bindStatus == 0)
				{
					inventory.containerList[c][i].bindStatus = 1;
					g_Log.AddMessageFormat("Updating bind status %s (%d):%s", cdef.css.display_name, cdef.CreatureDefID, itemDef->mDisplayName.c_str());
				}
			}
		}
	}
	size_t pos = 0;
	while(pos < inventory.containerList[EQ_CONTAINER].size())
	{
		bool del = false;
		ItemDef *itemDef = inventory.containerList[EQ_CONTAINER][pos].ResolveItemPtr();
		if(itemDef != NULL)
		{
			int slot = inventory.containerList[EQ_CONTAINER][pos].GetSlot();
			int res = inventory.VerifyEquipItem(itemDef, slot, cdef.css.level, cdef.css.profession);
			if(res != InventoryManager::EQ_ERROR_NONE)
			{
				int slot = inventory.GetFreeSlot(INV_CONTAINER);
				if(slot >= 0)
				{
					InventorySlot copy;
					copy.CopyFrom(inventory.containerList[EQ_CONTAINER][pos], true);
					copy.CCSID = inventory.GetCCSID(INV_CONTAINER, slot);
					inventory.containerList[INV_CONTAINER].push_back(copy);
					del = true;
				}
				else
					g_Log.AddMessageFormat("%s: No inventory space for: %s", cdef.css.display_name, itemDef->mDisplayName.c_str());
			}
		}
		if(del == false)
			pos++;
		else
		{
			g_Log.AddMessageFormat("%s: Moving unequippable item: %s", cdef.css.display_name, itemDef->mDisplayName.c_str());
			inventory.containerList[EQ_CONTAINER].erase(inventory.containerList[EQ_CONTAINER].begin() + pos);
		}
	}
	pendingChanges++;
}

//Called by the character creation functions to initialize some specific defaults.
void CharacterData :: OnCharacterCreation(void)
{
	CurrentVaultSize = g_Config.VaultInitialPurchaseSize;

	if(cdef.css.level <= 1)
	{
		//Add the default quest.
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(378);
		if(qdef != NULL)
			questJournal.activeQuests.AddItem(378, qdef);

		activeData.CurX = 5682;
		activeData.CurY = 682;
		activeData.CurZ = 5890;
		activeData.CurZone = 59;
		activeData.CurRotation = 225;
		std::string quickbar0;
		quickbar0 = "\"{[\\\"slotsY\\\"]=1,[\\\"slotsX\\\"]=8,[\\\"snapY\\\"]=null,[\\\"y\\\"]=0.863333,[\\\"x\\\"]=0.33625,[\\\"positionX\\\"]=84,[\\\"positionY\\\"]=26,[\\\"snapX\\\"]=null,[\\\"locked\\\"]=true,[\\\"visible\\\"]=true,[\\\"buttons\\\"]=[null,null,";
		switch(cdef.css.profession)
		{
		case Professions::KNIGHT:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("224\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("288\\\",");
			break;
		case Professions::ROGUE:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("232\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("292\\\",");
			break;
		case Professions::MAGE:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("240\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("297\\\",");
			break;
		case Professions::DRUID:
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("248\\\",");
			quickbar0.append("\\\"ABILITYid:");
			quickbar0.append("301\\\",");
			break;
		default:
			quickbar0.append("null,null,");
		}
		quickbar0.append("null,null,null,null]}\"");
		preferenceList.SetPref("quickbar.0", quickbar0.c_str());
	}
}

void CharacterData :: OnRankChange(int newRank)
{
	//This function is intended to be called from the main thread.
	//Since we're altering the character data, need to lock access.
	g_CharacterManager.GetThread("CharacterData::OnRankChange");
	BuildAvailableQuests(QuestDef);
	g_CharacterManager.ReleaseThread();
}

void CharacterData :: OnLevelChange(int newLevel)
{
	//This function is intended to be called from the main thread.
	//Since we're altering the character data, need to lock access.
	g_CharacterManager.GetThread("CharacterData::OnLevelChange");

	cdef.css.level = newLevel;
	BuildAvailableQuests(QuestDef);

	g_CharacterManager.ReleaseThread();
}

void CharacterData :: SetPlayerDefaults(void)
{
	cdef.DefHints = 1;
	cdef.css.mod_casting_speed = numeric_limits<float>::denorm_min();
}

void CharacterData :: AbilityRespec(CreatureInstance *ptr)
{
	//cdef.css.total_ability_points = cdef.css.level * ABILITY_POINTS_PER_LEVEL;
	cdef.css.total_ability_points = Global::GetAbilityPointsLevelCumulative(cdef.css.level);
	cdef.css.total_ability_points += ExtraAbilityPoints;

	cdef.css.current_ability_points = cdef.css.total_ability_points;

	//Allows this function to update an instantiated creature with the correct
	//ability point data.
	if(ptr != NULL)
	{
		ptr->css.total_ability_points = cdef.css.total_ability_points;
		ptr->css.current_ability_points = cdef.css.current_ability_points;
	}

	abilityList.AbilityList.clear();

	abilityList.AddAbility(188);  // Bind : 1
	abilityList.AddAbility(189);  // Bind : 1

	switch(cdef.css.profession)
	{
	case Professions::KNIGHT:
		abilityList.AddAbility(224);  //Assault : 1
		abilityList.AddAbility(288);  //Bash : 1
		abilityList.AddAbility(1);    //Two Hand Weapons : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(5);    //Bow Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		abilityList.AddAbility(10);   //Block : 1
		break;
	case Professions::ROGUE:
		abilityList.AddAbility(232);  //Assail : 1
		abilityList.AddAbility(292);  //Disembowel : 1
		abilityList.AddAbility(8);    //Dual Wield : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(3);    //Small Weapons : 1
		abilityList.AddAbility(6);    //Thrown Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		break;
	case Professions::MAGE:
		abilityList.AddAbility(240);  //Firebolt : 1
		abilityList.AddAbility(297);  //Pyro Blast : 1
		abilityList.AddAbility(3);    //Small Weapons : 1
		abilityList.AddAbility(7);    //Wand Weapons : 1
		break;
	case Professions::DRUID:
		abilityList.AddAbility(248);  //Sting : 1
		abilityList.AddAbility(301);  //Deadly Shot : 1
		abilityList.AddAbility(4);    //Pole Weapons : 1
		abilityList.AddAbility(2);    //One Hand Weapons : 1
		abilityList.AddAbility(5);    //Bow Weapons : 1
		abilityList.AddAbility(9);    //Parry : 1
		break;
	}
}

void CharacterData :: SetLastChannel(const char *name, const char *password)
{
	if(name == NULL)
		PrivateChannelName.clear();
	else
		PrivateChannelName = name;

	if(password == NULL)
		PrivateChannelPassword.clear();
	else
		PrivateChannelPassword = password;
}

bool CharacterData :: HengeHas(int creatureDefID)
{
	for(size_t i = 0; i < hengeList.size(); i++)
		if(hengeList[i] == creatureDefID)
			return true;
	return false;
}

void CharacterData :: HengeAdd(int creatureDefID)
{
	hengeList.push_back(creatureDefID);
}

void CharacterData :: HengeSort(void)
{
	std::sort(hengeList.begin(), hengeList.end());
}

bool CharacterData :: SetPermission(short filterType, const char *name, bool value)
{
	int a;
	for(a = 0; a < MaxPermissionDef; a++)
	{
		if(PermissionDef[a].type == filterType)
		{
			if(strcmp(PermissionDef[a].name, name) == 0)
			{
				if(value == true)
					PermissionSet[PermissionDef[a].index] |= PermissionDef[a].flag;
				else
					PermissionSet[PermissionDef[a].index] &= (~(PermissionDef[a].flag));
				return true;
			}
		}
	}
	return false;
}

bool CharacterData :: HasPermission(short permissionSet, unsigned int permissionFlag)
{
	if(PermissionSet[permissionSet] & permissionFlag)
		return true;

	return false;
}

bool CharacterData :: QualifyGarbage(void)
{
	//Return true if the character data is garbage and can be safely deleted.
	if(expireTime == 0)      //Character is still registered within the game.
		return false;

//	if(pendingChanges == 0)  //Character needs to be saved before it can be deleted.
//		return false;

	if(g_ServerTime >= expireTime)
		return true;

	return false;
}


void CharacterData :: RemoveSidekicks(int sidekickType)
{
	size_t pos = 0;
	while(pos < SidekickList.size())
	{
		if(SidekickList[pos].summonType == sidekickType)
			SidekickList.erase(SidekickList.begin() + pos);
		else
			pos++;
	}
}

void CharacterData :: AddSidekick(SidekickObject& skobj)
{
	SidekickList.push_back(skobj);
}

int CharacterData :: CountSidekick(int sidekickType)
{
	int count = 0;
	for(size_t i = 0; i < SidekickList.size(); i++)
		if(SidekickList[i].summonType == sidekickType)
			count++;

	return count;
}

void CharacterData :: AddAbilityPoints(int abilityPointCount)
{
	ExtraAbilityPoints += abilityPointCount;
}

bool CharacterData :: NotifyUnstick(bool peek)
{
	int minutesPassed = g_PlatformTime.getAbsoluteMinutes() - LastUnstickTime;
	if(peek == true)
		return (minutesPassed < 5);

	LastUnstickTime = g_PlatformTime.getAbsoluteMinutes();
	UnstickCount++;
	if(minutesPassed < 5)
		return true;

	return false;
}

void CharacterData :: Debug_CountItems(int *intArr)
{
	for(size_t c = 0; c < 6; c++)
	{
		for(size_t i = 0; i < inventory.containerList[c].size(); i++)
		{
			InventorySlot *slot = &inventory.containerList[c][i];
			ItemDef *itemDef = slot->ResolveItemPtr();
			if(itemDef != NULL)
			{
				intArr[(int)itemDef->mQualityLevel]++;
			}
		}
	}
}

//Return the total vault capacity by combining the default space with the character-specific
//expanded space.
int CharacterData :: VaultGetTotalCapacity(void)
{
	int slots = g_Config.VaultDefaultSize + CurrentVaultSize;
	return Util::ClipInt(slots, 0, MAX_VAULT_SIZE);
}

//Return true if the vault has reached its maximum limit (default + extra).  If the limit
//is reached, it should allow any more expansions.
bool CharacterData :: VaultIsMaximumCapacity(void)
{
	return ((g_Config.VaultDefaultSize + CurrentVaultSize) >= MAX_VAULT_SIZE);
}

//Perform a vault expansion.  Assumes that all conditions are met.  Adjusts the relevant information
//in the character data (NOTE: does not subtract credits, those are part of the operational creature stat
//set).
void CharacterData :: VaultDoPurchaseExpand(void)
{
	CurrentVaultSize += VAULT_EXPAND_SIZE;
	CreditsSpent += VAULT_EXPAND_CREDIT_COST;
}

/*
void CharacterData :: NamedLocationUpdate(const NamedLocation &location)
{
	NamedLocation *exist = NamedLocationGetPtr(location.mName.c_str());
	if(exist == NULL)
		namedLocation.push_back(location);
	else
		exist->CopyFrom(location);
}

void CharacterData :: NamedLocationGetPtr(const char *name)
{
	for(size_t i = 0; i < namedLocation.size(); i++)
		if(namedLocation[i].mName.compare(name) == 0)
			return &namedLocation[i];
}
*/

enum CharacterDataFileSections
{
	CDFS_None = 0,
	CDFS_General,
	CDFS_Stats,
	CDFS_Prefs,
	CDFS_Inv,
	CDFS_Quest,
	CDFS_Cooldown,
	CDFS_Abilities
};

int CheckSection_General(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Responsible for loading general variables from a character file
	//Assumes that the line is already extracted and separated
	if(strcmp(fr.SecBuffer, "ID") == 0)
	{
		cd.cdef.CreatureDefID = fr.BlockToInt(1);
	}
	else if(strcmp(fr.SecBuffer, "ACCOUNTID") == 0)
	{
		cd.AccountID = fr.BlockToInt(1);
	}
	else if(strcmp(fr.SecBuffer, "CHARACTERVERSION") == 0)
	{
		cd.characterVersion = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "INSTANCE") == 0)
	{
		cd.activeData.CurInstance = fr.BlockToInt(1);
	}
	else if(strcmp(fr.SecBuffer, "ZONE") == 0)
	{
		cd.activeData.CurZone = fr.BlockToInt(1);
		if(cd.activeData.CurZone <= 0)
			cd.activeData.CurZone = g_DefZone;
	}
	else if(strcmp(fr.SecBuffer, "X") == 0)
	{
		cd.activeData.CurX = fr.BlockToInt(1);
		if(cd.activeData.CurX == -1)
			cd.activeData.CurX = g_DefX;
	}
	else if(strcmp(fr.SecBuffer, "Y") == 0)
	{
		cd.activeData.CurY = fr.BlockToInt(1);
		if(cd.activeData.CurY == -1)
			cd.activeData.CurY = g_DefY;
	}
	else if(strcmp(fr.SecBuffer, "Z") == 0)
	{
		cd.activeData.CurZ = fr.BlockToInt(1);
		if(cd.activeData.CurZ == -1)
			cd.activeData.CurZ = g_DefZ;
	}
	else if(strcmp(fr.SecBuffer, "ROTATION") == 0)
	{
		cd.activeData.CurRotation = fr.BlockToIntC(1) & 0xFF;
	}
	else if(strcmp(fr.SecBuffer, "STATUSTEXT") == 0)
	{
		cd.StatusText = fr.BlockToString(1);
	}
	else if(strcmp(fr.SecBuffer, "SECONDSLOGGED") == 0)
	{
		cd.SecondsLogged = fr.BlockToULongC(1);
	}
	else if(strcmp(fr.SecBuffer, "SESSIONSLOGGED") == 0)
	{
		cd.SessionsLogged = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "TIMELOGGED") == 0)
	{
		strncpy(cd.TimeLogged, fr.BlockToStringC(1, 0), sizeof(cd.TimeLogged) - 1);
	}
	else if(strcmp(fr.SecBuffer, "LASTSESSION") == 0)
	{
		strncpy(cd.LastSession, fr.BlockToStringC(1, 0), sizeof(cd.LastSession) - 1);
	}
	else if(strcmp(fr.SecBuffer, "LASTLOGON") == 0)
	{
		strncpy(cd.LastLogOn, fr.BlockToStringC(1, 0), sizeof(cd.LastLogOn) - 1);
	}
	else if(strcmp(fr.SecBuffer, "LASTLOGOFF") == 0)
	{
		strncpy(cd.LastLogOff, fr.BlockToStringC(1, 0), sizeof(cd.LastLogOff) - 1);
	}
	else if(strcmp(fr.SecBuffer, "ORIGINALAPPEARANCE") == 0)
	{
		cd.originalAppearance = fr.BlockToStringC(1, 0);
	}
	else if(strcmp(fr.SecBuffer, "CURRENTVAULTSIZE") == 0)
	{
		cd.CurrentVaultSize = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "CREDITSPURCHASED") == 0)
	{
		cd.CreditsPurchased = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "CREDITSSPENT") == 0)
	{
		cd.CreditsSpent = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "EXTRAABILITYPOINTS") == 0)
	{
		cd.ExtraAbilityPoints = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "GROVERETURN") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
		fr.MultiBreak("=,");
		cd.groveReturnPoint[0] = fr.BlockToIntC(1);
		cd.groveReturnPoint[1] = fr.BlockToIntC(2);
		cd.groveReturnPoint[2] = fr.BlockToIntC(3);
		cd.groveReturnPoint[3] = fr.BlockToIntC(4);
	}
	else if(strcmp(fr.SecBuffer, "BINDRETURN") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
		fr.MultiBreak("=,");
		cd.bindReturnPoint[0] = fr.BlockToIntC(1);
		cd.bindReturnPoint[1] = fr.BlockToIntC(2);
		cd.bindReturnPoint[2] = fr.BlockToIntC(3);
		cd.bindReturnPoint[3] = fr.BlockToIntC(4);
	}
	else if(strcmp(fr.SecBuffer, "LASTWARPTIME") == 0)
	{
		cd.LastWarpTime = fr.BlockToULongC(1);
	}
	else if(strcmp(fr.SecBuffer, "UNSTICKCOUNT") == 0)
	{
		cd.UnstickCount = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "LASTUNSTICKTIME") == 0)
	{
		cd.LastUnstickTime = fr.BlockToULongC(1);
	}
	else if(strcmp(fr.SecBuffer, "HENGELIST") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';

		int r = fr.MultiBreak("=,");
		int a;
		for(a = 1; a < r; a++)
			cd.HengeAdd(fr.BlockToIntC(a));
		cd.HengeSort();
	}
	else if(strcmp(fr.SecBuffer, "ABILITIES") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';

		int r = fr.MultiBreak("=,");
		int a;
		for(a = 1; a < r; a++)
			cd.abilityList.AddAbility(fr.BlockToIntC(a));
	}
	else if(strcmp(fr.SecBuffer, "FRIENDLIST") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';

		int r = fr.MultiBreak("=,");
		if(r >= 3)
		{
			int CDefID = fr.BlockToIntC(1);
			char *name = fr.BlockToStringC(2, 0);

			//For compatibility with older versions, which don't save the name.
			if(CDefID != 0 && strlen(name) > 0)
				cd.AddFriend(CDefID, name);
		}
	}
	else if(strcmp(fr.SecBuffer, "GUILDLIST") == 0)
		{
			//Restore this entry so it can be re-broken with multibreak instead
			if(fr.BlockPos[1] > 0)
				fr.DataBuffer[fr.BlockPos[1] - 1] = '=';

			int r = fr.MultiBreak("=,");
			if(r >= 3)
			{
				int GuildDefID = fr.BlockToIntC(1);
				int valour = fr.BlockToIntC(2);
				if(GuildDefID != 0) {
					cd.JoinGuild(GuildDefID);
					cd.AddValour(GuildDefID, valour);
				}
			}
		}
	else if(strcmp(fr.SecBuffer, "SIDEKICK") == 0)
	{
		//Restore this entry so it can be re-broken with multibreak instead
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';

		fr.MultiBreak("=,");
		SidekickObject skobj;
		skobj.CDefID = fr.BlockToIntC(1);
		skobj.summonType = fr.BlockToIntC(2);
		skobj.summonParam = fr.BlockToIntC(3);
		cd.SidekickList.push_back(skobj);
	}
	else if(strcmp(fr.SecBuffer, "MAXSIDEKICKS") == 0)
	{
		cd.MaxSidekicks = fr.BlockToIntC(1);
	}
	else if(strcmp(fr.SecBuffer, "PRIVATECHANNELNAME") == 0)
	{
		cd.PrivateChannelName = fr.BlockToString(1);
	}
	else if(strcmp(fr.SecBuffer, "PRIVATECHANNELPASSWORD") == 0)
	{
		cd.PrivateChannelPassword = fr.BlockToString(1);
	}
	else if(strcmp(fr.SecBuffer, "PERMISSIONS") == 0)
	{
		if(fr.BlockPos[1] > 0)
			fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
		fr.MultiBreak("=,");
		for(int a = 1; a < fr.MULTIBLOCKCOUNT; a++)
		{
			if(fr.BlockLen[a] > 0)
			{
				if(cd.SetPermission(Perm_Account, fr.BlockToStringC(a, Case_Lower), true) == false)
					g_Log.AddMessageFormat("Warning: Unknown permission identifier [%s] in Character file.", fr.SecBuffer);
			}
			else
				break;
		}
	}
	else if(strcmp(fr.SecBuffer, "INSTANCESCALER") == 0)
	{
		cd.InstanceScaler = fr.BlockToStringC(1, 0);
	}
	else
	{
		g_Log.AddMessageFormat("Unknown identifier [%s] in character General section (line: %d).", fr.BlockToString(0), fr.LineNumber);
	}
	return 0;
}

int CheckSection_Stats(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	static char StatName[64];
	fr.BlockToStringC(0, Case_Lower);
	strncpy(StatName, fr.SecBuffer, fr.BlockLen[0]);
	if(fr.BlockLen[0] >= 63)
		StatName[63] = 0;
	else
		StatName[fr.BlockLen[0]] = 0;

	int res = WriteStatToSetByName(StatName, fr.BlockToStringC(1, 0), &cd.cdef.css);
	if(res == -1)
	{
		g_Log.AddMessageFormat("Warning: unknown stat name in Character list: %s", StatName);
	}
	return 0;
}


int CheckSection_Preference(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Singlebreak leaves both strings null terminated, so calling them in this
	//form won't overwrite the target buffer as the copy function would.
	cd.preferenceList.SetPref(fr.BlockToString(0), fr.BlockToString(1));
	//g_Log.AddMessageFormat("[%s]=[%s]", fr.BlockToString(0), fr.BlockToString(1));

	return 0;
}

int CheckSection_Inventory(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Expected format:
	//  ContainerName=SlotID=ItemID

	//Restore this entry so it can be re-broken with multibreak instead
	if(fr.BlockPos[1] > 0)
		fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
	int r = fr.MultiBreak("=,");

	int ContID = GetContainerIDFromName(fr.BlockToStringC(0, 0));
	if(ContID == -1)
	{
		g_Log.AddMessageFormat("Warning: Character [%s] unknown container name [%s]", cd.cdef.css.display_name, fr.BlockToStringC(0, 0));
		return -1;
	}
	int slot = fr.BlockToInt(1);
	int ID = fr.BlockToInt(2);
	if(cd.inventory.GetItemBySlot(ContID, slot) >= 0)
	{
		g_Log.AddMessageFormat("Warning: Character [%s] inventory [%s] slot already filled [%d]", cd.cdef.css.display_name, fr.BlockToStringC(0, 0), slot);
		return -1;
	}

	//Debug::TimeTrackStep tt("GetPointerByID");
	//g_Log.AddMessageFormat("Looking up: %d", ID);
	ItemDef *itemDef = g_ItemManager.GetPointerByID(ID);
	if(itemDef == NULL)
	{
		g_Log.AddMessageFormat("Warning: Character [%s] Item ID [%d] not found for container [%s]", cd.cdef.css.display_name, ID, fr.BlockToStringC(0, 0));
		Debug::Log("Warning: Character [%s] Item ID [%d] not found for container [%s]", cd.cdef.css.display_name, ID, fr.BlockToStringC(0, 0));
		return -1;
	}
	//tt.Finish();

	int count = 0;
	int customLook = 0;
	char bindStatus = 0;
	if(r >= 4)
		count = fr.BlockToInt(3);
	if(r >= 5)
		customLook = fr.BlockToIntC(4);
	if(r >= 6)
		bindStatus = (char)fr.BlockToIntC(5);

	InventorySlot newItem;
	newItem.CCSID = (ContID << 16) | slot;
	newItem.IID = ID;
	newItem.dataPtr = itemDef;
	newItem.count = count;
	newItem.customLook = customLook;
	newItem.bindStatus = bindStatus;
	if(cd.inventory.AddItem(ContID, newItem) == -1)
	{
		g_Log.AddMessageFormat("Warning: Character [%s] failed to add item (container:%s, slot:%d, itemID:%d)", cd.cdef.css.display_name, fr.BlockToStringC(0, 0), slot, ID);
		return -1;
	}
	return 0;
}

int CheckSection_Quest(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Expected formats:
	//  active=id,act,obj1comp,obj1count,obj2comp,obj2count,obj3comp,obj3count
	//  complete=a,b,c,d,e,f,g...
	//  repeat=id,startMinute,waitTime

	QuestReference newItem;
	newItem.Reset();

	if(fr.BlockPos[1] > 0)
		fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
	int r = fr.MultiBreak("=,");

	int complete, count;
	char *tag = fr.BlockToStringC(0, 0);
	if(strcmp(tag, "active") == 0)
	{
		// Index: 0   1  2   3        4         5        6         7        8
		//    active=id,act,obj1comp,obj1count,obj2comp,obj2count,obj3comp,obj3count
		newItem.QuestID = fr.BlockToIntC(1);
		newItem.CurAct = fr.BlockToIntC(2);
		for(int a = 0; a < 3; a++ )
		{
			int offset = a * 2;
			complete = fr.BlockToIntC(3 + offset);
			count = fr.BlockToIntC(4 + offset);
			newItem.ObjComplete[a] = complete;
			newItem.ObjCounter[a] = count;
		}
		cd.questJournal.activeQuests.AddItem(newItem);
	}
	else if(strcmp(tag, "complete") == 0)
	{
		for(int a = 1; a < r; a++)
		{
			newItem.QuestID = fr.BlockToIntC(a);
			cd.questJournal.completedQuests.AddItem(newItem);
		}
	}
	else if(strcmp(tag, "repeat") == 0)
	{
		int ID = fr.BlockToIntC(1);
		unsigned long startMinute = fr.BlockToULongC(2);
		unsigned long waitMinute = fr.BlockToULongC(3);
		cd.questJournal.AddQuestRepeatDelay(ID, startMinute, waitMinute);
	}
	return 0;
}

int CheckSection_Cooldown(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Expected formats:
	//  cooldownCategoryName=remainingTimeMS,elapsedTimeMS

	if(fr.BlockPos[1] > 0)
		fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
	fr.MultiBreak("=,");

	const char *name = fr.BlockToStringC(0, 0);
	int remain = fr.BlockToInt(1);  //For these, don't use the copy form so it doesn't overwrite the name block.
	int elapsed = fr.BlockToInt(2);
	cd.cooldownManager.LoadEntry(name, remain, elapsed);
	
	return 0;
}

int CheckSection_Abilties(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Expected formats:
	//  Ability=tier,buffType,ability ID,ability group ID,remain

	if(fr.BlockPos[1] > 0)
		fr.DataBuffer[fr.BlockPos[1] - 1] = '=';
	fr.MultiBreak("=,");

	unsigned char tier = fr.BlockToInt(1);
	unsigned char buffType = fr.BlockToInt(2);
	short abID = fr.BlockToInt(3);
	short abgID = fr.BlockToInt(4);
	unsigned long remain = fr.BlockToULongC(5);
	double remainS = remain / 1000.0;
	cd.buffManager.AddBuff(tier, buffType, abID, abgID, remainS);

	return 0;
}

int LoadCharacterFromStream(FileReader &fr, CharacterData &cd, const char *debugFilename)
{
	//Return codes:
	//   1  Section end marker reached.
	//   0  End of file reached.
	//  -1  Another entry was encountered

	//Default stat assignments will be assigned to the original set.
	//CharacterDataSet dataPtr = &cd.oss;

	//Which dataset is currently being loaded
	int Section = CDFS_General;

	bool curEntry = false;
	int r;
	while(fr.FileOpen())
	{
		long CurPos = ftell(fr.FileHandle[0]);
		r = fr.ReadLine();
		if(r > 0)
		{
			r = fr.SingleBreak("=");
			fr.BlockToStringC(0, Case_Upper);
			if(strcmp(fr.SecBuffer, "[ENTRY]") == 0)
			{
				Section = CDFS_General;
				if(curEntry == true)
				{
					//Reset the position so it doesn't interfere with reading the next
					//entry
					fr.FilePos = CurPos;
					fseek(fr.FileHandle[0], CurPos, SEEK_SET);
					return -1;
				}
				else
					curEntry = true;
			}
			else if(strcmp(fr.SecBuffer, "[END]") == 0)
			{
				return 1;
			}
			else if(strcmp(fr.SecBuffer, "[GENERAL]") == 0)
			{
				Section = CDFS_General;
			}
			else if(strcmp(fr.SecBuffer, "[STATS]") == 0)
			{
				Section = CDFS_Stats;
			}
			else if(strcmp(fr.SecBuffer, "[PREFS]") == 0)
			{
				Section = CDFS_Prefs;
			}
			else if(strcmp(fr.SecBuffer, "[INV]") == 0)
			{
				Section = CDFS_Inv;
			}
			else if(strcmp(fr.SecBuffer, "[QUEST]") == 0)
			{
				Section = CDFS_Quest;
			}
			else if(strcmp(fr.SecBuffer, "[COOLDOWN]") == 0)
			{
				Section = CDFS_Cooldown;
			}
			else if(strcmp(fr.SecBuffer, "[ABILITIES]") == 0)
			{
				Section = CDFS_Abilities;
			}
			else
			{
				if(Section == CDFS_General)
					CheckSection_General(fr, cd, debugFilename);
				else if(Section == CDFS_Stats)
					CheckSection_Stats(fr, cd, debugFilename);
				else if(Section == CDFS_Prefs)
					CheckSection_Preference(fr, cd, debugFilename);
				else if(Section == CDFS_Inv)
					CheckSection_Inventory(fr, cd, debugFilename);
				else if(Section == CDFS_Quest)
					CheckSection_Quest(fr, cd, debugFilename);
				else if(Section == CDFS_Cooldown)
					CheckSection_Cooldown(fr, cd, debugFilename);
				else if(Section == CDFS_Abilities)
					CheckSection_Abilties(fr, cd, debugFilename);
			}
		}
	}
	fr.CloseCurrent();

	//g_Log.AddMessageFormat("Loaded %d characters.", CharList.UsedCount);
	return 1;
}

void SaveCharacterToStream(FILE *output, CharacterData &cd)
{
	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "characterVersion=%d\r\n", cd.characterVersion);
	fprintf(output, "AccountID=%d\r\n", cd.AccountID);
	fprintf(output, "ID=%d\r\n", cd.cdef.CreatureDefID);
	fprintf(output, "Instance=%d\r\n", cd.activeData.CurInstance);
	fprintf(output, "Zone=%d\r\n", cd.activeData.CurZone);
	fprintf(output, "X=%d\r\n", cd.activeData.CurX);
	fprintf(output, "Y=%d\r\n", cd.activeData.CurY);
	fprintf(output, "Z=%d\r\n", cd.activeData.CurZ);
	fprintf(output, "Rotation=%d\r\n", cd.activeData.CurRotation);

	fprintf(output, "StatusText=%s\r\n", cd.StatusText.c_str());
	Util::WriteString(output, "InstanceScaler", cd.InstanceScaler);
	Util::WriteString(output, "PrivateChannelName", cd.PrivateChannelName);
	Util::WriteString(output, "PrivateChannelPassword", cd.PrivateChannelPassword);

	fprintf(output, "SecondsLogged=%lu\r\n", cd.SecondsLogged);
	fprintf(output, "SessionsLogged=%d\r\n", cd.SessionsLogged);
	fprintf(output, "TimeLogged=%s\r\n", cd.TimeLogged);
	fprintf(output, "LastSession=%s\r\n", cd.LastSession);
	fprintf(output, "LastLogOn=%s\r\n", cd.LastLogOn);
	fprintf(output, "LastLogOff=%s\r\n", cd.LastLogOff);
	fprintf(output, "OriginalAppearance=%s\r\n", cd.originalAppearance.c_str());

	fprintf(output, "CurrentVaultSize=%d\r\n", cd.CurrentVaultSize);
	fprintf(output, "CreditsPurchased=%d\r\n", cd.CreditsPurchased);
	fprintf(output, "CreditsSpent=%d\r\n", cd.CreditsSpent);
	fprintf(output, "ExtraAbilityPoints=%d\r\n", cd.ExtraAbilityPoints);

	fprintf(output, "GroveReturn=%d,%d,%d,%d\r\n", cd.groveReturnPoint[0], cd.groveReturnPoint[1], cd.groveReturnPoint[2], cd.groveReturnPoint[3]);
	fprintf(output, "BindReturn=%d,%d,%d,%d\r\n", cd.bindReturnPoint[0], cd.bindReturnPoint[1], cd.bindReturnPoint[2], cd.bindReturnPoint[3]);
	fprintf(output, "LastWarpTime=%lu\r\n", cd.LastWarpTime);
	fprintf(output, "UnstickCount=%d\r\n", cd.UnstickCount);
	fprintf(output, "LastUnstickTime=%lu\r\n", cd.LastUnstickTime);

	Util::WriteIntegerList(output, "hengeList", cd.hengeList);

	int write = 0;
	for(int a = 0; a < MaxPermissionDef; a++)
	{
		if((cd.PermissionSet[PermissionDef[a].index] & PermissionDef[a].flag) == PermissionDef[a].flag)
		{
			if(write == 0)
				fprintf(output, "Permissions=");
			if(write > 0)
				fputc(',', output);
			write++;

			fprintf(output, "%s", PermissionDef[a].name);
			if(write >= 5)
			{
				fprintf(output, "\r\n");
				write = 0;
			}
		}
	}
	if(write > 0)
		fprintf(output, "\r\n");

	Util::WriteIntegerList(output, "Abilities", cd.abilityList.AbilityList);

	//Guild list
	for(size_t i = 0; i < cd.guildList.size(); i++)
		fprintf(output, "GuildList=%d,%d\r\n", cd.guildList[i].GuildDefID, cd.guildList[i].Valour);

	//Friend list
	for(size_t i = 0; i < cd.friendList.size(); i++)
		fprintf(output, "FriendList=%d,%s\r\n", cd.friendList[i].CDefID, cd.friendList[i].Name.c_str());

	//Sidekicks
	fprintf(output, "MaxSidekicks=%d\r\n", cd.MaxSidekicks);
	/*
	fcount = (int)cd.SidekickList.size();
	written = 0;
	while(written < fcount)
	{
		if((written % 10) == 0)
		{
			if(written > 0)
				fprintf(output, "\r\n");
			fprintf(output, "Sidekicks=%d", cd.SidekickList[written]);
		}
		else
		{
			fprintf(output, ",%d", cd.SidekickList[written]);
		}
		written++;
	}
	if(written > 0)
		fprintf(output, "\r\n");
		*/
	for(size_t i = 0; i < cd.SidekickList.size(); i++)
		fprintf(output, "Sidekick=%d,%d,%d\r\n", cd.SidekickList[i].CDefID, cd.SidekickList[i].summonType, cd.SidekickList[i].summonParam);
	fprintf(output, "\r\n");

	//Stats
	fprintf(output, "[STATS]\r\n");
	int a;
	for(a = 0; a < NumStats; a++)
		if(isStatZero(a, &cd.cdef.css) == false)
			//if(isStatEqual(a, &cd.css, &defcd.css) == false)
				WriteStatToFile(a, &cd.cdef.css, output);

	fprintf(output, "\r\n");

	//Preferences
	fprintf(output, "[PREFS]\r\n");
	for(a = 0; a < (int)cd.preferenceList.PrefList.size(); a++)
	{
		/* Not comparing against defaults anymore
		int r = defcd.preferenceList.GetPrefIndex((char*)cd.preferenceList.PrefList[a].name.c_str());
		bool save = true;
		if(r >= 0)
		{
			if(defcd.preferenceList.PrefList[r].value.compare(cd.preferenceList.PrefList[a].value) == 0)
				save = false;
		}
		if(save == true)
			fprintf(output, "%s=%s\r\n", cd.preferenceList.PrefList[a].name.c_str(), cd.preferenceList.PrefList[a].value.c_str());
		*/
		fprintf(output, "%s=%s\r\n", cd.preferenceList.PrefList[a].name.c_str(), cd.preferenceList.PrefList[a].value.c_str());
	}
	fprintf(output, "\r\n");

	fprintf(output, "[INV]\r\n");
	int b;
	for(a = 0; a < MAXCONTAINER; a++)
	{
		for(b = 0; b < (int)cd.inventory.containerList[a].size(); b++)
		{
			InventorySlot *slot = &cd.inventory.containerList[a][b];
			fprintf(output, "%s=%lu,%d",
				GetContainerNameFromID((slot->CCSID & CONTAINER_ID) >> 16),
				slot->CCSID & CONTAINER_SLOT,
				slot->IID );

			bool extend = false;
			if(slot->count > 0 || slot->customLook != 0 || slot->bindStatus != 0)
				extend = true;

			if(extend == true)
				fprintf(output, ",%d,%d,%d", slot->count, slot->customLook, slot->bindStatus);

			fprintf(output, "\r\n");
		}
	}


	//Active quests
	fprintf(output, "\r\n[QUEST]\r\n");
	for(a = 0; a < (int)cd.questJournal.activeQuests.itemList.size(); a++)
	{
		QuestReference &qref = cd.questJournal.activeQuests.itemList[a];
		fprintf(output, "active=%d,%d", qref.QuestID, qref.CurAct);
		for(b = 0; b < 3; b++)
		{
			int comp = qref.ObjComplete[b];
			int count = qref.ObjCounter[b];
			fprintf(output, ",%d,%d", comp, count);
		}
		fprintf(output, "\r\n");
	}

	// Completed quests
	int fcount = (int)cd.questJournal.completedQuests.itemList.size();
	int written = 0;
	while(written < fcount)
	{
		if((written % 10) == 0)
		{
			if(written > 0)
				fprintf(output, "\r\n");
			fprintf(output, "complete=%d", cd.questJournal.completedQuests.itemList[written].QuestID);
		}
		else
		{
			fprintf(output, ",%d", cd.questJournal.completedQuests.itemList[written].QuestID);
		}
		written++;
	}
	if(written > 0)
		fprintf(output, "\r\n");

	// Quests waiting to open again for repeat.
	for(size_t i = 0; i < cd.questJournal.delayedRepeat.size(); i++)
	{
		QuestRepeatDelay *d = &cd.questJournal.delayedRepeat[i];
		fprintf(output, "repeat=%d,%lu,%lu\r\n", d->QuestID, d->StartTimeMinutes, d->WaitTimeMinutes);
	}



	fprintf(output, "\r\n[COOLDOWN]\r\n");
	cd.cooldownManager.SaveToStream(output);

	// TODO not yet
//	fprintf(output, "\r\n[ABILITIES]\r\n");
//	cd.buffManager.SaveToStream(output);


	fprintf(output, "\r\n");
}

/* TODO: Revamp
int GetCharacterIndex(int CDefID)
{
	//Search the loaded character list for an entry matching the given ID
	if(CDefID == 0)
		return -1;

	int a;
	for(a = 0; a < (int)CharList.size(); a++)
		if(CharList[a].cdef.CreatureDefID == CDefID)
			return a;

	return -1;
}
*/

/* TODO: Revamp
int GetCharacterIndexByName(char *name)
{
	int a;
	for(a = 0; a < (int)CharList.size(); a++)
		if(strcmp(CharList[a].cdef.css.display_name, name) == 0)
			return a;
	return -1;
}
*/

//GuildListObject :: GuildListObject()
//{
//	Clear();
//}
//
//GuildListObject :: ~GuildListObject()
//{
//}
//
//GuildListObject :: Clear(void)
//{
//	GuildDefID = 0;
//	Valour = 0;
//}

//

FriendListObject :: FriendListObject()
{
	Clear();
}

FriendListObject :: ~FriendListObject()
{
	Name.clear();
}

void FriendListObject :: Clear(void)
{
	Name.clear();
	CDefID = 0;
}



CharacterManager :: CharacterManager()
{
	CreateDefaultCharacter();
	cs.SetDebugName("CS_CHARMGR");
	cs.disabled = true;
}

CharacterManager :: ~CharacterManager()
{
	Clear();
}

void CharacterManager :: Clear(void)
{
	charList.clear();
}

int CharacterManager :: LoadCharacter(int CDefID, bool tempResource)
{
	char FileName[256];
	Util::SafeFormat(FileName, sizeof(FileName), "Characters\\%d.txt", CDefID);
	Platform::FixPaths(FileName);

	FileReader lfr;
	if(lfr.OpenText(FileName) != Err_OK)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] Could not open character file [%s]", FileName);
		return -1;
	}

	CharacterData newChar;
	LoadCharacterFromStream(lfr, newChar, FileName);
	lfr.CloseCurrent();
	if(newChar.cdef.CreatureDefID != CDefID)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] LoadCharacter() ID mismatch, expected [%d], got [%d]", CDefID, newChar.cdef.CreatureDefID);
		return -1;
	}

	if(tempResource == true)
		newChar.expireTime = g_ServerTime + CharacterManager::TEMP_EXPIRE_TIME;

	GetThread("CharacterManager::LoadCharacter");

	bool newInstance = false;
	CHARACTER_MAP::iterator it;
	it = charList.lower_bound(CDefID);
	if(it == charList.end())
	{
		charList.insert(charList.begin(), CHARACTER_PAIR(CDefID, newChar));
		newInstance = true;
	}
	else if(it->first != CDefID)
	{
		charList.insert(it, CHARACTER_PAIR(CDefID, newChar));
		newInstance = true;
	}
	else
		g_Log.AddMessageFormatW(MSG_ERROR, "[WARNING] LoadCharacter() ID [%d] already exists.", CDefID);

	if(newInstance == true)
	{
		charList[CDefID].SetPlayerDefaults();
		charList[CDefID].OnFinishedLoading();
	}

	ReleaseThread();
	g_Log.AddMessageFormatW(MSG_SHOW, "Successfully loaded creature: %d [%s]", CDefID, charList[CDefID].cdef.css.display_name);
	return 0;
}

void CharacterManager :: GetThread(const char *request)
{
	cs.Enter(request);
}

void CharacterManager :: ReleaseThread(void)
{
	cs.Leave();
}

CharacterData * CharacterManager :: GetPointerByID(int CDefID)
{
	CHARACTER_MAP::iterator it;
	it = charList.find(CDefID);
	if(it == charList.end())
		return NULL;

	//Since we're accessing it, there must be some demand.
	//Extend the expiration timer, if it has one.
	it->second.ExtendExpireTime();
	return &it->second;
}

CharacterData * CharacterManager :: GetCharacterByName(const char *name)
{
	//TODO: not thread safe?
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
		if(strcmp(it->second.cdef.css.display_name, name) == 0)
			return &it->second;
	return NULL;
}

CharacterData * CharacterManager :: GetDefaultCharacter(void)
{
	CreateDefaultCharacter();
	return &charList[0];
}

void CharacterManager :: CreateDefaultCharacter(void)
{
	CharacterData *ret = RequestCharacter(0, false);
	if(ret != NULL)
		return;

	CharacterData newObj;
	charList.insert(CHARACTER_PAIR(0, newObj));
}

void CharacterManager :: AddExternalCharacter(int CDefID, CharacterData &newChar)
{
	CHARACTER_MAP::iterator it;
	it = charList.lower_bound(CDefID);
	if(it == charList.end())
		charList.insert(charList.begin(), CHARACTER_PAIR(CDefID, newChar));
	else if(it->first != CDefID)
		charList.insert(it, CHARACTER_PAIR(CDefID, newChar));
	else
		g_Log.AddMessageFormatW(MSG_ERROR, "[WARNING] AddExternalCharacter() ID [%d] already exists.", CDefID);
}

void CharacterManager :: Compatibility_SaveList(FILE *output)
{
	/*
	CHARACTER_MAP::iterator it;
	CharacterData empty;
	CharacterData *defChar = g_CharacterManager.GetDefaultCharacter();
	SaveCharacterToStream(output, *defChar, empty);

	for(it = charList.begin(); it != charList.end(); ++it)
	{
		SaveCharacterToStream(output, it->second, *defChar);
		fprintf(output, "\r\n\r\n");
	}
	*/
}

void CharacterManager :: Compatibility_ResolveCharacters(void)
{
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		it->second.BuildAvailableQuests(QuestDef);
		it->second.questJournal.ResolveLoadedQuests();
		it->second.inventory.CountInventorySlots();
	}
}

CharacterData * CharacterManager :: RequestCharacter(int CDefID, bool tempOnly)
{
	CharacterData *retPtr = GetPointerByID(CDefID);
	if(retPtr != NULL)
	{
		if(tempOnly == true && retPtr->expireTime > 0)
			retPtr->SetExpireTime();
		else if(tempOnly == false)
			retPtr->EraseExpirationTime();

		return retPtr;
	}

	int r = LoadCharacter(CDefID, tempOnly);
	if(r >= 0)
	{
		retPtr = &charList[CDefID];
		if(tempOnly == true && retPtr->expireTime > 0)
			retPtr->SetExpireTime();
		else if(tempOnly == false)
			retPtr->EraseExpirationTime();

		return retPtr;
	}

	return NULL;
}

void CharacterManager :: CheckGarbageCharacters(void)
{
	if(GarbageTimer.ReadyWithUpdate(CharacterManager::TEMP_GARBAGE_INTERVAL) == true)
		RemoveGarbageCharacters();
}

void CharacterManager :: RemoveGarbageCharacters(void)
{
	GetThread("CharacterManager::RemoveGarbageCharacters");

	/*
	CHARACTER_MAP::iterator it;
	it = charList.begin();
	bool qualify = false;
	while(it != charList.end())
	{
		qualify = RemoveSingleGarbage(it->second);
		if(qualify == true)
		{
			g_Log.AddMessageFormat("Removing garbage character: %s (%d)", it->second.cdef.css.display_name, it->first);
			charList.erase(it++);
		}
		else
			++it;
	}*/

	while(RemoveSingleGarbage() == true);

	ReleaseThread();
}

bool CharacterManager :: RemoveSingleGarbage(void)  //CharacterData &charData)
{

	/*
	//Return true if the character can be safely removed.
	if(charData.QualifyGarbage() == false)
		return false;

	if(charData.pendingChanges > 0)
	{
		if(SaveCharacter(charData.cdef.CreatureDefID) == false)
		{
			g_Log.AddMessageFormat("[ERROR] RemoveSingleGarbage() failed to save character");
			return false;
		}
	}
	return true;
	*/

	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		if(it->second.expireTime > 0)
		{
			if(g_ServerTime >= it->second.expireTime)
			{
				if(it->second.pendingChanges != 0)
				if(SaveCharacter(it->first) == false)
				{
					g_Log.AddMessageFormat("[DEBUG] RemoveSingleGarbage failed to save");
					return true;
				}

				UnloadCharacter(it->first);
				return true;
			}
		}
	}
	return false;
}

void CharacterManager :: UnloadAllCharacters(void)
{
	GetThread("CharacterManager::UnloadAllCharacters");

	while(RemoveSingleCharacter() == true);

	ReleaseThread();
}

bool CharacterManager :: RemoveSingleCharacter(void)
{
	CHARACTER_MAP::iterator it;
	for(it = charList.begin(); it != charList.end(); ++it)
	{
		if(it->second.pendingChanges != 0)
			SaveCharacter(it->first);

		UnloadCharacter(it->first);
		return true;
	}
	return false;
}

bool CharacterManager :: SaveCharacter(int CDefID)
{
	CharacterData *ptr = GetPointerByID(CDefID);
	if(ptr == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SaveCharacter() invalid ID: %d", CDefID);
		return false;
	}

	char FileName[256];
	sprintf(FileName, "Characters\\%d.txt", CDefID);
	Platform::FixPaths(FileName);

	FILE *output = fopen(FileName, "wb");
	if(output == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SaveCharacter() could not open file [%s]", FileName);
		return false;
	}
	SaveCharacterToStream(output, *ptr);

	ptr->pendingChanges = 0;

	if(fflush(output) != 0)
		g_Log.AddMessageFormat("[CRITICAL] Error flushing file: %s", FileName);
	if(fclose(output) != 0)
		g_Log.AddMessageFormat("[CRITICAL] Error closing file: %s", FileName);
		
	g_Log.AddMessageFormat("Saved character %d [%s]", ptr->cdef.CreatureDefID, ptr->cdef.css.display_name);
	return true;
}

void CharacterManager :: UnloadCharacter(int CDefID)
{
	CHARACTER_MAP::iterator it;
	it = charList.find(CDefID);
	if(it != charList.end())
	{
		if(it->second.pendingChanges != 0)
			g_Log.AddMessageFormat("[WARNING] Unloading a character with pending changes:", CDefID);

		g_Log.AddMessageFormat("Unloading character: %d [%s]", CDefID, it->second.cdef.css.display_name);
		charList.erase(it);
	}
}

void NamedLocation :: SetData(const char *data)
{
	STRINGLIST params;
	Util::Split(data, ",", params);
	if(params.size() >= 5)
	{
		mName = params[0];
		mX = atoi(params[1].c_str());
		mY = atoi(params[2].c_str());
		mZ = atoi(params[3].c_str());
		mZone = atoi(params[4].c_str());
	}
};

void NamedLocation :: CopyFrom(const NamedLocation &source)
{
	mName = source.mName;
	mX = source.mX;
	mY = source.mY;
	mZ = source.mZ;
	mZone = source.mZone;
}
