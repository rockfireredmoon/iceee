require("UI/ActionContainer");
require("UI/Equipment");
require("UI/Screens");
require("Items/AuctionItemProtoAction");


class AuctionItem {

	auctioneerId = null;
	auctionId = null;
	proto = null;
	sellerName = null;
	remainingSeconds = null;
	copper = null;
	credits = null;
	bids = null;
	bidCopper = null;
	bidCredits = null;
		
	constructor() {
	}

	function fromUpdate(data) {
		auctioneerId = data.getStringUTF().tointeger();
		auctionId = data.getStringUTF().tointeger();
		proto = data.getStringUTF();
		sellerName = data.getStringUTF();
		remainingSeconds = data.getStringUTF().tointeger();
		copper = data.getStringUTF().tointeger();
		credits = data.getStringUTF().tointeger();
		bids = data.getStringUTF().tointeger();
		bidCopper = data.getStringUTF().tointeger();
		bidCredits = data.getStringUTF().tointeger();
		return this;
	}
}

class Screens.AuctionHouse extends GUI.BigFrame
{
	static MAX_DELIVERY_SLOTS = 3;
	
	static mClassName = "Screens.AuctionHouse";
	
	mScreenContainer = null;
	mAuctionHouseContainer = null;
	mColumns = 8;
	mRows = 15;
	mMaxVaultSize = 120;
	mCurrentAuctioneerId = -1;
	mDisabledSlotMaterial0 = "BG/ActionHolderPlug_64";
	mEnabledSlotMaterial0 = "BG/ActionHolder1_64";
	mEnabledSlotMaterial1 = "BG/ActionHolder2_64";
	mEnabledSlotMaterial2 = "BG/ActionHolder3_64";
	mNewsAndYourItemsHolder = null;
	mYourItemsBox = null;
	mSellAC = null;
	mBuyAC = null;
	mNewsBox = null;
	mCheckDistanceEvent = null;
	mUpdateTimesEvent = null;
	mAuctionButton = null;
	mHoursEntry = null;
	mDaysEntry = null;
	mReserveCopperEntry = null;
	mReserveCreditsEntry = null;
	mBuyItNowCopperEntry = null;
	mBuyItNowCreditsEntry = null;
	mBidCopperEntry = null;
	mBidCreditsEntry = null;
	mCommissionCopperEntry = null;
	mCommissionCreditsEntry = null;
	mTypeEntry = null;
	mQualityEntry = null;
	mBidButton = null;
	mBuyItNowButton = null;
	mBuyBuyItNowCopperEntry = null;
	mBuyBuyItNowCreditsEntry = null;
	mTabs = null;
	mSellFields = null;
	mBuyFields = null;
	mySellCentreBox = null;
	mSellFieldsLabel = null;
	mCommission = 0;
	mAuctioneerName = "<Unknown>";
	
	constructor()
	{
		GUI.BigFrame.constructor("Auction House", true, {
			x = 603,
			y = 7
		});
		
		//mScreenContainer = GUI.Container(GUI.BoxLayout());
		//mScreenContainer.getLayoutManager().setExpand(true);
		
		
		mScreenContainer = GUI.Container(GUI.GridLayout(1, 2));
		mScreenContainer.getLayoutManager().setColumns(254, "*");
		mScreenContainer.getLayoutManager().setRows("*");
		
		
		setPosition(400, 50);
		::_ItemDataManager.addListener(this);
		setContentPane(mScreenContainer);
		mScreenContainer.setInsets(5);
		::_Connection.addListener(this);
		mTitleBar.setAppearance("AuctionHouseTop");
		mContentPane.setAppearance("AuctionHouseSides");
		setCached(::Pref.get("video.UICache"));
		
		_buildYourItems();
		_buildAuctionHouse();
		
		mBuyFields.setVisible(false);
	}

	function close()
	{
		setVisible(false);
	}

	function disableSlot( slot )
	{
		if (mAuctionHouseContainer)
		{
			mAuctionHouseContainer.setSlotsBackgroundMaterial(slot, mDisabledSlotMaterial0, true);
			mAuctionHouseContainer.setValidDropSlot(slot, false);
		}
	}

	function enableSlot( slot )
	{
		if (mAuctionHouseContainer)
		{
			local textureNum = slot % 3;
			local texture;

			if (textureNum == 0)
			{
				texture = mEnabledSlotMaterial0;
			}
			else if (textureNum == 1)
			{
				texture = mEnabledSlotMaterial1;
			}
			else
			{
				texture = mEnabledSlotMaterial2;
			}

			mAuctionHouseContainer.setSlotsBackgroundMaterial(slot, texture, true);
			mAuctionHouseContainer.setValidDropSlot(slot, true);
		}
	}

