this.BuffType <- {
	BUFF = 0,
	DEBUFF = 1,
	WORLD = 2
};
class this.BuffDebuff extends this.Action
{
	mStatusModifierList = null;
	mAbilitySource = -1;
	mDuration = -1;
	mRecievedAt = -1;
	mCreature = null;
	constructor( creature, statusMod, image )
	{
		this.mStatusModifierList = [];
		this.mCreature = creature;
		this.mAbilitySource = statusMod.getAbilityID();
		this.mDuration = statusMod.getDuration();
		this.mRecievedAt = ::_gameTime.getGameTimeMiliseconds() / 1000.0;
		this.addStatusModifier(statusMod);
		this.Action.constructor("", image);
	}

	function addStatusModifier( statusModifier )
	{
		this.mStatusModifierList.append(statusModifier);
	}

	function getAbilityID()
	{
		return this.mAbilitySource;
	}

	function getTooltip( mods )
	{
		local container = this.GUI.Container(this.GUI.BoxLayoutV());
		container.getLayoutManager().setAlignment(0.0);
		container.setSize(150, 125);
		local abilityInfo = ::_AbilityManager.getAbilityById(this.mAbilitySource);
		
		local percentMultiplier = [
			128,
			129,
			130,
			131,
			132,
			133,
			162,
			163,
			330,
			331,
			332,
			385,
			386,
			387,
			388,
			478,
			479,
			480,
			481,
			5003,
			5008,
			5009,
			5015,
			5029,
			5030,
			5031,
			5032,
			5033,
			5034,
			5035,
			5036,
			5037,
			5038,
			5039,
			5040,
			5041,
			5042,
			5043,
			5045,
			5046,
			5047,
			5048,
			5063,
			5064,
			5065,
			5071,
			5072,
			5073,
			5074,
			5075,
			5076,
			5105,
			5106,
			5263,
			5264,
			5265,
			5318,
			5319,
			5320,
			5321,
			10000
		];

		if (abilityInfo.getIsValid())
		{
			local spacer = this.GUI.Spacer(150, 1);
			spacer.setAppearance("ColumnList/HeadingDivider");
			container.add(this.GUI.HTML("<b>" + abilityInfo.getName() + "</b>"));
			container.add(spacer);
			container.add(this.GUI.HTML(this.getCategoryType()));
			spacer = this.GUI.Spacer(150, 1);
			spacer.setAppearance("ColumnList/HeadingDivider");
			container.add(spacer);
			local statText = "";

			foreach( statMod in this.mStatusModifierList )
			{
				local statTextPrefix = "+";
				local amount = statMod.getAmount();

				if (amount < 0.0)
				{
					statTextPrefix = "-";
					amount = -amount;
				}

				local abilityId = this.mAbilitySource.tointeger();
				local foundId = false;

				foreach( id in percentMultiplier )
				{
					if (id == abilityId)
					{
						foundId = true;
						break;
					}
				}

				if (this.mCreature && ("baseStat" in this.Stat[statMod.getStatID()]) && foundId)
				{
					local baseStat = this.mCreature.getBaseStatValue(this.Stat[statMod.getStatID()].baseStat);

					if (baseStat == 0)
					{
						amount = amount * 100;
						amount = amount + "%";
					}
					else
					{
						local modAmount = amount;
						amount = baseStat * amount;
						amount = amount.tointeger();

						if (amount == 0)
						{
							if (modAmount > 9.9999997e-005)
							{
								amount++;
							}
							else if (modAmount < -9.9999997e-005)
							{
								amount--;
							}
						}
					}
				}

				local description = statMod.getDescription();

				if (description != "")
				{
					statText += description + "<br>";
				}
				else
				{
					statText += statTextPrefix + amount + " " + this.Stat[statMod.getStatID()].prettyName + "<br>";
				}
			}

			local stats = this.GUI.HTML(statText);
			container.add(stats);
			spacer = this.GUI.Spacer(150, 1);
			spacer.setAppearance("ColumnList/HeadingDivider");
			container.add(spacer);
			local currentGameTime = ::_gameTime.getGameTimeMiliseconds() / 1000;
			local runningFor = currentGameTime - this.mRecievedAt;
			local timeRemaining = this.mDuration - runningFor.tointeger();
			local timeTable = this.Util.paraseSecToTable(timeRemaining);
			local timeText = "Duration: ";

			if (timeTable.d > 0)
			{
				timeText += timeTable.d + "d ";
			}

			if (timeTable.h > 0)
			{
				timeText += timeTable.h + "h ";
			}

			if (timeTable.m > 0)
			{
				timeText += timeTable.m + "m ";
			}

			if (timeTable.s > 0)
			{
				timeText += timeTable.s + "s ";
			}

			//print("ICE! Tooltip (" + this.mAbilitySource + "): running for: " + runningFor + " current game time: " + currentGameTime + " receivedAt: " + this.mRecievedAt + " dur: " + this.mDuration + " timeRem: " + timeRemaining + " = " + timeText + "\n");
			
			local timeLeftLabel = this.GUI.Label(timeText);
			container.add(timeLeftLabel);
		}
		else
		{
			container.add(this.GUI.HTML("<b>Loading...</b>"));
		}

		return container;
	}

