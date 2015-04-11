this.require("UI/AbilityUtil");
this.require("UI/AbilityScreenDef");
class this.GUI.AbilityPrereqContainer extends this.GUI.Container
{
	static mClassName = "AbilityPrereqContainer";
	static LABEL_HEIGHT = 15;
	static NUM_PROFESSIONS = 4;
	static ABILITY_INFO_WIDTH = 230;
	mClassDisplay = null;
	mAbilityLevelLabel = null;
	mAbilityMustOwnLabel = null;
	mPreReqTitle = null;
	mLevelLabel = null;
	mCoreLabel = null;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.GridLayout(3, 1));
		this.getLayoutManager().setRows(this.LABEL_HEIGHT, this.LABEL_HEIGHT, 40);
		this.getLayoutManager().setColumns(this.ABILITY_INFO_WIDTH);
		this.mClassDisplay = [];
		this.mPreReqTitle = this.GUI.Label("Prerequisites");
		this.mPreReqTitle.setTextAlignment(0.5, 0.5);
		this.mPreReqTitle.setFontColor(this.Colors.white);
		this.add(this.mPreReqTitle);
		local levelClassComponent = this.GUI.Component(this.GUI.BoxLayout());
		this.add(levelClassComponent);
		this.mLevelLabel = this.GUI.Label("Level: ");
		this.mLevelLabel.setFontColor(this.Colors.white);
		levelClassComponent.add(this.mLevelLabel);
		this.mAbilityLevelLabel = this.GUI.Label("");
		this.mAbilityLevelLabel.setFontColor(this.Colors.white);
		levelClassComponent.add(this.mAbilityLevelLabel);
		levelClassComponent.add(this.GUI.Spacer(50, 10));
		this.mCoreLabel = this.GUI.Label("Core Class: ");
		this.mCoreLabel.setFontColor(this.Colors.white);
		levelClassComponent.add(this.mCoreLabel);

		for( local i = 0; i < this.NUM_PROFESSIONS; i++ )
		{
			local classComp = this.GUI.Component();
			classComp.setPreferredSize(10, 12);
			this.mClassDisplay.append(classComp);
			levelClassComponent.add(classComp);
		}

		this.mAbilityMustOwnLabel = this.GUI.HTML("");
		this.mAbilityMustOwnLabel.setFontColor(this.Colors.white);
		this.add(this.mAbilityMustOwnLabel);
	}

	function setLevel( levelText )
	{
		this.mAbilityLevelLabel.setText(levelText);

		if (levelText == "")
		{
			return;
		}

		if (::_avatar)
		{
			local ownLevel = ::_avatar.getStat(this.Stat.LEVEL, true);

			if (ownLevel < levelText.tointeger())
			{
				this.mAbilityLevelLabel.setFontColor(this.Colors.red);
			}
			else
			{
				this.mAbilityLevelLabel.setFontColor(this.Colors.mint);
			}
		}
	}

	function setMustOwn( mustOwnText )
	{
		this.mAbilityMustOwnLabel.setText(mustOwnText);
	}

	function setLabelsFontColor( color )
	{
		this.mAbilityLevelLabel.setFontColor(this.Colors[color]);
		this.mAbilityMustOwnLabel.setFontColor(this.Colors[color]);
		this.mPreReqTitle.setFontColor(this.Colors[color]);
		this.mLevelLabel.setFontColor(this.Colors[color]);
		this.mCoreLabel.setFontColor(this.Colors[color]);
	}

	function updateProfessionDisplay( classRequired )
	{
		this.mClassDisplay[0].setAppearance(classRequired.find("K") != null ? "ClassButton/Green/K" : "ClassButton/Red/K");
		this.mClassDisplay[1].setAppearance(classRequired.find("R") != null ? "ClassButton/Green/R" : "ClassButton/Red/R");
		this.mClassDisplay[2].setAppearance(classRequired.find("M") != null ? "ClassButton/Green/M" : "ClassButton/Red/M");
		this.mClassDisplay[3].setAppearance(classRequired.find("D") != null ? "ClassButton/Green/D" : "ClassButton/Red/D");
	}

}

