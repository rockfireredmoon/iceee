this.require("GUI/Panel");
this.require("Preferences");
this.QUICKBAR_COUNT <- 8;
this.QUICKBAR_SNAP_THRESHOLD <- 10;
this.QuickBarKeyType <- [
	"",
	"Ctrl+",
	"Alt+",
	"Shift+",
	"Ctrl+Shift+",
	"Alt+Shift+",
	"Ctrl+Alt+",
	"Ctrl+Shift+Alt+"
];
class this.UI.QuickBar extends this.GUI.Panel
{
	static mClassName = "QuickBar";
	mStartingQuickbarPositions = [
		{
			x = 269,
			y = 518
		},
		{
			x = 536,
			y = 518
		},
		{
			x = 269,
			y = 478
		},
		{
			x = 536,
			y = 478
		},
		{
			x = 269,
			y = 438
		},
		{
			x = 536,
			y = 438
		},
		{
			x = 269,
			y = 398
		},
		{
			x = 536,
			y = 398
		}
	];
	mSlotsX = 0;
	mSlotsY = 0;
	mSnapX = null;
	mSnapY = null;
	mRelX = 0.0;
	mRelY = 0.0;
	mContainer = null;
	mActionContainer = null;
	mMouseOffset = {};
	mDragging = false;
	mLocked = false;
	mIndex = null;
	mSlots = null;
	mPlusButton = null;
	mMinusButton = null;
	mMacroButton = null;
	mShowContextMenu = true;
	mDefaultPosX = 0.0;
	mDefaultPosY = 0.0;
	constructor( index )
	{
		this.mMouseOffset = {};
		this.mDragging = false;
		this.mLocked = false;
		this.mSnapX = null;
		this.mSnapY = null;
		this.mActionContainer = null;
		this.mIndex = index;
		this.mRelX = 1.0;
		this.mRelY = 1.0;
		this.GUI.Panel.constructor(this.GUI.BoxLayout());
		this.setInsets(4);
		this.initSlots();
		this.setLayout(1, 8);
		local xPos = this.mStartingQuickbarPositions[index].x.tofloat() / (::Screen.getWidth() - this.getWidth());
		local yPos = this.mStartingQuickbarPositions[index].y.tofloat() / (::Screen.getHeight() - this.getHeight() / 2);
		this.setPositionRel(xPos, yPos);
		this.updateRelativePosition();

		if (this.mIndex == 0)
		{
			this.setLocked(true);
		}
	}

	function activateIndex( index )
	{
		local actionButtons = this.mActionContainer.getAllActionButtons(false);

		if (actionButtons[index] != null)
		{
			actionButtons[index].activate();
			return true;
		}

		return false;
	}

	function initSlots()
	{
		this.mSlots = [];
		local i;

		for( i = 0; i < 8; i++ )
		{
			this.mSlots.append({
				slot = null,
				action = null
			});
		}
	}

	function prepareButtons()
	{
		local slotIndex = 0;

		foreach( s in this.mSlots )
		{
			local actionString = s.action;
			local button;

			if (actionString)
			{
				local macroLoc = actionString.find("MACRO");
				local abilityLoc = actionString.find("ABILITY");

				if (macroLoc != null)
				{
					local id = -1;
					local string = actionString.slice(macroLoc + "MACRO".len());
					local idLoc = string.find("id:");

					if (idLoc != null)
					{
						id = string.slice(idLoc + "id:".len());
					}

					local macro = ::_macroManager.getMacroByID(id.tointeger());

					if (macro)
					{
						this.mActionContainer.addAction(macro, false, slotIndex);
					}
				}
				else if (abilityLoc != null)
				{
					local string = actionString.slice(abilityLoc + "ABILITY".len());
					local abilityIdLoc = string.find("id:");
					local abilityId = string.slice("id:".len(), string.len());
					local ability = ::_AbilityManager.getAbilityById(abilityId.tointeger());

					if (ability)
					{
						this.mActionContainer.addAction(ability, false, slotIndex);
					}
				}
				else
				{
					local loc = actionString.find(":");

					if (loc != null)
					{
						local name = actionString.slice(0, loc);
						local data = actionString.slice(loc + 1);

						if (name == "itemdef")
						{
							this.mActionContainer.addAction(::_ItemManager.getItemDef(data.tointeger()), false, slotIndex);
						}
						else if (name == "item")
						{
							local itemAction = ::_ItemManager.getItem(data);
							this.mActionContainer.addAction(itemAction, false, slotIndex);
						}
					}
				}
			}

			slotIndex++;
		}

		this.mActionContainer.updateContainer();
	}

