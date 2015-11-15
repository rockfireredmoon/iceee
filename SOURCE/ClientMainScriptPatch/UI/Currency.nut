this.require("Globals");
class this.GUI.Currency extends this.GUI.Component
{
	mDefaultCurrencyComp = null;
	mGoldValue = null;
	mSilverValue = null;
	mCopperValue = null;
	mGoldIcon = null;
	mSilverIcon = null;
	mCopperIcon = null;
	mCurrency = 0;
	mGoldComponent = 0;
	mSilverComponent = 0;
	mCopperComponent = 0;
	mCaption = null;
	mBroadcaster = null;
	mCanEditCurrency = false;
	mCurrencyEdit = null;
	mGoldInputBox = null;
	mSilverInputBox = null;
	mCopperInputBox = null;
	constructor( ... )
	{
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.setChildrenInheritTooltip(true);
		this.mBroadcaster = this.MessageBroadcaster();

		if (vargc >= 1)
		{
			this.mCurrency = (vargv[0] + 0.5).tointeger();
			this.breakCurrencyIntoComponents();
		}

		if (vargc >= 2)
		{
			this.mCaption = this.GUI.HTML(vargv[1] + "  ");
			this.add(this.mCaption);
		}
		else
		{
			this.mCaption = this.GUI.HTML();
		}

		this.mDefaultCurrencyComp = this.GUI.Component(this.GUI.BoxLayout());
		this.add(this.mDefaultCurrencyComp);
		this.mGoldIcon = this.GUI.Component();
		this.mGoldIcon.setAppearance("Money/Gold");
		this.mGoldIcon.setPreferredSize(16, 16);
		this.mSilverIcon = this.GUI.Component();
		this.mSilverIcon.setAppearance("Money/Silver");
		this.mSilverIcon.setPreferredSize(16, 16);
		this.mCopperIcon = this.GUI.Container();
		this.mCopperIcon.setAppearance("Money/Copper");
		this.mCopperIcon.setPreferredSize(16, 16);
		this.mGoldValue = this.GUI.Label();
		this.mGoldValue.setFontColor("FFFF00");
		this.mSilverValue = this.GUI.Label();
		this.mSilverValue.setFontColor("D0D0D0");
		this.mCopperValue = this.GUI.Label();
		this.mCopperValue.setFontColor("FFAA00");
		this.mDefaultCurrencyComp.add(this.mGoldIcon);
		this.mDefaultCurrencyComp.add(this.mGoldValue);
		this.mDefaultCurrencyComp.add(this.mSilverIcon);
		this.mDefaultCurrencyComp.add(this.mSilverValue);
		this.mDefaultCurrencyComp.add(this.mCopperIcon);
		this.mDefaultCurrencyComp.add(this.mCopperValue);
		this.updateCurrencyValues();
		this.setCached(::Pref.get("video.UICache"));
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function breakCurrencyIntoComponents()
	{
		local currencyCopy = this.mCurrency;
		this.mGoldComponent = (currencyCopy / this.gCopperPerGold).tointeger();
		currencyCopy -= this.mGoldComponent * this.gCopperPerGold;
		this.mSilverComponent = (currencyCopy / this.gCopperPerSilver).tointeger();
		currencyCopy -= this.mSilverComponent * this.gCopperPerSilver;
		this.mCopperComponent = currencyCopy;
	}

	function setAlignment( align )
	{
		this.getLayoutManager().setPackAlignment(align);
	}

	function getCopper()
	{
		return this.mCopperComponent;
	}

	function getGold()
	{
		return this.mGoldComponent;
	}

	function getSilver()
	{
		return this.mSilverComponent;
	}

	function setAllowCurrencyEdit( value )
	{
		this.mCanEditCurrency = value;

		if (this.mCurrencyEdit)
		{
			this.mCurrencyEdit.setVisible(value);
		}
		else if (value)
		{
			this.add(this._createCurrencyInput());
			this.mCurrencyEdit.setVisible(value);
		}

		this.mDefaultCurrencyComp.setVisible(!value);
	}

	function setCaption( caption )
	{
		this.mCaption.setText(caption + "  ");
	}

	function setCurrentValue( value )
	{
		if (null != value)
		{
			if (value < 65535 - 1)
			{
				this.mCurrency = (value + 0.5).tointeger();
			}
			else
			{
				this.mCurrency = value;
			}
			
			this.breakCurrencyIntoComponents();
			this.updateCurrencyValues();
		}
	}

	function setFont( font )
	{
		this.mCaption.setFont(font);
		this.mGoldValue.setFont(font);
		this.mSilverValue.setFont(font);
		this.mCopperValue.setFont(font);

		if (font.getHeight() < 16)
		{
			this.mGoldIcon.setPreferredSize(8, 8);
			this.mSilverIcon.setPreferredSize(8, 8);
			this.mCopperIcon.setPreferredSize(8, 8);
		}
		else
		{
			this.mGoldIcon.setPreferredSize(12, 12);
			this.mSilverIcon.setPreferredSize(12, 12);
			this.mCopperIcon.setPreferredSize(12, 12);
		}
	}

	function setFontColor( color )
	{
		this.mCaption.setFontColor(color);
		this.mGoldValue.setFontColor(color);
		this.mSilverValue.setFontColor(color);
		this.mCopperValue.setFontColor(color);
	}

	function getCurrentValue()
	{
		return this.mCurrency.tointeger();
	}

	function getCurrencyFromInputBox( box )
	{
		local amt = 0;

		if (box)
		{
			amt = box.getText();

			if ("" == amt)
			{
				amt = -1;
			}
			else
			{
				amt = amt.tointeger();
			}
		}
		else
		{
			return -1;
		}

		return amt;
	}

	function getIputAmount()
	{
		local goldValue = this.getCurrencyFromInputBox(this.mGoldInputBox);

		if (goldValue >= 0)
		{
			this.mGoldComponent = goldValue;
		}

		local silverValue = this.getCurrencyFromInputBox(this.mSilverInputBox);

		if (silverValue >= 0)
		{
			this.mSilverComponent = silverValue;
		}

		local copperValue = this.getCurrencyFromInputBox(this.mCopperInputBox);

		if (copperValue >= 0)
		{
			this.mCopperComponent = copperValue;
		}

		this.updateCurrencyFromComponents();
		this.log.debug("New currency value: " + this.mGoldComponent + "g " + this.mSilverComponent + "s " + this.mCopperComponent + "c --> " + this.getCurrentValue());
		return this.mCurrency.tointeger();
	}
	
	function onTextChanged( text ) {
		this.getIputAmount();
		this.mBroadcaster.broadcastMessage("onCurrencyUpdated", this.mCurrency);
	}

	function onAccepted( evt )
	{
		this.getIputAmount();
		this.mBroadcaster.broadcastMessage("onCurrencyUpdated", this.mCurrency);
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function updateCurrencyValues()
	{
		if (this.mGoldComponent > 0)
		{
			this.mGoldValue.setVisible(true);
			local gap;

			if (this.mSilverComponent > 0 || this.mCopperComponent > 0)
			{
				gap = "  ";
			}
			else
			{
				gap = "";
			}

			this.mGoldValue.setText(this._padDigits(this.mGoldComponent, 1) + gap);
			this.mGoldIcon.setVisible(true);
			
			if(this.mCurrencyEdit) {
				this.mGoldInputBox.setText(this.mGoldComponent.tostring());
			}
		}
		else
		{
			this.mGoldValue.setVisible(false);
			this.mGoldIcon.setVisible(false);
			
			if(this.mCurrencyEdit) {
				this.mGoldInputBox.setText("");
			}
		}

		if (this.mSilverComponent > 0)
		{
			this.mSilverValue.setVisible(true);
			local gap;

			if (this.mCopperComponent > 0)
			{
				gap = "  ";
			}
			else
			{
				gap = "";
			}

			this.mSilverValue.setText(this._padDigits(this.mSilverComponent, 1) + gap);
			this.mSilverIcon.setVisible(true);
			
			
			if(this.mCurrencyEdit) {
				this.mSilverInputBox.setText(this.mSilverComponent.tostring());
			}
		}
		else
		{
			this.mSilverValue.setVisible(false);
			this.mSilverIcon.setVisible(false);
			
			if(this.mCurrencyEdit) {
				this.mSilverInputBox.setText("");
			}
		}

		if (this.mCopperComponent > 0 || this.mSilverComponent == 0 && this.mGoldComponent == 0)
		{
			this.mCopperValue.setVisible(true);
			this.mCopperValue.setText(this._padDigits(this.mCopperComponent, 1));
			this.mCopperIcon.setVisible(true);
			
			if(this.mCurrencyEdit) {
				this.mCopperInputBox.setText(this.mCopperComponent.tostring());
			}
		}
		else
		{
			this.mCopperValue.setVisible(false);
			this.mCopperIcon.setVisible(false);
			
			if(this.mCurrencyEdit) {
				this.mCopperInputBox.setText("");
			}
		}
	}

	function updateCurrencyFromComponents()
	{
		this.mCurrency = this.mGoldComponent * this.gCopperPerGold + this.mSilverComponent * this.gCopperPerSilver + this.mCopperComponent;
		this.updateCurrencyValues();
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _createCurrencyInput()
	{
		this.mCurrencyEdit = this.GUI.Component(this.GUI.BoxLayout(8));
		this.mCurrencyEdit.getLayoutManager().setExpand(false);
		local goldIcon = this.GUI.Component();
		goldIcon.setAppearance("Money/Gold");
		goldIcon.setPreferredSize(16, 16);
		local silverIcon = this.GUI.Component();
		silverIcon.setAppearance("Money/Silver");
		silverIcon.setPreferredSize(16, 16);
		local copperIcon = this.GUI.Component();
		copperIcon.setAppearance("Money/Copper");
		copperIcon.setPreferredSize(16, 16);
		this.mGoldInputBox = this.GUI.InputArea();
		this.mGoldInputBox.setSize(60, 14);
		this.mGoldInputBox.setInsets(1);
		this.mGoldInputBox.setAllowOnlyNumbers(true);
		this.mGoldInputBox.setMaxCharacters(7);
		this.mGoldInputBox.setCenterText(true);
		this.mGoldInputBox.addActionListener(this);
		this.mSilverInputBox = this.GUI.InputArea();
		this.mSilverInputBox.setSize(30, 14);
		this.mSilverInputBox.setInsets(1);
		this.mSilverInputBox.setAllowOnlyNumbers(true);
		this.mSilverInputBox.setMaxCharacters(2);
		this.mSilverInputBox.setCenterText(true);
		this.mSilverInputBox.addActionListener(this);
		this.mCopperInputBox = this.GUI.InputArea();
		this.mCopperInputBox.setSize(30, 14);
		this.mCopperInputBox.setInsets(1);
		this.mCopperInputBox.setAllowOnlyNumbers(true);
		this.mCopperInputBox.setMaxCharacters(2);
		this.mCopperInputBox.setCenterText(true);
		this.mCopperInputBox.addActionListener(this);
		this.mCurrencyEdit.add(goldIcon);
		this.mCurrencyEdit.add(this.mGoldInputBox);
		this.mCurrencyEdit.add(this.GUI.Spacer(5, 1));
		this.mCurrencyEdit.add(silverIcon);
		this.mCurrencyEdit.add(this.mSilverInputBox);
		this.mCurrencyEdit.add(this.GUI.Spacer(5, 1));
		this.mCurrencyEdit.add(copperIcon);
		this.mCurrencyEdit.add(this.mCopperInputBox);
		this.mCurrencyEdit.setVisible(true);
		return this.mCurrencyEdit;
	}

	function resetCurrencyInput()
	{
		if (this.mGoldInputBox)
		{
			this.mGoldInputBox.setText("");
		}

		if (this.mSilverInputBox)
		{
			this.mSilverInputBox.setText("");
		}

		if (this.mCopperInputBox)
		{
			this.mCopperInputBox.setText("");
		}
	}

	function getGoldInput()
	{
		return this.mGoldInputBox;
	}

	function getSilverInput()
	{
		return this.mSilverInputBox;
	}

	function getCopperInput()
	{
		return this.mCopperInputBox;
	}

	function _checkInsideComponent( x, y, component )
	{
		local pos = component.getPosition();
		local size = component.getSize();
		return x >= pos.x && x <= pos.x + size.width && y >= pos.y && y <= pos.y + size.height;
	}

	function _padDigits( value, digits )
	{
		local final = "";

		if (value < 1000 && digits >= 4)
		{
			final += "0";
		}

		if (value < 100 && digits >= 3)
		{
			final += "0";
		}

		if (value < 10 && digits >= 2)
		{
			final += "0";
		}

		final += value;
		return final;
	}

}

class this.GUI.Credits extends this.GUI.Component
{
	mCreditsValue = null;
	mCreditsIcon = null;
	mCredits = 0;
	mCanEditCredits = false;
	mCreditsPopup = null;
	mCreditsInputBox = null;
	mBroadcaster = null;
	mCaption = null;
	constructor( ... )
	{
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.setChildrenInheritTooltip(true);

		if (vargc >= 1)
		{
			this.mCredits = vargv[0];
		}

		if (vargc >= 2)
		{
			this.mCaption = this.GUI.HTML(vargv[1] + "  ");
			this.add(this.mCaption);
		}
		else
		{
			this.mCaption = this.GUI.HTML();
			this.add(this.mCaption);
		}

		this.mBroadcaster = this.MessageBroadcaster();
		this.mCreditsIcon = this.GUI.Container();
		this.mCreditsIcon.setAppearance("Credit");
		this.mCreditsIcon.setPreferredSize(16, 16);
		this.mCreditsValue = this.GUI.Label();
		this.mCreditsValue.setFontColor("53B512");
		this.add(this.mCreditsIcon);
		this.add(this.mCreditsValue);
		this.updateCreditDisplay();
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function setFontColor( color )
	{
		this.mCreditsValue.setFontColor(color);
	}

	function setFont( fontObj )
	{
		this.mCreditsValue.setFont(fontObj);
	}

	function getCreditsFromInputBox( box )
	{
		local amt = 0;

		if (box)
		{
			amt = box.getText();

			if ("" == amt)
			{
				amt = -1;
			}
			else
			{
				amt = amt.tointeger();
			}
		}
		else
		{
			return -1;
		}

		return amt;
	}

	function getCurrentValue()
	{
		return this.mCredits;
	}

	function onAccepted( evt )
	{
		local creditValue = this.getCreditsFromInputBox(this.mCreditsInputBox);

		if (creditValue >= 0)
		{
			this.mCredits = creditValue;
		}

		this.updateCreditDisplay();
		this.mBroadcaster.broadcastMessage("onCreditsUpdated", this.mCredits);
		this._closeCreditsPopup();
	}

	function onMouseReleased( evt )
	{
		if (this.mCanEditCredits)
		{
			this._createCreditInputPopup();
			local screenPos = this.mCreditsIcon.getScreenPosition();
			local inputSize = this.mCreditsValue.getSize();
			this.mCreditsPopup.setPosition(screenPos.x - inputSize.width / 2, screenPos.y - inputSize.height - 5);
		}
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function setAllowCreditsEdit( value )
	{
		this.mCanEditCredits = value;
	}

	function setAlignment( align )
	{
		this.getLayoutManager().setPackAlignment(align);
	}

	function setCaption( caption )
	{
		this.mCaption.setText(caption + "  ");
	}

	function setCurrentValue( value )
	{
		if (null != value)
		{
			this.mCredits = value;
			this.updateCreditDisplay();
		}
	}

	function updateCreditDisplay()
	{
		this.mCreditsValue.setText(this.mCredits.tostring());
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _closeCreditsPopup()
	{
		if (!this.mCreditsPopup)
		{
			return;
		}

		if (this.mCreditsInputBox)
		{
			this.remove(this.mCreditsInputBox);
			this.mCreditsInputBox = null;
		}

		this.mCreditsPopup.setVisible(false);
		this.mCreditsPopup.setOverlay(null);
		this.mCreditsPopup = null;
	}

	function _createCreditInputPopup()
	{
		this.mCreditsPopup = this.GUI.Panel(this.GUI.GridLayout(2, 1));
		this.mCreditsPopup.getLayoutManager().setGaps(2, 5);
		this.mCreditsPopup.setSize(120, 65);
		local creditsInput = this.GUI.Component(this.GUI.BoxLayout(4));
		creditsInput.getLayoutManager().setExpand(false);
		local creditIcon = this.GUI.Container();
		creditIcon.setAppearance("Credit");
		creditIcon.setPreferredSize(16, 16);
		this.mCreditsInputBox = this.GUI.InputArea();
		this.mCreditsInputBox.setSize(60, 14);
		this.mCreditsInputBox.setInsets(1);
		this.mCreditsInputBox.setFont(this.GUI.Font("Maiandra", 20));
		this.mCreditsInputBox.setAllowOnlyNumbers(true);
		this.mCreditsInputBox.setMaxCharacters(7);
		this.mCreditsInputBox.setCenterText(true);
		creditsInput.add(this.GUI.Spacer(10, 0));
		creditsInput.add(creditIcon);
		creditsInput.add(this.GUI.Spacer(5, 0));
		creditsInput.add(this.mCreditsInputBox);
		local buttons = this.GUI.Component(this.GUI.GridLayout(1, 2));
		local acceptButton = this.GUI.Button("Accept");
		acceptButton.setReleaseMessage("onAccepted");
		acceptButton.addActionListener(this);
		local cancelButton = this.GUI.Button("Cancel");
		cancelButton.setReleaseMessage("onCanceled");
		cancelButton.addActionListener(this);
		buttons.add(acceptButton);
		buttons.add(cancelButton);
		this.mCreditsPopup.add(creditsInput);
		this.mCreditsPopup.add(buttons);
		this.mCreditsPopup.setOverlay(this.GUI.CONFIRMATION_OVERLAY);
		this.mCreditsPopup.setVisible(true);
	}

	function onCanceled( evt )
	{
		this._closeCreditsPopup();
	}

}

class this.GUI.Reagents extends this.GUI.Component
{
	mReagentComp = null;
	mNoReagentLabel = null;
	mReagentList = [
		20522,
		20523,
		20524,
		20525,
		20526,
		20527
	];
	static FONT_COLOR = "98CEE5";
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(0.0);
		this.mReagentComp = {};
		this.setChildrenInheritTooltip(true);
		this._createReagentComp();
		this.mNoReagentLabel = this.GUI.Label("You currently have no reagents.");
		this.mNoReagentLabel.setFontColor(this.FONT_COLOR);
		this.add(this.mNoReagentLabel);
		this._checkNoReagents();
		::_ItemDataManager.addListener(this);
		::_ItemManager.addListener(this);
	}

	function _createReagentComp()
	{
		foreach( itemDefId in this.mReagentList )
		{
			local numReagents = ::_ItemDataManager.getNumItems(itemDefId);
			local reagentName = ::_ItemDataManager.getItemDef(itemDefId).getDisplayName();
			local reagentContainer = this.GUI.Container(this.GUI.BoxLayout());
			reagentContainer.getLayoutManager().setAlignment(0.0);

			if (numReagents == 0)
			{
				reagentContainer.setVisible(false);
			}

			reagentContainer.setData(itemDefId);
			local countLabel = this.GUI.Label(numReagents.tostring());
			countLabel.setFontColor(this.FONT_COLOR);
			countLabel.setData("count");
			reagentContainer.add(countLabel);
			local reagentLabel = this.GUI.Label(reagentName);
			reagentLabel.setFontColor(this.FONT_COLOR);
			reagentLabel.setData("reagent");
			reagentContainer.add(reagentLabel);
			this.mReagentComp[itemDefId] <- reagentContainer;
			this.add(reagentContainer);
		}
	}

	function _checkNoReagents()
	{
		foreach( comp in this.mReagentComp )
		{
			if (comp.isVisible())
			{
				this.mNoReagentLabel.setVisible(false);
				return;
			}
		}

		this.mNoReagentLabel.setVisible(true);
	}

	function updateCount( itemDefId )
	{
		local reagentContainer = this.mReagentComp[itemDefId];

		foreach( comp in reagentContainer.components )
		{
			if ("count" == comp.getData())
			{
				local numReagents = ::_ItemDataManager.getNumItems(itemDefId);
				comp.setText(numReagents.tostring());

				if (numReagents == 0)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}

		return false;
	}

	function updateReagentName( itemDefId )
	{
		local reagentContainer = this.mReagentComp[itemDefId];

		foreach( comp in reagentContainer.components )
		{
			if ("reagent" == comp.getData())
			{
				local reagentName = ::_ItemDataManager.getItemDef(itemDefId).getDisplayName();
				comp.setText(reagentName);
				break;
			}
		}
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (!(itemDefId in this.mReagentList))
		{
			return;
		}

		local shouldShow = this.updateCount(itemDefId);
		this.updateReagentName(itemDefId);

		if (shouldShow)
		{
			this.mReagentComp[itemDefId].setVisible(true);
		}
		else
		{
			this.mReagentComp[itemDefId].setVisible(false);
		}

		this._checkNoReagents();
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (containerName == "inv" && ::_avatar && ::_avatar.getID() == creatureId)
		{
			foreach( itemDefId in this.mReagentList )
			{
				local shouldShow = this.updateCount(itemDefId);
				this.updateReagentName(itemDefId);

				if (shouldShow)
				{
					this.mReagentComp[itemDefId].setVisible(true);
				}
				else
				{
					this.mReagentComp[itemDefId].setVisible(false);
				}
			}

			this._checkNoReagents();
		}
	}

	function onStacksUpdated( sender, itemAction, mNumUses )
	{
		local itemData = itemAction.mItemData;
		local itemDefId = itemData.mItemDefId;

		if (!(itemDefId in this.mReagentList))
		{
			return;
		}

		local shouldShow = this.updateCount(itemDefId);
		this.updateReagentName(itemDefId);

		if (shouldShow)
		{
			this.mReagentComp[itemDefId].setVisible(true);
		}
		else
		{
			this.mReagentComp[itemDefId].setVisible(false);
		}

		this._checkNoReagents();
	}

}

