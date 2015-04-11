this.require("UI/Screens");
this.require("GUI/Frame");
this.TradeStatus <- {
	PREPARING = 0,
	OFFER_MADE = 1,
	WAITING_ACCEPTANCE = 2,
	ACCEPTED = 3
};
class this.Screens.TradeScreen extends this.GUI.Frame
{
	static tradeDistance = 30;
	static mClassName = "Screens.TradeScreen";
	mOtherPlayerID = null;
	mOtherPlayerLabel = null;
	mOtherPlayerCurrency = null;
	mStatusLabel = null;
	mOtherPlayerInv = null;
	mMyLabel = null;
	mMyInv = null;
	mMyCurrency = null;
	mAcceptButton = null;
	mGlowImage = null;
	mPlayerGlowImage = null;
	static INNER_PANEL_WIDTH = 234;
	static INNER_PANEL_HEIGHT = 367;
	static SCREEN_WIDTH = 500;
	static mInfoPanelSlotHeight = 43;
	static mInfoPanelSlotWidth = 217;
	static NUM_PANEL_SLOTS = 7;
	constructor()
	{
		this.GUI.Frame.constructor(this.TXT("Trade"));
		this.setSize(this.SCREEN_WIDTH, 440);
		this.setInsets(0, 0, 0, 0);
		this.setPosition(40, 40);
		local baseContainer = this.GUI.Container(this.GUI.BoxLayout());
		baseContainer.setSize(this.SCREEN_WIDTH, 400);
		baseContainer.setPreferredSize(this.SCREEN_WIDTH, 400);
		baseContainer.setInsets(5, 8.5, 0, 8.5);
		baseContainer.getLayoutManager().setGap(15);
		local outerContainer = this.GUI.Container(null);
		outerContainer.setSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT + 43);
		outerContainer.setPreferredSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT + 43);
		baseContainer.add(outerContainer);
		local otherPlayerContainer = this.GUI.InnerPanel(null);
		otherPlayerContainer.setSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT);
		otherPlayerContainer.setPreferredSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT);
		outerContainer.add(otherPlayerContainer);
		this.mGlowImage = this.GUI.Component(null);
		this.mGlowImage.setSize(this.INNER_PANEL_WIDTH - 4, this.INNER_PANEL_HEIGHT - 4);
		this.mGlowImage.setPreferredSize(this.INNER_PANEL_WIDTH - 4, this.INNER_PANEL_HEIGHT - 4);
		this.mGlowImage.setAppearance("GreenGlow");
		this.mGlowImage.setPosition(2, 2);
		this.mGlowImage.setVisible(false);
		otherPlayerContainer.add(this.mGlowImage);
		this.mOtherPlayerLabel = this.GUI.Label("Waiting for player...");
		this.mOtherPlayerLabel.setWidth(this.INNER_PANEL_WIDTH);
		this.mOtherPlayerLabel.setTextAlignment(0.5, 0.5);
		this.mOtherPlayerLabel.setPosition(0, 8.5);
		otherPlayerContainer.add(this.mOtherPlayerLabel);
		this.mOtherPlayerInv = this.GUI.ActionContainer("trade_other", this.NUM_PANEL_SLOTS, 1, 0, 0, this, false);
		this.mOtherPlayerInv.setValidDropContainer(true);
		this.mOtherPlayerInv.setItemPanelVisible(true);
		this.mOtherPlayerInv.setCallback(this);
		this.mOtherPlayerInv.setPosition(8.5, 30);
		this.mOtherPlayerInv.setSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight * this.NUM_PANEL_SLOTS + 5);
		this.mOtherPlayerInv.setPreferredSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight * this.NUM_PANEL_SLOTS + 5);
		otherPlayerContainer.add(this.mOtherPlayerInv);
		local currencyComp = this.GUI.Component(this.GUI.BoxLayoutV());
		currencyComp.setSize(this.INNER_PANEL_WIDTH, 20);
		currencyComp.setPreferredSize(this.INNER_PANEL_WIDTH, 20);
		currencyComp.setPosition(8.5, 338);
		otherPlayerContainer.add(currencyComp);
		this.mOtherPlayerCurrency = this.GUI.Currency();
		currencyComp.add(this.mOtherPlayerCurrency);
		local statusComp = this.GUI.Component(this.GUI.BoxLayoutV());
		statusComp.setSize(this.INNER_PANEL_WIDTH, 20);
		statusComp.setPreferredSize(this.INNER_PANEL_WIDTH, 20);
		statusComp.setPosition(0, 375);
		this.mStatusLabel = this.GUI.Label("");
		this.mStatusLabel.setWidth(this.INNER_PANEL_WIDTH);
		this.mStatusLabel.setTextAlignment(0.5, 0.5);
		this.mStatusLabel.setPosition(0, 375);
		outerContainer.add(this.mStatusLabel);
		local myOuterContainer = this.GUI.Container(null);
		myOuterContainer.setSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT + 43);
		myOuterContainer.setPreferredSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT + 43);
		baseContainer.add(myOuterContainer);
		local myContainer = this.GUI.InnerPanel(null);
		myContainer.setSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT);
		myContainer.setPreferredSize(this.INNER_PANEL_WIDTH, this.INNER_PANEL_HEIGHT);
		myOuterContainer.add(myContainer);
		this.mPlayerGlowImage = this.GUI.Container(null);
		this.mPlayerGlowImage.setSize(this.INNER_PANEL_WIDTH - 4, this.INNER_PANEL_HEIGHT - 4);
		this.mPlayerGlowImage.setPreferredSize(this.INNER_PANEL_WIDTH - 4, this.INNER_PANEL_HEIGHT - 4);
		this.mPlayerGlowImage.setAppearance("GreenGlow");
		this.mPlayerGlowImage.setPosition(2, 2);
		this.mPlayerGlowImage.setVisible(false);
		myContainer.add(this.mPlayerGlowImage);
		this.mMyLabel = this.GUI.Label("Waiting for player...");
		this.mMyLabel.setWidth(this.INNER_PANEL_WIDTH);
		this.mMyLabel.setTextAlignment(0.5, 0.5);
		this.mMyLabel.setPosition(0, 8.5);
		myContainer.add(this.mMyLabel);
		this.mMyInv = this.GUI.ActionContainer("trade_avatar", this.NUM_PANEL_SLOTS, 1, 0, 0, this, false);
		this.mMyInv.setValidDropContainer(true);
		this.mMyInv.setItemPanelVisible(true);
		this.mMyInv.setAllowButtonDisownership(true);
		this.mMyInv.setPosition(8.5, 30);
		this.mMyInv.setSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight * this.NUM_PANEL_SLOTS + 5);
		this.mMyInv.setPreferredSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight * this.NUM_PANEL_SLOTS + 5);
		this.mMyInv.setCallback(this);
		this.mMyInv.addListener(this);
		myContainer.add(this.mMyInv);
		local myCurrencyComp = this.GUI.Component(this.GUI.BoxLayoutV());
		myCurrencyComp.setSize(this.INNER_PANEL_WIDTH, 20);
		myCurrencyComp.setPreferredSize(this.INNER_PANEL_WIDTH, 20);
		myCurrencyComp.setPosition(8.5, 338);
		myContainer.add(myCurrencyComp);
		this.mMyCurrency = this.GUI.Currency();
		this.mMyCurrency.setAllowCurrencyEdit(true);
		this.mMyCurrency.addListener(this);
		local goldInput = this.mMyCurrency.getGoldInput();

		if (goldInput)
		{
			goldInput.addActionListener(this);
			goldInput.setMousePressMessage("onInputPressed");
		}

		local silverInput = this.mMyCurrency.getSilverInput();

		if (silverInput)
		{
			silverInput.addActionListener(this);
			silverInput.setMousePressMessage("onInputPressed");
		}

		local copperInput = this.mMyCurrency.getCopperInput();

		if (copperInput)
		{
			copperInput.addActionListener(this);
			copperInput.setMousePressMessage("onInputPressed");
		}

		myCurrencyComp.add(this.mMyCurrency);
		local buttonGrid = this.GUI.Container(this.GUI.BoxLayoutV());
		buttonGrid.setSize(this.INNER_PANEL_WIDTH, 20);
		buttonGrid.setPreferredSize(this.INNER_PANEL_WIDTH, 20);
		buttonGrid.setPosition(0, 375);
		myOuterContainer.add(buttonGrid);
		this.mAcceptButton = this.GUI.Button("");
		this.mAcceptButton.setReleaseMessage("onAcceptTrade");
		this.mAcceptButton.addActionListener(this);
		buttonGrid.add(this.mAcceptButton);
		this.setContentPane(baseContainer);
		this.setContainerMoveProperties();
		this.setCached(::Pref.get("video.UICache"));
	}

	function addItemToOtherInventory( item )
	{
		this.mOtherPlayerInv.addActionFromButton(item, true);
	}

	function cleanup()
	{
		this.mMyCurrency.setCurrentValue(0);
		this.mMyCurrency.resetCurrencyInput();
		this.mOtherPlayerCurrency.setCurrentValue(0);
	}

	function onAcceptTrade( evt )
	{
		::_TradeManager.handleAccept();
	}

	function onCancelTrade( evt )
	{
		::_TradeManager.cancelTrade();
	}

	function onActionButtonAdded( container, actionbuttonslot, actionbutton, action )
	{
		return true;
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		local itemData = action.mItemData;

		if (itemData.mBound)
		{
			this.IGIS.error("This item is bound to you");
			return false;
		}
		else if (itemData.getType() == this.ItemType.QUEST)
		{
			this.IGIS.error("You cannot trade quest items");
			return false;
		}

		return true;
	}

	function onClosePressed()
	{
		::Screens.close("TradeScreen");
		::_TradeManager.cancelTrade();
	}

	function onCurrencyUpdated( amt )
	{
		::_TradeManager.changeCurrency(this.CurrencyCategory.COPPER, amt);
		return;
	}

	function onCreditsUpdated( amt )
	{
		::_TradeManager.changeCurrency(this.CurrencyCategory.CREDITS, amt);
		return;
	}

	function onActionButtonGained( newSlot, oldSlot )
	{
		::_TradeManager.updateTrade();
	}

	function onActionButtonLost( newSlot, oldSlot )
	{
		::_TradeManager.updateTrade();
	}

	function getMyTradingItems()
	{
		return this.mMyInv.getAllActionButtons(true);
	}

	function getMyTradingContainer()
	{
		return this.mMyInv;
	}

	function getCurrencyInputAmount()
	{
		return this.mMyCurrency.getIputAmount();
	}

	function removeAllItemsFromOtherInventory()
	{
		this.mOtherPlayerInv.removeAllActions();
	}

	function setAcceptButtonStatus( status )
	{
		this.mAcceptButton.setEnabled(status);
	}

	function setAcceptButtonText( text )
	{
		this.mAcceptButton.setText(text);
	}

	function setOtherPlayerGlowVisible( visible )
	{
		this.mGlowImage.setVisible(visible);
	}

	function setMyGlowVisible( visible )
	{
		this.mPlayerGlowImage.setVisible(visible);
	}

	function setContainerMoveProperties()
	{
		this.mMyInv.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.DESTROY, this));
		this.mMyInv.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this, this));
	}

	function setOtherCurrencyAmount( type, amt )
	{
		if (this.CurrencyCategory.COPPER == type)
		{
			this.mOtherPlayerCurrency.setCurrentValue(amt);
		}
	}

	function setMyName( name )
	{
		this.mMyLabel.setText(name);
	}

	function setTradeStatusText( text )
	{
		this.mStatusLabel.setText(text);
	}

	function setOtherName( name )
	{
		this.mOtherPlayerLabel.setText(name);
	}

	function offerCurrency()
	{
		this.mMyCurrency.onAccepted(null);
	}

	function onInputPressed( inputArea )
	{
		if (::_TradeManager.getPlayerTradeState() != this.TradeStatus.PREPARING)
		{
			::_TradeManager.updateMyTradeState(this.TradeStatus.PREPARING);
			this.onCurrencyUpdated(this.getCurrencyInputAmount());
			::_TradeManager.updateTrade();
		}
	}

	function setVisible( visible )
	{
		this.GUI.Frame.setVisible(visible);

		if (!this.mOtherPlayerInv)
		{
			return;
		}

		if (visible)
		{
		}
		else
		{
			this.mOtherPlayerInv.removeAllActions();
			this.mMyInv.removeAllActions();
			local inv = ::Screens.get("Inventory", false);

			if (inv)
			{
				inv.unlockAllActions();
			}
		}
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
		::_root.addListener(this);
	}

	function _removeNotify()
	{
		this.cleanup();
		this.GUI.Frame._removeNotify();
		::_root.removeListener(this);
	}

}

