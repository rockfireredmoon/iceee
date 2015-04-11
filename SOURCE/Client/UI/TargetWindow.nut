this.require("Combat/CombatEquations");
this.require("UI/Screens");
class this.Screens.TargetWindow extends this.GUI.Container
{
	mNameMiddleXPos = 94;
	mNameMiddleYPos = 7;
	mHealthBarMiddleXPos = 90;
	mHealthBarMiddleYPos = 45;
	mMightChargeMiddleXPos = 107;
	mMightChargeMiddleYPos = 39;
	mWillChargeMiddleXPos = 170;
	mWillChargeMiddleYPos = 39;
	mLevelMiddleXPos = 128;
	mLevelMiddleYPos = 62;
	mCastingTimeMiddleXPos = 35;
	mCastingTimeMiddleYPos = 42;
	mSpellNameMiddleXPos = 112;
	mSpellNameMiddleYPos = 41;
	mHealthBarXLeft = 7;
	mHealthBarYLeft = 34;
	mCastingBarXLeft = 59;
	mCastingBarYLeft = 59;
	mDragging = false;
	mMouseOffset = {};
	mMiddleSectionWidth = 125;
	mName = null;
	mLevel = null;
	mBuffContainer = null;
	mDebuffContainer = null;
	mMightCharges = [];
	mWillCharges = [];
	static MAX_CHARGES = 5;
	mMobType = null;
	mMainBackground = null;
	mCombatBackground = null;
	mTargetHealthBarBG = null;
	mMaxHealthSize = 0;
	mMaxCastingSize = 0;
	mHealthBar = null;
	mHealthLabel = null;
	mCastingBar = null;
	mCastingTimerLabel = null;
	mSpellLabel = null;
	mTargetCastingBarBG = null;
	mTargetCombatBG = null;
	mRarityIndicator = null;
	mRarityContainer = null;
	mCategory = null;
	mCastingLength = 0.0;
	mLastMiliSecond = 0.0;
	mTimer = null;
	mMenu = null;
	mNextFreeBuffColumn = 7;
	mNextFreeDebuffColumn = 7;
	mNextFreeDebuffRow = 0;
	mBuffContainerDirty = false;
	mDebuffContainerDirty = false;
	mLastObject = null;
	constructor()
	{
		this.GUI.Container.constructor(null);
		::_Connection.addListener(this);
		this.mTimer = ::Timer();
		this.setFont(this.GUI.Font("Maiandra", 12));
		this.setSize(260, 115);
		this.setPreferredSize(260, 115);
		this.setPosition(285, 15);
		this.mMightCharges = [];
		this.mWillCharges = [];
		this.mMightCharges = this.array(this.MAX_CHARGES);
		this.mWillCharges = this.array(this.MAX_CHARGES);
		local targetNameFont = this.GUI.Font("MaiandraOutline", 22);
		this.mName = this.GUI.Label("");
		this.mName.setFontColor(this.Colors.white);
		this.mName.setFont(targetNameFont);
		this.mName.setPosition(0, 15);
		this.mName.setSize(180, 20);
		this.mName.setPreferredSize(180, 20);
		this.mName.setTextAlignment(0.5, 1.0);
		this.mRarityContainer = this.GUI.Container(null);
		this.mRarityContainer.setAppearance("RarityShield");
		this.mRarityContainer.setSize(30, 38);
		this.mRarityContainer.setPreferredSize(30, 38);
		this.mRarityContainer.setPosition(193, 70);
		this.add(this.mRarityContainer);
		this.mMainBackground = this._buildMainBackground();
		this.mMainBackground.setPosition(0, 13);
		this.mCombatBackground = this._buildCombatBackground();
		this.mCombatBackground.setPosition(174, 3);
		this.add(this.mName);
		this.add(this.mMainBackground);
		this.add(this.mCombatBackground);
		this.mCastingBar.setVisible(false);
		this.mCastingTimerLabel.setVisible(false);
		this.mTargetCastingBarBG.setVisible(false);
		this.mMenu = this.GUI.PopupMenu();
		this.mMenu.addActionListener(this);
		local mainBackgroundSize = this.mMainBackground.getSize();
		this.mDebuffContainer = this.GUI.ActionContainer("target_buff_container", 2, 8, 0, 0, this, false, 16, 16);
		this.mDebuffContainer.setTransparent();
		this.mDebuffContainer.setSize(128, 32);
		this.mDebuffContainer.setPosition(50, mainBackgroundSize.height + 48);
		this.mDebuffContainer.setAllButtonsDraggable(false);
		this.mDebuffContainer.setPassThru(true);
		this.mDebuffContainer.generateTooltipWithPassThru(true);
		this.add(this.mDebuffContainer);
		this.mBuffContainer = this.GUI.ActionContainer("target_debuff_container", 1, 8, 0, 0, this, false, 16, 16);
		this.mBuffContainer.setPosition(50, mainBackgroundSize.height + 32);
		this.mBuffContainer.setSize(128, 16);
		this.mBuffContainer.setTransparent();
		this.mBuffContainer.setAllButtonsDraggable(false);
		this.mBuffContainer.setPassThru(true);
		this.mBuffContainer.generateTooltipWithPassThru(true);
		this.add(this.mBuffContainer);
		this.setCached(::Pref.get("video.UICache"));
	}

