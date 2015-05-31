this.require("Constants");
this.require("ActionManager");
this.require("Connection");
this.require("Combat/CombatEquations");
this.AbilityOwnageType <- {
	ABILITY_NOT_OWNED = 0,
	ABILITY_OWNED_NOT_CURRENT = 1,
	ABILITY_OWNED_AND_CURRENT = 2
};
class this.AbilityHelper 
{
	mMaxListCount = 20;
	mCurrentListOffset = null;
	mQueryResults = null;
	mMessageBroadcaster = this.MessageBroadcaster();
	constructor()
	{
		this.mQueryResults = [];
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "ab.list")
		{
			this.mQueryResults.extend(results);

			if (this.mCurrentListOffset == null)
			{
				::AbilityUtil.saveAbilityData(this.mQueryResults);
				this.mQueryResults.clear();
				this.mQueryResults = [];
				::_AbilityManager.handleUpdatingAbilities(this.mQueryResults, false);
				this.mMessageBroadcaster.broadcastMessage("onAbilitiesReceived", ::_AbilityManager.getAbilities());
			}

			if (qa.args.len() > 0 && typeof qa.args[0] == "string" && qa.args[0] == "RANGE")
			{
				this._getNextAbilities(results.len());
			}
		}
	}

	function _getNextAbilities( count )
	{
		if (this.mCurrentListOffset != null)
		{
			if (count != null && count < this.mMaxListCount)
			{
				this.AbilityUtil.saveAbilityData(this.mQueryResults);
				this.mQueryResults.clear();
				this.mQueryResults = [];
				::_AbilityManager.handleUpdatingAbilities(this.mQueryResults, false);
				this.mMessageBroadcaster.broadcastMessage("onAbilitiesReceived", ::_AbilityManager.getAbilities());
				this.mCurrentListOffset = null;
			}
			else
			{
				if (count != null)
				{
					this.mCurrentListOffset += count;
				}

				this._Connection.sendQuery("ab.list", this, [
					"RANGE",
					this.mCurrentListOffset,
					this.mMaxListCount
				]);
			}
		}
	}

	function updateClientAbilities()
	{
		if (this.mCurrentListOffset == null)
		{
			this.mCurrentListOffset = 0;
			this._getNextAbilities(null);
		}
	}

}

class this.AbilityManager extends this.DefaultQueryHandler
{
	mAbilities = {};
	mMessageBroadcaster = this.MessageBroadcaster();
	mMaxListCount = 20;
	mCurrentListOffset = null;
	mCooldowns = null;
	mGotCooldownInfo = false;
	mPartyCasting = false;
	mClearQuickbars = false;
	mTimer = null;
	mOwnageRetrieved = false;
	constructor()
	{
		this.reset();
	}

	function addAbilityListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function abBuy( args )
	{
		this._Connection.sendQuery("ab.buy", this, args);
	}

	function abRespec()
	{
		this._Connection.sendQuery("ab.respec", this);
	}

	function getAbilityByName( name )
	{
		foreach( id, ab in this.mAbilities )
		{
			if (ab.getName() == name)
			{
				return ab;
			}
		}

		return null;
	}

	function getAbilityById( id )
	{
		if (!(id in this.mAbilities))
		{
			local ab = this.Ability(id, "", "");
			this.mAbilities[id] <- ab;
		}

		return this.mAbilities[id];
	}

	function getAbilities()
	{
		return this.mAbilities;
	}

	function getCategoryUseTime( mCooldownCategory )
	{
		if (mCooldownCategory in this.mCooldowns)
		{
			return this.mCooldowns[mCooldownCategory].usedTime;
		}
		else
		{
			return 0;
		}
	}

	function getTimerMilliseconds()
	{
		return this.mTimer.getMilliseconds();
	}

	function getTimeUntilCategoryUseable( category )
	{
		if (category in this.mCooldowns)
		{
			return this.mCooldowns[category].timeRemaining - this.mTimer.getMilliseconds();
		}
		else
		{
			return 0;
		}
	}

	function getRemainingCoolDownTime( ability )
	{
		return this.getTimeUntilCategoryUseable(ability.getCooldownCategory());
	}

	function isOwnageRetrieved()
	{
		return this.mOwnageRetrieved;
	}

	function isPartyCasting()
	{
		return this.mPartyCasting;
	}

