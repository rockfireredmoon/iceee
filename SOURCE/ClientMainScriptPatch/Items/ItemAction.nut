require("ActionManager");

/**
	Implement the Action class as it pertains to items. 
*/
class ItemAction extends Action {
	/** The server-recognized itemId this item is associated with. */
	mItemId = null;
	
	/** The server-provided data for this item */
	mItemData = null;
	
	/** The server-provided data for the behavior of this item */
	mItemDefData = null;
	
	/** The server-provided data for the look of this item */
	mLookDefData = null;
			
	/** How many "uses" of the item we have */
	mNumUses = null;
	
	/** True if this represents a stacking object */
	mIsStacking = false;
	mInfoFrame = null;
	
	constructor( name, icon, itemId, item )	{
		Action.constructor(name, icon);
		mItemId = itemId;
		_updateItem(item, null, null);
	}

	
	/**	
		Returns true if all data is valid and ready for this item.		
	*/
	function isValid() {
		return mItemData.isValid();
	}

	function getItemDefId() {
		if (mItemData == null) {
			return null;
		}
		else {
			return mItemData.mItemDefId;
		}
	}

	/**
		Get the ItemDef this item uses.
	*/
	function _getItemDef() {
		if (mItemData == null || mItemData.mItemDefId == null || mItemData.mItemDefId <= 0)	{
			return null;
		}

		return ::_ItemDataManager.getItemDef(mItemData.mItemDefId);
	}


	/**
		Get the ItemDef this item uses.
	*/
	function _getItemDefAction() {
		if (mItemData == null || mItemData.mItemDefId == null || mItemData.mItemDefId <= 0)	{
			return null;
		}

		return ::_ItemManager.getItemDef(mItemData.mItemDefId);
	}
		
	/**	
		Gets the number of items/uses this item has.		
		
	*/
	function getNumStacks() {
		return mIsStacking ? mNumUses : 0;
	}

	
	/**
		Returns useful information about this item for display to the user.
	*/
	function getTooltip( mods, ... ) {
		local force = false;
		local optionalComponent;

		if (vargc > 0) {
			force = vargv[0];
		}

		force = true;

		if (vargc > 1) {
			optionalComponent = vargv[1];
		}

		local itemdef = _getItemDef();

		if (itemdef) {
			local showbuyvalue = false;

			if (mods && "showBuyValue" in mods) 	{
				showbuyvalue = mods.showBuyValue;
			}

			local item;

			if (mods && "item" in mods)	{
				item = mods.item;
			}

			local showBindingInfo = true;

			if (mods && "showBindingInfo" in mods) {
				showBindingInfo = mods.showBindingInfo;
			}

			return itemdef.getTooltip(mods, force, optionalComponent, item, showBindingInfo);
		}
		else {
			return "Loading...";
		}
	}

	function getEquipmentType() {
		if (mItemDefData) {
			return mItemDefData.mEquipType;
		}
		else {
			return Action.getEquipmentType();
		}
	}

	
	/**
		Returns a sort of 'mini' tooltip with just the vitals, to be displayed when the
		ActionContainer is in 'wide' mode.
		
		@return - The vitals in a GUI.Component
	*/
	function getInfoPanel( mods ) {
		local itemdef = _getItemDef();

		if (itemdef) {
			local showbuyvalue = false;

			if (mods && "showBuyValue" in mods)	{
				showbuyvalue = mods.showBuyValue;
			}

			local miniVersion = false;

			if (mods && "miniVersion" in mods) {
				miniVersion = mods.miniVersion;
			}

			local hideValue = false;

			if (mods && "hideValue" in mods) {
				hideValue = mods.hideValue;
			}

			return itemdef.getInfoPanel(showbuyvalue, miniVersion, hideValue);
		}
		else {
			return Action.getInfoPanel(null);
		}
	}

	function isUsable() {
		local def = _getItemDef();
		return def == null ? false : def.isUsable();
	}