	function addMod( mod )
	{
		if (!(mod instanceof this.BuffDebuff) && !(mod instanceof this.StatusEffect))
		{
			return;
		}

		local modType = mod.getBuffType();
		local actionContainer;
		local indexToPlaceActionIn = -1;

		if (modType == this.BuffType.BUFF)
		{
			indexToPlaceActionIn = this.mNextFreeBuffColumn;
			actionContainer = this.mBuffContainer;
			this.mBuffContainerDirty = true;
		}
		else if (modType == this.BuffType.DEBUFF)
		{
			indexToPlaceActionIn = this.mNextFreeDebuffRow * 8 + this.mNextFreeDebuffColumn;
			actionContainer = this.mDebuffContainer;
			this.mDebuffContainerDirty = true;
		}
		else if (modType == this.BuffType.WORLD)
		{
			return;
		}
		else
		{
			return;
		}

		actionContainer.addAction(mod, false, indexToPlaceActionIn);

		if (modType == this.BuffType.BUFF)
		{
			this.mNextFreeBuffColumn--;

			if (this.mNextFreeBuffColumn < 0)
			{
			}
		}
		else if (modType == this.BuffType.DEBUFF)
		{
			this.mNextFreeDebuffColumn--;

			if (this.mNextFreeDebuffColumn < 0)
			{
				this.mNextFreeDebuffRow++;

				if (this.mNextFreeDebuffRow >= 2)
				{
				}

				this.mNextFreeDebuffColumn = 7;
			}
		}
	}

