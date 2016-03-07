this.ItemLabelType <- {
	NONE = 0,
	NEW = 1,
	HOT = 2,
	LIMITED = 3,
	SOLD_OUT = 4,
	EXPIRED = 5
};
class this.GUI.CreditShopItem extends this.GUI.Component
{
	static mClassName = "CreditShopItem";
	ItemTextColor = {
		[this.ItemLabelType.NONE] = {
			color = this.Colors.white,
			text = ""
		},
		[this.ItemLabelType.NEW] = {
			color = this.Colors["Bright Green"],
			text = "New!"
		},
		[this.ItemLabelType.HOT] = {
			color = this.Colors.red,
			text = "Hot!"
		},
		[this.ItemLabelType.LIMITED] = {
			color = this.Colors.yellow,
			text = "Ltd!"
		},
		[this.ItemLabelType.SOLD_OUT] = {
			color = this.Colors["Blue Steel"],
			text = "Sold Out"
		},
		[this.ItemLabelType.EXPIRED] = {
			color = this.Colors["Blue Steel"],
			text = "Expired"
		}
	};
	static BASE_WIDTH = 246;
	static BASE_HEIGHT = 138;
	static DESCRIPTION_WIDTH = 175;
	static DESCRIPTION_HEIGHT = 90;
	mCreditItem = null;
	mItemDefId = null;
	mItemTitle = null;
	mItemImageContainer = null;
	mItemLabel = null;
	mCredits = null;
	mCopper = null;
	mDescription = null;
	mMessageBroadcaster = null;
	mItemImage = null;
	mPreviousVisible = false;
	mBuyButton = null;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setGap(-5);
		this.setSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.setPreferredSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.setSelected(false);
		this.setInsets(0, 5, 0, 5);
		this.mItemTitle = this.GUI.Label("Title");
		this.mItemTitle.setSize(this.BASE_WIDTH - 10, 16);
		this.mItemTitle.setPreferredSize(this.BASE_WIDTH - 10, 16);
		this.mItemTitle.setFontColor(this.Colors.white);
		this.mItemTitle.setFont(::GUI.Font("Maiandra", 26, false));
		this.mItemTitle.setTextAlignment(0.5, 0.5);
		this.add(this.mItemTitle);
		local centerComp = this.GUI.Container(this.GUI.BoxLayout());
		centerComp.getLayoutManager().setGap(5);
		this.add(centerComp);
		local leftItemSide = this.GUI.Container(this.GUI.BoxLayoutV());
		leftItemSide.getLayoutManager().setAlignment(0.0);
		centerComp.add(leftItemSide);
		local itemContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		itemContainer.setSize(60, 65);
		itemContainer.setPreferredSize(60, 65);
		itemContainer.setInsets(5, 0, 5, 0);
		itemContainer.setAppearance("TextInputFields");
		itemContainer.getLayoutManager().setGap(5);
		leftItemSide.add(itemContainer);
		this.mItemImage = this.GUI.CreditShopItemImage();
		itemContainer.add(this.mItemImage);
		this.mItemLabel = this.GUI.Label(this.ItemTextColor[this.ItemLabelType.HOT].text);
		this.mItemLabel.setFont(::GUI.Font("Maiandra", 26, false));
		this.mItemLabel.setFontColor(this.ItemTextColor[this.ItemLabelType.HOT].color);
		itemContainer.add(this.mItemLabel);
		this.mCredits = this.GUI.Credits(99999);
		this.mCredits.setFontColor(this.Colors.white);
		
		this.mCopper = this.GUI.Currency(99999);
		this.mCopper.setFontColor(this.Colors.white);
		
		local currencyContainer = this.GUI.Container(this.GUI.BoxLayout());
		currencyContainer.getLayoutManager().setGap(5);
		currencyContainer.add(this.mCredits);
		currencyContainer.add(this.mCopper);
		