class this.TradeManager 
{
	mCurrentlyTradingID = -1;
	mTradeScreen = null;
	mMyTradeState = this.TradeStatus.PREPARING;
	mOtherTradeState = this.TradeStatus.PREPARING;
	mMyCopperOffered = 0;
	mOtherCopperOffered = 0;
	mMakeOffer = false;
	constructor()
	{
		this.mCurrentlyTradingID = -1;
		this.mMyTradeState = this.TradeStatus.PREPARING;
		this.mOtherTradeState = this.TradeStatus.PREPARING;
		::_Connection.addListener(this);
	}

	function acceptOffer()
	{
		::_Connection.sendQuery("trade.accept", this);
	}

	function addItems( protoList )
	{
		this.mTradeScreen.removeAllItemsFromOtherInventory();

		foreach( proto in protoList )
		{
			local newActionButton = this.GUI.ActionButton();
			newActionButton.fillOutFromProto(proto);
			this.mTradeScreen.addItemToOtherInventory(newActionButton);
			this.updateMyTradeState(this.TradeStatus.PREPARING);
			this.updateOtherTradeState(this.TradeStatus.PREPARING);
		}
	}

	function askToTrade( playerID )
	{
		local player = this._sceneObjectManager.getCreatureByID(playerID);
		local playerName = player.getStat(this.Stat.DISPLAY_NAME);
		local TradeMessage = playerName + " wishes to trade.";
		this.mCurrentlyTradingID = playerID;
		this.GUI.MessageBox.showEx(TradeMessage, [
			"Accept",
			"Decline"
		], this, "onTradeDecision", playerID);
	}

