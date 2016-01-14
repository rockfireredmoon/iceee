#include <vector>
#include "Globals.h"
#include "Inventory.h"
#include "Util.h"
#include "StringList.h"
#include "Config.h"
#include "Debug.h"

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
	timeLoaded = source.timeLoaded;

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
	timeLoaded = source.timeLoaded;
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
long InventorySlot :: AdjustTimes() {
	if(secondsRemaining > -1) {
		secondsRemaining = GetTimeRemaining();
	}
	timeLoaded = g_ServerTime;
	return secondsRemaining;
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

void InventorySlot :: ApplyFromItemDef(ItemDef *itemDef) {
	secondsRemaining = -1;
	ApplyItemIntegerType(itemDef->mIvType1, itemDef->mIvMax1);
	ApplyItemIntegerType(itemDef->mIvType2, itemDef->mIvMax2);
	if(secondsRemaining > -1) {
		timeLoaded = g_ServerTime;
	}
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
	if(secondsRemaining < 0) {
		return -1;
	}
	long s = secondsRemaining - ( ( g_ServerTime - timeLoaded ) / 1000 );
	if(s < 0) {
		s = 0;
	}
	return s;
}

bool InventorySlot :: IsExpired(void)
{
	return secondsRemaining == 0;
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
	if(item.IID == 0)
	{
		return -1;
	}

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


	if(itemDef->mOwnershipRestriction > 0) {
		// Make sure there are less
		int count = GetItemCount(INV_CONTAINER, itemDef->mID);
		count += GetItemCount(BANK_CONTAINER, itemDef->mID);
		count += GetItemCount(EQ_CONTAINER, itemDef->mID);
		count += GetItemCount(DELIVERY_CONTAINER, itemDef->mID);
		count += GetItemCount(STAMPS_CONTAINER, itemDef->mID);
		if(count >= itemDef->mOwnershipRestriction) {
			// Already have the limit of this item
			SetError(ERROR_LIMIT);
			return NULL;
		}
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

	newItem.ApplyFromItemDef(itemDef);

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

int InventoryManager :: ScanRemoveItems(int containerID, int itemID, int count, std::vector<InventoryQuery> &resultList)
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
	std::vector<InventoryQuery> iq;
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

int InventoryManager :: RemoveItems(int containerID, std::vector<InventoryQuery> &resultList)
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

	std::vector<bool> tempCon;
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

ItemDef * InventoryManager :: GetBestSpecialItem(int invID, char specialItemType)
{
	std::vector<InventorySlot> inv = containerList[invID];
	ItemDef *itemDef = NULL;
	for(std::vector<InventorySlot>::iterator it = inv.begin(); it != inv.end() ; ++it) {
		InventorySlot sl = *it;
		if(!sl.IsExpired()) {
			ItemDef *iDef = g_ItemManager.GetPointerByID(sl.IID);
			if(iDef != NULL && iDef->mSpecialItemType == specialItemType) {
				if(itemDef == NULL || (itemDef != NULL && iDef->mIvMax1 > itemDef->mIvMax1)) {
					itemDef = iDef;
				}
			}
		}
	}
	return itemDef;
}

int InventoryManager :: CountUsedSlots(int containerID)
{
	int size = containerList[containerID].size();
	std::vector<bool> tempCon;
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
		if(tempCon[a] == true)
			count++;

	return count;
}


int InventoryManager :: CountFreeSlots(int containerID)
{
	int size = containerList[containerID].size();
	if(size >= MaxContainerSlot[containerID])
		return 0;

	std::vector<bool> tempCon;
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

InventorySlot * InventoryManager :: PickRandomItem(int containerID) {

	std::vector<int> tempCon;
	tempCon.resize(MaxContainerSlot[containerID]);
	unsigned int a;
	unsigned int c = 0;
	for(a = 0; a < containerList[containerID].size() && a < MaxContainerSlot[containerID]; a++)
	{
		unsigned int slot = containerList[containerID][a].CCSID & CONTAINER_SLOT;
		if(slot < MaxContainerSlot[containerID]) {
			tempCon.push_back(a);
			c++;
		}
	}
	return &containerList[containerID][tempCon[randmodrng(0, c)]];

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

int InventoryManager :: ItemMove(char *buffer, char *convBuf, CharacterStatSet *css, bool localCharacterVault, int origContainer, int origSlot, InventoryManager *destInv, int destContainer, int destSlot, bool updateDest)
{
	int wpos = 0;
	int origItemIndex = GetItemBySlot(origContainer, origSlot);
	if(origItemIndex == -1)
	{
		g_Log.AddMessageFormatW(MSG_ERROR, "[ERROR] ItemMove: item not found (container: %d, slot: %d)", origContainer, origSlot);
		return 0;
	}

	int destItemIndex = destInv->GetItemBySlot(destContainer, destSlot);

	//Don't move an item onto itself.  This was causing stacked items to be deleted.
	if(origContainer == destContainer && origSlot == destSlot && destInv == this)
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
		int index = destInv->GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_MAIN_HAND);
		if(index >= 0)
		{
			ItemDef *destItem = destInv->containerList[EQ_CONTAINER][index].ResolveSafeItemPtr();
			if(ItemManager::IsWeaponTwoHanded(destItem->mEquipType, destItem->mWeaponType) == true)
				return -2;
		}
	}
	//Repeat for reverse.
	if((origContainer == EQ_CONTAINER) && (origSlot == ItemEquipSlot::WEAPON_MAIN_HAND) && (destItemIndex >= 0))
	{
		InventorySlot &dest = destInv->containerList[destContainer][destItemIndex];
		ItemDef *check = dest.ResolveSafeItemPtr();
		if(ItemManager::IsWeaponTwoHanded(check->mEquipType, check->mWeaponType) == true)
			if(GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_OFF_HAND) >= 0)
				return -1;
	}
	if(origContainer == EQ_CONTAINER && origSlot == ItemEquipSlot::WEAPON_OFF_HAND)
	{
		int index = destInv->GetItemBySlot(EQ_CONTAINER, ItemEquipSlot::WEAPON_MAIN_HAND);
		if(index >= 0)
		{
			ItemDef *destItem = destInv->containerList[EQ_CONTAINER][index].ResolveSafeItemPtr();
			if(ItemManager::IsWeaponTwoHanded(destItem->mEquipType, destItem->mWeaponType) == true)
				return -2;
		}
	}


	if((destContainer == BANK_CONTAINER && localCharacterVault == false) || destContainer == DELIVERY_CONTAINER)
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
		InventorySlot &dest = destInv->containerList[destContainer][destItemIndex];

		//HACK: Very special case! When swapping the item in the destination slot into the equipment
		//slot, it must be verified for equipment restriction checks.
		if(origContainer == EQ_CONTAINER)
		{
			ItemDef *destItem = dest.ResolveItemPtr();
			if(destItem == NULL)
				return 0;
			int res = destInv->VerifyEquipItem(destItem, origSlot, css->level, css->profession);
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
					if(updateDest) {
						wpos += destInv->AddItemUpdate(&buffer[wpos], convBuf, &dest);
					}
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
					if(updateDest) {
						wpos += destInv->AddItemUpdate(&buffer[wpos], convBuf, &dest);
					}
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
		if(updateDest) {
			wpos += destInv->RemoveItemUpdate(&buffer[wpos], convBuf, &dest);
		}

		InventorySlot temp;
		temp.CopyFrom(dest, false);
		dest.CopyFrom(source, false);
		source.CopyFrom(temp, false);

		if(updateDest) {
			wpos += destInv->AddItemUpdate(&buffer[wpos], convBuf, &dest);
		}
		wpos += AddItemUpdate(&buffer[wpos], convBuf, &source);
		return wpos;
	}

	//Check if moving among a single container.  If so, just update the CCSID.
	//No need to add or erase slots.
	if(destContainer == origContainer && destInv == this)
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
	if(updateDest) {
		wpos += destInv->AddItemUpdate(&buffer[wpos], convBuf, &dest);
	}
	destInv->containerList[destContainer].push_back(dest);
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
	wpos += PutInteger(&buffer[wpos], slot->GetTimeRemaining());   //timeRemaining

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
	case ItemEquipType::BLUE_CHARM: targetEquipSlot = ItemEquipSlot::BLUE_CHARM; break;
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



int AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot)
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

	//Since flags is now 255, need extra placeholder data
	//if (flags & FLAG_ITEM_BOUND)
	wpos += PutByte(&buffer[wpos], slot->bindStatus);       //mBound

	//if (flags & FLAG_ITEM_TIME_REMAINING)
	wpos += PutInteger(&buffer[wpos], slot->GetTimeRemaining());   //timeRemaining

	PutShort(&buffer[1], wpos - 3);     //Set size

	return wpos;

	/*
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 70);       //_handleItemUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);       //Placeholder for size

	char MessageBuf[128];
	sprintf(MessageBuf, "%X", slot->CCSID);
	wpos += PutStringUTF(&buffer[wpos], MessageBuf);  //itemID
	wpos += PutByte(&buffer[wpos], ITEM_DEF | ITEM_LOOK_DEF | ITEM_CONTAINER);       //mask   zero = delete item?
	wpos += PutByte(&buffer[wpos], 255);       //flags

	//for ITEM_DEF
	wpos += PutInteger(&buffer[wpos], slot->IID);

	//for ITEM_LOOK_DEF
	wpos += PutInteger(&buffer[wpos], 0);

	//Since flags is now 255, need extra placeholder data
	//if (flags & FLAG_ITEM_BOUND)
	wpos += PutByte(&buffer[wpos], 0);       //mBound

	//if (flags & FLAG_ITEM_TIME_REMAINING)
	wpos += PutInteger(&buffer[wpos], -1);   //timeRemaining

	PutShort(&buffer[1], wpos - 3);     //Set size
	return wpos;
	*/
}

int RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot)
{
	//This function assumes that the buffer position in the argument list
	//is set to the current write position.
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


int PrepExt_TradeItemOffer(char *buffer, char *convBuf, int offeringPlayerID, std::vector<InventorySlot>& itemList)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&buffer[wpos], offeringPlayerID);   //traderID
	wpos += PutByte(&buffer[wpos], TradeEventTypes::ITEMS_OFFERED);     //eventType

	wpos += PutByte(&buffer[wpos], itemList.size());   //number of item proto strings
	for(size_t a = 0; a < itemList.size(); a++)
	{
		GetItemProto(convBuf, itemList[a].IID, itemList[a].count);
		wpos += PutStringUTF(&buffer[wpos], convBuf);
	}
	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}


