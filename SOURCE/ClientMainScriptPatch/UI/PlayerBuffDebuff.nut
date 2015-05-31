this.require("UI/ActionContainer");
this.require("UI/Screens");
this.MAX_BUFFS <- 8;
this.MAX_DEBUFFS <- 16;
class this.Screens.PlayerBuffDebuff extends this.GUI.Container
{
	mStatusEffectsPerRow = 8;
	mBuffContainer = null;
	mDebuffContainer = null;
	mNextFreeBuffColumn = 7;
	mNextFreeDebuffColumn = 7;
	mNextFreeDebuffRow = 0;
	mBlinkingEvents = null;
	mWorldBuffHolder = null;
	mWorldBuffContainer = null;
	mWorldBuffSep = null;
	mNextFreeWorldBuffColumn = 2;
	mRemovedBuffs = false;
	mRemovedDebuffs = false;
	mRemovedWorldBuffs = false;
	constructor( ... )
	{
		this.GUI.Container.constructor(null);
		this.mBlinkingEvents = {};
		local buffDebuffContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mBuffContainer = this.GUI.ActionContainer("buff_container", 1, this.mStatusEffectsPerRow, 2, 0, this, false);
		this.mDebuffContainer = this.GUI.ActionContainer("debuff_container", this.MAX_DEBUFFS / this.mStatusEffectsPerRow, this.mStatusEffectsPerRow, 2, 2, this, false);
		buffDebuffContainer.add(this.mBuffContainer);
		buffDebuffContainer.add(this.mDebuffContainer);
		buffDebuffContainer.setSize(270, 100);
		buffDebuffContainer.setPreferredSize(270, 100);
		this.mBuffContainer.setTransparent();
		this.mBuffContainer.setAllButtonsDraggable(false);
		this.mBuffContainer.setPassThru(true);
		this.mDebuffContainer.setTransparent();
		this.mDebuffContainer.setAllButtonsDraggable(false);
		this.mDebuffContainer.setPassThru(true);
		this.mWorldBuffHolder = this.GUI.Container();
		this.mWorldBuffContainer = this.GUI.ActionContainer("player_worldbuff", 1, 3, 2, 2, this, false);
		this.mWorldBuffContainer.setTransparent();
		this.mWorldBuffContainer.setAllButtonsDraggable(false);
		this.mWorldBuffContainer.setPassThru(true);
		this.mWorldBuffSep = this.GUI.Component();
		this.mWorldBuffSep.setSize(33, 33);
		this.mWorldBuffSep.setPreferredSize(33, 33);
		this.mWorldBuffSep.setAppearance("Icon/WorldBuffSep");
		this.mWorldBuffSep.setTooltip("World Buffs");
		this.mWorldBuffHolder.add(this.mWorldBuffContainer);
		this.mWorldBuffHolder.add(this.mWorldBuffSep);
		this.mWorldBuffHolder.setSize(148, 32);
		this.mWorldBuffHolder.setPreferredSize(148, 32);
		this.mWorldBuffHolder.setPosition(0, 0);
		buffDebuffContainer.setPosition(145, 0);
		this.add(this.mWorldBuffHolder);
		this.add(this.GUI.Spacer(5, 0));
		this.add(buffDebuffContainer);
		this.setPassThru(true);
		this.generateTooltipWithPassThru(true);
		this.setSize(406, 100);
		this.setPreferredSize(406, 100);
		local size = this.getSize();
		local miniMap = this.Screens.get("MiniMapScreen", false);

		if (miniMap)
		{
			local minimapSize = miniMap.getSize();
			local xPos = ::Screen.getWidth() - size.width - minimapSize.width - 15;
			this.setPosition(xPos, 5);
		}
		else
		{
			local xPos = ::Screen.getWidth() - size.width;
			this.setPosition(xPos, 5);
		}

		this.updateWorldBuffPosition();
	}

	function onScreenResize()
	{
		local size = this.getPreferredSize();
		this.setSize(size.width, size.height);
		local miniMap = this.Screens.get("MiniMapScreen", false);

		if (miniMap)
		{
			local minimapSize = miniMap.getSize();
			local xPos = ::Screen.getWidth() - size.width - minimapSize.width - 15;
			this.setPosition(xPos, 5);
		}
		else
		{
			local xPos = ::Screen.getWidth() - size.width;
			this.setPosition(xPos, 5);
		}
	}

