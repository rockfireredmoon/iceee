this.require("UI/AbilityScreenDef");
this.require("UI/AbilityUtil");
this.require("UI/AbilityComponents");
this.require("AbilityManager");
this.require("UI/Screens");
class this._AbilityScreenAction extends this.Action
{
	mAbilityId = 0;
	mName = null;
	mIcon = null;
	mPipContainer = null;
	mInfoFrame = null;
	constructor( abilityId, name, icon, pipContainer )
	{
		::Action.constructor(name, icon);
		this.mName = name;
		this.mIcon = icon;
		this.mAbilityId = abilityId;
		this.mPipContainer = pipContainer;
		this.mPipContainer.addListener(this);
	}

	function getAbilityId()
	{
		return this.mAbilityId;
	}

	function getTooltip( mods )
	{
		local tooltipComp = ::AbilityUtil.buildToolTipComponent(this.mAbilityId, mods);
		return tooltipComp;
	}

	function getQuickbarString()
	{
		return "ABILITY" + "id:" + this.mAbilityId + "name:" + this.mName + "icon:" + this.mIcon;
	}

	function onUpdateAbilityId( abilityId )
	{
		if (abilityId == 0)
		{
			return;
		}

		if (this.mPipContainer)
		{
			this.mAbilityId = abilityId;
			local ability = ::_AbilityManager.getAbilityById(abilityId);
			local combinedImage = ability.getForegroundImage() + "|" + ability.getBackgroundImage();
			this.setImage(combinedImage);
			this.setName(ability.getName());
			this.mName = ability.getName();
			this.mIcon = combinedImage;
			local as = this.Screens.get("AbilityFrame", false);

			if (!as)
			{
				return;
			}

			as._updateIconPurchasable(ability);
		}
	}

	function sendActivationRequest()
	{
		local as = this.Screens.get("AbilityFrame", false);

		if (!as || as.mMessageBoxActive || !this.mPipContainer || !(this.mPipContainer instanceof this.GUI.AbilityPipContainer))
		{
			return;
		}

		local id = this.mAbilityId;
		local abilityIds = [];

		if (this.mPipContainer.getNextDisabledAbilityId() != 0)
		{
			id = this.mPipContainer.getNextDisabledAbilityId();
		}

		abilityIds = this.mPipContainer.getAbilityIds();
		as._setCurrentAbilitySelection(id, abilityIds);
	}

	function showExtraDataScreen()
	{
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!this.mInfoFrame)
		{
			this.mInfoFrame = this.GUI.Frame("AbilityScreen Action: " + this.mName);
		}

		this.mInfoFrame.setVisible(true);
		local component = this.GUI.Component(this.GUI.BoxLayoutV());
		component.setInsets(5, 5, 5, 5);
		component.getLayoutManager().setAlignment(0);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = this.Util.addNewTextLine(textString, "Ability Id", this.getAbilityId());
		textString = this.Util.addNewTextLine(textString, "Icon", this.mIcon);
		textString = this.Util.addNewTextLine(textString, "Foreground Image", this.mForegroundImage);
		textString = this.Util.addNewTextLine(textString, "Background Image", this.mBackgroundImage);
		height = heightSize * 4;
		local ability = ::_AbilityManager.getAbilityById(this.mAbilityId);

		if (ability)
		{
			local data = ::Util.addAbilityDataInfo(textString, ability, height, heightSize);
			textString = data.text;
			height = data.height;
		}

		local htmlComp = this.GUI.HTML("");
		htmlComp.setInsets(0, 5, 0, 5);
		htmlComp.setWrapText(true, htmlComp.getFont(), WIDTH - 50);
		htmlComp.setText(textString);

		if (height > MAX_HEIGHT)
		{
			this.mInfoFrame.setSize(WIDTH, MAX_HEIGHT);
			this.mInfoFrame.setPreferredSize(WIDTH, MAX_HEIGHT);
			local scrollArea = ::GUI.ScrollPanel();
			scrollArea.attach(htmlComp);
			this.mInfoFrame.setContentPane(scrollArea);
		}
		else
		{
			this.mInfoFrame.setSize(WIDTH, height + 25);
			this.mInfoFrame.setPreferredSize(WIDTH, height + 25);
			this.mInfoFrame.setContentPane(htmlComp);
		}
	}

}

