class this.CreditShopManager extends this.DefaultQueryHandler
{
	mMessageBroadcaster = this.MessageBroadcaster();
	mConsumables = null;
	mCharms = null;
	mArmors = null;
	mBags = null;
	mRecipes = null;
	mNewItems = null;
	CategoryTypes = null;
	constructor()
	{
		::_Connection.addListener(this);
		this.mConsumables = [];
		this.mCharms = [];
		this.mArmors = [];
		this.mBags = [];
		this.mRecipes = [];
		this.mNewItems = [];
		this.CategoryTypes = {};
		this.CategoryTypes = {
			CONSUMABLES = {
				dataList = this.mConsumables,
				lastPage = 1
			},
			CHARMS = {
				dataList = this.mCharms,
				lastPage = 1
			},
			ARMOR = {
				dataList = this.mArmors,
				lastPage = 1
			},
			BAGS = {
				dataList = this.mBags,
				lastPage = 1
			},
			RECIPES = {
				dataList = this.mRecipes,
				lastPage = 1
			},
			NEW = {
				dataList = this.mNewItems,
				lastPage = 1
			}
		};
	}

	function getData( key )
	{
		if (key in this.CategoryTypes)
		{
			return this.CategoryTypes[key].dataList;
		}

		return [];
	}

	function getLastPage( key )
	{
		if (key in this.CategoryTypes)
		{
			return this.CategoryTypes[key].lastPage;
		}

		return 1;
	}

	function setLastPage( key, currentPage )
	{
		if (key in this.CategoryTypes)
		{
			this.CategoryTypes[key].lastPage = currentPage;
		}
	}

