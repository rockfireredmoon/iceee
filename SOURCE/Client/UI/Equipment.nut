this.require("Constants");
this.require("UI/ActionContainer");
this.require("Combat/CombatEquations");
this.require("UI/Screens");
this.EquipmentContainerNames <- {
	[this.ItemEquipSlot.WEAPON_MAIN_HAND] = "eq_main_hand",
	[this.ItemEquipSlot.WEAPON_OFF_HAND] = "eq_off_hand",
	[this.ItemEquipSlot.WEAPON_RANGED] = "eq_ranged",
	[this.ItemEquipSlot.ARMOR_HEAD] = "eq_head",
	[this.ItemEquipSlot.ARMOR_NECK] = "eq_neck",
	[this.ItemEquipSlot.ARMOR_SHOULDER] = "eq_shoulders",
	[this.ItemEquipSlot.ARMOR_CHEST] = "eq_chest",
	[this.ItemEquipSlot.ARMOR_ARMS] = "eq_arms",
	[this.ItemEquipSlot.ARMOR_HANDS] = "eq_hands",
	[this.ItemEquipSlot.ARMOR_WAIST] = "eq_waist",
	[this.ItemEquipSlot.ARMOR_LEGS] = "eq_legs",
	[this.ItemEquipSlot.ARMOR_FEET] = "eq_feet",
	[this.ItemEquipSlot.ARMOR_RING_L] = "eq_ring_l",
	[this.ItemEquipSlot.ARMOR_RING_R] = "eq_ring_r",
	[this.ItemEquipSlot.ARMOR_AMULET] = "eq_amulet",
	[this.ItemEquipSlot.FOCUS_FIRE] = "eq_focus_fire",
	[this.ItemEquipSlot.FOCUS_FROST] = "eq_focus_frost",
	[this.ItemEquipSlot.FOCUS_MYSTIC] = "eq_focus_mystic",
	[this.ItemEquipSlot.FOCUS_DEATH] = "eq_focus_death",
	[this.ItemEquipSlot.COSMETIC_SHOULDER_L] = "eq_cosmetic_shoulder_l",
	[this.ItemEquipSlot.COSMETIC_HIP_L] = "eq_cosmetic_hip_l",
	[this.ItemEquipSlot.COSMETIC_SHOULDER_R] = "eq_cosmetic_shoulder_r",
	[this.ItemEquipSlot.COSMETIC_HIP_R] = "eq_cosmetic_hip_r",
	[this.ItemEquipSlot.RED_CHARM] = "eq_red_charm",
	[this.ItemEquipSlot.GREEN_CHARM] = "eq_green_charm",
	[this.ItemEquipSlot.BLUE_CHARM] = "eq_blue_charm",
	[this.ItemEquipSlot.ORANGE_CHARM] = "eq_orange_charm",
	[this.ItemEquipSlot.YELLOW_CHARM] = "eq_yellow_charm",
	[this.ItemEquipSlot.PURPLE_CHARM] = "eq_purple_charm"
};
this.EquipmentMapContainer <- {
	[this.ItemEquipType.NONE] = [],
	[this.ItemEquipType.WEAPON_1H] = [
		this.ItemEquipSlot.WEAPON_MAIN_HAND,
		this.ItemEquipSlot.WEAPON_OFF_HAND
	],
	[this.ItemEquipType.WEAPON_1H_UNIQUE] = [
		this.ItemEquipSlot.WEAPON_MAIN_HAND,
		this.ItemEquipSlot.WEAPON_OFF_HAND
	],
	[this.ItemEquipType.WEAPON_1H_MAIN] = [
		this.ItemEquipSlot.WEAPON_MAIN_HAND
	],
	[this.ItemEquipType.WEAPON_1H_OFF] = [
		this.ItemEquipSlot.WEAPON_OFF_HAND
	],
	[this.ItemEquipType.WEAPON_2H] = [
		this.ItemEquipSlot.WEAPON_MAIN_HAND
	],
	[this.ItemEquipType.WEAPON_RANGED] = [
		this.ItemEquipSlot.WEAPON_RANGED
	],
	[this.ItemEquipType.ARMOR_SHIELD] = [
		this.ItemEquipSlot.WEAPON_OFF_HAND
	],
	[this.ItemEquipType.ARMOR_HEAD] = [
		this.ItemEquipSlot.ARMOR_HEAD
	],
	[this.ItemEquipType.ARMOR_NECK] = [
		this.ItemEquipSlot.ARMOR_NECK
	],
	[this.ItemEquipType.ARMOR_SHOULDER] = [
		this.ItemEquipSlot.ARMOR_SHOULDER
	],
	[this.ItemEquipType.ARMOR_CHEST] = [
		this.ItemEquipSlot.ARMOR_CHEST
	],
	[this.ItemEquipType.ARMOR_ARMS] = [
		this.ItemEquipSlot.ARMOR_ARMS
	],
	[this.ItemEquipType.ARMOR_HANDS] = [
		this.ItemEquipSlot.ARMOR_HANDS
	],
	[this.ItemEquipType.ARMOR_WAIST] = [
		this.ItemEquipSlot.ARMOR_WAIST
	],
	[this.ItemEquipType.ARMOR_LEGS] = [
		this.ItemEquipSlot.ARMOR_LEGS
	],
	[this.ItemEquipType.ARMOR_FEET] = [
		this.ItemEquipSlot.ARMOR_FEET
	],
	[this.ItemEquipType.ARMOR_RING] = [
		this.ItemEquipSlot.ARMOR_RING_L,
		this.ItemEquipSlot.ARMOR_RING_R
	],
	[this.ItemEquipType.ARMOR_RING_UNIQUE] = [
		this.ItemEquipSlot.ARMOR_RING_L,
		this.ItemEquipSlot.ARMOR_RING_R
	],
	[this.ItemEquipType.ARMOR_AMULET] = [
		this.ItemEquipSlot.ARMOR_AMULET
	],
	[this.ItemEquipType.FOCUS_FIRE] = [
		this.ItemEquipSlot.FOCUS_FIRE
	],
	[this.ItemEquipType.FOCUS_FROST] = [
		this.ItemEquipSlot.FOCUS_FROST
	],
	[this.ItemEquipType.FOCUS_MYSTIC] = [
		this.ItemEquipSlot.FOCUS_MYSTIC
	],
	[this.ItemEquipType.FOCUS_DEATH] = [
		this.ItemEquipSlot.FOCUS_DEATH
	],
	[this.ItemEquipType.CONTAINER] = [
		this.ItemEquipSlot.CONTAINER_0,
		this.ItemEquipSlot.CONTAINER_1,
		this.ItemEquipSlot.CONTAINER_2,
		this.ItemEquipSlot.CONTAINER_3
	],
	[this.ItemEquipType.COSEMETIC_SHOULDER] = [
		this.ItemEquipSlot.COSMETIC_SHOULDER_L,
		this.ItemEquipSlot.COSMETIC_HIP_L,
		this.ItemEquipSlot.COSMETIC_SHOULDER_R,
		this.ItemEquipSlot.COSMETIC_HIP_R
	],
	[this.ItemEquipType.COSEMETIC_HIP] = [
		this.ItemEquipSlot.COSMETIC_SHOULDER_L,
		this.ItemEquipSlot.COSMETIC_HIP_L,
		this.ItemEquipSlot.COSMETIC_SHOULDER_R,
		this.ItemEquipSlot.COSMETIC_HIP_R
	],
	[this.ItemEquipType.RED_CHARM] = [
		this.ItemEquipSlot.RED_CHARM
	],
	[this.ItemEquipType.GREEN_CHARM] = [
		this.ItemEquipSlot.GREEN_CHARM
	],
	[this.ItemEquipType.BLUE_CHARM] = [
		this.ItemEquipSlot.BLUE_CHARM
	],
	[this.ItemEquipType.ORANGE_CHARM] = [
		this.ItemEquipSlot.ORANGE_CHARM
	],
	[this.ItemEquipType.YELLOW_CHARM] = [
		this.ItemEquipSlot.YELLOW_CHARM
	],
	[this.ItemEquipType.PURPLE_CHARM] = [
		this.ItemEquipSlot.PURPLE_CHARM
	]
};
class this.Screens.Equipment extends this.GUI.Frame
{
	STAT_NORMAL = 0;
	STAT_BUFFED = 1;
	STAT_DEBUFFED = 2;
	static mScreenName = "Equipment";
	mConstructed = false;
	mBroadcaster = null;
	mEQContainer = null;
	mAdvancedStatSection = null;
	mDefensiveStatSection = null;
	mOffensiveStatSection = null;
	mPaperDollTab = null;
	mEQAmulet = null;
	mEQArms = null;
	mEQRingLeft = null;
	mEQHead = null;
	mEQNeck = null;
	mEQChest = null;
	mEQWaist = null;
	mEQLegs = null;
	mEQFeet = null;
	mEQShoulders = null;
	mEQHands = null;
	mEQRingRight = null;
	mEQMainHand = null;
	mEQOffHand = null;
	mEQRanged = null;
	mEQRedCharm = null;
	mEQOrangeCharm = null;
	mEQPurpleCharm = null;
	mEQYellowCharm = null;
	mEQGreenCharm = null;
	mEQBlueCharm = null;
	mEQLeftShoulder = null;
	mEQLeftHip = null;
	mEQRightShoulder = null;
	mEQRightHip = null;
	mLevelClass = null;
	mHealth = null;
	mStrength = null;
	mDexterity = null;
	mConstitution = null;
	mPsyche = null;
	mSpirit = null;
	mMeleeArmorRating = null;
	mFireArmorRating = null;
	mMysticArmorRating = null;
	mFrostArmorRating = null;
	mDeathArmorRating = null;
	mToHitMelee = null;
	mToHitMagic = null;
	mDodge = null;
	mMagicDeflect = null;
	mParry = null;
	mBlock = null;
	mMeleeDPS = null;
	mAttackSpeed = null;
	mCastingSpeed = null;
	mRunSpeed = null;
	mHealthRegen = null;
	mMeleeCritRate = null;
	mMagicCritRate = null;
	mHealingDPS = null;
	mRangedDPS = null;
	mMeleeResist = null;
	mFireResist = null;
	mMysticResist = null;
	mFrostResist = null;
	mDeathResist = null;
	mAdvanceStatButton = null;
	mOffenseButton = null;
	mDefenseButton = null;
	mShowingMainPaperdoll = true;
	mShowingAdvancedStats = false;
	mDragItem = null;
	mHasDisabledEquipment = false;
	START_X_POS = 0;
	START_Y_POS = 50;
	constructor()
	{
		this.GUI.BigFrame.constructor("Character Screen");
		this.setFont(this.GUI.Font("Maiandra", 20));

		if (::_avatar)
		{
			::_avatar.addListener(this);
		}

		this.mEQContainer = this.GUI.Container(null);
		this.mEQContainer.setInsets(10, 0, 0, 5);
		this.setPosition(this.START_X_POS, this.START_Y_POS);
		local basicStatsSection = this._createBasicStatSection();
		basicStatsSection.setPosition(0, 0);
		this.mEQContainer.add(basicStatsSection);
		this.mPaperDollTab = this.GUI.TabbedPane(true);
		local basicStatsSize = basicStatsSection.getPreferredSize();
		this.mPaperDollTab.setSize(148, 335);
		this.mPaperDollTab.setPosition(basicStatsSize.width - 5, 0);
		this.mPaperDollTab.setTabPlacement("top");
		this.mPaperDollTab.setContentFrameAppearance("InnerPanel");
		this.mPaperDollTab.addTab("", this._createPaperDollSection());
		this.mPaperDollTab.addTab("   ", this._createCharmSection());
		this.mEQContainer.add(this.mPaperDollTab);
		this.mPaperDollTab.setButtonContainerAppearance(0, "TabBar/TabContainer/Round");
		this.mPaperDollTab.setButtonContainerAppearance(1, "TabBar/TabContainer/Round");
		this.mPaperDollTab.addActionListener(this);
		this.mPaperDollTab.setButtonAppearance(0, "TabBar/TabButton/Inactive/Equipment/", "TabBar/TabButton/Inactive/Equipment/");
		this.mPaperDollTab.setButtonAppearance(1, "TabBar/TabButton/Inactive/Charm/", "TabBar/TabButton/Inactive/Charm/");
		this.mPaperDollTab.setButtonContainerTooltip(0, "Armor");
		this.mPaperDollTab.setButtonContainerTooltip(1, "Charm");
		local TAB_SIZE_WIDTH = 31;
		local TAB_SIZE_HEIGHT = 31;
		this.mPaperDollTab.setButtonSize(0, TAB_SIZE_WIDTH, TAB_SIZE_HEIGHT);
		this.mPaperDollTab.setButtonSize(1, TAB_SIZE_WIDTH, TAB_SIZE_HEIGHT);
		this.mPaperDollTab.setButtonSize(2, TAB_SIZE_WIDTH, TAB_SIZE_HEIGHT);
		local TAB_CONTAINER_WIDTH = 42;
		local TAB_CONTAINER_HEIGHT = 38;
		this.mPaperDollTab.setButtonContainerSize(0, TAB_CONTAINER_WIDTH, TAB_CONTAINER_HEIGHT);
		this.mPaperDollTab.setButtonContainerSize(1, TAB_CONTAINER_WIDTH, TAB_CONTAINER_HEIGHT);
		this.mPaperDollTab.setButtonContainerSize(2, TAB_CONTAINER_WIDTH, TAB_CONTAINER_HEIGHT);
		local weaponFocusSize = this.mPaperDollTab.getSize();
		this.mAdvancedStatSection = this._createAdvancedStatsSection();
		this.mAdvancedStatSection.setPosition(0, basicStatsSize.height + weaponFocusSize.height + 2);
		local toolbarSize = this.getPreferredSize();
		this.setSize(basicStatsSize.width + this.mPaperDollTab.getSize().width, toolbarSize.height + weaponFocusSize.height + 4);
		this.setContentPane(this.mEQContainer);
		::_Connection.addListener(this);
		::_ItemDataManager.addListener(this);
		this.mBroadcaster = this.MessageBroadcaster();
		local eqInventory = ::_ItemDataManager.getContents("eq");
		this.mConstructed = true;
		::_sceneObjectManager.addListener(this);
		::_AbilityManager.addAbilityListener(this);

		if (!::_AbilityManager.isOwnageRetrieved())
		{
			::_AbilityManager.getAbilityOwnageList();
		}

		this.setCached(::Pref.get("video.UICache"));
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Frame._removeNotify();
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function checkStatBuffOrDebuff( statID )
	{
		local statusModifiers = ::_avatar.getStatusModifiers();
		local result = this.STAT_NORMAL;

		foreach( modifier in statusModifiers )
		{
			if (modifier.getStatID() == statID)
			{
				if (modifier.getAmount() > 0)
				{
					result = this.STAT_BUFFED;
				}
				else if (modifier.getAmount() < 0)
				{
					result = this.STAT_DEBUFFED;
					return result;
				}
			}
		}

		return result;
	}

	function fillOutEquipmentScreen()
	{
		this._updateName();
		this._updateClassLevel();
		this._updateHealth();
		this._updateStrength();
		this._updateDexterity();
		this._updateConstitution();
		this._updatePsyche();
		this._updateSpirit();
		this._updateMeleeAR();
		this._updateFireAR();
		this._updateMysticAR();
		this._updateFrostAR();
		this._updateDeathAR();
		this._updateToHitMelee();
		this._updateToHitMagic();
		this._updateCritMelee();
		this._updateCritMagic();
		this._updateAttackSpeed();
		this._updateCastSpeed();
		this._updateMeleeDPS();
		this._updateCastSpeed();
		this._updateHealingDPS();
		this._updateRangedDPS();
		this._updateDodgeRate();
		this._updateMagicDeflect();
		this._updateParryRate();
		this._updateBlockRate();
		this._updateRunSpeed();
		this._updateRegenHealth();
	}

	function findItemOwner( id )
	{
		if (this.mEQAmulet.getActionInSlot(0) && this.mEQAmulet.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQAmulet;
		}
		else if (this.mEQArms.getActionInSlot(0) && this.mEQArms.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQArms;
		}
		else if (this.mEQRingLeft.getActionInSlot(0) && this.mEQRingLeft.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQRingLeft;
		}
		else if (this.mEQHead.getActionInSlot(0) && this.mEQHead.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQHead;
		}
		else if (this.mEQNeck.getActionInSlot(0) && this.mEQNeck.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQNeck;
		}
		else if (this.mEQChest.getActionInSlot(0) && this.mEQChest.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQChest;
		}
		else if (this.mEQWaist.getActionInSlot(0) && this.mEQWaist.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQWaist;
		}
		else if (this.mEQLegs.getActionInSlot(0) && this.mEQLegs.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQLegs;
		}
		else if (this.mEQFeet.getActionInSlot(0) && this.mEQFeet.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQFeet;
		}
		else if (this.mEQShoulders.getActionInSlot(0) && this.mEQShoulders.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQShoulders;
		}
		else if (this.mEQHands.getActionInSlot(0) && this.mEQHands.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQHands;
		}
		else if (this.mEQRingRight.getActionInSlot(0) && this.mEQRingRight.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQRingRight;
		}
		else if (this.mEQMainHand.getActionInSlot(0) && this.mEQMainHand.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQMainHand;
		}
		else if (this.mEQOffHand.getActionInSlot(0) && this.mEQOffHand.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQOffHand;
		}
		else if (this.mEQRanged.getActionInSlot(0) && this.mEQRanged.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQRanged;
		}
		else if (this.mEQRedCharm.getActionInSlot(0) && this.mEQRedCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQRedCharm;
		}
		else if (this.mEQOrangeCharm.getActionInSlot(0) && this.mEQOrangeCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQOrangeCharm;
		}
		else if (this.mEQPurpleCharm.getActionInSlot(0) && this.mEQPurpleCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQPurpleCharm;
		}
		else if (this.mEQYellowCharm.getActionInSlot(0) && this.mEQYellowCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQYellowCharm;
		}
		else if (this.mEQGreenCharm.getActionInSlot(0) && this.mEQGreenCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQGreenCharm;
		}
		else if (this.mEQBlueCharm.getActionInSlot(0) && this.mEQBlueCharm.getActionInSlot(0).getItemId() == id)
		{
			return this.mEQBlueCharm;
		}

		return null;
	}

	function findMatchingContainer( containerSlot )
	{
		switch(containerSlot)
		{
		case this.ItemEquipSlot.ARMOR_HEAD:
			return this.mEQHead;
			break;

		case this.ItemEquipSlot.ARMOR_NECK:
			return this.mEQNeck;
			break;

		case this.ItemEquipSlot.ARMOR_CHEST:
			return this.mEQChest;
			break;

		case this.ItemEquipSlot.ARMOR_ARMS:
			return this.mEQArms;
			break;

		case this.ItemEquipSlot.ARMOR_HANDS:
			return this.mEQHands;
			break;

		case this.ItemEquipSlot.ARMOR_WAIST:
			return this.mEQWaist;
			break;

		case this.ItemEquipSlot.ARMOR_LEGS:
			return this.mEQLegs;
			break;

		case this.ItemEquipSlot.ARMOR_FEET:
			return this.mEQFeet;
			break;

		case this.ItemEquipSlot.ARMOR_SHOULDER:
			return this.mEQShoulders;
			break;

		case this.ItemEquipSlot.WEAPON_MAIN_HAND:
			return this.mEQMainHand;
			break;

		case this.ItemEquipSlot.WEAPON_OFF_HAND:
			return this.mEQOffHand;
			break;

		case this.ItemEquipSlot.WEAPON_RANGED:
			return this.mEQRanged;
			break;

		case this.ItemEquipSlot.ARMOR_RING_L:
			return this.mEQRingLeft;
			break;

		case this.ItemEquipSlot.ARMOR_RING_R:
			return this.mEQRingRight;
			break;

		case this.ItemEquipSlot.ARMOR_AMULET:
			return this.mEQAmulet;
			break;

		case this.ItemEquipSlot.COSMETIC_SHOULDER_L:
			return this.mEQLeftShoulder;
			break;

		case this.ItemEquipSlot.COSMETIC_HIP_L:
			return this.mEQLeftHip;
			break;

		case this.ItemEquipSlot.COSMETIC_SHOULDER_R:
			return this.mEQRightShoulder;
			break;

		case this.ItemEquipSlot.COSMETIC_HIP_R:
			return this.mEQRightHip;
			break;

		case this.ItemEquipSlot.RED_CHARM:
			return this.mEQRedCharm;
			break;

		case this.ItemEquipSlot.BLUE_CHARM:
			return this.mEQBlueCharm;
			break;

		case this.ItemEquipSlot.GREEN_CHARM:
			return this.mEQGreenCharm;
			break;

		case this.ItemEquipSlot.ORANGE_CHARM:
			return this.mEQOrangeCharm;
			break;

		case this.ItemEquipSlot.YELLOW_CHARM:
			return this.mEQYellowCharm;
			break;

		case this.ItemEquipSlot.PURPLE_CHARM:
			return this.mEQPurpleCharm;
			break;

		case this.ItemEquipSlot.CONTAINER_0:
		case this.ItemEquipSlot.CONTAINER_1:
		case this.ItemEquipSlot.CONTAINER_2:
		case this.ItemEquipSlot.CONTAINER_3:
			local invScreen = this.Screens.get("Inventory", true);

			if (invScreen)
			{
				local bagContainer = invScreen.getBagActionContainer();
				return bagContainer;
				break;
			}

		default:
			return null;
			break;
		}
	}

	function findMatchingContainerGivenName( containerName )
	{
		switch(containerName)
		{
		case "eq_head":
			return this.mEQHead;
			break;

		case "eq_neck":
			return this.mEQNeck;
			break;

		case "eq_chest":
			return this.mEQChest;
			break;

		case "eq_arms":
			return this.mEQArms;
			break;

		case "eq_hands":
			return this.mEQHands;
			break;

		case "eq_waist":
			return this.mEQWaist;
			break;

		case "eq_legs":
			return this.mEQLegs;
			break;

		case "eq_feet":
			return this.mEQFeet;
			break;

		case "eq_shoulders":
			return this.mEQShoulders;
			break;

		case "eq_main_hand":
			return this.mEQMainHand;
			break;

		case "eq_off_hand":
			return this.mEQOffHand;
			break;

		case "eq_ranged":
			return this.mEQRanged;
			break;

		case "eq_ring_l":
			return this.mEQRingLeft;
			break;

		case "eq_ring_r":
			return this.mEQRingRight;
			break;

		case "eq_amulet":
			return this.mEQAmulet;
			break;

		case "eq_cosmetic_shoulder_l":
			return this.mEQLeftShoulder;
			break;

		case "eq_cosmetic_hip_l":
			return this.mEQLeftHip;
			break;

		case "eq_cosmetic_shoulder_r":
			return this.mEQRightShoulder;
			break;

		case "eq_cosmetic_hip_r":
			return this.mEQRightHip;
			break;

		case "eq_red_charm":
			return this.mEQRedCharm;
			break;

		case "eq_blue_charm":
			return this.mEQBlueCharm;
			break;

		case "eq_green_charm":
			return this.mEQGreenCharm;
			break;

		case "eq_orange_charm":
			return this.mEQOrangeCharm;
			break;

		case "eq_yellow_charm":
			return this.mEQYellowCharm;
			break;

		case "eq_purple_charm":
			return this.mEQPurpleCharm;
			break;

		default:
			return null;
			break;
		}
	}

	function getActionButtonEquipSlot( item )
	{
		local containerSlots = this.EquipmentMapContainer[item.getEquipmentType()];
		local lastGoodActionSlot;
		local showErrorMessage = true;
		local previousContainer;

		foreach( container in containerSlots )
		{
			local actionContainer = this.findMatchingContainer(container);

			if (actionContainer && previousContainer != actionContainer)
			{
				foreach( key, actionButtonSlot in actionContainer.getAllActionButtonSlots() )
				{
					if (actionButtonSlot)
					{
						if (this.isValidDropSlot(actionButtonSlot, item.getItemDefId(), showErrorMessage))
						{
							local action = actionContainer.getActionInSlot(key);

							if (action && "mItemData" in action)
							{
								if (lastGoodActionSlot == null)
								{
									lastGoodActionSlot = actionButtonSlot;
								}
							}
							else
							{
								return actionButtonSlot;
							}
						}

						showErrorMessage = false;
					}
				}

				previousContainer = actionContainer;
			}
		}

		return lastGoodActionSlot;
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (creatureId == ::_avatar.getID() && containerName == "eq")
		{
			this.mEQRedCharm.removeAllActions();
			this.mEQOrangeCharm.removeAllActions();
			this.mEQPurpleCharm.removeAllActions();
			this.mEQYellowCharm.removeAllActions();
			this.mEQGreenCharm.removeAllActions();
			this.mEQBlueCharm.removeAllActions();

			foreach( itemId in container.mContents )
			{
				local item = ::_ItemManager.getItem(itemId);
				local actionContainer = this.findMatchingContainer(item.mItemData.mContainerSlot);

				if (actionContainer)
				{
					actionContainer.addAction(item, true, 0);
				}
			}

			this._updateQuickbarWithEquipment();
			this.fillOutEquipmentScreen();
			this.mBroadcaster.broadcastMessage("onEquipContainerUpdated", this);
		}
	}

	function onAttemptedItemDisown( actionContainer, action, slotIndex )
	{
		if (actionContainer)
		{
			local callback = {
				currAction = action,
				slotNum = slotIndex,
				currActionContainer = actionContainer,
				function onActionSelected( mb, alt )
				{
					if (alt == "Yes")
					{
						if (this._Connection)
						{
							this._Connection.sendQuery("item.delete", this, [
								this.currAction.getItemId()
							]);
							this.currActionContainer._removeActionButton(this.slotNum);
							this._quickBarManager.removeItemFromQuickbar(this.currAction);
						}
					}
				}

			};
			this.GUI.MessageBox.showYesNo("This item will be destroyed.  Are you sure you want to destroy it? ", callback);
		}
	}

	function onDragOver( evt )
	{
		if (!this._getDroppedComponent(evt))
		{
			return;
		}

		evt.acceptDrop(this.GUI.DnDEvent.ACTION_MOVE);
		evt.consume();
	}

	function onDrop( evt )
	{
		local transferable = evt.getTransferable();

		if (typeof transferable != "instance" || !(transferable instanceof this.GUI.ActionButton))
		{
			return;
		}

		local dragItem = transferable.getAction();

		if (dragItem)
		{
			local actionButtonSlot = this.getActionButtonEquipSlot(dragItem);

			if (actionButtonSlot)
			{
				actionButtonSlot.onDrop(evt);
			}
		}

		evt.consume();
	}

	function onStatUpdated( sceneObject, statID, value )
	{
		if (sceneObject != ::_avatar)
		{
			return;
		}

		switch(statID)
		{
		case this.Stat.LEVEL:
		case this.Stat.PROFESSION:
			this._updateClassLevel();
			this._updateRegenHealth();
			break;

		case this.Stat.DISPLAY_NAME:
			this._updateName();
			break;

		case this.Stat.STRENGTH:
			this._updateStrength();
			break;

		case this.Stat.DEXTERITY:
			this._updateDexterity();
			break;

		case this.Stat.CONSTITUTION:
			this._updateConstitution();
			break;

		case this.Stat.PSYCHE:
			this._updatePsyche();
			break;

		case this.Stat.SPIRIT:
			this._updateSpirit();
			break;

		case this.Stat.MELEE_ATTACK_SPEED:
		case this.Stat.MOD_ATTACK_SPEED:
			this._updateAttackSpeed();
			break;

		case this.Stat.MAGIC_ATTACK_SPEED:
		case this.Stat.MOD_CASTING_SPEED:
			this._updateCastSpeed();
			break;

		case this.Stat.BASE_DAMAGE_MELEE:
			this._updateMeleeDPS();
			break;

		case this.Stat.DAMAGE_RESIST_MELEE:
		case this.Stat.DR_MOD_MELEE:
			this._updateMeleeAR();
			break;

		case this.Stat.DAMAGE_RESIST_FIRE:
		case this.Stat.DR_MOD_FIRE:
			this._updateFireAR();
			break;

		case this.Stat.DAMAGE_RESIST_FROST:
		case this.Stat.DR_MOD_FROST:
			this._updateFrostAR();
			break;

		case this.Stat.DAMAGE_RESIST_MYSTIC:
		case this.Stat.DR_MOD_MYSTIC:
			this._updateMysticAR();
			break;

		case this.Stat.DAMAGE_RESIST_DEATH:
		case this.Stat.DR_MOD_DEATH:
			this._updateDeathAR();
			break;

		case this.Stat.BASE_MOVEMENT:
		case this.Stat.MOD_MOVEMENT:
			this._updateRunSpeed();
			break;

		case this.Stat.BASE_DODGE:
			this._updateDodgeRate();
			break;

		case this.Stat.BASE_DEFLECT:
			this._updateMagicDeflect();
			break;

		case this.Stat.BASE_PARRY:
		case this.Stat.MOD_PARRY:
			this._updateParryRate();
			break;

		case this.Stat.BASE_BLOCK:
		case this.Stat.MOD_BLOCK:
			this._updateBlockRate();
			break;

		case this.Stat.BASE_MELEE_TO_HIT:
		case this.Stat.MOD_MELEE_TO_HIT:
			this._updateToHitMelee();
			break;

		case this.Stat.BASE_MELEE_CRITICAL:
		case this.Stat.MOD_MELEE_TO_CRIT:
			this._updateCritMelee();
			break;

		case this.Stat.BASE_MAGIC_SUCCESS:
		case this.Stat.MOD_MAGIC_TO_HIT:
			this._updateToHitMagic();
			break;

		case this.Stat.BASE_MAGIC_CRITICAL:
		case this.Stat.MOD_MAGIC_TO_CRIT:
			this._updateCritMagic();
			break;

		case this.Stat.BASE_HEALING:
		case this.Stat.MOD_HEALING:
			this._updateHealingDPS();
			break;

		case this.Stat.HEALTH:
		case this.Stat.HEALTH_MOD:
			this._updateHealth();
			break;

		case this.Stat.WEAPON_DAMAGE_2H:
		case this.Stat.WEAPON_DAMAGE_1H:
		case this.Stat.WEAPON_DAMAGE_POLE:
		case this.Stat.WEAPON_DAMAGE_SMALL:
			this._updateMeleeDPS();
			break;

		default:
			break;
		}
	}

	function onTabSelected( tabPane, tab )
	{
		if (tab.index == 1)
		{
			::_tutorialManager.onScreenOpened("Charms");
		}
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		local item = container.getSlotContents(slotIndex);
		local itemAction = item.getActionButton().getAction();
		local itemID = itemAction.mItemId;
		local serverIndex = container.getSlotsRestriction(0);
		local previousSlotContainer = oldSlotsButton.getPreviousActionContainer();
		local oldActionButtonSlot = oldSlotsButton.getActionButtonSlot();
		local oldSlotContainerName = "";
		local oldSlotIndex = 0;

		if (previousSlotContainer && oldActionButtonSlot)
		{
			oldSlotContainerName = previousSlotContainer.getContainerName();
			oldSlotIndex = previousSlotContainer.getIndexOfSlot(oldActionButtonSlot);
		}

		if (serverIndex < 0)
		{
			return;
		}

		if ("mItemDefData" in itemAction)
		{
			local itemDefData = itemAction.mItemDefData;
			local weaponType = itemDefData.getWeaponType();

			if (weaponType == this.WeaponType.TWO_HAND)
			{
				local inventory = this.Screens.get("Inventory", true);

				if (inventory)
				{
					local offhandButton = this.mEQOffHand.getActionButtonFromIndex(0);

					if (offhandButton != null)
					{
						local inventoryAC = inventory.getMyActionContainer();
						inventoryAC.simulateButtonDrop(offhandButton);
					}
				}
			}
		}

		if (item.getSwapped() == true)
		{
			item.setSwapped(false);
		}
		else
		{
			local queryArgument = [];
			queryArgument.append(itemID);
			queryArgument.append("eq");
			queryArgument.append(serverIndex);

			if (::_Connection.getProtocolVersionId() >= 19)
			{
				queryArgument.append(oldSlotContainerName);
				queryArgument.append(oldSlotIndex);
			}

			this._Connection.sendQuery("item.move", this, queryArgument);
		}
	}

	function onItemDefUpdated( itemDefId, itemDef )
	{
		if (!itemDef.isValid())
		{
			return;
		}

		local containersToCheck = [];

		switch(itemDef.mEquipType)
		{
		case this.ItemEquipType.NONE:
			return;
			break;

		case this.ItemEquipType.WEAPON_1H:
			containersToCheck.append(this.mEQMainHand);
			containersToCheck.append(this.mEQOffHand);
			break;

		case this.ItemEquipType.WEAPON_1H_MAIN:
			containersToCheck.append(this.mEQMainHand);
			break;

		case this.ItemEquipType.WEAPON_1H_UNIQUE:
			containersToCheck.append(this.mEQMainHand);
			containersToCheck.append(this.mEQOffHand);
			break;

		case this.ItemEquipType.WEAPON_1H_OFF:
			containersToCheck.append(this.mEQOffHand);
			break;

		case this.ItemEquipType.WEAPON_2H:
			containersToCheck.append(this.mEQMainHand);
			break;

		case this.ItemEquipType.WEAPON_RANGED:
			containersToCheck.append(this.mEQRanged);
			break;

		case this.ItemEquipType.ARMOR_SHIELD:
			containersToCheck.append(this.mEQOffHand);
			break;

		case this.ItemEquipType.ARMOR_HEAD:
			containersToCheck.append(this.mEQHead);
			break;

		case this.ItemEquipType.ARMOR_NECK:
			containersToCheck.append(this.mEQNeck);
			break;

		case this.ItemEquipType.ARMOR_SHOULDER:
			containersToCheck.append(this.mEQShoulders);
			break;

		case this.ItemEquipType.ARMOR_CHEST:
			containersToCheck.append(this.mEQChest);
			break;

		case this.ItemEquipType.ARMOR_ARMS:
			containersToCheck.append(this.mEQArms);
			break;

		case this.ItemEquipType.ARMOR_HANDS:
			containersToCheck.append(this.mEQHands);
			break;

		case this.ItemEquipType.ARMOR_WAIST:
			containersToCheck.append(this.mEQWaist);
			break;

		case this.ItemEquipType.ARMOR_LEGS:
			containersToCheck.append(this.mEQLegs);
			break;

		case this.ItemEquipType.ARMOR_FEET:
			containersToCheck.append(this.mEQFeet);
			break;

		case this.ItemEquipType.ARMOR_RING:
			containersToCheck.append(this.mEQRingLeft);
			containersToCheck.append(this.mEQRingRight);
			break;

		case this.ItemEquipType.ARMOR_RING_UNIQUE:
			containersToCheck.append(this.mEQRingLeft);
			containersToCheck.append(this.mEQRingRight);
			break;

		case this.ItemEquipType.ARMOR_AMULET:
			containersToCheck.append(this.mEQAmulet);
			break;

		case this.ItemEquipType.RED_CHARM:
			containersToCheck.append(this.mEQRedCharm);
			break;

		case this.ItemEquipType.GREEN_CHARM:
			containersToCheck.append(this.mEQGreenCharm);
			break;

		case this.ItemEquipType.BLUE_CHARM:
			containersToCheck.append(this.mEQBlueCharm);
			break;

		case this.ItemEquipType.ORANGE_CHARM:
			containersToCheck.append(this.mEQOrangeCharm);
			break;

		case this.ItemEquipType.YELLOW_CHARM:
			containersToCheck.append(this.mEQYellowCharm);
			break;

		case this.ItemEquipType.PURPLE_CHARM:
			containersToCheck.append(this.mEQPurpleCharm);
			break;
		}

		local updateStats = false;

		foreach( container in containersToCheck )
		{
			local action = container.getActionInSlot(0);

			if (action && ("mItemDefId" in action) && action.mItemDefId == itemDefId)
			{
				updateStats = true;
			}
		}

		if (updateStats)
		{
			this.fillOutEquipmentScreen();
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "item.move")
		{
		}
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);

		if (qa.args.len() < 5)
		{
			return;
		}

		if (qa.query == "item.move")
		{
			::Util.handleMoveItemBack(qa);
		}
		else
		{
		}
	}