		this.mBuyButton = this.GUI.NarrowButton("BUY");
		this.mBuyButton.setFixedSize(62, 32);
		this.mBuyButton.addActionListener(this);
		this.mBuyButton.setPressMessage("_buyPressed");
		leftItemSide.add(this.mBuyButton);
		this.mDescription = this.GUI.HTML("");
		this.mDescription.setSize(this.DESCRIPTION_WIDTH, this.DESCRIPTION_HEIGHT);
		this.mDescription.setPreferredSize(this.DESCRIPTION_WIDTH, this.DESCRIPTION_HEIGHT);
		this.mDescription.setFont(::GUI.Font("Maiandra", 22, false));
		this.mDescription.setFontColor(this.Colors["GUI Gold"]);
		
		local descComp = this.GUI.Container(this.GUI.BoxLayoutV());
		descComp.getLayoutManager().setGap(5);
		descComp.add(this.mDescription);
		descComp.add(currencyContainer);
		
		centerComp.add(descComp);
		::_ItemDataManager.addListener(this);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setVisible(this.mPreviousVisible);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();

		if (this.mWidget)
		{
			this.mWidget.addListener(this);
		}
	}

	function _removeNotify()
	{
		if (this.mWidget)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setSelected( value )
	{
		if (value)
		{
			this.setAppearance("Panel");
			this.mMessageBroadcaster.broadcastMessage("creditShopItemSelected", this);
		}
		else
		{
			this.setAppearance("TradeBorder");
		}
	}

	function onMouseReleased( evt )
	{
		this.setSelected(true);
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function _buyPressed( evt )
	{
		this.setSelected(true);
		local callback = {
			creditItemId = this.mCreditItem.getCreditItemId(),
			function onActionSelected( mb, alt )
			{
				if (alt == "Yes")
				{
					if (::_creditShopManager)
					{
						::_creditShopManager.requestBuyItem(this.creditItemId);
					}
				}
			}

		};
		this.GUI.MessageBox.showYesNo("Are you sure you wish to purchase " + this.mCreditItem.getTitle() + "?", callback);
	}

	function updateItem( creditItem )
	{
		this.mCreditItem = creditItem;
		local visible = false;

		if (this.mCreditItem)
		{
			local labelType = creditItem.getItemLabelType();
			local titleText = creditItem.getTitle();
			local descriptionText = creditItem.getDescription();
			local itemDefId = creditItem.getItemDefId();
			local numStacks = creditItem.getNumCount();
			local copperAmount = creditItem.getPriceCopper();
			local creditAmount = creditItem.getPriceCredits();
			local currency = creditItem.getPriceCurrency();
			this.mItemDefId = itemDefId;

			if (itemDefId)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemDefId);

				if (itemDef)
				{
					this.mItemImage.setImage(itemDef.getIcon());
					this.mItemImage.updateItemDefId(itemDefId);
				}

				this.mItemImage.updateCount(numStacks);
				this._updateTitle(titleText.tostring());
				this._updateDescription(descriptionText.tostring());
				this._updateCost(copperAmount, creditAmount, currency);
				this._updateLabelType(labelType);
				visible = true;
			}
		}

		if (this.mPreviousVisible != visible)
		{
			this.setVisible(visible);
			this.mPreviousVisible = visible;
		}
	}

	function _updateTitle( title )
	{
		this.mItemTitle.setText(title);
	}

	function _updateDescription( description )
	{
		this.mDescription.setText(description);
	}

	function _updateCost(copperAmount, creditAmount, currency)
	{
		this.mCopper.setCurrentValue(copperAmount);
		this.mCredits.setCurrentValue(creditAmount);
		this.mCopper.setVisible(currency == 0 || currency == 2);
		this.mCredits.setVisible(currency == 1 || currency == 2);
	}

	function _updateLabelType( type )
	{
		if (type >= this.ItemLabelType.NONE && type <= this.ItemLabelType.EXPIRED)
		{
			this.mItemLabel.setText(this.ItemTextColor[type].text);
			this.mItemLabel.setFontColor(this.ItemTextColor[type].color);
		}
		else
		{
			this.mItemLabel.setText(this.ItemTextColor[this.ItemLabelType.NONE].text);
			this.mItemLabel.setFontColor(this.ItemTextColor[this.ItemLabelType.NONE].color);
		}

		if (type == this.ItemLabelType.EXPIRED || type == this.ItemLabelType.SOLD_OUT)
		{
			this.mBuyButton.setEnabled(false);
		}
		else
		{
			this.mBuyButton.setEnabled(true);
		}
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (this.mItemDefId == 0 || itemDefId != this.mItemDefId)
		{
			return;
		}

		local itemDef = ::_ItemDataManager.getItemDef(this.mItemDefId);

		if (itemDef)
		{
			local image = itemDef.getIcon();
			this.mItemImage.setImage(image);
		}
	}

}