	function fillOut( sceneObject, ... )
	{
		this.updateName(sceneObject);
		this.updateLevel(sceneObject);
		this.updateHealth(sceneObject);
		this.updateWillCharges(sceneObject);
		this.updateMightCharges(sceneObject);
		this.updateRarity(sceneObject);
		this.updateCategory(sceneObject);
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function onEnterFrame()
	{
		if (!::_avatar)
		{
			return;
		}

		local target = ::_avatar.getTargetObject();

		if (target && target.isCasting())
		{
			this.mCastingBar.setVisible(true);
			this.mCastingTimerLabel.setVisible(true);
			this.mTargetCastingBarBG.setVisible(true);
			this.mSpellLabel.setVisible(true);
			local abilityID = target.getUsingAbilityID();
			local ability = this._AbilityManager.getAbilityById(abilityID);
			local warmupDuration = ability.getWarmupDuration();
			local timeRemaining = target.getCastingTimeRemaining();
			this.mSpellLabel.setText(ability.getName());
			this.setCastingTimeRemaining(timeRemaining / 1000.0, warmupDuration / 1000.0);
		}
		else
		{
			this.mCastingBar.setVisible(false);
			this.mCastingTimerLabel.setVisible(false);
			this.mTargetCastingBarBG.setVisible(false);
			this.mSpellLabel.setVisible(false);
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local newpos = ::Screen.getCursorPos();
			local deltax = newpos.x - this.mMouseOffset.x;
			local deltay = newpos.y - this.mMouseOffset.y;
			local pos = this.getPosition();
			pos.x += deltax;
			pos.y += deltay;
			this.mMouseOffset = newpos;
			this.setPosition(pos);
			this.fitToScreen();
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mMouseOffset = ::Screen.getCursorPos();
			this.mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mDragging = false;
			evt.consume();
		}
		else if (evt.button == this.MouseEvent.RBUTTON)
		{
			local target = ::_avatar.getTargetObject();

			if (::partyManager.isInParty() == true)
			{
				this.mMenu.removeAllMenuOptions();
				this.mMenu.addMenuOption("Leave", "Leave Party");

				if (::partyManager.isLeader())
				{
					this.mMenu.addMenuOption("Loot", "Change Loot Setting");

					if (target != ::_avatar)
					{
						this.mMenu.addMenuOption("Kick", "Remove From Party");
						this.mMenu.addMenuOption("inCharge", "Make Leader");
					}

					this.mMenu.showMenu();
				}
				else
				{
					this.mMenu.addMenuOption("Loot", "View Loot Setting");
					::partyManager.isCharacterInParty(::_avatar.getID(), this.mMenu);
				}
			}
		}
	}

	function onMenuItemPressed( menu, menuID )
	{
		local target = ::_avatar.getTargetObject();

		if (!target)
		{
			return;
		}

		switch(menuID)
		{
		case "Leave":
			::_Connection.sendQuery("party", this, [
				"quit"
			]);
			break;

		case "Loot":
			::partyManager.showLootWindow();
			break;

		case "Kick":
			::_Connection.sendQuery("party", ::partyManager, [
				"kick",
				target.getID()
			]);
			break;

		case "inCharge":
			::_Connection.sendQuery("party", ::partyManager, [
				"setLeader",
				target.getID()
			]);
			break;
		}
	}

	function removeAllMods()
	{
		if (this.mNextFreeBuffColumn != 7)
		{
			this.mNextFreeBuffColumn = 7;
			this.mBuffContainer.removeAllActions();
		}

		if (this.mNextFreeDebuffRow != 0 || this.mNextFreeDebuffColumn != 7)
		{
			this.mNextFreeDebuffRow = 0;
			this.mNextFreeDebuffColumn = 7;
			this.mDebuffContainer.removeAllActions();
		}
	}

	function setCastingTimeRemaining( time, totalTime )
	{
		if (time <= 0.001)
		{
			this.mSpellLabel.setText("");
			this.mCastingTimerLabel.setText("");
		}
		else
		{
			local percent = time / totalTime;
			local newWidth = this.mMaxCastingSize * percent;
			local castingBarSize = this.mCastingBar.getSize();
			local timeToShow = this.Util.limitSignificantDigits(time, 1);

			if (this.Util.hasRemainder(timeToShow))
			{
				this.mCastingTimerLabel.setText(timeToShow + "s");
			}
			else
			{
				this.mCastingTimerLabel.setText(timeToShow + ".0s");
			}

			this.mCastingBar.setSize(newWidth, castingBarSize.height);
			this.mCastingBar.setPosition(this.mCastingBarXLeft, this.mCastingBarYLeft);
		}
	}

	function setHealthPercent( percent, ... )
	{
		local anim = vargc == 0 || vargv[0];

		if (percent >= 0.0 && percent <= 1.0)
		{
			local newWidth = this.mMaxHealthSize * percent;
			local healthBarHeight = this.mHealthBar.getSize().height;
			this.mHealthBar.setPosition(this.mHealthBarXLeft, this.mHealthBarYLeft);
			this.mHealthBar.setDestWidth(newWidth, anim);
			local percentToDisplay = this.Util.limitSignificantDigits(percent * 100.0, 0);
			this.mHealthLabel.setText(percentToDisplay.tostring() + "%");
			this._centerHealthBarText();
		}
	}

	function setLevel( targetLevel )
	{
		if (targetLevel == null)
		{
			return;
		}

		local avatarLevel = ::_avatar.getStat(this.Stat.LEVEL, true);

		if (avatarLevel == null)
		{
			return;
		}

		local color = "";
		local levelDifference = targetLevel - avatarLevel;

		if (levelDifference <= -10)
		{
			color = this.ChannelColors.conGrey;
		}
		else if (levelDifference <= -7)
		{
			color = this.ChannelColors.conDarkGreen;
		}
		else if (levelDifference <= -4)
		{
			color = this.ChannelColors.conLightGreen;
		}
		else if (levelDifference <= -1)
		{
			color = this.ChannelColors.conBrightGreen;
		}
		else if (0 == levelDifference)
		{
			color = this.ChannelColors.conWhite;
		}
		else if (1 == levelDifference)
		{
			color = this.ChannelColors.conBlue;
		}
		else if (2 == levelDifference)
		{
			color = this.ChannelColors.conYellow;
		}
		else if (3 == levelDifference)
		{
			color = this.ChannelColors.conOrange;
		}
		else if (4 == levelDifference)
		{
			color = this.ChannelColors.conRed;
		}
		else if (5 == levelDifference)
		{
			color = this.ChannelColors.conPurple;
		}
		else if (levelDifference >= 6)
		{
			color = this.ChannelColors.conBrightPurple;
		}

		this.mLevel.setText("<font size=\"18\" color=\"" + color + "\"><b>" + targetLevel.tostring() + "</b></font>");
		this._centerLevelText();
	}

	function setMightCharges( charges )
	{
		local chargesToSet = charges;

		if (null == chargesToSet)
		{
			chargesToSet = 0;
		}
		else if (chargesToSet > 5)
		{
			chargesToSet = 5;
		}

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			if (i < chargesToSet)
			{
				this.mMightCharges[i].setVisible(true);
			}
			else
			{
				this.mMightCharges[i].setVisible(false);
			}
		}
	}

