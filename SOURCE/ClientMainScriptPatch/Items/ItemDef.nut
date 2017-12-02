/**
	Defines all of the attributes of any item.  If we need to, we could probably get smart
	about it and split off the weapon and armor into a weapondef and armordef, but that would
	required {@link _handleItemDefUpdateMsg} to figure out what type of item it is
	before assigning values.
*/

class ItemDefData {
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
	
	//Charm specific
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
	
	mValue = -1; // < 0 means auto-calculate based on level, etc
	mValueType = CurrencyCategory.COPPER;
	
	mFlavorText = "";
	
	mResultItem = 0;
	mKeyComponent = 0;
	mCraftComponents = null;
	mSpecialItemType = SpecialItemType.NONE;
	mQualityLevel = QualityLevel.POOR;
	mMinUseLevel = -1;
	mOwnershipRestriction = 0;
	mValid = false;
	mTooltipComponent = null;
	mLastShowBuyValue = null;
	
	constructor( id ) {
		mID = id;
		mCraftComponents = {};
	}

	function getID() {
		return mID;
	}

	function getType() {
		return mType;
	}

	function getDisplayName() {
		return mDisplayName;
	}

	/**
		Returns the name of the quest associated with the item.
	*/
	function getQuestName() {
		return mSv1;
	}

	function getIcon() {
		return mIcon;
	}

	function isValid() {
		return mValid;
	}

	function isUsable() {
		return mValid && mUseAbilityId != 0;
	}

	function getEquipType() {
		return mEquipType;
	}

	function getWeaponType() {
		return mWeaponType;
	}

	function getArmorType() {
		return mArmorType;
	}

	function setAppearance( appearance ) {
		if (typeof appearance == "array") {
			local allNull = true;

			foreach( a in appearance ) {
				if (a != null) {
					allNull = false;
					break;
				}
			}

			if (allNull == true)
				appearance = null;
		}

		mAppearance = appearance;
	}

	function getAppearance() {
		if (typeof mAppearance == "array") {
			local allNull = true;

			foreach( a in mAppearance ) {
				if (a != null) {
					allNull = false;
					break;
				}
			}

			if (allNull == true)
				mAppearance = null;
		}

		return mAppearance;
	}

	
	/**
		Returns useful information about this item for display to the user.
	*/
	function getTooltip( tooltipMods, ... ) {
		local force = false;
		local optionalComponent;
		local item;
		local showBindingInfo = true;
		local showBuyValue = false;
		local mods = tooltipMods;

		if (mods != null) {
			if ("showBuyValue" in mods)
				showBuyValue = mods.showBuyValue;

			if ("showBindingInfo" in mods)
				showBindingInfo = mods.showBindingInfo;
		}

		if (vargc > 0 && vargv[0] == true)
			force = true;

		force = true;

		if (vargc > 1)
			optionalComponent = vargv[1];

		if (vargc > 2)
			item = vargv[2];

		if ((!mTooltipComponent || force || mLastShowBuyValue != showBuyValue) && mValid == true) {
			mLastShowBuyValue = showBuyValue;
			mTooltipComponent = GUI.Container(GUI.BoxLayoutV());
			mTooltipComponent.getLayoutManager().setExpand(true);
			renderTooltip(mTooltipComponent, showBuyValue, optionalComponent, item, showBindingInfo, mods);
		}
		else if (!mTooltipComponent) {
			mTooltipComponent = GUI.Container(GUI.BoxLayoutV());
			local loading = GUI.Label(TXT("Loading..."));
			loading.setFont(GUI.Font("Maiandra", 32, true));
			mTooltipComponent.add(loading);
		}

		return mTooltipComponent;
	}

