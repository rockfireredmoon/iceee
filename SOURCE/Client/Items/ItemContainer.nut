class this.ItemContainer 
{
	mContents = null;
	mCount = 0;
	mCreatureId = null;
	mContainerName = null;
	mUpdatePending = true;
	mLoading = true;
	mID = 0;
	constructor( id )
	{
		this.mUpdatePending = true;
		this.mContents = [];
		this.mID = id;
		::_ItemManager.addListener(this);
	}

	function containsItem( itemId )
	{
		foreach( item in this.mContents )
		{
			if (item == itemId)
			{
				return true;
			}
		}

		return false;
	}

	function setUpdatePending( which )
	{
		this.mUpdatePending = which;
	}

	function isUpdatePending()
	{
		return this.mUpdatePending;
	}

	function getCreatureId()
	{
		return this.mCreatureId;
	}

	function getName()
	{
		return this.mContainerName;
	}

	function removeItem( id )
	{
		local x;

		for( x = 0; x < this.mCount; x++ )
		{
			if (this.mContents[x] == id)
			{
				this.mContents.remove(x);
				this.mCount = this.mContents.len();
				this.mUpdatePending = true;
				return;
			}
		}
	}

	function addItem( id )
	{
		local x;

		for( x = 0; x < this.mCount; x++ )
		{
			if (this.mContents[x] == id)
			{
				return;
			}
		}

		this.mContents.append(id);
		this.mCount = this.mContents.len();
		this.mUpdatePending = true;
	}

	function hasAllItems()
	{
		if (this.mContainerName == null)
		{
			return false;
		}

		if (this.mContents.len() != this.mCount)
		{
			this.mContents = [];

			foreach( itemId, item in ::_ItemDataManager.mItemCache )
			{
				if (item.getContainerId() == this.mID && item.isValid())
				{
					this.mContents.append(itemId);
				}
			}

			if (this.mContents.len() == this.mCount)
			{
				this.mLoading = false;
			}

			return false;
		}

		if (this.mContents.len() > 0)
		{
			foreach( itemId in this.mContents )
			{
				local item = ::_ItemDataManager.getItem(itemId);

				if (item.getContainerId() == this.mID && !item.isValid())
				{
					return false;
				}
			}
		}

		return true;
	}

	function onStacksUpdated( sender, itemAction, mNumUses )
	{
		local itemData = itemAction.mItemData;

		if (itemData)
		{
			if (itemData.getContainerId() == this.mID)
			{
				this.mUpdatePending = true;
			}
		}
	}

}