class this.GUI.CreditShopItemImage extends this.GUI.Container
{
	static mClassName = "CreditShopItemImage";
	mBackgroundImage = null;
	mForegroundImage = null;
	mCountLabel = null;
	mIconSize = 32;
	mLastCombinedImageName = "";
	mItemDefId = 0;
	constructor()
	{
		this.GUI.Container.constructor(null);
		this.setSize(this.mIconSize, this.mIconSize);
		this.setPreferredSize(this.mIconSize, this.mIconSize);
		this.setChildrenInheritTooltip(true);
		this.mBackgroundImage = this.GUI.Image();
		this.mBackgroundImage.setLayoutManager(null);
		this.mBackgroundImage.setSize(this.mIconSize, this.mIconSize);
		this.add(this.mBackgroundImage);
		this.mForegroundImage = this.GUI.Image();
		this.mForegroundImage.setLayoutManager(null);
		this.mForegroundImage.setSize(this.mIconSize, this.mIconSize);
		this.mForegroundImage.setPosition(0, 0);
		this.mBackgroundImage.add(this.mForegroundImage);
		this.mCountLabel = this.GUI.Label("");
		this.mCountLabel.setSize(30, 16);
		this.mCountLabel.setPreferredSize(30, 16);
		this.mCountLabel.setTextAlignment(1.0, 1.0);
		this.mCountLabel.setFont(this.GUI.Font("MaiandraOutline", 14));
		this.mCountLabel.setFontColor("ffff00");
		this.mCountLabel.setPosition(0, 18);
		this.mForegroundImage.add(this.mCountLabel);
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setImage( image )
	{
		if (image == "" || image == this.mLastCombinedImageName)
		{
			return;
		}

		this.mLastCombinedImageName = image;
		local splitImages = this.Util.split(image, "|");
		local foregroundImage = splitImages[0];
		local backgroundImage = this.BackgroundImages.GREY;

		if (splitImages.len() > 1 && splitImages[1] != "")
		{
			if (splitImages[1].find(".png") != null)
			{
				backgroundImage = splitImages[1];
			}
			else
			{
				backgroundImage = this.BackgroundImages[splitImages[1].toupper()];
			}
		}

		this.mForegroundImage.setImageName(foregroundImage);
		this.mBackgroundImage.setImageName(backgroundImage);
	}

	function updateCount( count )
	{
		if (count > 1)
		{
			this.mCountLabel.setText(count.tostring());
		}
		else
		{
			this.mCountLabel.setText("");
		}
	}

	function updateItemDefId( itemDefId )
	{
		this.mItemDefId = itemDefId;
	}

	function onMouseEnter( evt )
	{
		local itemDef = ::_ItemDataManager.getItemDef(this.mItemDefId);

		if (itemDef)
		{
			local mods = {};
			mods.showBuyValue <- false;
			local tooltipComp = itemDef.getTooltip(mods);

			if (tooltipComp)
			{
				::Util.setCertainComponentVisible(tooltipComp, "LIFETIME_LABEL", true);
				::Util.setCertainComponentVisible(tooltipComp, "EXPIRE_LABEL", false);
			}

			this.setTooltip(tooltipComp);
		}
	}

}

class this.GUI.NineItemPanel extends this.GUI.Component
{
	static mClassName = "NineItemPanel";
	static NUM_ITEMS = 9;
	mCreditShopItems = null;
	mPageComponent = null;
	mKey = null;
	mCurrentItemSelected = null;
	constructor( TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT, TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y, BASE_HEIGHT, BASE_WIDTH )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.mCreditShopItems = [];
		local topPanelInsetSize = 10;
		this.setSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setPreferredSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setInsets(topPanelInsetSize, topPanelInsetSize, topPanelInsetSize, topPanelInsetSize);
		this.setPosition(TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y);
		this.getLayoutManager().setGap(5);
		local container = this.GUI.Container(this.GUI.GridLayout(3, 3));
		container.getLayoutManager().setRows(BASE_HEIGHT, BASE_HEIGHT, BASE_HEIGHT);
		container.getLayoutManager().setColumns(BASE_WIDTH, BASE_WIDTH, BASE_WIDTH);
		container.getLayoutManager().setGaps(12, 10);
		this.add(container);