	function cancelTrade()
	{
		this.mCurrentlyTradingID = -1;
		::_Connection.sendQuery("trade.cancel", this);
		this.cleanup();
	}

	function getPlayerTradeState()
	{
		return this.mMyTradeState;
	}

	function cleanup()
	{
		this.mMyTradeState = this.TradeStatus.PREPARING;
		this.mOtherTradeState = this.TradeStatus.PREPARING;
		this.mMyCopperOffered = 0;
		this.mOtherCopperOffered = 0;
		this.mMakeOffer = false;

		if (this.mTradeScreen)
		{
			this.mTradeScreen.cleanup();
			this.mTradeScreen.setVisible(false);
		}

		local inventory = ::Screens.get("Inventory", true);

		if (inventory)
		{
			inventory.refreshInventory();
		}
	}

	function changeCurrency( type, amt )
	{
		this.updateMyTradeState(this.TradeStatus.PREPARING);

		if (this.CurrencyCategory.CREDITS == type)
		{
			::_Connection.sendQuery("trade.currency", this, [
				"CREDITS",
				amt
			]);
		}
		else if (this.CurrencyCategory.COPPER == type)
		{
			this.mMyCopperOffered = amt;
			::_Connection.sendQuery("trade.currency", this, [
				"COPPER",
				amt
			]);
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "trade.currency")
		{
			if (this.mMakeOffer)
			{
				this.makeOffer();
			}
		}
		else
		{
			  // [008]  OP_JMP            0      0    0    0
		}
	}

