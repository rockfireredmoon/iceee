this.require("Constants");
class this.GUI.SpecialOfferNotification extends this.GUI.Component
{
	static mScreenName = "SpecialOfferNotification";
	static C_BASE_WIDTH = 190;
	static C_BASE_HEIGHT = 70;
	mSpecialOfferItem = null;
	mDragging = false;
	mMouseOffset = {};
	constructor( offerItem )
	{
		this.GUI.Component.constructor(null);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT + 20);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT + 20);
		local baseFrame = this.GUI.Frame("Special Offer");
		baseFrame.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		baseFrame.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		baseFrame.setMovable(false);
		baseFrame.setVisible(true);
		this.add(baseFrame);
		local baseComponent = this.GUI.Component(null);
		baseFrame.setContentPane(baseComponent);
		local titleHTML = this.GUI.HTML(offerItem.getTitle());
		titleHTML.getLayoutManager().setAlignment("center");
		titleHTML.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT - 30);
		titleHTML.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT - 30);
		titleHTML.setFont(::GUI.Font("Maiandra", 28, false));
		titleHTML.setFontColor(this.Colors.lavender);
		baseComponent.add(titleHTML);
		local viewComp = this.GUI.InnerPanel(this.GUI.BoxLayoutV());
		viewComp.setInsets(7, 3, 0, 3);
		viewComp.setAppearance("GoldBorder");
		viewComp.setSize(85, 40);
		viewComp.setPreferredSize(85, 40);
		viewComp.setPosition(57, 55);
		this.add(viewComp);
		local narrowButton = this.GUI.BlueNarrowButton("View");
		narrowButton.setFixedSize(75, 32);
		narrowButton.addActionListener(this);
		narrowButton.setPressMessage("_viewPressed");
		viewComp.add(narrowButton);
		this.mSpecialOfferItem = offerItem;
		local sz = this.getSize();
		this.setPosition((this.Screen.getWidth() - sz.width) / 2, 5);
		baseFrame.addActionListener(this);
		this._exitGameStateRelay.addListener(this);
		this.setOverlay("GUI/SpecialPurchaseOffer");
	}

	function _viewPressed( button )
	{
		local offerWindow = this.GUI.SpecialOfferWindow(this.mSpecialOfferItem);
		offerWindow.setVisible(true);
		this.destroy();
	}

	function onExitGame()
	{
		this.destroy();
	}

	function onFrameClosed()
	{
		this._specialOfferManager.removeSpecialOffer(this.mSpecialOfferItem.getId());
		this.destroy();
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local newpos = ::Screen.getCursorPos();
			local deltax = newpos.x - this.mMouseOffset.x;
			local deltay = newpos.y - this.mMouseOffset.y;
			local pos = this.getPosition();
			pos.x += deltax;
			pos.y += deltay;
			this.mMouseOffset = newpos;
			this.setPosition(pos);
			this.fitToScreen();
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mMouseOffset = ::Screen.getCursorPos();
			this.mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mDragging = false;
			evt.consume();
		}
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

}