	function getMovable()
	{
		return mMovable;
	}

	function getActionContainer()
	{
		return mAuctionHouseContainer;
	}

	function getYourItemContainer() {
		return mSellAC;
	}
	
	function getBuyContainer() {
		return mBuyAC;
	}

	function onActionButtonAdded( container, actionbuttonslot, actionbutton, action )
	{
		local disabled = false;

		if (container == mAuctionHouseContainer)
		{
			local itemdef = action._getItemDef();
			local profession = Professions[::_avatar.getStat(Stat.PROFESSION)].name;
			local avatarlevel = ::_avatar.getStat(Stat.LEVEL);

			if (itemdef && itemdef.getType() == ItemType.BASIC)
			{
				disabled = false;
			}
			else
			{
				local classrestrictions = itemdef.getClassRestrictions();

				if (avatarlevel < itemdef.getMinUseLevel())
				{
					disabled = true;
				}

				if (!classrestrictions[profession.tolower()])
				{
					disabled = true;
				}
			}
		}

		actionbuttonslot.setSlotDisabled(disabled);
		return true;
	}

	function onAttemptedItemDisown( actionContainer, action, slotIndex ) {
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		print("ICE! container updated " + containerName + "\n");
		if (!container.hasAllItems())
			return;
		

		if (creatureId == ::_avatar.getID()) {
			
			if(containerName == "auction") {
				if (mSellAC)
				{
					print("ICE! container updated - clearing auction box\n");
					mSellAC.removeAllActions();
					
					local hasItem = false;
					
					foreach( itemId in container.mContents ) {
						hasItem = true;
						local item = ::_ItemManager.getItem(itemId);
						mSellAC.addAction(item, false, item.mItemData.mContainerSlot);
											
						mReserveCopperEntry.setCurrentValue(item.mItemDefData.getSellValue().tofloat());
						mBuyItNowCopperEntry.setCurrentValue((item.mItemDefData.getSellValue() * 2).tofloat());	
						
						if (item.mItemDefData.mValueType == CurrencyCategory.CREDITS) {
							mReserveCreditsEntry.setCurrentValue(item.mItemDefData.getBuyValue().tofloat());
							mBuyItNowCreditsEntry.setCurrentValue((item.mItemDefData.getBuyValue() * 2).tofloat());
						}
						
						mReserveCopperEntry.mGoldInputBox.setSize(40, 16);
						mBuyItNowCopperEntry.mGoldInputBox.setSize(40, 16);
						
						mHoursEntry.setText("0");	
						mDaysEntry.setText("1");
						
						break;
					}
					
					if(!hasItem) {
						mReserveCopperEntry.setCurrentValue(0);				
						mBuyItNowCopperEntry.setCurrentValue(0);
						mReserveCreditsEntry.setCurrentValue(0);
						mBuyItNowCreditsEntry.setCurrentValue(0);
						if(mSellFields) {
							mySellCentreBox.remove(mSellFields);
							mySellCentreBox.add(mSellFieldsLabel);
							mSellFields = null;
						}
					}
					else if(!mSellFields) {
						mSellFields = _createSellFields();
						_recalcCommission();
						mySellCentreBox.remove(mSellFieldsLabel);
						mySellCentreBox.add(mSellFields);
					}
					
					mSellAC.updateContainer();
				}
			}
			
		}
	}

	function onClosePressed()
	{
		close();
	}

