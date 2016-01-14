this.require("GUI/Button");
this.require("Items/ItemManager");
class this.GUI.ActionButton extends this.GUI.Button
{
	mCountLabel = null;
	mCountLabelText = "";
	mCooldownLabel = null;
	mBackground = null;
	mForeground = null;
	mKeybindingLabel = null;
	mKeybindingLabelText = null;
	mFlash = null;
	mGreyedOut = null;
	mDisabledOverlay = null;
	mHighlightOverlay = null;
	mMouseOverGlow = null;
	mInvisibleLayer = null;
	mTimer = null;
	mFlashInitiated = false;
	mFlashStartTime = 0;
	mFlashHalfTime = 0;
	mFlashEndTime = 0;
	mAction = null;
	mActionButtonSlot = null;
	mPrevActionContainer = null;
	mFlashDuration = 0.5 * this.gMilisecPerSecond;
	mButtonWidth = 32;
	mButtonHeight = 32;
	mStackCount = null;
	mButtonState = 0;
	mRewardCheckBox = null;
	mInsufficientResources = false;
	mInRange = true;
	mValidHostility = true;
	mInsufficentReagents = false;
	mGroundTargetActivated = false;
	mAbilityUpdated = false;
	mHasRequiredEquipment = true;
	mHasRequiredStatus = true;
	mHasRequiredTargetCriteria = false;
	mAnotherLayer = null;
	mExtraComponent = null;
	static CHECK_BOX_SIZE = 18;
	static DEFAULT_KEY_LABEL_COLOR = "white";
	static OUT_OF_RANGE_KEY_LABEL_COLOR = "red";
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor("", vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor("");
		}

