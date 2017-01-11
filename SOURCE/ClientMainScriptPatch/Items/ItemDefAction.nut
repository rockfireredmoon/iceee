this.require("ActionManager");
class this.ItemDefAction extends this.Action
{
	mItemDefId = null;
	mItemDefData = null;
	mNumStacks = 0;
	mAbility = null;
	mLooks = 0;
	mInfoFrame = null;
	constructor( name, icon, itemDefId, itemdef )
	{
		this.Action.constructor(name, icon);
		this.mItemDefId = itemDefId;
		this._updateItemDef(itemdef);
		this.mAbility = null;
	}

	function setAbility( ability )
	{
		this.mAbility = ability;
	}

	function getNumStacks()
	{
		return this.mNumStacks;
	}

	function updateStackCount()
	{
		this.mNumStacks = ::_ItemDataManager.getDefIdStackCount(this.mItemDefId);

		if (this.mItemDefData.mType == this.ItemType.CONSUMABLE || this.mItemDefData.mIvType1 == this.ItemIntegerType.STACKING)
		{
			::_quickBarManager.updateDefIdStackCount(this.mItemDefId, this.mNumStacks);
		}
	}

	function getTooltip( mods )
	{
		return this.mItemDefData.getTooltip(mods);
	}

	function getIsValid()
	{
		return this.mItemDefData && this.mItemDefData.mValid;
	}

	function getInfoPanel( mods )
	{
		local itemdef = this.mItemDefData;

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
		local def = this.mItemDefData;
		return def == null ? false : def.isUsable();
	}

	function isAvailableForUse()
	{
		local timeUntilAvail = this.getTimeUntilAvailable();
		local timeUntilAvail = this.getTimeUntilAvailable();

		if (timeUntilAvail <= 0)
		{
			return ::_AbilityManager.getTimeUntilCategoryUseable("Global") <= 0;
		}
		else
		{
			return false;
		}

		if (this.mNumStacks != 0)
		{
			return true;
		}

		return false;
	}

	function getTimeUntilAvailable()
	{
		if (this.mAbility)
		{
			return this.mAbility.getTimeUntilAvailable();
		}

		return 0;
	}

	function isWarmingUp()
	{
		if (this.mAbility)
		{
			return this.mAbility.isWarmingUp();
		}

		return false;
	}

	function getWarmupStartTime()
	{
		if (this.mAbility)
		{
			return this.mAbility.getWarmupStartTime();
		}

		return 0;
	}

	function getWarmupEndTime()
	{
		if (this.mAbility)
		{
			return this.mAbility.getWarmupEndTime();
		}

		return 0;
	}

	function getWarmupTimeLeft()
	{
		if (this.mAbility)
		{
			return this.mAbility.getWarmupTimeLeft();
		}

		return 0;
	}

	function getWarmupDuration()
	{
		if (this.mAbility)
		{
			return this.mAbility.getWarmupDuration();
		}

		return 0;
	}

	function getAbilityDuration()
	{
		if (this.mAbility)
		{
			return this.mAbility.getDuration();
		}

		return 0;
	}

	function getChannelTimeLeft()
	{
		if (this.mAbility)
		{
			return this.mAbility.getChannelTimeLeft();
		}

		return 0;
	}

	function getChannelStartTime()
	{
		if (this.mAbility)
		{
			return this.mAbility.getChannelStartTime();
		}

		return 0;
	}

	function getChannelEndTime()
	{
		if (this.mAbility)
		{
			return this.mAbility.getChannelEndTime();
		}

		return 0;
	}

	function getTimeUsed()
	{
		if (this.mAbility)
		{
			return this.mAbility.getTimeUsed();
		}

		return 0;
	}

	function getRange()
	{
		if (this.mAbility)
		{
			return this.mAbility.getRange();
		}

		return -1;
	}

	function cancel()
	{
	}

	function sendActivationRequest()
	{
		print("ICE! ItemDefAction sendActivationRequest\n");
		if (this.isAvailableForUse())
		{
			local selection = this._avatar.getTargetObject();

			if (selection)
			{
				print("ICE! sendActivationRequest with selection\n");
				::_Connection.sendQuery("item.def.use", this, [
					this.mItemDefId,
					selection
				]);
			}
			else
			{
				print("ICE! sendActivationRequest with default\n");
				::_Connection.sendQuery("item.def.use", this, [
					this.mItemDefId
				]);
			}

			return true;
		}
		else
			print("ICE! sendActivationRequest not available\n");

		return false;
	}

	function getType()
	{
		return "itemdef";
	}

	function getQuickbarString()
	{
		return this.getType() + ":" + this.mItemDefId.tostring();
	}

	function _updateItemDef( itemDefData )
	{
		this.mItemDefData = itemDefData;
		this.updateStackCount();

		if (itemDefData)
		{
			this.setName(itemDefData.mDisplayName);
			this.setImage(itemDefData.mIcon);
		}
	}

	function showExtraDataScreen()
	{
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!this.mInfoFrame)
		{
			this.mInfoFrame = this.GUI.Frame("Item Def Action: " + this.mName);
		}

		this.mInfoFrame.setVisible(true);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = this.Util.addNewTextLine(textString, "Stack Count", this.getNumStacks());
		textString = this.Util.addNewTextLine(textString, "Foreground Image", this.mForegroundImage);
		textString = this.Util.addNewTextLine(textString, "Background Image", this.mBackgroundImage);
		height = heightSize * 3;
		local data;

		if (this.mItemDefData)
		{
			data = this.Util.addItemDefDataInfo(textString, this.mItemDefData, height, heightSize);
		}
		else if (this.mItemDefId && ::_ItemDataManager.getItemDef(this.mItemDefId))
		{
			data = this.Util.addItemDefDataInfo(textString, ::_ItemDataManager.getItemDef(this.mItemDefId), height, heightSize);
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
		::_Connection.sendQuery("item.create", null, this.mItemDefId);
	}

	function onCreateRecipeComponents( button )
	{
		local itemDefData = this.mItemDefData;

		if (!this.mItemDefData && this.mItemDefId && ::_ItemDataManager.getItemDef(this.mItemDefId))
		{
			itemDefData = ::_ItemDataManager.getItemDef(this.mItemDefId);
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

		if (!this.mItemDefData && this.mItemDefId && ::_ItemDataManager.getItemDef(this.mItemDefId))
		{
			itemDefData = ::_ItemDataManager.getItemDef(this.mItemDefId);
		}

		if (itemDefData)
		{
			::_Connection.sendQuery("item.create", null, itemDefData.mResultItem);
		}
	}

}