int CheckSection_Inventory(FileReader &fr, InventoryManager &cd, const char *debugFilename, const char *debugName, const char *debugType)
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
		return -2;
	}
	int slot = fr.BlockToInt(1);
	int ID = fr.BlockToInt(2);
	if(cd.GetItemBySlot(ContID, slot) >= 0)
	{
		g_Log.AddMessageFormat("Warning: %s [%s] inventory [%s] slot already filled [%d]", debugType, debugName, fr.BlockToStringC(0, 0), slot);
		return -1;
	}

	//Debug::TimeTrackStep tt("GetPointerByID");
	//g_Log.AddMessageFormat("Looking up: %d", ID);
	ItemDef *itemDef = g_ItemManager.GetPointerByID(ID);
	if(itemDef == NULL)
	{
		g_Log.AddMessageFormat("[INVENTORY] Warning: %s [%s] Item ID [%d] not found for container [%s]", debugType, debugName, ID, fr.BlockToStringC(0, 0));
		Debug::Log("[INVENTORY] Warning: %s [%s] Item ID [%d] not found for container [%s]", debugType, debugName, ID, fr.BlockToStringC(0, 0));
		return -1;
	}
	//tt.Finish();

	int count = 0;
	int customLook = 0;
	long secondsRemaining = -1;
	char bindStatus = 0;
	if(r >= 4)
		count = fr.BlockToInt(3);
	if(r >= 5)
		customLook = fr.BlockToIntC(4);
	if(r >= 6)
		bindStatus = (char)fr.BlockToIntC(5);
	if(r >= 7)
		secondsRemaining = fr.BlockToLongC(6);

	InventorySlot newItem;
	newItem.CCSID = (ContID << 16) | slot;
	newItem.IID = ID;
	newItem.dataPtr = itemDef;
	newItem.count = count;
	newItem.customLook = customLook;
	newItem.bindStatus = bindStatus;
	newItem.secondsRemaining = secondsRemaining;
	newItem.timeLoaded = g_ServerTime;
	if(cd.AddItem(ContID, newItem) == -1)
	{
		g_Log.AddMessageFormat("Warning: %s [%s] failed to add item (container:%s, slot:%d, itemID:%d)", debugType, debugName, fr.BlockToStringC(0, 0), slot, ID);
		return -1;
	}
	return 0;
}
