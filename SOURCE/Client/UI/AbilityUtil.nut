this.AbilityUtil <- {};
this.AbilityUtil.buildToolTipComponent <- function ( abilityId, ... )
{
	local mods;

	if (vargc > 0)
	{
		mods = vargv[0];
	}

	local C_ABILITY_INFO_WIDTH = 250;
	local toolTipComponent = this.GUI.Container(this.GUI.BoxLayoutV());
	toolTipComponent.getLayoutManager().setExpand(true);
	local selectedAbility = ::_AbilityManager.getAbilityById(abilityId);

	if (abilityId == 0 || !selectedAbility || selectedAbility && selectedAbility.getName() == "")
	{
		return toolTipComponent;
	}

	local nameLabel = this.GUI.Label(selectedAbility.getName());
	toolTipComponent.add(nameLabel);
	nameLabel.setFontColor(this.Color(1.0, 1.0, 0.0, 1.0));
	local showBindingInfo = true;

	if (mods)
	{
		if ("showBindingInfo" in mods)
		{
			showBindingInfo = mods.showBindingInfo;
		}

		local label;

		if (showBindingInfo)
		{
			if ("bind" in mods)
			{
				switch(mods.bind)
				{
				case "bound":
					label = this.GUI.HTML();
					label.setText("<font color=\"FF0000\"><b>" + "Bound to character" + "</b></font>");
					break;

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
		}

		if (label != null)
		{
			toolTipComponent.add(label);
		}
	}

	local dynamicComponent = ::AbilityUtil.generateDynamicComponent(selectedAbility, null, true);
	toolTipComponent.add(dynamicComponent);
	local descriptionHTML = this.GUI.HTML(selectedAbility.getDescription());
	descriptionHTML.setMaximumSize(C_ABILITY_INFO_WIDTH, 400);
	descriptionHTML.setMinimumSize(C_ABILITY_INFO_WIDTH, 400);
	descriptionHTML.setResize(true);
	toolTipComponent.add(descriptionHTML);

	if (mods)
	{
		if ("bindLocation" in mods)
		{
			local bindLocationHTML = this.GUI.HTML("Current Bind location: " + mods.bindLocation);
			toolTipComponent.add(bindLocationHTML);
		}
	}

	return toolTipComponent;
};
this.AbilityUtil.handleSectionText <- function ( dynamicComponent, currComponent, endText )
{
	local MIN_RIGHT_SIDE = 85;
	local section = this.GUI.Container(this.GUI.GridLayout(1, 2));
	section.getLayoutManager().setColumns("*", MIN_RIGHT_SIDE);
	dynamicComponent.add(section);
	section.add(currComponent);
	local htmlComp = this.GUI.HTML(endText);
	section.add(htmlComp, {
		anchor = this.GUI.GridLayout.RIGHT
	});
};
this.AbilityUtil.generateDynamicComponent <- function ( selectedAbility, myComponent, ... )
{
	local showExtraData = false;

	if (vargc > 0 && vargv[0])
	{
		showExtraData = vargv[0];
	}

	local dynamicComponent;

	if (myComponent)
	{
		dynamicComponent = myComponent;
		dynamicComponent.removeAll();
	}
	else
	{
		dynamicComponent = this.GUI.Component(this.GUI.BoxLayoutV());
	}

	dynamicComponent.getLayoutManager().setAlignment(0.0);
	dynamicComponent.getLayoutManager().setExpand(true);

	if (!selectedAbility)
	{
		local invisibleLabel = this.GUI.Label("");
		dynamicComponent.add(invisibleLabel);
		return dynamicComponent;
	}

	local castsection = this.GUI.Container(this.GUI.GridLayout(1, 2));
	dynamicComponent.add(castsection);
	local abilityUseStr = "";

	if (selectedAbility.getUseType() & this.AbilityUseType.CAST)
	{
		abilityUseStr = "Cast";
	}
	else if (selectedAbility.getUseType() & this.AbilityUseType.CHANNELED)
	{
		abilityUseStr = "Channel";
	}
	else if (selectedAbility.getUseType() & this.AbilityUseType.PASSIVE)
	{
		abilityUseStr = "Passive";
	}

	if (selectedAbility.getWarmupDuration() != 0)
	{
		local timeLabel = this.GUI.HTML(::Util.parseMilliToTimeStr(selectedAbility.getWarmupDuration()) + abilityUseStr);
		timeLabel.setFontColor(this.Color(1.0, 1.0, 1.0, 1.0));
		castsection.add(timeLabel);
	}
	else if (selectedAbility.getWarmupDuration() == 0 && abilityUseStr == "Cast")
	{
		local timeLabel = this.GUI.HTML("Instant " + abilityUseStr);
		timeLabel.setFontColor(this.Color(1.0, 1.0, 1.0, 1.0));
		castsection.add(timeLabel);
	}
	else
	{
		local timeLabel = this.GUI.HTML(abilityUseStr);
		timeLabel.setFontColor(this.Color(1.0, 1.0, 1.0, 1.0));
		castsection.add(timeLabel);
	}

	local typeHTML = this.GUI.HTML(selectedAbility.getAbilityClass());
	castsection.add(typeHTML, {
		anchor = this.GUI.GridLayout.RIGHT
	});
	local tierText = "Tier: " + selectedAbility.getTier();
	local rangeText = "";

	if (selectedAbility.getRange() > -1)
	{
		rangeText = "Range: " + selectedAbility.getRange() / 10 + "m";
	}

	local foundTier = false;

	if (selectedAbility.getMight() != 0)
	{
		local costMightLabel = this.GUI.HTML("<font color=\"" + this.Colors.white + "\">Cost:  </font><font color=\"" + this.Colors.mint + "\"> " + selectedAbility.getMight() + " Might</font>");
		this.AbilityUtil.handleSectionText(dynamicComponent, costMightLabel, tierText);
		tierText = rangeText;
		foundTier = true;
	}

	if (selectedAbility.getWill() != 0)
	{
		local costWillLabel = this.GUI.HTML("<font color=\"" + this.Colors.white + "\">Cost:  </font><font color=\"" + this.Colors.sky + "\"> " + selectedAbility.getWill() + " Will</font>");
		this.AbilityUtil.handleSectionText(dynamicComponent, costWillLabel, tierText);

		if (foundTier)
		{
			tierText = "";
		}
		else
		{
			tierText = rangeText;
			foundTier = true;
		}
	}

	if (selectedAbility.getAddMeleeCharge() != 0)
	{
		local meleeLabel = this.GUI.HTML("<font color=\"" + this.Colors.white + "\">Generates </font><font color=\"" + this.Colors.mint + "\"> " + selectedAbility.getAddMeleeCharge() + " Physical Charge</font>");
		this.AbilityUtil.handleSectionText(dynamicComponent, meleeLabel, tierText);

		if (foundTier)
		{
			tierText = "";
		}
		else
		{
			tierText = rangeText;
			foundTier = true;
		}
	}

	if (selectedAbility.getAddMagicCharge() != 0)
	{
		local magicLabel = this.GUI.HTML("<font color=\"" + this.Colors.white + "\">Generates </font><font color=\"" + this.Colors.sky + "\"> " + selectedAbility.getAddMagicCharge() + " Magic Charge</font>");
		this.AbilityUtil.handleSectionText(dynamicComponent, magicLabel, tierText);

		if (foundTier)
		{
			tierText = "";
		}
		else
		{
			tierText = rangeText;
			foundTier = true;
		}
	}

	if (showExtraData)
	{
		if (selectedAbility.getCooldownDuration() != 0)
		{
			local cooldownLabel = this.GUI.HTML("Cooldown " + ::Util.parseMilliToTimeStr(selectedAbility.getCooldownDuration()));
			cooldownLabel.setFontColor(this.Color(1.0, 1.0, 1.0, 1.0));
			this.AbilityUtil.handleSectionText(dynamicComponent, cooldownLabel, tierText);

			if (foundTier)
			{
				tierText = "";
			}
			else
			{
				tierText = rangeText;
				foundTier = true;
			}
		}

		if (selectedAbility.getDuration() != 0)
		{
			local durationLabel = this.GUI.HTML("Duration " + ::Util.parseMilliToTimeStr(selectedAbility.getDuration()));
			durationLabel.setFontColor(this.Color(1.0, 1.0, 1.0, 1.0));
			this.AbilityUtil.handleSectionText(dynamicComponent, durationLabel, tierText);

			if (foundTier)
			{
				tierText = "";
			}
			else
			{
				tierText = rangeText;
				foundTier = true;
			}
		}
	}

	if (!foundTier && tierText != "")
	{
		local emptyHTML = this.GUI.HTML("");
		this.AbilityUtil.handleSectionText(dynamicComponent, emptyHTML, tierText);
		tierText = rangeText;

		if (tierText != "")
		{
			local nextEmptyHTML = this.GUI.HTML("");
			this.AbilityUtil.handleSectionText(dynamicComponent, nextEmptyHTML, tierText);
		}
	}
	else if (tierText != "")
	{
		local emptyHTML = this.GUI.HTML("");
		this.AbilityUtil.handleSectionText(dynamicComponent, emptyHTML, tierText);
	}

	local specialRequirements = selectedAbility.getSpecialRequirement();

	if (specialRequirements.len() > 0)
	{
		foreach( rule in specialRequirements )
		{
			if (this.Util.startsWith(rule, "CheckBuffLimits"))
			{
				continue;
			}

			if (this.Util.startsWith(rule, "NotSilenced"))
			{
				continue;
			}

			local label = this.GUI.HTML("");
			label.setFontColor(this.Colors.lavender);
			dynamicComponent.add(label);
			label.setText("Requires " + rule);
		}
	}

	local reagents = selectedAbility.getReagents();

	foreach( itemDefId, count in reagents )
	{
		local label = this.GUI.HTML("");
		label.setFontColor(this.Colors.lavender);
		dynamicComponent.add(label);
		local reagentName = "Reagent: ";
		local itemDef = ::_ItemDataManager.getItemDef(itemDefId);
		reagentName = reagentName + itemDef.getDisplayName() + " x " + count;
		label.setText("Consumes " + reagentName);
	}

	return dynamicComponent;
};
this.AbilityUtil.getPageIndexFromAbility <- function ( ab )
{
	local cat = this.Util.trim(ab.getCategory());
	local lower_cat = cat.tolower();

	if (lower_cat in this.AbilityCategoryType)
	{
		return this.AbilityCategoryType[lower_cat];
	}
	else
	{
		this.log.debug("AbilityScreen ERROR: Missing category in received Ability: " + ab.getName() + " category: " + lower_cat);
		return -1;
	}
};
this.AbilityUtil.getAbilityDefPath <- function ()
{
	local basePath = this._cache.getBaseURL();

	if (basePath.slice(0, 8) != "file:///")
	{
		throw this.Exception("Ability path unavailble for base URL: " + basePath);
	}

	basePath = basePath.slice(8);
	basePath += "/../../Media/Catalogs/";
	return basePath;
};
this.AbilityUtil.saveAbilityData <- function ( results )
{
	local name = "Abilities";
	local filename = this.AbilityUtil.getAbilityDefPath() + name + ".nut";
	local out = "AbilityIndex <- " + this.serialize(results) + ";";
	::System.writeToFile(filename, out);
};