	function onTargetObjectChanged( creature, target )
	{
		this.setPartyCasting(false);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "ab.list")
		{
			this.getAbilityCooldowns();
			this._handleAbList(qa, results);
		}
		else if (qa.query == "ab.buy")
		{
			this._handleAbBuy(qa, results);
		}
		else if (qa.query == "ab.respec")
		{
			if (results[0][0] == "true")
			{
				this.log.debug("handleAbRespec()");
				this.handleAbRespec();
			}
		}
		else if (qa.query == "ab.remainingcooldowns")
		{
			for( local i = 0; i < results.len(); i++ )
			{
				local category = results[i][0];
				local remainingTime = results[i][1].tointeger();

				if (remainingTime > 0)
				{
					local timeElapsed = results[i][2].tointeger();
					this.setCategoryCooldownTime(category, remainingTime);
					this.setCategoryUsedtime(category, this.mTimer.getMilliseconds() - timeElapsed);
					::_quickBarManager.setCategoryUsable(category, false);
				}
			}
		}
		else if (qa.query == "ab.ownage.list")
		{
			for( local i = 0; i < results.len(); i++ )
			{
				local abilityId = results[i][0].tointeger();
				local ab = this.getAbilityById(abilityId);
				local ownage = this.AbilityOwnageType.ABILITY_OWNED_AND_CURRENT;
				ab.setOwnage(ownage);
			}

			if (this.mClearQuickbars)
			{
				this.mClearQuickbars = false;
				this.clearQuickbarsOfUnknownAbilities();
			}

			this.mOwnageRetrieved = true;
			this.mMessageBroadcaster.broadcastMessage("onAbilitiesReceived", this.mAbilities);
			this.mMessageBroadcaster.broadcastMessage("onAbilityOwnageUpdate");
		}
		else
		{
			throw this.Exception("Unknown query: " + qa.query);
		}
	}

	function clearQuickbarsOfUnknownAbilities()
	{
		local quickbarsTouched = {};

		for( local i = 0; i < 8; i++ )
		{
			local quickbar = ::_quickBarManager.getQuickBar(i);
			local quickbarAC = quickbar.getActionContainer();
			local actionButtons = quickbarAC.getAllActionButtons(true);

			for( local j = 0; j < actionButtons.len(); j++ )
			{
				local action = actionButtons[j].getAction();

				if (action instanceof this.Ability)
				{
					if (action.getCategory() != "System" && action.getOwnage() == 0)
					{
						quickbarAC.removeAction(action);
						quickbarsTouched[i] <- true;
					}
				}
			}
		}

		foreach( k, v in quickbarsTouched )
		{
			::_quickBarManager.saveQuickbar(k);
		}
	}

	function updateQuickbarsWithMostPowerfulAbilities()
	{
		local quickbarsTouched = {};

		for( local i = 0; i < 8; i++ )
		{
			local quickbar = ::_quickBarManager.getQuickBar(i);
			local quickbarAC = quickbar.getActionContainer();
			local actionButtons = quickbarAC.getAllActionButtons(true);

			for( local j = 0; j < actionButtons.len(); j++ )
			{
				local action = actionButtons[j].getAction();

				if (action instanceof this.Ability)
				{
					if (action.getCategory() != "System")
					{
						local mostPowerfulAbility = this.findMostPowerfulAbilityTier(action);

						if (action != mostPowerfulAbility)
						{
							actionButtons[j].bindActionToButton(mostPowerfulAbility);
							quickbarsTouched[i] <- true;
						}
					}
				}
			}
		}

		foreach( k, v in quickbarsTouched )
		{
			::_quickBarManager.saveQuickbar(k);
		}
	}

	function findMostPowerfulAbilityTier( ability )
	{
		local mostPowerfulAbility = ability;
		local highestTier = ability.getTier();

		foreach( ab in this.mAbilities )
		{
			if (ab.getName() == ability.getName())
			{
				if (ab.getOwnage() > 0 && ab.getTier() > highestTier)
				{
					highestTier = ab.getTier();
					mostPowerfulAbility = ab;
				}
			}
		}

		return mostPowerfulAbility;
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error("" + qa.query + " [" + qa.correlationId + "] failed: " + error);

		if (qa.query == "ab.buy" || qa.query == "ab.respec")
		{
			local notify = this.GUI.ConfirmationWindow();
			notify.setConfirmationType(this.GUI.ConfirmationWindow.OK);
			notify.setText(this.TXT(error));
		}
	}

	function onQueryTimeout( qa )
	{
		this.log.warn("Query " + qa.query + " [" + qa.correlationId + "] timed out");
		this._requestNextAbilities(null);
	}

	function removeAbilityListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function refreshAbs( args )
	{
		if (this.mCurrentListOffset == null)
		{
			if (args != null)
			{
				this._Connection.sendQuery("ab.list", this, args);
			}
			else
			{
				this.mCurrentListOffset = 0;
				this._requestNextAbilities(null);
			}
		}
	}

	function getAbilityCooldowns()
	{
		if (!this.mGotCooldownInfo)
		{
			::_Connection.sendQuery("ab.remainingcooldowns", this);
			this.mGotCooldownInfo = true;
		}
	}

	function getAbilityOwnageList()
	{
		this._Connection.sendQuery("ab.ownage.list", this);
	}

	function resetAllAbilityOwnageToNone()
	{
		foreach( id, ab in this.mAbilities )
		{
			ab.setOwnage(this.AbilityOwnageType.ABILITY_NOT_OWNED);
		}
	}

	function reset()
	{
		this.mCurrentListOffset = null;
		this.mAbilities = {};
		this.mCooldowns = {};
		this.mGotCooldownInfo = false;
		this.mTimer = ::Timer();
		this.resetAllAbilityOwnageToNone();
		this.mOwnageRetrieved = false;
	}

	function setCategoryCooldownTime( cooldownCategory, cooldownTimeRemaining )
	{
		if (cooldownCategory == null || cooldownCategory == "")
		{
			return;
		}

		if (!(cooldownCategory in this.mCooldowns))
		{
			this.mCooldowns[cooldownCategory] <- {
				timeRemaining = cooldownTimeRemaining + this.mTimer.getMilliseconds(),
				usedTime = this.mTimer.getMilliseconds()
			};
		}
		else
		{
			this.mCooldowns[cooldownCategory] = {
				timeRemaining = cooldownTimeRemaining + this.mTimer.getMilliseconds(),
				usedTime = this.mTimer.getMilliseconds()
			};
		}
	}

	function setCategoryUsedtime( cooldownCategory, usedTime )
	{
		if (cooldownCategory == null || cooldownCategory == "")
		{
			return;
		}

		if (!(cooldownCategory in this.mCooldowns))
		{
			return;
		}

		this.mCooldowns[cooldownCategory].usedTime = usedTime;
	}

	function setPartyCasting( val )
	{
		if (val == this.mPartyCasting)
		{
			return;
		}

		this.mPartyCasting = val;

		if (this.mPartyCasting)
		{
			if (this._avatar)
			{
				this._avatar.addListener(this);
			}
		}
		else
		{
			if (this._avatar)
			{
				this._avatar.removeListener(this);
			}

			local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

			if (selfTargetWindow)
			{
				selfTargetWindow.uncheckPartyGem();
			}
		}
	}

	function avatarStatsUpdated()
	{
		this.mMessageBroadcaster.broadcastMessage("onAvatarStatsUpdated");
	}

	function _handleAbList( qa, results, ... )
	{
		this.log.debug("_handleAbList()");
		this.handleUpdatingAbilities(results);

		if (this.mCurrentListOffset == null)
		{
			this.mMessageBroadcaster.broadcastMessage("onAbilitiesReceived", this.mAbilities);
		}

		if (qa.args.len() > 0 && typeof qa.args[0] == "string" && qa.args[0] == "RANGE")
		{
			this._requestNextAbilities(results.len());
		}
	}

	function handleUpdatingAbilities( results, ... )
	{
		local shouldSetOwnage = true;

		if (vargc > 0)
		{
			shouldSetOwnage = vargv[0];
		}

		foreach( row in results )
		{
			local name;
			local id;
			local name;
			local warmupTime;
			local duration;
			local coolDownCategory;
			local coolDownTime;
			local target;
			local activationCriteria;
			local onActivate;
			local onIterate;
			local visualCue;
			local warmupCue = "";
			local range;
			local icon;
			local onRebound;
			local hostility;
			local interval;
			local actions = "";
			local tier;
			local prereq;
			local description;
			local category;
			local coordinateX = 0;
			local coordinateY = 0;
			local groupID = 0;
			local ownage = 0;
			local abilityClass = "";
			local useType = 0;
			local addMeleeCharge = 0;
			local addMagicCharge = 0;
			local goldCost = 0;
			local buffCategory = "";
			local targetCriteria = "";

			if (row.len() == 26)
			{
				id = row[0].tointeger();
				name = row[1];
				hostility = row[2].tointeger();
				warmupTime = row[3].tointeger();
				warmupCue = row[4];
				duration = row[5].tofloat();
				interval = row[6].tofloat();
				coolDownCategory = row[7];
				coolDownTime = row[8].tointeger();
				activationCriteria = row[9];
				actions = row[10];
				visualCue = row[11];
				tier = row[12].tointeger();
				prereq = row[13];
				icon = row[14];
				description = row[15];
				category = row[16];
				coordinateX = row[17].tointeger();
				coordinateY = row[18].tointeger();
				groupID = row[19].tointeger();
				abilityClass = row[20];
				useType = row[21].tointeger();
				addMeleeCharge = row[22].tointeger();
				addMagicCharge = row[23].tointeger();
				ownage = row[24].tointeger();
				goldCost = row[25].tointeger();
			}
			else if (row.len() == 27)
			{
				id = row[0].tointeger();
				name = row[1];
				hostility = row[2].tointeger();
				warmupTime = row[3].tointeger();
				warmupCue = row[4];
				duration = row[5].tofloat();
				interval = row[6].tofloat();
				coolDownCategory = row[7];
				coolDownTime = row[8].tointeger();
				activationCriteria = row[9];
				actions = row[10];
				visualCue = row[11];
				tier = row[12].tointeger();
				prereq = row[13];
				icon = row[14];
				description = row[15];
				category = row[16];
				coordinateX = row[17].tointeger();
				coordinateY = row[18].tointeger();
				groupID = row[19].tointeger();
				abilityClass = row[20];
				useType = row[21].tointeger();
				addMeleeCharge = row[22].tointeger();
				addMagicCharge = row[23].tointeger();
				ownage = row[24].tointeger();
				goldCost = row[25].tointeger();
				buffCategory = row[26];
			}
			else if (row.len() == 28)
			{
				id = row[0].tointeger();
				name = row[1];
				hostility = row[2].tointeger();
				warmupTime = row[3].tointeger();
				warmupCue = row[4];
				duration = row[5].tofloat();
				interval = row[6].tofloat();
				coolDownCategory = row[7];
				coolDownTime = row[8].tointeger();
				activationCriteria = row[9];
				actions = row[10];
				visualCue = row[11];
				tier = row[12].tointeger();
				prereq = row[13];
				icon = row[14];
				description = row[15];
				category = row[16];
				coordinateX = row[17].tointeger();
				coordinateY = row[18].tointeger();
				groupID = row[19].tointeger();
				abilityClass = row[20];
				useType = row[21].tointeger();
				addMeleeCharge = row[22].tointeger();
				addMagicCharge = row[23].tointeger();
				ownage = row[24].tointeger();
				goldCost = row[25].tointeger();
				buffCategory = row[26];
				targetCriteria = row[27];
			}

			range = -1;
			local ir = activationCriteria.find("InRange");

			if (ir != null)
			{
				local start_pos = ir + 6;
				local expect_par = true;
				local skipping = false;
				local reading_value = false;
				local success = false;
				local value_start;
				local value_end;

				for( local i = start_pos; i < activationCriteria.len(); i++ )
				{
					local char = activationCriteria[i];

					if (expect_par)
					{
						if (char == 40)
						{
							expect_par = false;
							skipping = true;
						}
					}
					else if (skipping)
					{
						if (char != 32)
						{
							skipping = false;
							reading_value = true;
							value_start = i;
						}
					}
					else if (reading_value)
					{
						if (char == 32 || char == 41)
						{
							success = true;
							value_end = i;
							break;
						}
						else
						{
						}
					}
					else
					{
						success = false;
						break;
					}
				}

				if (success)
				{
					local valstr = activationCriteria.slice(value_start, value_end);
					local _range = valstr.tofloat();

					if (_range == null)
					{
						this.log.debug("Invalid range in ability!!");
					}
					else
					{
						range = _range;
					}
				}
				else
				{
					this.log.debug("Invalid range in ability!!");
				}
			}

			local purchaseLevelRequired = 0;
			local purchasePointsRequired = 2;
			local purchaseAbilitiesRequired = [];
			local purchaseClassRequired = "";

			if (prereq.len() > 0)
			{
				local prereqs = this.Util.split(prereq, ",");

				if (prereqs[0].len() > 0)
				{
					purchaseLevelRequired = prereqs[0].tointeger();
				}

				if (prereqs[1].len() > 0)
				{
					purchasePointsRequired = prereqs[1].tointeger();
				}

				if (prereqs[3].len() > 0)
				{
					try
					{
						local rawAbReqs = this.Util.replace(prereqs[3], "(", "");
						rawAbReqs = this.Util.replace(rawAbReqs, ")", "");
						rawAbReqs = this.Util.replace(rawAbReqs, " ", "");
						local parsedAbReqs = this.Util.split(rawAbReqs, "|");

						foreach( abReq in parsedAbReqs )
						{
							if (abReq != "")
							{
								purchaseAbilitiesRequired.append(abReq.tointeger());
							}
						}
					}
					catch( err )
					{
						purchaseAbilitiesRequired.append(0);
					}
				}

				if (prereqs[4].len() > 0)
				{
					purchaseClassRequired = prereqs[4];
				}
			}

			local ab;

			if (!(id in this.mAbilities))
			{
				ab = this.Ability(id, name, icon);
				this.mAbilities[id] <- ab;
			}
			else
			{
				ab = this.mAbilities[id];
				ab.setName(name);
				ab.setImage(icon);
			}

			ab.setVisualCue(visualCue);
			ab.setWarmupCue(warmupCue);
			ab.setWarmupDuration(warmupTime);
			ab.setCooldownDuration(coolDownTime);
			ab.setRange(range);
			ab.setCategory(category);
			ab.setCooldownCategory(coolDownCategory);
			ab.setSlotCoordinates([
				coordinateX,
				coordinateY
			]);
			ab.setGroupId(groupID);
			ab.setDescription(description);
			ab.setActions(actions);
			ab.setActivationCriteria(activationCriteria);
			ab.setHostility(hostility);
			ab.setTargetCriteria(targetCriteria);

			if (shouldSetOwnage)
			{
				ab.setOwnage(ownage);
			}

			ab.setPurchaseLevelRequired(purchaseLevelRequired);
			ab.setPurchasePointsRequired(purchasePointsRequired);
			ab.setPurchaseAbilityRequired(purchaseAbilitiesRequired);
			ab.setPurchaseClassRequired(purchaseClassRequired);
			ab.setTier(tier);
			ab.setDuration(duration);
			ab.setAbilityClass(abilityClass);
			ab.setUseType(useType);
			ab.setAddMeleeCharge(addMeleeCharge);
			ab.setAddMagicCharge(addMagicCharge);
			ab.setGoldCost(goldCost);
			ab.setBuffCategory(buffCategory);
			ab.setValid(true);
			
			this.mMessageBroadcaster.broadcastMessage("onAbilityUpdate", this.mAbilities);
		}

		foreach( category, value in this.mCooldowns )
		{
			if (this.getTimeUntilCategoryUseable(category) >= 0)
			{
				::_quickBarManager.setCategoryUsable(category, false);
			}
		}
	}

	function _requestNextAbilities( count )
	{
		if (this.mCurrentListOffset != null)
		{
			if (count != null && count < this.mMaxListCount)
			{
				this.mMessageBroadcaster.broadcastMessage("onAbilitiesReceived", this.mAbilities);
				this.mCurrentListOffset = null;
			}
			else
			{
				if (count != null)
				{
					this.mCurrentListOffset += count;
				}

				this._Connection.sendQuery("ab.list", this, [
					"RANGE",
					this.mCurrentListOffset,
					this.mMaxListCount
				]);
			}
		}
	}

	function _handleAbBuy( qa, results )
	{
		this.log.debug("_handleAbBuy()");

		if (results.len() != 1)
		{
			return;
		}

		local abilityId = results[0][0].tointeger();
		local ab = this.getAbilityById(abilityId);

		if (ab != null)
		{
			ab.setOwnage(this.AbilityOwnageType.ABILITY_OWNED_AND_CURRENT);
			this.mMessageBroadcaster.broadcastMessage("onUpdateBuyAbility", ab);
		}

		this.updateQuickbarsWithMostPowerfulAbilities();
	}

	function handleAbRespec()
	{
		this.resetAllAbilityOwnageToNone();
		::_AbilityManager.getAbilityOwnageList();
		this.mMessageBroadcaster.broadcastMessage("onRespecializeAbility");
		this.mClearQuickbars = true;
	}

}