	function getCurrentTradingState()
	{
		return this.mCurrentTradeState;
	}

	function getTradingPlayerID()
	{
		return this.mCurrentlyTradingID;
	}

	function handleAccept()
	{
		if (this.mMyTradeState == this.TradeStatus.WAITING_ACCEPTANCE && this.mOtherTradeState == this.TradeStatus.WAITING_ACCEPTANCE)
		{
			this.acceptOffer();
			this.updateMyTradeState(this.TradeStatus.ACCEPTED);
		}
		else if (this.mMyCopperOffered != this.mTradeScreen.getCurrencyInputAmount())
		{
			this.mMakeOffer = true;
			this.mTradeScreen.offerCurrency();
		}
		else
		{
			this.makeOffer();
		}
	}

	function makeOffer()
	{
		if (this.mOtherTradeState == this.TradeStatus.WAITING_ACCEPTANCE)
		{
			this.updateMyTradeState(this.TradeStatus.WAITING_ACCEPTANCE);
		}
		else
		{
			this.updateMyTradeState(this.TradeStatus.OFFER_MADE);
		}

		this.mMakeOffer = false;
		this._Connection.sendQuery("trade.offer", this);
	}

	function onCurrencyChanged( currencyType, currencyAmt )
	{
		if (this.mOtherCopperOffered != currencyAmt)
		{
			this.updateOtherTradeState(this.TradeStatus.PREPARING);
			this.updateMyTradeState(this.TradeStatus.PREPARING);

			if (this.CurrencyCategory.COPPER == currencyType)
			{
				this.mOtherCopperOffered = currencyAmt;
			}

			this.mTradeScreen.setOtherCurrencyAmount(currencyType, currencyAmt);
		}
	}