class this.GUI.AbilityPurchaseComp extends this.GUI.Component
{
	static mClassName = "AbilityPurchaseComp";
	static ABILITY_INFO_WIDTH = 230;
	static LABEL_HEIGHT = 15;
	mAbilityId = 0;
	mAbilityCost = null;
	mAbilityPointCostLabel = null;
	mPurchaseButton = null;
	mMessageBroadcaster = null;
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.mMessageBroadcaster = this.MessageBroadcaster();
		local labelsContainer = this.GUI.Container(this.GUI.GridLayout(2, 1));
		labelsContainer.getLayoutManager().setRows(this.LABEL_HEIGHT, this.LABEL_HEIGHT);
		labelsContainer.getLayoutManager().setColumns(this.ABILITY_INFO_WIDTH - 75);
		this.mAbilityCost = this.GUI.Currency();
		labelsContainer.add(this.mAbilityCost);
		this.mAbilityPointCostLabel = this.GUI.Label("Ability Point Cost:");
		this.mAbilityPointCostLabel.setFontColor(this.Colors.white);
		labelsContainer.add(this.mAbilityPointCostLabel);
		this.add(labelsContainer);
		this.mPurchaseButton = this.GUI.Button("Purchase", this, "_onPurchasePressed");
		this.mPurchaseButton.setEnabled(false);
		this.add(this.mPurchaseButton);
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function setAbilityId( abilityId )
	{
		this.mAbilityId = abilityId;
	}

	function setAbilityCost( cost )
	{
		this.mAbilityCost.setCurrentValue(cost);
	}

	function setAbilityPointCost( pointCostText )
	{
		this.mAbilityPointCostLabel.setText(pointCostText);
	}

	function purchaseButtonSetEnabled( enabled )
	{
		this.mPurchaseButton.setEnabled(enabled);
	}

	function isPurchaseButtonEnabled()
	{
		return this.mPurchaseButton.isEnabled();
	}

	function setLabelsFontColor( color )
	{
		this.mAbilityPointCostLabel.setFontColor(this.Colors[color]);
	}

	function _onPurchasePressed( button )
	{
		this.log.debug("_onPurchasePressed()");

		if (this.mAbilityId == 0)
		{
			return;
		}

		local callback = {
			id = this.mAbilityId,
			broadcaster = this.mMessageBroadcaster,
			function onActionSelected( mb, alt )
			{
				this.broadcaster.broadcastMessage("onRestoreAllButtons");

				if (alt == "Yes")
				{
					::_AbilityManager.abBuy(this.id);
				}
			}

		};
		this.mMessageBroadcaster.broadcastMessage("onDisableAllButtons");
		local ability = ::_AbilityManager.getAbilityById(this.mAbilityId);
		this.GUI.MessageBox.showYesNo("Confirm purchase of " + ability.getName() + ". Press Yes to proceed. Press No to cancel.", callback);
	}

}

class this.GUI.AbilityRespecializeComp extends this.GUI.Component
{
	static mClassName = "AbilityRespecializeComp";
	static ABILITY_INFO_WIDTH = 230;
	static LABEL_HEIGHT = 15;
	mAbilityPointsLabel = null;
	mTotalPointsSpentLabel = null;
	mRespecButton = null;
	mMessageBroadcaster = null;
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.BoxLayout());
		this.mMessageBroadcaster = this.MessageBroadcaster();
		local labelsContainer = this.GUI.Container(this.GUI.GridLayout(2, 1));
		local LABEL_HEIGHT = 15;
		labelsContainer.getLayoutManager().setRows(LABEL_HEIGHT, LABEL_HEIGHT);
		labelsContainer.getLayoutManager().setColumns(this.ABILITY_INFO_WIDTH - 75);
		this.mAbilityPointsLabel = this.GUI.Label("Ability Points:");
		this.mAbilityPointsLabel.setFontColor(this.Colors.white);
		labelsContainer.add(this.mAbilityPointsLabel);
		this.mTotalPointsSpentLabel = this.GUI.Label("Total Points Spent:");
		this.mTotalPointsSpentLabel.setFontColor(this.Colors.white);
		labelsContainer.add(this.mTotalPointsSpentLabel);
		this.add(labelsContainer);
		this.mRespecButton = this.GUI.Button("Respecialize", this, "_onRespecializePressed");
		this.add(this.mRespecButton);
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function updateTotalAbilityPoints()
	{
		this.mAbilityPointsLabel.setText("Ability Points: " + ::_avatar.getStat(this.Stat.CURRENT_ABILITY_POINTS, true));
	}

	function updateCurrentAbilityPoints()
	{
		local total = ::_avatar.getStat(this.Stat.TOTAL_ABILITY_POINTS, true);
		local current = ::_avatar.getStat(this.Stat.CURRENT_ABILITY_POINTS, true);

		if (current == null)
		{
			current = 0;
		}

		this.mTotalPointsSpentLabel.setText("Total Points Spent: " + (total - current).tostring());
	}

	function respecButtonSetEnabled( enabled )
	{
		this.mRespecButton.setEnabled(enabled);
	}

	function isRespecButtonEnabled()
	{
		return this.mRespecButton.isEnabled();
	}

	function _onRespecializePressed( button )
	{
		this.log.debug("_onRespecializePressed()");
		local callback = {
			broadcaster = this.mMessageBroadcaster,
			function onActionSelected( mb, alt )
			{
				this.broadcaster.broadcastMessage("onRestoreAllButtons");

				if (alt == "Yes")
				{
					::_AbilityManager.abRespec();
				}
			}

		};
		this.mMessageBroadcaster.broadcastMessage("onDisableAllButtons");
		this.GUI.MessageBox.showYesNo("Confirm respecialization. Press Yes to proceed. Press No to cancel.", callback);
	}

}