		this.mUseMouseOverEffect = false;
		this.setUseMouseOverEffect(false);
		this.mBackground = ::GUI.Image();
		this.mBackground.setLayoutManager(null);
		this.mBackground.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mBackground.setVisible(true);
		this.mForeground = ::GUI.Image();
		this.mForeground.setLayoutManager(null);
		this.mForeground.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mForeground.setVisible(true);
		this.mRewardCheckBox = this.GUI.CheckBox(null);
		this.mRewardCheckBox.setLayoutManager(null);
		this.mRewardCheckBox.setSize(this.CHECK_BOX_SIZE, this.CHECK_BOX_SIZE);
		this.mRewardCheckBox.setPreferredSize(this.CHECK_BOX_SIZE, this.CHECK_BOX_SIZE);
		this.mRewardCheckBox.setPosition(0, this.mForeground.getSize().height - this.CHECK_BOX_SIZE);
		this.mRewardCheckBox.setVisible(false);
		this.mForeground.add(this.mRewardCheckBox);
		this.mInvisibleLayer = ::GUI.Container(null);
		this.mInvisibleLayer.setAppearance("ActionButtonOverlay");
		this.mInvisibleLayer.setPosition(0, 0);
		this.mInvisibleLayer.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mInvisibleLayer.setVisible(true);
		this.mInvisibleLayer.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.0));
		this.mBackground.add(this.mInvisibleLayer);
		this.mInvisibleLayer.add(this.mForeground);
		this.mFlash = ::GUI.Container(null);
		this.mFlash.setPosition(0, 0);
		this.mFlash.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mFlash.setAppearance("ActionButtonOverlay");
		this.mFlash.setVisible(false);
		this.mGreyedOut = ::GUI.Container(null);
		this.mGreyedOut.setPosition(0, 0);
		this.mGreyedOut.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mGreyedOut.setAppearance("ActionButtonOverlay");
		this.mGreyedOut.setVisible(false);
		this.mGreyedOut.setBlendColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
		this.mGreyedOut.setHeight(this.mButtonHeight);
		this.mGreyedOut.setPosition(0.0, 0.0);
		this.mDisabledOverlay = ::GUI.Container(null);
		this.mDisabledOverlay.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mDisabledOverlay.setAppearance("ActionButtonOverlay");
		this.showDisabledOverlay(false);
		this.mDisabledOverlay.setBlendColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
		this.mDisabledOverlay.setHeight(this.mButtonHeight);
		this.mDisabledOverlay.setPosition(0.0, 0.0);
		this.mHighlightOverlay = this.GUI.Container(null);
		this.mHighlightOverlay.setPosition(0, 0);
		this.mHighlightOverlay.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mHighlightOverlay.setVisible(false);
		this.mHighlightOverlay.setPassThru(true);
		this.mHighlightOverlay.setAppearance("ActionButtonOverlay");
		this.mHighlightOverlay.setBlendColor(this.Color(0.0, 1.0, 0.0, 0.40000001));
		this.mMouseOverGlow = this.GUI.Container(null);
		this.mMouseOverGlow.setPosition(0, 0);
		this.mMouseOverGlow.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mMouseOverGlow.setVisible(false);
		this.mMouseOverGlow.setPassThru(true);
		this.mMouseOverGlow.setAppearance("ActionButtonOverlay");
		this.mMouseOverGlow.setBlendColor(this.Color(1.0, 1.0, 0.0, 0.40000001));
		this.mAnotherLayer = ::GUI.Container(null);
		this.mAnotherLayer.setPosition(0, 0);
		this.mAnotherLayer.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mAnotherLayer.setVisible(true);
		this.mAnotherLayer.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.0));
		this.mAnotherLayer.add(this.mFlash);
		this.mAnotherLayer.add(this.mGreyedOut);
		this.mAnotherLayer.add(this.mHighlightOverlay);
		this.mAnotherLayer.add(this.mMouseOverGlow);
		this.mAnotherLayer.add(this.mDisabledOverlay);
		this.mForeground.add(this.mAnotherLayer);
		this.add(this.mBackground);
		this.setAppearance("ActionButton");
		this.setInsets(0);
		this.setLayoutManager(null);
		this.setSelection(true);
		this.mTimer = ::Timer();
		local font = this.GUI.Font("MaiandraOutline", 14);
		local cooldownFont = this.GUI.Font("MaiandraOutline", 18);
		this.mCountLabel = this.GUI.Label("");
		this.mCountLabel.setFont(font);
		this.mForeground.add(this.mCountLabel);
		this.mKeybindingLabel = this.GUI.Label("");
		this.mKeybindingLabel.setFont(font);
		this.mForeground.add(this.mKeybindingLabel);
		this.mCooldownLabel = this.GUI.Label("");
		this.mCooldownLabel.setFont(cooldownFont);
		this.mForeground.add(this.mCooldownLabel);
	}

	function activate()
	{
		local sound = this.mAction.getSound();

		if (sound)
		{
			::Audio.playSound(sound);
		}

		if (!this.mAction.sendActivationRequest())
		{
			return;
		}
		else
		{
			this._enterFrameRelay.addListener(this);
		}
	}

	function addExtraComponent( component )
	{
		if (this.mExtraComponent == null)
		{
			this.mExtraComponent = component;
			this.mAnotherLayer.add(this.mExtraComponent);
		}
	}

	function bindActionToButton( action )
	{
		this.mAction = action;
		this.onStacksUpdated(this, this.mAction, this.mAction.getNumStacks());

		if (this.mAction)
		{
			this.mForeground.setImageName(this.mAction.getForegroundImage());
			this.mBackground.setImageName(this.mAction.getBackgroundImage());
			this.handleSpecialIconBehavior();
		}

		if (action && (action instanceof this.Ability))
		{
			this.mHasRequiredEquipment = action.hasRequiredEquipment();
			this._showGrayedOutOverlay();
		}
	}

	function checkTargetCriteria( target )
	{
		local targetCriteria = this.mAction.getTargetCriteria();

		if (target == null && targetCriteria != "")
		{
			this.mHasRequiredTargetCriteria = false;
			return false;
		}

		this.mHasRequiredTargetCriteria = true;

		if (targetCriteria.find("Alive"))
		{
			this.mHasRequiredTargetCriteria = !target.hasStatusEffect(this.StatusEffects.DEAD);
		}
		else if (targetCriteria.find("Dead"))
		{
			this.mHasRequiredTargetCriteria = target.hasStatusEffect(this.StatusEffects.DEAD);
		}
	}

	function clearCooldownVisuals()
	{
		this.mFlashInitiated = false;
		this.mFlash.setVisible(false);
		this.mGreyedOut.setBlendColor(this.Color(0.30000001, 0.30000001, 0.30000001, 0.0));
		this.mGreyedOut.setVisible(false);
		this.mCooldownLabel.setText("");
		this.mCooldownLabel.setVisible(false);
		this._enterFrameRelay.removeListener(this);
	}

	function destroy()
	{
		::GUI.Button.destroy();
	}

	function flashReady( endTime )
	{
		local time = this.mTimer.getMilliseconds();

		if (this.mFlashEndTime <= time)
		{
			this.mFlashStartTime = time;
			this.mFlashHalfTime = time + endTime / 2;
			this.mFlashEndTime = time + endTime;
		}
	}

	function fillOutFromProto( proto )
	{
		if (proto == "")
		{
			return;
		}

		local itemProtoAction = this.ItemProtoAction(proto);
		this.bindActionToButton(itemProtoAction);
	}

	function forceRedHighlight( value )
	{
		this.mAnotherLayer.setAppearance("ActionButtonOverlay");

		if (value)
		{
			this.mAnotherLayer.setBlendColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
		}
		else
		{
			this.mAnotherLayer.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.0));
		}
	}

	function getAction()
	{
		return this.mAction;
	}

	function getActionButtonSlot()
	{
		return this.mActionButtonSlot;
	}

	function getImageName()
	{
		return this.mForeground.getImageName() + "|" + this.mBackground.getImageName();
	}

	function getEquipmentType()
	{
		return this.mEquipmentType;
	}

	function getExtraComponent()
	{
		return this.mExtraComponent;
	}

	function getTimer()
	{
		return this.mTimer;
	}

	function getPreviousActionContainer()
	{
		return this.mPrevActionContainer;
	}

	function handleLeftButtonClick( evt )
	{
		local currentSlot = this.getActionButtonSlot();

		if (currentSlot && currentSlot.getActionContainer())
		{
			local actionContainer = currentSlot.getActionContainer();
			local acOwner = actionContainer.getOwner();

			if (acOwner && "onLeftButtonClicked" in acOwner)
			{
				if (!acOwner.onLeftButtonClicked(this, evt))
				{
					return;
				}
			}
		}
	}

	function handleLeftButtonReleased( evt )
	{
		if (this.mActionButtonSlot)
		{
			this.mActionButtonSlot.setSelected(true);
		}

		if (evt.isShiftDown() || evt.isAltDown() || evt.isControlDown())
		{
			this.mAction.modifiedAction(this, evt.isShiftDown(), evt.isAltDown(), evt.isControlDown());
			return;
		}

		if ("onActionButtonMouseReleased" in this.mActionButtonSlot.getActionContainer().getOwner())
		{
			this.mActionButtonSlot.getActionContainer().getOwner().onActionButtonMouseReleased(this.mAction, this.mRewardCheckBox);
			return;
		}

		local currentSlot = this.getActionButtonSlot();

		if (currentSlot && currentSlot.getActionContainer())
		{
			local actionContainer = currentSlot.getActionContainer();
			local acOwner = actionContainer.getOwner();

			if (acOwner && "onLeftButtonReleased" in acOwner)
			{
				if (!acOwner.onLeftButtonReleased(this, evt))
				{
					return;
				}
			}
		}

		if (!this.mActionButtonSlot || !this.mActionButtonSlot.isUsable(evt))
		{
			return;
		}

		this.activate();
	}

	function handleRightButtonClick( evt )
	{
		local currentSlot = this.getActionButtonSlot();

		if (currentSlot && currentSlot.getActionContainer())
		{
			local actionContainer = currentSlot.getActionContainer();
			local acOwner = actionContainer.getOwner();

			if (acOwner && "onRightButtonClicked" in acOwner)
			{
				if (!acOwner.onRightButtonClicked(this, evt))
				{
					return;
				}
			}
		}
	}

	function handleRightButtonReleased( evt )
	{
		if (this.mAction && this.Key.isDown(this.Key.VK_SHIFT) && evt.isControlDown() && this.Util.hasPermission("dev"))
		{
			this.mAction.showExtraDataScreen();
			return;
		}

		local currentSlot = this.getActionButtonSlot();

		if (currentSlot && currentSlot.getActionContainer())
		{
			local actionContainer = currentSlot.getActionContainer();
			local acOwner = actionContainer.getOwner();

			if (acOwner && "onRightButtonReleased" in acOwner)
			{
				if (!acOwner.onRightButtonReleased(this, evt))
				{
					return;
				}
			}
		}

		local menu = this.GUI.PopupMenu();
		menu.addActionListener(this);
		local showMenu = false;

		if (this.mActionButtonSlot && this.mActionButtonSlot.isUsable(evt))
		{
			menu.addMenuOption("Use", "Use", "onUse");
			showMenu = true;
		}

		if (this.mAction.getNumStacks() > 1)
		{
			menu.addMenuOption("Split", "Split Stack", "onSplit");
			showMenu = true;
		}

		if (showMenu)
		{
			menu.showMenu();
		}
	}

	function handleSpecialIconBehavior()
	{
		if (this.mAction instanceof this.Ability)
		{
			if (this.mAction.getIsValid() && !this.mAbilityUpdated)
			{
				if (this.mAction.getRange() > -1)
				{
					local target;
					::_Connection.addListener(this);

					if (::_avatar)
					{
						target = ::_avatar.getTargetObject();
					}

					this.onTargetPositionUpdated(target);
				}
				else if (this.mAction.getWill() > 0 || this.mAction.getMight() > 0)
				{
					::_Connection.addListener(this);
				}
				else if (this.mAction.getStatusRequirements().len() > 0)
				{
					::_Connection.addListener(this);
				}

				if (this.mAction.getActions().find("GTAE") != null)
				{
					this.mAction.addActionListener(this);
				}

				if (this.mAction.isReagentNeeded())
				{
					::_ItemDataManager.addListener(this);
					this.updateReagentsNeeded();
				}

				if (this.mAction.getEquipRequirements().len() > 0)
				{
					local equipScreen = this.Screens.get("Equipment", true);

					if (equipScreen)
					{
						equipScreen.addListener(this);
					}
				}

				this.mAbilityUpdated = true;
			}
		}
		else
		{
			::_Connection.removeListener(this);

			if (::_avatar)
			{
				::_avatar.removeListener(this);
				local equipScreen = this.Screens.get("Equipment", false);

				if (equipScreen)
				{
					equipScreen.removeListener(this);
				}
			}

			if (this.mAction)
			{
				this.mAction.removeActionListener(this);
				::_ItemDataManager.removeListener(this);
			}
		}
	}

	function onAbilitiesReceived( abilities )
	{
		this.handleSpecialIconBehavior();
	}

	function onAbilityUpdate( abilities )
	{
		foreach( ability in abilities )
		{
			if (this.mAction == ability)
			{
				local foregroundImage = ability.mForegroundImage;
				local backgroundImage = ability.mBackgroundImage;

				if (backgroundImage != "" && foregroundImage != this.mForeground.getImageName() || foregroundImage != "" && backgroundImage != this.mBackground.getImageName())
				{
					this.mAction.setImage(foregroundImage + "|" + backgroundImage);
					this.mForeground.setImageName(this.mAction.getForegroundImage());
					this.mBackground.setImageName(this.mAction.getBackgroundImage());
				}

				return;
			}
		}
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (containerName == "inv" && ::_avatar && ::_avatar.getID() == creatureId)
		{
			this.updateReagentsNeeded();
		}
	}

	function onDragComplete( evt, newParent )
	{
		if (this.mAction && this.mPrevActionContainer)
		{
			this.mPrevActionContainer.onDropActionButton(this);
		}

		if (!newParent)
		{
			if (this.mActionButtonSlot)
			{
				this.mActionButtonSlot.actionButtonDroppedNoParent();
			}
		}
	}

	function onDragRequested( evt )
	{
		if (this.mAction && this.mActionButtonSlot && this.mActionButtonSlot.isActionDraggable())
		{
			evt.acceptDrag(this, this.GUI.DnDEvent.ACTION_MOVE, this.mForeground, this.mBackground, this);
			this.mActionButtonSlot.getActionContainer().onDragActionButton(this);
		}
	}

	function onEnterFrame()
	{
		if (!this.mAction)
		{
			return;
		}

		if (this.mAction.isWarmingUp())
		{
			this.mGreyedOut.setVisible(true);
			this.mGreyedOut.setBlendColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
			this.mGreyedOut.setHeight(this.mButtonHeight);
			this.mGreyedOut.setPosition(0, 0);
		}
		else if (this.mAction.isAwaitingServerResponse())
		{
		}
		else if (this.mAction.isUsable() && !this.mAction.isAvailableForUse())
		{
			local timeUntilReady = this.mAction.getTimeUntilAvailable();
			local timeUsed = this.mAction.getTimeUsed();

			if (timeUntilReady <= 0)
			{
				timeUntilReady = ::_AbilityManager.getTimeUntilCategoryUseable("Global");
				timeUsed = ::_AbilityManager.getCategoryUseTime("Global");
			}

			local curTime = ::_AbilityManager.getTimerMilliseconds();
			local timeEnd = curTime + timeUntilReady - this.mFlashDuration;
			local percentage = (timeEnd - curTime).tofloat() / (timeEnd - timeUsed).tofloat();
			local height = this.mButtonHeight * percentage - this.mFlashDuration / this.gMilisecPerSecond;
			this.mGreyedOut.setBlendColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
			this.mGreyedOut.setVisible(true);
			this.mGreyedOut.setHeight(height);
			local pos = this.mGreyedOut.getPosition();
			this.mGreyedOut.setPosition(pos.x, this.mButtonHeight - height.tointeger());

			if (timeUntilReady <= this.mFlashDuration && timeUntilReady >= 0.0)
			{
				if (this.mFlashInitiated == false)
				{
					this.flashReady(timeUntilReady);
					this.mFlashInitiated = true;
				}

				this._updateFlash(this.mTimer.getMilliseconds());
			}

			if (timeUntilReady > 0)
			{
				if (timeUntilReady <= this.gMilisecPerMinute)
				{
					if (timeUntilReady > 10 * this.gMilisecPerSecond)
					{
						this.mCooldownLabel.setText((timeUntilReady.tofloat() / this.gMilisecPerSecond.tofloat() + 0.99000001).tointeger() + "s");
					}
					else
					{
						this.mCooldownLabel.setText((timeUntilReady.tofloat() / this.gMilisecPerSecond.tofloat() + 0.99000001).tointeger().tostring());
					}
				}
				else if (timeUntilReady <= this.gMilisecPerHour)
				{
					this.mCooldownLabel.setText((timeUntilReady.tofloat() / this.gMilisecPerMinute.tofloat() + 0.99000001).tointeger() + "m");
				}
				else if (timeUntilReady <= this.gMilisecPerDay)
				{
					this.mCooldownLabel.setText((timeUntilReady.tofloat() / this.gMilisecPerHour.tofloat() + 0.99000001).tointeger() + "h");
				}
				else
				{
					this.mCooldownLabel.setText((timeUntilReady.tofloat() / this.gMilisecPerHour.tofloat() + 0.99000001).tointeger() + "d");
				}

				local size = this.mCooldownLabel.getPreferredSize();
				local Width = this.mButtonWidth / 2.0 - size.width / 2.0;
				local Height = this.mButtonHeight / 2.0 - size.height / 2.0;
				this.mCooldownLabel.setPosition(Width, Height);
				this.mCooldownLabel.setVisible(true);
			}
			else
			{
				this.mCooldownLabel.setVisible(false);
				this.mGreyedOut.setBlendColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
				this.mGreyedOut.setVisible(true);
				this.mGreyedOut.setHeight(this.mButtonHeight);
				this.mGreyedOut.setPosition(0, 0);
			}
		}
		else
		{
			this.clearCooldownVisuals();
		}
	}

	function onItemDefUpdated( itemDefId, action )
	{
		if (("mItemDefId" in this.mAction) && this.mAction.mItemDefId == itemDefId)
		{
			if (this.mActionButtonSlot && "refreshInfoPanel" in this.mActionButtonSlot)
			{
				this.mActionButtonSlot.refreshInfoPanel();
			}
		}

		local ours = ("mItemDefData" in this.mAction) && ("mID" in this.mAction.mItemDefData) && this.mAction.mItemDefData.mID == itemDefId;

		if (!ours && (this.mAction instanceof this.ItemProtoAction))
		{
			ours = this.mAction.mItemDefId == itemDefId;
		}

		if (ours)
		{
			local image = action.mIcon;

			if (image != "" && image != this.mForeground.getImageName() || image != "" && image != this.mBackground.getImageName())
			{
				this.mAction.setImage(image);
				this.mForeground.setImageName(this.mAction.getForegroundImage());
				this.mBackground.setImageName(this.mAction.getBackgroundImage());
			}
		}
	}

	function updateImageIfNeeded()
	{
		if (this.mAction)
		{
			local foregroundImage = this.mAction.getForegroundImage();
			local backgroundImage = this.mAction.getBackgroundImage();

			if (this.mForeground && this.mForeground.getImageName() != foregroundImage || this.mBackground && this.mBackground.getImageName() != backgroundImage)
			{
				this.mForeground.setImageName(foregroundImage);
				this.mBackground.setImageName(backgroundImage);
			}
		}
	}

	function getActionTooltip( actionButton, mods )
	{
		local tooltip;

		if (!actionButton)
		{
			return tooltip;
		}

		local action = actionButton.getAction();

		if (action)
		{
			local actionButtonSlot = actionButton.getActionButtonSlot();
			local actionContainer = actionButtonSlot.getActionContainer();

			if (actionButtonSlot && actionContainer)
			{
				local acMods = actionContainer.getTooltipRenderingModifiers(action.getType());

				if (acMods)
				{
					foreach( k, v in acMods )
					{
						mods[k] <- v;
					}
				}

				if (actionContainer.mActionContainerOwner && actionContainer.mActionContainerOwner.mClassName == "QuickBar")
				{
					this.setMouseOverGlow(true);
				}
			}

			local timeRemaining = -1;

			if ("mItemData" in action)
			{
				timeRemaining = action.mItemData.getTimeRemaining();
			}

			if (!actionContainer.getShowBindingInfo())
			{
				mods.showBindingInfo <- false;
			}

			if (action && (action instanceof this.ItemAction))
			{
				mods.item <- action;
			}

			if (timeRemaining != -1)
			{
				local timeText;

				if (timeRemaining > 0)
				{
					timeText = this.Util.parseSecToTimeStr(timeRemaining);
					timeText = "Expires in " + timeText;
				}
				else
				{
					timeText = "Expired";
				}

				local label = this.GUI.Label(timeText);
				label.setData("EXPIRE_LABEL");
				
				tooltip = action.getTooltip(mods, true, label);

				if (tooltip)
				{
					::Util.setCertainComponentVisible(tooltip, "LIFETIME_LABEL", false);
					::Util.setCertainComponentVisible(tooltip, "EXPIRE_LABEL", true);
				}
			}
			else
			{
				tooltip = action.getTooltip(mods);
			}
		}

		return tooltip;
	}

	function onMouseEnter( evt )
	{
		local mods = {};
		this.clearChildtooltips();
		local tooltip = this.getActionTooltip(this, mods);

		if (tooltip)
		{
			this.setTooltip(tooltip);
			local actionContainer = this.getActionButtonSlot().getActionContainer();
			local action = this.mAction;
			local equipType;

			if (this.mAction instanceof this.ItemProtoAction)
			{
				action = this.mAction._getItemDef();
				equipType = action.mEquipType;
			}
			else if (this.mAction instanceof this.ItemAction)
			{
				if (action.mItemDefData)
				{
					equipType = action.mItemDefData.mEquipType;
				}
			}
			else
			{
				return;
			}

			local showEqComparisons = ::Pref.get("gameplay.eqcomparisons") || this.Key.isDown(this.Key.VK_SHIFT);

			if (showEqComparisons == true && action.isValid() && actionContainer.getShowEquipmentComparison())
			{
				local equipment = this.Screens.get("Equipment", true);

				if (equipment)
				{
					local eqContainers = this.deepClone(this.EquipmentMapContainer[equipType]);
					local foundOffHand = false;
					local foundMainHand = false;

					foreach( k, v in eqContainers )
					{
						if (v == this.ItemEquipSlot.WEAPON_OFF_HAND)
						{
							foundOffHand = true;
						}
						else if (v == this.ItemEquipSlot.WEAPON_MAIN_HAND)
						{
							foundMainHand = true;
						}
					}

					if (foundOffHand && !foundMainHand)
					{
						eqContainers.append(this.ItemEquipSlot.WEAPON_MAIN_HAND);
					}

					if (foundMainHand && !foundOffHand)
					{
						eqContainers.append(this.ItemEquipSlot.WEAPON_OFF_HAND);
					}

					local previousContainer;

					foreach( eqContainer in eqContainers )
					{
						local actionContainer = equipment.findMatchingContainer(eqContainer);

						if (actionContainer && previousContainer != actionContainer)
						{
							foreach( key, actionButtonSlot in actionContainer.getAllActionButtonSlots() )
							{
								if (actionButtonSlot)
								{
									local equippedActionButton = actionButtonSlot.getActionButton();

									if (equippedActionButton)
									{
										local equippedAction = equippedActionButton.getAction();

										if (equippedAction && (equippedAction instanceof this.ItemAction))
										{
											mods.item <- equippedAction;
											mods.CurrentlyEquipped <- true;
											this.addChildtootip(this.getActionTooltip(equippedActionButton, mods));
										}
									}
								}
							}

							previousContainer = actionContainer;
						}
					}
				}
			}
		}
	}

	function onMouseExit( evt )
	{
		if (this.mAction && this.mActionButtonSlot && this.mActionButtonSlot.mActionContainer && this.mActionButtonSlot.mActionContainer.mActionContainerOwner && this.mActionButtonSlot.mActionContainer.mActionContainerOwner.mClassName == "QuickBar" && !this.mGroundTargetActivated)
		{
			this.setMouseOverGlow(false);
		}
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.handleLeftButtonClick(evt);
			evt.consume();
		}
		else if (evt.button == this.MouseEvent.RBUTTON)
		{
			this.handleRightButtonClick(evt);
		}
	}

	function onMouseReleased( evt )
	{
		if (!this.mAction)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.handleLeftButtonReleased(evt);
			evt.consume();
		}
		else if (evt.button == this.MouseEvent.RBUTTON)
		{
			this.handleRightButtonReleased(evt);
		}
	}

	function onStacksUpdated( sender, itemAction, numUses )
	{
		if (this.mAction == itemAction)
		{
			this.mStackCount = numUses;
			this.refreshStackCount();
		}
		else
		{
			this.updateReagentsNeeded();
		}
	}

	function onSplit( menu )
	{
		local numEntry = this.GUI.InputArea();
		numEntry.setAllowOnlyNumbers(true);
		numEntry.setWidth(100);
		numEntry.setText("1");
		local box = this.GUI.Container(this.GUI.BoxLayout());
		box.add(this.GUI.HTML("<font size=\"24\">Number to split:  </font>"));
		box.getLayoutManager().setAlignment(0.5);
		box.add(numEntry);
		local callback = {
			actionButton = this,
			numberEntry = numEntry,
			action = this.mAction,
			function onActionSelected( mb, alt )
			{
				if (alt != "Ok")
				{
					return;
				}

				if (this.numberEntry.getValue() < 1 || this.numberEntry.getValue() > this.action.getNumStacks() - 1)
				{
					this.IGIS.error("Invalid stack size.");
					return;
				}

				this.actionButton.mAction.split(this.numberEntry.getValue());
			}

		};
		this.GUI.MessageBox.showOkCancel(box, callback);
	}

	function onUse( menu )
	{
		this.activate();
	}

	function onGroundTargetActivate()
	{
		this.mGroundTargetActivated = true;
		this.setMouseOverGlow(true);
	}

	function onGroundTargetDone()
	{
		this.mGroundTargetActivated = false;
		this.setMouseOverGlow(false);
	}

	function onCreatureUpdated( component, creature )
	{
		if (!::_avatar || !this.mAction || !(this.mAction instanceof this.Ability) || ::_avatar.getID() != creature.getID())
		{
			return;
		}

		local avatarMight = ::_avatar.getStat(this.Stat.MIGHT, true);
		local avatarWill = ::_avatar.getStat(this.Stat.WILL, true);
		local avatarMightCharge = ::_avatar.getStat(this.Stat.MIGHT_CHARGES, true);
		local avatarWillCharge = ::_avatar.getStat(this.Stat.WILL_CHARGES, true);
		local actionMight = this.mAction.getMight();
		local actionWill = this.mAction.getWill();
		local actionMightMinCharge = this.mAction.getMightMinCharge();
		local actionWillMinCharge = this.mAction.getWillMinCharge();

		if (avatarMight < actionMight || avatarWill < actionWill || avatarMightCharge < actionMightMinCharge || avatarWillCharge < actionWillMinCharge)
		{
			this.mInsufficientResources = true;
		}
		else
		{
			this.mInsufficientResources = false;
		}

		this._showGrayedOutOverlay();
	}

	function onEquipContainerUpdated( component )
	{
		if (!::_avatar || !this.mAction || !(this.mAction instanceof this.Ability))
		{
			return;
		}

		this.mHasRequiredEquipment = this.mAction.hasRequiredEquipment();
		this._showGrayedOutOverlay();
	}

	function onStatusUpdate( creature )
	{
		if (!::_avatar || !this.mAction || !(this.mAction instanceof this.Ability) || ::_avatar.getID() != creature.getID())
		{
			return;
		}

		this.mHasRequiredStatus = this.mAction.hasStatusRequirements();
		this._showGrayedOutOverlay();
	}

	function onTargetObjectChanged( component, target )
	{
		if (!target)
		{
			this.setDisabledOverlayColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
			this.showDisabledOverlay(false);
			return;
		}

		if (this.mAction && (this.mAction instanceof this.Ability))
		{
			this.mValidHostility = false;
			local spellHostility = this.mAction.getHostility();

			switch(spellHostility)
			{
			case -1:
				if (target.isPlayer() && !target.hasStatusEffect(this.StatusEffects.PVPABLE))
				{
					this.mValidHostility = true;
				}

				break;

			case 0:
				if (target.isPlayer() && !target.hasStatusEffect(this.StatusEffects.PVPABLE))
				{
					this.mValidHostility = true;
				}

				break;

			case 1:
				if (target.isPlayer())
				{
					this.mValidHostility = target.hasStatusEffect(this.StatusEffects.PVPABLE);
				}
				else if (target.hasStatusEffect(this.StatusEffects.UNATTACKABLE) || target.hasStatusEffect(this.StatusEffects.INVINCIBLE))
				{
					this.mValidHostility = false;
				}
				else
				{
					this.mValidHostility = true;
				}

				break;
			}
		}

		this.onTargetPositionUpdated(target);
	}

	function onTargetPositionUpdated( target )
	{
		local isDisabled = true;

		if (this.mAction && target && ::_avatar)
		{
			local maxDistance = this.mAction.getRange() + this.Util.getRangeOffset(::_avatar, target);
			local targetPosition = target.getPosition();
			local avatarPosition = ::_avatar.getPosition();

			if (::Math.DetermineDistanceBetweenTwoPoints(targetPosition, avatarPosition) < maxDistance)
			{
				this.mInRange = true;
				this.mKeybindingLabel.setFontColor(this.Colors[this.DEFAULT_KEY_LABEL_COLOR]);
				isDisabled = false;
			}
		}

		if (this.mAction && this.mAction.getRange() == -1)
		{
			this.mInRange = true;
			this.mKeybindingLabel.setFontColor(this.Colors[this.DEFAULT_KEY_LABEL_COLOR]);
			isDisabled = false;
		}

		if (isDisabled)
		{
			this.mInRange = false;
			this.mKeybindingLabel.setFontColor(this.Colors[this.OUT_OF_RANGE_KEY_LABEL_COLOR]);
		}

		this._showGrayedOutOverlay();
	}

	function refreshStackCount()
	{
		local owner;

		if (this.mActionButtonSlot)
		{
			owner = this.mActionButtonSlot.getActionContainer();
		}

		if (owner && owner.getContainerName().find("quickbar") != null && ("mItemDefData" in this.mAction) && this.mAction.mItemDefData && this.mAction.mItemDefData.mType == this.ItemType.CONSUMABLE)
		{
			this.setCountText(this.mStackCount.tostring());
			this._showGrayedOutOverlay();
		}
		else if (this.mStackCount && this.mStackCount != 1)
		{
			this.setCountText(this.mStackCount.tostring());
		}
		else
		{
			this.setCountText("");
		}
	}

	function removeExtraComponent()
	{
		if (this.mExtraComponent)
		{
			this.mAnotherLayer.remove(this.mExtraComponent);
			this.mExtraComponent = null;
		}
	}

	function setActionButtonSlot( actionButtonSlot )
	{
		this.mActionButtonSlot = actionButtonSlot;
	}

	function setCountText( text )
	{
		this.mCountLabelText = text;
		this.mCountLabel.setText(this.mCountLabelText);
		local size = this.mCountLabel.getPreferredSize();
		local Width = this.mButtonWidth - size.width;
		local Height = this.mButtonHeight - size.height;
		this.mCountLabel.setPosition(Width, Height);
		local itemType = this.mAction.getUseType();

		if (itemType == this.ItemType.CONSUMABLE)
		{
			this._setCountColor("ffffff");
		}
		else
		{
			this._setCountColor("ffff00");
		}
	}

	function setDisabledOverlayAppearance( appearance )
	{
		this.mDisabledOverlay.setAppearance(appearance);
	}

	function setDisabledOverlayColor( color )
	{
		this.mDisabledOverlay.setBlendColor(color);
	}

	function setHighlightOverlay( visible )
	{
		this.mHighlightOverlay.setVisible(visible);
	}

	function setMouseOverGlow( visible )
	{
		this.mMouseOverGlow.setVisible(visible);
	}

	function setPreviousActionContainer( container )
	{
		this.mPrevActionContainer = container;
	}

	function setStackCount( stacks )
	{
		this.mStackCount = stacks;
		this.refreshStackCount();
	}

	function setActionButtonSize( width, height )
	{
		this.mButtonWidth = width;
		this.mButtonHeight = height;
		this.GUI.Button.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mBackground.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mForeground.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mInvisibleLayer.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mFlash.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mGreyedOut.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mDisabledOverlay.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mHighlightOverlay.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mMouseOverGlow.setSize(this.mButtonWidth, this.mButtonHeight);
		this.mAnotherLayer.setSize(this.mButtonWidth, this.mButtonHeight);
	}

	function setImageName( imageName )
	{
		if (imageName == null)
		{
			return;
		}

		this.mAction.setImage(imageName);
		this.mForeground.setImageName(this.mAction.getForegroundImage());
		this.mBackground.setImageName(this.mAction.getBackgroundImage());
	}

	function setRewardCheckBoxVisible( visible )
	{
		this.mRewardCheckBox.setVisible(visible);
	}

	function setChecked( checked )
	{
		if (this.mRewardCheckBox.isVisible())
		{
			this.mRewardCheckBox.setChecked(checked);
		}
	}

	function isChecked()
	{
		if (this.mRewardCheckBox.isVisible())
		{
			return this.mRewardCheckBox.getChecked();
		}

		return false;
	}

	function showDisabledOverlay( value )
	{
		this.mDisabledOverlay.setVisible(value);
	}

	function updateReagentsNeeded()
	{
		if (!this.mAction || !(this.mAction instanceof this.Ability) || !this.mAction.isReagentNeeded())
		{
			return;
		}

		this.mInsufficentReagents = false;
		local reagents = this.mAction.getReagents();

		foreach( itemDefId, count in reagents )
		{
			local numStacks = ::_ItemDataManager.getNumItems(itemDefId);

			if (numStacks < count)
			{
				this.mInsufficentReagents = true;
				break;
			}
		}

		local generator = ::_ItemDataManager.getNumItems(900004) + ::_ItemDataManager.getNumItems(900006) + ::_ItemDataManager.getNumItems(900007) + ::_ItemDataManager.getNumItems(900008);

		if (generator > 0)
		{
			this.mInsufficentReagents = false;
		}

		this._showGrayedOutOverlay();
	}

	function _addNotify()
	{
		this.GUI.Button._addNotify();

		if (::_avatar)
		{
			::_avatar.addListener(this);
		}

		::_ItemManager.addListener(this);
		::_AbilityManager.addAbilityListener(this);
		::_ItemDataManager.addListener(this);
		::_Connection.addListener(this);
	}

	function _removeNotify()
	{
		::_enterFrameRelay.removeListener(this);
		::_AbilityManager.addAbilityListener(this);
		::_ItemDataManager.removeListener(this);
		::_ItemManager.removeListener(this);
		::_Connection.removeListener(this);

		if (::_avatar)
		{
			::_avatar.removeListener(this);
		}

		this.GUI.Button._removeNotify();
	}

	function _setCountColor( color )
	{
		this.mCountLabel.setFontColor(color);
	}

	function _setKeybindingText( keybindingText )
	{
		this.mKeybindingLabelText = keybindingText;

		if (keybindingText)
		{
			this.mKeybindingLabel.setText(this.mKeybindingLabelText);
		}

		local labelSize = this.mKeybindingLabel.getPreferredSize();
		local Width = this.mButtonWidth - labelSize.width;
		this.mKeybindingLabel.setPosition(Width, 0.0);
		this.mKeybindingLabel.setVisible(true);
	}

	function _showGrayedOutOverlay()
	{
		if (!::_avatar)
		{
			return;
		}

		if (!(this.mAction instanceof this.Ability))
		{
			return;
		}

		local targetObject = ::_avatar.getTargetObject();

		if (!targetObject)
		{
			this.setDisabledOverlayColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
			this.showDisabledOverlay(false);
			return;
		}

		local targetDead = targetObject.hasStatusEffect(this.StatusEffects.DEAD);

		if (!targetObject.isPlayer() && targetDead)
		{
			this.setDisabledOverlayColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
			this.showDisabledOverlay(false);
			return;
		}

		this.checkTargetCriteria(targetObject);

		if (this.mInsufficientResources)
		{
			this.setDisabledOverlayColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
			this.showDisabledOverlay(true);
			return;
		}

		if (!this.mAction.doesAbilityRequireTarget())
		{
			this.setDisabledOverlayColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
			this.showDisabledOverlay(false);
			return;
		}

		if (!this.mValidHostility || !this.mHasRequiredTargetCriteria || !this.mInRange || this.mInsufficentReagents || !this.mHasRequiredEquipment || !this.mHasRequiredStatus || this.mStackCount == 0)
		{
			this.setDisabledOverlayColor(this.Color(0.07, 0.13, 0.36000001, 0.60000002));
			this.showDisabledOverlay(true);
		}
		else
		{
			this.setDisabledOverlayColor(this.Color(1.0, 0.0, 0.0, 0.40000001));
			this.showDisabledOverlay(false);
		}
	}

	function _updateFlash( time )
	{
		if (time < this.mFlashEndTime)
		{
			local alpha = 0.0;

			if (time <= this.mFlashHalfTime)
			{
				alpha = 1.0 - (this.mFlashHalfTime - time).tofloat() / (this.mFlashHalfTime - this.mFlashStartTime).tofloat();
			}
			else
			{
				alpha = (this.mFlashEndTime - time).tofloat() / (this.mFlashEndTime - this.mFlashHalfTime).tofloat();
			}

			this.mFlash.setBlendColor(this.Color(1.0, 1.0, 1.0, alpha));
			this.mFlash.setVisible(true);
		}
	}

}

