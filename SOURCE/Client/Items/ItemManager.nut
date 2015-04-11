this.require("Items/ItemAction");
this.require("Items/ItemDefAction");
this.require("Items/ItemDataManager");
class this.ItemManager extends this.MessageBroadcaster
{
	mItemActionCache = null;
	mItemDefActionCache = null;
	events = {
		function onItemUpdated( itemId, item, itemdef, lookdef )
		{
		}

		function onItemDefUpdated( itemDefId, itemdef )
		{
		}

		function onItemRemoved( itemId )
		{
		}

	};
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.mItemActionCache = {};
		this.mItemDefActionCache = {};
		::_ItemDataManager.addListener(this);
	}

	function getItem( itemId )
	{
		if (!(itemId in this.mItemActionCache))
		{
			local item = this._ItemDataManager.getItem(itemId);
			this.mItemActionCache[itemId] <- this.ItemAction("Unknown", "Icon/QuestionMark", itemId, item);
		}

		return this.mItemActionCache[itemId];
	}

	function getItemDef( itemDefId )
	{
		if (!(itemDefId in this.mItemDefActionCache))
		{
			local itemDef = this._ItemDataManager.getItemDef(itemDefId);
			this.mItemDefActionCache[itemDefId] <- this.ItemDefAction("Unknown", "Icon/QuestionMark", itemDefId, itemDef);
		}

		return this.mItemDefActionCache[itemDefId];
	}

	function onItemUpdated( itemId, item, itemdef, lookdef )
	{
		local itemAction = this.getItem(itemId);
		itemAction._updateItem(item, itemdef, lookdef);
		this.broadcastMessage("onItemUpdated", itemId, itemAction);
		local oldNumUses = item.getStackCount();
		local mNumUses = itemAction.getNumStacks();

		if (mNumUses != null && mNumUses != oldNumUses)
		{
			this.broadcastMessage("onStacksUpdated", this, itemAction, mNumUses);
		}

		local itemdefaction = this.getItemDef(item.mItemDefId);
		itemdefaction.updateStackCount();
	}

	function onItemRemoved( itemId, itemDefId )
	{
		if (itemId in this.mItemActionCache)
		{
			delete this.mItemActionCache[itemId];
		}
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		local itemDefAction = this.getItemDef(itemDefId);
		itemDefAction._updateItemDef(itemdef);
		itemDefAction.updateStackCount();
		this.broadcastMessage("onItemDefUpdated", itemDefId, itemdef);
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		foreach( itemDefId, itemDefAction in this.mItemDefActionCache )
		{
			itemDefAction.updateStackCount();
		}
	}

	function reset()
	{
		this.mItemActionCache = {};
	}

}