this.RequirementTable <- {
	hasMainHandWeapon = "a main weapon.",
	hasBow = "a bow.",
	hasRangedWeapon = "a ranged weapon.",
	hasWand = "a wand.",
	has2HWeapon = "a two handed weapon.",
	hasPoleWeapon = "a pole weapon.",
	Behind = "you to be behind your enemy.",
	hasShield = "a shield.",
	has2HorPoleWeapon = "a two handed or pole weapon.",
	NearbySanctuary = "you to be nearby a sanctuary.",
	function MightCharge( minCharges, maxCharges )
	{
		if (minCharges == maxCharges)
		{
			if (minCharges == 1)
			{
				return minCharges + " Physical Charge";
			}

			return minCharges + " Physical Charges";
		}
		else
		{
			return minCharges + " - " + maxCharges + " Physical Charges";
		}
	}

	function WillCharge( minCharges, maxCharges )
	{
		if (minCharges == maxCharges)
		{
			if (minCharges == 1)
			{
				return minCharges + " Magic Charge";
			}

			return minCharges + " Magic Charges";
		}
		else
		{
			return minCharges + " - " + maxCharges + " Magic Charges";
		}
	}

	function NotTransformed()
	{
		return "the player not to be transformed";
	}

	function NotStatus( status )
	{
		return "the player not to be " + status;
	}

	function HasStatus( status )
	{
		return "the player to be " + status;
	}

	function PercentMaxHealth( percentile )
	{
		percentile = percentile.tofloat() * 100.0;
		return "the player to be at least " + percentile.tostring() + "% healthy.";
	}

	IN_COMBAT = "in combat.",
	INVINCIBLE = "invincible.",
	INVISIBLE = "invisible.",
	DEAD = "dead.",
	WALK_IN_SHADOWS = "in Walk in Shadows."
};
this.EquipmentRequirementsTable <- {
	hasMainHandWeapon = {
		equipType = "ItemEquipSlot",
		slot = this.ItemEquipSlot.WEAPON_MAIN_HAND
	},
	has2HWeapon = {
		equipType = "ItemEquipType",
		slot = this.ItemEquipType.WEAPON_2H
	},
	hasShield = {
		equipType = "ItemEquipType",
		slot = this.ItemEquipType.ARMOR_SHIELD
	},
	hasBow = {
		equipType = "WeaponType",
		slot = this.WeaponType.BOW,
		itemEquipSlot = this.ItemEquipSlot.WEAPON_RANGED
	},
	hasRangedWeapon = {
		equipType = "ItemEquipSlot",
		slot = this.ItemEquipSlot.WEAPON_RANGED
	},
	hasWand = {
		equipType = "WeaponType",
		slot = this.WeaponType.WAND,
		itemEquipSlot = this.ItemEquipSlot.WEAPON_RANGED
	},
	hasPoleWeapon = {
		equipType = "WeaponType",
		slot = this.WeaponType.POLE,
		itemEquipSlot = this.ItemEquipSlot.WEAPON_MAIN_HAND
	},
	has2HorPoleWeapon = {
		equipType = "EitherWeapon",
		types = [
			"has2HWeapon",
			"hasPoleWeapon"
		]
	}
};
this.StatusRequirementsTable <- {
	Behind = "testing",
	IN_COMBAT = this.StatusEffects.IN_COMBAT,
	INVINCIBLE = this.StatusEffects.INVINCIBLE,
	INVISIBLE = this.StatusEffects.INVISIBLE,
	WALK_IN_SHADOWS = this.StatusEffects.WALK_IN_SHADOWS,
	DEAD = this.StatusEffects.DEAD,
	CheckBuffLimits = "",
	avatar = ::_avatar,
	combatEquations = ::_combatEquations,
	function NotStatus( status, avatar )
	{
		if (!avatar)
		{
			return false;
		}

		return !avatar.hasStatusEffect(status);
	}

	function HasStatus( status, avatar )
	{
		if (!avatar)
		{
			return false;
		}

		return avatar.hasStatusEffect(status);
	}

	PercentMaxHealth = "Use predefined function for this value"
};
class this.Ability extends this.Action
{
	mValid = false;
	mId = 0;
	mVisualCue = "";
	mTooltipComponent = null;
	mWarmupBar = null;
	mWarmupCue = "";
	mWarmingUp = false;
	mWarmupDuration = 0.0;
	mWarmupEndTime = 0.0;
	mWarmupStartTime = 0.0;
	mWarmupSetbackTime = 0;
	mChannelStartTime = 0;
	mChannelEndTime = 0;
	mChannelBar = null;
	mCooldownTime = 0.0;
	mUseType = 0;
	mRange = -1.0;
	mTimer = null;
	mCategory = "";
	mCooldownCategory = "";
	mCoordinates = [];
	mGroupId = 0;
	mOwnage = 0;
	mDescription = "";
	mActions = "";
	mBuffCategory = "";
	mActivationCriteria = "";
	mTargetCriteria = "";
	mPurchaseLevelRequired = 0;
	mPurchasePointsRequired = 0;
	mPurchaseAbilitiesRequired = null;
	mPurchaseClassRequired = "";
	mTier = 0;
	mDuration = 0;
	mHostility = 0;
	mWill = 0;
	mMight = 0;
	mWillMinCharge = 0;
	mWillMaxCharge = 0;
	mMightMinCharge = 0;
	mMightMaxCharge = 0;
	mSpecialRequirements = null;
	mEquipRequirements = null;
	mStatusRequirements = null;
	mAddMeleeCharge = 0;
	mAddMagicCharge = 0;
	mAbilityClass = "";
	mGoldCost = 0;
	mRequestedUse = false;
	mReagents = null;
	mNeedReagents = false;
	mInfoFrame = null;
	AbilityRequirementTags = {
		WILL = "Will",
		MIGHT = "Might",
		WILL_CHARGE = "WillCharge",
		MIGHT_CHARGE = "MightCharge",
		IN_RANGE = "InRange",
		NOT_SILENCED = "NotSilenced",
		REAGENT = "Reagent"
	};
	constructor( id, name, image )
	{
		this.Action.constructor(name, image);
		this.mValid = false;
		this.mId = id;
		this.mTimer = ::Timer();
		this.mPurchaseAbilitiesRequired = [];
		this.mSpecialRequirements = [];
		this.mEquipRequirements = [];
		this.mStatusRequirements = [];
		this.mReagents = {};
		this.mActionSound = this.CoolDownAbilityActionSound(this);
	}