	function getBuffType()
	{
		local abilityInfo = ::_AbilityManager.getAbilityById(this.mAbilitySource);
		local category = abilityInfo.getBuffCategory();

		if (this.Util.startsWith(category, "+"))
		{
			return this.BuffType.BUFF;
		}
		else if (this.Util.startsWith(category, "-"))
		{
			return this.BuffType.DEBUFF;
		}
		else if (this.Util.startsWith(category, "w"))
		{
			return this.BuffType.WORLD;
		}
	}

	function getCategoryType()
	{
		local abilityInfo = ::_AbilityManager.getAbilityById(this.mAbilitySource);
		local category = abilityInfo.getBuffCategory();
		local splitStrings = this.Util.split(category, "/");

		if (splitStrings.len() == 3)
		{
			return splitStrings[1];
		}
		else
		{
			return "";
		}
	}

	function getDuration()
	{
		return this.mDuration;
	}

}

class this.StatusModifier 
{
	mStatID = null;
	mAbilityID = -1;
	mAmount = -1;
	mDuration = -1;
	mRecievedAt = -1;
	mDescription = "";
	constructor( statID, abilityID, amount, duration, description )
	{
		this.mStatID = statID;
		this.mAmount = amount;
		this.mAbilityID = abilityID;
		this.mDuration = duration;
		this.mDescription = description;
	}

	function getStatID()
	{
		return this.mStatID;
	}

	function getAbilityID()
	{
		return this.mAbilityID;
	}

	function getAmount()
	{
		return this.mAmount;
	}

	function getDescription()
	{
		return this.mDescription;
	}

	function getDuration()
	{
		return this.mDuration;
	}

}

class this.StatusEffect extends this.Action
{
	mStatusEffectType = -1;
	constructor( effectType )
	{
		this.mStatusEffectType = effectType;
		this.Action.constructor("", this.StatusEffects[this.mStatusEffectType].icon);
	}

	function getEffectID()
	{
		return this.mStatusEffectType;
	}

	function getBuffType()
	{
		if (this.StatusEffects[this.mStatusEffectType].type == "Debuff")
		{
			return this.BuffType.DEBUFF;
		}
		else if (this.StatusEffects[this.mStatusEffectType].type == "Buff")
		{
			return this.BuffType.BUFF;
		}
		else if (this.StatusEffects[this.mStatusEffectType].type == "World")
		{
			return this.BuffType.WORLD;
		}
	}

	function getTooltip( mods )
	{
		return this.StatusEffects[this.mStatusEffectType].prettyName;
	}

}

