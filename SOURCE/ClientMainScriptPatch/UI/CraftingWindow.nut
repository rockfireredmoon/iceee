this.require("UI/Screens");
class this.Screens.CraftingWindow extends this.GUI.Frame
{
	static mClassName = "CraftingWindow";
	static BASE_WIDTH = 425;
	static BASE_HEIGHT = 322;
	static MAX_COMPONENTS = 6;
	static INSET_AMOUNT = 10;
	mListContainer = null;
	mCreatedItemContainer = null;
	mCreatedItemLabel = null;
	mCraftingItemsComps = null;
	mCraftPart = null;
	mKeyItemComp = null;
	mCancelButton = null;
	mAdvancedCraftButton = null;
	mCreateButton = null;
	mBackgroudComp = null;
	mCreatedItemId = 0;
	mListItemDefId = 0;
	mCrafterId = -1;
	mCheckDistanceEvent = null;
	mInfoPanelSlotHeight = 43;
	mInfoPanelSlotWidth = 217;
	mTabPane = null;
	mAdvancedContainer = null;
	
	constructor()
	{
		this.mCraftingItemsComps = [];
		this.GUI.Frame.constructor("Crafting");
		this.addActionListener(this);
		this.setSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.setPreferredSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		
		local centerComp = this.GUI.Component(this.GUI.BoxLayoutV());
		centerComp.getLayoutManager().setAlignment(0.0);
		local topComp = this._topComponent();
		centerComp.add(topComp);
		local bottomComp = this._bottomComponent();
		centerComp.add(bottomComp);
		
		// This
		mTabPane = this.GUI.TabbedPane();
		mTabPane.setTabPlacement("top");
		mTabPane.setTabFontColor("E4E4E4");
		mTabPane.addTab("Standard", centerComp);
		mTabPane.addTab("Advanced", _advComponent());
		mTabPane.addActionListener(this);
		mTabPane.setInsets(3);
		
		setContentPane(mTabPane);
		
		::_ItemDataManager.addListener(this);
		::_Connection.addListener(this);
	}

	function setCrafterId( id )
	{
		this.mCrafterId = id;
	}

	function getCrafterId()
	{
		return this.mCrafterId;
	}
	
	function onAdvancedHelpPressed(button) {
		_URLManager.LaunchURL("Advanced Crafting");
	}
	
	function onAdvancedCraftPressed(button) {
		local items = mAdvancedContainer.getAllActionButtons(true);
		local queryArgument = [this.mCrafterId];
		foreach(item in items)
		{
			queryArgument.append(item.mAction.mItemId);
		}
		if(queryArgument.len() == 0)
		{
			IGIS.info("You have not provided any items.");
			return;
		}
		button.setEnabled(false);
		::_Connection.sendQuery("mod.craft", this, queryArgument);
	}
	
	function _advComponent() {
		mAdvancedContainer = GUI.ActionContainer("advcraft", 1, 5, 0, 0, this, false);
		mAdvancedContainer.setItemPanelVisible(false);
		mAdvancedContainer.setValidDropContainer(true);
		mAdvancedContainer.setAllowButtonDisownership(false);
		mAdvancedContainer.setSlotDraggable(false, 0);
		mAdvancedContainer.addListener(this);
		mAdvancedContainer.addAcceptingFromProperties("inventory", AcceptFromProperties(this, this));

		mAdvancedCraftButton = GUI.NarrowButton("Craft");
		mAdvancedCraftButton.addActionListener(this);
		mAdvancedCraftButton.setReleaseMessage("onAdvancedCraftPressed");

		local mAdvancedCancelButton = GUI.RedNarrowButton("Cancel");
		mAdvancedCancelButton.addActionListener(this);
		mAdvancedCancelButton.setReleaseMessage("_cancelPress");

		local mButtonHelp = GUI.NarrowButton("Crafting Help Reference");
		mButtonHelp.addActionListener(this);
		mButtonHelp.setReleaseMessage("onAdvancedHelpPressed");
		mButtonHelp.setFixedSize(220, 32);

		local buttonRow = GUI.Container(GUI.BoxLayout());
		buttonRow.add(GUI.Spacer(2, 0));  //Seems to need a left margin?
		buttonRow.add(mAdvancedCraftButton);
		buttonRow.add(mAdvancedCancelButton);
		buttonRow.setPreferredSize(220, 32);

		local mContainer = GUI.Container(GUI.BoxLayoutV());

		local text = "Advanced Crafting does not require Recipe items, you just need to know<br/>" +
					 "the ingredients. Drag ingredients into the slots, then click <font color=\"00FF00\">Craft</font>.<br/>" +
					 "If your recipe is correct, they will be removed, and you will be granted the<br/>" +
					 "resulting item(s).";

		local label = GUI.HTML();
		label.setText(text);
		mContainer.add(label);
		mContainer.add(GUI.Spacer(0, 40));
		mContainer.add(mAdvancedContainer);
		mContainer.add(GUI.Spacer(0, 48));
		mContainer.add(buttonRow);
		mContainer.add(mButtonHelp);
		
		return mContainer;
	}