class this.GUI.Pip extends this.GUI.Container
{
	static mClassName = "Pip";
	static PIP_IMAGE_WIDTH = 10;
	static PIP_IMAGE_HEIGHT = 4;
	static DEFAULT_IMAGE = "Pip";
	mIsEnabled = false;
	mDisabled = false;
	constructor()
	{
		this.GUI.Container.constructor(null);
		this.setSize(this.PIP_IMAGE_WIDTH, this.PIP_IMAGE_HEIGHT);
		this.setPreferredSize(this.PIP_IMAGE_WIDTH, this.PIP_IMAGE_HEIGHT);
		this.setAppearance(this.DEFAULT_IMAGE);
		this.setTooltip("Power Level");
	}

	function setAppearance( appearance )
	{
		if (this.mIsEnabled)
		{
			this.GUI.Container.setAppearance(appearance + "/On");
		}
		else
		{
			this.GUI.Container.setAppearance(appearance);
		}
	}

	function setEnabled( enabled )
	{
		this.mIsEnabled = enabled;

		if (!this.mDisabled)
		{
			this.setAppearance(this.DEFAULT_IMAGE);
		}
	}

	function isEnabled()
	{
		return this.mIsEnabled;
	}

	function setDisabled( disabled )
	{
		this.mDisabled = disabled;
		this.mIsEnabled = !disabled;

		if (this.mDisabled)
		{
			this.setAppearance(this.DEFAULT_IMAGE + "/Disabled");
		}
		else
		{
			this.setAppearance(this.DEFAULT_IMAGE);
		}
	}

}