	function onOfferMade()
	{
		this.updateOtherTradeState(this.TradeStatus.WAITING_ACCEPTANCE);

		if (this.mMyTradeState == this.TradeStatus.OFFER_MADE)
		{
			this.updateMyTradeState(this.TradeStatus.WAITING_ACCEPTANCE);
		}
	}

	function offerCancel()
	{
		this.updateMyTradeState(this.TradeStatus.PREPARING);
		this.updateOtherTradeState(this.TradeStatus.PREPARING);
	}

	function onTradeCancelled( reason )
	{
		switch(reason)
		{
		case this.CloseReasons.COMPLETE:
			break;

		case this.CloseReasons.TIMEOUT:
			this.IGIS.error("Trade has timed out.");
			break;

		case this.CloseReasons.DISTANCE:
			this.IGIS.error("Trade has been canceled.  You are too far away to continue trading.");
			break;

		case this.CloseReasons.CANCELED:
			this.IGIS.error("Trade has been canceled");
			break;

		case this.CloseReasons.INSUFFICIENT_FUNDS:
			this.IGIS.error("Insufficient funds to complete trade.");
			break;

		case this.CloseReasons.INSUFFICIENT_SPACE:
			this.IGIS.error("Insufficient space to complete trade.");
			break;

		case this.CloseReasons.PLAYER_DEAD:
			this.IGIS.error("You can not trade while one of the players are dead.");
			break;
		}

		this.cleanup();
	}

	function onTradeDecision( source, action )
	{
		if (action == "Accept")
		{
			this.onTradeAccepted();
			::_Connection.sendQuery("trade.start", this);
		}
		else if (action == "Decline")
		{
			this.cancelTrade();
		}
	}