	function onRightButtonReleased( actionButton, evt )
	{
		local actionSlot = actionButton.getActionButtonSlot();
		local action = actionButton.getAction();

		if (action && (action instanceof this.BuffDebuff) && actionSlot)
		{
			local actionContainer = actionSlot.getActionContainer();

			if (actionContainer)
			{
				if (actionContainer == this.mBuffContainer)
				{
					::_Connection.sendQuery("buff.remove", this, [
						action.getAbilityID()
					]);
				}
			}
		}
	}

	function playerBuffBlinkdown( input )
	{
		local actionContainer = input[0];
		local mod = input[1];
		local actionButtonIndex = actionContainer.findSlotIndexOfAction(mod);

		if (actionButtonIndex != -1 && mod.getAbilityID() in this.mBlinkingEvents)
		{
			local actionButton = actionContainer.getSlotContents(actionButtonIndex);

			if (actionButton)
			{
				local visible = !this.mBlinkingEvents[mod.getAbilityID()].visible;
				actionButton.setVisible(visible);
				local duration = mod.getDuration();

				if (duration >= 0.5)
				{
					local blinkEvent = ::_eventScheduler.fireIn(0.5, this, "playerBuffBlinkdown", [
						actionContainer,
						mod
					]);
					this.mBlinkingEvents[mod.getAbilityID()] = {
						event = blinkEvent,
						visible = visible
					};
				}
				else
				{
					delete this.mBlinkingEvents[mod.getAbilityID()];
				}
			}
		}
		else if (mod.getAbilityID() in this.mBlinkingEvents)
		{
			delete this.mBlinkingEvents[mod.getAbilityID()];
		}
	}

	function removeBuffMods( updateContainer )
	{
		if (this.mNextFreeBuffColumn != 7)
		{
			this.mNextFreeBuffColumn = 7;
			this.mBuffContainer.removeAllActions(updateContainer);
			this.mRemovedBuffs = true;
		}
	}

	function removeDebuffMods( updateContainer )
	{
		if (this.mNextFreeDebuffColumn != 7 || this.mNextFreeDebuffRow != 0)
		{
			this.mNextFreeDebuffColumn = 7;
			this.mNextFreeDebuffRow = 0;
			this.mDebuffContainer.removeAllActions(updateContainer);
			this.mRemovedDebuffs = true;
		}
	}

	function removeWorldMods( updateContainer )
	{
		if (this.mNextFreeWorldBuffColumn != 2)
		{
			this.mNextFreeWorldBuffColumn = 2;
			this.mWorldBuffContainer.removeAllActions(updateContainer);
			this.mRemovedWorldBuffs = true;
		}
	}