class this.GUI.SpecialOfferWindow extends this.GUI.Container
{
	static mScreenName = "SpecialOfferWindow";
	static C_BASE_WIDTH = 610;
	static C_BASE_HEIGHT = 425;
	static C_TITLE_WIDTH = 465;
	mSpecialOfferItem = null;
	mDiscountText = null;
	mRegularPriceEndText = null;
	mCurrentCreditLabel = null;
	mDragging = false;
	mMouseOffset = {};
	constructor( offerItem )
	{
		this.mSpecialOfferItem = offerItem;
		this.GUI.Container.constructor(this.GUI.BoxLayoutV());
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setAppearance("PurchaseBorder");
		local spacerTemp = this.GUI.Spacer(10, 9);
		this.add(spacerTemp);
		local topTitleHTML = this.GUI.HTML("Special Offer");
		topTitleHTML.setSize(this.C_TITLE_WIDTH, 30);
		topTitleHTML.setPreferredSize(this.C_TITLE_WIDTH, 30);
		topTitleHTML.setFont(::GUI.Font("Maiandra", 28, false));
		topTitleHTML.setFontColor(this.Colors.mint);
		topTitleHTML.getLayoutManager().setAlignment("center");
		this.add(topTitleHTML);
		local titleHTML = this.GUI.HTML(this.mSpecialOfferItem.getTitle());
		titleHTML.setSize(this.C_TITLE_WIDTH, 34);
		titleHTML.setPreferredSize(this.C_TITLE_WIDTH, 34);
		titleHTML.setFont(::GUI.Font("Maiandra", 34, false));
		titleHTML.setFontColor(this.Colors.lavender);
		titleHTML.getLayoutManager().setAlignment("center");
		this.add(titleHTML);
		local descriptionHTML = this.GUI.HTML(this.mSpecialOfferItem.getDescription());
		descriptionHTML.setSize(this.C_TITLE_WIDTH, 110);
		descriptionHTML.setPreferredSize(this.C_TITLE_WIDTH, 110);
		descriptionHTML.setFont(::GUI.Font("Maiandra", 34, false));
		descriptionHTML.setFontColor(this.Colors.white);
		this.add(descriptionHTML);
		local itemPanel = this.GUI.Container(this.GUI.BoxLayoutV());
		itemPanel.setAppearance("TradeBorder");
		itemPanel.getLayoutManager().setGap(5);
		itemPanel.setInsets(7, 0, 0, 0);
		itemPanel.setSize(500, 150);
		itemPanel.setPreferredSize(500, 150);
		this.add(itemPanel);
		local specialItemComp = this.GUI.SpecialItemComp(this.mSpecialOfferItem);
		itemPanel.add(specialItemComp);
		local bottomComp = this.GUI.Component(this.GUI.BoxLayout());
		bottomComp.setSize(495, 86);
		bottomComp.setPreferredSize(495, 86);
		itemPanel.add(bottomComp);
		local priceComp = this.GUI.Component(null);
		priceComp.setSize(290, 86);
		priceComp.setPreferredSize(290, 86);
		priceComp.setInsets(0, 0, 0, 15);
		bottomComp.add(priceComp);
		this.mDiscountText = this.GUI.HTML("Default");
		this.mDiscountText.setSize(290, 30);
		this.mDiscountText.setPreferredSize(290, 30);
		this.mDiscountText.setPosition(0, 10);
		this.mDiscountText.setFont(::GUI.Font("Maiandra", 28, false));
		this.mDiscountText.setFontColor(this.Colors.mint);
		this.mDiscountText.getLayoutManager().setAlignment("center");
		priceComp.add(this.mDiscountText);
		local regularPriceComp = this.GUI.Container(this.GUI.BoxLayout());
		regularPriceComp.setSize(270, 30);
		regularPriceComp.setPreferredSize(270, 30);
		regularPriceComp.setPosition(25, 35);
		priceComp.add(regularPriceComp);
		local regularPriceText = this.GUI.HTML("(Regular price is ");
		regularPriceText.setSize(124, 30);
		regularPriceText.setPreferredSize(124, 30);
		regularPriceText.setFont(::GUI.Font("Maiandra", 28, false));
		regularPriceText.setFontColor(this.Colors.white);
		regularPriceComp.add(regularPriceText);
		local creditIcon = this.GUI.Container();
		creditIcon.setAppearance("Credit");
		creditIcon.setPreferredSize(16, 16);
		regularPriceComp.add(creditIcon);
		this.mRegularPriceEndText = this.GUI.HTML(" credits.)");
		this.mRegularPriceEndText.setFontColor(this.Colors.white);
		this.mRegularPriceEndText.setFont(::GUI.Font("Maiandra", 28, false));
		this.mRegularPriceEndText.setSize(140, 30);
		this.mRegularPriceEndText.setPreferredSize(140, 30);
		regularPriceComp.add(this.mRegularPriceEndText);
		local buttonHolder = this.GUI.Component(this.GUI.BoxLayoutV());
		buttonHolder.setSize(200, 86);
		buttonHolder.setPreferredSize(200, 86);
		buttonHolder.setInsets(4, 0, -5, 0);
		buttonHolder.getLayoutManager().setGap(-2);
		bottomComp.add(buttonHolder);
		local buyNowButton = this.GUI.LtGreenBigButton("Buy Now!");
		buyNowButton.setFixedSize(178, 50);
		buyNowButton.addActionListener(this);
		buyNowButton.setPressMessage("_buyNowPressed");
		buttonHolder.add(buyNowButton);
		local declineButton = this.GUI.RedNarrowButton("Decline");
		declineButton.setFixedSize(70, 32);
		declineButton.addActionListener(this);
		declineButton.setPressMessage("_declinePressed");
		buttonHolder.add(declineButton);
		local creditAndLink = this.GUI.Component(this.GUI.BoxLayout());
		creditAndLink.setSize(500, 38);
		creditAndLink.setPreferredSize(500, 38);
		creditAndLink.getLayoutManager().setGap(15);
		creditAndLink.setInsets(10, 0, 0, 100);
		this.add(creditAndLink);
		local creditAmt = 0;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		this.mCurrentCreditLabel = this.GUI.Label("Current Credits: " + creditAmt);
		this.mCurrentCreditLabel.setFontColor(this.Colors.white);
		this.mCurrentCreditLabel.setFont(::GUI.Font("Maiandra", 22, false));
		creditAndLink.add(this.mCurrentCreditLabel);
		local buyCreditButton = this.GUI.BuyCreditButton("");
		buyCreditButton.addActionListener(this);
		buyCreditButton.setPressMessage("_onBuyCreditLinkPressed");
		creditAndLink.add(buyCreditButton);
		this.setOverlay(this.GUI.OVERLAY);
		this._updateDiscountText();
		this._updateRegularPriceCredit();
		this.centerOnScreen();
		this._exitGameStateRelay.addListener(this);
		::_Connection.addListener(this);
	}