	/**
		Determine's if the item can be used, based on stack count first
		and then the presence/state of an associated ability.
	*/
	function isAvailableForUse() {
		if (mNumUses == null) {
			return false;
		}

		return mNumUses > 0;
	}

	function getQuickBarAction() {
		//If this is a consumable item, use the item Def as the action
		//otherwise use the item ( since each item is unique )
		if (mItemDefData && (mItemDefData.mType == ItemType.CONSUMABLE || mItemDefData.mIvType1 == ItemIntegerType.STACKING)) {
			return _getItemDefAction();
		}
		else {
			return this;
		}
	}
	
	/**
		Provides the class-level type of this action.
	*/
	function getType() {
		return "item";
	}
	
	/**
		Returns the ItemId associated with this button.
	*/
	function getItemId() {
		return mItemId;
	}

	function getQuickbarString() {
		return getType() + ":" + mItemId.tostring();
	}
	
	/**
		Cancel the activation of this item
	*/
	function cancel() {
	}

	/**
		Activate the ability associated with this item.
	*/
	function sendActivationRequest() {
		print("ICE! sendActivationRequest " + mItemId + "\n");
		if (isAvailableForUse()) {
			print("ICE! using " + mItemId + "\n");
			::_Connection.sendQuery("item.use", this, [
				mItemId
			]);
			return true;
		}
		else {
			print("ICE! not usable " + mItemId + "\n");
			return false;
		}
	}

	function onQueryError( qa, msg ) {
		if (qa.query == "item.use")	{
			IGIS.error(msg);
		}
	}

	function split( newStackSize ) {
		::_Connection.sendQuery("item.split", this, [
			mItemId,
			newStackSize
		]);
	}

	function onQueryComplete( qa, results ) {
	}

	
	/**
		Preform a special operation based on the ctrl, shif, and alt keys.  
	*/
	function modifiedAction( actionbutton, shift, alt, control ) {
		if (control) {
			if (::Screens.get("TradeScreen", true).isVisible())	{
				local tradeScreen = ::Screens.get("TradeScreen", true);
				local inventory = ::Screens.get("Inventory", true);
				local currentSlot = actionbutton.getActionButtonSlot();

				if (currentSlot && currentSlot.getActionContainer()) {
					local currentcontainer = currentSlot.getActionContainer();

					if (currentcontainer.isIndexLocked(currentSlot)) {
						return;
					}
								
					
					// Determine if there is a scenario we can do an automated move for
					// We don't have a container manager yet, so while this is a bit naughty, it gets
					// the job done until we have one.
					
					// TODO: Add a container manager.

					switch(currentcontainer.getContainerName()) {
					case "inventory":
						tradeScreen.getMyTradingContainer().simulateButtonDrop(actionbutton);
						break;

					case "trade_avatar":
						inventory.mInventoryContainer.simulateButtonDrop(actionbutton);
						break;
					}
				}
			}
		}
	}

	function showExtraDataScreen() {
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!mInfoFrame) {
			mInfoFrame = GUI.Frame("Item Action: " + mName);
		}

		mInfoFrame.setVisible(true);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = Util.addNewTextLine(textString, "Item Id", mItemId);
		textString = Util.addNewTextLine(textString, "Stack Count", getNumStacks());
		textString = Util.addNewTextLine(textString, "Foreground Image", mForegroundImage);
		textString = Util.addNewTextLine(textString, "Background Image", mBackgroundImage);
		height = heightSize * 4;
		local data = Util.addItemDataInfo(textString, mItemData, height, heightSize);
		textString = data.text;
		height = data.height;

		if (mItemDefData)
		{
			data = Util.addItemDefDataInfo(textString, mItemDefData, height, heightSize);
		}
		else if (getItemDefId() && ::_ItemDataManager.getItemDef(getItemDefId()))
		{
			data = Util.addItemDefDataInfo(textString, ::_ItemDataManager.getItemDef(getItemDefId()), height, heightSize);
		}

