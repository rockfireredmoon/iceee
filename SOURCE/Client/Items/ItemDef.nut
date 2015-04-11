class this.ItemDefData 
{
	mID = 0;
	mType = 0;
	mDisplayName = "Unknown";
	mAppearance = null;
	mIcon = "Icon/QuestionMark";
	mIvType1 = 0;
	mIvMax1 = 0;
	mIvType2 = 0;
	mIvMax2 = 0;
	mSv1 = "";
	mContainerSlots = 0;
	mAutoTitleType = 0;
	mLevel = 0;
	mBindingType = 0;
	mEquipType = 0;
	mWeaponType = 0;
	mWeaponDamageMin = 0;
	mWeaponDamageMax = 0;
	mWeaponSpeed = 0;
	mWeaponExtraDamangeRating = 0;
	mWeaponExtraDamageType = 0;
	mEquipEffectId = 0;
	mUseAbilityId = 0;
	mActionAbilityId = 0;
	mArmorType = 0;
	mArmorResistMelee = 0;
	mArmorResistFire = 0;
	mArmorResistFrost = 0;
	mArmorResistMystic = 0;
	mArmorResistDeath = 0;
	mBonusStrength = 0;
	mBonusDexterity = 0;
	mBonusConstitution = 0;
	mBonusPsyche = 0;
	mBonusSpirit = 0;
	mBonusWill = 0;
	mMeleeHitMod = 0;
	mMeleeCritMod = 0;
	mMagicHitMod = 0;
	mMagicCritMod = 0;
	mParryMod = 0;
	mBlockMod = 0;
	mRunSpeedMod = 0;
	mRegenHealthMod = 0;
	mAttackSpeedMod = 0;
	mCastSpeedMod = 0;
	mHealingMod = 0;
	mValue = -1;
	mValueType = this.CurrencyCategory.COPPER;
	mFlavorText = "";
	mResultItem = 0;
	mKeyComponent = 0;
	mCraftComponents = null;
	mSpecialItemType = this.SpecialItemType.NONE;
	mQualityLevel = this.QualityLevel.POOR;
	mMinUseLevel = -1;
	mOwnershipRestriction = 0;
	mValid = false;
	mTooltipComponent = null;
	mLastShowBuyValue = null;
	constructor( id )
	{
		this.mID = id;
		this.mCraftComponents = {};
	}

	function getID()
	{
		return this.mID;
	}

	function getType()
	{
		return this.mType;
	}

	function getDisplayName()
	{
		return this.mDisplayName;
	}

	function getQuestName()
	{
		return this.mSv1;
	}

	function getIcon()
	{
		return this.mIcon;
	}

	function isValid()
	{
		return this.mValid;
	}

	function isUsable()
	{
		return this.mValid && this.mUseAbilityId != 0;
	}

	function getEquipType()
	{
		return this.mEquipType;
	}

	function getWeaponType()
	{
		return this.mWeaponType;
	}

	function getArmorType()
	{
		return this.mArmorType;
	}

	function setAppearance( appearance )
	{
		if (typeof appearance == "array")
		{
			local allNull = true;

			foreach( a in appearance )
			{
				if (a != null)
				{
					allNull = false;
					break;
				}
			}

			if (allNull == true)
			{
				appearance = null;
			}
		}

		this.mAppearance = appearance;
	}

	function getAppearance()
	{
		if (typeof this.mAppearance == "array")
		{
			local allNull = true;

			foreach( a in this.mAppearance )
			{
				if (a != null)
				{
					allNull = false;
					break;
				}
			}

			if (allNull == true)
			{
				this.mAppearance = null;
			}
		}

		return this.mAppearance;
	}

	function getTooltip( tooltipMods, ... )
	{
		local force = false;
		local optionalComponent;
		local item;
		local showBindingInfo = true;
		local showBuyValue = false;
		local mods = tooltipMods;

		if (mods != null)
		{
			if ("showBuyValue" in mods)
			{
				showBuyValue = mods.showBuyValue;
			}

			if ("showBindingInfo" in mods)
			{
				showBindingInfo = mods.showBindingInfo;
			}
		}

		if (vargc > 0 && vargv[0] == true)
		{
			force = true;
		}

		force = true;

		if (vargc > 1)
		{
			optionalComponent = vargv[1];
		}

		if (vargc > 2)
		{
			item = vargv[2];
		}

		if ((!this.mTooltipComponent || force || this.mLastShowBuyValue != showBuyValue) && this.mValid == true)
		{
			this.mLastShowBuyValue = showBuyValue;
			this.mTooltipComponent = this.GUI.Container(this.GUI.BoxLayoutV());
			this.mTooltipComponent.getLayoutManager().setExpand(true);
			this.renderTooltip(this.mTooltipComponent, showBuyValue, optionalComponent, item, showBindingInfo, mods);
		}
		else if (!this.mTooltipComponent)
		{
			this.mTooltipComponent = this.GUI.Container(this.GUI.BoxLayoutV());
			local loading = this.GUI.Label(this.TXT("Loading..."));
			loading.setFont(this.GUI.Font("Maiandra", 32, true));
			this.mTooltipComponent.add(loading);
		}

		return this.mTooltipComponent;
	}

	function renderTooltip( component, showbuyvalue, optionalComponent, item, ... )
	{
		local showBindingInfo = true;
		local mods = [];
		local currentlyEquipped = false;

		if (vargc > 0)
		{
			showBindingInfo = vargv[0];
		}

		if (vargc > 1)
		{
			mods = vargv[1];
		}

		if ("CurrentlyEquipped" in mods)
		{
			currentlyEquipped = mods.CurrentlyEquipped;
		}

		switch(this.mType)
		{
		case this.ItemType.UNKNOWN:
		case this.ItemType.BASIC:
		case this.ItemType.SYSTEM:
			this._addNameRow(component);

			if (showBindingInfo)
			{
				this._addBindingRow(component, item);
			}

			this._addDivider(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.SPECIAL:
			this._addNameRow(component);

			if (showBindingInfo)
			{
				this._addBindingRow(component, item);
			}

			this._addDivider(component);

			if (optionalComponent)
			{
				component.add(optionalComponent);
			}

			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			this._addSpecialItemTypeSection(component);
			break;

		case this.ItemType.WEAPON:
			if (currentlyEquipped)
			{
				component.add(this.GUI.Label("Currently Equipped"));
				this._addDivider(component);
			}

			this._addNameRow(component);
			this._addClassRestrictionsLevelRow(component, this.getClassRestrictions(), this.getWeaponPower());
			this._addDivider(component);
			this._addWeaponSection(component, item);
			this._addDivider(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);

			if (this._addBonusSection(component) || this._addEffectSection(component))
			{
				this._addDivider(component);
			}

			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.ARMOR:
			if (currentlyEquipped)
			{
				component.add(this.GUI.Label("Currently Equipped"));
				this._addDivider(component);
			}

			this._addNameRow(component);
			this._addClassRestrictionsLevelRow(component, this.getClassRestrictions(), this.getArmorPower());
			this._addDivider(component);
			this._addArmorSection(component, item);
			this._addDivider(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);

			if (this._addBonusSection(component) || this._addEffectSection(component))
			{
				this._addDivider(component);
			}

			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.CHARM:
			if (currentlyEquipped)
			{
				component.add(this.GUI.Label("Currently Equipped"));
				this._addDivider(component);
			}

			this._addNameRow(component);
			this._addClassRestrictionsLevelRow(component, this.getClassRestrictions(), 30);
			this._addDivider(component);
			this._addCharmSection(component, item);
			this._addDivider(component);

			if (this._addBonusSection(component))
			{
				this._addDivider(component);
			}

			if (this._addCharmBonusSection(component))
			{
				this._addDivider(component);
			}

			if (this._addEffectSection(component))
			{
				this._addDivider(component);
			}

			if (optionalComponent)
			{
				component.add(optionalComponent);
			}

			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.CONSUMABLE:
			if (this.mUseAbilityId != 0 && this.mUseAbilityId != "")
			{
				local consumableMods = {};

				if (item && ("mItemData" in item) && item.mItemData.mBound)
				{
					consumableMods.bind <- "bound";
				}
				else
				{
					consumableMods.bind <- this.mBindingType;
				}

				if (!showBindingInfo)
				{
					consumableMods.showBindingInfo <- false;
				}

				local abilityComp = ::AbilityUtil.buildToolTipComponent(this.mUseAbilityId, consumableMods);
				component.add(abilityComp);
			}
			else
			{
				this._addNameRow(component);

				if (showBindingInfo)
				{
					this._addBindingRow(component, item);
				}

				this._addDivider(component);

				if (this._addEffectSection(component))
				{
					this._addDivider(component);
				}

				this._buildOwnershipRestriction(component);
				this._addLifetimeValue(component);
				this._addValueRow(component, showbuyvalue);

				if (this.mFlavorText != "")
				{
					this._addDivider(component);
					this._addFlavorText(component);
				}
			}

			break;

		case this.ItemType.CONTAINER:
			if (currentlyEquipped)
			{
				component.add(this.GUI.Label("Currently Equipped"));
				this._addDivider(component);
			}

			this._addNameRow(component);
			this._addDivider(component);

			if (showBindingInfo)
			{
				this._addBindingRow(component, item);
			}

			this._addContainerSection(component);
			this._addDivider(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.QUEST:
			this._addNameRow(component);
			this._addDivider(component);

			if (showBindingInfo)
			{
				this._addBindingRow(component, item);
			}

			this._addQuestSection(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			break;

		case this.ItemType.RECIPE:
			this._addNameRow(component);
			this._addClassRestrictionsLevelRow(component, this.getClassRestrictions(), this.mLevel);
			this._addDivider(component);

			if (showBindingInfo)
			{
				this._addBindingRow(component, item);
			}

			this._addCraftSection(component);
			this._addDivider(component);
			this._buildOwnershipRestriction(component);
			this._addLifetimeValue(component);
			this._addValueRow(component, showbuyvalue);

			if (this.mFlavorText != "")
			{
				this._addDivider(component);
				this._addFlavorText(component);
			}

			local result_item = ::_ItemDataManager.getItemDef(this.mResultItem);

			if (result_item)
			{
				result_item.renderTooltip(component, showbuyvalue, null, null, null);
			}

			break;
		}
	}

	function _isObjectiveItemComplete()
	{
		local questId = -1;

		if (this.mIvType1 == this.ItemIntegerType.QUEST_ID)
		{
			questId = this.mIvMax1;
		}
		else if (this.mIvType2 == this.ItemIntegerType.QUEST_ID)
		{
			questId = this.mIvMax2;
		}

		if (questId)
		{
			local questData = ::_questManager.getPlayerQuestDataById(questId);

			if (questData)
			{
				local questObjectives = questData.getObjectives();

				foreach( objectiveData in questObjectives )
				{
					if (objectiveData.getItemId() == this.getID() || objectiveData.getCreatureDefId() == this.getID())
					{
						if (objectiveData.isCompleted())
						{
							return true;
						}
						else
						{
							return false;
						}
					}
				}
			}
		}

		return false;
	}

	function getInfoPanel( showbuyvalue, miniVersion, hideValue )
	{
		local vitals;

		if (this.mValid == true)
		{
			if (true == miniVersion)
			{
				vitals = this.GUI.Container(this.GUI.GridLayout(3, 1));
				vitals.getLayoutManager().setColumns(80);
				vitals.add(this._buildName(), {
					anchor = this.GUI.GridLayout.LEFT
				});
			}
			else
			{
				vitals = this.GUI.Container(this.GUI.GridLayout(3, 2));
				vitals.getLayoutManager().setColumns(102, 40);
				vitals.add(this._buildName(), {
					span = 2,
					anchor = this.GUI.GridLayout.LEFT
				});
			}

			local level = this.GUI.Label(this.TXT("Level") + " " + this.mLevel);
			level.setFont(this.GUI.Font("Maiandra", 16));
			vitals.add(level, {
				anchor = this.GUI.GridLayout.LEFT
			});
			vitals.add(level);

			if (false == miniVersion)
			{
				vitals.add(this._buildType(), {
					anchor = this.GUI.GridLayout.RIGHT
				});
			}

			local value;

			if (!hideValue)
			{
				if (this.mValueType == this.CurrencyCategory.COPPER)
				{
					value = this.GUI.Currency();
					value.setCurrentValue(showbuyvalue ? this.getBuyValue() : this.getSellValue());
					value.setAlignment(0);
				}
				else
				{
					value = this.GUI.Credits();
					value.setCurrentValue(this.getBuyValue());
				}
			}
			else
			{
				value = this.GUI.Spacer(0, 0);
			}

			value.setFont(this.GUI.Font("Maiandra", 16));
			vitals.add(value, {
				anchor = this.GUI.GridLayout.LEFT
			});

			if (false == miniVersion)
			{
				vitals.add(this._buildSubtype(), {
					anchor = this.GUI.GridLayout.RIGHT
				});
			}
		}
		else
		{
			vitals = this.GUI.Container(this.GUI.BoxLayoutV());
			local loading = this.GUI.Label(this.TXT("Loading"));
			vitals.add(loading);
		}

		return vitals;
	}

	function _addDivider( container )
	{
		container.add(this.GUI.PopupMenuDivider(0));
	}

	function _addNameRow( container )
	{
		container.add(this._buildName());
	}

	function _addClassRestrictionsLevelRow( container, restrictions, power )
	{
		local vitalsrow = this.GUI.Container(this.GUI.BoxLayout());
		vitalsrow.getLayoutManager().setGap(2);
		local ClassK = this.GUI.Component();
		ClassK.setAppearance(restrictions.knight ? "ClassButton/Green/K" : "ClassButton/Red/K");
		ClassK.setPreferredSize(10, 12);
		vitalsrow.add(ClassK);
		local ClassR = this.GUI.Component();
		ClassR.setAppearance(restrictions.rogue ? "ClassButton/Green/R" : "ClassButton/Red/R");
		ClassR.setPreferredSize(10, 12);
		vitalsrow.add(ClassR);
		local ClassM = this.GUI.Component();
		ClassM.setAppearance(restrictions.mage ? "ClassButton/Green/M" : "ClassButton/Red/M");
		ClassM.setPreferredSize(10, 12);
		vitalsrow.add(ClassM);
		local ClassD = this.GUI.Component();
		ClassD.setAppearance(restrictions.druid ? "ClassButton/Green/D" : "ClassButton/Red/D");
		ClassD.setPreferredSize(10, 12);
		vitalsrow.add(ClassD);
		local levellabel = this.GUI.Label(" " + this.TXT("Level") + " " + this.mLevel + " ");
		levellabel.setFont(this.GUI.Font("Maiandra", 16, true));
		vitalsrow.add(levellabel);
		container.add(vitalsrow);
	}

	function _buildName()
	{
		local namelabel = this.GUI.Label(this.mDisplayName);
		namelabel.setSize(50, 30);
		namelabel.setPreferredSize(50, 30);
		namelabel.setFont(this.GUI.Font("Maiandra", 16, true));
		namelabel.setAutoFit(true);
		local color = this.Colors.white;

		switch(this.mQualityLevel)
		{
		case this.QualityLevel.POOR:
			color = this.Colors["Item Grey"];
			break;

		case this.QualityLevel.STANDARD:
			color = this.Colors["Item White"];
			break;

		case this.QualityLevel.GOOD:
			color = this.Colors["Item Green"];
			break;

		case this.QualityLevel.SUPERIOR:
			color = this.Colors["Item Blue"];
			break;

		case this.QualityLevel.EPIC:
			color = this.Colors["Item Purple"];
			break;

		case this.QualityLevel.LEGENDARY:
			color = this.Colors["Item Yellow"];
			break;

		case this.QualityLevel.ARTIFACT:
			color = this.Colors["Item Orange"];
			break;
		}

		namelabel.setFontColor(color);
		return namelabel;
	}

	function _buildOwnershipRestriction( container )
	{
		if (this.mOwnershipRestriction > 0)
		{
			container.add(this.GUI.HTML("<b>Max Allowed: " + this.mOwnershipRestriction + "</b>"));
			this._addDivider(container);
		}
	}

	function _buildType()
	{
		return this.GUI.Label(this.ItemTypeNames[this.mType]);
	}

	function _buildSubtype()
	{
		switch(this.mType)
		{
		case this.ItemType.WEAPON:
			return this.GUI.Label(this.WeaponTypeNames[this.mWeaponType]);
			break;

		case this.ItemType.ARMOR:
			return this.GUI.Label("");
			break;
		}

		return this.GUI.Label("");
	}

	function _addBindingRow( component, item )
	{
		local binding = this._buildBinding(item);

		if (binding)
		{
			component.add(binding);
		}

		return binding;
	}

	function _buildBinding( item )
	{
		local label;

		if (item && ("mItemData" in item) && item.mItemData.mBound)
		{
			label = this.GUI.HTML();
			label.setText("<font color=\"FF0000\"><b>" + this.TXT("Bound to character") + "</b></font>");
		}
		else
		{
			switch(this.mBindingType)
			{
			case this.ItemBindingType.BIND_ON_PICKUP:
				label = this.GUI.HTML();
				label.setText("<font color=\"F0BBFF\"><b>" + this.TXT("Bind") + ":</b> " + this.TXT("Pickup") + "</font>");
				break;

			case this.ItemBindingType.BIND_ON_EQUIP:
				label = this.GUI.HTML();
				label.setText("<font color=\"F0BBFF\"><b>" + this.TXT("Bind") + ":</b> " + this.TXT("Equip") + "</font>");
				break;
			}
		}

		return label;
	}

	function _addQuestSection( container )
	{
		if (this.mSv1 != "")
		{
			local questlabel = this.GUI.HTML(this.mSv1);
			container.add(questlabel);
			this._addDivider(container);
		}
	}

	function _addCraftSection( container )
	{
		local keyHTML = this.GUI.HTML("<font color=\"" + this.Colors.teal + "\"><b>Key Component<br></font>" + ::_ItemDataManager.getItemDef(this.mKeyComponent).getDisplayName() + "<br/>");
		container.add(keyHTML);
		this._addDivider(container);
		local compTitleHTML = this.GUI.HTML("<font color=\"" + this.Colors.purple + "\"><b>Components</font>");
		container.add(compTitleHTML);

		foreach( id, count in this.mCraftComponents )
		{
			local craftCompHTML = this.GUI.HTML(count + "  " + ::_ItemDataManager.getItemDef(id).getDisplayName());
			container.add(craftCompHTML);
		}

		this._addDivider(container);
		local itemCraftHTML = this.GUI.HTML("<font color=\"" + this.Colors.cyan + "\"><b>Item crafted    </font>" + ::_ItemDataManager.getItemDef(this.mResultItem).getDisplayName());
		container.add(itemCraftHTML);
	}

	function _addArmorSection( container, item )
	{
		if (this.mBindingType != this.ItemBindingType.BIND_NEVER || this.mEquipType == this.ItemEquipType.ARMOR_RING_UNIQUE)
		{
			local equipunique = this.GUI.HTML();

			if (this.mEquipType == this.ItemEquipType.ARMOR_RING_UNIQUE)
			{
				equipunique.setText("<font color=\"FFFF00\"><b>" + this.TXT("Equip Unique") + "</font>");
			}
			else
			{
				equipunique = null;
				  // [031]  OP_JMP            0      0    0    0
			}

			local bindsection;

			if (equipunique != null)
			{
				bindsection = this.GUI.Container(this.GUI.GridLayout(1, 2));
			}
			else
			{
				bindsection = this.GUI.Container(this.GUI.GridLayout(1, 1));
			}

			local binding = this._buildBinding(item);

			if (binding == null)
			{
				binding = this.GUI.Container();
			}

			bindsection.add(binding);

			if (equipunique != null)
			{
				bindsection.add(equipunique, {
					anchor = this.GUI.GridLayout.RIGHT
				});
			}

			container.add(bindsection);
		}

		local bindtypec = this.GUI.Container(this.GUI.GridLayout(1, 2));
		local bindloc = this.GUI.Label();

		switch(this.mEquipType)
		{
		case this.ItemEquipType.ARMOR_SHIELD:
			bindloc.setText(this.TXT("Shield"));
			break;

		case this.ItemEquipType.ARMOR_HEAD:
			bindloc.setText(this.TXT("Head"));
			break;

		case this.ItemEquipType.ARMOR_NECK:
			bindloc.setText(this.TXT("Neck"));
			break;

		case this.ItemEquipType.ARMOR_SHOULDER:
			bindloc.setText(this.TXT("Shoulder"));
			break;

		case this.ItemEquipType.ARMOR_CHEST:
			bindloc.setText(this.TXT("Chest"));
			break;

		case this.ItemEquipType.ARMOR_ARMS:
			bindloc.setText(this.TXT("Arms"));
			break;

		case this.ItemEquipType.ARMOR_HANDS:
			bindloc.setText(this.TXT("Hands"));
			break;

		case this.ItemEquipType.ARMOR_WAIST:
			bindloc.setText(this.TXT("Waist"));
			break;

		case this.ItemEquipType.ARMOR_LEGS:
			bindloc.setText(this.TXT("Legs"));
			break;

		case this.ItemEquipType.ARMOR_FEET:
			bindloc.setText(this.TXT("Feet"));
			break;

		case this.ItemEquipType.ARMOR_RING:
		case this.ItemEquipType.ARMOR_RING_UNIQUE:
			bindloc.setText(this.TXT("Ring"));
			break;

		case this.ItemEquipType.ARMOR_AMULET:
			bindloc.setText(this.TXT("Amulet"));
			break;
		}

		bindtypec.add(bindloc);
		container.add(bindtypec);
		local ratinglabel = this.GUI.HTML("<b>" + this.TXT("Armor") + ":</b> " + this.mArmorResistMelee);
		container.add(ratinglabel);
	}

	function _addWeaponSection( container, item )
	{
		if (this.mBindingType != this.ItemBindingType.BIND_NEVER || this.mEquipType == this.ItemEquipType.WEAPON_1H_UNIQUE)
		{
			local bindsection = this.GUI.Container(this.GUI.GridLayout(1, 2));
			local binding = this._buildBinding(item);

			if (binding == null)
			{
				binding = this.GUI.Container();
			}

			bindsection.add(binding);
			local equipunique = this.GUI.HTML();

			if (this.mEquipType == this.ItemEquipType.WEAPON_1H_UNIQUE)
			{
				equipunique.setText("<font color=\"FFFF00\"><b>" + this.TXT("Equip Unique") + "</font>");
			}
			else
			{
			}

			bindsection.add(equipunique, {
				anchor = this.GUI.GridLayout.RIGHT
			});
			container.add(bindsection);
		}

		if (this.mWeaponType != this.WeaponType.ARCANE_TOTEM)
		{
			local typesection = this.GUI.Container(this.GUI.GridLayout(1, 2));
			local weapontypelabel = this.GUI.HTML("<b>" + this.WeaponTypeNames[this.mWeaponType] + "</b>");
			typesection.add(weapontypelabel);
			local handlabel = this.GUI.HTML();

			switch(this.mEquipType)
			{
			case this.ItemEquipType.WEAPON_1H_MAIN:
				handlabel.setText("<font color=\"FFFF00\"><b>" + this.TXT("Main Hand Only") + "</font>");
				break;

			case this.ItemEquipType.WEAPON_1H_OFF:
				handlabel.setText("<font color=\"FFFF00\"><b>" + this.TXT("Off Hand Only") + "</font>");
				break;
			}

			typesection.add(handlabel, {
				anchor = this.GUI.GridLayout.RIGHT
			});
			container.add(typesection);
			local statssection = this.GUI.Container(this.GUI.GridLayout(1, 1));
			local damagelabel = this.GUI.HTML("<b>" + this.TXT("Damage") + ":</b> " + this.mWeaponDamageMin + " - " + this.mWeaponDamageMax);
			statssection.add(damagelabel);
			container.add(statssection);

			if (this.mWeaponExtraDamangeRating > 0)
			{
				local additionaldamagelabel = this.GUI.HTML();

				switch(this.mWeaponExtraDamageType)
				{
				case this.DamageType.MELEE:
					additionaldamagelabel.setText("<b>" + this.TXT("Additional Damage") + ":</b> +" + this.mWeaponExtraDamangeRating);
					break;

				case this.DamageType.FIRE:
					additionaldamagelabel.setText("<b>" + this.TXT("Additional Damage") + ":</b> +" + this.mWeaponExtraDamangeRating + " <font color=\"FF0000\">" + this.TXT("Fire") + "</font>");
					break;

				case this.DamageType.FROST:
					additionaldamagelabel.setText("<b>" + this.TXT("Additional Damage") + ":</b> +" + this.mWeaponExtraDamangeRating + " <font color=\"FFFFFF\">" + this.TXT("Frost") + "</font>");
					break;

				case this.DamageType.MYSTIC:
					additionaldamagelabel.setText("<b>" + this.TXT("Additional Damage") + ":</b> +" + this.mWeaponExtraDamangeRating + " <font color=\"55DDFF\">" + this.TXT("Mystic") + "</font>");
					break;

				case this.DamageType.DEATH:
					additionaldamagelabel.setText("<b>" + this.TXT("Additional Damage") + ":</b> +" + this.mWeaponExtraDamangeRating + " <font color=\"A0A0A0\">" + this.TXT("Death") + "</font>");
					break;
				}

				container.add(additionaldamagelabel);
			}
		}
	}

	function _addBonusSection( container )
	{
		local found = false;
		local bonuses = {
			Strength = this.mBonusStrength,
			Dexterity = this.mBonusDexterity,
			Constitution = this.mBonusConstitution,
			Psyche = this.mBonusPsyche,
			Spirit = this.mBonusSpirit
		};

		if (this.mType == this.ItemType.CHARM)
		{
			local foundBonus = false;

			foreach( name, amount in bonuses )
			{
				if (amount > 0)
				{
					local effect = "";

					if (foundBonus == false)
					{
						foundBonus = true;
						effect = this.GUI.HTML("<b>First charm : +15%<br/>Second charm +10%<br/>Third charm +5%</b>");
						container.add(effect);
					}

					effect = this.GUI.HTML("<font color=\"55DDFF\"><b>+" + this.TXT(name) + "</b></font>");
					container.add(effect);
					found = true;
				}
			}
		}
		else
		{
			foreach( name, amount in bonuses )
			{
				if (amount > 0)
				{
					local effect = this.GUI.HTML("<font color=\"55DDFF\"><b>+" + amount + " " + this.TXT(name) + "</b></font>");
					container.add(effect);
					found = true;
				}
			}
		}

		return found;
	}

	function _addCharmBonusSection( container )
	{
		local found = false;
		local bonuses = {
			["To Hit Melee"] = this.mMeleeHitMod,
			["To Crit Melee"] = this.mMeleeCritMod,
			["To Hit Magic"] = this.mMagicHitMod,
			["To Crit Magic"] = this.mMagicCritMod,
			["To Parry"] = this.mParryMod,
			["To Block"] = this.mBlockMod,
			["Run Speed"] = this.mRunSpeedMod,
			["Regen Health"] = this.mRegenHealthMod,
			["Attack Speed Melee"] = this.mAttackSpeedMod,
			["Casting Speed"] = this.mCastSpeedMod,
			Healing = this.mHealingMod
		};

		foreach( name, amount in bonuses )
		{
			if (amount > 0)
			{
				local effect = this.GUI.HTML("<font color=\"55DDFF\"><b>+" + this.TXT(name) + "</b></font>");
				container.add(effect);
				found = true;
			}
		}

		return found;
	}

	function _addFlavorText( container )
	{
		local flavorText = this.GUI.HTML("<font color=\"FFFFFF\"><i><b>" + this.mFlavorText + "</b></i></font>");
		flavorText.setMaximumSize(200, 300);
		flavorText.setResize(true);
		container.add(flavorText);
	}

	function _addLifetimeValue( container )
	{
		local hourTime = 0;

		if (this.mIvType1 == this.ItemIntegerType.LIFETIME)
		{
			hourTime = this.mIvMax1;
		}

		if (this.mIvType2 == this.ItemIntegerType.LIFETIME)
		{
			hourTime = this.mIvMax2;
		}

		if (hourTime != 0)
		{
			local timeText = "Item Lifetime: ";

			if (hourTime == -1)
			{
				timeText = "Forever";
			}
			else
			{
				timeText = ::Util.parseHourToTimeStr(hourTime);
			}

			local timeLabel = this.GUI.HTML("<font color=\"" + this.Colors.lavender + "\"><b>Item Lifetime: </b></font><i>" + timeText + "</i>");
			timeLabel.setData("LIFETIME_LABEL");
			container.add(timeLabel);
		}
	}

	function _addSpecialItemTypeSection( container )
	{
		if (this.mSpecialItemType == this.SpecialItemType.NONE)
		{
			return;
		}

		local bonusValue = 0;

		if (this.mIvType1 == this.ItemIntegerType.BONUS_VALUE)
		{
			bonusValue = this.mIvMax1;
		}

		if (this.mIvType2 == this.ItemIntegerType.BONUS_VALUE)
		{
			bonusValue = this.mIvMax2;
		}

		local specialText = {
			[this.SpecialItemType.REAGENT_GENERATOR] = {
				text = "Replaces the need to carry reagents that are required to use certain abilities. " + "This device takes the cost of the reagents directly from your available coin. "
			},
			[this.SpecialItemType.ITEM_GRINDER] = {
				text = "When destroying items you recieve " + bonusValue + "% of the store selling value in coin in return."
			},
			[this.SpecialItemType.XP_BOOST] = {
				text = "Grants a " + bonusValue + "% boost to earned experience when killing enemies."
			}
		};

		if (this.mSpecialItemType in specialText)
		{
			this._addDivider(container);
			local specialItemHTML = this.GUI.HTML("<font color=\"FFFFFF\"><b>" + specialText[this.mSpecialItemType].text + "</b></font>");
			specialItemHTML.setMaximumSize(200, 400);
			specialItemHTML.setMinimumSize(200, 100);
			specialItemHTML.setResize(true);
			container.add(specialItemHTML);
		}
	}

	function _addEffectSection( container )
	{
		local found = false;

		if (this.mEquipEffectId != 0 && this.mEquipEffectId != "")
		{
			local EquipEffect = this.GUI.HTML("<font color=\"FFD080\"><b>" + this.TXT("Equip") + ":</b> " + this.mEquipEffectId + "</font>");
			EquipEffect.setMaximumSize(200, 200);
			EquipEffect.setResize(true);
			container.add(EquipEffect);
			found = true;
		}

		if (this.mUseAbilityId != 0 && this.mUseAbilityId != "")
		{
			local UseEffect = this.GUI.HTML("<font color=\"FFD080\"><b>" + this.TXT("Use") + ":</b> " + this.mUseAbilityId + "</font>");
			UseEffect.setMaximumSize(200, 200);
			UseEffect.setResize(true);
			container.add(UseEffect);
			found = true;
		}

		if (this.mActionAbilityId != 0 && this.mActionAbilityId != "")
		{
			local ActionEffect = this.GUI.HTML("<font color=\"FFD080\"><b>" + this.TXT("Action") + ":</b> " + this.mActionAbilityId + "</font>");
			ActionEffect.setMaximumSize(200, 200);
			ActionEffect.setResize(true);
			ActionEffect.validate();
			container.add(ActionEffect);
			found = true;
		}

		return found;
	}

	function _addValueRow( container, showbuyvalue )
	{
		if (this.mValueType == this.CurrencyCategory.COPPER)
		{
			local value = this.GUI.Currency(showbuyvalue ? this.getBuyValue() : this.getSellValue());
			value.setFont(this.GUI.Font("Maiandra", 16));
			value.setAlignment(1);
			container.add(value);
		}
	}

	function _addDurabilityRow( container )
	{
		local durability = this.getDynamicMax(this.ItemIntegerType.DURABILITY);
		local durabilitylabel = this.GUI.HTML("<b>Durability:</b> " + durability + "/" + durability);
		container.add(durabilitylabel);
	}

	function _addCharmSection( container, item )
	{
		local binding = this._buildBinding(item);

		if (binding)
		{
			container.add(binding);
		}

		local charmType = this.GUI.HTML();

		switch(this.mEquipType)
		{
		case this.ItemEquipType.RED_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.red + "\">Red</font>");
			break;

		case this.ItemEquipType.BLUE_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.blue + "\">Blue</font>");
			break;

		case this.ItemEquipType.YELLOW_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.yellow + "\">Yellow</font>");
			break;

		case this.ItemEquipType.ORANGE_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.orange + "\">Orange</font>");
			break;

		case this.ItemEquipType.PURPLE_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.purple + "\">Purple</font>");
			break;

		case this.ItemEquipType.GREEN_CHARM:
			charmType.setText("<b>" + this.TXT("Charm") + ":</b> <font color=\"" + this.Colors.green + "\">Green</font>");
			break;
		}

		container.add(charmType);
	}

	function _addContainerSection( container )
	{
		local containerlabel = this.GUI.HTML("<b>" + this.TXT("Container") + ":</b> " + this.mContainerSlots + " " + this.TXT("Slots"));
		container.add(containerlabel);
	}

	function _getPrice()
	{
		local price;
		price = this.mValue;
		return price;
	}

	function getContainerSlots()
	{
		return this.mContainerSlots;
	}

	function getBuyValue()
	{
		if (this.mValueType == this.CurrencyCategory.COPPER)
		{
			return this._getPrice() * this.gVendorMarkup;
		}
		else if (this.mValueType == this.CurrencyCategory.CREDITS)
		{
			return this.mValue;
		}
	}

	function getSellValue()
	{
		if (this.mValueType == this.CurrencyCategory.COPPER)
		{
			return this._getPrice();
		}
		else if (this.mValueType == this.CurrencyCategory.CREDITS)
		{
			return 0;
		}
	}

	function getWeaponPower()
	{
		local power = 0;
		power += this.mWeaponDamageMin * 0.1;
		power += this.mWeaponDamageMax * 0.1;
		power += this.mWeaponSpeed * 0.050000001;
		power += this.mWeaponExtraDamangeRating * 0.050000001;
		return power;
	}

	function getArmorPower()
	{
		local power = 0;
		power += this.mArmorResistMelee * 0.1;
		power += this.mArmorResistFire * 0.050000001;
		power += this.mArmorResistFrost * 0.050000001;
		power += this.mArmorResistMystic * 0.050000001;
		power += this.mArmorResistDeath * 0.050000001;
		return power;
	}

	function getFocusPower()
	{
		local power = 0;
		power += this.getDynamicMax(0) * 0.30000001;
		return power;
	}

	function getDynamicMax( type )
	{
		if (!this.mValid)
		{
			return null;
		}

		if (this.mIvType1 == type)
		{
			return this.mIvMax1;
		}

		if (this.mIvType2 == type)
		{
			return this.mIvMax2;
		}

		return null;
	}

	function getDynamicValue( type, itemData )
	{
		if (!this.mValid)
		{
			return null;
		}

		if (this.mIvType1 == type)
		{
			return itemData.mIv1;
		}

		if (this.mIvType2 == type)
		{
			return itemData.mIv2;
		}

		return null;
	}

	function getDamagePerSecond()
	{
		local avgdamage = (this.mWeaponDamageMax - this.mWeaponDamageMin) * 0.5 + this.mWeaponDamageMin;
		return avgdamage / this.mWeaponSpeed + this.mWeaponExtraDamangeRating;
	}

	function getClassRestrictions()
	{
		switch(this.mType)
		{
		case this.ItemType.WEAPON:
			return this.WeaponTypeClassRestrictions[this.mWeaponType];

		case this.ItemType.ARMOR:
			return this.ArmorTypeClassRestrictions[this.mArmorType];
		}

		return {
			none = false,
			knight = true,
			mage = true,
			rogue = true,
			druid = true
		};
	}

	function getResultItem()
	{
		return this.mResultItem;
	}

	function getKeyComponent()
	{
		return this.mKeyComponent;
	}

	function getCraftComponent()
	{
		return this.mCraftComponents;
	}

	function getSpecialItemType()
	{
		return this.mSpecialItemType;
	}

	function getLevel()
	{
		return this.mLevel;
	}

	function getMinUseLevel()
	{
		return this.mMinUseLevel;
	}

	function getQualityLevel()
	{
		return this.mQualityLevel;
	}

	function getBindingType()
	{
		return this.mBindingType;
	}

}