	function setPositionRel( x, y )
	{
		this.mRelX = x;
		this.mRelY = y;
		local w = this.getWidth() / ::Screen.getWidth().tofloat();
		local h = this.getHeight() / ::Screen.getHeight().tofloat();
		this.setDefaultPosition((this.mRelX - w / 2) * ::Screen.getWidth(), (this.mRelY - h / 2) * ::Screen.getHeight());
		this.fitInScreen();
	}

	function setDefaultPosition( x, y )
	{
		this.mDefaultPosX = x;
		this.mDefaultPosY = y;
	}

	function getDefaultPosition()
	{
		return {
			x = this.mDefaultPosX,
			y = this.mDefaultPosY
		};
	}

	function updateRelativePosition()
	{
		if (this.mIndex != 0)
		{
			this.setPositionRel(this.mRelX, this.mRelY);
			this.updateSnap();
		}
	}

	function updateDisplayedBinding( index, usage )
	{
		local modifiedPrefix = "";

		if (usage != "")
		{
			local splitKeys = this.Util.split(usage, "+");
			local foundModButton = false;
			local key = "";

			foreach( splitKey in splitKeys )
			{
				switch(splitKey)
				{
				case "Ctrl":
					modifiedPrefix += "c";
					foundModButton = true;
					break;

				case "Alt":
					modifiedPrefix += "a";
					foundModButton = true;
					break;

				case "Shift":
					modifiedPrefix += "s";
					foundModButton = true;
					break;

				default:
					key = splitKey;
				}
			}

			if (foundModButton)
			{
				modifiedPrefix += "-";
			}

			modifiedPrefix += key;
		}

		this.mActionContainer.setSlotKeyBinding(index, modifiedPrefix);
	}

