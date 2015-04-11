class this.SpecialOfferManager extends this.DefaultQueryHandler
{
	mMessageBroadcaster = this.MessageBroadcaster();
	mSpecialOffers = null;
	constructor()
	{
		::_Connection.addListener(this);
		this._exitGameStateRelay.addListener(this);
		this.mSpecialOffers = {};
	}

	function addSpecialOffer( specialOfferItem )
	{
		local specialOfferId = specialOfferItem.getId();

		if (!(specialOfferId in this.mSpecialOffers))
		{
			this.mSpecialOffers[specialOfferId] <- specialOfferItem;
			local notification = this.GUI.SpecialOfferNotification(specialOfferItem);
		}
	}

	function removeSpecialOffer( specialOfferId )
	{
		if (specialOfferId in this.mSpecialOffers)
		{
			delete this.mSpecialOffers[specialOfferId];
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

	function onQueryComplete( qa, results )
	{
		if (qa.query == "special.offer.buy")
		{
			this.IGIS.info("Purchased item successfully.");
		}
		else
		{
			  // [008]  OP_JMP            0      0    0    0
		}
	}

	function onQueryError( qa, results )
	{
		if (qa.query == "special.offer.buy")
		{
			this.IGIS.error("Failed to purchase item: " + results);
		}
		else
		{
			  // [009]  OP_JMP            0      0    0    0
		}
	}

	function onExitGame()
	{
		this.mSpecialOffers.clear();
		this.mSpecialOffers = {};
	}

}

class this.SpecialOfferItem 
{
	mId = -1;
	mTitle = null;
	mDescription = null;
	mPercentDiscount = 0;
	mCreditItemId = null;
	mOfferCost = null;
	mOfferItemTitle = null;
	mOfferItemDesc = null;
	mOfferItemProto = null;
	mHashCode = null;
	mItemDefId = null;
	mStackCount = 1;
	constructor( id, discount, offerItemCost, creditItemId, title, desc, offerItemTitle, offerItemDesc, offerItemProto, hashcode )
	{
		this.mId = id;
		this.mTitle = title;
		this.mDescription = desc;
		this.mPercentDiscount = discount;
		this.mCreditItemId = creditItemId;
		this.mOfferCost = offerItemCost;
		this.mOfferItemTitle = offerItemTitle;
		this.mOfferItemDesc = offerItemDesc;
		this.mOfferItemProto = offerItemProto;
		this.mHashCode = hashcode;
		local itemData = this.split(offerItemProto, ":");

		if (itemData.len() > 2)
		{
			local itemDefId = this.Util.replace(itemData[0], "item", "");
			this.mItemDefId = itemDefId.tointeger();
			local count = itemData[2];
			this.mStackCount = count.tointeger();

			if (this.mStackCount > 0)
			{
				this.mStackCount = this.mStackCount + 1;
			}
		}

		this.mDescription = this.Util.replace(this.mDescription, "%%", this.mPercentDiscount + "%");
	}

	function getId()
	{
		return this.mId;
	}

	function getTitle()
	{
		return this.mTitle;
	}

	function getDescription()
	{
		return this.mDescription;
	}

	function getPercentDiscount()
	{
		return this.mPercentDiscount;
	}

	function getStackCount()
	{
		return this.mStackCount;
	}

	function getOfferItemTitle()
	{
		return this.mOfferItemTitle;
	}

	function getOfferItemDesc()
	{
		return this.mOfferItemDesc;
	}

	function getOfferItemFullPrice()
	{
		return this.mOfferCost;
	}

	function getOfferItemDiscountPrice()
	{
		local discountAmount = this.mOfferCost.tofloat() * this.mPercentDiscount.tofloat() * 0.0099999998;
		local discountPrice = this.mOfferCost - discountAmount + 0.5;
		discountPrice = discountPrice.tointeger();
		return discountPrice;
	}

	function getOfferItemDefId()
	{
		return this.mItemDefId;
	}

	function getHashCode()
	{
		return this.mHashCode;
	}

}