class this.Screens.AbilityFrame extends this.GUI.Frame
{
	static mClassName = "Screens.AbilityFrame";
	mAbilityContainer = null;
	ABILITY_INFO_WIDTH = 230;
	MAX_PLAYER_ABILITY = 5000;
	mAbilityId = 0;
	mRightSideComponent = null;
	mAbilityName = null;
	mAbilityDescription = null;
	mDynamicComponent = null;
	mAbilityProgressBar = null;
	mPreReqContainer = null;
	mPurchaseComponent = null;
	mRespecializeComp = null;
	mCurrentPage = 0;
	mPageRadioGroup = null;
	mRadioGroupButton = null;
	mRadioGroupButtonPreviousState = null;
	mLevelLabelBracket = null;
	mIconLabelsLayout = null;
	mAbilityContainerLayout = null;
	mPurchaseButtonPreviousState = null;
	mRespecButtonPreviousState = null;
	mButtonContainers = null;
	mTierComps = null;
	mAbilityImprovement = null;
	static mIconColumns = 6;
	static mIconRows = 9;
	mIconPageSize = 0;
	static MAX_PAGES = 12;
	static ICON_SPACING_X = 30;
	static ICON_SPACING_Y = 21;
	static ICON_SLOT_SIZE = 32;
	static BORDER_WIDTH = 26;
	static BORDER_HEIGHT = 16 * 2;
	static INACTIVE_TAB = "Ability/Tab/InActive";
	static ACTIVE_TAB = "Ability/Tab/Active";
	mIconHighlightLook = null;
	mMessageBoxActive = false;
	constructor()
	{
		this.GUI.Frame.constructor("Ability");
		this.mIconPageSize = this.mIconColumns * this.mIconRows;
		this.mTierComps = {};
		this.mButtonContainers = {};
		this.mRadioGroupButton = [];
		this.mRadioGroupButtonPreviousState = [];
		this.mIconHighlightLook = {};
		local labelWidth = 40;
		local tierHeight = 28;
		this.mAbilityContainerLayout = this.GUI.Container(this.GUI.GridLayout(2, 1));
		this.mAbilityContainerLayout.setAppearance("SilverTopFrame");
		this.mAbilityContainerLayout.setInsets(0, 0, 0, 0);
		local containerHeight = this.mIconRows * this.ICON_SLOT_SIZE + (this.mIconRows - 1) * this.ICON_SPACING_Y;
		this.mAbilityContainerLayout.getLayoutManager().setRows(tierHeight, containerHeight);
		local containerWidth = this.mIconColumns * this.ICON_SLOT_SIZE + (this.mIconColumns - 1) * this.ICON_SPACING_X;
		this.mAbilityContainerLayout.getLayoutManager().setColumns(containerWidth);
		local tierSpacing = containerWidth / this.mIconColumns - labelWidth - 3;
		this.mIconLabelsLayout = this.GUI.Container(this.GUI.BoxLayout());
		this.mIconLabelsLayout.getLayoutManager().setGap(tierSpacing);
		this.mIconLabelsLayout.setInsets(0, this.BORDER_WIDTH, 0, this.BORDER_WIDTH - 10);
		this.mIconLabelsLayout.setSize(containerWidth, tierHeight);
		this.mIconLabelsLayout.setPreferredSize(containerWidth, tierHeight);

		for( local i = 1; i <= this.AbilityTierType.MAX; i++ )
		{
			local tierContainer = this.GUI.Component(null);
			tierContainer.setSize(labelWidth, tierHeight);
			tierContainer.setPreferredSize(labelWidth, tierHeight);
			local tierLabel = this.GUI.Label(this.AbilityTierType[i].labelText);
			tierLabel.setFontColor(this.Colors.white);
			tierContainer.add(tierLabel);
			this.mTierComps[i] <- tierLabel;
			this.mIconLabelsLayout.add(tierContainer);
		}

		this.mAbilityContainerLayout.add(this.mIconLabelsLayout);
		this.mAbilityContainer = this.GUI.ActionContainer("ability_screen_container", this.mIconRows, this.mIconColumns, this.ICON_SPACING_X, this.ICON_SPACING_Y, this, false);
		this.mAbilityContainer.setIconContainerInsets(10, 15, 10, 20);
		this.mAbilityContainer.setMaxPages(this.MAX_PAGES);
		this.mAbilityContainer.setPagingInfoEnabled(false);
		this.mAbilityContainer.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.CLONE, this));

		for( local i = 0; i < this.MAX_PAGES * this.mIconPageSize; ++i )
		{
			this.mAbilityContainer.setSlotUseMode(i, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		}

		this.mAbilityContainerLayout.add(this.mAbilityContainer);
		local topLeftContainer = this.GUI.Container(this.GUI.BorderLayout());
		topLeftContainer.setInsets(2, 0, 6, 6);
		topLeftContainer.add(this.mAbilityContainerLayout, this.GUI.BorderLayout.CENTER);
		this.mRightSideComponent = this._buildDescriptionComponent();
		local lowerHalfContainer = this.GUI.Container(this.GUI.BoxLayout());
		lowerHalfContainer.getLayoutManager().setGap(5);
		lowerHalfContainer.setInsets(-2, 5, 0, 5);
		lowerHalfContainer.setAppearance("SilverBottomHalfBorder");
		topLeftContainer.add(lowerHalfContainer, this.GUI.BorderLayout.SOUTH);
		this.mPageRadioGroup = this.GUI.RadioGroup();
		this.mPageRadioGroup.addListener(this);

		foreach( key, abilityTree in this.AbilityTreeType )
		{
			if (("name" in abilityTree) && "indexPage" in abilityTree)
			{
				local container = this.GUI.Container(null);
				container.setSize(38, 36);
				container.setPreferredSize(38, 36);
				container.setAppearance(this.INACTIVE_TAB);
				lowerHalfContainer.add(container);
				local button = this.GUI.ImageButton();
				button.setRadioGroup(this.mPageRadioGroup);
				button.setData(key);
				button.setSize(24, 24);
				button.setPreferredSize(24, 24);
				button.setAppearance("Icon/AB_" + abilityTree.name);
				button.setGlowImageName("Icon/AB_" + abilityTree.name);
				button.setTooltip(abilityTree.name);
				button.setGlowEnabled(false);
				local diff = (38 - 24) / 2;
				button.setPosition(diff, diff);
				container.add(button);
				this.mButtonContainers[key] <- container;
				this.mRadioGroupButton.append(button);
				this.mRadioGroupButtonPreviousState.append(false);
			}
		}

		local baseContainer = this.GUI.Container(this.GUI.BoxLayout());
		baseContainer.add(topLeftContainer);
		baseContainer.add(this.mRightSideComponent);
		baseContainer.getLayoutManager().setAlignment(0.0);
		this.setContentPane(baseContainer);
		local preferredSize = this.getPreferredSize();
		this.setSize(preferredSize);
		this.centerOnScreen();
		::_Connection.addListener(this);
		::_AbilityHelper.addListener(this);
		::_AbilityManager.addAbilityListener(this);
		::_AbilityManager.getAbilityOwnageList();
		this.onAbilityPointsUpdate();
		this._togoProfessionPage();
		this.setCached(::Pref.get("video.UICache"));
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value)
		{
			::_tutorialManager.onScreenOpened("AbilityScreen");
		}
	}

	function _buildDescriptionComponent()
	{
		local SPACER_WIDTH = 150;
		local infoAbilityContainer = this.GUI.InnerPanel(this.GUI.GridLayout(11, 1));
		infoAbilityContainer.getLayoutManager().setRows(20, 35, 20, 1, 65, 1, 115, 1, 140, 1, 30);
		infoAbilityContainer.getLayoutManager().setColumns(this.ABILITY_INFO_WIDTH);
		infoAbilityContainer.setInsets(2, 5, 5, 5);
		this.mAbilityImprovement = this.GUI.Label("Ability Improvement At:");
		this.mAbilityImprovement.setFont(this.GUI.Font("Maiandra", 20));
		this.mAbilityImprovement.setTextAlignment(0.5, 0.5);
		this.mAbilityImprovement.setFontColor(this.Colors.white);
		infoAbilityContainer.add(this.mAbilityImprovement);
		this.mAbilityProgressBar = this.GUI.AbilityProgressBar();
		this.mAbilityProgressBar.addActionListener(this);
		local tempComp = this.GUI.Container(this.GUI.BoxLayoutV());
		tempComp.getLayoutManager().setAlignment(0.5);
		tempComp.add(this.mAbilityProgressBar);
		infoAbilityContainer.add(tempComp);
		this.mAbilityName = this.GUI.Label();
		this.mAbilityName.setTextAlignment(0.5, 1.0);
		this.mAbilityName.setFont(this.GUI.Font("Maiandra", 32));
		this.mAbilityName.setFontColor(this.Colors.white);
		infoAbilityContainer.add(this.mAbilityName);
		local sep = this.GUI.Spacer(SPACER_WIDTH, 1);
		sep.setAppearance("ColumnList/HeadingDivider");
		infoAbilityContainer.add(sep);
		this.mPreReqContainer = this.GUI.AbilityPrereqContainer();
		infoAbilityContainer.add(this.mPreReqContainer);
		local secondSep = this.GUI.Spacer(SPACER_WIDTH, 1);
		secondSep.setAppearance("ColumnList/HeadingDivider");
		infoAbilityContainer.add(secondSep);
		this.mAbilityDescription = this.GUI.HTML();
		this.mAbilityDescription.setMaximumSize(500, null);
		this.mAbilityDescription.setFontColor(this.Colors.white);
		infoAbilityContainer.add(this.mAbilityDescription);
		local thirdSep = this.GUI.Spacer(SPACER_WIDTH, 1);
		thirdSep.setAppearance("ColumnList/HeadingDivider");
		infoAbilityContainer.add(thirdSep);
		this.mDynamicComponent = ::AbilityUtil.generateDynamicComponent(null, null);
		infoAbilityContainer.add(this.mDynamicComponent);
		local fourthSep = this.GUI.Spacer(SPACER_WIDTH, 1);
		fourthSep.setAppearance("ColumnList/HeadingDivider");
		infoAbilityContainer.add(fourthSep);
		this.mPurchaseComponent = this.GUI.AbilityPurchaseComp();
		this.mPurchaseComponent.addListener(this);
		infoAbilityContainer.add(this.mPurchaseComponent);
		local rightContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		rightContainer.setInsets(0, 5, 0, 0);
		rightContainer.add(infoAbilityContainer);
		this.mRespecializeComp = this.GUI.AbilityRespecializeComp();
		this.mRespecializeComp.addListener(this);
		rightContainer.add(this.mRespecializeComp);
		local buyAbilityContainer = this.GUI.Container(this.GUI.BoxLayout());
		rightContainer.add(buyAbilityContainer);
		local abilityPurchaseButton = this.GUI.AbilityButton("Purchase Ability Points", this, "_onAbilityPurchasePressed");
		abilityPurchaseButton.addActionListener(this);
		abilityPurchaseButton.setReleaseMessage("_onAbilityPurchasePressed");
		abilityPurchaseButton.setFont(this.GUI.Font("Maiandra", 24));
		abilityPurchaseButton.setFontColor(this.Colors.white);
		buyAbilityContainer.add(abilityPurchaseButton);
		return rightContainer;
	}

	function _onAbilityPurchasePressed( button )
	{
		this.Screens.show("AbilityPointBuyScreen");
	}

	function _togoProfessionPage()
	{
		local profession = ::_avatar.getStat(this.Stat.PROFESSION, true);

		if (profession >= 1 && profession <= 4)
		{
			this.mPageRadioGroup.setSelected(this.mRadioGroupButton[profession - 1]);
			this.itemSelected(this.mRadioGroupButton[profession - 1]);
		}
		else
		{
			this.mPageRadioGroup.setSelected(this.mRadioGroupButton[4]);
			this.itemSelected(this.mRadioGroupButton[4]);
		}
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
		this.onLevelUpdate(::_avatar.getStat(this.Stat.LEVEL, true));
	}

	function _removeNotify()
	{
		this.GUI.Frame._removeNotify();
	}

	function onUpdateBuyAbility( ability )
	{
		local page_index = ::AbilityUtil.getPageIndexFromAbility(ability);
		local coord = ability.getSlotCoordinates();
		local id = ability.getId();

		if (id < this.MAX_PLAYER_ABILITY && page_index != -1 && coord[0] >= 0 && coord[0] < this.mIconColumns && coord[1] >= 0 && coord[1] < this.mIconRows && !(ability.getTier() == 7 || ability.getTier() == 8))
		{
			local index_in_page = coord[0] + coord[1] * this.mIconColumns;
			local abs_index = index_in_page + page_index * this.mIconPageSize;
			local actionButton = this.mAbilityContainer.getActionButtonFromIndex(abs_index);

			if (actionButton != null)
			{
				local pipCont = actionButton.getExtraComponent();

				if (pipCont && typeof pipCont == "instance" && (pipCont instanceof this.GUI.AbilityPipContainer))
				{
					if (pipCont.enablePip())
					{
						local id = ability.getId();

						if (pipCont.getNextDisabledAbilityId() != 0)
						{
							id = pipCont.getNextDisabledAbilityId();
						}

						local abilityIds = pipCont.getAbilityIds();
						this._setCurrentAbilitySelection(id, abilityIds);
					}
				}
			}
		}
	}

	function onRespecializeAbility()
	{
		this._setCurrentAbilitySelection(0);
		this._clearAllPips();
		this.onAbilityPointsUpdate();
		this._updateDraggable();
	}

	function _clearAllPips()
	{
		local actionButtons = this.mAbilityContainer.getAllActionButtons();

		foreach( button in actionButtons )
		{
			local pipCont = button.getExtraComponent();

			if (pipCont && typeof pipCont == "instance" && (pipCont instanceof this.GUI.AbilityPipContainer))
			{
				pipCont.clearPips();
			}
		}
	}

	function onAbilityPointsUpdate()
	{
		this.mRespecializeComp.updateTotalAbilityPoints();
		this.mRespecializeComp.updateCurrentAbilityPoints();
	}

	function onAbilitiesReceived( abilities )
	{
		this.log.debug("onAbilitiesReceived() ABSCREEN");
		this.mAbilityContainer.removeAllActions();

		foreach( id, ab in abilities )
		{
			local page_index = ::AbilityUtil.getPageIndexFromAbility(ab);
			local coord = ab.getSlotCoordinates();

			if (id < this.MAX_PLAYER_ABILITY && page_index != -1 && coord[0] >= 0 && coord[0] < this.mIconColumns && coord[1] >= 0 && coord[1] < this.mIconRows && !(ab.getTier() == 7 || ab.getTier() == 8))
			{
				local index_in_page = coord[0] + coord[1] * this.mIconColumns;
				local abs_index = index_in_page + page_index * this.mIconPageSize;
				local combinedImage = ab.getForegroundImage() + "|" + ab.getBackgroundImage();
				local actionButton = this.mAbilityContainer.getActionButtonFromIndex(abs_index);

				if (actionButton != null)
				{
					local pipCont = actionButton.getExtraComponent();

					if (pipCont && typeof pipCont == "instance" && (pipCont instanceof this.GUI.AbilityPipContainer))
					{
						pipCont.addPip(id);

						if (ab.getOwnage() > this.AbilityOwnageType.ABILITY_NOT_OWNED)
						{
							pipCont.enablePip();
						}
					}
				}
				else
				{
					local pipContainer = this.GUI.AbilityPipContainer();
					pipContainer.addPip(id);
					local actionButton = this.mAbilityContainer.addAction(this._AbilityScreenAction(id, ab.getName(), combinedImage, pipContainer), false, abs_index);
					actionButton.addExtraComponent(pipContainer);

					if (ab.getOwnage() > this.AbilityOwnageType.ABILITY_NOT_OWNED)
					{
						pipContainer.enablePip();
					}
				}
			}
		}

		this._updateDraggable();
		this._togoProfessionPage();
		this.mAbilityContainer.updateContainer();
	}

	function _updateSlotVisibility()
	{
		for( local i = 0; i < this.mIconPageSize; ++i )
		{
			this.mAbilityContainer.setSlotVisible(i, this.mAbilityContainer.slotHasButton(i));
			local abs_index = i + this.mCurrentPage * this.mIconPageSize;

			if (abs_index in this.mIconHighlightLook)
			{
				this._updateSlotDraggable(this.mIconHighlightLook[abs_index], i);
			}
		}
	}

	function _updateDraggable()
	{
		local abilities = ::_AbilityManager.getAbilities();

		foreach( id, ability in abilities )
		{
			this._updateIconPurchasable(ability);
		}
	}

	function _updateIconPurchasable( ability )
	{
		local page_index = ::AbilityUtil.getPageIndexFromAbility(ability);
		local coord = ability.getSlotCoordinates();
		local groupId = ability.getGroupId();
		local id = ability.getId();

		if (id < this.MAX_PLAYER_ABILITY && page_index != -1 && coord[0] >= 0 && coord[0] < this.mIconColumns && coord[1] >= 0 && coord[1] < this.mIconRows && !(ability.getTier() == 7 || ability.getTier() == 8))
		{
			local index_in_page = coord[0] + coord[1] * this.mIconColumns;
			local abs_index = index_in_page + page_index * this.mIconPageSize;
			local actionButton = this.mAbilityContainer.getActionButtonFromIndex(abs_index);

			if (actionButton != null)
			{
				local pipCont = actionButton.getExtraComponent();

				if (pipCont && typeof pipCont == "instance" && (pipCont instanceof this.GUI.AbilityPipContainer))
				{
					local abilityId = ability.getId();
					local look = pipCont.getHighlightType();
					this.mIconHighlightLook[abs_index] <- look;

					if (this.mCurrentPage == page_index)
					{
						this._updateSlotDraggable(look, index_in_page);
					}
				}
			}
		}
	}

	function _updateSlotDraggable( look, index_in_page )
	{
		if ((look & this.AbilityScreenFlags.OWN) > 0 && (look & this.AbilityScreenFlags.PASSIVE) == 0)
		{
			this.mAbilityContainer.setSlotDraggable(true, index_in_page);
		}
		else
		{
			this.mAbilityContainer.setSlotDraggable(false, index_in_page);
		}
	}

	function onLevelUpdate( level )
	{
		this._updateDraggable();

		foreach( key, tierData in this.AbilityTierType )
		{
			if (("minLevel" in tierData) && ("maxLevel" in tierData) && level >= tierData.minLevel)
			{
				this.mTierComps[key].setFontColor(this.Colors.white);
			}
			else if (("minLevel" in tierData) && "maxLevel" in tierData)
			{
				this.mTierComps[key].setFontColor(this.Colors.grey40);
			}
		}
	}

	function itemSelected( button )
	{
		local type = button.getData();
		local parentComp = button.mParentComponent;
		this.resetTabsToInactive();

		if (parentComp)
		{
			parentComp.setAppearance(this.ACTIVE_TAB);
		}

		this.mAbilityContainer.gotoPage(this.AbilityTreeType[type].indexPage);
		this.mCurrentPage = this.AbilityTreeType[type].indexPage - 1;
		this._updateSlotVisibility();
		this._setCurrentAbilitySelection(0);
	}

	function resetTabsToInactive()
	{
		foreach( container in this.mButtonContainers )
		{
			container.setAppearance(this.INACTIVE_TAB);
		}
	}

	function _setRightSideLabelsFontColor( color )
	{
		this.mPurchaseComponent.setLabelsFontColor(color);
		this.mPreReqContainer.setLabelsFontColor(color);
		this.mAbilityName.setFontColor(this.Colors[color]);
		this.mAbilityDescription.setFontColor(this.Colors[color]);
		this.mAbilityImprovement.setFontColor(this.Colors[color]);
	}

	function _setCurrentAbilitySelection( id, ... )
	{
		this.mAbilityId = id;
		this.mPurchaseComponent.setAbilityId(this.mAbilityId);
		local abilityIds = [];
		local shouldUpdateAbilityBar = true;

		if (vargc > 0)
		{
			abilityIds = vargv[0];
		}

		if (vargc > 1)
		{
			shouldUpdateAbilityBar = vargv[1];
		}

		if (id == 0)
		{
			this.mAbilityName.setText("");
			this.mAbilityDescription.setText("");
			this.mAbilityProgressBar.resetImages();
			this.mPreReqContainer.setMustOwn("");
			this.mPreReqContainer.setLevel("");
			this.mPreReqContainer.updateProfessionDisplay("");
			this.mPurchaseComponent.purchaseButtonSetEnabled(false);
			this.mPurchaseComponent.setAbilityCost(0);
			this.mPurchaseComponent.setAbilityPointCost("Ability Point Cost: ");
			this._setRightSideLabelsFontColor("Medium Dark Grey");

			if (this.mDynamicComponent)
			{
				this.mDynamicComponent.setVisible(false);
			}
		}
		else
		{
			local ab = ::_AbilityManager.getAbilityById(id);

			if (!ab.getIsValid())
			{
				return;
			}

			this._setRightSideLabelsFontColor("white");

			if (this.mAbilityProgressBar && shouldUpdateAbilityBar)
			{
				this.mAbilityProgressBar.updateAbilityBarImages(id, abilityIds);
			}

			local name = ab.getName();
			local desc = ab.getDescription();
			local abilityIdsRequired = ab.getPurchaseAbilitiesRequired();
			local abilitiesRequiredText = [];
			local abilitiesRequired = [];

			foreach( reqAb in abilityIdsRequired )
			{
				if (reqAb != 0)
				{
					local ability = ::_AbilityManager.getAbilityById(reqAb);
					abilitiesRequired.append(ability);

					if (ability != null)
					{
						abilitiesRequiredText.append("Tier " + ability.getTier() + " " + ability.getName());
					}
				}
			}

			local classRequired = ab.getPurchaseClassRequired();
			local levelRequired = ab.getPurchaseLevelRequired();
			local pointsRequired = ab.getPurchasePointsRequired();
			local coord = ab.getSlotCoordinates();
			local goldCost = ab.getGoldCost();
			local page_index = ::AbilityUtil.getPageIndexFromAbility(ab);
			local abilitityIdsRequired = ab.getPurchaseAbilitiesRequired();
			local ownCurrentAbility = ab.getOwnage();
			local index_in_page = coord[0] + coord[1] * this.mIconColumns;
			local abs_index = index_in_page + page_index * this.mIconPageSize;
			local allRequirementsPurchased = false;

			if ((this.mIconHighlightLook[abs_index] & this.AbilityScreenFlags.CAN_PURCHASE) != 0)
			{
				allRequirementsPurchased = true;
			}

			if (allRequirementsPurchased)
			{
				foreach( reqAbId in abilitityIdsRequired )
				{
					local requiredAbility = ::_AbilityManager.getAbilityById(reqAbId);

					if (!(reqAbId != 0 && reqAbId && requiredAbility.getOwnage() <= 1) && !(ownCurrentAbility > 1))
					{
					}
					else
					{
						allRequirementsPurchased = false;
					}
				}
			}

			this.mPurchaseComponent.purchaseButtonSetEnabled(allRequirementsPurchased);
			this.mAbilityName.setText(name);
			this.mAbilityDescription.setText(desc);
			this.mPreReqContainer.updateProfessionDisplay(classRequired);
			this.mPurchaseComponent.setAbilityCost(goldCost);
			this.mPurchaseComponent.setAbilityPointCost("Ability Point Cost: " + pointsRequired);

			if (levelRequired)
			{
				this.mPreReqContainer.setLevel(levelRequired.tostring());
			}

			local requirementString = "";
			local requirements = "";
			local hasAllAbilities = true;

			if (abilitiesRequiredText.len() > 0)
			{
				local hasPreviousAbility = false;

				for( local i = 0; i < abilitiesRequired.len(); i++ )
				{
					if (abilitiesRequired[i] != 0 && abilitiesRequired[i] && abilitiesRequired[i].getOwnage() >= 1)
					{
						hasPreviousAbility = true;
					}
					else
					{
						hasPreviousAbility = false;
						hasAllAbilities = false;
					}

					if (requirements != "")
					{
						requirements += ", ";
					}

					if (hasPreviousAbility)
					{
						requirements += "<font color=\"" + this.Colors.mint + "\">" + abilitiesRequiredText[i] + "</font>";
					}
					else
					{
						requirements += "<font color=\"" + this.Colors.red + "\">" + abilitiesRequiredText[i] + "</font>";
					}
				}
			}

			if (requirements != "")
			{
				if (hasAllAbilities)
				{
					requirementString = "<font color=\"" + this.Colors.mint + "\">" + "Must Have: " + "</font>";
				}
				else
				{
					requirementString = "<font color=\"" + this.Colors.red + "\">" + "Must Have: " + "</font>";
				}

				requirementString += requirements;
			}

			this.mPreReqContainer.setMustOwn(requirementString);
			this.mDynamicComponent = ::AbilityUtil.generateDynamicComponent(::_AbilityManager.getAbilityById(this.mAbilityId), this.mDynamicComponent, true);
			this.mDynamicComponent.setVisible(true);
		}
	}

	function onDisableAllButtons()
	{
		this._allButtonsSaveState();
		this._allButtonsSetEnabled(false);
		this.mMessageBoxActive = true;
	}

	function onRestoreAllButtons()
	{
		this.mMessageBoxActive = false;
		this._allButtonsRestoreState();
	}

	function _allButtonsSaveState()
	{
		if (this.mRadioGroupButton.len() != this.mRadioGroupButtonPreviousState.len())
		{
			throw this.Exception("mRadioGroupButton and mRadioGroupButtonPreviousState must be the same length!");
		}

		foreach( i, radioButton in this.mRadioGroupButton )
		{
			this.mRadioGroupButtonPreviousState[i] = radioButton.isEnabled();
		}

		this.mPurchaseButtonPreviousState = this.mPurchaseComponent.isPurchaseButtonEnabled();
		this.mRespecButtonPreviousState = this.mRespecializeComp.isRespecButtonEnabled();
	}

	function _allButtonsRestoreState()
	{
		if (this.mRadioGroupButton.len() != this.mRadioGroupButtonPreviousState.len())
		{
			throw this.Exception("mRadioGroupButton and mRadioGroupButtonPreviousState must be the same length!");
		}

		foreach( i, radioButton in this.mRadioGroupButton )
		{
			this.mRadioGroupButton[i].setEnabled(this.mRadioGroupButtonPreviousState[i]);
		}

		this.mPurchaseComponent.purchaseButtonSetEnabled(this.mPurchaseButtonPreviousState);
		this.mRespecializeComp.respecButtonSetEnabled(this.mRespecButtonPreviousState);
	}

	function _allButtonsSetEnabled( enable )
	{
		foreach( button in this.mRadioGroupButton )
		{
			button.setEnabled(enable);
		}

		this.mPurchaseComponent.purchaseButtonSetEnabled(enable);
		this.mRespecializeComp.respecButtonSetEnabled(enable);
	}

}