	function renderTooltip( component, showbuyvalue, optionalComponent, item, ... ) {
		local showBindingInfo = true;
		local mods = [];
		local currentlyEquipped = false;

		if (vargc > 0)
			showBindingInfo = vargv[0];

		if (vargc > 1)
			mods = vargv[1];

		if ("CurrentlyEquipped" in mods)
			currentlyEquipped = mods.CurrentlyEquipped;

		switch(mType)
		{
		case ItemType.UNKNOWN:
		case ItemType.BASIC:
		case ItemType.SYSTEM:
			_addNameRow(component);

			if (showBindingInfo)
				_addBindingRow(component, item);

			_addDivider(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.SPECIAL:
			_addNameRow(component);
			
			if(getDynamicMax(ItemIntegerType.BOOK_PAGE) != null) {
				_addPageRow(component);
			}

			if (showBindingInfo)
				_addBindingRow(component, item);

			_addDivider(component);

			if (optionalComponent)
				component.add(optionalComponent);

			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			_addSpecialItemTypeSection(component);
			break;

		case ItemType.WEAPON:
			if (currentlyEquipped) {
				component.add(GUI.Label("Currently Equipped"));
				_addDivider(component);
			}

			_addNameRow(component);
			_addClassRestrictionsLevelRow(component, getClassRestrictions(), getWeaponPower());
			_addDivider(component);
			_addWeaponSection(component, item);
			_addDivider(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);

			if (_addBonusSection(component) || _addEffectSection(component))
				_addDivider(component);

			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.ARMOR:
			if (currentlyEquipped) {
				component.add(GUI.Label("Currently Equipped"));
				_addDivider(component);
			}

			_addNameRow(component);
			_addClassRestrictionsLevelRow(component, getClassRestrictions(), getArmorPower());
			_addDivider(component);
			_addArmorSection(component, item);
			_addDivider(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);

			if (_addBonusSection(component) || _addEffectSection(component))
				_addDivider(component);

			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.CHARM:
			if (currentlyEquipped) {
				component.add(GUI.Label("Currently Equipped"));
				_addDivider(component);
			}

			_addNameRow(component);
			_addClassRestrictionsLevelRow(component, getClassRestrictions(), 30);
			_addDivider(component);
			_addCharmSection(component, item);
			_addDivider(component);

			if (_addBonusSection(component))
				_addDivider(component);

			if (_addCharmBonusSection(component))
				_addDivider(component);

			if (_addEffectSection(component))
				_addDivider(component);

			if (optionalComponent)
				component.add(optionalComponent);

			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.CONSUMABLE:
			if (mUseAbilityId != 0 && mUseAbilityId != "") {
				local consumableMods = {};

				if (item && ("mItemData" in item) && item.mItemData.mBound)
					consumableMods.bind <- "bound";
				else
					consumableMods.bind <- mBindingType;

				if (!showBindingInfo)
					consumableMods.showBindingInfo <- false;

				local abilityComp = ::AbilityUtil.buildToolTipComponent(mUseAbilityId, consumableMods);
				component.add(abilityComp);
			}
			else {
				_addNameRow(component);

				if (showBindingInfo) {
					_addBindingRow(component, item);
				}

				_addDivider(component);

				if (_addEffectSection(component)) {
					_addDivider(component);
				}

				_buildOwnershipRestriction(component);
				_addLifetimeValue(component);
				_addValueRow(component, showbuyvalue);

				if (mFlavorText != "") {
					_addDivider(component);
					_addFlavorText(component);
				}
			}

			break;

		case ItemType.CONTAINER:
			if (currentlyEquipped) {
				component.add(GUI.Label("Currently Equipped"));
				_addDivider(component);
			}

			_addNameRow(component);
			_addDivider(component);

			if (showBindingInfo) {
				_addBindingRow(component, item);
			}

			_addContainerSection(component);
			_addDivider(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.QUEST:
			_addNameRow(component);
			_addDivider(component);

			if (showBindingInfo) {
				_addBindingRow(component, item);
			}

			_addQuestSection(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			break;

		case ItemType.RECIPE:
			_addNameRow(component);
			_addClassRestrictionsLevelRow(component, getClassRestrictions(), mLevel);
			_addDivider(component);

			if (showBindingInfo) {
				_addBindingRow(component, item);
			}

			_addCraftSection(component);
			_addDivider(component);
			_buildOwnershipRestriction(component);
			_addLifetimeValue(component);
			_addValueRow(component, showbuyvalue);

			if (mFlavorText != "") {
				_addDivider(component);
				_addFlavorText(component);
			}

			local result_item = ::_ItemDataManager.getItemDef(mResultItem);

			if (result_item)
				result_item.renderTooltip(component, showbuyvalue, null, null, null);

			break;
		}
	}

	function _isObjectiveItemComplete() {
		local questId = -1;

		if (mIvType1 == ItemIntegerType.QUEST_ID) {
			questId = mIvMax1;
		}
		else if (mIvType2 == ItemIntegerType.QUEST_ID) {
			questId = mIvMax2;
		}

		if (questId) {
			local questData = ::_questManager.getPlayerQuestDataById(questId);

			if (questData) {
				local questObjectives = questData.getObjectives();

				foreach( objectiveData in questObjectives ) {
					if (objectiveData.getItemId() == getID() || objectiveData.getCreatureDefId() == getID()) {
						if (objectiveData.isCompleted())
							return true;
						else
							return false;
					}
				}
			}
		}

		return false;
	}

		
	/**
		Creates a component containing the most basic information about this Item Def.
		@param showbuyvalue - true to show the buy value, false to hide it
		@param miniVersion - true to show the smaller version of the info panel, false to
						     show the full sized version.
		@pararm hideValue - true to hide any buy/sell value associated with the item.  False to
							display it (default)
	*/
	function getInfoPanel( showbuyvalue, miniVersion, hideValue ) {
		local vitals;

		if (mValid == true) {
			if (true == miniVersion) {
				vitals = GUI.Container(GUI.GridLayout(3, 1));
				vitals.getLayoutManager().setColumns(80);
				vitals.add(_buildName(), {
					anchor = GUI.GridLayout.LEFT
				});
			}
			else {
				vitals = GUI.Container(GUI.GridLayout(3, 2));
				vitals.getLayoutManager().setColumns(102, 40);
				vitals.add(_buildName(), {
					span = 2,
					anchor = GUI.GridLayout.LEFT
				});
			}

			local level = GUI.Label(TXT("Level") + " " + mLevel);
			level.setFont(GUI.Font("Maiandra", 16));
			vitals.add(level, {
				anchor = GUI.GridLayout.LEFT
			});
			vitals.add(level);

			if (false == miniVersion) {
				vitals.add(_buildType(), {
					anchor = GUI.GridLayout.RIGHT
				});
			}

			local value;

			if (!hideValue) {
				if (mValueType == CurrencyCategory.COPPER) {
					value = GUI.Currency();
					value.setCurrentValue(showbuyvalue ? getBuyValue() : getSellValue());
					value.setAlignment(0);
				}
				else {
					value = GUI.Credits();
					value.setCurrentValue(getBuyValue());
				}
			}
			else {
				value = GUI.Spacer(0, 0);
			}

			value.setFont(GUI.Font("Maiandra", 16));
			vitals.add(value, {
				anchor = GUI.GridLayout.LEFT
			});

			if (false == miniVersion) {
				vitals.add(_buildSubtype(), {
					anchor = GUI.GridLayout.RIGHT
				});
			}
		}
		else {
			vitals = GUI.Container(GUI.BoxLayoutV());
			local loading = GUI.Label(TXT("Loading"));
			vitals.add(loading);
		}

		return vitals;
	}
	
	/** 
		Adds a horizontal break to the container.
	*/
	function _addDivider( container ) {
		container.add(GUI.PopupMenuDivider(0));
	}

	/** 
		Add a name row to the container
	*/
	function _addNameRow( container ) {
		container.add(_buildName());
	}

	/** 
		Add a page row to the container
	*/
	function _addPageRow( container ) {
		container.add(_buildPage());
	}

	/**
		Creates a row in the container displaying the class restrictions, and level.
	*/
	function _addClassRestrictionsLevelRow( container, restrictions, power ) {
		local vitalsrow = GUI.Container(GUI.BoxLayout());
		vitalsrow.getLayoutManager().setGap(2);
		local ClassK = GUI.Component();
		ClassK.setAppearance(restrictions.knight ? "ClassButton/Green/K" : "ClassButton/Red/K");
		ClassK.setPreferredSize(10, 12);
		vitalsrow.add(ClassK);
		local ClassR = GUI.Component();
		ClassR.setAppearance(restrictions.rogue ? "ClassButton/Green/R" : "ClassButton/Red/R");
		ClassR.setPreferredSize(10, 12);
		vitalsrow.add(ClassR);
		local ClassM = GUI.Component();
		ClassM.setAppearance(restrictions.mage ? "ClassButton/Green/M" : "ClassButton/Red/M");
		ClassM.setPreferredSize(10, 12);
		vitalsrow.add(ClassM);
		local ClassD = GUI.Component();
		ClassD.setAppearance(restrictions.druid ? "ClassButton/Green/D" : "ClassButton/Red/D");
		ClassD.setPreferredSize(10, 12);
		vitalsrow.add(ClassD);
		local levellabel = GUI.Label(" " + TXT("Level") + " " + mLevel + " ");
		levellabel.setFont(GUI.Font("Maiandra", 16, true));
		vitalsrow.add(levellabel);
		container.add(vitalsrow);
	}
	
	/**
		Builds a single label indicating the page this book is for
	*/
	function _buildPage() {
		local pagelabel = GUI.Label("Page: " + mIvMax2);
		pagelabel.setSize(50, 30);
		pagelabel.setPreferredSize(50, 30);
		pagelabel.setFont(GUI.Font("Maiandra", 14, true));
		pagelabel.setAutoFit(true);
		return pagelabel;
	}
	
	/**
		Builds a single label indicating the name
	*/
	function _buildName() {
		local namelabel = GUI.Label(mDisplayName);
		namelabel.setSize(50, 30);
		namelabel.setPreferredSize(50, 30);
		namelabel.setFont(GUI.Font("Maiandra", 16, true));
		namelabel.setAutoFit(true);
		local color = Colors.white;

		switch(mQualityLevel) {
		case QualityLevel.POOR:
			color = Colors["Item Grey"];
			break;

		case QualityLevel.STANDARD:
			color = Colors["Item White"];
			break;

		case QualityLevel.GOOD:
			color = Colors["Item Green"];
			break;

		case QualityLevel.SUPERIOR:
			color = Colors["Item Blue"];
			break;

		case QualityLevel.EPIC:
			color = Colors["Item Purple"];
			break;

		case QualityLevel.LEGENDARY:
			color = Colors["Item Yellow"];
			break;

		case QualityLevel.ARTIFACT:
			color = Colors["Item Orange"];
			break;
		}

		namelabel.setFontColor(color);
		return namelabel;
	}

	function _buildOwnershipRestriction( container ) {
		if (mOwnershipRestriction > 0) {
			container.add(GUI.HTML("<b>Max Allowed: " + mOwnershipRestriction + "</b>"));
			_addDivider(container);
		}
	}
	
	/**
		Creates a single label indicating the type
	*/
	function _buildType() {
		return GUI.Label(ItemTypeNames[mType]);
	}
	
	/**
		Creates a single label indicating the sub-type.
	*/
	function _buildSubtype() {
		switch(mType)
		{
		case ItemType.WEAPON:
			return GUI.Label(WeaponTypeNames[mWeaponType]);
			break;

		case ItemType.ARMOR:
			return GUI.Label("");
			break;
		}

		return GUI.Label("");
	}

	function _addBindingRow( component, item ) {
		local binding = _buildBinding(item);

		if (binding)
			component.add(binding);

		return binding;
	}
	
	/**
		Creates a label for the binding type
	*/
	function _buildBinding( item ) {
		local label;

		if (item && ("mItemData" in item) && item.mItemData.mBound)	{
			label = GUI.HTML();
			label.setText("<font color=\"FF0000\"><b>" + TXT("Bound to character") + "</b></font>");
		}
		else {
			switch(mBindingType)
			{
			case ItemBindingType.BIND_ON_PICKUP:
				label = GUI.HTML();
				label.setText("<font color=\"F0BBFF\"><b>" + TXT("Bind") + ":</b> " + TXT("Pickup") + "</font>");
				break;

			case ItemBindingType.BIND_ON_EQUIP:
				label = GUI.HTML();
				label.setText("<font color=\"F0BBFF\"><b>" + TXT("Bind") + ":</b> " + TXT("Equip") + "</font>");
				break;
			}
		}

		return label;
	}
		
	/**
		Creates the quest section of the tooltip, including the quest name.
	*/
	function _addQuestSection( container ) {
		if (mSv1 != "")	{
			local questlabel = GUI.HTML(mSv1);
			container.add(questlabel);
			_addDivider(container);
		}
	}

	function _addCraftSection( container ) {
		local keyHTML = GUI.HTML("<font color=\"" + Colors.teal + "\"><b>Key Component<br></font>" + ::_ItemDataManager.getItemDef(mKeyComponent).getDisplayName() + "<br/>");
		container.add(keyHTML);
		_addDivider(container);
		local compTitleHTML = GUI.HTML("<font color=\"" + Colors.purple + "\"><b>Components</font>");
		container.add(compTitleHTML);

		foreach( id, count in mCraftComponents ) {
			local craftCompHTML = GUI.HTML(count + "  " + ::_ItemDataManager.getItemDef(id).getDisplayName());
			container.add(craftCompHTML);
		}

		_addDivider(container);
		local itemCraftHTML = GUI.HTML("<font color=\"" + Colors.cyan + "\"><b>Item crafted    </font>" + ::_ItemDataManager.getItemDef(mResultItem).getDisplayName());
		container.add(itemCraftHTML);
	}
	
	/**
		Creates the armor section of the tooltip, including weapon and armor
		bind details and strength.  It also builds the weapon resistance section,
		which displays resistance against elements such as fire, ice, and Greg.
	*/
	function _addArmorSection( container, item )
	{
		if (mBindingType != ItemBindingType.BIND_NEVER || mEquipType == ItemEquipType.ARMOR_RING_UNIQUE)
		{
			local equipunique = GUI.HTML();

			if (mEquipType == ItemEquipType.ARMOR_RING_UNIQUE)
			{
				equipunique.setText("<font color=\"FFFF00\"><b>" + TXT("Equip Unique") + "</font>");
			}
			else
			{
				equipunique = null;
				  // [031]  OP_JMP            0      0    0    0
			}

			local bindsection;

			if (equipunique != null)
				bindsection = GUI.Container(GUI.GridLayout(1, 2));
			else
				bindsection = GUI.Container(GUI.GridLayout(1, 1));

			local binding = _buildBinding(item);

			if (binding == null)
				binding = GUI.Container();

			bindsection.add(binding);

			if (equipunique != null) {
				bindsection.add(equipunique, {
					anchor = GUI.GridLayout.RIGHT
				});
			}

			container.add(bindsection);
		}

		local bindtypec = GUI.Container(GUI.GridLayout(1, 2));
		local bindloc = GUI.Label();

		switch(mEquipType)
		{
		case ItemEquipType.ARMOR_SHIELD:
			bindloc.setText(TXT("Shield"));
			break;

		case ItemEquipType.ARMOR_HEAD:
			bindloc.setText(TXT("Head"));
			break;

		case ItemEquipType.ARMOR_NECK:
			bindloc.setText(TXT("Neck"));
			break;

		case ItemEquipType.ARMOR_SHOULDER:
			bindloc.setText(TXT("Shoulder"));
			break;

		case ItemEquipType.ARMOR_CHEST:
			bindloc.setText(TXT("Chest"));
			break;

		case ItemEquipType.ARMOR_ARMS:
			bindloc.setText(TXT("Arms"));
			break;

		case ItemEquipType.ARMOR_HANDS:
			bindloc.setText(TXT("Hands"));
			break;

		case ItemEquipType.ARMOR_WAIST:
			bindloc.setText(TXT("Waist"));
			break;

		case ItemEquipType.ARMOR_LEGS:
			bindloc.setText(TXT("Legs"));
			break;

		case ItemEquipType.ARMOR_FEET:
			bindloc.setText(TXT("Feet"));
			break;

		case ItemEquipType.ARMOR_RING:
		case ItemEquipType.ARMOR_RING_UNIQUE:
			bindloc.setText(TXT("Ring"));
			break;

		case ItemEquipType.ARMOR_AMULET:
			bindloc.setText(TXT("Amulet"));
			break;
		}

		bindtypec.add(bindloc);
		container.add(bindtypec);
		local ratinglabel = GUI.HTML("<b>" + TXT("Armor") + ":</b> " + mArmorResistMelee);
		container.add(ratinglabel);
	}
	
	/**
		Creates the weapon section of the tooltip, including weapon and armor
		bind details and strength.
	*/
	function _addWeaponSection( container, item ) {
		if (mBindingType != ItemBindingType.BIND_NEVER || mEquipType == ItemEquipType.WEAPON_1H_UNIQUE)	{
			local bindsection = GUI.Container(GUI.GridLayout(1, 2));
			local binding = _buildBinding(item);

			if (binding == null)
				binding = GUI.Container();

			bindsection.add(binding);
			local equipunique = GUI.HTML();

			if (mEquipType == ItemEquipType.WEAPON_1H_UNIQUE) {
				equipunique.setText("<font color=\"FFFF00\"><b>" + TXT("Equip Unique") + "</font>");
			}

			bindsection.add(equipunique, {
				anchor = GUI.GridLayout.RIGHT
			});
			container.add(bindsection);
		}

		if (mWeaponType != WeaponType.ARCANE_TOTEM) {
			local typesection = GUI.Container(GUI.GridLayout(1, 2));
			local weapontypelabel = GUI.HTML("<b>" + WeaponTypeNames[mWeaponType] + "</b>");
			typesection.add(weapontypelabel);
			local handlabel = GUI.HTML();

			switch(mEquipType)
			{
			case ItemEquipType.WEAPON_1H_MAIN:
				handlabel.setText("<font color=\"FFFF00\"><b>" + TXT("Main Hand Only") + "</font>");
				break;

			case ItemEquipType.WEAPON_1H_OFF:
				handlabel.setText("<font color=\"FFFF00\"><b>" + TXT("Off Hand Only") + "</font>");
				break;
			}

			typesection.add(handlabel, {
				anchor = GUI.GridLayout.RIGHT
			});
			container.add(typesection);
			local statssection = GUI.Container(GUI.GridLayout(1, 1));
			local damagelabel = GUI.HTML("<b>" + TXT("Damage") + ":</b> " + mWeaponDamageMin + " - " + mWeaponDamageMax);
			statssection.add(damagelabel);
			container.add(statssection);

			if (mWeaponExtraDamangeRating > 0) {
				local additionaldamagelabel = GUI.HTML();

				switch(mWeaponExtraDamageType)
				{
				case DamageType.MELEE:
					additionaldamagelabel.setText("<b>" + TXT("Additional Damage") + ":</b> +" + mWeaponExtraDamangeRating);
					break;

				case DamageType.FIRE:
					additionaldamagelabel.setText("<b>" + TXT("Additional Damage") + ":</b> +" + mWeaponExtraDamangeRating + " <font color=\"FF0000\">" + TXT("Fire") + "</font>");
					break;

				case DamageType.FROST:
					additionaldamagelabel.setText("<b>" + TXT("Additional Damage") + ":</b> +" + mWeaponExtraDamangeRating + " <font color=\"FFFFFF\">" + TXT("Frost") + "</font>");
					break;

				case DamageType.MYSTIC:
					additionaldamagelabel.setText("<b>" + TXT("Additional Damage") + ":</b> +" + mWeaponExtraDamangeRating + " <font color=\"55DDFF\">" + TXT("Mystic") + "</font>");
					break;

				case DamageType.DEATH:
					additionaldamagelabel.setText("<b>" + TXT("Additional Damage") + ":</b> +" + mWeaponExtraDamangeRating + " <font color=\"A0A0A0\">" + TXT("Death") + "</font>");
					break;
				}

				container.add(additionaldamagelabel);
			}
		}
	}
	
	/**
		Adds a list of bonus modifiers to the container
	*/
	function _addBonusSection( container ) {
		local found = false;
		local bonuses = {
			Strength = mBonusStrength,
			Dexterity = mBonusDexterity,
			Constitution = mBonusConstitution,
			Psyche = mBonusPsyche,
			Spirit = mBonusSpirit
		};

		if (mType == ItemType.CHARM) {
			local foundBonus = false;

			foreach( name, amount in bonuses ) {
				if (amount > 0)	{
					local effect = "";

					if (foundBonus == false) {
						foundBonus = true;
						effect = GUI.HTML("<b>First charm : +15%<br/>Second charm +10%<br/>Third charm +5%</b>");
						container.add(effect);
					}

					effect = GUI.HTML("<font color=\"55DDFF\"><b>+" + TXT(name) + "</b></font>");
					container.add(effect);
					found = true;
				}
			}
		}
		else {
			foreach( name, amount in bonuses ) {
				if (amount > 0)	{
					local effect = GUI.HTML("<font color=\"55DDFF\"><b>+" + amount + " " + TXT(name) + "</b></font>");
					container.add(effect);
					found = true;
				}
			}
		}

		return found;
	}
	
	/**
		Adds a list of charm bonus modifiers to the container
	*/
	function _addCharmBonusSection( container ) {
		local found = false;
		local bonuses = {
			["To Hit Melee"] = mMeleeHitMod,
			["To Crit Melee"] = mMeleeCritMod,
			["To Hit Magic"] = mMagicHitMod,
			["To Crit Magic"] = mMagicCritMod,
			["To Parry"] = mParryMod,
			["To Block"] = mBlockMod,
			["Run Speed"] = mRunSpeedMod,
			["Regen Health"] = mRegenHealthMod,
			["Attack Speed Melee"] = mAttackSpeedMod,
			["Casting Speed"] = mCastSpeedMod,
			Healing = mHealingMod
		};

		foreach( name, amount in bonuses ) {
			if (amount > 0) {
				local effect = GUI.HTML("<font color=\"55DDFF\"><b>+" + TXT(name) + "</b></font>");
				container.add(effect);
				found = true;
			}
		}

		return found;
	}
	
	/**
		Add some fun text for the item
	*/
	function _addFlavorText( container ) {
		local flavorText = GUI.HTML("<font color=\"FFFFFF\"><i><b>" + mFlavorText + "</b></i></font>");
		flavorText.setMaximumSize(200, 300);
		flavorText.setResize(true);
		container.add(flavorText);
	}
	
	/**
	* Adds a label that tells players how long this item will last when used
	* @param - container - component you want to attach the label to
	*/
	function _addLifetimeValue( container ) {
		local hourTime = 0;

		//If ivType1 or ivType2 is a Lifetime type, show the time it will last
		if (mIvType1 == ItemIntegerType.LIFETIME)
			hourTime = mIvMax1;

		//If ivType1 or ivType2 is a Lifetime type, show the time it will last
		if (mIvType2 == ItemIntegerType.LIFETIME)
			hourTime = mIvMax2;

		if (hourTime != 0) {
			local timeText = "Item Lifetime: ";
			
			//If hourTime is -1 it means it last forever
			if (hourTime == -1)
				timeText = "Forever";
			else
				timeText = ::Util.parseHourToTimeStr(hourTime);

			local timeLabel = GUI.HTML("<font color=\"" + Colors.lavender + "\"><b>Item Lifetime: </b></font><i>" + timeText + "</i>");
			timeLabel.setData("LIFETIME_LABEL");
			container.add(timeLabel);
		}
	}

	function _addSpecialItemTypeSection( container ) {
		//If item is not a special item type, return
		if (mSpecialItemType == SpecialItemType.NONE)
			return;

		local bonusValue = 0;
		
		//If ivType1 or ivType2 is a a special item bonus value
		if (mIvType1 == ItemIntegerType.BONUS_VALUE)
			bonusValue = mIvMax1;
		
		//If ivType1 or ivType2 is a a special item bonus value
		if (mIvType2 == ItemIntegerType.BONUS_VALUE)
			bonusValue = mIvMax2;

		local specialText = {
			[SpecialItemType.REAGENT_GENERATOR] = {
				text = "Replaces the need to carry reagents that are required to use certain abilities. " + "This device takes the cost of the reagents directly from your available coin. "
			},
			[SpecialItemType.ITEM_GRINDER] = {
				text = "When destroying items you recieve " + bonusValue + "% of the store selling value in coin in return."
			},
			[SpecialItemType.XP_BOOST] = {
				// TODO Em - which stat
				text = "Grants a " + bonusValue + "% boost to experienced gained."
			},
			[SpecialItemType.PORTABLE_REFASHIONER] = {
				text = "Allows items to be refashioned everywhere."
			}
		};

		if (mSpecialItemType in specialText) {
			_addDivider(container);
			local specialItemHTML = GUI.HTML("<font color=\"FFFFFF\"><b>" + specialText[mSpecialItemType].text + "</b></font>");
			specialItemHTML.setMaximumSize(200, 400);
			specialItemHTML.setMinimumSize(200, 100);
			specialItemHTML.setResize(true);
			container.add(specialItemHTML);
		}
	}
	
	/**
		Adds the effects of this item to the container.
	*/
	function _addEffectSection( container ) {
		local found = false;

		if (mEquipEffectId != 0 && mEquipEffectId != "") {
			local EquipEffect = GUI.HTML("<font color=\"FFD080\"><b>" + TXT("Equip") + ":</b> " + mEquipEffectId + "</font>");
			EquipEffect.setMaximumSize(200, 200);
			EquipEffect.setResize(true);
			container.add(EquipEffect);
			found = true;
		}

		if (mUseAbilityId != 0 && mUseAbilityId != "") {
			local UseEffect = GUI.HTML("<font color=\"FFD080\"><b>" + TXT("Use") + ":</b> " + mUseAbilityId + "</font>");
			UseEffect.setMaximumSize(200, 200);
			UseEffect.setResize(true);
			container.add(UseEffect);
			found = true;
		}

		if (mActionAbilityId != 0 && mActionAbilityId != "") {
			local ActionEffect = GUI.HTML("<font color=\"FFD080\"><b>" + TXT("Action") + ":</b> " + mActionAbilityId + "</font>");
			ActionEffect.setMaximumSize(200, 200);
			ActionEffect.setResize(true);
			ActionEffect.validate();
			container.add(ActionEffect);
			found = true;
		}

		return found;
	}
	
	/**
		Creates the charm section of the tooltip, displaying special charm bonuses.
	*/
	function _addValueRow( container, showbuyvalue ) {
		if (mValueType == CurrencyCategory.COPPER) {
			local value = GUI.Currency(showbuyvalue ? getBuyValue() : getSellValue());
			value.setFont(GUI.Font("Maiandra", 16));
			value.setAlignment(1);
			container.add(value);
		}
	}
	
	/**
		Adds a row indicating the durability.
	*/
	function _addDurabilityRow( container ) {
		local durability = getDynamicMax(ItemIntegerType.DURABILITY);
		local durabilitylabel = GUI.HTML("<b>Durability:</b> " + durability + "/" + durability);
		container.add(durabilitylabel);
	}
	
	/**
		Adds a section for 'Focus' item types.
	*/
	function _addCharmSection( container, item ) {
		local binding = _buildBinding(item);

		if (binding)
			container.add(binding);

		local charmType = GUI.HTML();

		switch(mEquipType)
		{
		case ItemEquipType.RED_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.red + "\">Red</font>");
			break;

		case ItemEquipType.BLUE_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.blue + "\">Blue</font>");
			break;

		case ItemEquipType.YELLOW_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.yellow + "\">Yellow</font>");
			break;

		case ItemEquipType.ORANGE_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.orange + "\">Orange</font>");
			break;

		case ItemEquipType.PURPLE_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.purple + "\">Purple</font>");
			break;