	function setLayout( slotsY, slotsX )
	{
		this.mSlotsX = slotsX;
		this.mSlotsY = slotsY;

		if (this.mActionContainer)
		{
			this.remove(this.mActionContainer);
		}

		local oldKeybindings = [];

		if (this.mActionContainer)
		{
			local actionSlots = this.mActionContainer.getAllActionButtonSlots();

			foreach( slot in actionSlots )
			{
				oldKeybindings.append(slot.getKeybinding());
			}
		}

		this.mActionContainer = this.GUI.ActionContainer("quickbar", this.mSlotsY, this.mSlotsX, 0, 0, this, false);
		this.mActionContainer.setShowBindingInfo(false);
		this.mActionContainer.setShowEquipmentComparison(false);
		this.add(this.mActionContainer);

		for( local i = 0; i < oldKeybindings.len(); i++ )
		{
			this.updateDisplayedBinding(i, oldKeybindings[i]);
		}

		this.mActionContainer.setSlotUseMode(0, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(1, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(2, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(3, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(4, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(5, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(6, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setSlotUseMode(7, this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		this.mActionContainer.setAllowButtonDisownership(true);
		this.setContainerMoveProperties();

		if (this.mIndex != 0)
		{
			local sz = this.getPreferredSize();
			this.setSize(this.getPreferredSize());
			this.invalidate();
			local pos = this.getPosition();
			this.mRelX = pos.x.tofloat() / (::Screen.getWidth() - this.getWidth());
			this.mRelY = pos.y.tofloat() / (::Screen.getHeight() - this.getHeight());
			this.setOverlay("GUI/QuickBarOverlay");
			this.updateSnap();
		}

		this.prepareButtons();
	}

	function removeBinding( binding )
	{
		local oldKeybindings = [];

		if (this.mActionContainer)
		{
			local actionSlots = this.mActionContainer.getAllActionButtonSlots();

			foreach( slot in actionSlots )
			{
				if (slot.getKeybinding() == binding)
				{
					slot.setKeybinding("");
				}
			}
		}
	}

	function setLocked( which )
	{
		this.mLocked = which;
	}

	function setDefaultKeybindings()
	{
		for( local i = 0; i < this.mSlots.len(); i++ )
		{
			this.updateDisplayedBinding(i, this.QuickBarKeyType[this.mIndex] + (i + 1));
		}
	}

	function getLocked()
	{
		return this.mLocked;
	}

	function getIndex()
	{
		return this.mIndex;
	}

	function onMenuItemPressed( menu, menuID )
	{
		if (menuID.find("Layout") != null)
		{
			local tmp = menuID.slice(menuID.find(".") + 1);
			local parts = this.Util.split(tmp, "x");
			this.setLayout(parts[0].tointeger(), parts[1].tointeger());
			this.save();
		}
		else if (menuID == "AddButton")
		{
		}
		else if (menuID == "ToggleLock")
		{
			this.setLocked(!this.mLocked);
			this.save();
		}
	}

	function showPopUp()
	{
		local menu = this.GUI.PopupMenu();
		menu.addMenuOption("ToggleLock", this.mLocked ? "Unlock" : "Lock");
		menu.addMenuOption("-", "-");
		menu.addMenuOption("Layout", "Layout");
		menu.addMenuOption("Layout.1x8", "1x8");
		menu.addMenuOption("Layout.8x1", "8x1");
		menu.addMenuOption("Layout.2x4", "2x4");
		menu.addMenuOption("Layout.4x2", "4x2");
		menu.addActionListener(this);
		menu.showMenu();
	}

	function fitInScreen()
	{
		if (this.mIndex == 0)
		{
			return;
		}

		local pos = this.getDefaultPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function onMousePressed( evt )
	{
		if (evt.button == 1)
		{
			if (this.mLocked == false)
			{
				this.mMouseOffset = ::Screen.getCursorPos();
				this.mDragging = true;
			}
		}
		else if (evt.button == 3)
		{
			if (this.mShowContextMenu)
			{
				this.showPopUp();
			}
		}

		evt.consume();
	}

	function onMouseReleased( evt )
	{
		if (this.mDragging)
		{
			this.mDragging = false;
			evt.consume();
			local pos = this.getPosition();

			if (pos.x < this.QUICKBAR_SNAP_THRESHOLD)
			{
				this.mSnapX = 0.0;
			}
			else if (pos.x + this.getWidth() > ::Screen.getWidth() - this.QUICKBAR_SNAP_THRESHOLD)
			{
				this.mSnapX = 1.0;
			}
			else
			{
				this.mSnapX = null;
			}

			if (pos.y < this.QUICKBAR_SNAP_THRESHOLD)
			{
				this.mSnapY = 0.0;
			}
			else if (pos.y + this.getHeight() > ::Screen.getHeight() - this.QUICKBAR_SNAP_THRESHOLD)
			{
				this.mSnapY = 1.0;
			}
			else
			{
				this.mSnapY = null;
			}

			this.updateSnap();
			this.save();
		}
	}

	function updateSnap()
	{
		local pos = this.getPosition();
		this.setDefaultPosition(this.mSnapX != null ? ::Screen.getWidth() * this.mSnapX : pos.x, this.mSnapY != null ? ::Screen.getHeight() * this.mSnapY : pos.y);
		this.fitInScreen();
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local pos = this.getPosition();
			local newMousePos = ::Screen.getCursorPos();
			pos.x += newMousePos.x - this.mMouseOffset.x;
			pos.y += newMousePos.y - this.mMouseOffset.y;
			pos.x = pos.x > 0 ? pos.x : 0;
			pos.y = pos.y > 0 ? pos.y : 0;
			pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
			pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
			this.mRelX = pos.x.tofloat() / (::Screen.getWidth() - this.getWidth());
			this.mRelY = pos.y.tofloat() / (::Screen.getHeight() - this.getHeight());
			this.mMouseOffset = newMousePos;
			this.setPosition(pos);
			evt.consume();
		}
	}

	function updateDefIdStackCount( defId, newStackCount )
	{
		local buttons = this.mActionContainer.getAllActionButtons(false);

		foreach( button in buttons )
		{
			if (button)
			{
				local quickbarAction = button.getAction().getQuickBarAction();

				if (quickbarAction && (quickbarAction instanceof this.ItemDefAction) && defId == quickbarAction.mItemDefId)
				{
					button.setStackCount(newStackCount);
				}
			}
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
		this.mWidget.setChildProcessingEvents(true);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Panel._removeNotify();
	}

	function save()
	{
		::_quickBarManager.saveQuickbar(this.mIndex);
	}

	function setContainerMoveProperties()
	{
		this.mActionContainer.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.MOVE, this));
		this.mActionContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this, this));
		this.mActionContainer.addAcceptingFromProperties("equipment", this.AcceptFromProperties(null, this));
		this.mActionContainer.addAcceptingFromProperties("quickbar", this.AcceptFromProperties(null, this));
		this.mActionContainer.addAcceptingFromProperties("macro_container", this.AcceptFromProperties(null, this));
		this.mActionContainer.addAcceptingFromProperties("ability_screen_container", this.AcceptFromProperties(this, this));
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local actionButton = oldSlot.getActionButton();

		if (!actionButton)
		{
			return false;
		}

		local action = actionButton.getAction();

		if (!action)
		{
			return false;
		}

		if (action instanceof this._AbilityScreenAction)
		{
			local abilityDef = ::_AbilityManager.getAbilityById(action.mAbilityId);

			if (abilityDef.mAbilityClass == "Passive")
			{
				this.IGIS.error("Passive abilities cannot be moved into quickbars.");
				return false;
			}
		}

		return true;
	}

	function getActionButtonValid( slot, button )
	{
		local action = button.getAction();

		if (!action || action.getQuickBarAction() == null)
		{
			return false;
		}

		return true;
	}

	function findSlotIndex( slot )
	{
		for( local i = 0; i < this.mSlots.len(); i++ )
		{
			local buttonSlot = this.mSlots[i];

			if (buttonSlot.slot == slot)
			{
				return i;
			}
		}

		return null;
	}

	function removeItem( action )
	{
		if (action && action.mItemDefData && (action.mItemDefData.mType == this.ItemType.CONSUMABLE || action.mItemDefData.mIvType1 == this.ItemIntegerType.STACKING))
		{
			return;
		}

		local buttons = this.mActionContainer.getAllActionButtons(false);

		foreach( s in buttons )
		{
			if (s)
			{
				local quickbarAction = s.getAction().getQuickBarAction();

				if (quickbarAction && (quickbarAction instanceof this.ItemAction) && action.getItemId() == quickbarAction.getItemId())
				{
					local slotIndex = this.mActionContainer.findSlotIndexOfAction(quickbarAction);
					this.mActionContainer._removeActionButton(slotIndex);
				}
			}
		}

		this.save();
	}

	function onActionButtonGained( newSlot, oldSlot )
	{
		local oldContainerName = oldSlot.getActionContainer().getContainerName();
		local button = newSlot.getActionButton();
		local action = button.getAction();

		if (oldContainerName == "ability_screen_container")
		{
			local abilityAction = ::_AbilityManager.getAbilityById(action.getAbilityId());
			button.bindActionToButton(abilityAction);
			action = abilityAction;
		}
		else if (oldContainerName == "inventory")
		{
			button.bindActionToButton(action.getQuickBarAction());
		}

		if (action && action.getIsValid())
		{
			if (::_AbilityManager.getTimeUntilCategoryUseable(action.getCooldownCategory()) > 0)
			{
				this._enterFrameRelay.addListener(button);
			}
		}

		this.save();
	}

	function onActionButtonLost( newSlot, oldSlot )
	{
		this.save();
	}

	function getActionContainer()
	{
		return this.mActionContainer;
	}

	function serialize()
	{
		local buttons = this.mActionContainer.getAllActionButtons(false);
		local quickbarStrings = [];
		local idx = 0;

		foreach( s in buttons )
		{
			local actionString;

			if (s)
			{
				local quickbarAction = s.getAction();
				actionString = quickbarAction.getQuickbarString();
				quickbarStrings.append(actionString);
			}
			else
			{
				quickbarStrings.append(null);
			}

			this.mSlots[idx].action = actionString;
			idx++;
		}

		local pos = this.getPosition();
		return ::serialize({
			x = this.mRelX,
			y = this.mRelY,
			slotsX = this.mSlotsX,
			slotsY = this.mSlotsY,
			buttons = quickbarStrings,
			locked = this.mLocked,
			visible = this.mIsVisible,
			snapX = this.mSnapX,
			snapY = this.mSnapY,
			positionX = pos.x,
			positionY = pos.y
		});
	}

	function unserialize( data )
	{
		local table = ::unserialize(data);
		local i;

		for( i = 0; i < 8; i++ )
		{
			local bindingName = table.buttons[i];
			local s = this.mSlots[i];
			s.action = bindingName;
		}

		if (this.mIndex == 0)
		{
			this.setVisible(true);
		}
		else
		{
			this.setVisible(table.visible);
		}

		this.setLayout(table.slotsY, table.slotsX);

		if (this.mIndex != 0)
		{
			this.setPositionRel(table.x, table.y);

			if ("snapX" in table)
			{
				this.mSnapX = table.snapX;
			}

			if ("snapY" in table)
			{
				this.mSnapY = table.snapY;
			}

			this.updateSnap();
			local pos = this.getPosition();

			if ("positionX" in table)
			{
				pos.x = table.positionX;
			}

			if ("positionY" in table)
			{
				pos.y = table.positionY;
			}

			this.setDefaultPosition(pos.x, pos.y);
			this.fitInScreen();
		}

		if ("locked" in table)
		{
			this.setLocked(table.locked);
		}

		if (this.mIndex == 0)
		{
			this.setLocked(true);
		}

		::_quickBarManager.setQuickbarUnserialized(this);
	}

}

::_macroWindow <- null;
class this.UI.QuickBarManager 
{
	mContents = null;
	mQuickBars = null;
	mVisible = false;
	mMessageBroadcaster = null;
	mUnserializedQuickbars = null;
	constructor()
	{
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mQuickBars = [];
		this.mUnserializedQuickbars = this.array(this.QUICKBAR_COUNT);

		for( local i = 0; i < this.QUICKBAR_COUNT; i++ )
		{
			this.mUnserializedQuickbars[i] = false;
		}

		this.hide();
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function initialize()
	{
		local x;

		for( x = 0; x < this.QUICKBAR_COUNT; x++ )
		{
			local qb = this.UI.QuickBar(x);

			if (x == 0)
			{
				local mainScreen = this.Screens.get("MainScreen", true);

				if (mainScreen)
				{
					mainScreen.addQuickBar(qb);
				}
			}
			else
			{
				qb.setVisible(false);
			}

			this.mQuickBars.append(qb);
		}

		this.setDefaultKeybindings();
	}

	function shutdown()
	{
		foreach( b in this.mQuickBars )
		{
			::Pref.set("quickbar." + b.getIndex().tostring(), null);
			b.destroy();
		}

		this.mQuickBars = [];
	}

	function abilityUseRequested( abilityId )
	{
		foreach( quickbar in this.mQuickBars )
		{
			local quickbarAc = quickbar.getActionContainer();

			if (quickbarAc)
			{
				local actionButtonList = quickbarAc.getAllActionButtons();

				if (actionButtonList.len() > 0)
				{
					foreach( actionButton in actionButtonList )
					{
						local action = actionButton.getAction();

						if (action && (action instanceof this.Ability))
						{
							if (action.getId() == abilityId)
							{
								local abilityActiveAnimation = this.GUI.Container();
								abilityActiveAnimation.setSize(32, 32);
								actionButton.addExtraComponent(abilityActiveAnimation);
								abilityActiveAnimation.setMaterial("Icon/Selection");
							}
						}
					}
				}
			}
		}
	}

	function abilityUsed( abilityId )
	{
		foreach( quickbar in this.mQuickBars )
		{
			local quickbarAc = quickbar.getActionContainer();

			if (quickbarAc)
			{
				local actionButtonList = quickbarAc.getAllActionButtons();

				if (actionButtonList.len() > 0)
				{
					foreach( actionButton in actionButtonList )
					{
						local action = actionButton.getAction();

						if (action && (action instanceof this.Ability))
						{
							if (action.getId() == abilityId)
							{
								actionButton.removeExtraComponent();
							}
						}
					}
				}
			}
		}
	}

	function isQuickBarVisible( index )
	{
		if (index == 0)
		{
			return true;
		}

		if (index < this.mQuickBars.len())
		{
			return this.mQuickBars[index].isVisible();
		}
	}

	function isQuickbarUnserialized( index )
	{
		return this.mUnserializedQuickbars[index];
	}

	function getQuickBar( index )
	{
		if (index < this.mQuickBars.len())
		{
			return this.mQuickBars[index];
		}
	}

	function setQuickBarVisible( index, which )
	{
		if (index == 0)
		{
			this.mQuickBars[index].setVisible(true);
			return;
		}

		this.mQuickBars[index].setVisible(which);
	}

	function setDefaultKeybindings()
	{
		for( local j = 0; j < this.QUICKBAR_COUNT; j++ )
		{
			this.mQuickBars[j].setDefaultKeybindings();
		}
	}

	function removeBindingFromAllQuickbars( binding )
	{
		for( local i = 0; i < this.mQuickBars.len(); i++ )
		{
			this.mQuickBars[i].removeBinding(binding);
		}
	}

	function getQuickBarCount()
	{
		return this.mQuickBars.len();
	}

	function onScreenResize()
	{
		foreach( index, b in this.mQuickBars )
		{
			if (index != 0)
			{
				b.updateRelativePosition();
			}
		}
	}

	function removeItemFromQuickbar( action )
	{
		foreach( b in this.mQuickBars )
		{
			b.removeItem(action);
		}
	}

	function updateDefIdStackCount( defId, newStackCount )
	{
		foreach( bar in this.mQuickBars )
		{
			bar.updateDefIdStackCount(defId, newStackCount);
		}
	}

	function saveQuickbar( id )
	{
		::Pref.set("quickbar." + id.tostring(), this.mQuickBars[id].serialize(), true, false);
	}

	function changeIcon( row, slot, id )
	{
		this.mContents[row][slot] = id;
		this._saveToDisk();
	}

	function getVisible()
	{
		return this.mVisible;
	}

	function show()
	{
		::Screen.setOverlayVisible("GUI/QuickBarOverlay", true);
		this.mVisible = true;
	}

	function hide()
	{
		::Screen.setOverlayVisible("GUI/QuickBarOverlay", false);
		this.mVisible = false;
	}

	function destroy()
	{
	}

	function toggleQuickbar( quickbarNum )
	{
		local visible = this.mQuickBars[quickbarNum].isVisible();

		if (quickbarNum == 0)
		{
			visible = false;
		}

		this.setQuickBarVisible(quickbarNum, !visible);
		::_quickBarManager.saveQuickbar(quickbarNum);
		this.mUnserializedQuickbars[quickbarNum] = true;
	}

	function setQuickbarUnserialized( quickbar )
	{
		this.mMessageBroadcaster.broadcastMessage("onQuickbarUnserialized", this, quickbar);
		this.mUnserializedQuickbars[quickbar.getIndex()] = true;
	}

	function setCategoryUsable( cooldownCategory, value )
	{
		if (cooldownCategory == null || cooldownCategory == "")
		{
			return;
		}

		foreach( quickbar in this.mQuickBars )
		{
			local buttons = quickbar.mActionContainer.getAllActionButtons();

			foreach( button in buttons )
			{
				local action = button.getAction();

				if (action && action.getIsValid())
				{
					if (action.getCooldownCategory() == cooldownCategory || cooldownCategory == "Global")
					{
						if (value)
						{
							button.clearCooldownVisuals();
						}
						else
						{
							this._enterFrameRelay.addListener(button);
						}
					}
				}
			}
		}
	}

}

this.gActionBackgrounds <- [
	"StandardGrey",
	"StandardCyan",
	"StandardBlue",
	"StandardYellow",
	"Green",
	"Purple",
	"Red"
];
this.gActionIcons <- [
	"QuestionMark",
	"Male",
	"Female",
	"Apple",
	"Axe",
	"Bedroll",
	"Katars",
	"Shovel",
	"Sword",
	"Uber_Sword",
	"LetterB",
	"LetterE",
	"LetterS",
	"LetterV"
];
this.Pref.PreferenceUpdate_quickbar_0 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(0).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_1 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(1).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_2 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(2).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_3 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(3).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_4 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(4).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_5 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(5).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_6 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(6).unserialize(value);
	}
};
this.Pref.PreferenceUpdate_quickbar_7 <- function ( value )
{
	if (value != "")
	{
		::_quickBarManager.getQuickBar(7).unserialize(value);
	}
};