		for( local i = 0; i < this.NUM_ITEMS; i++ )
		{
			local item = this.GUI.CreditShopItem();
			item.addActionListener(this);
			this.mCreditShopItems.append(item);
			container.add(item);
		}

		this.mPageComponent = this.GUI.PageComponent();
		this.mPageComponent.setTotalPages(4);
		this.mPageComponent.addActionListener(this);
		this.mPageComponent.setSize(TOP_CENTER_WIDTH, 40);
		this.mPageComponent.setPreferredSize(TOP_CENTER_WIDTH, 40);
		this.add(this.mPageComponent);
	}

	function creditShopItemSelected( itemSelected )
	{
		if (this.mCurrentItemSelected && this.mCurrentItemSelected != itemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = itemSelected;
	}

	function _updateViewItem( count, creditItem )
	{
		if (count >= 0 && count < this.mCreditShopItems.len())
		{
			this.mCreditShopItems[count].updateItem(creditItem);
		}
	}

	function resetItems( key, serverSpoofUpdate )
	{
		if (!serverSpoofUpdate)
		{
			foreach( shopItem in this.mCreditShopItems )
			{
				shopItem.setSelected(false);
			}
		}

		this.mKey = key;
		local items = ::_creditShopManager.getData(this.mKey);
		local totalPages = items.len() / this.NUM_ITEMS;
		local extraPage = items.len() % this.NUM_ITEMS;

		if (extraPage > 0)
		{
			totalPages = totalPages + 1;
		}

		this.mPageComponent.setTotalPages(totalPages);
		local lastSelectedPage = ::_creditShopManager.getLastPage(this.mKey);
		this.mPageComponent.setCurrentPage(lastSelectedPage);
		this.updateItems();
	}

	function updateItems()
	{
		foreach( key, items in this.mCreditShopItems )
		{
			this.mCreditShopItems[key].updateItem(null);
		}

		local currentPage = this.mPageComponent.getCurrentPage();

		if (currentPage == 0)
		{
			return;
		}

		::_creditShopManager.setLastPage(this.mKey, currentPage);
		local i = this.NUM_ITEMS * (currentPage - 1);
		local j = 0;
		local creditItemsTable = ::_creditShopManager.getData(this.mKey);
		local items = [];

		foreach( creditItem in creditItemsTable )
		{
			items.append(creditItem);
		}

		while (i < items.len() && j < this.mCreditShopItems.len())
		{
			this._updateViewItem(j, items[i]);
			j = j + 1;
			i++;
		}
		
	}

	function onPreviousButtonPressed( button )
	{
		if (this.mCurrentItemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = null;
		this.updateItems();
	}

	function onNextButtonPressed( button )
	{
		if (this.mCurrentItemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = null;
		this.updateItems();
	}

}

class this.GUI.CreditPreviewPanel extends this.GUI.Component
{
	static mClassName = "CreditPreviewPanel";
	static NUM_ITEMS = 6;
	mCreditShopItems = null;
	mKey = null;
	mPageComponent = null;
	mCurrentItemSelected = null;
	mPreviewSpacer = null;
	mPreviewImage = null;
	mPreviewLabel = null;
	mPreviewContainer = null;
	mPreviewsLoaded = false;
	constructor( TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT, TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y, BASE_HEIGHT, BASE_WIDTH )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.mCreditShopItems = [];
		local topPanelInsetSize = 10;
		local X_GAP_SIZE = 12;
		local Y_GAP_SIZE = 10;
		this.setSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setPreferredSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setInsets(topPanelInsetSize, topPanelInsetSize, topPanelInsetSize, topPanelInsetSize);
		this.setPosition(TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y);
		this.getLayoutManager().setGap(5);
		local itemPreviewContainer = this.GUI.Container(this.GUI.BoxLayout());
		itemPreviewContainer.getLayoutManager().setGap(X_GAP_SIZE);
		this.add(itemPreviewContainer);
		local container = this.GUI.Container(this.GUI.GridLayout(3, 2));
		container.getLayoutManager().setRows(BASE_HEIGHT, BASE_HEIGHT, BASE_HEIGHT);
		container.getLayoutManager().setColumns(BASE_WIDTH, BASE_WIDTH);
		container.getLayoutManager().setGaps(X_GAP_SIZE, Y_GAP_SIZE);
		itemPreviewContainer.add(container);

		for( local i = 0; i < this.NUM_ITEMS; i++ )
		{
			local item = this.GUI.CreditShopItem();
			item.addActionListener(this);
			this.mCreditShopItems.append(item);
			container.add(item);
		}

		::_ItemDataManager.addListener(this);
		this.mPreviewContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mPreviewContainer.setSize(BASE_WIDTH, (BASE_HEIGHT + Y_GAP_SIZE) * 3 - Y_GAP_SIZE);
		this.mPreviewContainer.setPreferredSize(BASE_WIDTH, (BASE_HEIGHT + Y_GAP_SIZE) * 3 - Y_GAP_SIZE);
		itemPreviewContainer.add(this.mPreviewContainer);
		this.mPreviewLabel = this.GUI.Label("Preview");
		this.mPreviewLabel.setFontColor(this.Colors.white);
		this.mPreviewLabel.setFont(::GUI.Font("Maiandra", 30, false));
		this.mPreviewLabel.setTextAlignment(0.5, 0.5);
		this.mPageComponent = this.GUI.PageComponent();
		this.mPageComponent.addActionListener(this);
		this.mPageComponent.setSize(TOP_CENTER_WIDTH, 40);
		this.mPageComponent.setPreferredSize(TOP_CENTER_WIDTH, 40);
		this.add(this.mPageComponent);
	}

	function creditShopItemSelected( itemSelected )
	{
		if (this.mCurrentItemSelected && this.mCurrentItemSelected != itemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = itemSelected;
		local itemDef = ::_ItemDataManager.getItemDef(itemSelected.mItemDefId);
		this.updatePreviewImage(itemDef);
	}

	function getPreviewsLoaded()
	{
		return this.mPreviewsLoaded;
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		if (this.mPreviewsLoaded && this.mCurrentItemSelected && itemDefId == this.mCurrentItemSelected.mItemDefId)
		{
			local itemDef = ::_ItemDataManager.getItemDef(itemDefId);

			if (itemDef.isValid())
			{
				this.updatePreviewImage(itemDef);
			}
		}
	}

	function onPreviousButtonPressed( button )
	{
		if (this.mCurrentItemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = null;
		this.updatePreviewImage(null);
		this.updateItems();
	}

	function onNextButtonPressed( button )
	{
		if (this.mCurrentItemSelected)
		{
			this.mCurrentItemSelected.setSelected(false);
		}

		this.mCurrentItemSelected = null;
		this.updatePreviewImage(null);
		this.updateItems();
	}

	function resetItems( key, serverSpoofUpdate )
	{
		if (!serverSpoofUpdate)
		{
			foreach( shopItem in this.mCreditShopItems )
			{
				shopItem.setSelected(false);
			}

			if (this.mPreviewImage && (this.mPreviewImage instanceof this.GUI.Image))
			{
				this.mPreviewImage.setImageName("ArmorPreview-Default.png");
			}
		}

		this.mKey = key;
		local items = ::_creditShopManager.getData(this.mKey);
		local totalPages = items.len() / this.NUM_ITEMS;
		local extraPage = items.len() % this.NUM_ITEMS;

		if (extraPage > 0)
		{
			totalPages = totalPages + 1;
		}

		this.mPageComponent.setTotalPages(totalPages);
		local lastSelectedPage = ::_creditShopManager.getLastPage(this.mKey);
		this.mPageComponent.setCurrentPage(lastSelectedPage);
		this.updateItems();
	}

	function setPreviewsLoaded( value )
	{
		this.mPreviewsLoaded = value;
	}

	function updatePreviewImage( itemDef )
	{
		if (itemDef == null || !this.mPreviewsLoaded)
		{
			if (!this.mPreviewImage || (this.mPreviewImage instanceof this.GUI.Image))
			{
				if (this.mPreviewImage)
				{
					this.mPreviewContainer.remove(this.mPreviewImage);
					this.mPreviewContainer.remove(this.mPreviewLabel);
					this.mPreviewContainer.remove(this.mPreviewSpacer);
				}

				this.mPreviewSpacer = this.GUI.Spacer(0, 176);
				this.mPreviewContainer.add(this.mPreviewSpacer);
				this.mPreviewImage = this.GUI.ProgressAnimation();
				this.mPreviewImage.setSize(64, 64);
				this.mPreviewImage.setPreferredSize(64, 64);
				this.mPreviewContainer.add(this.mPreviewImage);
				this.mPreviewLabel.setText("Loading...");
				this.mPreviewContainer.add(this.mPreviewLabel);
			}
		}
		else if (itemDef.isValid())
		{
			if (!this.mPreviewImage || (this.mPreviewImage instanceof this.GUI.ProgressAnimation))
			{
				if (this.mPreviewImage)
				{
					this.mPreviewContainer.remove(this.mPreviewImage);
					this.mPreviewContainer.remove(this.mPreviewLabel);
					this.mPreviewContainer.remove(this.mPreviewSpacer);
				}

				this.mPreviewImage = this.GUI.Image();
				local size = this.mPreviewContainer.getSize();
				this.mPreviewImage.setSize(size.width, size.height - 15);
				this.mPreviewImage.setPreferredSize(size.width, size.height - 15);
				this.mPreviewContainer.add(this.mPreviewImage);
				this.mPreviewSpacer = this.GUI.Spacer(0, 5);
				this.mPreviewContainer.add(this.mPreviewSpacer);
				this.mPreviewLabel.setText("Preview");
				this.mPreviewContainer.add(this.mPreviewLabel);
			}

			local imageName = "ArmorPreview-" + itemDef.getDisplayName();
			imageName = this.Util.replace(imageName, " ", "_");
			this.mPreviewImage.setImageName(imageName + ".png");
		}
	}

	function updateItems()
	{
		foreach( key, items in this.mCreditShopItems )
		{
			this.mCreditShopItems[key].updateItem(null);
		}

		local currentPage = this.mPageComponent.getCurrentPage();
		local i = this.NUM_ITEMS * (currentPage - 1);
		local j = 0;

		if (currentPage == 0)
		{
			return;
		}

		::_creditShopManager.setLastPage(this.mKey, currentPage);
		local creditItemsTable = ::_creditShopManager.getData(this.mKey);
		local items = [];

		foreach( creditItem in creditItemsTable )
		{
			items.append(creditItem);
		}

		while (i < items.len() && j < this.mCreditShopItems.len())
		{
			this._updateViewItem(j, items[i]);
			j = j + 1;
			i++;
		}
	}

	function _updateViewItem( count, creditItem )
	{
		if (count >= 0 && count < this.mCreditShopItems.len())
		{
			this.mCreditShopItems[count].updateItem(creditItem);
		}
	}

}

class this.GUI.CreditNameChangePanel extends this.GUI.Component
{
	static mClassName = "CreditPreviewPanel";
	static NUM_ITEMS = 6;
	static CENTER_WIDTH = 463;
	static CENTER_HEIGHT = 290;
	static FONT_COLOR = this.Colors.white;
	static CHARACTER_LIMIT = 30;
	mApprovalText = null;
	mCredits = null;
	mLastCheckedName = null;
	mNameInput = null;
	mCheckValidNameEvent = null;
	mPurchaseButton = null;
	constructor( TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT, TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y, BASE_HEIGHT, BASE_WIDTH )
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		local topPanelInsetSize = 10;
		this.setSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setPreferredSize(TOP_CENTER_WIDTH, TOP_CENTER_HEIGHT);
		this.setInsets(75, 0, 0, 0);
		this.setPosition(TOP_POSITION_OFFSET_X, TOP_POSITION_OFFSET_Y);
		local centerContainer = this.GUI.Container(null);
		centerContainer.setSize(this.CENTER_WIDTH, this.CENTER_HEIGHT);
		centerContainer.setPreferredSize(this.CENTER_WIDTH, this.CENTER_HEIGHT);
		this.add(centerContainer);
		local titleLabel = this.GUI.Label("Last Name Change");
		titleLabel.setSize(this.CENTER_WIDTH, 40);
		titleLabel.setPreferredSize(this.CENTER_WIDTH, 40);
		titleLabel.setFontColor(this.FONT_COLOR);
		titleLabel.setFont(::GUI.Font("Maiandra", 36, false));
		titleLabel.setTextAlignment(0.5, 0.5);
		centerContainer.add(titleLabel);
		local enterNewLabel = this.GUI.Label("Enter New Last Name (" + this.CHARACTER_LIMIT + " Character Limit for First and Last Name)");
		enterNewLabel.setSize(this.CENTER_WIDTH, 30);
		enterNewLabel.setPreferredSize(this.CENTER_WIDTH, 30);
		enterNewLabel.setFontColor(this.FONT_COLOR);
		enterNewLabel.setFont(::GUI.Font("Maiandra", 30, false));
		enterNewLabel.setTextAlignment(0, 0.5);
		enterNewLabel.setPosition(0, 60);
		centerContainer.add(enterNewLabel);
		this.mNameInput = this.GUI.InputArea();
		this.mNameInput.setSize(325, 32);
		this.mNameInput.setPreferredSize(325, 32);
		this.mNameInput.setPosition(0, 92);
		this.mNameInput.setFont(this.GUI.Font("Maiandra", 28));
		this.mNameInput.addActionListener(this);
		this.mNameInput.setAutoCapitalize(true);
		this.mNameInput.setAllowOnlyLetters(true);
		this.mNameInput.setAllowSpaces(false);
		centerContainer.add(this.mNameInput);
		this.mApprovalText = this.GUI.InputArea();
		this.mApprovalText.setSize(457, 114);
		this.mApprovalText.setPreferredSize(457, 114);
		this.mApprovalText.setPosition(0, 135);
		this.mApprovalText.setLocked(true);
		this.mApprovalText.setAllowTextEntryOnClick(false);
		this.mApprovalText.setMultiLine(true);
		this.mApprovalText.setFont(this.GUI.Font("Maiandra", 28));
		centerContainer.add(this.mApprovalText);
		this.mPurchaseButton = this.GUI.NarrowButton("Purchase");
		this.mPurchaseButton.setFixedSize(93, 32);
		this.mPurchaseButton.setPosition(146, 258);
		this.mPurchaseButton.setFont(::GUI.Font("Maiandra", 22, false));
		this.mPurchaseButton.addActionListener(this);
		this.mPurchaseButton.setPressMessage("_purchasePressed");
		centerContainer.add(this.mPurchaseButton);
		this.mCredits = this.GUI.Credits(99999);
		this.mCredits.setFontColor(this.Colors.white);
		this.mCredits.setPosition(247, 259);
		this.mCredits.setSize(156, 24);
		this.mCredits.setPreferredSize(156, 24);
		centerContainer.add(this.mCredits);
		this.updateCredits(::_creditShopManager.getNameChangeCost());

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			local creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
			this.updatePurchaseButton(creditAmt);
		}
	}

	function updateCredits( amount )
	{
		this.mCredits.setCurrentValue(amount);
	}

	function updatePurchaseButton( value )
	{
		if (value >= this.mCredits.getCurrentValue())
		{
			this.mPurchaseButton.setEnabled(true);
		}
		else
		{
			this.mPurchaseButton.setEnabled(false);
		}
	}

	function updateApprovalText( text )
	{
		this.mApprovalText.setText(text);
	}

	function onInputComplete( inputbox )
	{
		this._validateName();
	}

	function onTextChanged( inputbox )
	{
		if (this.mCheckValidNameEvent)
		{
			::_eventScheduler.cancel(this.mCheckValidNameEvent);
		}

		this.mCheckValidNameEvent = ::_eventScheduler.fireIn(1.0, this, "_validateName");
	}

	function onClose()
	{
		if (this.mCheckValidNameEvent)
		{
			::_eventScheduler.cancel(this.mCheckValidNameEvent);
		}
	}

	function _validateName()
	{
		if (this.mCheckValidNameEvent)
		{
			::_eventScheduler.cancel(this.mCheckValidNameEvent);
		}

		if (!::_avatar || !::_avatar.getStat(this.Stat.DISPLAY_NAME) || this.mLastCheckedName == this.mNameInput.getText())
		{
			return;
		}

		local name = ::_avatar.getStat(this.Stat.DISPLAY_NAME);
		name = this.split(name, " ");
		local firstName = name[0];
		local lastName = this.mLastCheckedName = this.mNameInput.getText();
		::_Connection.sendQuery("validate.name", this, firstName, lastName);
	}

	function _purchasePressed( button )
	{
		if (!::_avatar || !::_avatar.getStat(this.Stat.DISPLAY_NAME))
		{
			return;
		}

		local name = ::_avatar.getStat(this.Stat.DISPLAY_NAME);
		name = this.split(name, " ");
		local firstName = name[0];
		local lastName = this.mLastCheckedName = this.mNameInput.getText();
		::_Connection.sendQuery("item.market.purchase.name", this, firstName, lastName);
	}

	function onQueryComplete( qa, results )
	{
		switch(qa.query)
		{
		case "validate.name":
			this.updateCredits(::_creditShopManager.getNameChangeCost());
			this.updateApprovalText("Name Approved");
			break;

		case "item.market.purchase.name":
			if (qa.args.len() > 1)
			{
				local firstName = qa.args[0];
				local lastName = qa.args[1];
				local message = "Your name has been changed to " + firstName + " " + lastName + ".";
				this.IGIS.info(message);
				this.updateApprovalText(message);
			}

			break;

		default:
			break;
		}
	}

	function onQueryError( qa, error )
	{
		this.updateApprovalText(error);

		if (qa.query == "item.market.purchase.name")
		{
			local message = "Purchasing new name failed.  " + error;
			this.IGIS.error(message);
		}
		else
		{
		}
	}

}

