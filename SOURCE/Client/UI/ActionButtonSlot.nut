class this.GUI.ActionButtonSlot extends this.GUI.Container
{
	mEmptyBackgroundMat = "BG/ActionHolder";
	mSwitchBackgroundOnEmpty = true;
	mBackground = null;
	mHighlightOverlay = null;
	mButtonSlotWidth = 32;
	mButtonSlotHeight = 32;
	mInfoPanelSlotHeight = 43;
	mInfoPanelSlotWidth = 217;
	mMiniInfoPanelSlotHeight = 43;
	mMiniInfoPanelSlotWidth = 140;
	mMiniInfoPanelVersion = false;
	mActionButton = null;
	mPopupMenu = null;
	mKeybindingLabel = null;
	mKeybindingLabelText = null;
	mEquipmentRestriction = this.ItemEquipSlot.NONE;
	mActionContainer = null;
	mShowInfoPanel = false;
	mInfoPanelContents = null;
	mValidDropTarget = true;
	mButtonSlotOffset = 6;
	mInfoPaneOffset = 10;
	mInQuickbar = false;
	mDraggable = true;
	mSwapped = false;
	mForceRedHighlight = false;
	mDisabled = false;
	mUseMode = null;
	mBroadcaster = null;
	USE_NONE = null;
	USE_LEFT_CLICK = "LEFT_CLICK";
	USE_LEFT_DOUBLE_CLICK = "LEFT_DOUBLE_CLICK";
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Container.constructor();
		}
		else
		{
			this.GUI.Container.constructor();
		}

		if (vargc > 1)
		{
			this.mInQuickbar = vargv[1];
		}

		this.mBackground = ::GUI.Component(null);
		this.mBackground.setAppearance("Icon");
		this.setBackgroundMaterial(this.mEmptyBackgroundMat, true);
		this.mBackground.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mBackground.setPosition(0, 0);
		this.mBackground.setVisible(true);
		this.mBackground.setInsets(0, 0, 0, 0);
		this.mBroadcaster = this.MessageBroadcaster();
		this.setAppearance("ActionButton");
		this.setLayoutManager(null);
		this.mHighlightOverlay = this.GUI.Component(null);
		this.mHighlightOverlay.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mHighlightOverlay.setVisible(false);
		this.mHighlightOverlay.setPassThru(true);
		this.mHighlightOverlay.setAppearance("ActionButtonOverlay");
		this.mHighlightOverlay.setBlendColor(this.Color(0.0, 1.0, 0.0, 0.40000001));
		local component1YPos = 2;
		local component2YPos = component1YPos + this.mInfoPanelSlotWidth / 3;
		local component3YPos = component2YPos + this.mInfoPanelSlotWidth / 3;
		this.mKeybindingLabel = this.GUI.Label("");

		if (this.mMiniInfoPanelVersion)
		{
			this.setSize(this.mMiniInfoPanelSlotWidth, this.mMiniInfoPanelSlotHeight);
		}
		else
		{
			this.setSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight);
		}

		this.mBackground.add(this.mHighlightOverlay);
		this.add(this.mBackground);
	}

	function actionButtonDroppedNoParent()
	{
		this.mActionContainer.attemptDisownButton(this);
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function getActionContainer()
	{
		return this.mActionContainer;
	}

	function getActionButton()
	{
		return this.mActionButton;
	}

	function getBackgroundMaterial()
	{
		return this.mBackground.getMaterial();
	}

	function getEquipmentRestriction()
	{
		return this.mEquipmentRestriction;
	}

	function getKeybinding()
	{
		return this.mKeybindingLabelText;
	}

	function getPreferredSize()
	{
		if (this.mShowInfoPanel)
		{
			if (this.mMiniInfoPanelVersion)
			{
				return {
					width = this.mMiniInfoPanelSlotWidth,
					height = this.mMiniInfoPanelSlotHeight
				};
			}
			else
			{
				return {
					width = this.mInfoPanelSlotWidth,
					height = this.mInfoPanelSlotHeight
				};
			}
		}
		else
		{
			return {
				width = this.mButtonSlotWidth,
				height = this.mButtonSlotHeight
			};
		}
	}

	function getPopupMenu()
	{
		return this.mPopupMenu;
	}

	function getSwapped()
	{
		return this.mSwapped;
	}

	function isItemPlacementValid( itemType )
	{
		if (this.mEquipmentRestriction == this.ItemEquipSlot.NONE)
		{
			return true;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.WEAPON_MAIN_HAND)
		{
			return itemType == this.ItemEquipType.WEAPON_1H || itemType == this.ItemEquipType.WEAPON_1H_UNIQUE || itemType == this.ItemEquipType.WEAPON_1H_MAIN || itemType == this.ItemEquipType.WEAPON_2H || itemType == this.ItemEquipType.WEAPON_1H_OFF;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			return itemType == this.ItemEquipType.WEAPON_1H || itemType == this.ItemEquipType.WEAPON_1H_OFF || itemType == this.ItemEquipType.WEAPON_1H_UNIQUE || itemType == this.ItemEquipType.ARMOR_SHIELD;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.WEAPON_RANGED)
		{
			return itemType == this.ItemEquipType.WEAPON_RANGED;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_HEAD)
		{
			return itemType == this.ItemEquipType.ARMOR_HEAD;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_NECK)
		{
			return itemType == this.ItemEquipType.ARMOR_NECK;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_SHOULDER)
		{
			return itemType == this.ItemEquipType.ARMOR_SHOULDER;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_CHEST)
		{
			return itemType == this.ItemEquipType.ARMOR_CHEST;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_ARMS)
		{
			return itemType == this.ItemEquipType.ARMOR_ARMS;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_HANDS)
		{
			return itemType == this.ItemEquipType.ARMOR_HANDS;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_WAIST)
		{
			return itemType == this.ItemEquipType.ARMOR_WAIST;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_LEGS)
		{
			return itemType == this.ItemEquipType.ARMOR_LEGS;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_FEET)
		{
			return itemType == this.ItemEquipType.ARMOR_FEET;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_RING_L)
		{
			return itemType == this.ItemEquipType.ARMOR_RING || itemType == this.ItemEquipType.ARMOR_RING_UNIQUE;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_RING_R)
		{
			return itemType == this.ItemEquipType.ARMOR_RING || itemType == this.ItemEquipType.ARMOR_RING_UNIQUE;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ARMOR_AMULET)
		{
			return itemType == this.ItemEquipType.ARMOR_AMULET;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.FOCUS_FIRE)
		{
			return itemType == this.ItemEquipType.FOCUS_FIRE;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.FOCUS_FROST)
		{
			return itemType == this.ItemEquipType.FOCUS_FROST;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.FOCUS_MYSTIC)
		{
			return itemType == this.ItemEquipType.FOCUS_MYSTIC;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.FOCUS_DEATH)
		{
			return itemType == this.ItemEquipType.FOCUS_DEATH;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.CONTAINER_0 || this.mEquipmentRestriction == this.ItemEquipSlot.CONTAINER_1 || this.mEquipmentRestriction == this.ItemEquipSlot.CONTAINER_2 || this.mEquipmentRestriction == this.ItemEquipSlot.CONTAINER_3)
		{
			return itemType == this.ItemEquipType.CONTAINER;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.COSMETIC_SHOULDER_L || this.mEquipmentRestriction == this.ItemEquipSlot.COSMETIC_HIP_L || this.mEquipmentRestriction == this.ItemEquipSlot.COSMETIC_SHOULDER_R || this.mEquipmentRestriction == this.ItemEquipSlot.COSMETIC_HIP_R)
		{
			return true;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.RED_CHARM)
		{
			return itemType == this.ItemEquipType.RED_CHARM;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.BLUE_CHARM)
		{
			return itemType == this.ItemEquipType.BLUE_CHARM;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.GREEN_CHARM)
		{
			return itemType == this.ItemEquipType.GREEN_CHARM || itemType == this.ItemEquipType.YELLOW_CHARM || itemType == this.ItemEquipType.BLUE_CHARM;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.YELLOW_CHARM)
		{
			return itemType == this.ItemEquipType.YELLOW_CHARM;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.ORANGE_CHARM)
		{
			return itemType == this.ItemEquipType.ORANGE_CHARM || itemType == this.ItemEquipType.YELLOW_CHARM || itemType == this.ItemEquipType.RED_CHARM;
		}
		else if (this.mEquipmentRestriction == this.ItemEquipSlot.PURPLE_CHARM)
		{
			return itemType == this.ItemEquipType.PURPLE_CHARM || itemType == this.ItemEquipType.RED_CHARM || itemType == this.ItemEquipType.BLUE_CHARM;
		}
	}

	function isDisabled()
	{
		return this.mDisabled;
	}

	function isForcedRed()
	{
		return this.mForceRedHighlight;
	}

	function isUsable( evt )
	{
		if (!this.mUseMode)
		{
			return false;
		}

		local button = evt.button;
		local clickCount = evt.clickCount;

		switch(this.mUseMode)
		{
		case "LEFT_CLICK":
			return evt.button == this.MouseEvent.LBUTTON && evt.clickCount == 1;

		case "LEFT_DOUBLE_CLICK":
			return evt.button == this.MouseEvent.LBUTTON && evt.clickCount == 2;
		}

		return false;
	}

	function isActionDraggable()
	{
		return this.mDraggable;
	}

	function onDragEnter( evt )
	{
	}

	function onDragExit( evt )
	{
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

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			if (this.mActionButton)
			{
				this.setSelected(true);
			}
		}
	}

	function onDrop( evt )
	{
		if (!this.mValidDropTarget)
		{
			return;
		}

		evt.consume();
		local actionButton = this._getDroppedComponent(evt);
		local action = actionButton.getAction();

		if ("getActionButtonValid" in this.mActionContainer)
		{
			if (this.mActionContainer.getActionButtonValid(this, actionButton) == false)
			{
				return;
			}
		}

		if (!this.isItemPlacementValid(action.getEquipmentType()))
		{
			return;
		}

		local oldSlot = actionButton.getActionButtonSlot();

		if (oldSlot != null)
		{
			local callback = {
				actionButtonSlot = this,
				oldSlot = oldSlot,
				actionButton = actionButton,
				function onActionSelected( mb, alt )
				{
					if (alt == "Continue")
					{
						this.actionButtonSlot.finishDrop(this.oldSlot, this.actionButton);
					}
				}

			};
			local showPopup = ::Pref.get("other.BindPopup");
			local actionBound = this.isActionBoundOnPickupEquip(action);
			local oldContainerName;

			if (oldSlot.getActionContainer())
			{
				oldContainerName = oldSlot.getActionContainer().getContainerName();
			}

			local currentContainerName = this.mActionContainer.getContainerName();
			local currentSlotAction;
			local currentActionBound = false;

			if (this.mActionButton)
			{
				currentSlotAction = this.mActionButton.getAction();
				currentActionBound = this.isActionBoundOnPickupEquip(currentSlotAction);
			}

			if (showPopup && (actionBound || currentActionBound) && (currentContainerName == "inventory" || currentContainerName == "bag_inventory" || this.Util.startsWith(currentContainerName, "inv") || this.Util.startsWith(currentContainerName, "eq")))
			{
				local boundText = "This item will be bound to you permanently if you choose to equip it.";

				if (actionBound && action._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_PICKUP)
				{
					boundText = "This item will be bound to you permanently if you choose to pick it up.";
					this.GUI.MessageBox.showEx(boundText, [
						"Continue",
						"Cancel"
					], callback);
				}
				else if (actionBound && action._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_EQUIP && (currentContainerName == "bag_inventory" || this.Util.startsWith(currentContainerName, "eq")) || currentActionBound && currentSlotAction._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_EQUIP && (oldContainerName == "bag_inventory" || this.Util.startsWith(oldContainerName, "eq")))
				{
					this.GUI.MessageBox.showEx(boundText, [
						"Continue",
						"Cancel"
					], callback);
				}
				else
				{
					this.finishDrop(oldSlot, actionButton);
				}
			}
			else
			{
				this.finishDrop(oldSlot, actionButton);
			}
		}
	}

	function isActionBoundOnPickupEquip( action )
	{
		if (action && ("mItemData" in action) && action.mItemData && !action.mItemData.mBound && action._getItemDef() && (action._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_EQUIP || action._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_PICKUP))
		{
			return true;
		}

		return false;
	}

	function finishDrop( oldSlot, actionButton )
	{
		local oldActionContainer = oldSlot.getActionContainer();
		local results = this.mActionContainer.handleSwapIn(oldSlot, this);

		if (results.dropSucessful == true)
		{
			oldActionContainer.handleSwapOut(oldSlot, this, results.buttonsSwapped);
			this.mBroadcaster.broadcastMessage("onButtonDropped", this, actionButton);
			this.refreshInfoPanel();
		}
	}

	function removeActionButton()
	{
		if (this.mActionButton)
		{
			this.mActionButton.setActionButtonSlot(null);
			this.remove(this.mActionButton);
			this.mActionButton = null;

			if (this.mSwitchBackgroundOnEmpty)
			{
				this.restoreEmptyMaterial();
			}

			this.mBackground.setVisible(true);
		}

		this.setInfoPanelState(this.mShowInfoPanel);
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function restoreEmptyMaterial()
	{
		this.setBackgroundMaterial(this.mEmptyBackgroundMat);
	}

	function setAcceptDroppedButtons( validTarget )
	{
		this.mValidDropTarget = validTarget;
	}

	function setActionButton( actionButton )
	{
		if (actionButton == null)
		{
			this.removeActionButton();
			return;
		}

		local action = actionButton.getAction();

		if (!this.isItemPlacementValid(action.getEquipmentType()))
		{
			return false;
		}

		this.mKeybindingLabel.setVisible(false);
		this.remove(this.mActionButton);
		this.mActionButton = actionButton;
		this.mBackground.setVisible(this.mActionButton == null);

		if (this.mActionButton)
		{
			this.mActionButton.setActionButtonSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
			this.mActionButton._setKeybindingText(this.mKeybindingLabelText);
			this.mActionButton.setVisible(true);
			this.setInfoPanelState(this.mShowInfoPanel);
			this.mActionButton.setActionButtonSlot(this);
			this.add(this.mActionButton);
		}

		if (!action.isAvailableForUse())
		{
			this._enterFrameRelay.addListener(this.mActionButton);
		}

		this.refreshInfoPanel();
		return true;
	}

	function setActionContainer( actionContainer )
	{
		this.mActionContainer = actionContainer;
	}

	function setBackgroundMaterial( material, ... )
	{
		local apperance = this.mBackground.getAppearance();

		if (apperance == null)
		{
			this.mBackground.setMaterial(null, false);
			return;
		}

		local newMaterial = apperance + "/" + material;

		if (vargc > 0 && vargv[0] == true)
		{
			this.mEmptyBackgroundMat = material;
		}

		this.mBackground.setMaterial(newMaterial);
	}

	function setEquipmentRestriction( restriction )
	{
		this.mEquipmentRestriction = restriction;
	}

	function setInfoPanelState( state, ... )
	{
		this.mShowInfoPanel = state;

		if (this.mShowInfoPanel == true)
		{
			if (vargc > 0)
			{
				this.mMiniInfoPanelVersion = vargv[0];
			}

			this.mBackground.setPosition(this.mButtonSlotOffset, this.mButtonSlotOffset);

			if (this.mActionButton)
			{
				this.mActionButton.setPosition(this.mButtonSlotOffset, this.mButtonSlotOffset);
			}

			if (this.mMiniInfoPanelVersion)
			{
				this.setSize(this.mMiniInfoPanelSlotWidth, this.mMiniInfoPanelSlotHeight);
			}
			else
			{
				this.setSize(this.mInfoPanelSlotWidth, this.mInfoPanelSlotHeight);
			}

			this.setAppearance("ActionSlotImage");
		}
		else
		{
			this.mBackground.setPosition(0, 0);

			if (this.mActionButton)
			{
				this.mActionButton.setPosition(0, 0);
			}

			this.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		}

		this.refreshInfoPanel();
	}

	function setRestoreBackgroundOnEmpty( value )
	{
		this.mSwitchBackgroundOnEmpty = value;
	}

	function setSlotDisabled( value )
	{
		if (this.mActionButton)
		{
			this.mActionButton.showDisabledOverlay(value);
			this.mDisabled = value;
		}
	}

	function setIconHighlightRed( value )
	{
		this.mForceRedHighlight = value;

		if (this.mActionButton)
		{
			this.mActionButton.forceRedHighlight(value);
		}
	}

	function setSelected( value )
	{
		if (this.mActionContainer)
		{
			this.mActionContainer.setSlotSelected(this, value);
		}
	}

	function setSwapped( swapped )
	{
		this.mSwapped = swapped;
	}

	function setUseMode( mode )
	{
		this.mUseMode = mode;
	}

	function refreshInfoPanel()
	{
		if (!this.mShowInfoPanel)
		{
			return;
		}

		if (this.mInfoPanelContents != null)
		{
			this.remove(this.mInfoPanelContents);
		}

		if (this.mActionButton != null)
		{
			local mods;

			if (this.mActionContainer)
			{
				mods = this.mActionContainer.getTooltipRenderingModifiers(this.mActionButton.getAction().getType());
			}

			if (this.mMiniInfoPanelVersion)
			{
				if (null == mods)
				{
					mods = {};
				}

				mods.miniVersion <- true;
			}

			this.mActionButton.updateImageIfNeeded();
			this.mInfoPanelContents = this.mActionButton.getAction().getInfoPanel(mods);

			if (this.mInfoPanelContents)
			{
				this.mInfoPanelContents.setPosition(38.0 + this.mButtonSlotOffset, 3);
				this.mBackground.validate();
				this.mInfoPanelContents.setSize(this.getSize().width.tofloat() - (44.0 + this.mButtonSlotOffset), this.getSize().height - 8);
				this.add(this.mInfoPanelContents);
			}
		}
		else
		{
			this.mInfoPanelContents = null;
		}
	}

	function setDraggable( draggable )
	{
		this.mDraggable = draggable;
	}

	function setPopupMenu( menu )
	{
		this.mPopupMenu = menu;
	}

	function setKeybinding( keybinding )
	{
		this.mKeybindingLabelText = keybinding;

		if (keybinding)
		{
			if (this.mActionButton)
			{
				this.mActionButton._setKeybindingText(keybinding);
			}
			else
			{
				this.mKeybindingLabel.setText(this.mKeybindingLabelText);
			}
		}

		local labelSize = this.mKeybindingLabel.getPreferredSize();
		local Width = this.mButtonSlotWidth - labelSize.width - 4;
		this.mKeybindingLabel.setPosition(Width, 0.0);
	}

	function setKeybindingVisible( value )
	{
		this.mKeybindingLabel.setVisible(value);
	}

	function setInQuickbar( val )
	{
		this.mInQuickbar = val;
	}

	function setHeight( height )
	{
		this.mButtonSlotHeight = height;
		this.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mBackground.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mHighlightOverlay.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
	}

	function setHighlightOverlay( visible )
	{
		this.mHighlightOverlay.setVisible(visible);

		if (this.mActionButton)
		{
			this.mActionButton.setHighlightOverlay(visible);
		}
	}

	function setWidth( width )
	{
		this.mButtonSlotWidth = width;
		this.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mBackground.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
		this.mHighlightOverlay.setSize(this.mButtonSlotWidth, this.mButtonSlotHeight);
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.RBUTTON)
		{
			local joinedMenu;

			if (this.mActionButton)
			{
				local action = this.mActionButton.getAction();

				if (action)
				{
					local actionGUI = action.getPopupGui();

					if (actionGUI)
					{
						joinedMenu = actionGUI.createJoinedMenu(this.mPopupMenu, actionGUI, true);
					}
				}
			}

			if (joinedMenu)
			{
				joinedMenu.showMenu();
				evt.consume();
			}
			else if (this.mPopupMenu)
			{
				this.mPopupMenu.showMenu();
				evt.consume();
			}
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
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

}