	function onExitGame()
	{
		this.destroy();
	}

	function _onBuyCreditLinkPressed( button )
	{
		local callback = {
			function onActionSelected( mb, alt )
			{
				if (alt == "Ok")
				{
					this.System.openURL("http://www.eartheternal.com/credits");
				}
			}

		};
		this.GUI.MessageBox.showOkCancel("This will open a new browser window.", callback);
	}

	function onCreditUpdated( value )
	{
		this.mCurrentCreditLabel.setText("Current Credits: " + value);
	}

	function _buyNowPressed( button )
	{
		local itemCost = this.mSpecialOfferItem.getOfferItemDiscountPrice();
		local creditAmt = 1000000;

		if (::_avatar && ::_avatar.getStat(this.Stat.CREDITS) != null)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		if (creditAmt >= itemCost)
		{
			local purchaseMessageBox = this.GUI.MessageBox.showEx("Are you sure you wish to purchase this item?", [
				"Purchase",
				"Decline"
			], this, "_onPurchaseSelected");
			purchaseMessageBox.setButtonColor(0, "Blue");
			purchaseMessageBox.setButtonColor(1, "Red");
			purchaseMessageBox.setButtonSize(0, 90, 32);
			purchaseMessageBox.setButtonSize(1, 70, 32);
		}
		else
		{
			local messageBox = this.GUI.MessageBox.showEx("You currently do not have enough Credits to make<br>this purchase.  " + "Would you like to buy more Credits?<br>Current credits: " + creditAmt, [
				"Buy Credits!",
				"Decline"
			], this, "_onBuyCredits");
			messageBox.setButtonColor(0, "Blue");
			messageBox.setButtonColor(1, "Red");
			messageBox.setButtonSize(0, 100, 32);
			messageBox.setButtonSize(1, 70, 32);
		}
	}

	function _onPurchaseSelected( source, action )
	{
		switch(action)
		{
		case "Purchase":
			if (this.isInventoryFull())
			{
				local messageBox = this.GUI.MessageBox.showEx("Cannot complete purchase.<br>   Your inventory is full.", [
					"Continue"
				], this);
				messageBox.setButtonColor(0, "Blue");
				messageBox.setButtonSize(0, 70, 32);
				messageBox.setButtonSize(1, 70, 32);
			}
			else
			{
				::_Connection.sendQuery("special.offer.buy", this, this.mSpecialOfferItem.getHashCode(), this.mSpecialOfferItem.getId(), this.mSpecialOfferItem.getOfferItemTitle());
			}

			break;

		default:
			if (action == "Decline")
			{
			}
		}
	}

	function _onBuyCredits( source, action )
	{
		switch(action)
		{
		case "Buy Credits!":
			this._onBuyCreditLinkPressed(null);
			break;

		default:
			if (action == "Decline")
			{
			}
		}
	}