	function setName( name )
	{
		this.mName.setText(name);
		this.mName.setAutoFit(true);
	}

	function setVisible( value )
	{
		this.GUI.Component.setVisible(value);
	}

	function setRarity( rarity )
	{
		switch(rarity)
		{
		case this.CreatureRarityType.HEROIC:
			this.mRarityIndicator.setAppearance("HostileHeroic");
			this.mRarityIndicator.setTooltip("Heroic");
			this.mRarityContainer.setTooltip("Heroic");
			break;

		case this.CreatureRarityType.EPIC:
			this.mRarityIndicator.setAppearance("HostileEpic");
			this.mRarityIndicator.setTooltip("Epic");
			this.mRarityContainer.setTooltip("Epic");
			break;

		case this.CreatureRarityType.LEGEND:
			this.mRarityIndicator.setAppearance("HostileLegend");
			this.mRarityIndicator.setTooltip("Legend");
			this.mRarityContainer.setTooltip("Legend");
			break;

		case this.CreatureRarityType.NORMAL:
		default:
			this.mRarityIndicator.setAppearance("HostileNormal");
			this.mRarityIndicator.setTooltip("Normal");
			this.mRarityContainer.setTooltip("Normal");
			break;
		}
	}

	function setCategory( category )
	{
		local categoryName = "Icon-Player-Portrait.png";

		if (category != "")
		{
			categoryName = "Icon-" + category + "-Portrait.png";
		}

		this.mCategory.setImageName(categoryName);
		this.mCategory.setVisible(true);
	}

