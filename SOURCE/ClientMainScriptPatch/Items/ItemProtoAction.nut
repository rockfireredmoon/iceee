class this.ItemProtoAction extends this.Action
{
	mItemDefId = -1;
	mProto = null;
	mStackCount = 1;
	mInfoFrame = null;
	mItemId = "";
	mLookId = null;
	
	constructor( proto )
	{
		this.mProto = proto;
		local attributes = this.Util.split(proto, ":");
		local itemIndex = attributes[0].find("item");

		if (itemIndex == null)
		{
			return;
		}

		local itemId = attributes[0].slice(itemIndex + 4);
		mLookId = attributes[1].tointeger();
		this.mStackCount = attributes[2].tointeger() + 1;
		local itemDef = ::_ItemDataManager.getItemDef(itemId.tointeger());
		this.Action.constructor(itemDef.mDisplayName, itemDef.mIcon);
		this.mItemDefId = itemId.tointeger();
		::_ItemDataManager.addListener(this);
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

	function getType()
	{
		return "itemProto";
	}

	function getItemDefId()
	{
		return this.mItemDefId;
	}

	function getProto()
	{
		return this.mProto;
	}

	function getNumStacks()
	{
		return this.mStackCount;
	}

	function getTooltip( mods )
	{
		local itemdef = this._getItemDef();

		if (itemdef)
		{
			local showbuyvalue = false;

			if (mods && "showBuyValue" in mods)
			{
				showbuyvalue = mods.showBuyValue;
			}

			return itemdef.getTooltip(showbuyvalue);
		}
		else
		{
			return "Loading...";
		}
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		local storeditemDef = this._getItemDef();

		if (itemDefId == storeditemDef.mID)
		{
			if (itemDef)
			{
				this.setName(itemDef.mDisplayName);
				this.setImage(itemDef.mIcon);
			}
		}
	}

	function _getItemDef()
	{
		return ::_ItemDataManager.getItemDef(this.mItemDefId);
	}

	function showExtraDataScreen()
	{
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!this.mInfoFrame)
		{
			this.mInfoFrame = this.GUI.Frame("Item Proto Action: " + this.mName);
		}

		this.mInfoFrame.setVisible(true);
		local component = this.GUI.Component(this.GUI.BoxLayoutV());
		component.setInsets(5, 5, 5, 5);
		component.getLayoutManager().setAlignment(0);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = this.Util.addNewTextLine(textString, "Stack Count", this.getNumStacks());
		textString = this.Util.addNewTextLine(textString, "Foreground Image", this.mForegroundImage);
		textString = this.Util.addNewTextLine(textString, "Background Image", this.mBackgroundImage);
		height = heightSize * 4;
		local isRecipe = false;

		if (this._getItemDef())
		{
			local data = this.Util.addItemDefDataInfo(textString, this._getItemDef(), height, heightSize);
			textString = data.text;
			height = data.height;
			isRecipe = data.isRecipe;
		}

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
		local itemDefData;

		if (this.getItemDefId() && ::_ItemDataManager.getItemDef(this.getItemDefId()))
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
		local itemDefData;

		if (this.getItemDefId() && ::_ItemDataManager.getItemDef(this.getItemDefId()))
		{
			itemDefData = ::_ItemDataManager.getItemDef(this.getItemDefId());
		}

		if (itemDefData)
		{
			::_Connection.sendQuery("item.create", null, itemDefData.mResultItem);
		}
	}

}