	function setName( name )
	{
		this.mName = name;
	}

	function getName()
	{
		return this.mName;
	}

	function setUseType( type )
	{
		this.mUseType = type;
	}

	function getUseType()
	{
		return this.mUseType;
	}

	function setAddMagicCharge( addMagicCharge )
	{
		this.mAddMagicCharge = addMagicCharge;
	}

	function getAddMagicCharge()
	{
		return this.mAddMagicCharge;
	}

	function setAddMeleeCharge( addMeleeCharge )
	{
		this.mAddMeleeCharge = addMeleeCharge;
	}

	function getAddMeleeCharge()
	{
		return this.mAddMeleeCharge;
	}

	function setAbilityClass( name )
	{
		this.mAbilityClass = name;
	}

	function getAbilityClass()
	{
		return this.mAbilityClass;
	}

	function setPurchaseLevelRequired( value )
	{
		this.mPurchaseLevelRequired = value;
	}

	function setGoldCost( value )
	{
		this.mGoldCost = value;
	}

	function setBuffCategory( value )
	{
		this.mBuffCategory = value;
	}

	function getBuffCategory()
	{
		return this.mBuffCategory;
	}

	function getGoldCost()
	{
		return this.mGoldCost;
	}

	function setActivationCriteria( value )
	{
		this.mActivationCriteria = value;
		this._splitSpecialRequirementsData(this.mActivationCriteria);
	}