	function _topComponent()
	{
		local listComp = this.GUI.Container(this.GUI.BoxLayout());
		listComp.getLayoutManager().setGap(10);
		listComp.setInsets(0, 0, 0, 40);
		local numRows = 1;
		local numColumns = 1;
		local spacing = 5;
		local scrollContainer = this.GUI.Container(this.GUI.BoxLayout());
		scrollContainer.setInsets(-8, 0, 0, 6);
		scrollContainer.setSize(55, 57);
		scrollContainer.setPreferredSize(55, 57);
		scrollContainer.setAppearance("Crafting/RecipeScroll");
		listComp.add(scrollContainer);
		this.mListContainer = this.GUI.ActionContainer("crafting_list_container", numRows, numColumns, spacing, spacing, this, false);
		this.mListContainer.setAllowButtonDisownership(true);
		this.mListContainer.setTransparent();
		this.mListContainer.addListener(this);
		scrollContainer.add(this.mListContainer);
		local craftingListSlots = this.mListContainer.getAllActionButtonSlots();

		foreach( actionButtonSlot in craftingListSlots )
		{
			actionButtonSlot.setAppearance(null);
		}

		this.setContainerMoveProperties();
		local resultMarker = this.GUI.Component(null);
		resultMarker.setSize(44, 33);
		resultMarker.setPreferredSize(44, 33);
		resultMarker.setAppearance("Crafting/ResultMarker");
		listComp.add(resultMarker);
		this.mCreatedItemContainer = this.GUI.ActionContainer("crafting_created_item_container", 1, 1, 0, 0, this, false);
		this.mCreatedItemContainer.setItemPanelVisible(true);
		this.mCreatedItemContainer.setAllButtonsDraggable(false);
		this.mCreatedItemContainer.setVisible(false);
		listComp.add(this.mCreatedItemContainer);
		this.mCreatedItemLabel = this.GUI.Label("");
		this.mCreatedItemLabel.setFont(this.GUI.Font("Maiandra", 20));
		return listComp;
	}