	function onTradeAccepted()
	{
		this.mTradeScreen = this.Screens.get("TradeScreen", true);
		this.Screens.show("TradeScreen", true);
		this.mTradeScreen.setMyName(::_avatar.getStat(this.Stat.DISPLAY_NAME));
		local player = this._sceneObjectManager.getCreatureByID(this.mCurrentlyTradingID);
		local playerName = player.getStat(this.Stat.DISPLAY_NAME);
		this.mTradeScreen.setOtherName(playerName);
		this.updateMyTradeState(this.TradeStatus.PREPARING);
		this.updateOtherTradeState(this.TradeStatus.PREPARING);
	}

	function requestTrade( targetID )
	{
		this.mCurrentlyTradingID = targetID;
		::_Connection.sendQuery("trade.start", this, [
			targetID
		]);
	}

	function updateTrade()
	{
		local items = this.mTradeScreen.getMyTradingItems();
		local queryArgument = [];

		foreach( item in items )
		{
			queryArgument.append(item.mAction.mItemId);
		}

		this.updateMyTradeState(this.TradeStatus.PREPARING);
		this.updateOtherTradeState(this.TradeStatus.PREPARING);
		this._Connection.sendQuery("trade.items", this, queryArgument);
	}

	function updateMyTradeState( state )
	{
		switch(state)
		{
		case this.TradeStatus.PREPARING:
			this.mMyTradeState = this.TradeStatus.PREPARING;
			this.mTradeScreen.setAcceptButtonStatus(true);
			this.mTradeScreen.setOtherPlayerGlowVisible(false);
			this.mTradeScreen.setMyGlowVisible(false);
			this.mTradeScreen.setAcceptButtonText("Offer");
			break;

		case this.TradeStatus.OFFER_MADE:
			this.mMyTradeState = this.TradeStatus.OFFER_MADE;
			this.mTradeScreen.setAcceptButtonStatus(false);
			this.mTradeScreen.setAcceptButtonText("Offer");
			this.mTradeScreen.setMyGlowVisible(true);
			break;

		case this.TradeStatus.WAITING_ACCEPTANCE:
			this.mMyTradeState = this.TradeStatus.WAITING_ACCEPTANCE;
			this.mTradeScreen.setAcceptButtonStatus(true);
			this.mTradeScreen.setAcceptButtonText("Accept");
			this.mTradeScreen.setMyGlowVisible(true);
			break;

		case this.TradeStatus.ACCEPTED:
			this.mMyTradeState = this.TradeStatus.ACCEPTED;
			this.mTradeScreen.setAcceptButtonStatus(false);
			this.mTradeScreen.setAcceptButtonText("Accepted");
			break;
		}

		if (this.mOtherTradeState == this.TradeStatus.PREPARING)
		{
			this.mTradeScreen.setTradeStatusText("Preparing...");
			this.mTradeScreen.setOtherPlayerGlowVisible(false);
		}
		else
		{
			this.mTradeScreen.setTradeStatusText("Ready!");
			this.mTradeScreen.setOtherPlayerGlowVisible(true);
		}
	}

	function updateOtherTradeState( state )
	{
		switch(state)
		{
		case this.TradeStatus.PREPARING:
			this.mOtherTradeState = this.TradeStatus.PREPARING;
			this.mTradeScreen.setTradeStatusText("Preparing...");
			this.mTradeScreen.setOtherPlayerGlowVisible(false);
			this.mTradeScreen.setMyGlowVisible(false);
			break;

		case this.TradeStatus.OFFER_MADE:
			this.mOtherTradeState = this.TradeStatus.OFFER_MADE;
			this.mTradeScreen.setOtherPlayerGlowVisible(true);
			this.mTradeScreen.setTradeStatusText("Ready!");
			break;

		case this.TradeStatus.WAITING_ACCEPTANCE:
			this.mOtherTradeState = this.TradeStatus.WAITING_ACCEPTANCE;
			this.mTradeScreen.setOtherPlayerGlowVisible(true);
			this.mTradeScreen.setTradeStatusText("Ready!");
			break;

		case this.TradeStatus.ACCEPTED:
			break;
		}
	}

}