	function onRightButtonReleased( actionButton, evt )
	{
		local abSlot = actionButton.getActionButtonSlot();
		
		if (abSlot)
		{
			local actionContainer = abSlot.getActionContainer();
			
			
			if (actionContainer == mSellAC) {
				print("ICE! Right click of SELL\n");
			}

			//if (actionContainer == mSellAC)
			//{
				//local slotIndex = mSellAC.getIndexOfSlot(abSlot);

				//if (slotIndex != null)
				//{
					//slotIndex = slotIndex + (mSellAC.getCurrentPageNumber() - 1) * mDeliveryRows * mDeliveryColumns;
					//::_Connection.sendQuery("vault.lootdeliveryitem", this, [
						//mCurrentAuctioneerId,
						//slotIndex
					//]);
					//return false;
				//}
			//}
			//else if (actionContainer == mAuctionHouseContainer)
			//{
				//if (Key.isDown(Key.VK_CONTROL))
				//{
					//local inventory = Screens.get("Inventory", true);

					//if (inventory)
					//{
						//local freeslots = inventory.getFreeSlotsRemaining();

						//if (freeslots > 0)
						//{
							//local inventoryAC = inventory.getMyActionContainer();
							//inventoryAC.simulateButtonDrop(actionButton);
						//}
					//}
				//}
			//}
		}

		return true;
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (!::_avatar)
		{
			return;
		}

		local proff = ::_avatar.getStat(Stat.PROFESSION);

		if (proff == null)
		{
			return;
		}

		local profession = Professions[::_avatar.getStat(Stat.PROFESSION)].name;
		local avatarlevel = ::_avatar.getStat(Stat.LEVEL);
		local visibleItemCount = mAuctionHouseContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = mAuctionHouseContainer.getSlotContents(i);

			if (actionButtonSlot)
			{
				local actionButton = actionButtonSlot.getActionButton();

				if (actionButton)
				{
					local action = actionButton.getAction();
					local disabled = false;
					local itemdef = action._getItemDef();

					if (itemdef == itemdef)
					{
						if (itemdef && itemdef.getType() == ItemType.BASIC)
						{
							disabled = false;
						}
						else
						{
							local classrestrictions = itemdef.getClassRestrictions();

							if (avatarlevel < itemdef.getMinUseLevel())
							{
								disabled = true;
							}

							if (!classrestrictions[profession.tolower()])
							{
								disabled = true;
							}
						}

						actionButtonSlot.setSlotDisabled(disabled);
					}
				}
			}
		}
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		print("ICE! onItemMovedInContainer( " + container + ", " + slotIndex + ", " + oldSlotsButton + " )\n");
		local item = container.getSlotContents(slotIndex);
		local itemID = item.getActionButton().getAction().mItemId;
		local queryArgument = [];
		local previousSlotContainer = oldSlotsButton.getPreviousActionContainer();
		local oldActionButtonSlot = oldSlotsButton.getActionButtonSlot();
		local oldSlotContainerName = "";
		local oldSlotIndex = 0;

		if (previousSlotContainer && oldActionButtonSlot)
		{
			oldSlotContainerName = previousSlotContainer.getContainerName();
			oldSlotIndex = previousSlotContainer.getIndexOfSlot(oldActionButtonSlot);
		}
		
		if (container == mBuyAC) {
			if (item.getSwapped() == true) {
				item.setSwapped(false);
			}
			else {
				local itemAction  = item.getActionButton().getAction();
				
				mBidCopperEntry.setCurrentValue(itemAction.mAuctionItem.bidCopper.tofloat());
				mBidCreditsEntry.setCurrentValue(itemAction.mAuctionItem.bidCredits.tofloat());
				
				mBuyBuyItNowCopperEntry.setCurrentValue(itemAction.mAuctionItem.copper.tofloat());
				mBuyBuyItNowCreditsEntry.setCurrentValue(itemAction.mAuctionItem.credits.tofloat());
				
				mBuyFields.setVisible(true);
			}
		}
		
		if (container == mSellAC) {
			if (item.getSwapped() == true)
				item.setSwapped(false);
			else {
				queryArgument.append(itemID);
				queryArgument.append("auction");
				queryArgument.append(slotIndex);

				if (::_Connection.getProtocolVersionId() >= 19)
				{
					queryArgument.append(oldSlotContainerName);
					queryArgument.append(oldSlotIndex);
				}

				queryArgument.append(::_avatar.getID());
				queryArgument.append(mCurrentAuctioneerId);
				_Connection.sendQuery("item.move", this, queryArgument);
			}
		}