	function craftingScreenDistanceCheck()
	{
		local crafter = ::_sceneObjectManager.getCreatureByID(this.mCrafterId);

		if (crafter)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), crafter.getPosition()) > this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the crafter to continue crafting.");
				this.setVisible(false);
				this.mCrafterId = -1;
			}
			else
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "craftingScreenDistanceCheck");
			}
		}
		else
		{
			this.IGIS.error("This crafter no longer exists.");
			this.setVisible(false);
			this.mCrafterId = -1;
		}
	}

	function setVisible( value )
	{
		this.GUI.Component.setVisible(value);

		if (!value)
		{
			if (this.mCheckDistanceEvent)
			{
				::_eventScheduler.cancel(this.mCheckDistanceEvent);
			}
			clearAdvancedWindow();
			clearCraftingWindow();
			local inv = ::Screens.get("Inventory",false)
			if(inv)
				inv.unlockAllActions();
		}
		else
		{
			this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "craftingScreenDistanceCheck");
		}
		
	}

	function _bottomComponent()
	{
		local bottomComp = this.GUI.Container(this.GUI.BoxLayout());
		bottomComp.setInsets(0, this.INSET_AMOUNT, 0, this.INSET_AMOUNT);
		bottomComp.getLayoutManager().setAlignment(0.0);
		bottomComp.getLayoutManager().setPackAlignment(0.0);
		bottomComp.getLayoutManager().setGap(10);
		local leftComp = this.GUI.Component(this.GUI.BoxLayoutV());
		leftComp.getLayoutManager().setGap(7);
		leftComp.setSize(256, 200);
		leftComp.setPreferredSize(256, 200);
		leftComp.getLayoutManager().setAlignment(0.0);
		bottomComp.add(leftComp);
		local COMPONENT_INSET = 2;
		this.mKeyItemComp = this.GUI.CraftingItemCount();
		this.mKeyItemComp.addListener(this);
		this.mKeyItemComp.setAppearance("InnerPanel");
		this.mKeyItemComp.setInsets(COMPONENT_INSET + 2, 7, COMPONENT_INSET, 10);
		leftComp.add(this.mKeyItemComp);
		local recipeComponent = this.GUI.InnerPanel(this.GUI.BoxLayoutV());
		recipeComponent.getLayoutManager().setAlignment(0.0);
		recipeComponent.setInsets(0, 7, 0, 10);
		leftComp.add(recipeComponent);

		for( local i = 0; i < this.MAX_COMPONENTS; i++ )
		{
			local itemCraftingLabel = this.GUI.CraftingItemCount();
			itemCraftingLabel.addListener(this);
			this.mCraftingItemsComps.append(itemCraftingLabel);
			recipeComponent.add(itemCraftingLabel);
		}

		local buttonComp = this.GUI.Component(this.GUI.BoxLayout());
		buttonComp.getLayoutManager().setPackAlignment(0.5);
		buttonComp.getLayoutManager().setGap(30);
		buttonComp.setInsets(0, 0, 0, 38);
		leftComp.add(buttonComp);
		this.mCancelButton = this.GUI.RedNarrowButton("Cancel");
		this.mCancelButton.setFixedSize(74, 32);
		this.mCancelButton.addActionListener(this);
		this.mCancelButton.setReleaseMessage("_cancelPress");
		buttonComp.add(this.mCancelButton);
		this.mCreateButton = this.GUI.NarrowButton("Create");
		this.mCreateButton.setFixedSize(74, 32);
		this.mCreateButton.addActionListener(this);
		this.mCreateButton.setReleaseMessage("_createCraftingItemPress");
		this.mCreateButton.setEnabled(false);
		buttonComp.add(this.mCreateButton);
		this.mBackgroudComp = this.GUI.Component(null);
		this.mBackgroudComp.setSize(147, 195);
		this.mBackgroudComp.setPreferredSize(147, 195);
		this.mBackgroudComp.setAppearance("Crafting/DefaultBG");
		bottomComp.add(this.mBackgroudComp);
		return bottomComp;
	}

	function setContainerMoveProperties()
	{
		this.mListContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this, this));
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		local itemData = action.mItemData;
		
		
		if(newSlot.getActionContainer().getContainerName() == "advcraft") 
		{
			return true;
		}
		else
		{

			if (itemData.mBound)
			{
				this.IGIS.error("This item is bound to you");
				return false;
			}
	
			if (itemData && itemData.getType() == this.ItemType.RECIPE)
			{
				local playerLevel = ::_avatar.getStat(this.Stat.LEVEL, true);
				local itemDef = ::_ItemDataManager.getItemDef(itemData.mItemDefId);
	
				if (itemDef.getMinUseLevel() > playerLevel)
				{
					this.IGIS.error("You current level is insufficient to create that item!");
					return false;
				}
	
				return true;
			}
	
			this.IGIS.error("You can only place recipe items in here.");
		}
		
		return false;
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		if (this.mListContainer == container)
		{
			this.clearCraftingWindow();
			local item = container.getSlotContents(slotIndex);
			local itemDefId = item.getActionButton().getAction().getItemDefId();
			this._handleListItemAdded(itemDefId);
		}
		
		if (this.mAdvancedContainer == container)
		{
			mAdvancedCraftButton.setEnabled(!this.mAdvancedContainer.isContainerEmpty());
		}
	}
	
	function getAdvCraftContainer()
		return this.mAdvancedContainer;

	function onActionButtonLost( container, slot )
	{
		this.clearCraftingWindow();
		this.clearAdvancedWindow();
	}

	function getActionButtonValid( slot, button )
	{
		local action = button.getAction();

		if (!action)
		{
			return false;
		}

		return true;
	}

	function onCraftReceipeRemove( action )
	{
		local buttons = this.mListContainer.getAllActionButtons(false);

		foreach( slot in buttons )
		{
			if (slot)
			{
				local craftAction = slot.getAction();

				if (craftAction && action.getItemId() == craftAction.getItemId())
				{
					local slotIndex = this.mListContainer.findSlotIndexOfAction(craftAction);
					this.mListContainer._removeActionButton(slotIndex);
					this.clearCraftingWindow();
				}
			}
		}
	}

	function onCraftCountUpdate( craftComponent )
	{
		local isCompleted = true;

		if (!this.mKeyItemComp.isComplete())
		{
			isCompleted = false;
		}

		foreach( i, craftComp in this.mCraftingItemsComps )
		{
			if (!craftComp.isComplete())
			{
				isCompleted = false;
				break;
			}
		}

		this.mCreateButton.setEnabled(isCompleted);
	}

	function setBackgroundImage( image )
	{
		this.mBackgroudComp.setAppearance(image);
	}

	function _cancelPress( button )
	{
		this.Screens.toggle("CraftingWindow");
		this.mListContainer.removeAllActions();
		this.mAdvancedContainer.removeAllActions();
		this.clearCraftingWindow();
		this.clearAdvancedWindow();
	}

	function onFrameClosed()
	{
		this._cancelPress(null);
	}

	function _createCraftingItemPress( button )
	{
		if (::_Connection.getProtocolVersionId() >= 10)
		{
			this.mAdvancedCraftButton.setEnabled(false);
			if (this.mListItemDefId != 0)
			{
				::_Connection.sendQuery("craft.create", this, [
					this.mListItemDefId,
					this.mCrafterId
				]);
			}
			else
			{
				this.IGIS.error("No list current being crafted");
			}
		}
		else
		{
			::_Connection.sendQuery("craft.create", this, this.mListItemDefId);
		}
	}
	
	function onQueryComplete( qa, rows )
	{
		this.mAdvancedCraftButton.setEnabled(true);
		if (qa.query == "mod.craft")
		{
			mAdvancedContainer.removeAllActions();
			this.clearAdvancedWindow();
		}
		else
		{
			this.mListContainer.removeAllActions();
			this.clearCraftingWindow();
		}
		this.IGIS.info("Item has been crafted");
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " failed: " + error);
	}
	
	function clearAdvancedWindow()
	{
		if(this.mAdvancedContainer) {
			this.mAdvancedContainer.removeAllActions();
			this.mAdvancedCraftButton.setEnabled(false);
		}
	}

	function clearCraftingWindow()
	{
		if(mCreatedItemContainer) {
			this.mCreatedItemLabel.setText("");
			this.mCreatedItemContainer.removeAllActions();
			this.mCreatedItemContainer.setVisible(false);
			this.mCreatedItemId = 0;
			this.mListItemDefId = 0;
			this.mKeyItemComp.clear();
	
			foreach( itemComps in this.mCraftingItemsComps )
			{
				itemComps.clear();
			}
	
			this.mCreateButton.setEnabled(false);
		}
	}

	function _handleListItemAdded( itemDefId )
	{
		this.mListItemDefId = itemDefId;
		local itemDef = ::_ItemDataManager.getItemDef(this.mListItemDefId);

		if (!itemDef && itemDef.getType() != this.ItemType.RECIPE)
		{
			return;
		}

		this.mCreatedItemId = itemDef.getResultItem();
		local backgroundImage = "Crafting/DefaultBG";
		local keyCompItemDefId = itemDef.getKeyComponent();
		local keyTotalCount = 1;
		local componentIds = itemDef.getCraftComponent();
		local item = ::_ItemManager.getItemDef(this.mCreatedItemId);
		local itemBeingCreatedActionButton = this.mCreatedItemContainer.addAction(item, true);
		this.mCreatedItemLabel.setText(::_ItemDataManager.getItemDef(this.mCreatedItemId).getDisplayName());

		if (keyCompItemDefId != 0)
		{
			this.mKeyItemComp.updateTotalCount(keyTotalCount);
			this.mKeyItemComp.updateItemDefId(keyCompItemDefId);
		}

		local i = 0;

		foreach( id, count in componentIds )
		{
			this.mCraftingItemsComps[i].updateTotalCount(count);
			this.mCraftingItemsComps[i].updateItemDefId(id);
			i = i + 1;
		}

		this.setBackgroundImage(backgroundImage);
		this.mCreatedItemContainer.setVisible(true);
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (this.mCreatedItemId == 0 || itemDefId != this.mCreatedItemId)
		{
			return;
		}

		this.mCreatedItemLabel.setText(::_ItemDataManager.getItemDef(this.mCreatedItemId).getDisplayName());
	}

}