class this.GUI.AbilityPipContainer extends this.GUI.Container
{
	static mClassName = "AbilityPipComp";
	static ACTION_HOLDER_ICON_SIZE = 44;
	static DEFAULT_ICON_SIZE = 32;
	static SILVER_HOLDER = "Ability/Holder/Silver";
	static GOLD_HOLDER = "Ability/Holder/Gold";
	mPipComps = null;
	mAbilityIds = null;
	mLastEnabledPipCount = -1;
	mMessageBroadcaster = null;
	mLockImage = null;
	constructor()
	{
		local shiftAmount = (this.ACTION_HOLDER_ICON_SIZE - this.DEFAULT_ICON_SIZE) / 2;
		this.GUI.Container.constructor(null);
		this.setPreferredSize(this.ACTION_HOLDER_ICON_SIZE, this.ACTION_HOLDER_ICON_SIZE);
		this.setSize(this.ACTION_HOLDER_ICON_SIZE, this.ACTION_HOLDER_ICON_SIZE);
		this.setPosition(-shiftAmount, -shiftAmount);
		this.setAppearance(this.SILVER_HOLDER);
		this.mPipComps = [];
		this.mAbilityIds = [];
		this.mLockImage = this.GUI.Component(null);
		this.mLockImage.setAppearance("Lock");
		this.mLockImage.setSize(this.DEFAULT_ICON_SIZE, this.DEFAULT_ICON_SIZE);
		this.mLockImage.setPreferredSize(this.DEFAULT_ICON_SIZE, this.DEFAULT_ICON_SIZE);
		this.mLockImage.setPosition(shiftAmount, shiftAmount);
		this.mLockImage.setVisible(false);
		this.add(this.mLockImage);
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function getAbilityIds()
	{
		return this.mAbilityIds;
	}

	function abilityIdCompare( a, b )
	{
		if (a > b)
		{
			return 1;
		}
		else if (a < b)
		{
			return -1;
		}

		return 0;
	}

	function addPip( abilityId )
	{
		local numPips = this.mPipComps.len() + 1;
		local pipComp = this.GUI.Pip();
		pipComp.setPosition(-6, this.ACTION_HOLDER_ICON_SIZE - pipComp.getSize().height * numPips - 6);
		this.mPipComps.append(pipComp);
		this.add(pipComp);
		this.mAbilityIds.append(abilityId);
		this.mAbilityIds.sort(this.abilityIdCompare);
	}

	function enablePip()
	{
		foreach( pipComp in this.mPipComps )
		{
			if (!pipComp.isEnabled())
			{
				pipComp.setEnabled(true);
				this.mLastEnabledPipCount = this.mLastEnabledPipCount + 1;

				if (this.mLastEnabledPipCount < this.mAbilityIds.len())
				{
					this.mMessageBroadcaster.broadcastMessage("onUpdateAbilityId", this.mAbilityIds[this.mLastEnabledPipCount]);
				}

				return true;
			}
		}

		return false;
	}

	function getTotalPips()
	{
		return this.mPipComps.len();
	}

	function isAllPipsEnabled()
	{
		if (this.mLastEnabledPipCount + 1 == this.mPipComps.len())
		{
			return true;
		}

		return false;
	}

	function getStartAbilityId()
	{
		if (this.mAbilityIds.len() > 0)
		{
			return this.mAbilityIds[0];
		}

		return 0;
	}

	function clearPips()
	{
		foreach( pipComp in this.mPipComps )
		{
			pipComp.setEnabled(false);
		}

		this.mLastEnabledPipCount = -1;
		this.setAppearance(this.SILVER_HOLDER);
		this.mMessageBroadcaster.broadcastMessage("onUpdateAbilityId", this.mAbilityIds[0]);
	}

	function getNextDisabledAbilityId()
	{
		if (this.mLastEnabledPipCount + 1 < this.mAbilityIds.len())
		{
			return this.mAbilityIds[this.mLastEnabledPipCount + 1];
		}

		return 0;
	}

	function _hasMyProfession( professions )
	{
		if (professions == null || professions.len() == 0)
		{
			return true;
		}
		else
		{
			local profession = ::_avatar.getStat(this.Stat.PROFESSION, true);
			professions = professions.toupper();

			if (profession >= 1 && profession <= 4)
			{
				return professions.find(this.AbilityTreeType[profession - 1].profession) != null;
			}
			else
			{
				return false;
			}
		}
	}

	function getHighlightType()
	{
		local look = 0;
		local abilityId = this.getNextDisabledAbilityId();

		if (this.isAllPipsEnabled())
		{
			look = look | this.AbilityScreenFlags.OWN;
		}
		else if (abilityId != 0 && ::_AbilityManager.getAbilityById(abilityId))
		{
			local ability = ::_AbilityManager.getAbilityById(abilityId);
			local requiredLevel = ability.getPurchaseLevelRequired();
			local ownLevel = ::_avatar.getStat(this.Stat.LEVEL, true);

			if (this.mLastEnabledPipCount != -1)
			{
				look = look | this.AbilityScreenFlags.OWN;
			}

			local hasProfesssion = this._hasMyProfession(ability.getPurchaseClassRequired());
			local canPurchase = true;

			if (hasProfesssion)
			{
				local requiredAbilityId = ability.getPurchaseAbilitiesRequired();

				if (ownLevel < requiredLevel)
				{
					canPurchase = false;
				}

				if (canPurchase)
				{
					if (requiredAbilityId.len() == 0)
					{
					}
					else
					{
						foreach( reqAb in requiredAbilityId )
						{
							local requiredAbility = ::_AbilityManager.getAbilityById(reqAb);

							if (!requiredAbility || requiredAbilityId == 0 || requiredAbility.getOwnage() <= 1)
							{
								canPurchase = false;
							}
						}
					}
				}
			}
			else
			{
				canPurchase = false;
			}

			if (canPurchase)
			{
				look = look | this.AbilityScreenFlags.CAN_PURCHASE;
			}

			if (!hasProfesssion)
			{
				this._handleLockDisabledIcon();
			}
		}

		if (abilityId != 0)
		{
			local ability = ::_AbilityManager.getAbilityById(abilityId);

			if (ability.getUseType() & this.AbilityUseType.PASSIVE)
			{
				look = look | this.AbilityScreenFlags.PASSIVE;
			}
		}

		this._updateHolderAppearance(look);
		return look;
	}

	function _handleLockDisabledIcon()
	{
		this.mLockImage.setVisible(true);

		foreach( pipComp in this.mPipComps )
		{
			pipComp.setDisabled(true);
		}
	}

	function _updateHolderAppearance( look )
	{
		if ((look & this.AbilityScreenFlags.OWN) > 0)
		{
			this.setAppearance(this.GOLD_HOLDER);
		}
		else
		{
			this.setAppearance(this.SILVER_HOLDER);
		}
	}

}

class this.GUI.AbilityProgressBar extends this.GUI.Container
{
	static mClassName = "AbilityProgressBar";
	static IMAGE_SIZE = 18;
	static MAX_ABILITY_BAR = 6;
	static PROGRESS_BAR_WIDTH = 156;
	static PROGRESS_BAR_HEIGHT = 24;
	static INACTIVE_COLOR = "585858";
	mAbilityIds = null;
	mMessageBroadcaster = null;
	mRadioGroup = null;
	static AbilityImageTypes = {
		DEFAULT = null,
		ON = "AbilityBar/On",
		NEXT_LEVEL = "AbilityBar/NextLevel"
	};
	mAbilityBars = null;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.BoxLayout());
		this.setSize(this.PROGRESS_BAR_WIDTH, this.PROGRESS_BAR_HEIGHT);
		this.setPreferredSize(this.PROGRESS_BAR_WIDTH, this.PROGRESS_BAR_HEIGHT);
		this.setInsets(1, 0, 0, 13);
		this.getLayoutManager().setGap(4);
		this.setAppearance("ImpBar");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mAbilityBars = [];
		this.mRadioGroup = this.GUI.RadioGroup();
		this.mRadioGroup.addListener(this);