	function clearItemList()
	{
		foreach( key, data in this.CategoryTypes )
		{
			data.dataList.clear();
		}
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function requestItemMarketList( ... )
	{
		if (vargc > 0)
		{
			local category = vargv[0];
			::_Connection.sendQuery("item.market.list", this, category);
		}
		else
		{
			::_Connection.sendQuery("item.market.list", this);
		}
	}

	function requestBuyItem( creditItemId )
	{
		::_Connection.sendQuery("item.market.buy", this, creditItemId);
	}

	function onQueryComplete( qa, results )
	{
		switch(qa.query)
		{
		case "item.market.list":
			this._handleItemMarkerList(qa, results);
			break;

		case "item.market.buy":
			this.IGIS.info("Purchased item successfully.");
			break;

		default:
			break;
		}
	}

	function onQueryError( qa, results )
	{
		if (qa.query == "item.market.buy")
		{
			this.IGIS.error("Failed to purchase item: " + results);
		}
		else
		{
			  // [009]  OP_JMP            0      0    0    0
		}
	}

	function _handleItemMarkerList( qa, results )
	{
		this.clearItemList();

		foreach( item in results )
		{
			local id = item[0].tointeger();
			local title = item[1];
			local description = item[2];
			local status = item[3];
			local category = item[4];
			local beginDate = item[5];
			local endDate = item[6];
			local priceAmount = item[7].tointeger();
			local priceCurrency = item[8];
			local quantityLimit = item[9].tointeger();
			local quantitySold = item[10].tointeger();
			local itemProto = item[11];
			local creditShopItem = this.CreditShopItem(id, title, description, status, category, beginDate, endDate, priceAmount, priceCurrency, quantityLimit, quantitySold, itemProto);
			local isExpired = false;

			if (endDate != "")
			{
				local date = this.Util.split(endDate, ":");

				if (date.len() > 1 && date[0] == "Expired")
				{
					isExpired = true;
				}
			}

			if (status == "NEW")
			{
				local dataHolder = this.CategoryTypes.NEW.dataList;
				creditShopItem.setItemLabelType(this.ItemLabelType.NEW);
				dataHolder.append(creditShopItem);
			}
			else if (status == "HOT")
			{
				creditShopItem.setItemLabelType(this.ItemLabelType.HOT);
			}
			else if (quantityLimit > 0)
			{
				creditShopItem.setItemLabelType(this.ItemLabelType.LIMITED);
			}

			if (quantityLimit > 0 && quantitySold >= quantityLimit)
			{
				creditShopItem.setItemLabelType(this.ItemLabelType.SOLD_OUT);
			}
			else if (isExpired)
			{
				creditShopItem.setItemLabelType(this.ItemLabelType.EXPIRED);
			}

			if (category in this.CategoryTypes)
			{
				local dataHolder = this.CategoryTypes[category].dataList;
				dataHolder.append(creditShopItem);
			}
		}

		foreach( key, data in this.CategoryTypes )
		{
			data.dataList.sort(this.compareCreditItemIds);
		}

		this.mMessageBroadcaster.broadcastMessage("onCreditItemsUpdate", this);
	}

	function compareCreditItemIds( creditShopItemA, creditShopItemB )
	{
		if (creditShopItemA.getCreditItemId() > creditShopItemB.getCreditItemId())
		{
			return 1;
		}
		else if (creditShopItemA.getCreditItemId() < creditShopItemB.getCreditItemId())
		{
			return -1;
		}

		return 0;
	}

}

class this.CreditShopItem 
{
	mCreditItemId = null;
	mTitle = null;
	mDescription = null;
	mOfferStatus = null;
	mCategory = null;
	mBeginDate = null;
	mEndDate = null;
	mPriceAmount = null;
	mPriceCurrency = null;
	mQuantityLimit = null;
	mQuantitySold = null;
	mItemProto = null;
	mItemDefId = null;
	mCount = 1;
	mItemLabelType = this.ItemLabelType.NONE;
	constructor( id, title, description, status, category, beginDate, endDate, priceAmount, priceCurrency, quantityLimit, quantitySold, itemProto )
	{
		this.mCreditItemId = id;
		this.mTitle = title;
		this.mDescription = description;
		this.mOfferStatus = status;
		this.mCategory = category;
		this.mBeginDate = beginDate;
		this.mEndDate = endDate;
		this.mPriceAmount = priceAmount;
		this.mPriceCurrency = priceCurrency;
		this.mQuantityLimit = quantityLimit;
		this.mQuantitySold = quantitySold;
		this.mItemProto = itemProto;
		local itemData = this.split(itemProto, ":");

		if (itemData.len() > 2)
		{
			local itemDefId = this.Util.replace(itemData[0], "item", "");
			this.mItemDefId = itemDefId.tointeger();
			local count = itemData[2];
			this.mCount = count.tointeger();

			if (this.mCount > 0)
			{
				this.mCount = this.mCount + 1;
			}
		}

		if (this.mTitle == "")
		{
			local itemDef = ::_ItemDataManager.getItemDef(this.mItemDefId);

			if (itemDef)
			{
				this.mTitle = itemDef.getDisplayName();
			}
		}
	}

	function setItemLabelType( labelType )
	{
		this.mItemLabelType = labelType;
	}

	function getCreditItemId()
	{
		return this.mCreditItemId;
	}

	function getTitle()
	{
		return this.mTitle;
	}

	function getDescription()
	{
		return this.mDescription;
	}

	function getOfferStatus()
	{
		return this.mOfferStatus;
	}

	function getCategory()
	{
		return this.mCategory;
	}

	function getBeginDate()
	{
		return this.mBeginDate;
	}

	function getEndDate()
	{
		return this.mEndDate;
	}

	function getPriceAmount()
	{
		return this.mPriceAmount;
	}

	function getPriceCurrency()
	{
		return this.mPriceCurrency;
	}

	function getQuantityLimit()
	{
		return this.mQuantityLimit;
	}

	function getQuantitySold()
	{
		return this.mQuantitySold;
	}

	function getItemDefId()
	{
		return this.mItemDefId;
	}

	function getNumCount()
	{
		return this.mCount;
	}

	function getItemLabelType()
	{
		return this.mItemLabelType;
	}

}