		textString = data.text;
		height = data.height;
		local isRecipe = data.isRecipe;
		local htmlComp = GUI.HTML("");
		htmlComp.setInsets(0, 5, 0, 5);
		htmlComp.setWrapText(true, htmlComp.getFont(), WIDTH - 50);
		htmlComp.setText(textString);
		local baseComp = GUI.Component(GUI.BoxLayoutV());
		baseComp.setInsets(5, 5, 5, 5);
		baseComp.getLayoutManager().setAlignment(0.5);

		if (height > MAX_HEIGHT)
		{
			mInfoFrame.setSize(WIDTH, MAX_HEIGHT);
			mInfoFrame.setPreferredSize(WIDTH, MAX_HEIGHT);
			local scrollArea = ::GUI.ScrollPanel();
			scrollArea.setSize(WIDTH, MAX_HEIGHT - 60);
			scrollArea.setPreferredSize(WIDTH, MAX_HEIGHT - 60);
			scrollArea.attach(htmlComp);
			baseComp.add(scrollArea);
		}
		else
		{
			mInfoFrame.setSize(WIDTH, height + 25);
			mInfoFrame.setPreferredSize(WIDTH, height + 25);
			baseComp.add(htmlComp);
		}

		local bottomComp = GUI.Component(GUI.BoxLayout());
		baseComp.add(bottomComp);
		local button = GUI.Button("Create Item");
		button.setPressMessage("onCreateItem");
		button.addActionListener(this);
		bottomComp.add(button);

		if (isRecipe)
		{
			local createRecipeButton = GUI.Button("Create Recipe Components");
			createRecipeButton.setPressMessage("onCreateRecipeComponents");
			createRecipeButton.addActionListener(this);
			bottomComp.add(createRecipeButton);
			local createResultButton = GUI.Button("Create Result Item");
			createResultButton.setPressMessage("onCreateResultItem");
			createResultButton.addActionListener(this);
			bottomComp.add(createResultButton);
		}

		mInfoFrame.setContentPane(baseComp);
	}

	function onCreateItem( button ) {
		::_Connection.sendQuery("item.create", null, getItemDefId());
	}

	function onCreateRecipeComponents( button ) {
		local itemDefData = mItemDefData;

		if (!mItemDefData && getItemDefId() && ::_ItemDataManager.getItemDef(getItemDefId())) {
			itemDefData = ::_ItemDataManager.getItemDef(getItemDefId());
		}

		if (itemDefData) {
			::_Connection.sendQuery("item.create", null, itemDefData.mKeyComponent);

			foreach( itemDefId, amount in itemDefData.mCraftComponents ) {
				for( local i = 0; i < amount; i++ )	{
					::_Connection.sendQuery("item.create", null, itemDefId);
				}
			}
		}
	}

	function onCreateResultItem( button ) {
		local itemDefData = mItemDefData;

		if (!mItemDefData && getItemDefId() && ::_ItemDataManager.getItemDef(getItemDefId())) {
			itemDefData = ::_ItemDataManager.getItemDef(getItemDefId());
		}

		if (itemDefData) {
			::_Connection.sendQuery("item.create", null, itemDefData.mResultItem);
		}
	}
	
	// ---------------------------------------------------------
	// Private
	// ---------------------------------------------------------
	
	/**
		Updates the item based on the raw datasets fetched from
		the ItemDataManager.  ItemManager will call this anytime
		information is fetched or changed on the item.
	*/
	function _updateItem( itemData, itemDefData, lookItemDefData ) {
		mItemData = itemData;
		mItemDefData = itemDefData;
		mLookDefData = lookItemDefData;
		mNumUses = itemData.getStackCount();

		if (itemDefData) {
			mName = itemDefData.mDisplayName;
		}

		if(itemDefData && itemDefData.mIvType1 ==  ItemIntegerType.BOOK_PAGE) {
			// Book pages can always be used
			mNumUses = 1
			mIsStacking = false;
		}
		else if (mNumUses != null) {
			mNumUses += 1;
			mIsStacking = true;
		}
		else {
			mNumUses = null;
		}
		
		if (lookItemDefData) {
			setImage(lookItemDefData.mIcon);
		}
	}

}