		if (container == mAuctionHouseContainer)
		{
			//if (item.getSwapped() == true)
			//{
				//item.setSwapped(false);
			//}
			//else
			//{
				//queryArgument.append(itemID);
				//queryArgument.append("auctionhouse");
				//queryArgument.append(slotIndex);

				//if (::_Connection.getProtocolVersionId() >= 19)
				//{
					//queryArgument.append(oldSlotContainerName);
					//queryArgument.append(oldSlotIndex);
				//}

				//queryArgument.append(::_avatar.getID());
				//queryArgument.append(mCurrentAuctioneerId);
				//_Connection.sendQuery("item.move", this, queryArgument);
			//}
		}
	}

	function onLevelUpdate( level )
	{
		if (!::_avatar)
		{
			return;
		}

		local proff = ::_avatar.getStat(Stat.PROFESSION);

		if (proff == null)
		{
			return;
		}

		local profession = Professions[proff].name;
		local avatarlevel = ::_avatar.getStat(Stat.LEVEL);
		local visibleItemCount = mAuctionHouseContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = mAuctionHouseContainer.getSlotContents(i);

			if (actionButtonSlot)
			{
				local actionButton = actionButtonSlot.getActionButton();

				if (actionButton)
				{
					local action = actionButton.getAction();
					local disabled = false;
					local itemdef = action._getItemDef();

					if (itemdef && itemdef.getType() == ItemType.BASIC)
					{
						disabled = false;
					}
					else
					{
						local classrestrictions = itemdef.getClassRestrictions();

						if (avatarlevel < itemdef.getMinUseLevel())
						{
							disabled = true;
						}

						if (!classrestrictions[profession.tolower()])
						{
							disabled = true;
						}
					}
					
					actionButtonSlot.setSlotDisabled(disabled);
				}
			}
		}
	}

	function onQueryComplete( qa, rows ) {
	
		switch(qa.query) {
		case "ah.buy":
			IGIS.info("You won the item!");
			mBuyAC.removeAllActions();
			mBuyFields.setVisible(false);
			break;
		case "ah.bid":
			IGIS.info("Your bid was accepted!");
			break;
		case "ah.auction":
			IGIS.info("Your item is now in the auction!");
			break;
		case "ah.contents":
			print("ICE! Got contents of AH\n");
			
			mAuctionHouseContainer.removeAllActions();
			local resultLen = rows.len();
			print("ICE! Rows " + rows.len() + "\n");
			local i = 0;
			foreach( res in rows ) { 
				if(i == 0) {
					mCommission = res[0].tofloat();
					mAuctioneerName = res[1];
					setTitle(mAuctioneerName + "'s Auction House");
				} else {
					local auctionItem = AuctionItem();
					auctionItem.auctionId = res[0];
					auctionItem.proto = res[1];
					auctionItem.sellerName = res[2];
					auctionItem.remainingSeconds = res[3].tointeger();
					auctionItem.copper = res[4].tointeger();
					auctionItem.credits = res[5].tointeger();
					auctionItem.bids = res[6].tointeger();
					auctionItem.bidCopper = res[7].tointeger();
					auctionItem.bidCredits = res[8].tointeger();
					if (i >= resultLen - 1) {
						mAuctionHouseContainer.addAction(AuctionItemProtoAction(auctionItem), true);
					}
					else {
						mAuctionHouseContainer.addAction(AuctionItemProtoAction(auctionItem), false);
					}
				}
				i++;
			}
			
			
			refreshAuction();
			
			_resizeWindow();
			GUI.Frame.setVisible(true);
			
			break;
			
		}
	}

	function onQueryError( qa, error ) {
		IGIS.error(error);

		switch(qa.query) {
		case "item.move":
			::Util.handleMoveItemBack(qa);
			break;

		case "ah.size":
			break;

		case "vault.lootdeliveryitem":
			refreshAuction();
			break;
		}
	}

	function onValidDropSlot( newSlot, oldSlot ) {
		local oldActionContainer = oldSlot.getActionContainer();

		if (!oldActionContainer)
			return false;

		local newActionContainer = newSlot.getActionContainer();

		if (!newActionContainer) 
			return false;

		if (::_avatar && ::_avatar.isDead()) {
			IGIS.error("You cannot move items when you are dead.");
			return false;
		}
	}
	
	function refreshAuction() {
		local auction = ::_ItemDataManager.getContents("auction");
		onContainerUpdated("auction", ::_avatar.getID(), auction);
	}

	function setAuctioneerId( id ) {
		mCurrentAuctioneerId = id;
		if (mCurrentAuctioneerId != null)
			_refreshAuctionHouse();
	}

	function setVisible( visible ) {
		if (visible) {
			print("ICE! Making AH visible\n");
			::Audio.playSound("Sound-InventoryOpen.ogg");
			
			_resizeWindow();
			GUI.Frame.setVisible(true);
			
			if (!mCheckDistanceEvent)
				mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "auctioneerDistanceCheck");
				
			if (!mUpdateTimesEvent)
				mUpdateTimesEvent = ::_eventScheduler.fireIn(1, this, "updateTimes");
		}
		else {
			mCurrentAuctioneerId = -1;
			GUI.Frame.setVisible(false);
			::Audio.playSound("Sound-InventoryClose.ogg");

			if (mCheckDistanceEvent) {
				::_eventScheduler.cancel(mCheckDistanceEvent);
				mCheckDistanceEvent = null;
			}
			
			if (mUpdateTimesEvent) {
				::_eventScheduler.cancel(mUpdateTimesEvent);
				mUpdateTimesEvent = null;
			}
		}
	}
	
	function onCreditsUpdated( value ) {
		_recalcCommission();
	}
	
	function onCurrencyUpdated( value ) {
		_recalcCommission();
	}
	
	function onTextChanged( text ) {
		_recalcCommission();
	}
	
	function onRemoveAuctionHouseItem(auctioneerId, auctionId) {
		if(auctioneerId == mCurrentAuctioneerId) {
			// TODO this is a bit too brute force. Instead it should only remove the item changed
			_refreshAuctionHouse();
		}
	}
	
	function onUpdateAuctionHouseItem(auctioneerId, auctionId, remainingSeconds, bids, bidCopper, bidCredits) {
		if(auctioneerId == mCurrentAuctioneerId) {
			// TODO this is a bit too brute force. Instead it should only update the item changed (applying filters locally)
			_refreshAuctionHouse();
		}
	}
	
	function onNewAuctionHouseItem(auctionItem) {
		if(auctionItem.auctioneerId == mCurrentAuctioneerId) {
			// TODO this is a bit too brute force. Instead it should only update the item changed (applying filters locally)
			//mAuctionHouseContainer.addAction(AuctionItemProtoAction(auctionItem), true);
			_refreshAuctionHouse();
		}
	}
	
	function onActionButtonLost(newSlot, oldSlot) {
		if(newSlot == null && oldSlot != null) {
			mBuyFields.setVisible(false);
		}
	}

	function updateTimes() {
		if (!isVisible()) {
			return;
		}
		//updateTime
		local buttons = mAuctionHouseContainer.getAllActionButtonSlots();
		foreach(button in buttons) {
			local actionButton = button.getActionButton();
			if(actionButton) {
				local action = actionButton.getAction();
				if(action)
					action.updateTime();
			}
		} 
		mUpdateTimesEvent = ::_eventScheduler.fireIn(1, this, "updateTimes");
	}
	
	function auctioneerDistanceCheck() {
		if (!isVisible()) {
			return;
		}

		local vault = ::_sceneObjectManager.getCreatureByID(mCurrentAuctioneerId);

		if (vault) {
			if (Math.manhattanDistanceXZ(::_avatar.getPosition(), vault.getPosition()) > Util.getRangeOffset(_avatar, vault) + MAX_USE_DISTANCE) {
				IGIS.error("You are too far away from the auction to continue using it.");
				setVisible(false);
				mCurrentAuctioneerId = -1;
			}
			else {
				mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "auctioneerDistanceCheck");
			}
		}
		else {
			IGIS.error("This auctioneer no longer exists.");
			setVisible(false);
			mCurrentAuctioneerId = -1;
		}
	}

	function _addNotify() {
		GUI.ContainerFrame._addNotify();
	}

	function _buildAuctionHouse() {
	
		mTypeEntry = GUI.DropDownList();
		mTypeEntry.addChoice("All");
		foreach(k, v in ItemTypeNameDef) {
			if(k > 0)
				mTypeEntry.addChoice(v.name);
		}
		mTypeEntry.addSelectionChangeListener(this);
		
		mQualityEntry = GUI.DropDownList();
		mQualityEntry.addChoice("All");
		mQualityEntry.addChoice("Poor");
		mQualityEntry.addChoice("Standard");
		mQualityEntry.addChoice("Good");
		mQualityEntry.addChoice("Superior");
		mQualityEntry.addChoice("Epic");
		mQualityEntry.addChoice("Legendary");
		mQualityEntry.addChoice("Artifact");
		mQualityEntry.addSelectionChangeListener(this);
	
	
		mAuctionHouseContainer = GUI.InventoryActionContainer("auctionhouse", 10, 1, 0, 0, this);
		mAuctionHouseContainer.setUseMode(GUI.ActionButtonSlot.USE_LEFT_DOUBLE_CLICK);
		mAuctionHouseContainer.setItemPanelVisible(true);
		mAuctionHouseContainer.setCallback(this);
		mAuctionHouseContainer.setAllButtonsDraggable(true);
		mAuctionHouseContainer.setAllowButtonDisownership(false);
		mAuctionHouseContainer.setFrontInsert(true);
		mAuctionHouseContainer.setTooltipRenderingModifiers("itemProto", {
			showBuyValue = true
		});
		mAuctionHouseContainer.addMovingToProperties("auctionbuy", MoveToProperties(MovementTypes.CLONE));
		mAuctionHouseContainer.addMovingToProperties("itempreview", MoveToProperties(this.MovementTypes.CLONE));
		
		//mAuctionHouseContainer.setSize(preferredSize.width + 50, preferredSize.height + 4);
		//mAuctionHouseContainer.setPreferredSize(500, 100);
		//mAuctionHouseContainer.setSize(375,390);
		//mAuctionHouseContainer.setPreferredSize(375,390);
		
		local filterPanel = GUI.Container(GUI.GridLayout(2, 2));
		filterPanel.setInsets(0,0,4,0);
		filterPanel.getLayoutManager().setColumns(102, "*");
		filterPanel.getLayoutManager().setGaps(2, 2);
		filterPanel.add(GUI.Label("Type"));
		filterPanel.add(mTypeEntry);
		filterPanel.add(GUI.Label("Rarity"));
		filterPanel.add(mQualityEntry);
		
		local centreBox = GUI.Container(GUI.GridLayout(2, 1));
		centreBox.getLayoutManager().setColumns("*");
		centreBox.getLayoutManager().setRows(64, "*");
		centreBox.setInsets(4);
		centreBox.add(filterPanel);
		centreBox.add(mAuctionHouseContainer);
		
		mScreenContainer.add(centreBox);
	}
	
	function _buildYourItems() {
	
		mNewsBox = _buildNewsBox();
		mYourItemsBox = _buildSellBuyBox();
		
		mNewsAndYourItemsHolder = GUI.Container(GUI.GridLayout(2, 1));
		mNewsAndYourItemsHolder.getLayoutManager().setColumns("*");
		mNewsAndYourItemsHolder.getLayoutManager().setRows(255, "*");
		mNewsAndYourItemsHolder.add(mNewsBox);
		mNewsAndYourItemsHolder.add(mYourItemsBox);
		
		mScreenContainer.add(mNewsAndYourItemsHolder);
	}

	function _buildNewsBox() {
		local newsBox = GUI.Panel();
		newsBox.setAppearance("AuctionHouseImage");
		newsBox.setSize(254, 255);
		newsBox.setPreferredSize(254, 255);
		return newsBox;
	}
	
	function _buildSellBuyBox() {
		local sellTab = _buildSellTab();
		local buyTab = _buildBuyTab();
		mTabs = GUI.TabbedPane();
		mTabs.setTabPlacement("top");
		mTabs.addTab("Sell", sellTab);
		mTabs.addTab("Buy", buyTab);
		return mTabs;
	}
	
	function _buildBuyTab() {
		mBuyAC = GUI.ActionContainer("auctionbuy", 1, 1, 0, 0, this);
		mBuyAC.setPagingInfoEnabled(false);
		mBuyAC.setAllButtonsDraggable(true);
		mBuyAC.setAllowButtonDisownership(true);
		mBuyAC.setItemPanelVisible(true);
		mBuyAC.addListener(this);
		mBuyAC.setCallback(this);
		mBuyAC.setTooltipRenderingModifiers("auctionItemProto", {
			hideValue = true
		});
		mBuyAC.addAcceptingFromProperties("auctionhouse", AcceptFromProperties(this));
		
		mBidCopperEntry = GUI.Currency();	
		mBidCopperEntry.setAllowCurrencyEdit(true);
		
		mBidCreditsEntry = GUI.Credits();
		mBidCreditsEntry.setAllowCreditsEdit(true);
		mBidCreditsEntry.setSize(64, 16);
		
		mBuyBuyItNowCopperEntry = GUI.Currency();	
		mBuyBuyItNowCopperEntry.setAllowCurrencyEdit(false);
		
		mBuyBuyItNowCreditsEntry = GUI.Credits();
		mBuyBuyItNowCreditsEntry.setAllowCreditsEdit(false);
		mBuyBuyItNowCreditsEntry.setSize(64, 16);
		
		local bidPanel = GUI.Container();
		bidPanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		bidPanel.add(mBidCopperEntry);
		bidPanel.add(mBidCreditsEntry);
		bidPanel.setInsets(0, 0, 0, 6);
		
		local buyItNowPanel = GUI.Container();
		buyItNowPanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		buyItNowPanel.add(mBuyBuyItNowCopperEntry);
		buyItNowPanel.add(mBuyBuyItNowCreditsEntry);
		buyItNowPanel.setInsets(0, 0, 0, 6);
		
		// Send Button
		mBidButton = GUI.NarrowButton("Bid!");
		mBidButton.setPressMessage("_bidPressed");
		mBidButton.addActionListener(this);
		
		// Send Button
		mBuyItNowButton = GUI.NarrowButton("Buy!");
		mBuyItNowButton.setPressMessage("_buyItNowPressed");
		mBuyItNowButton.addActionListener(this);
		
		// Options
		
		mBuyFields = GUI.Container();
		mBuyFields.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.VERTICAL));
		mBuyFields.getLayoutManager().setAlignment(0.5);
		mBuyFields.getLayoutManager().setGap(8);
		//mBuyFields.getLayoutManager().setExpand(true);
		mBuyFields.add(bidPanel);
		mBuyFields.add(mBidButton);
		mBuyFields.add(GUI.HTML("<font size=\"16\">.. or ..</font>"));
		mBuyFields.add(buyItNowPanel);
		mBuyFields.add(mBuyItNowButton);
		
		// Center
		local centreBox = GUI.Container(GUI.GridLayout(2, 1));
		centreBox.getLayoutManager().setColumns("*");
		centreBox.getLayoutManager().setRows(44, "*");
		centreBox.add(mBuyAC);
		centreBox.add(mBuyFields, {
			anchor = GUI.GridLayout.CENTER
		});
		
		// Container
		local yourItemsContainer = GUI.Container(GUI.BorderLayout());	
		yourItemsContainer.add(centreBox, GUI.BorderLayout.CENTER);
		return yourItemsContainer;
		
	}
	
	function _buildSellTab() {
		mSellAC = GUI.ActionContainer("auction", 1, 1, 0, 0, this);
		mSellAC.setAllButtonsDraggable(true);
		//mSellAC.setAllowButtonDisownership(true);
		mSellAC.setItemPanelVisible(true);
		mSellAC.addListener(this);
		mSellAC.setCallback(this);
		mSellAC.setTooltipRenderingModifiers("itemProto", {
			hideValue = true,
			resizeInfoPanel = false
		});
		mSellAC.setMaximumSize(254, 44);
		
		mSellAC.addMovingToProperties("inventory", MoveToProperties(MovementTypes.MOVE));
		mSellAC.addAcceptingFromProperties("inventory", AcceptFromProperties(this));
		
		// Input fields
		mHoursEntry = GUI.InputArea();
		mHoursEntry.setSize(32, 15);
		mHoursEntry.setAllowOnlyNumbers(true);
		mHoursEntry.setMaxCharacters(2);
		mHoursEntry.setCenterText(true);
		mHoursEntry.addActionListener(this);
		
		mDaysEntry = GUI.InputArea();
		mDaysEntry.setSize(32, 15);
		mDaysEntry.setAllowOnlyNumbers(true);
		mDaysEntry.setMaxCharacters(2);
		mDaysEntry.setCenterText(true);
		mDaysEntry.addActionListener(this);
		
		mCommissionCopperEntry = GUI.Currency();	
		
		mCommissionCreditsEntry = GUI.Credits();
		mCommissionCreditsEntry.setSize(64, 16);

		mReserveCopperEntry = GUI.Currency();	
		mReserveCopperEntry.addListener(this);
		mReserveCopperEntry.setAllowCurrencyEdit(true);
		
		mReserveCreditsEntry = GUI.Credits();	
		mReserveCreditsEntry.addListener(this);
		mReserveCreditsEntry.setAllowCreditsEdit(true);
		mReserveCreditsEntry.setSize(64, 16);
		
		mBuyItNowCopperEntry = GUI.Currency();	
		mBuyItNowCopperEntry.setAllowCurrencyEdit(true);
		mBuyItNowCopperEntry.addListener(this);
		
		mBuyItNowCreditsEntry = GUI.Credits();
		mBuyItNowCreditsEntry.setAllowCreditsEdit(true);
		mBuyItNowCreditsEntry.setSize(64, 16);
		mBuyItNowCreditsEntry.addListener(this);
		
		// Send Button
		mAuctionButton = GUI.NarrowButton("Auction!");
		mAuctionButton.setPressMessage("_auctionPressed");
		mAuctionButton.addActionListener(this);
		
		mSellFieldsLabel = GUI.HTML("<font size=\"16\">Drag an item here to auction it.</font>");
		
		mySellCentreBox= GUI.Container(GUI.GridLayout(2, 1));
		mySellCentreBox.getLayoutManager().setColumns("*");
		mySellCentreBox.getLayoutManager().setRows(44, "*");
		mySellCentreBox.add(mSellAC);
		mySellCentreBox.add(mSellFieldsLabel);
		
		return mySellCentreBox;
	}
	
	function _createSellFields() {
	
		//
		local timePanel = GUI.Container();
		timePanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		timePanel.getLayoutManager().setAlignment(0.5);
		timePanel.getLayoutManager().setGap(8);
		timePanel.add(mDaysEntry);
		timePanel.add(GUI.Label("Days"));
		timePanel.add(mHoursEntry);
		timePanel.add(GUI.Label("Hours"));
		//timePanel.setInsets(0, 0, 0, 48);
		
		local reservePanel = GUI.Container();
		reservePanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		reservePanel.add(mReserveCopperEntry);
		reservePanel.add(mReserveCreditsEntry);
		//reservePanel.setInsets(0, 0, 0, 6);
		//
		
		local buyItNowPanel = GUI.Container();
		buyItNowPanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		buyItNowPanel.add(mBuyItNowCopperEntry);
		buyItNowPanel.add(mBuyItNowCreditsEntry);
		//buyItNowPanel.setInsets(0, 0, 0, 6);
		
		local commissionPanel = GUI.Container();
		commissionPanel.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.HORIZONTAL));
		commissionPanel.add(GUI.Label("Commission:"));
		commissionPanel.add(mCommissionCopperEntry);
		commissionPanel.add(mCommissionCreditsEntry);
		//commissionPanel.setInsets(0, 0, 0, 26);
		
		// Options
		
		local fields = GUI.Container();
		fields.setLayoutManager(GUI.BoxLayout(GUI.BoxLayout.VERTICAL));
		fields.getLayoutManager().setAlignment(0.5);
		fields.getLayoutManager().setGap(6);
		fields.add(timePanel);
		fields.add(GUI.Label("Reserve"));
		fields.add(reservePanel);
		fields.add(GUI.Label("Buy It Now"));
		fields.add(buyItNowPanel);
		fields.add(commissionPanel);
		fields.add(mAuctionButton);
		
		return fields;
	}
	
	function _recalcCommission() {
	
		local days = Util.trim(mDaysEntry.getText()).tointeger();
		local hours = Util.trim(mHoursEntry.getText()).tointeger();
		local totalhours = ( days * 24 ) + hours;
	
		local reserveCopper = mReserveCopperEntry.getIputAmount();
		local reserveCredits = mReserveCreditsEntry.getCurrentValue();
		local buyItNowCopper = mBuyItNowCopperEntry.getIputAmount();
		local buyItNowCredits = mBuyItNowCreditsEntry.getCurrentValue();
	
		local copper = reserveCopper;
		if(buyItNowCopper > copper) 
			copper = buyItNowCopper;
		
		local credits = reserveCredits;
		if(buyItNowCredits > credits) 
			credits = buyItNowCredits;
		
		mCommissionCopperEntry.setCurrentValue((copper * ( mCommission / 100.0 ) * totalhours).tointeger());		
		mCommissionCreditsEntry.setCurrentValue((credits * ( mCommission / 100.0 ) * totalhours).tointeger());
	}
	
	function _resizeWindow() {		
		setSize(730, 580);
		setPreferredSize(730, 580);
	}
	
	function _removeNotify() {
		close();
		GUI.ContainerFrame._removeNotify();
	}

	function _auctionPressed( button )	{
		sendAuction();
	}
	
	function _buyItNowPressed(button) {
		local item = mBuyAC.getSlotContents(0);
		local itemAction = item.getActionButton().getAction();
		::_Connection.sendQuery("ah.buy", this, [
			mCurrentAuctioneerId,
			itemAction.mAuctionItem.auctionId.tostring()
		]);
	}

	function _bidPressed( button )	{
		local copper = mBidCopperEntry.getIputAmount();
		local credits = mBidCreditsEntry.getCurrentValue();
		local item = mBuyAC.getSlotContents(0);
		local itemAction = item.getActionButton().getAction();
		
		if(copper <= itemAction.mAuctionItem.bidCopper && credits <= itemAction.mAuctionItem.bidCredits ) {
			IGIS.error("Bid rejected. Your bid must be higher than the current bid");
		}
		else {
			::_Connection.sendQuery("ah.bid", this, [
				mCurrentAuctioneerId,
				copper.tostring(),
				credits.tostring(),
				itemAction.mAuctionItem.auctionId.tostring()
			]);
		}
	}
	
	function _refreshAuctionHouse() {
		mAuctionHouseContainer.removeAllActions();
		::_Connection.sendQuery("ah.contents", this, [
			mCurrentAuctioneerId,
			mTypeEntry.getCurrentIndex() == 0 ? -1 : mTypeEntry.getCurrentIndex(),
			mQualityEntry.getCurrentIndex() - 1
		]);
	}

	function onSelectionChange( list ) {
		_refreshAuctionHouse();
	}
	
	function sendAuction() {		
		
		local days = Util.trim(mDaysEntry.getText());
		local hours = Util.trim(mHoursEntry.getText());
		local reserveCopper = mReserveCopperEntry.getIputAmount().tostring();
		local reserveCredits = mReserveCreditsEntry.getCurrentValue().tostring();
		local buyItNowCopper = mBuyItNowCopperEntry.getIputAmount().tostring();
		local buyItNowCredits = mBuyItNowCreditsEntry.getCurrentValue().tostring();
		
		::_Connection.sendQuery("ah.auction", this, [
			mCurrentAuctioneerId,
			days,
			hours,
			reserveCopper,
			reserveCredits,
			buyItNowCopper,
			buyItNowCredits,
		]);
	}

}