		case ItemEquipType.GREEN_CHARM:
			charmType.setText("<b>" + TXT("Charm") + ":</b> <font color=\"" + Colors.green + "\">Green</font>");
			break;
		}

		container.add(charmType);
	}

	function _addContainerSection( container ) {
		local containerlabel = GUI.HTML("<b>" + TXT("Container") + ":</b> " + mContainerSlots + " " + TXT("Slots"));
		container.add(containerlabel);
	}

	function _getPrice() {
		local price;
		price = mValue;
		return price;
	}

	function getContainerSlots() {
		return mContainerSlots;
	}

	function getBuyValue() {
		if (mValueType == CurrencyCategory.COPPER)
			return _getPrice() * gVendorMarkup;
		else if (mValueType == CurrencyCategory.CREDITS)
			return mValue;
	}

	function getSellValue()	{
		if (mValueType == CurrencyCategory.COPPER)
			return _getPrice();
		else if (mValueType == CurrencyCategory.CREDITS)
			return 0;
	}

	function getWeaponPower()
	{
		local power = 0;
		power += mWeaponDamageMin * 0.1;
		power += mWeaponDamageMax * 0.1;
		power += mWeaponSpeed * 0.050000001;
		power += mWeaponExtraDamangeRating * 0.050000001;
		return power;
	}

	function getArmorPower() {
		local power = 0;
		power += mArmorResistMelee * 0.1;
		power += mArmorResistFire * 0.050000001;
		power += mArmorResistFrost * 0.050000001;
		power += mArmorResistMystic * 0.050000001;
		power += mArmorResistDeath * 0.050000001;
		return power;
	}

	function getFocusPower() {
		local power = 0;
		power += getDynamicMax(0) * 0.30000001;
		return power;
	}

	function getDynamicMax( type ) {
		if (!mValid)
			return null;

		if (mIvType1 == type)
			return mIvMax1;

		if (mIvType2 == type)
			return mIvMax2;

		return null;
	}

	function getDynamicValue( type, itemData ) {
		if (!mValid) 
			return null;

		if (mIvType1 == type)
			return itemData.mIv1;

		if (mIvType2 == type)
			return itemData.mIv2;

		return null;
	}

	function getDamagePerSecond() {
		local avgdamage = (mWeaponDamageMax - mWeaponDamageMin) * 0.5 + mWeaponDamageMin;
		return avgdamage / mWeaponSpeed + mWeaponExtraDamangeRating;
	}

	function getClassRestrictions() {
		switch(mType)
		{
		case ItemType.WEAPON:
			return WeaponTypeClassRestrictions[mWeaponType];

		case ItemType.ARMOR:
			return ArmorTypeClassRestrictions[mArmorType];
		}

		return {
			none = false,
			knight = true,
			mage = true,
			rogue = true,
			druid = true
		};
	}

	function getResultItem() {
		return mResultItem;
	}

	function getKeyComponent() {
		return mKeyComponent;
	}

	function getCraftComponent() {
		return mCraftComponents;
	}

	function getSpecialItemType() {
		return mSpecialItemType;
	}

	function getLevel() {
		return mLevel;
	}

	function getMinUseLevel() {
		return mMinUseLevel;
	}

	function getQualityLevel() {
		return mQualityLevel;
	}

	function getBindingType() {
		return mBindingType;
	}

}

