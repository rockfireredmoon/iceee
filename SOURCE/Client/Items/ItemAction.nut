this.require("ActionManager");
class this.ItemAction extends this.Action
{
	mItemId = null;
	mItemData = null;
	mItemDefData = null;
	mLookDefData = null;
	mNumUses = null;
	mIsStacking = false;
	mInfoFrame = null;
	constructor( name, icon, itemId, item )
	{
		this.Action.constructor(name, icon);
		this.mItemId = itemId;
		this._updateItem(item, null, null);
	}

	function isValid()
	{
		return this.mItemData.isValid();
	}

	function getItemDefId()
	{
		if (this.mItemData == null)
		{
			return null;
		}
		else
		{
			return this.mItemData.mItemDefId;
		}
	}

	function _getItemDef()
	{
		if (this.mItemData == null || this.mItemData.mItemDefId == null || this.mItemData.mItemDefId <= 0)
		{
			return null;
		}

		return ::_ItemDataManager.getItemDef(this.mItemData.mItemDefId);
	}

	function _getItemDefAction()
	{
		if (this.mItemData == null || this.mItemData.mItemDefId == null || this.mItemData.mItemDefId <= 0)
		{
			return null;
		}

		return ::_ItemManager.getItemDef(this.mItemData.mItemDefId);
	}

	function getNumStacks()
	{
		return this.mIsStacking ? this.mNumUses : 0;
	}

	function getTooltip( mods, ... )
	{
		local force = false;
		local optionalComponent;

		if (vargc > 0)
		{
			force = vargv[0];
		}

		force = true;

		if (vargc > 1)
		{
			optionalComponent = vargv[1];
		}

		local itemdef = this._getItemDef();

		if (itemdef)
		{
			local showbuyvalue = false;

			if (mods && "showBuyValue" in mods)
			{
				showbuyvalue = mods.showBuyValue;
			}

			local item;

			if (mods && "item" in mods)
			{
				item = mods.item;
			}

			local showBindingInfo = true;

			if (mods && "showBindingInfo" in mods)
			{
				showBindingInfo = mods.showBindingInfo;
			}

			return itemdef.getTooltip(mods, force, optionalComponent, item, showBindingInfo);
		}
		else
		{
			return "Loading...";
		}
	}

	function getEquipmentType()
	{
		if (this.mItemDefData)
		{
			return this.mItemDefData.mEquipType;
		}
		else
		{
			return this.Action.getEquipmentType();
		}
	}

	function getInfoPanel( mods )
	{
		local itemdef = this._getItemDef();

		if (itemdef)
		{
			local showbuyvalue = false;

			if (mods && "showBuyValue" in mods)
			{
				showbuyvalue = mods.showBuyValue;
			}

			local miniVersion = false;

			if (mods && "miniVersion" in mods)
			{
				miniVersion = mods.miniVersion;
			}

			local hideValue = false;

			if (mods && "hideValue" in mods)
			{
				hideValue = mods.hideValue;
			}

			return itemdef.getInfoPanel(showbuyvalue, miniVersion, hideValue);
		}
		else
		{
			return this.Action.getInfoPanel(null);
		}
	}

	function isUsable()
	{
		local def = this._getItemDef();
		return def == null ? false : def.isUsable();
	}

	function isAvailableForUse()
	{
		if (this.mNumUses == null)
		{
			return false;
		}

		return this.mNumUses > 0;
	}

	function getQuickBarAction()
	{
		if (this.mItemDefData && (this.mItemDefData.mType == this.ItemType.CONSUMABLE || this.mItemDefData.mIvType1 == this.ItemIntegerType.STACKING))
		{
			return this._getItemDefAction();
		}
		else
		{
			return this;
		}
	}

	function getType()
	{
		return "item";
	}

	function getItemId()
	{
		return this.mItemId;
	}

	function getQuickbarString()
	{
		return this.getType() + ":" + this.mItemId.tostring();
	}

	function cancel()
	{
	}

	function sendActivationRequest()
	{
		if (this.isAvailableForUse())
		{
			::_Connection.sendQuery("item.use", this, [
				this.mItemId
			]);
			return true;
		}
		else
		{
			return false;
		}
	}

	function onQueryError( qa, msg )
	{
		if (qa.query == "item.use")
		{
			this.IGIS.error(msg);
		}
	}

	function split( newStackSize )
	{
		::_Connection.sendQuery("item.split", this, [
			this.mItemId,
			newStackSize
		]);
	}

	function onQueryComplete( qa, results )
	{
	}

