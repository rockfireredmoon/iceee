this.require("Items/ItemDef");
class this.ItemDataManager extends this.MessageBroadcaster
{
	mItemCache = null;
	mItemDefCache = null;
	mItemDefUpdateCallbacks = null;
	mContainerCache = null;
	mContainerIds = null;
	mRequestCache = null;
	mActiveRequestCount = 0;
	events = {
		function onItemUpdated( itemId, item, itemdef, itemlookdef )
		{
		}

		function onItemDefUpdated( itemDefId, itemDef )
		{
		}

		function onItemRemoved( itemId, itemDefId )
		{
		}

		function onContainerUpdated( containerName, creatureId, container )
		{
		}

		function onContainerInvalid( containerName, creatureId )
		{
		}

	};
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.mItemCache = {};
		this.mItemDefCache = {};
		this.mItemDefUpdateCallbacks = {};
		this.mContainerCache = {};
		this.mContainerIds = {};
		this.mRequestCache = [];
		this.mActiveRequestCount = 0;
		::_eventScheduler.repeatIn(0.5, 0.5, this, "_updateContainers");
	}

	function getItem( itemId )
	{
		if (!(itemId in this.mItemCache))
		{
			local req = {};
			req.type <- "item";
			req.itemId <- itemId;
			this._pushRequest(req);
			this.mItemCache[itemId] <- this.ItemData();
		}

		return this.mItemCache[itemId];
	}

	function isItemDefCached( itemId )
	{
		return itemId in this.mItemCache;
	}

	function isItemDefFetched( itemDefId )
	{
		if ((itemDefId in this.mItemDefCache) && this.mItemDefCache[itemDefId].mValid == true)
		{
			return true;
		}

		return false;
	}

	function getItemDef( itemDefId, ... )
	{
		local callback;

		if (vargc > 0)
		{
			callback = vargv[0];
		}

		if (!(itemDefId in this.mItemDefCache))
		{
			if (itemDefId > 0)
			{
				local req = {};
				req.type <- "itemdef";
				req.itemDefId <- itemDefId;
				this._pushRequest(req);
			}

			this.mItemDefCache[itemDefId] <- this.ItemDefData(itemDefId);

			if (callback != null)
			{
				if (!(itemDefId in this.mItemDefUpdateCallbacks))
				{
					local callbackList = [];
					callbackList.append(callback);
					this.mItemDefUpdateCallbacks[itemDefId] <- {
						callbacks = callbackList
					};
				}
				else
				{
					this.mItemDefUpdateCallbacks[itemDefId].callbacks.append(callback);
				}
			}
		}
		else if (this.mItemDefCache[itemDefId].isValid())
		{
			if (callback != null)
			{
				callback.doWork(this.mItemDefCache[itemDefId]);
			}
		}
		else if (callback != null)
		{
			if (!(itemDefId in this.mItemDefUpdateCallbacks))
			{
				local callbackList = [];
				callbackList.append(callback);
				this.mItemDefUpdateCallbacks[itemDefId] <- {
					callbacks = callbackList
				};
			}
			else
			{
				this.mItemDefUpdateCallbacks[itemDefId].callbacks.append(callback);
			}
		}

		return this.mItemDefCache[itemDefId];
	}

	function getItemDefCache()
	{
		return this.mItemDefCache;
	}

	function getContents( containerName, ... )
	{
		local creatureId = ::_avatar.getID();
		local force = true;

		if (vargc > 0)
		{
			creatureId = vargv[0];
		}

		if (vargc > 1)
		{
			force = vargv[1];
		}

		if (!(creatureId in this.mContainerIds))
		{
			this.mContainerIds[creatureId] <- {};
		}

		if (!(containerName in this.mContainerIds[creatureId]) || force == true)
		{
			this.mContainerIds[creatureId][containerName] <- this.InventoryMapping[containerName];
			local req = {};
			req.type <- "container";
			req.creatureId <- creatureId;
			req.containerName <- containerName;
			this._pushRequest(req);
		}

		return this.getContainerById(this.mContainerIds[creatureId][containerName]);
	}

	function getContainerById( containerId )
	{
		if (!(containerId in this.mContainerCache))
		{
			this.mContainerCache[containerId] <- this.ItemContainer(containerId);
		}

		return this.mContainerCache[containerId];
	}

	function getDefIdStackCount( itemDefId, ... )
	{
		if (!::_avatar)
		{
			return;
		}

		local creatureId = ::_avatar.getID();
		local containerName = "inv";

		if (vargc >= 1)
		{
			containerName = vargv[0];
		}

		if (vargc >= 2)
		{
			creatureId = vargv[1];
		}

		local inventory = this.getContents(containerName, creatureId, false);
		local count = 0;

		foreach( itemId in inventory.mContents )
		{
			local item = this.getItem(itemId);

			if (item.mItemDefId == itemDefId)
			{
				local c = item.getStackCount();

				if (c != null)
				{
					count += c + 1;
				}
			}
		}

		return count;
	}

	function getNumItems( itemDefId, ... )
	{
		if (!::_avatar)
		{
			return;
		}

		local creatureId = ::_avatar.getID();
		local containerName = "inv";

		if (vargc >= 1)
		{
			containerName = vargv[0];
		}

		if (vargc >= 2)
		{
			creatureId = vargv[1];
		}

		local inventory = this.getContents(containerName, creatureId, false);
		local count = 0;

		foreach( itemId in inventory.mContents )
		{
			local item = this.getItem(itemId);

			if (item.mItemDefId == itemDefId)
			{
				local c = item.getStackCount();

				if (c != null)
				{
					count += c + 1;
				}
				else
				{
					count = count + 1;
				}
			}
		}

		return count;
	}

	function updateItemId( newItemId, oldItemId )
	{
		if (oldItemId in this.mItemCache)
		{
			this.mItemCache[newItemId] <- this.mItemCache[oldItemId];
			delete this.mItemCache[oldItemId];
			local previousContainerId = this.mItemCache[newItemId].mContainerItemId;
			local container = this.getContainerById(previousContainerId);

			if (container)
			{
				for( local x = 0; x != container.mContents.len(); x++ )
				{
					if (container.mContents[x] == oldItemId)
					{
						container.mContents[x] = newItemId;
						return;
					}
				}
			}
		}
	}

	function _pushRequest( request )
	{
		this.mRequestCache.append(request);
		this._processNextRequest();
	}

	function _requestHandled()
	{
		if (this.mActiveRequestCount > 0)
		{
			this.mActiveRequestCount--;
		}

		this._processNextRequest();
	}

	function _processNextRequest()
	{
		local REQUEST_THROTTLE = 20;

		if (this.mRequestCache.len() == 0 || this.mActiveRequestCount >= REQUEST_THROTTLE || !::_Connection.isPlaying() && !::_Connection.isLobby())
		{
			return;
		}

		this.mActiveRequestCount++;
		local req = this.mRequestCache.pop();

		switch(req.type)
		{
		case "item":
			this._requestItem(req.itemId);
			break;

		case "itemdef":
			this._requestItemDef(req.itemDefId);
			break;

		case "container":
			this._requestContents(req.containerName, req.creatureId);
			break;
		}
	}

	function _requestItem( itemId )
	{
		::_Connection.sendInspectItem(itemId);
	}

	function _requestItemDef( itemDefId )
	{
		::_Connection.sendInspectItemDef(itemDefId);
	}

	function _requestContents( containerName, creatureId )
	{
		if (creatureId != ::_avatar.getID())
		{
			::_Connection.sendQuery("item.contents", this, [
				containerName,
				creatureId
			]);
		}
		else
		{
			::_Connection.sendQuery("item.contents", this, [
				containerName
			]);
		}
	}

	function _onItemUpdate( itemId, update )
	{
		this._requestHandled();

		if (!(itemId in this.mItemCache))
		{
			this.mItemCache[itemId] <- this.ItemData();
		}

		local previousContainerId = this.mItemCache[itemId].getContainerId();

		foreach( index, row in update )
		{
			this.mItemCache[itemId][index] = row;
		}

		this.mItemCache[itemId].mValid = true;
		local itemdef = this.getItemDef(this.mItemCache[itemId].mItemDefId);
		local itemlookdef = itemdef;

		if (this.mItemCache[itemId].mItemLookDefId > 0)
		{
			itemlookdef = this.getItemDef(this.mItemCache[itemId].mItemLookDefId);
		}

		this.broadcastMessage("onItemUpdated", itemId, this.mItemCache[itemId], itemdef, itemlookdef);

		if (previousContainerId != null && previousContainerId != this.mItemCache[itemId].getContainerId())
		{
			local container = this.getContainerById(previousContainerId);

			if (container != null)
			{
				container.removeItem(itemId);
			}
		}

		if (previousContainerId != this.mItemCache[itemId].getContainerId())
		{
			local container = this.getContainerById(this.mItemCache[itemId].getContainerId());

			if (container != null)
			{
				container.addItem(itemId);
			}
		}
	}

	function _updateContainers()
	{
		foreach( k, v in this.mContainerCache )
		{
			if (v.isUpdatePending() && v.hasAllItems() && ::_avatar)
			{
				this.log.debug("Sending update for container: " + k);
				v.setUpdatePending(false);
				this.broadcastMessage("onContainerUpdated", v.getName(), v.getCreatureId(), v);
			}
		}
	}

	function _onItemRemoved( itemId )
	{
		this._requestHandled();

		if (!(itemId in this.mItemCache))
		{
			return;
		}

		local previousContainerId = this.mItemCache[itemId].mContainerItemId;
		local itemDefId = 0;

		if (itemId in this.mItemCache)
		{
			itemDefId = this.mItemCache[itemId].mItemDefId;
			delete this.mItemCache[itemId];
		}

		this.broadcastMessage("onItemRemoved", itemId, itemDefId);

		if (previousContainerId != null)
		{
			local container = this.getContainerById(previousContainerId);

			if (container != null)
			{
				container.removeItem(itemId);
			}
		}
	}

	function _onItemDefUpdate( itemDefId, update )
	{
		this._requestHandled();

		if (!(itemDefId in this.mItemDefCache))
		{
			return;
		}

		this.mItemDefCache[itemDefId] = update;
		this.mItemDefCache[itemDefId].mValid = true;

		if (itemDefId in this.mItemDefUpdateCallbacks)
		{
			foreach( callback in this.mItemDefUpdateCallbacks[itemDefId].callbacks )
			{
				callback.doWork(this.mItemDefCache[itemDefId]);
			}

			delete this.mItemDefUpdateCallbacks[itemDefId];
		}

		try
		{
			if (update.mAppearance != "")
			{
				update.mAppearance = this.unserialize(update.mAppearance);
			}
			else
			{
				update.mAppearance = null;
			}
		}
		catch( err )
		{
			update.mAppearance = null;
		}

		this.broadcastMessage("onItemDefUpdated", itemDefId, update);

		foreach( itemId, item in this.mItemCache )
		{
			if (item.mItemDefId == itemDefId || item.mItemLookDefId == itemDefId)
			{
				local itemlookdef = this.mItemDefCache[item.mItemDefId];

				if (item.mItemLookDefId > 0)
				{
					itemlookdef = this.getItemDef(item.mItemLookDefId);
				}

				this.broadcastMessage("onItemUpdated", itemId, item, this.mItemDefCache[item.mItemDefId], itemlookdef);
			}
		}
	}

	static function onQueryComplete( qa, results )
	{
		this._requestHandled();

		if (qa.query == "item.contents")
		{
			local containerName = qa.args[0];
			local creatureId = ::_avatar.getID();

			if (qa.args.len() > 1)
			{
				creatureId = qa.args[1];
			}

			if (creatureId == false)
			{
				return;
			}

			local containerId = results[0][0].tointeger();
			local count = results[0][1].tointeger();
			local container = this.getContainerById(containerId);
			container.mCount = count;
			container.mCreatureId = creatureId;
			container.mContainerName = containerName;
			this.mContainerIds[creatureId][containerName] = containerId;
			this.broadcastMessage("onItemContentsReceived", container);
		}
	}

	function onQueryError( qa, error )
	{
		this._requestHandled();

		if (qa.query == "item.contents")
		{
			local containerName = qa.args[0];
			local creatureId = qa.args.len() > 1 ? qa.args[1] : this._avatar.getID();
			this.broadcastMessage("onContainerInvalid", containerName, creatureId);
		}
		else
		{
			this.log.error("Query Error in Item Manager " + error);
		}
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function reset()
	{
		this.mActiveRequestCount = 0;
		this.mContainerCache = {};
		this.mContainerIds = {};
		this.mRequestCache = [];
		this.mItemDefCache = {};
		this.mItemCache = {};
	}

}

