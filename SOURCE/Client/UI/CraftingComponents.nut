class this.GUI.CraftingItemCount extends this.GUI.Component
{
	static mClassName = "CraftingItemCount";
	static LABEL_WIDTH = 215;
	static LABEL_HEIGHT = 18;
	static COMPLETED_LABEL_COLOR = "Bright Green";
	static NOT_COMPLETED_LABEL_COLOR = "red";
	mItemDefId = 0;
	mTitleLabel = null;
	mCountLabel = null;
	mBroadcaster = null;
	mCurrentCount = 0;
	mTotalCount = 0;
	constructor()
	{
		local fontSize = 18;
		this.mBroadcaster = this.MessageBroadcaster();
		this.GUI.Component.constructor(this.GUI.GridLayout(1, 2));
		this.getLayoutManager().setRows(this.LABEL_HEIGHT);
		this.getLayoutManager().setColumns(this.LABEL_WIDTH, "*");
		this.mTitleLabel = this.GUI.Label("");
		this.mTitleLabel.setAutoFit(true);
		this.mTitleLabel.setFont(this.GUI.Font("Maiandra", fontSize));
		this.mTitleLabel.setFontColor(this.Colors.white);
		this.add(this.mTitleLabel);
		local countHolder = this.GUI.Container(this.GUI.BoxLayoutV());
		countHolder.getLayoutManager().setAlignment(1.0);
		this.add(countHolder);
		this.mCountLabel = this.GUI.Label("");
		this.mCountLabel.setFont(this.GUI.Font("Maiandra", fontSize));
		countHolder.add(this.mCountLabel);
		::_ItemDataManager.addListener(this);
		::_ItemManager.addListener(this);
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function updateTotalCount( count )
	{
		this.mTotalCount = count;
		this._refreshCountLabel();
	}

	function updateItemDefId( itemDefId )
	{
		this.mItemDefId = itemDefId;
		this._updateTitle(::_ItemDataManager.getItemDef(this.mItemDefId).getDisplayName());
		local numStacks = ::_ItemDataManager.getNumItems(this.mItemDefId);
		this.updateItemAmount(numStacks);
	}

	function updateItemAmount( count )
	{
		if (count == this.mCurrentCount)
		{
			return;
		}

		if (count > this.mTotalCount)
		{
			this.mCurrentCount = this.mTotalCount;
		}
		else
		{
			this.mCurrentCount = count;
		}

		this.mBroadcaster.broadcastMessage("onCraftCountUpdate", this);
		this._refreshCountLabel();
	}

	function _refreshCountLabel()
	{
		this.mCountLabel.setText(this.mCurrentCount + "/" + this.mTotalCount);
		local labelColor = this.NOT_COMPLETED_LABEL_COLOR;

		if (this.mCurrentCount >= this.mTotalCount)
		{
			labelColor = this.COMPLETED_LABEL_COLOR;
		}

		this.mCountLabel.setFontColor(this.Colors[labelColor]);
	}

	function isComplete()
	{
		if (this.mCurrentCount >= this.mTotalCount)
		{
			return true;
		}

		return false;
	}

	function _updateTitle( titleText )
	{
		this.mTitleLabel.setText(titleText);
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (this.mItemDefId == 0 || itemDefId != this.mItemDefId)
		{
			return;
		}

		this._updateTitle(::_ItemDataManager.getItemDef(this.mItemDefId).getDisplayName());
		local numStacks = ::_ItemDataManager.getNumItems(itemDefId);
		this.updateItemAmount(numStacks);
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (this.mItemDefId != 0 && containerName == "inv" && ::_avatar && ::_avatar.getID() == creatureId)
		{
			local numStacks = ::_ItemDataManager.getNumItems(this.mItemDefId);
			this.updateItemAmount(numStacks);
		}
	}

	function onStacksUpdated( sender, itemAction, mNumUses )
	{
		local itemData = itemAction.mItemData;
		local itemDefId = itemData.mItemDefId;

		if (itemDefId == 0 || itemDefId != this.mItemDefId)
		{
			return;
		}

		local numStacks = ::_ItemDataManager.getNumItems(itemDefId);
		this.updateItemAmount(numStacks);
	}

	function clear()
	{
		this.mTitleLabel.setText("");
		this.mCountLabel.setText("");
		this.mCurrentCount = 0;
		this.mTotalCount = 0;
		this.mItemDefId = 0;
	}

}