	function setHostility( value )
	{
		this.mHostility = value;
	}

	function setTargetCriteria( criteria )
	{
		this.mTargetCriteria = criteria;
	}

	function getTargetCriteria()
	{
		return this.mTargetCriteria;
	}

	function getHostility()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mHostility;
	}

	function _setWill( value )
	{
		this.mWill = value;
	}

	function _setMight( value )
	{
		this.mMight = value;
	}

	function getWill()
	{
		return this.mWill;
	}

	function getMight()
	{
		return this.mMight;
	}

	function _setWillMinCharge( value )
	{
		this.mWillMinCharge = value;
	}

	function _setWillMaxCharge( value )
	{
		this.mWillMaxCharge = value;
	}

	function _setMightMinCharge( value )
	{
		this.mMightMinCharge = value;
	}

	function _setMightMaxCharge( value )
	{
		this.mMightMaxCharge = value;
	}

	function getWillMinCharge()
	{
		return this.mWillMinCharge;
	}

	function getWillMaxCharge()
	{
		return this.mWillMaxCharge;
	}

	function getMightMinCharge()
	{
		return this.mMightMinCharge;
	}

	function getMightMaxCharge()
	{
		return this.mMightMaxCharge;
	}

	function _splitSpecialRequirementsData( activationCriteria )
	{
		if (activationCriteria != "" && activationCriteria.find(",") != null)
		{
			if (activationCriteria.find("INVISIBLE"))
			{
				this.print("Invisible string");
			}

			local results = ::Util.split(activationCriteria, "),");
			local specialRequirements = [];

			foreach( rule in results )
			{
				local found = false;

				if (rule != "")
				{
					foreach( tag in this.AbilityRequirementTags )
					{
						local pos = rule.find(tag);

						if (pos != null)
						{
							local data = ::Util.replace(rule, tag + "(", "");
							data = ::Util.replace(data, ")", "");
							local pos1 = data.find(tag);

							if (pos1 != null)
							{
								continue;
							}

							switch(tag)
							{
							case this.AbilityRequirementTags.WILL:
								this._setWill(data.tointeger());
								found = true;
								break;

							case this.AbilityRequirementTags.MIGHT:
								this._setMight(data.tointeger());
								found = true;
								break;

							case this.AbilityRequirementTags.WILL_CHARGE:
								local chargeResults = [];
								chargeResults = ::Util.split(data, ",");

								if (chargeResults.len() > 1)
								{
									this._setWillMinCharge(chargeResults[0].tointeger());
									this._setWillMaxCharge(chargeResults[1].tointeger());
									found = false;
								}

								break;

							case this.AbilityRequirementTags.MIGHT_CHARGE:
								local chargeResults = [];
								chargeResults = ::Util.split(data, ",");

								if (chargeResults.len() > 1)
								{
									this._setMightMinCharge(chargeResults[0].tointeger());
									this._setMightMaxCharge(chargeResults[1].tointeger());
									found = false;
								}

								break;

							case this.AbilityRequirementTags.REAGENT:
								local reagents = [];
								reagents = ::Util.split(data, ",");

								if (reagents.len() > 1)
								{
									try
									{
										local reagentName = ::_ItemDataManager.getItemDef(reagents[0].tointeger());
										this._addReagents(reagents[0].tointeger(), reagents[1].tointeger());
										found = true;
									}
									catch( error )
									{
										this.log.error(error);
									}
								}

								break;

							case this.AbilityRequirementTags.IN_RANGE:
								found = true;
								break;
							}

							break;
						}
					}

					if (!found)
					{
						local needToAddRule = true;

						if (rule.find("(") && rule.find(")") == null)
						{
							rule = rule + ")";
						}
						else if (rule.find("(") && rule.find(")"))
						{
						}
						else if (rule.find(","))
						{
							local rules = ::Util.split(rule, ",");

							foreach( myRule in rules )
							{
								specialRequirements.append(myRule);
							}

							needToAddRule = false;
						}

						if (needToAddRule)
						{
							specialRequirements.append(rule);
						}
					}
				}
			}

			this._setSpecialRequirement(specialRequirements);
		}
	}

	function _parseRequirement( req )
	{
		local bpos = req.find("(");
		local epos = req.find(")");

		if (bpos && epos && bpos < epos)
		{
			local fn_name = req.slice(0, bpos);
			fn_name = ::Util.trim(fn_name);

			if (fn_name in this.RequirementTable)
			{
				try
				{
					//If function in the status requirements table add it to it
					if(fn_name in StatusRequirementsTable)
					{
						mStatusRequirements.append(req);
					}
					
					local res = eval("return " + req + ";", RequirementTable);
					return res;
				}
				catch(err)
				{
						
					log.debug("Error evaluating: " + req);
					
					return err + " " + req;
				}
				
			}
			else
			{
				this.log.error("Unregistered Ability Requirement : " + fn_name);
			}
		}
		else
		{
			req = ::Util.trim(req);

			if (req in this.RequirementTable)
			{
				if (req in this.EquipmentRequirementsTable)
				{
					this.mEquipRequirements.append(this.EquipmentRequirementsTable[req]);
				}

				return this.RequirementTable[req];
			}
			else
			{
				this.log.error("Unregistered Ability Requirement : " + req);
			}
		}

		return req;
	}

	function doesAbilityRequireTarget()
	{
		if (this.mActions.find("onActivate:ST:") != null || this.mActions.find("onActivate:STAE:") != null)
		{
			return true;
		}

		return false;
	}

	function _setSpecialRequirement( specialRequirements )
	{
		this.mSpecialRequirements = [];
		this.mEquipRequirements = [];
		this.mStatusRequirements = [];

		foreach( req in specialRequirements )
		{
			this.mSpecialRequirements.append(this._parseRequirement(req));
		}
	}

	function getSpecialRequirement()
	{
		return this.mSpecialRequirements;
	}

	function isReagentNeeded()
	{
		return this.mNeedReagents;
	}

	function _addReagents( reagentId, count )
	{
		this.mReagents[reagentId] <- count;
		this.mNeedReagents = true;
	}

	function getReagents()
	{
		return this.mReagents;
	}

	function getActivationCriteria()
	{
		return this.mActivationCriteria;
	}

	function setPurchasePointsRequired( value )
	{
		this.mPurchasePointsRequired = value;
	}

	function setPurchaseAbilityRequired( value )
	{
		this.mPurchaseAbilitiesRequired = value;
	}

	function setPurchaseClassRequired( value )
	{
		this.mPurchaseClassRequired = value;
	}

	function activate()
	{
		if (!this.mValid)
		{
			return false;
		}

		if (this.mAbilityClass == "Passive")
		{
			return;
		}

		::_AbilityManager.setCategoryCooldownTime(this.mCooldownCategory, this.mCooldownTime);
		::_quickBarManager.setCategoryUsable(this.mCooldownCategory, false);
		this.mRequestedUse = false;
		this.mWarmingUp = false;
		::_quickBarManager.abilityUsed(this.mId);

		if (this.mActivationCriteria.find("hasMainHandWeapon") != null || this.mActivationCriteria.find("has2HorPoleWeapon") != null || this.mActivationCriteria.find("hasShield") != null)
		{
			if (::_avatar.isRangedAutoAttackActive())
			{
				::_avatar.stopAutoAttack(true);
				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
			}

			if (!::_avatar.isMeleeAutoAttackActive())
			{
				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
				::_avatar.startAutoAttack(false, true);
			}
		}
		else if (this.mActivationCriteria.find("hasBow") || this.mActivationCriteria.find("hasWand") || this.mActivationCriteria.find("hasRangedWeapon"))
		{
			if (::_avatar.isMeleeAutoAttackActive())
			{
				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
				::_avatar.stopAutoAttack(false);
			}

			local eqScreen = this.Screens.get("Equipment", false);

			if (eqScreen)
			{
				local rangedAC = eqScreen.findMatchingContainer(this.ItemEquipSlot.WEAPON_RANGED);

				if (rangedAC)
				{
					local rangedItem = rangedAC.getActionInSlot(0);

					if (rangedItem)
					{
						if (!::_avatar.isRangedAutoAttackActive())
						{
							::_avatar.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
							::_avatar.startAutoAttack(true, true);
						}
					}
					else
					{
					}
				}
			}
		}

		return true;
	}

	function setSlotCoordinates( coords )
	{
		this.mCoordinates = coords;
	}

	function getSlotCoordinates()
	{
		return this.mCoordinates;
	}

	function setGroupId( value )
	{
		this.mGroupId = value;
	}

	function getGroupId()
	{
		return this.mGroupId;
	}

	function getOwnage()
	{
		return this.mOwnage;
	}

	function setOwnage( ownage )
	{
		this.mOwnage = ownage;
	}

	function getPurchasePointsRequired()
	{
		return this.mPurchasePointsRequired;
	}

	function getPurchaseLevelRequired()
	{
		return this.mPurchaseLevelRequired;
	}

	function getPurchaseAbilitiesRequired()
	{
		return this.mPurchaseAbilitiesRequired;
	}

	function getPurchaseClassRequired()
	{
		return this.mPurchaseClassRequired;
	}

	function setTier( tier )
	{
		this.mTier = tier;
	}

	function getTier()
	{
		return this.mTier;
	}

	function setDuration( duration )
	{
		this.mDuration = duration;
	}

	function getDuration()
	{
		return this.mDuration;
	}

	function cancel()
	{
		this.mWarmupEndTime = 0.0;
		this.mWarmingUp = false;

		if (this.mChannelBar)
		{
			this.mChannelBar.setVisible(false);
			this.mChannelBar = null;
		}
	}

	function setValid( which )
	{
		this.mValid = which;
	}

	function getIsValid()
	{
		return this.mValid;
	}

	function getId()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mId;
	}

	function setCategory( category )
	{
		this.mCategory = category;
	}

	function setCooldownCategory( cooldownCategory )
	{
		this.mCooldownCategory = cooldownCategory;
	}

	function getCategory()
	{
		if (!this.mValid)
		{
			return "";
		}

		return this.mCategory;
	}

	function getCooldownCategory()
	{
		return this.mCooldownCategory;
	}

	function setDescription( name )
	{
		this.mDescription = name;
	}

	function _parseDescription( desc )
	{
		local result = "";
		local current = desc;
		local stats = {};

		foreach( statId, value in this._avatar.getStats() )
		{
			local stat_name = "a_" + ::Stat[statId].name.tolower();
			stats[stat_name] <- value;
		}

		while (true)
		{
			local bpos = current.find("<?");
			local epos = current.find("?>");

			if (bpos < epos)
			{
				local prev = current.slice(0, bpos);
				local formula = current.slice(bpos + 2, epos);
				current = current.slice(epos + 2);

				try
				{
					this.log.debug("Parsing formula: " + formula);
					formula = this.eval(" return " + formula.tolower() + ";", stats);
				}
				catch( err )
				{
					this.log.debug("Malformed formula in ability: " + formula);
					formula = "ERR?";
				}

				result = result + prev + formula.tointeger().tostring();
			}
			else
			{
				return result + current;
			}
		}
	}

	function getDescription()
	{
		if (!this.mValid)
		{
			return "";
		}

		return this._parseDescription(this.mDescription);
	}

	function setRange( value )
	{
		this.mRange = value;
	}

	function getRange()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mRange;
	}

	function getTooltip( mods )
	{
		if (!this.mValid)
		{
			return "";
		}

		if (mods == null)
		{
			mods = {};
		}

		this.setupAdditionalMods(mods);
		local force = true;

		if ("force" in mods)
		{
			force = mods.force;
		}

		if (this.mTooltipComponent && !force)
		{
			return this.mTooltipComponent;
		}
		else
		{
			this.mTooltipComponent = ::AbilityUtil.buildToolTipComponent(this.mId, mods);

			if (this.mTooltipComponent)
			{
				return this.mTooltipComponent;
			}
			else
			{
				return "";
			}
		}
	}

	function setupAdditionalMods( mods )
	{
		if (this.mId == 189)
		{
			mods.force <- true;
			local bindLocation = ::_avatar.getStat(this.Stat.TRANSLOCATE_DESTINATION);

			if (bindLocation != null)
			{
				mods.bindLocation <- bindLocation;
			}
			else
			{
				mods.bindLocation <- "nowhere";
			}
		}
	}

	function isUsable()
	{
		return this.mValid;
	}

	function isAvailableForUse()
	{
		local timeUntilAvail = this.getTimeUntilAvailable();

		if (timeUntilAvail <= 0)
		{
			return ::_AbilityManager.getTimeUntilCategoryUseable("Global") <= 0;
		}
		else
		{
			return false;
		}
	}

	function getQuickbarString()
	{
		return "ABILITY" + "id:" + this.mId;
	}

	function getTimeUntilAvailable()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return ::_AbilityManager.getTimeUntilCategoryUseable(this.mCooldownCategory);
	}

	function getTimeUsed()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return ::_AbilityManager.getCategoryUseTime(this.mCooldownCategory);
	}

	function getType()
	{
		if (!this.mValid)
		{
			return "";
		}

		return "ability";
	}

	function setWarmupCue( value )
	{
		this.mWarmupCue = value;
	}

	function getWarmupCue()
	{
		if (!this.mValid)
		{
			return "";
		}

		return this.mWarmupCue;
	}

	function setCooldownDuration( value )
	{
		this.mCooldownTime = value;
	}

	function getCooldownDuration()
	{
		return this.mCooldownTime;
	}

	function setWarmupDuration( value )
	{
		this.mWarmupDuration = value;
	}

	function getWarmupDuration()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mWarmupDuration;
	}

	function getWarmupEndTime()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mWarmupEndTime + this.mWarmupSetbackTime;
	}

	function getWarmupStartTime()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mWarmupStartTime;
	}

	function getWarmupTimeLeft()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.getWarmupEndTime() - this.mTimer.getMilliseconds();
	}

	function getChannelTimeLeft()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.getChannelEndTime() - this.mTimer.getMilliseconds();
	}

	function getChannelStartTime()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mChannelStartTime;
	}

	function getChannelEndTime()
	{
		if (!this.mValid)
		{
			return 0;
		}

		return this.mChannelEndTime;
	}

	function setVisualCue( value )
	{
		this.mVisualCue = value;
	}

	function getVisualCue()
	{
		if (!this.mValid)
		{
			return "";
		}

		return this.mVisualCue;
	}

	function setActions( value )
	{
		this.mActions = value;
	}

	function getActions()
	{
		if (!this.mValid)
		{
			return "";
		}

		return this.mActions;
	}

	function isWarmingUp()
	{
		if (!this.mValid)
		{
			return false;
		}

		return this.mWarmingUp;
	}

	function isAwaitingServerResponse()
	{
		return this.mRequestedUse;
	}

	function sendActivationRequest( ... )
	{
		local activate = vargc > 0 ? vargv[0] : true;
		local flags = vargc > 1 ? vargv[1] : 0;
		local ground = vargc > 2 ? vargv[2] : null;

		if (!this.mValid)
		{
			return false;
		}

		local actions = this.getActions();

		if (actions.find("GTAE") != null && this._groundTargetTool)
		{
			if (!this._groundTargetTool.inUse())
			{
				this._groundTargetTool.setAbility(this);
				local indexOfGTAE = actions.find("GTAE");
				local indexOfOpenParam = actions.find("(", indexOfGTAE);
				local indexOfCloseParam = actions.find(")", indexOfOpenParam);
				local size = actions.slice(indexOfOpenParam + 1, indexOfCloseParam).tointeger();
				this._groundTargetTool.setSize(size * 2, size * 2);
				this._tools.push(this._groundTargetTool);
				this.mBroadcaster.broadcastMessage("onGroundTargetActivate");
			}
			else
			{
				this.print("Already have another ability in use");
			}

			return true;
		}

		if (this._AbilityManager.isPartyCasting())
		{
			flags = flags | this.AbilityFlags.PARTY_CAST;
		}

		if (this.isAvailableForUse() || this.mCooldownCategory == "autoMelee")
		{
			if (this.mCooldownCategory != "autoMelee")
			{
				::_quickBarManager.abilityUseRequested(this.mId);
			}

			::_tutorialManager.onAbilityActivated(this.mId);

			if (::_avatar.isDead())
			{
				local foundUseableAbility = false;
				local mDeathAvailableAbilityIds = [
					10000,
					10001,
					10002,
					10003
				];

				foreach( abilityId in mDeathAvailableAbilityIds )
				{
					if (abilityId == this.mId)
					{
						foundUseableAbility = true;
					}
				}

				if (!foundUseableAbility)
				{
					return false;
				}
			}

			if (activate)
			{
				this._Connection.sendAbilityActivate(this.mId, flags, ground);
			}
			else
			{
				this._Connection.sendAbilityActivate(-this.mId, flags, ground);
			}

			this.mRequestedUse = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	function sendActivationTargeted( ... )
	{
		local activate = vargc > 0 ? vargv[0] : true;
		local flags = vargc > 1 ? vargv[1] : 0;
		local ground = vargc > 2 ? vargv[2] : null;
		this.mBroadcaster.broadcastMessage("onGroundTargetDone");

		if (!this.mValid)
		{
			return false;
		}

		if (this._AbilityManager.isPartyCasting())
		{
			flags = flags | this.AbilityFlags.PARTY_CAST;
		}

		if (::_avatar.isDead())
		{
			local foundUseableAbility = false;
			local mDeathAvailableAbilityIds = [
				10000,
				10001,
				10002,
				10003
			];

			foreach( abilityId in mDeathAvailableAbilityIds )
			{
				if (abilityId == this.mId)
				{
					foundUseableAbility = true;
				}
			}

			if (!foundUseableAbility)
			{
				return false;
			}
		}

		if (this.isAvailableForUse())
		{
			if (activate)
			{
				this._Connection.sendAbilityActivate(this.mId, flags, ground);
			}
			else
			{
				this._Connection.sendAbilityActivate(-this.mId, flags, ground);
			}

			this.mRequestedUse = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	function warmup( showProgress, creature )
	{
		this.mRequestedUse = false;

		if (this.mWarmupDuration > 0 && showProgress)
		{
			this.mWarmupStartTime = this.mTimer.getMilliseconds();
			local warmupDuration = this.mWarmupDuration;
			local totalCastMod = 0.0;
			local modCastingSpeed = creature.getStat(this.Stat.MOD_CASTING_SPEED);
			local magicAttackSpeed = creature.getStat(this.Stat.MAGIC_ATTACK_SPEED);

			if (modCastingSpeed)
			{
				totalCastMod += modCastingSpeed;
			}

			if (modCastingSpeed)
			{
				totalCastMod += magicAttackSpeed * 0.001;
			}

			warmupDuration -= warmupDuration * totalCastMod;

			if (warmupDuration <= 0)
			{
				return;
			}

			this.mWarmupEndTime = this.mWarmupStartTime + warmupDuration;
			this.mWarmupSetbackTime = 0;
			this.mWarmingUp = true;
			this.mWarmupBar = this.GUI.WarmupBar(this);
			this.mWarmupBar.setOverlay(this.GUI.POPUP_OVERLAY);
			this.mWarmupBar.setText(this.getName());
			local width = this.mWarmupBar.getWidth() / 2;
			local height = this.mWarmupBar.getHeight() / 2;
			this.mWarmupBar.setPosition(::Screen.getWidth() / 2 - width, ::Screen.getHeight() * 0.85000002 - height);
		}
	}

	function channel( showProgress, creature, channelLengthMultiplier )
	{
		if (this.mDuration > 0 && showProgress)
		{
			this.mChannelStartTime = this.mTimer.getMilliseconds();
			this.mChannelEndTime = this.mChannelStartTime + this.mDuration * channelLengthMultiplier;
			this.mChannelBar = this.GUI.ChannelBar(this);
			this.mChannelBar.setOverlay(this.GUI.POPUP_OVERLAY);
			this.mChannelBar.setText(this.getName());
			local width = this.mChannelBar.getWidth() / 2;
			local height = this.mChannelBar.getHeight() / 2;
			this.mChannelBar.setPosition(::Screen.getWidth() / 2 - width, ::Screen.getHeight() * 0.85000002 - height);
		}
	}

	function setback()
	{
		if (this.mWarmupSetbackTime == 0)
		{
			this.mWarmupSetbackTime = 1000;
		}
		else if (this.mWarmupSetbackTime == 1000)
		{
			this.mWarmupSetbackTime = 1500;
		}
		else if (this.mWarmupSetbackTime == 1500)
		{
			this.mWarmupSetbackTime = 1750;
		}
		else if (this.mWarmupSetbackTime == 1750)
		{
			this.mWarmupSetbackTime = 2000;
		}
	}

	function getEquipRequirements()
	{
		return this.mEquipRequirements;
	}

	function hasRequiredEquipment()
	{
		if (this.mEquipRequirements.len() == 0)
		{
			return true;
		}

		local equipScreen = this.Screens.get("Equipment", true);

		if (!::_avatar || !equipScreen)
		{
			return false;
		}

		foreach( key, equipData in this.mEquipRequirements )
		{
			if (equipData.equipType == "EitherWeapon")
			{
				local types = equipData.types;
				local found = false;

				foreach( equipType in types )
				{
					if (this._hasItemEquip(this.EquipmentRequirementsTable[equipType]))
					{
						found = true;
						break;
					}
				}

				if (found == false)
				{
					return false;
				}
			}
			else if (!this._hasItemEquip(equipData))
			{
				return false;
			}
		}

		return true;
	}

	function _hasItemEquip( equipData )
	{
		switch(equipData.equipType)
		{
		case "ItemEquipSlot":
			if (!this._hasItemInEquipSlot(equipData.slot))
			{
				return false;
			}

			break;

		case "ItemEquipType":
			if (!this._hasItemEquipTypeEquiped(equipData.slot))
			{
				return false;
			}

			break;

		case "WeaponType":
			if (!this._hasWeaponTypeEquiped(equipData.slot, equipData.itemEquipSlot))
			{
				return false;
			}

			break;
		}

		return true;
	}

	function _hasItemInEquipSlot( slot )
	{
		local equipScreen = this.Screens.get("Equipment", true);
		local equipContainer = equipScreen.findMatchingContainer(slot);

		if (!equipContainer || !equipContainer.getActionInSlot(0) || !("mItemData" in equipContainer.getActionInSlot(0)))
		{
			return false;
		}

		return true;
	}

	function _hasItemEquipTypeEquiped( equipType )
	{
		local equipScreen = this.Screens.get("Equipment", true);

		if (!::_avatar || !equipScreen)
		{
			return false;
		}

		local containerSlots = ::EquipmentMapContainer[equipType];

		foreach( container in containerSlots )
		{
			local actionContainer = equipScreen.findMatchingContainer(container);

			if (actionContainer)
			{
				local action = actionContainer.getActionInSlot(0);

				if ("mItemData" in action)
				{
					local itemId = action.mItemData;
					local itemDef = ::_ItemDataManager.getItemDef(itemId.mItemDefId);
					local equipPosition = itemDef.getEquipType();

					if (equipPosition == equipType)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	function _hasWeaponTypeEquiped( weaponType, itemEquipSlot )
	{
		local equipScreen = this.Screens.get("Equipment", true);

		if (!::_avatar || !equipScreen)
		{
			return false;
		}

		local actionContainer = equipScreen.findMatchingContainer(itemEquipSlot);

		if (actionContainer)
		{
			local action = actionContainer.getActionInSlot(0);

			if ("mItemData" in action)
			{
				local itemId = action.mItemData;
				local itemDef = ::_ItemDataManager.getItemDef(itemId.mItemDefId);
				local myWeaponType = itemDef.getWeaponType();

				if (myWeaponType == weaponType)
				{
					return true;
				}
			}
		}

		return false;
	}

	function _hasPercentileHealth( percentile )
	{
		if (!::_avatar || !::_combatEquations)
		{
			return false;
		}

		local con = ::_avatar.getStat(this.Stat.CONSTITUTION, true);
		local baseHealth = ::_avatar.getStat(this.Stat.BASE_HEALTH, true);
		local bonusHealth = ::_avatar.getStat(this.Stat.HEALTH_MOD, true);
		local maxHealth = ::_combatEquations.calcMaxHealth(baseHealth, con, bonusHealth, 0);
		local curHealth = ::_avatar.getStat(this.Stat.HEALTH, true);

		if (maxHealth == 0)
		{
			return false;
		}

		local currHealthPercent = curHealth.tofloat() / maxHealth.tofloat();

		if (currHealthPercent < percentile)
		{
			return false;
		}

		return true;
	}

	function getStatusRequirements()
	{
		return this.mStatusRequirements;
	}

	
	/*
	 * Checks to see if player has the all the status requirements in order to activate the abilities
	 */
	function hasStatusRequirements()
	{
		//Go through all the status requirements needed, and check if player
		//has the requirements to activate their ability
		foreach( req in mStatusRequirements )	{
			try
			{
				local percentHealthTag = "PercentMaxHealth";

				if (req.find(percentHealthTag) != null)	{
					local data = ::Util.replace(req, percentHealthTag + "(", "");
					data = ::Util.replace(data, ")", "");
					local percentile = data.tofloat();

					if (!_hasPercentileHealth(percentile))
						return false;
				}
				else {
					//If Avatar is null, reset avatar
					if (!StatusRequirementsTable.avatar && ::_avatar)
						StatusRequirementsTable.avatar <- _avatar;

					local rule = ::Util.replace(req, ")", ",avatar)");

					if (!eval("return " + rule + ";", StatusRequirementsTable))
						return false;
				}
			}
			catch( err ) {
				log.debug("Error evaluating: " + req + " errMessage: " + err);
				return false;
			}
		}

		return true;
	}

	function showExtraDataScreen()
	{
		local MAX_HEIGHT = 400;
		local WIDTH = 350;

		if (!this.mInfoFrame)
		{
			this.mInfoFrame = this.GUI.Frame("Ability Action: " + this.mName);
		}

		this.mInfoFrame.setVisible(true);
		local component = this.GUI.Component(this.GUI.BoxLayoutV());
		component.setInsets(5, 5, 5, 5);
		component.getLayoutManager().setAlignment(0);
		local heightSize = 16;
		local height = 0;
		local textString = "";
		textString = this.Util.addNewTextLine(textString, "Ability Id", this.getId());
		textString = this.Util.addNewTextLine(textString, "Foreground Image", this.mForegroundImage);
		textString = this.Util.addNewTextLine(textString, "Background Image", this.mBackgroundImage);
		height = heightSize * 3;
		local ability = ::_AbilityManager.getAbilityById(this.getId());

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

class this.MeleeAbility extends this.Action
{
	constructor( image )
	{
		this.setImage(image);
		this.mActionSound = this.ToggleMeleeAbilityActionSound(this);
	}

	function sendActivationRequest()
	{
		if (::_avatar.isRangedAutoAttackActive())
		{
			::_avatar.stopAutoAttack(true);
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
		}

		if (::_avatar.isMeleeAutoAttackActive())
		{
			local ab = this._AbilityManager.getAbilityByName("stop_melee");

			if (ab)
			{
				ab.sendActivationRequest();
			}

			::_avatar.stopAutoAttack(false);
		}
		else
		{
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
			::_avatar.startAutoAttack(false, true);
		}
	}

	function getTooltip( mods )
	{
		return this.GUI.Label("Melee Auto-Attack");
	}

}

class this.RangedAbility extends this.Action
{
	constructor( image )
	{
		this.setImage(image);
		this.mActionSound = this.ToggleMeleeAbilityActionSound(this);
	}

	function sendActivationRequest()
	{
		if (::_avatar.isMeleeAutoAttackActive())
		{
			::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
			::_avatar.stopAutoAttack(false);
		}

		if (::_avatar.isRangedAutoAttackActive())
		{
			local ab = this._AbilityManager.getAbilityByName("stop_melee");

			if (ab)
			{
				ab.sendActivationRequest();
			}

			::_avatar.stopAutoAttack(true);
		}
		else
		{
			local eqScreen = this.Screens.get("Equipment", false);

			if (eqScreen)
			{
				local rangedAC = eqScreen.findMatchingContainer(this.ItemEquipSlot.WEAPON_RANGED);

				if (rangedAC)
				{
					local rangedItem = rangedAC.getActionInSlot(0);

					if (rangedItem)
					{
						::_avatar.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
						::_avatar.startAutoAttack(true, true);
					}
					else
					{
						this.IGIS.error("You don\'t have a range weapon equipped.");
						return;
					}
				}
			}
		}
	}

	function getTooltip( mods )
	{
		return this.GUI.Label("Ranged Auto-Attack");
	}

}

