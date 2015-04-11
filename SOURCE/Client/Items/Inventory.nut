class this.Inventory extends this.MessageBroadcaster
{
	constructor( pNumSlots )
	{
		::MessageBroadcaster.constructor();
		this.mSlots = [];
		this.mSlots.resize(pNumSlots);
		this.mNumSlots = pNumSlots;
	}

	function getFirstEmtpySlot()
	{
		if (this.mUsedSlots != this.mNumSlots)
		{
			for( local x = 0; x < this.mNumSlots; x++ )
			{
				if (this.mSlots[x] == null)
				{
					return x;
				}
			}
		}

		return -1;
	}

	function addItem( pItem )
	{
		if (this.mUsedSlots != this.mNumSlots)
		{
			this.addItemToSlot(pItem, this.getFirstEmtpySlot());
			return true;
		}

		return false;
	}

	function removeItem( pItem )
	{
		local slot = this.getSlotByItem(pItem);

		if (slot == -1)
		{
			return false;
		}

		return this.removeItemFromSlot(slot);
	}

	function addItemToSlot( pItem, pSlotNum )
	{
		if (this.mUsedSlots != this.mNumSlots)
		{
			this.mSlots[pSlotNum] = pItem;
			this.mUsedSlots++;
			this.broadcastMessage("onUpdate");
			return true;
		}

		return false;
	}

	function removeItemFromSlot( pSlotNum )
	{
		if (pSlotNum != null)
		{
			this.mSlots[pSlotNum] = null;
			this.mUsedSlots--;
			this.broadcastMessage("onUpdate");
			return true;
		}

		return false;
	}

	function getItemInSlot( pSlotNum )
	{
		return this.mSlots[pSlotNum];
	}

	function getSlotByItem( pItem )
	{
		for( local x = 0; x < this.mNumSlots; x++ )
		{
			if (this.mSlots[x] == pItem)
			{
				return x;
			}
		}

		return -1;
	}

	function getUnslottedList()
	{
		local tempArray = [];

		for( local x = 0; x < this.mNumSlots; x++ )
		{
			if (this.mSlots[x] != null)
			{
				tempArray.append(this.mSlots[x]);
			}
		}

		return tempArray;
	}

	function getNumUsedSlots()
	{
		return this.mUsedSlots;
	}

	function getTotalSlots()
	{
		return this.mNumSlots;
	}

	function onTransfer( ... )
	{
		local pRepItem = vargv[0];
		local pSourceInventory = vargv[1];

		if (pSourceInventory != this)
		{
			if (this.mUsedSlots != this.mNumSlots)
			{
				pSourceInventory.removeItem(pRepItem);
				this.addItem(pRepItem);
			}
		}
	}

	mInventoryElement = null;
	mUsedSlots = 0;
	mNumSlots = 0;
	mSlots = [];
}