	function modifiedAction( actionbutton, shift, alt, control )
	{
		if (control)
		{
			if (::Screens.get("TradeScreen", true).isVisible())
			{
				local tradeScreen = ::Screens.get("TradeScreen", true);
				local inventory = ::Screens.get("Inventory", true);
				local currentSlot = actionbutton.getActionButtonSlot();

				if (currentSlot && currentSlot.getActionContainer())
				{
					local currentcontainer = currentSlot.getActionContainer();

					if (currentcontainer.isIndexLocked(currentSlot))
					{
						return;
					}

					switch(currentcontainer.getContainerName())
					{
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

	function _updateItem( itemData, itemDefData, lookItemDefData )
	{
		this.mItemData = itemData;
		this.mItemDefData = itemDefData;
		this.mLookDefData = lookItemDefData;
		this.mNumUses = itemData.getStackCount();

		if (this.mNumUses != null)
		{
			this.mNumUses += 1;
			this.mIsStacking = true;
		}
		else
		{
			this.mNumUses = null;
		}

		if (itemDefData)
		{
			this.mName = itemDefData.mDisplayName;
		}

		if (lookItemDefData)
		{
			this.setImage(lookItemDefData.mIcon);
		}
	}

	function showExtraDataScreen()
	{
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!this.mInfoFrame)
		{
			this.mInfoFrame = this.GUI.Frame("Item Action: " + this.mName);
		}

		this.mInfoFrame.setVisible(true);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = this.Util.addNewTextLine(textString, "Item Id", this.mItemId);
		textString = this.Util.addNewTextLine(textString, "Stack Count", this.getNumStacks());
		textString = this.Util.addNewTextLine(textString, "Foreground Image", this.mForegroundImage);
		textString = this.Util.addNewTextLine(textString, "Background Image", this.mBackgroundImage);
		height = heightSize * 4;
		local data = this.Util.addItemDataInfo(textString, this.mItemData, height, heightSize);
		textString = data.text;
		height = data.height;

		if (this.mItemDefData)
		{
			data = this.Util.addItemDefDataInfo(textString, this.mItemDefData, height, heightSize);
		}
		else if (this.getItemDefId() && ::_ItemDataManager.getItemDef(this.getItemDefId()))
		{
			data = this.Util.addItemDefDataInfo(textString, ::_ItemDataManager.getItemDef(this.getItemDefId()), height, heightSize);
		}

		textString = data.text;
		height = data.height;
		local isRecipe = data.isRecipe;
		local htmlComp = this.GUI.HTML("");
		htmlComp.setInsets(0, 5, 0, 5);
		htmlComp.setWrapText(true, htmlComp.getFont(), WIDTH - 50);
		htmlComp.setText(textString);
		local baseComp = this.GUI.Component(this.GUI.BoxLayoutV());
		baseComp.setInsets(5, 5, 5, 5);
		baseComp.getLayoutManager().setAlignment(0.5);

		if (height > MAX_HEIGHT)
		{
			this.mInfoFrame.setSize(WIDTH, MAX_HEIGHT);
			this.mInfoFrame.setPreferredSize(WIDTH, MAX_HEIGHT);
			local scrollArea = ::GUI.ScrollPanel();
			scrollArea.setSize(WIDTH, MAX_HEIGHT - 60);
			scrollArea.setPreferredSize(WIDTH, MAX_HEIGHT - 60);
			scrollArea.attach(htmlComp);
			baseComp.add(scrollArea);
		}
		else
		{
			this.mInfoFrame.setSize(WIDTH, height + 25);
			this.mInfoFrame.setPreferredSize(WIDTH, height + 25);
			baseComp.add(htmlComp);
		}

		local bottomComp = this.GUI.Component(this.GUI.BoxLayout());
		baseComp.add(bottomComp);
		local button = this.GUI.Button("Create Item");
		button.setPressMessage("onCreateItem");
		button.addActionListener(this);
		bottomComp.add(button);

		if (isRecipe)
		{
			local createRecipeButton = this.GUI.Button("Create Recipe Components");
			createRecipeButton.setPressMessage("onCreateRecipeComponents");
			createRecipeButton.addActionListener(this);
			bottomComp.add(createRecipeButton);
			local createResultButton = this.GUI.Button("Create Result Item");
			createResultButton.setPressMessage("onCreateResultItem");
			createResultButton.addActionListener(this);
			bottomComp.add(createResultButton);
		}

		this.mInfoFrame.setContentPane(baseComp);
	}

	function onCreateItem( button )
	{
		::_Connection.sendQuery("item.create", null, this.getItemDefId());
	}

	function onCreateRecipeComponents( button )
	{
		local itemDefData = this.mItemDefData;

		if (!this.mItemDefData && this.getItemDefId() && ::_ItemDataManager.getItemDef(this.getItemDefId()))
		{
			itemDefData = ::_ItemDataManager.getItemDef(this.getItemDefId());
		}

		if (itemDefData)
		{
			::_Connection.sendQuery("item.create", null, itemDefData.mKeyComponent);

			foreach( itemDefId, amount in itemDefData.mCraftComponents )
			{
				for( local i = 0; i < amount; i++ )
				{
					::_Connection.sendQuery("item.create", null, itemDefId);
				}
			}
		}
	}

	function onCreateResultItem( button )
	{
		local itemDefData = this.mItemDefData;

		if (!this.mItemDefData && this.getItemDefId() && ::_ItemDataManager.getItemDef(this.getItemDefId()))
		{
			itemDefData = ::_ItemDataManager.getItemDef(this.getItemDefId());
		}

		if (itemDefData)
		{
			::_Connection.sendQuery("item.create", null, itemDefData.mResultItem);
		}
	}

}