	function isInventoryFull()
	{
		local vault = this.Screens.get("Vault", false);

		if (vault && vault.isVisible())
		{
			local freeslots = vault.getFreeSlotsRemaining();

			if (freeslots > 0)
			{
				return false;
			}
		}

		local inventory = this.Screens.get("Inventory", true);

		if (inventory)
		{
			if (inventory.getFreeSlotsRemaining() > 0)
			{
				return false;
			}
		}

		return true;
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "special.offer.buy")
		{
			if (qa.args.len() > 2)
			{
				local itemTitle = qa.args[2];
				this.IGIS.info("You have purchased " + itemTitle + " and it has been placed in your inventory!");
			}
			else
			{
				this.IGIS.info("Purchased item successfully.");
			}

			this.removeOfferCloseWindow();
		}
		else
		{
			  // [027]  OP_JMP            0      0    0    0
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

	function _declinePressed( button )
	{
		this.removeOfferCloseWindow();
	}

	function removeOfferCloseWindow()
	{
		this._specialOfferManager.removeSpecialOffer(this.mSpecialOfferItem.getId());
		this.destroy();
	}

	function _updateDiscountText()
	{
		this.mDiscountText.setText("Buy now and save " + this.mSpecialOfferItem.getPercentDiscount() + "% off");
	}

	function _updateRegularPriceCredit()
	{
		this.mRegularPriceEndText.setText(this.mSpecialOfferItem.getOfferItemFullPrice() + " credits.)");
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local newpos = ::Screen.getCursorPos();
			local deltax = newpos.x - this.mMouseOffset.x;
			local deltay = newpos.y - this.mMouseOffset.y;
			local pos = this.getPosition();
			pos.x += deltax;
			pos.y += deltay;
			this.mMouseOffset = newpos;
			this.setPosition(pos);
			this.fitToScreen();
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mMouseOffset = ::Screen.getCursorPos();
			this.mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mDragging = false;
			evt.consume();
		}
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

}

class this.GUI.SpecialItemComp extends this.GUI.Component
{
	static mScreenName = "SpecialItemComp";
	static C_BASE_WIDTH = 488;
	static C_BASE_HEIGHT = 50;
	mSpecialOfferItem = null;
	mItemImage = null;
	mItemTitle = null;
	mDescription = null;
	mCredits = null;
	mItemDefId = -1;
	constructor( offerItem )
	{
		this.mSpecialOfferItem = offerItem;
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.getLayoutManager().setGap(20);
		this.setSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setPreferredSize(this.C_BASE_WIDTH, this.C_BASE_HEIGHT);
		this.setAppearance("TextInputFields");
		this.setInsets(-2, 0, 0, 10);
		this.mItemImage = this.GUI.CreditShopItemImage();
		this.add(this.mItemImage);
		local TITLE_DESC_WIDTH = 320;
		local titleDescComp = this.GUI.Component(this.GUI.BoxLayoutV());
		titleDescComp.setSize(TITLE_DESC_WIDTH, 50);
		titleDescComp.setPreferredSize(TITLE_DESC_WIDTH, 50);
		this.add(titleDescComp);
		this.mItemTitle = this.GUI.HTML("Default");
		this.mItemTitle.setSize(TITLE_DESC_WIDTH, 18);
		this.mItemTitle.setPreferredSize(TITLE_DESC_WIDTH, 18);
		this.mItemTitle.setFont(::GUI.Font("Maiandra", 24, false));
		this.mItemTitle.setFontColor(this.Colors.white);
		this.mItemTitle.getLayoutManager().setAlignment("center");
		titleDescComp.add(this.mItemTitle);
		this.mDescription = this.GUI.HTML("Default");
		this.mDescription.setSize(TITLE_DESC_WIDTH, 36);
		this.mDescription.setPreferredSize(TITLE_DESC_WIDTH, 36);
		this.mDescription.setFont(::GUI.Font("Maiandra", 18, false));
		this.mDescription.setFontColor(this.Colors["GUI Gold"]);
		this.mDescription.getLayoutManager().setAlignment("center");
		this.mDescription.getLayoutManager().setGaps(0, -5);
		titleDescComp.add(this.mDescription);
		this.mCredits = this.GUI.Credits(9999);
		this.mCredits.setFont(::GUI.Font("Maiandra", 34, false));
		this.mCredits.setSize(80, 40);
		this.mCredits.setPreferredSize(80, 40);
		this.mCredits.setAlignment(1.0);
		this.add(this.mCredits);
		this.updateItem();
		::_ItemDataManager.addListener(this);
	}

	function updateItem()
	{
		if (this.mSpecialOfferItem)
		{
			local titleText = this.mSpecialOfferItem.getOfferItemTitle();
			local descriptionText = this.mSpecialOfferItem.getOfferItemDesc();
			local itemDefId = this.mSpecialOfferItem.getOfferItemDefId();
			local numStacks = this.mSpecialOfferItem.getStackCount();
			local creditAmount = this.mSpecialOfferItem.getOfferItemDiscountPrice();
			this.mItemDefId = itemDefId;

			if (itemDefId)
			{
				local itemDef = ::_ItemDataManager.getItemDef(itemDefId);

				if (itemDef)
				{
					this.mItemImage.setImage(itemDef.getIcon());
					this.mItemImage.updateItemDefId(itemDefId);
				}
			}

			this.mItemImage.updateCount(numStacks);
			this._updateTitle(titleText.tostring());
			this._updateDescription(descriptionText.tostring());
			this._updateCredits(creditAmount);
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

	function _updateCredits( amount )
	{
		this.mCredits.setCurrentValue(amount);
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