		for( local i = 0; i < this.MAX_ABILITY_BAR; i++ )
		{
			local number = i + 1;
			local abilityBar = this.GUI.Button(number.tostring());
			abilityBar.setFixedSize(this.IMAGE_SIZE, this.IMAGE_SIZE);
			abilityBar.setAppearance(this.AbilityImageTypes.DEFAULT);
			abilityBar.setFontColor(this.Colors.black);
			abilityBar.setDisabledFontColor(this.INACTIVE_COLOR);
			abilityBar.setEnabled(false);
			abilityBar.addActionListener(this);
			abilityBar.setPressMessage("_selectedAbility");
			abilityBar.setRadioGroup(this.mRadioGroup);
			this.add(abilityBar);
			this.mAbilityBars.append(abilityBar);
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function updateAbilityBarImages( currAbilityId, abilityIds )
	{
		if (currAbilityId == 0)
		{
			return;
		}

		local currAbility = ::_AbilityManager.getAbilityById(currAbilityId);

		if (currAbility && currAbility.getIsValid())
		{
			this.resetImages();
			this.mAbilityIds = abilityIds;

			foreach( abilityId in abilityIds )
			{
				local ability = ::_AbilityManager.getAbilityById(abilityId);

				if (ability && ability.getTier())
				{
					local tier = ability.getTier();

					if (tier <= this.mAbilityBars.len() && ability.getOwnage() > this.AbilityOwnageType.ABILITY_NOT_OWNED)
					{
						this.mAbilityBars[tier - 1].setAppearance(this.AbilityImageTypes.ON);
						this.mAbilityBars[tier - 1].setEnabled(true);
						this.mAbilityBars[tier - 1].setData(abilityId);
						this.mAbilityBars[tier - 1].setSelection(true);
						this.mAbilityBars[tier - 1].setTooltip("Tier " + tier.tostring());
					}
					else if (tier <= this.mAbilityBars.len())
					{
						this.mAbilityBars[tier - 1].setAppearance(this.AbilityImageTypes.NEXT_LEVEL);
						this.mAbilityBars[tier - 1].setEnabled(true);
						this.mAbilityBars[tier - 1].setData(abilityId);
						this.mAbilityBars[tier - 1].setSelection(true);
						this.mAbilityBars[tier - 1].setTooltip("Tier " + tier.tostring());
					}

					if (abilityId == currAbilityId)
					{
						this.mRadioGroup.setSelected(this.mAbilityBars[tier - 1], true);
					}
				}
			}
		}
	}

	function _selectedAbility( button )
	{
		local abilityId = button.getData();
		this.mRadioGroup.setSelected(button);

		if (abilityId)
		{
			local ability = ::_AbilityManager.getAbilityById(abilityId);
			this.mMessageBroadcaster.broadcastMessage("_setCurrentAbilitySelection", abilityId, this.mAbilityIds, false);
		}
	}

	function resetImages()
	{
		foreach( abilityBar in this.mAbilityBars )
		{
			abilityBar.setAppearance(this.AbilityImageTypes.DEFAULT);
			abilityBar.setEnabled(false);
			abilityBar.setSelection(false);
			abilityBar.setData(null);
			abilityBar.setTooltip("");
			this.mAbilityIds = null;
		}
	}

}