	function setWillCharges( charges )
	{
		local chargesToSet = charges;

		if (null == chargesToSet)
		{
			chargesToSet = 0;
		}
		else if (chargesToSet > 5)
		{
			chargesToSet = 5;
		}

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			if (i < chargesToSet)
			{
				this.mWillCharges[i].setVisible(true);
			}
			else
			{
				this.mWillCharges[i].setVisible(false);
			}
		}
	}

	function onCreatureUpdated( callingFunction, creature )
	{
		if (::_avatar && ::_avatar.getTargetObject() == creature)
		{
			this.fillOut(creature);
		}
	}

	function updateCategory( sceneObject )
	{
		this.setCategory(sceneObject.getStat(this.Stat.CREATURE_CATEGORY));
	}

	function updateHealth( sceneObject )
	{
		local con = sceneObject.getStat(this.Stat.CONSTITUTION, true);
		local baseHealth = sceneObject.getStat(this.Stat.BASE_HEALTH, true);
		local bonusHealth = sceneObject.getStat(this.Stat.HEALTH_MOD, true);
		local rarity = sceneObject.getStat(this.Stat.RARITY, true);
		local maxHealth = ::_combatEquations.calcMaxHealth(baseHealth, con, bonusHealth, rarity);
		local curHealth = sceneObject.getStat(this.Stat.HEALTH, true);

		if (maxHealth != 0)
		{
			if (curHealth > 0 || curHealth == 0 && sceneObject.isDead())
			{
				this.setHealthPercent(curHealth.tofloat() / maxHealth.tofloat(), this.mLastObject == sceneObject);
			}
		}
		else
		{
			this.setHealthPercent(1.0, false);
		}

		this.mLastObject = sceneObject;
	}

	function updateLevel( sceneObject )
	{
		this.setLevel(sceneObject.getStat(this.Stat.LEVEL, true));
	}

	function updateMods( sceneObject )
	{
		local mods = sceneObject.getUniqueStatusModifiers();
		this.removeAllMods();

		foreach( mod in mods )
		{
			if (mod instanceof this.StatusEffect)
			{
				if (this.StatusEffects[mod.getEffectID()].icon != "")
				{
					this.addMod(mod);
				}
			}
			else
			{
				this.addMod(mod);
			}
		}

		if (this.mBuffContainerDirty)
		{
			this.mBuffContainer.updateContainer();
			this.mBuffContainerDirty = false;
		}

		if (this.mDebuffContainerDirty)
		{
			this.mDebuffContainer.updateContainer();
			this.mDebuffContainerDirty = false;
		}
	}

	function updateMightCharges( sceneObject )
	{
		this.setMightCharges(sceneObject.getStat(this.Stat.MIGHT_CHARGES, true));
	}

	function updateName( sceneObject )
	{
		this.setName(sceneObject.getStat(this.Stat.DISPLAY_NAME));
	}

	function updateWillCharges( sceneObject )
	{
		this.setWillCharges(sceneObject.getStat(this.Stat.WILL_CHARGES, true));
	}

	function updateRarity( sceneObject )
	{
		if (sceneObject.hasStatusEffect(this.StatusEffects.INVINCIBLE) || sceneObject.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
		{
			this.mRarityIndicator.setVisible(false);
			this.mRarityContainer.setVisible(false);
		}
		else
		{
			this.mRarityIndicator.setVisible(true);
			this.mRarityContainer.setVisible(true);
			this.setRarity(sceneObject.getStat(this.Stat.RARITY, true));
		}
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		::_enterFrameRelay.addListener(this);
		this.mWidget.addListener(this);
	}

	function _buildCombatBackground()
	{
		local rightSection = this.GUI.Container(null);
		this.mCategory = this.GUI.Image();
		this.mCategory.setPosition(-10, 13);
		this.mCategory.setSize(82, 79);
		this.mLevel = this.GUI.HTML("");
		this.mLevel.setSize(15, 20);
		this.mLevel.setPreferredSize(15, 20);
		this.mLevel.setTooltip("Level");
		this.mLevel.setChildrenInheritTooltip(true);
		rightSection.add(this.mCategory);
		local CHARGES_SIZE = 13;
		local mightPositions = {
			[0] = {
				x = 6,
				y = 52
			},
			[1] = {
				x = -3,
				y = 44
			},
			[2] = {
				x = -5,
				y = 33
			},
			[3] = {
				x = 1.5,
				y = 23
			},
			[4] = {
				x = 8,
				y = 14
			}
		};
		local willPositions = {
			[0] = {
				x = 48,
				y = 52
			},
			[1] = {
				x = 54,
				y = 44
			},
			[2] = {
				x = 56,
				y = 33
			},
			[3] = {
				x = 50,
				y = 23
			},
			[4] = {
				x = 43,
				y = 14
			}
		};

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			this.mMightCharges[i] = this.GUI.Container();
			this.mMightCharges[i].setSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mMightCharges[i].setPreferredSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mMightCharges[i].setPosition(mightPositions[i].x, mightPositions[i].y);
			this.mMightCharges[i].setAppearance("PlayerMight");
			rightSection.add(this.mMightCharges[i]);
			this.mWillCharges[i] = this.GUI.Container();
			this.mWillCharges[i].setSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mWillCharges[i].setPreferredSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mWillCharges[i].setPosition(willPositions[i].x, willPositions[i].y);
			this.mWillCharges[i].setAppearance("PlayerWill");
			rightSection.add(this.mWillCharges[i]);
		}

		rightSection.add(this.mLevel);
		this.mRarityIndicator = this.GUI.Container(null);
		this.mRarityIndicator.setSize(21, 21);
		this.mRarityIndicator.setPreferredSize(21, 21);
		this.mRarityIndicator.setPosition(20, 75);
		this.mRarityIndicator.setAppearance("HostileNormal");
		this.mRarityIndicator.setTooltip("Normal");
		rightSection.add(this.mRarityIndicator);
		rightSection.setSize(82, 80);
		rightSection.setPreferredSize(82, 80);
		return rightSection;
	}

	function _buildMainBackground()
	{
		local middleSection = this.GUI.Container(null);
		this.mTargetHealthBarBG = this.GUI.Container();
		this.mTargetHealthBarBG.setSize(250, 77);
		this.mTargetHealthBarBG.setPreferredSize(250, 77);
		this.mTargetHealthBarBG.setAppearance("TargetBG");
		this.mTargetCastingBarBG = this.GUI.Container();
		this.mTargetCastingBarBG.setSize(152, 26);
		this.mTargetCastingBarBG.setPreferredSize(152, 26);
		this.mTargetCastingBarBG.setPosition(15, 52);
		this.mTargetCastingBarBG.setAppearance("TargetAndCastingBarBG");
		this.mHealthBar = this.GUI.AnimatedBar();
		this.mHealthBar.setSize(158, 15);
		this.mHealthBar.setPreferredSize(158, 15);
		this.mHealthBar.setPosition(this.mHealthBarXLeft, this.mHealthBarYLeft);
		this.mHealthBar.setAppearance("HealthBar");
		this.mHealthBar.setTooltip("Health bar");
		this.mMaxHealthSize = this.mHealthBar.getSize().width;
		this.mCastingBar = this.GUI.Container();
		this.mCastingBar.setSize(104, 10);
		this.mCastingBar.setPreferredSize(104, 10);
		this.mCastingBar.setPosition(this.mCastingBarXLeft, this.mCastingBarYLeft);
		this.mCastingBar.setAppearance("CastingBar");
		this.mMaxCastingSize = this.mCastingBar.getSize().width;
		local healthBarFont = this.GUI.Font("MaiandraOutline", 22);
		local castingTimerFont = this.GUI.Font("MaiandraOutline", 16);
		local spellNameFont = this.GUI.Font("MaiandraOutline", 14);
		this.mHealthLabel = this.GUI.Label("");
		this.mHealthLabel.setFont(healthBarFont);
		this.mHealthLabel.setFontColor(this.Colors.white);
		this.mHealthLabel.setTooltip("Health bar");
		this.mCastingTimerLabel = this.GUI.Label("");
		this.mCastingTimerLabel.setPosition(20, 55);
		this.mCastingTimerLabel.setFont(castingTimerFont);
		this.mCastingTimerLabel.setFontColor(this.Colors.white);
		local labelContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		labelContainer.setPosition(60, 63);
		middleSection.add(labelContainer);
		this.mSpellLabel = this.GUI.Label("Testing 1 2 3");
		this.mSpellLabel.setSize(100, 20);
		this.mSpellLabel.setPreferredSize(100, 20);
		this.mSpellLabel.setFont(spellNameFont);
		this.mSpellLabel.setFontColor(this.Colors.white);
		labelContainer.add(this.mSpellLabel);
		middleSection.add(this.mTargetHealthBarBG);
		middleSection.add(this.mTargetCastingBarBG);
		middleSection.add(this.mHealthBar);
		middleSection.add(this.mHealthLabel);
		middleSection.add(this.mCastingBar);
		middleSection.add(this.mCastingTimerLabel);
		middleSection.setSize(218, 52);
		middleSection.setPreferredSize(218, 52);
		return middleSection;
	}

	function _centerHealthBarText()
	{
		local font = this.mHealthLabel.getFont();
		local fontMetrics = font.getTextMetrics(this.mHealthLabel.getText());
		this.mHealthLabel.setPosition(this.mHealthBarMiddleXPos - fontMetrics.width / 2, this.mHealthBarMiddleYPos - fontMetrics.height / 2);
	}

	function _centerLevelText()
	{
		local font = this.mLevel.getFont();
		local fontMetrics = font.getTextMetrics(this.mLevel.getText());
		this.mLevel.setPosition(this.mLevelMiddleXPos - fontMetrics.width / 2, this.mLevelMiddleYPos - fontMetrics.height / 2);
	}

	function _centerNameText()
	{
		local font = this.mName.getFont();
		local fontMetrics = font.getTextMetrics(this.mName.getText());
		this.mName.setPosition(this.mNameMiddleXPos - fontMetrics.width / 2, this.mNameMiddleYPos - fontMetrics.height / 2 + 6);
	}

	function _removeNotify()
	{
		this._enterFrameRelay.removeListener(this);
		this.mWidget.removeListener(this);
		this.GUI.Component._removeNotify();
	}

}