	function updateMods( currentMods )
	{
		local dirtyBuffContainer = false;
		local dirtyDebuffContainer = false;
		local dirtyWorldContainer = false;
		this.removeBuffMods(false);
		this.removeDebuffMods(false);
		this.removeWorldMods(false);
		
		foreach( mod in currentMods )
		{
			if (!(mod instanceof this.BuffDebuff) && !(mod instanceof this.StatusEffect))
			{
				continue;
			}

			if (mod instanceof this.StatusEffect)
			{
				if (this.StatusEffects[mod.getEffectID()].icon == "")
				{
					continue;
				}
			}

			local modType = mod.getBuffType();
			local actionContainer;
			local indexToPlaceActionIn = -1;
			

			if (modType == this.BuffType.BUFF)
			{
				indexToPlaceActionIn = this.mNextFreeBuffColumn;
				actionContainer = this.mBuffContainer;
				dirtyBuffContainer = true;
			}
			else if (modType == this.BuffType.DEBUFF)
			{
				indexToPlaceActionIn = this.mNextFreeDebuffRow * this.mStatusEffectsPerRow + this.mNextFreeDebuffColumn;
				actionContainer = this.mDebuffContainer;
				dirtyDebuffContainer = true;
			}
			else if (modType == this.BuffType.WORLD)
			{
				indexToPlaceActionIn = this.mNextFreeWorldBuffColumn;
				actionContainer = this.mWorldBuffContainer;
				dirtyWorldContainer = true;
			}
			else
			{
				continue;
			}

			actionContainer.addAction(mod, false, indexToPlaceActionIn);

			if (mod instanceof this.BuffDebuff)
			{
				local duration = mod.getDuration();
				duration -= 10;
				local abilityId = mod.getAbilityID();

				if (!(abilityId in this.mBlinkingEvents))
				{
					local actionButtonIndex = -1;
					local actionButtons = actionContainer.getAllActionButtons(false);
					local i = 0;

					while (i < actionButtons.len())
					{
						local actionButton = actionButtons[i];

						if (actionButton)
						{
							local action = actionButton.getAction();

							if (action instanceof this.BuffDebuff)
							{
								if (mod.getAbilityID() == action.getAbilityID())
								{
									actionButtonIndex = i;
								}
							}
						}

						i++;
					}

					if (actionButtonIndex != -1)
					{
						local actionButton = actionContainer.getSlotContents(actionButtonIndex);

						if (actionButton)
						{
							actionButton.setVisible(true);
						}
					}

					local blinkEvent = ::_eventScheduler.fireIn(duration, this, "playerBuffBlinkdown", [
						actionContainer,
						mod
					]);
					this.mBlinkingEvents[abilityId] <- {
						event = blinkEvent,
						visible = true
					};
				}
				else
				{
					local actionButtonIndex = -1;
					local actionButtons = actionContainer.getAllActionButtons(false);
					local i = 0;

					while (i < actionButtons.len())
					{
						local actionButton = actionButtons[i];

						if (actionButton)
						{
							local action = actionButton.getAction();

							if (action instanceof this.BuffDebuff)
							{
								if (mod.getAbilityID() == action.getAbilityID())
								{
									actionButtonIndex = i;
								}
							}
						}

						i++;
					}

					if (actionButtonIndex != -1)
					{
						if (duration > 0.0 && duration < 2147483647 / 1000)
						{
							::_eventScheduler.cancel(this.mBlinkingEvents[abilityId].event);
							delete this.mBlinkingEvents[abilityId];
							local actionButton = actionContainer.getSlotContents(actionButtonIndex);

							if (actionButton)
							{
								actionButton.setVisible(true);
							}

							this.mBlinkingEvents[abilityId] <- {
								event = null,
								visible = true
							};
							local blinkEvent = ::_eventScheduler.fireIn(duration, this, "playerBuffBlinkdown", [
								actionContainer,
								mod
							]);
							this.mBlinkingEvents[abilityId].event = blinkEvent;
						}
						else
						{
							local actionButton = actionContainer.getSlotContents(actionButtonIndex);

							if (actionButton)
							{
								actionButton.setVisible(this.mBlinkingEvents[abilityId].visible);
							}

							this.mBlinkingEvents[abilityId].event.messageArg[1] = mod;
						}
					}
				}
			}

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

					this.mNextFreeDebuffColumn = this.mStatusEffectsPerRow - 1;
				}
			}
			else if (modType == this.BuffType.WORLD)
			{
				this.mNextFreeWorldBuffColumn--;
				this.mWorldBuffHolder.setVisible(true);
			}
		}

		if (dirtyBuffContainer || this.mRemovedBuffs)
		{
			this.mBuffContainer.updateContainer();
		}

		if (dirtyDebuffContainer || this.mRemovedDebuffs)
		{
			this.mDebuffContainer.updateContainer();
		}

		if (dirtyWorldContainer || this.mRemovedWorldBuffs)
		{
			this.mWorldBuffContainer.updateContainer();
		}

		this.mRemovedBuffs = false;
		this.mRemovedDebuffs = false;
		this.mRemovedWorldBuffs = false;
		this.updateWorldBuffPosition();
	}

	function updateWorldBuffPosition()
	{
		if (this.mNextFreeWorldBuffColumn != 2)
		{
			local widthOffset = (this.mNextFreeBuffColumn + 1) * 32;
			this.mWorldBuffHolder.setPosition(widthOffset, -3);
			this.mWorldBuffHolder.setVisible(true);
		}
		else
		{
			this.mWorldBuffHolder.setVisible(false);
		}
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		::_root.addListener(this);
		this.setOverlay("GUI/BuffDebuffOverlay");
		this.Screen.setOverlayVisible("GUI/BuffDebuffOverlay", true);
	}

	function _removeNotify()
	{
		this.GUI.Container._removeNotify();
		::_root.removeListener(this);
	}

}