	function onUpdateBuyAbility( ability )
	{
		if (this.mHasDisabledEquipment)
		{
			foreach( abilityType in ::AbilityIdType )
			{
				if (abilityType == ability.getId())
				{
					this.onAbilityOwnageUpdate();
					break;
				}
			}
		}
	}

	function highlightSlots( actionButton )
	{
		this.mDragItem = actionButton.getAction();

		if (this.mDragItem)
		{
			this.setEquipSlotHighlight(this.mDragItem, true);
		}
	}

	function handleClearHighlight()
	{
		if (this.mDragItem)
		{
			this.setEquipSlotHighlight(this.mDragItem, false);
			this.mDragItem = null;
		}
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function setContainerMoveProperties( container )
	{
		container.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		container.addMovingToProperties(container.getContainerName(), this.MoveToProperties(this.MovementTypes.MOVE));
		container.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.CLONE));
		container.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		container.addAcceptingFromProperties(container.getContainerName(), this.AcceptFromProperties(this));
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		local itemData = action.mItemData;

		if (::_avatar && ::_avatar.isDead())
		{
			this.IGIS.error("You cannot equip items when you are dead.");
			return false;
		}

		return this.isValidDropSlot(newSlot, itemData.mItemDefId);
	}

	function isValidDropSlot( newSlot, itemDefId, ... )
	{
		local displayError = true;
		local testOnly = false;

		if (vargc > 0)
		{
			displayError = vargv[0];

			if (vargc > 1)
			{
				testOnly = vargv[1];
			}
		}

		local avLevel = ::_avatar.getStat(this.Stat.LEVEL);
		local itemdef = ::_ItemDataManager.getItemDef(itemDefId);

		if (!itemdef)
		{
			if (displayError)
			{
				this.IGIS.error("The item you are trying to equip is invalid");
			}

			return false;
		}

		local itemLevel = itemdef.getMinUseLevel();
		local cr = itemdef.getClassRestrictions();
		local profession = ::_avatar.getStat(this.Stat.PROFESSION, true);

		if (avLevel < itemLevel)
		{
			if (displayError)
			{
				this.IGIS.error("You need to be level " + itemLevel.tostring() + " to equip this item");
			}

			return false;
		}

		local matches = false;

		switch(profession)
		{
		case this.Professions.KNIGHT:
			matches = ("knight" in cr) && cr.knight;
			break;

		case this.Professions.ROGUE:
			matches = ("rogue" in cr) && cr.rogue;
			break;

		case this.Professions.MAGE:
			matches = ("mage" in cr) && cr.mage;
			break;

		case this.Professions.DRUID:
			matches = ("druid" in cr) && cr.druid;
			break;

		default:
			if (displayError)
			{
				this.IGIS.error("Your profession is wrong, consult an especialist");
			}

			return false;
		}

		if (!matches)
		{
			if (displayError)
			{
				this.IGIS.error("This item cannot be equipped by " + this.Professions[profession].name + "s");
			}

			return false;
		}

		local er = newSlot.getEquipmentRestriction();
		local wt = itemdef.getWeaponType();
		local armorType = itemdef.getArmorType();

		if (wt != this.WeaponType.NONE && wt != this.WeaponType.ARCANE_TOTEM && er == this.ItemEquipSlot.WEAPON_OFF_HAND && profession != this.Professions.ROGUE)
		{
			if (displayError)
			{
				this.IGIS.error("Only Rogues can equip weapons in the off hand.");
			}

			return false;
		}

		local hasMainHandWeapon = false;
		local hasOffHandWeapon = false;
		local hasTwoHandedMainHandWeapon = false;
		local mhc = this.findMatchingContainer(this.ItemEquipSlot.WEAPON_MAIN_HAND);

		if (mhc)
		{
			local ac = mhc.getActionInSlot(0);

			if ("mItemData" in ac)
			{
				local aid = ac.mItemData;
				local aidef = ::_ItemDataManager.getItemDef(aid.mItemDefId);
				local mwt = aidef.getWeaponType();

				if (mwt == this.WeaponType.TWO_HAND)
				{
					hasTwoHandedMainHandWeapon = true;
				}

				hasMainHandWeapon = true;
			}
		}

		local ohc = this.findMatchingContainer(this.ItemEquipSlot.WEAPON_OFF_HAND);

		if (ohc)
		{
			local ac = ohc.getActionInSlot(0);

			if ("mItemData" in ac)
			{
				hasOffHandWeapon = true;
			}
		}

		local equipType = itemdef.getEquipType();

		if (!testOnly)
		{
			if (wt == this.WeaponType.TWO_HAND)
			{
				if (hasOffHandWeapon && hasMainHandWeapon)
				{
					local inventory = this.Screens.get("Inventory", true);

					if (inventory)
					{
						if (inventory.getFreeSlotsRemaining() < 1)
						{
							if (displayError)
							{
								this.IGIS.error("No room in your inventory for both hand items.");
							}

							return false;
						}
					}
					else
					{
						return false;
					}
				}
			}
			else if (er == this.ItemEquipSlot.WEAPON_OFF_HAND && wt == this.WeaponType.ARCANE_TOTEM)
			{
				if (hasTwoHandedMainHandWeapon)
				{
					if (displayError)
					{
						this.IGIS.error("You cannot equip a talisman with a 2-handed weapon.");
					}

					return false;
				}
			}
			else if (armorType == this.ArmorType.SHIELD)
			{
				if (hasTwoHandedMainHandWeapon)
				{
					if (displayError)
					{
						this.IGIS.error("You cannot equip a shield with a 2-handed weapon.");
					}

					return false;
				}
			}
		}

		return true;
	}

	function setAttackSpeed( atkSpeed, status )
	{
		if (null == atkSpeed)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mAttackSpeed, atkSpeed.tostring() + "%");
	}

	function setBlockRate( blockRate, status )
	{
		if (null == blockRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mBlock, blockRate.tostring() + "%");
	}

	function setCastingSpeed( castSpeed, status )
	{
		if (null == castSpeed)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mCastingSpeed, castSpeed.tostring() + "%");
	}

	function setConstitution( conCur, conMax, status )
	{
		if (null == conCur || null == conMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mConstitution, conCur.tostring() + "(" + conMax.tostring() + ")");
	}

	function setDeathArmorRating( ar, status )
	{
		if (null == ar)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mDeathArmorRating, ar.tostring());
	}

	function setDeathResist( dr, status )
	{
		if (null == dr)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mDeathResist, dr.tostring() + "%");
	}

	function setDexterity( dexCur, dexMax, status )
	{
		if (null == dexCur || null == dexMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mDexterity, dexCur.tostring() + "(" + dexMax.tostring() + ")");
	}

	function setDodgeRate( dodgeRate, status )
	{
		if (null == dodgeRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mDodge, dodgeRate.tostring() + "%");
	}

	function setEquipSlotHighlight( item, isVisible )
	{
		local equipType = item.getEquipmentType();
		local containerSlots = this.EquipmentMapContainer[equipType];
		local myMap = this.deepClone(this.EquipmentMapContainer);
		local previousContainer;

		foreach( container in containerSlots )
		{
			local actionContainer = this.findMatchingContainer(container);

			if (actionContainer && previousContainer != actionContainer)
			{
				foreach( key, actionButtonSlot in actionContainer.getAllActionButtonSlots() )
				{
					if (actionButtonSlot)
					{
						if (this.isValidDropSlot(actionButtonSlot, item.getItemDefId(), false, true))
						{
							actionButtonSlot.setHighlightOverlay(isVisible);
						}
					}
				}

				previousContainer = actionContainer;
			}
		}
	}

	function setFireArmorRating( ar, status )
	{
		if (null == ar)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mFireArmorRating, ar.tostring());
	}

	function setFireDPS( DPS, status )
	{
		if (null == DPS)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mFireDPS, DPS.tostring());
	}

	function setFireResist( fr, status )
	{
		if (null == fr)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mFireResist, fr.tostring() + "%");
	}

	function setFrostArmorRating( ar, status )
	{
		if (null == ar)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mFrostArmorRating, ar.tostring());
	}

	function setRangedDPS( dps, status )
	{
		if (null == dps)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mRangedDPS, dps.tostring());
	}

	function setFrostResist( fr, status )
	{
		if (null == fr)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mFrostResist, fr.tostring() + "%");
	}

	function setHealingDPS( DPS, status )
	{
		if (null == DPS)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mHealingDPS, DPS.tostring());
	}

	function setHealth( healthCur, healthMax, status )
	{
		if (null == healthCur || null == healthMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mHealth, healthCur.tostring() + "/" + healthMax.tostring());
	}

	function setLevelClass( level, charClass )
	{
		if (null == level || null == charClass)
		{
			return;
		}

		this.mLevelClass.setText("Level " + level.tostring() + " " + charClass);
	}

	function setParryRate( parryRate, status )
	{
		if (null == parryRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mParry, parryRate.tostring() + "%");
	}

	function setPsyche( psyCur, psyMax, status )
	{
		if (null == psyCur || null == psyMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mPsyche, psyCur.tostring() + "(" + psyMax.tostring() + ")");
	}

	function setMagicCritRate( critRate, status )
	{
		if (null == critRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMagicCritRate, critRate.tostring() + "%");
	}

	function setMagicHitRate( hitRate, status )
	{
		if (null == hitRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mToHitMagic, hitRate.tostring() + "%");
	}

	function setMeleeArmorRating( ar, status )
	{
		if (null == ar)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMeleeArmorRating, ar.tostring());
	}

	function setMeleeCritRate( critRate, status )
	{
		if (null == critRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMeleeCritRate, critRate.tostring() + "%");
	}

	function setMeleeDPS( DPS, status )
	{
		if (null == DPS)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMeleeDPS, DPS.tostring());
	}

	function setMeleeHitRate( hitRate, status )
	{
		if (null == hitRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mToHitMelee, hitRate.tostring() + "%");
	}

	function setMeleeResist( mr, status )
	{
		if (null == mr)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMeleeResist, mr.tostring() + "%");
	}

	function setMysticArmorRating( ar, status )
	{
		if (null == ar)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMysticArmorRating, ar.tostring());
	}

	function setMysticResist( mr, status )
	{
		if (null == mr)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMysticResist, mr.tostring() + "%");
	}

	function setName( name )
	{
		if (null == name)
		{
			return;
		}

		this.setTitle(name);
	}

	function setHealthRegenRate( regenRate, status )
	{
		if (null == regenRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mHealthRegen, regenRate.tostring() + "/sec");
	}

	function setMagicDeflectRate( deflectRate, status )
	{
		if (null == deflectRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mMagicDeflect, deflectRate.tostring() + "%");
	}

	function setRunSpeed( runSpeed, status )
	{
		if (null == runSpeed)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mRunSpeed, runSpeed.tostring() + "%");
	}

	function setSpirit( spiCur, spiMax, status )
	{
		if (null == spiCur || null == spiMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mSpirit, spiCur.tostring() + "(" + spiMax.tostring() + ")");
	}

	function setStrength( strCur, strMax, status )
	{
		if (null == strCur || null == strMax)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mStrength, strCur.tostring() + "(" + strMax.tostring() + ")");
	}

	function setTextColorBasedOnStatus( status, label, text )
	{
		label.setText(text);

		if (this.STAT_BUFFED == status)
		{
			label.setFontColor(this.Colors.green);
		}
		else if (this.STAT_NORMAL == status)
		{
			label.setFontColor(this.Colors["GUI Gold"]);
		}
		else if (this.STAT_DEBUFFED == status)
		{
			label.setFontColor(this.Colors.red);
		}
	}

	function setVisible( value )
	{
		this.GUI.Panel.setVisible(value);

		if (this.mConstructed && value)
		{
			this.fillOutEquipmentScreen();
		}

		if (value)
		{
			::Audio.playSound("Sound-EquipmentOpen.ogg");
		}
		else
		{
			::Audio.playSound("Sound-EquipmentClose.ogg");
		}
	}

	function setWillRegenRate( regenRate, status )
	{
		if (null == regenRate)
		{
			return;
		}

		this.setTextColorBasedOnStatus(status, this.mWillRegen, regenRate.tostring() + "/sec");
	}

	function toggleAdvancedStats( evt )
	{
		local advanceStatSize = this.mAdvancedStatSection.getSize();
		local currentWindowSize = this.getSize();

		if (this.mShowingAdvancedStats)
		{
			this.setSize(currentWindowSize.width, currentWindowSize.height - advanceStatSize.height - 3);
			this.mEQContainer.remove(this.mAdvancedStatSection);
			this.mAdvanceStatButton.setToggled(false);
		}
		else
		{
			this.setSize(currentWindowSize.width, currentWindowSize.height + advanceStatSize.height + 3);
			this.mAdvancedStatSection.setPosition(-30, currentWindowSize.height - 55);
			this.mEQContainer.add(this.mAdvancedStatSection, this.GUI.BorderLayout.SOUTH);
			this.mAdvanceStatButton.setToggled(true);
		}

		this.mShowingAdvancedStats = !this.mShowingAdvancedStats;
	}

	function toggleDefense( evt )
	{
		this.mAdvancedStatSection.add(this.mDefensiveStatSection);
		this.mAdvancedStatSection.remove(this.mOffensiveStatSection);
		this.mDefenseButton.setToggled(true);
		this.mOffenseButton.setToggled(false);
	}

	function toggleOffense( evt )
	{
		this.mAdvancedStatSection.remove(this.mDefensiveStatSection);
		this.mAdvancedStatSection.add(this.mOffensiveStatSection);
		this.mDefenseButton.setToggled(false);
		this.mOffenseButton.setToggled(true);
	}

	function _createAdvancedStatsSection()
	{
		local advancedTabs = this.GUI.TabbedPane(true);
		advancedTabs.setSize(353, 115);
		advancedTabs.setTabPlacement("left");
		advancedTabs.setContentFrameAppearance("InnerPanel");
		advancedTabs.addTab("", this._createOffenseAdvancedStat());
		advancedTabs.addTab(" ", this._createDefenseAdvancedStat());
		advancedTabs.setButtonContainerInset(0, 0.5, 0, 0, 3.5);
		advancedTabs.setButtonContainerInset(1, 0.5, 0, 0, 3.5);
		advancedTabs.setButtonContainerAppearance(0, "TabBar/TabContainer/Round/Left");
		advancedTabs.setButtonContainerAppearance(1, "TabBar/TabContainer/Round/Left");
		advancedTabs.setButtonAppearance(0, "TabBar/TabButton/Inactive/Offense/", "TabBar/TabButton/Inactive/Offense/");
		advancedTabs.setButtonAppearance(1, "TabBar/TabButton/Inactive/Defense/", "TabBar/TabButton/Inactive/Defense/");
		advancedTabs.setButtonContainerTooltip(0, "Offense");
		advancedTabs.setButtonContainerTooltip(1, "Defense");
		local TAB_SIZE_WIDTH = 31;
		local TAB_SIZE_HEIGHT = 31;
		advancedTabs.setButtonSize(0, TAB_SIZE_WIDTH, TAB_SIZE_HEIGHT);
		advancedTabs.setButtonSize(1, TAB_SIZE_WIDTH, TAB_SIZE_HEIGHT);
		local TAB_CONTAINER_WIDTH = 38;
		local TAB_CONTAINER_HEIGHT = 42;
		advancedTabs.setButtonContainerSize(0, TAB_CONTAINER_WIDTH, TAB_CONTAINER_HEIGHT);
		advancedTabs.setButtonContainerSize(1, TAB_CONTAINER_WIDTH, TAB_CONTAINER_HEIGHT);
		return advancedTabs;
	}

	function _createAttachmentSection()
	{
		local container = this.GUI.Container(null);
		local attachmentSection = this.GUI.Container(null);
		attachmentSection.setSize(130, 245);
		attachmentSection.setAppearance("PaperDoll");
		this.mEQLeftShoulder = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.COSMETIC_SHOULDER_L], this.ItemEquipSlot.COSMETIC_SHOULDER_L, "EQ_Storage_64");
		this.mEQLeftShoulder.setPosition(10, 75);
		this.mEQRightShoulder = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.COSMETIC_SHOULDER_R], this.ItemEquipSlot.COSMETIC_SHOULDER_R, "EQ_Storage_64");
		this.mEQRightShoulder.setPosition(90, 75);
		this.mEQLeftHip = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.COSMETIC_HIP_L], this.ItemEquipSlot.COSMETIC_HIP_L, "EQ_Storage_64");
		this.mEQLeftHip.setPosition(10, 145);
		this.mEQRightHip = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.COSMETIC_HIP_R], this.ItemEquipSlot.COSMETIC_HIP_R, "EQ_Storage_64");
		this.mEQRightHip.setPosition(90, 145);
		attachmentSection.add(this.mEQLeftShoulder);
		attachmentSection.add(this.mEQRightShoulder);
		attachmentSection.add(this.mEQLeftHip);
		attachmentSection.add(this.mEQRightHip);
		container.add(attachmentSection);
		return container;
	}

	function _createBasicStatSection()
	{
		local hBarWidth = 170;
		local basicStats = this.GUI.Container(this.GUI.BoxLayoutV());
		basicStats.setInsets(0, 10, 0, 5);
		basicStats.getLayoutManager().setGap(-1.5);
		basicStats.setSize(180, 340);
		basicStats.setPreferredSize(180, 340);
		this.mLevelClass = this.GUI.Label("Level 34 Rogue");
		this.mLevelClass.setFont(this.GUI.Font("Maiandra", 26));
		basicStats.add(this.mLevelClass);
		local SLIDER_HEIGHT = 10;
		local sep = this.GUI.Spacer(hBarWidth, SLIDER_HEIGHT);
		sep.setAppearance("HSliderBar");
		basicStats.add(sep);
		this.mHealth = this.GUI.Label("9999 / 9999");
		basicStats.add(this._createLabelValueCombo("Health : ", this.mHealth));
		sep = this.GUI.Spacer(hBarWidth, SLIDER_HEIGHT);
		sep.setAppearance("HSliderBar");
		basicStats.add(sep);
		local statisticsLabel = this.GUI.Label("Statistics");
		statisticsLabel.setFont(this.GUI.Font("Maiandra", 26));
		basicStats.add(statisticsLabel);
		this.mStrength = this.GUI.Label("9999 (9999)");
		basicStats.add(this._createLabelValueComboRightAlign("Strength : ", this.mStrength));
		this.mDexterity = this.GUI.Label("9999 (9999)");
		basicStats.add(this._createLabelValueComboRightAlign("Dexterity : ", this.mDexterity));
		this.mConstitution = this.GUI.Label("9999 (9999)");
		basicStats.add(this._createLabelValueComboRightAlign("Constitution : ", this.mConstitution));
		this.mPsyche = this.GUI.Label("9999 (9999)");
		basicStats.add(this._createLabelValueComboRightAlign("Psyche : ", this.mPsyche));
		this.mSpirit = this.GUI.Label("9999 (9999)");
		basicStats.add(this._createLabelValueComboRightAlign("Spirit : ", this.mSpirit));
		sep = this.GUI.Spacer(hBarWidth, SLIDER_HEIGHT);
		sep.setAppearance("HSliderBar");
		basicStats.add(sep);
		local defenseLabel = this.GUI.Label("Defenses");
		defenseLabel.setFont(this.GUI.Font("Maiandra", 26));
		basicStats.add(defenseLabel);
		basicStats.add(this._createArmorRatingSection());
		sep = this.GUI.Spacer(150, 2);
		basicStats.add(sep);
		sep = this.GUI.Spacer(hBarWidth, SLIDER_HEIGHT);
		sep.setAppearance("HSliderBar");
		basicStats.add(sep);
		sep = this.GUI.Spacer(150, 2);
		basicStats.add(sep);
		return basicStats;
	}

	function _createButtonSection()
	{
		local buttonGrid = this.GUI.Container(this.GUI.GridLayout(3, 3));
		buttonGrid.setSize(150, 60);
		buttonGrid.getLayoutManager().setGaps(5, 0);
		buttonGrid.getLayoutManager().setRows(28, 32, 20);
		buttonGrid.getLayoutManager().setColumns(30, 150, 30);
		this.mAdvanceStatButton = this.GUI.NarrowButton("Advanced Stats");
		this.mAdvanceStatButton.setFixedSize(125, 32);
		this.mAdvanceStatButton.addActionListener(this);
		this.mAdvanceStatButton.setReleaseMessage("toggleAdvancedStats");
		return this.mAdvanceStatButton;
	}

	function _createDefenseAdvancedStat()
	{
		this.mDefensiveStatSection = this.GUI.Container(this.GUI.GridLayout(3, 1));
		this.mDefensiveStatSection.getLayoutManager().setRows(50, 10, "*");
		local topHalf = this.GUI.Container(this.GUI.GridLayout(3, 3));
		topHalf.getLayoutManager().setColumns(115, 20, 115);
		topHalf.getLayoutManager().setRows(15, 15, 15);
		this.mDodge = this.GUI.Label("999.99%");
		topHalf.add(this._createLabelValueCombo("Dodge", this.mDodge));
		topHalf.add(this.GUI.Spacer(1, 1));
		this.mMagicDeflect = this.GUI.Label("999.99%");
		topHalf.add(this._createLabelValueCombo("Magic Deflect", this.mMagicDeflect));
		this.mParry = this.GUI.Label("999.99%");
		topHalf.add(this._createLabelValueCombo("Parry", this.mParry));
		topHalf.add(this.GUI.Spacer(1, 1));
		this.mBlock = this.GUI.Label("999.99%");
		topHalf.add(this._createLabelValueCombo("Block", this.mBlock));
		this.mRunSpeed = this.GUI.Label("999.99%");
		topHalf.add(this._createLabelValueCombo("Run Speed", this.mRunSpeed));
		topHalf.add(this.GUI.Spacer(1, 1));
		this.mHealthRegen = this.GUI.Label("999.9/sec");
		topHalf.add(this._createLabelValueCombo("Regen Health", this.mHealthRegen));
		this.mDefensiveStatSection.add(topHalf);
		local drLabel = this.GUI.Label("Damage Reduction");
		drLabel.setTextAlignment(0.40000001, 0.0);
		this.mDefensiveStatSection.add(drLabel);
		this.mDefensiveStatSection.add(this._createDamageReductionSection());
		return this.mDefensiveStatSection;
	}

	function _createArmorRatingSection()
	{
		local defenseSection = this.GUI.Container(this.GUI.BoxLayoutV());
		defenseSection.getLayoutManager().setGap(-8);
		local fontSize = 20;
		this.mMeleeArmorRating = this.GUI.Label("9999");
		this.mFireArmorRating = this.GUI.Label("9999");
		this.mMysticArmorRating = this.GUI.Label("9999");
		this.mFrostArmorRating = this.GUI.Label("9999");
		this.mDeathArmorRating = this.GUI.Label("9999");
		local armorLabel = this.GUI.Label("Armor : ");
		armorLabel.setFontColor(this.Colors.white);
		defenseSection.add(this._createLabelValueComboRightAlign("Armor : ", this.mMeleeArmorRating, fontSize));
		return defenseSection;
	}

	function _createEQActionContainer( name, equipSlot, backgroundTexture )
	{
		local newActionContainer = this.GUI.ActionContainer(name, 1, 1, 0, 0, this, false);
		newActionContainer.setItemPanelVisible(false);
		newActionContainer.setValidDropContainer(true);
		newActionContainer.setAllowButtonDisownership(false);
		newActionContainer.setShowEquipmentComparison(false);
		newActionContainer.addListener(this);
		newActionContainer.setSlotRestriction(0, equipSlot);
		newActionContainer.setSlotsBackgroundMaterial(0, backgroundTexture, true);
		this.setContainerMoveProperties(newActionContainer);
		local size = newActionContainer.getPreferredSize();
		newActionContainer.setSize(size.width, size.height);
		return newActionContainer;
	}

	function _createPaperDollSection()
	{
		local paperDollContainer = this.GUI.Container(null);
		paperDollContainer.setSize(140, 310);
		local paperDollSection = this.GUI.Container(null);
		paperDollSection.setSize(139, 286);
		paperDollSection.setPosition(-1, -1);
		paperDollSection.setAppearance("PaperDoll");
		local widthPosition = 52;
		this.mEQAmulet = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_AMULET], this.ItemEquipSlot.ARMOR_AMULET, "EQ_Amulet_64");
		this.mEQAmulet.setPosition(11, 49);
		this.mEQArms = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_ARMS], this.ItemEquipSlot.ARMOR_ARMS, "EQ_Armor_Arms_64");
		this.mEQArms.setPosition(11, 109);
		this.mEQRingLeft = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_RING_L], this.ItemEquipSlot.ARMOR_RING_L, "EQ_Ring_64");
		this.mEQRingLeft.setPosition(11, 185);
		this.mEQHead = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_HEAD], this.ItemEquipSlot.ARMOR_HEAD, "EQ_Armor_Head_64");
		this.mEQHead.setPosition(widthPosition, 10);
		this.mEQNeck = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_NECK], this.ItemEquipSlot.ARMOR_NECK, "EQ_Armor_Neck_64");
		this.mEQNeck.setPosition(widthPosition, 49);
		this.mEQChest = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_CHEST], this.ItemEquipSlot.ARMOR_CHEST, "EQ_Armor_Chest_64");
		this.mEQChest.setPosition(widthPosition, 90);
		this.mEQWaist = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_WAIST], this.ItemEquipSlot.ARMOR_WAIST, "EQ_Armor_Waist_64");
		this.mEQWaist.setPosition(widthPosition, 129);
		this.mEQLegs = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_LEGS], this.ItemEquipSlot.ARMOR_LEGS, "EQ_Armor_Pants_64");
		this.mEQLegs.setPosition(widthPosition, 169);
		this.mEQFeet = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_FEET], this.ItemEquipSlot.ARMOR_FEET, "EQ_Armor_Feet_64");
		this.mEQFeet.setPosition(widthPosition, 208);
		this.mEQShoulders = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_SHOULDER], this.ItemEquipSlot.ARMOR_SHOULDER, "EQ_Armor_Shoulders_64");
		this.mEQShoulders.setPosition(92, 74);
		this.mEQHands = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_HANDS], this.ItemEquipSlot.ARMOR_HANDS, "EQ_Armor_Hands_64");
		this.mEQHands.setPosition(92, 145);
		this.mEQRingRight = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ARMOR_RING_R], this.ItemEquipSlot.ARMOR_RING_R, "EQ_Ring_64");
		this.mEQRingRight.setPosition(92, 185);
		paperDollSection.add(this.mEQAmulet);
		paperDollSection.add(this.mEQArms);
		paperDollSection.add(this.mEQRingLeft);
		paperDollSection.add(this.mEQHead);
		paperDollSection.add(this.mEQNeck);
		paperDollSection.add(this.mEQChest);
		paperDollSection.add(this.mEQWaist);
		paperDollSection.add(this.mEQLegs);
		paperDollSection.add(this.mEQFeet);
		paperDollSection.add(this.mEQShoulders);
		paperDollSection.add(this.mEQHands);
		paperDollSection.add(this.mEQRingRight);
		paperDollContainer.add(paperDollSection);
		local weaponSection = this._createWeaponSection();
		weaponSection.setPosition(10, 245);
		paperDollContainer.add(weaponSection);
		return paperDollContainer;
	}

	function _createCharmSection()
	{
		local charmWidth = 139;
		local charmHeight = 286;
		local container = this.GUI.Container(null);
		local charmRollout = this.GUI.Container(null);
		charmRollout.setAppearance("CharmBackground");
		charmRollout.setSize(charmWidth, charmHeight);
		charmRollout.setPreferredSize(charmWidth, charmHeight);
		this.mEQRedCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.RED_CHARM], this.ItemEquipSlot.RED_CHARM, "EQ_Charm_Red_64");
		local charmButtonSize = this.mEQRedCharm.getSize();
		this.mEQRedCharm.setPosition(53, 40);
		this.mEQOrangeCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.ItemEquipSlot.ORANGE_CHARM, "EQ_Charm_Orange_64");
		this.mEQOrangeCharm.setPosition(24, 104);
		this.mEQPurpleCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.ItemEquipSlot.PURPLE_CHARM, "EQ_Charm_Purple_64");
		this.mEQPurpleCharm.setPosition(83, 104);
		this.mEQYellowCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.YELLOW_CHARM], this.ItemEquipSlot.YELLOW_CHARM, "EQ_Charm_Yellow_64");
		this.mEQYellowCharm.setPosition(15, 172);
		this.mEQGreenCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.ItemEquipSlot.GREEN_CHARM, "EQ_Charm_Green_64");
		this.mEQGreenCharm.setPosition(53, 233);
		this.mEQBlueCharm = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.BLUE_CHARM], this.ItemEquipSlot.BLUE_CHARM, "EQ_Charm_Blue_64");
		this.mEQBlueCharm.setPosition(91, 172);
		charmRollout.add(this.mEQRedCharm);
		charmRollout.add(this.mEQOrangeCharm);
		charmRollout.add(this.mEQPurpleCharm);
		charmRollout.add(this.mEQYellowCharm);
		charmRollout.add(this.mEQGreenCharm);
		charmRollout.add(this.mEQBlueCharm);
		this.mEQRedCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQRedCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQRedCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.AcceptFromProperties(this));
		this.mEQRedCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.AcceptFromProperties(this));
		this.mEQYellowCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQYellowCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQYellowCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.AcceptFromProperties(this));
		this.mEQYellowCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.AcceptFromProperties(this));
		this.mEQBlueCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQBlueCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQBlueCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.AcceptFromProperties(this));
		this.mEQBlueCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.AcceptFromProperties(this));
		this.mEQOrangeCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.RED_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQOrangeCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQOrangeCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.YELLOW_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQOrangeCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQOrangeCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.YELLOW_CHARM], this.AcceptFromProperties(this));
		this.mEQOrangeCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.RED_CHARM], this.AcceptFromProperties(this));
		this.mEQOrangeCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.AcceptFromProperties(this));
		this.mEQOrangeCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.AcceptFromProperties(this));
		this.mEQPurpleCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.BLUE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQPurpleCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.RED_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQPurpleCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQPurpleCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQPurpleCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.BLUE_CHARM], this.AcceptFromProperties(this));
		this.mEQPurpleCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.RED_CHARM], this.AcceptFromProperties(this));
		this.mEQPurpleCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.AcceptFromProperties(this));
		this.mEQPurpleCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.GREEN_CHARM], this.AcceptFromProperties(this));
		this.mEQGreenCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.BLUE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQGreenCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQGreenCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.YELLOW_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQGreenCharm.addMovingToProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.MoveToProperties(this.MovementTypes.MOVE));
		this.mEQGreenCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.YELLOW_CHARM], this.AcceptFromProperties(this));
		this.mEQGreenCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.BLUE_CHARM], this.AcceptFromProperties(this));
		this.mEQGreenCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.ORANGE_CHARM], this.AcceptFromProperties(this));
		this.mEQGreenCharm.addAcceptingFromProperties(this.EquipmentContainerNames[this.ItemEquipSlot.PURPLE_CHARM], this.AcceptFromProperties(this));
		container.add(charmRollout);
		return container;
	}

	function _createLabelValueCombo( label, value )
	{
		local combo = this.GUI.Container(this.GUI.BoxLayout());
		local headerLabel = this.GUI.Label(label);
		headerLabel.setFontColor(this.Colors.white);
		headerLabel.setFont(this.GUI.Font("Maiandra", 22));
		combo.add(headerLabel);
		combo.add(value);
		return combo;
	}

	function _createLabelValueComboRightAlign( label, value, ... )
	{
		local fontSize = 22;

		if (vargc > 0)
		{
			fontSize = vargv[0];
		}

		local combo = this.GUI.Container(this.GUI.BoxLayout());
		local titleContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		titleContainer.getLayoutManager().setAlignment(1.0);
		titleContainer.setSize(90, 22);
		titleContainer.setPreferredSize(90, 22);
		local headerLabel = this.GUI.Label(label);
		headerLabel.setFontColor(this.Colors.white);
		headerLabel.setFont(this.GUI.Font("Maiandra", fontSize));
		headerLabel.setTextAlignment(0.5, 1.0);
		titleContainer.add(headerLabel);
		local valueContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		valueContainer.getLayoutManager().setAlignment(0.0);
		valueContainer.setSize(70, 22);
		valueContainer.setPreferredSize(70, 22);
		valueContainer.add(value);
		combo.add(titleContainer);
		combo.add(valueContainer);
		return combo;
	}

	function _createDamageReductionSection()
	{
		local combo = this.GUI.Container(this.GUI.GridLayout(2, 5));
		combo.getLayoutManager().setRows(15, 15);
		combo.getLayoutManager().setColumns(55, 55, 55, 55, 55);
		this.mMeleeResist = this.GUI.Label("999.99%");
		this.mFireResist = this.GUI.Label("999.99%");
		this.mMysticResist = this.GUI.Label("999.99%");
		this.mFrostResist = this.GUI.Label("999.99%");
		this.mDeathResist = this.GUI.Label("999.99%");
		combo.add(this.GUI.Label("Melee"));
		combo.add(this.GUI.Label("Fire"));
		combo.add(this.GUI.Label("Mystic"));
		combo.add(this.GUI.Label("Frost"));
		combo.add(this.GUI.Label("Death"));
		combo.add(this.mMeleeResist);
		combo.add(this.mFireResist);
		combo.add(this.mMysticResist);
		combo.add(this.mFrostResist);
		combo.add(this.mDeathResist);
		return combo;
	}

	function _createOffenseAdvancedStat()
	{
		this.mOffensiveStatSection = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mMeleeDPS = this.GUI.Label("999.99");
		this.mRangedDPS = this.GUI.Label("999.99");
		local offensiveDpsSection = this.GUI.Container(this.GUI.GridLayout(1, 3));
		offensiveDpsSection.getLayoutManager().setColumns(130, 35, 130);
		offensiveDpsSection.add(this._createLabelValueCombo("Melee DPS", this.mMeleeDPS));
		offensiveDpsSection.add(this.GUI.Spacer(1, 1));
		offensiveDpsSection.add(this._createLabelValueCombo("Ranged DPS", this.mRangedDPS));
		this.mOffensiveStatSection.add(offensiveDpsSection);
		this.mHealingDPS = this.GUI.Label("999.99");
		this.mOffensiveStatSection.add(this._createLabelValueCombo("Healing DPS", this.mHealingDPS));
		local hitCritSection = this.GUI.Container(this.GUI.GridLayout(2, 3));
		hitCritSection.getLayoutManager().setColumns(125, 20, 125);
		this.mToHitMelee = this.GUI.Label("999.99%");
		this.mToHitMagic = this.GUI.Label("999.99%");
		this.mMeleeCritRate = this.GUI.Label("999.99%");
		this.mMagicCritRate = this.GUI.Label("999.99%");
		hitCritSection.add(this._createLabelValueCombo("To Hit Melee", this.mToHitMelee));
		hitCritSection.add(this.GUI.Spacer(1, 1));
		hitCritSection.add(this._createLabelValueCombo("To Hit Magic", this.mToHitMagic));
		hitCritSection.add(this._createLabelValueCombo("Critical Melee", this.mMeleeCritRate));
		hitCritSection.add(this.GUI.Spacer(1, 1));
		hitCritSection.add(this._createLabelValueCombo("Critical Magic", this.mMagicCritRate));
		this.mOffensiveStatSection.add(hitCritSection);
		local speedSection = this.GUI.Container(this.GUI.GridLayout(1, 3));
		speedSection.getLayoutManager().setColumns(125, 25, 125);
		this.mAttackSpeed = this.GUI.Label("999");
		this.mCastingSpeed = this.GUI.Label("999");
		speedSection.add(this._createLabelValueCombo("Attack Speed", this.mAttackSpeed));
		speedSection.add(this.GUI.Spacer(1, 1));
		speedSection.add(this._createLabelValueCombo("Cast Speed", this.mCastingSpeed));
		this.mOffensiveStatSection.add(speedSection);
		return this.mOffensiveStatSection;
	}

	function _createOffenseDefenseButtons()
	{
		local buttonGrid = this.GUI.Container(this.GUI.GridLayout(1, 2));
		buttonGrid.getLayoutManager().setGaps(5, 0);
		buttonGrid.getLayoutManager().setColumns(70, 70);
		this.mOffenseButton = this.GUI.Button("Offense");
		this.mOffenseButton.setFont(this.GUI.Font("Maiandra", 12));
		this.mOffenseButton.addActionListener(this);
		this.mOffenseButton.setReleaseMessage("toggleOffense");
		this.mOffenseButton.setToggledExplicit(true);
		this.mDefenseButton = this.GUI.Button("Defense");
		this.mDefenseButton.setFont(this.GUI.Font("Maiandra", 12));
		this.mDefenseButton.addActionListener(this);
		this.mDefenseButton.setReleaseMessage("toggleDefense");
		buttonGrid.add(this.mOffenseButton);
		buttonGrid.add(this.mDefenseButton);
		return buttonGrid;
	}

	function _createWeaponSection()
	{
		local weaponSection = this.GUI.Container(this.GUI.GridLayout(1, 4));
		weaponSection.getLayoutManager().setGaps(9, 0);
		weaponSection.getLayoutManager().setColumns(32, 32, 32, 32);
		local size = weaponSection.getSize();
		weaponSection.setSize(150, 50);
		weaponSection.setPreferredSize(size.width, size.height);
		this.mEQMainHand = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.WEAPON_MAIN_HAND], this.ItemEquipSlot.WEAPON_MAIN_HAND, "EQ_MainWeapon_64");
		this.mEQOffHand = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.WEAPON_OFF_HAND], this.ItemEquipSlot.WEAPON_OFF_HAND, "EQ_MainWeapon_64");
		this.mEQRanged = this._createEQActionContainer(this.EquipmentContainerNames[this.ItemEquipSlot.WEAPON_RANGED], this.ItemEquipSlot.WEAPON_RANGED, "EQ_Ranged_64");
		weaponSection.add(this.mEQMainHand);
		weaponSection.add(this.mEQOffHand);
		weaponSection.add(this.mEQRanged);
		return weaponSection;
	}

	function _getDroppedComponent( evt )
	{
		local t = evt.getTransferable();

		if (typeof t != "instance" || !(t instanceof this.GUI.ActionButton))
		{
			return null;
		}

		return t;
	}

	function _updateAttackSpeed()
	{
		local status = this.STAT_NORMAL;
		local attackSpeed = 100;
		local modAttackSpeed = ::_avatar.getStat(this.Stat.MOD_ATTACK_SPEED, true);

		if (modAttackSpeed == null)
		{
			modAttackSpeed = 0;
		}

		attackSpeed += attackSpeed * modAttackSpeed;

		if (attackSpeed < 100)
		{
			status = this.STAT_DEBUFFED;
		}
		else if (attackSpeed > 100)
		{
			status = this.STAT_BUFFED;
		}

		this.setAttackSpeed(attackSpeed, status);
		this._updateMeleeDPS();
		this._updateRangedDPS();
	}

	function _updateBlockRate()
	{
		local baseBlock = ::_avatar.getStat(this.Stat.BASE_BLOCK, true);
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		local str = ::_avatar.getStat(this.Stat.STRENGTH, true);
		local blockMod = ::_avatar.getStat(this.Stat.MOD_BLOCK, true);

		if (blockMod == null)
		{
			blockMod = 0;
		}

		if (baseBlock == null || dex == null || str == null)
		{
			return;
		}

		blockMod *= 100;
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_BLOCK);

		if (status == this.STAT_NORMAL)
		{
			if (blockMod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (blockMod < 0)
			{
				status = this.STAT_DEBUFFED;
			}
		}

		this.setBlockRate(::_combatEquations.calcBlockRate(baseBlock, dex, str, blockMod), status);
	}

	function _updateCastSpeed()
	{
		local status = this.STAT_NORMAL;
		local castSpeed = 100;
		local castSpeedMod = ::_avatar.getStat(this.Stat.MOD_CASTING_SPEED, true);

		if (castSpeedMod == null)
		{
			castSpeedMod = 0;
		}

		castSpeed += castSpeed * castSpeedMod;

		if (castSpeed < 100)
		{
			status = this.STAT_DEBUFFED;
		}
		else if (castSpeed > 100)
		{
			status = this.STAT_BUFFED;
		}

		this.setCastingSpeed(castSpeed, status);
	}

	function _updateConstitution()
	{
		local con = ::_avatar.getStat(this.Stat.CONSTITUTION, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.CONSTITUTION);
		local baseCon = ::_avatar.getBaseStatValue(this.Stat.CONSTITUTION);
		this.setConstitution(baseCon, con, status);
		this._updateRegenHealth();
	}

	function _updateCritMagic()
	{
		local baseCrit = ::_avatar.getStat(this.Stat.BASE_MAGIC_CRITICAL, true);
		local psyche = ::_avatar.getStat(this.Stat.PSYCHE, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_MAGIC_CRITICAL);
		local mod = ::_avatar.getStat(this.Stat.MOD_MAGIC_TO_CRIT, true);

		if (mod == null)
		{
			mod = 0;
		}

		if (baseCrit == null || psyche == null)
		{
			return;
		}

		mod *= 100;

		if (status == this.STAT_NORMAL)
		{
			if (mod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (mod < 0)
			{
				status = this.STAT_DEBUFFED;
			}
		}

		this.setMagicCritRate(::_combatEquations.calcMagicCritRate(baseCrit, psyche, mod), status);
	}

	function _updateCritMelee()
	{
		local baseCrit = ::_avatar.getStat(this.Stat.BASE_MELEE_CRITICAL, true);
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_MELEE_CRITICAL);
		local mod = ::_avatar.getStat(this.Stat.MOD_MELEE_TO_CRIT, true);

		if (mod == null)
		{
			mod = 0;
		}

		if (baseCrit == null || dex == null || status == null)
		{
			return;
		}

		mod *= 100;

		if (status == this.STAT_NORMAL)
		{
			if (mod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (mod < 0)
			{
				status = this.STAT_DEBUFFED;
			}
		}

		this.setMeleeCritRate(::_combatEquations.calcMeleeCritRate(baseCrit, dex, mod), status);
	}

	function _updateClassLevel()
	{
		local profession = ::_avatar.getStat(this.Stat.PROFESSION, true);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);

		if (profession == null || level == null)
		{
			return;
		}

		this.setLevelClass(level, this.Professions[profession].name);
	}

	function _updateDeathAR()
	{
		local deathAR = ::_avatar.getStat(this.Stat.DAMAGE_RESIST_DEATH, true);
		local deathARMod = ::_avatar.getStat(this.Stat.DR_MOD_DEATH, true);

		if (deathARMod != null)
		{
			deathAR += deathAR * (deathARMod / 1000.0);
		}

		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_DEATH);
		this.setDeathArmorRating(deathAR.tointeger(), status);
		this._updateDeathResist(deathAR);
	}

	function _updateRangedDPS()
	{
		local totalDPS = 0.0;
		local equipment = this.mEQRanged.getActionInSlot(0);
		local status = this.STAT_NORMAL;

		if (equipment)
		{
			switch(equipment.mItemDefData.mWeaponType)
			{
			case this.WeaponType.WAND:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_WAND);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				break;

			case this.WeaponType.BOW:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_BOW);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				break;

			case this.WeaponType.THROWN:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_THROWN);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				break;
			}
		}

		this.setRangedDPS(totalDPS, status);
	}

	function _updateDeathResist( deathAR )
	{
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_DEATH);
		local deathResist = ::_combatEquations.calcDamageReduction(deathAR, level);
		this.setDeathResist(deathResist, status);
	}

	function _updateDexterity()
	{
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.DEXTERITY);
		local baseDex = ::_avatar.getBaseStatValue(this.Stat.DEXTERITY);
		this.setDexterity(baseDex, dex, status);
		this._updateDodgeRate();
		this._updateParryRate();
		this._updateBlockRate();
		this._updateCritMelee();
		this._updateToHitMelee();
	}

	function _updateDodgeRate()
	{
		local baseDodge = ::_avatar.getStat(this.Stat.BASE_DODGE, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_DODGE);
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		this.setDodgeRate(::_combatEquations.calcDodge(baseDodge, dex), status);
	}

	function _updateFireAR()
	{
		local fireAR = ::_avatar.getStat(this.Stat.DAMAGE_RESIST_FIRE, true);
		local fireARMod = ::_avatar.getStat(this.Stat.DR_MOD_FIRE, true);

		if (fireARMod != null)
		{
			fireAR += fireAR * (fireARMod / 1000.0);
		}

		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_FIRE);
		this.setFireArmorRating(fireAR.tointeger(), status);
		this._updateFireResist(fireAR);
	}

	function _updateFireResist( fireAR )
	{
		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_FIRE);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local fireResist = ::_combatEquations.calcDamageReduction(fireAR.tointeger(), level);
		this.setFireResist(fireResist, status);
	}

	function _updateFrostAR()
	{
		local frostAR = ::_avatar.getStat(this.Stat.DAMAGE_RESIST_FROST, true);
		local frostARMod = ::_avatar.getStat(this.Stat.DR_MOD_FROST, true);

		if (frostARMod != null)
		{
			frostAR += frostAR * (frostARMod / 1000.0);
		}

		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_FROST);
		this.setFrostArmorRating(frostAR.tointeger(), status);
		this._updateFrostResist(frostAR);
	}

	function _updateFrostResist( frostAR )
	{
		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_FROST);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local frostResist = ::_combatEquations.calcDamageReduction(frostAR, level);
		this.setFrostResist(frostResist, status);
	}

	function _updateHealingDPS()
	{
		local castingSpeed = ::_avatar.getStat(this.Stat.MAGIC_ATTACK_SPEED, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.MAGIC_ATTACK_SPEED);
		this.setHealingDPS(::_combatEquations.calcDPS(::_avatar.getStat(this.Stat.BASE_HEALING, true), castingSpeed), status);
	}

	function _updateHealth()
	{
		local baseHealth = ::_avatar.getStat(this.Stat.BASE_HEALTH, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_HEALTH);
		local curHealth = ::_avatar.getStat(this.Stat.HEALTH, true);
		local con = ::_avatar.getStat(this.Stat.CONSTITUTION, true);
		local bonusHealth = ::_avatar.getStat(this.Stat.HEALTH_MOD, true);
		local maxHealth = ::_combatEquations.calcMaxHealth(baseHealth, con, bonusHealth, 0);
		this.setHealth(curHealth, maxHealth, status);
	}

	function _updateMagicDeflect()
	{
		local baseDeflect = ::_avatar.getStat(this.Stat.BASE_DEFLECT, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_DEFLECT);
		local psyche = ::_avatar.getStat(this.Stat.PSYCHE, true);
		this.setMagicDeflectRate(::_combatEquations.calcMagicDeflectRate(baseDeflect, psyche), status);
	}

	function _updateMeleeAR()
	{
		local meleeAR = ::_avatar.getStat(this.Stat.DAMAGE_RESIST_MELEE, true);
		local meleeARMod = ::_avatar.getStat(this.Stat.DR_MOD_MELEE, true);

		if (meleeARMod != null)
		{
			meleeAR += meleeAR * (meleeARMod / 1000.0);
		}

		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_MELEE);
		this.setMeleeArmorRating(meleeAR.tointeger(), status);
		this._updateMeleeResist(meleeAR);
	}

	function _updateMeleeDPS()
	{
		local totalDPS = 0.0;
		local equipment = this.mEQMainHand.getActionInSlot(0);
		local status = this.STAT_NORMAL;
		local check2ndHand = false;

		if (!equipment)
		{
			check2ndHand = true;
		}
		else
		{
			switch(equipment.mItemDefData.mWeaponType)
			{
			case this.WeaponType.SMALL:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_SMALL);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				check2ndHand = true;
				break;

			case this.WeaponType.ONE_HAND:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_1H);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				check2ndHand = true;
				break;

			case this.WeaponType.TWO_HAND:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_2H);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				break;

			case this.WeaponType.POLE:
				status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_POLE);
				totalDPS = ::_combatEquations.calcWeaponDPS(equipment);
				break;
			}
		}

		if (check2ndHand)
		{
			equipment = this.mEQOffHand.getActionInSlot(0);

			if (equipment)
			{
				switch(equipment.mItemDefData.mWeaponType)
				{
				case this.WeaponType.SMALL:
					if (status != this.STAT_DEBUFFED)
					{
						status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_SMALL);
					}

					totalDPS += ::_combatEquations.calcWeaponDPS(equipment);
					break;

				case this.WeaponType.ONE_HAND:
					if (status != this.STAT_DEBUFFED)
					{
						status = this.checkStatBuffOrDebuff(this.Stat.WEAPON_DAMAGE_1H);
					}

					totalDPS += ::_combatEquations.calcWeaponDPS(equipment);
					break;
				}
			}
		}

		this.setMeleeDPS(totalDPS, status);
	}

	function _updateMeleeResist( meleeAR )
	{
		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_MELEE);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local meleeResist = ::_combatEquations.calcDamageReduction(meleeAR, level);
		this.setMeleeResist(meleeResist, status);
	}

	function _updateMysticAR()
	{
		local mysticAR = ::_avatar.getStat(this.Stat.DAMAGE_RESIST_MYSTIC, true);
		local mysticARMod = ::_avatar.getStat(this.Stat.DR_MOD_MYSTIC, true);

		if (mysticARMod != null)
		{
			mysticAR += mysticAR * (mysticARMod / 1000.0);
		}

		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_MYSTIC);
		this.setMysticArmorRating(mysticAR.tointeger(), status);
		this._updateMysticResist(mysticAR);
	}

	function _updateMysticResist( mysticAR )
	{
		local status = this.checkStatBuffOrDebuff(this.Stat.DAMAGE_RESIST_MYSTIC);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local mysticResist = ::_combatEquations.calcDamageReduction(mysticAR, level);
		this.setMysticResist(mysticResist, status);
	}

	function _updateName()
	{
		this.setTitle(::_avatar.getStat(this.Stat.DISPLAY_NAME));
	}

	function _updateParryRate()
	{
		local baseParry = ::_avatar.getStat(this.Stat.BASE_PARRY, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_PARRY);
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		local str = ::_avatar.getStat(this.Stat.STRENGTH, true);
		local parryMod = ::_avatar.getStat(this.Stat.MOD_PARRY, true);

		if (parryMod == null)
		{
			parryMod = 0;
		}

		parryMod *= 100;

		if (status == this.STAT_NORMAL)
		{
			if (parryMod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (parryMod < 0)
			{
				this.staus = this.STAT_DEBUFFED;
			}
		}

		this.setParryRate(::_combatEquations.calcParry(baseParry, dex, str, parryMod), status);
	}

	function _updatePsyche()
	{
		local psyche = ::_avatar.getStat(this.Stat.PSYCHE, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.PSYCHE);
		local basePsy = ::_avatar.getBaseStatValue(this.Stat.PSYCHE);
		this.setPsyche(basePsy, psyche, status);
		this._updateMagicDeflect();
		this._updateCritMagic();
		this._updateToHitMagic();
	}

	function _updateRegenHealth()
	{
		local spirit = ::_avatar.getStat(this.Stat.SPIRIT, true);
		local con = ::_avatar.getStat(this.Stat.CONSTITUTION, true);
		local level = ::_avatar.getStat(this.Stat.LEVEL, true);
		local mod = this._avatar.getStat(this.Stat.MOD_HEALTH_REGEN, true);
		local status = this.STAT_NORMAL;

		if (mod == null)
		{
			mod = 0;
		}

		if (spirit == null || con == null || level == null)
		{
			return;
		}

		if (mod > 0.001)
		{
			status = this.STAT_BUFFED;
		}
		else if (mod < 0)
		{
			status = this.STAT_DEBUFFED;
		}

		this.setHealthRegenRate(::_combatEquations.calcHealthRegen(false, spirit, con, level, mod), status);
	}

	function _updateRunSpeed()
	{
		local movement = 100;
		local status = this.STAT_NORMAL;
		local mod = ::_avatar.getStat(this.Stat.MOD_MOVEMENT, true);

		if (mod == null)
		{
			mod = 0;
		}

		movement += mod;

		if (mod > 1)
		{
			status = this.STAT_BUFFED;
		}
		else if (mod < 0)
		{
			status = this.STAT_DEBUFFED;
		}

		this.setRunSpeed(movement, status);
	}

	function _updateSpirit()
	{
		local spirit = ::_avatar.getStat(this.Stat.SPIRIT, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.SPIRIT);
		local baseSpi = ::_avatar.getBaseStatValue(this.Stat.SPIRIT);
		this.setSpirit(baseSpi, spirit, status);
		this._updateRegenHealth();
		this._updateBlockRate();
	}

	function _updateStrength()
	{
		local str = ::_avatar.getStat(this.Stat.STRENGTH, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.STRENGTH);
		local baseStr = ::_avatar.getBaseStatValue(this.Stat.STRENGTH);
		this.setStrength(baseStr, str, status);
		this._updateParryRate();
	}

	function _updateToHitMagic()
	{
		local baseHitRate = ::_avatar.getStat(this.Stat.BASE_MAGIC_SUCCESS, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_MAGIC_SUCCESS);
		local psyche = ::_avatar.getStat(this.Stat.PSYCHE, true);
		local mod = ::_avatar.getStat(this.Stat.MOD_MAGIC_TO_HIT);

		if (mod == null)
		{
			mod = 0;
		}

		if (baseHitRate == null || psyche == null)
		{
			return;
		}

		mod *= 100;

		if (status == this.STAT_NORMAL)
		{
			if (mod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (mod < 0)
			{
				status = this.STAT_DEBUFFED;
			}
		}

		this.setMagicHitRate(::_combatEquations.calcMagicHitRate(baseHitRate, psyche, mod), status);
	}

	function _updateToHitMelee()
	{
		local baseHitRate = ::_avatar.getStat(this.Stat.BASE_MELEE_TO_HIT, true);
		local status = this.checkStatBuffOrDebuff(this.Stat.BASE_MELEE_TO_HIT);
		local dex = ::_avatar.getStat(this.Stat.DEXTERITY, true);
		local mod = ::_avatar.getStat(this.Stat.MOD_MELEE_TO_HIT);

		if (mod == null)
		{
			mod = 0;
		}

		if (baseHitRate == null || dex == null)
		{
			return;
		}

		mod *= 100;

		if (status == this.STAT_NORMAL)
		{
			if (mod > 0.001)
			{
				status = this.STAT_BUFFED;
			}
			else if (mod < 0)
			{
				status = this.STAT_DEBUFFED;
			}
		}

		this.setMeleeHitRate(::_combatEquations.calcMeleeHitRate(baseHitRate, dex, mod), status);
	}

	function _updateQuickbarWithEquipment()
	{
		local quickbar1 = ::_quickBarManager.getQuickBar(0);
		local quickbarContainer = quickbar1.getActionContainer();
		local quickbarAb = quickbarContainer.getActionButtonFromIndex(0);

		if (quickbarAb)
		{
			local mainHandIcon = this.mEQMainHand.getActionButtonFromIndex(0);

			if (mainHandIcon)
			{
				quickbarAb.setImageName(mainHandIcon.getImageName());
			}
			else
			{
				local actionButtonSlot = this.mEQMainHand.getSlotContents(0);
				quickbarAb.setImageName(actionButtonSlot.getBackgroundMaterial());
			}
		}

		quickbarAb = quickbarContainer.getActionButtonFromIndex(1);

		if (quickbarAb)
		{
			local rangedIcon = this.mEQRanged.getActionButtonFromIndex(0);

			if (rangedIcon)
			{
				quickbarAb.setImageName(rangedIcon.getImageName());
			}
			else
			{
				local actionButtonSlot = this.mEQRanged.getSlotContents(0);
				quickbarAb.setImageName(actionButtonSlot.getBackgroundMaterial());
			}
		}
	}

}

