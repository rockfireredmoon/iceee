class this.ItemData 
{
	CONTAINER_ID = 4294901760;
	CONTAINER_SLOT = 65535;
	mItemDefId = -1;
	mItemLookDefId = -1;
	mContainerItemId = null;
	mContainerSlot = null;
	mIv1 = null;
	mIv2 = null;
	mValid = false;
	mBound = false;
	timeRemaining = -1;
	timeRemainingRecievedAt = -1;
	function getContainerId()
	{
		return this.mContainerItemId;
	}

	function isValid()
	{
		if (!this.mValid)
		{
			return false;
		}

		local itemdef = ::_ItemDataManager.getItemDef(this.mItemDefId);

		if (!itemdef.mValid)
		{
			return false;
		}

		if (this.mItemLookDefId > 0)
		{
			local itemlookdef = ::_ItemDataManager.getItemDef(this.mItemLookDefId);

			if (!itemlookdef.mValid)
			{
				return false;
			}
		}

		return true;
	}

	function getTimeRemaining()
	{
		if (this.timeRemaining != -1)
		{
			return this.timeRemaining - (::_gameTime.getGameTimeSeconds() - this.timeRemainingRecievedAt);
		}

		return -1;
	}

	function getType()
	{
		local def = ::_ItemDataManager.getItemDef(this.mItemDefId);
		return def.getType();
	}

	function getStackCount()
	{
		local def = ::_ItemDataManager.getItemDef(this.mItemDefId);
		return def.getDynamicValue(this.ItemIntegerType.STACKING, this);
	}

	function getDurability()
	{
		local def = ::_ItemDataManager.getItemDef(this.mItemDefId);
		return def.getDynamicValue(this.ItemIntegerType.DURABILITY, this);
	}

}

