this.require("UI/ActionButtonSlot");
this.MovementTypes <- {
	CLONE = 0,
	MOVE = 1,
	LOCK = 2,
	DESTROY = 3
};
this.ContainerLists <- {};
class this.GUI.ActionContainer extends this.GUI.Container
{
	mPageInfo = null;
	mPageLabel = null;
	mPageBack = null;
	mPageForward = null;
	mPagingInfoEnabled = true;
	mPagingInfoVisible = false;
	mItemPanelVisible = false;
	mMiniItemPanelVersion = false;
	mRowsVisible = 0;
	mColumnsVisible = 0;
	mMaxPages = 1;
	mCurrentPageNumber = 1;
	mActionButtonList = [];
	mActionButtonSlotList = [];
	mIconContainer = null;
	mPageContainer = null;
	mPerPage = 0;
	mAutoPosition = true;
	mContainerInvisible = false;
	mBroadcaster = null;
	mKeyBindingEnabled = false;
	mTempAdd = null;
	mTempRemove = null;
	mShowDebuggingButtons = false;
	mSlotWidthOverride = -1;
	mSlotHeightOverride = -1;
	mContainerName = null;
	mMovingToProperties = null;
	mAcceptingFromProperties = null;
	mLockedList = null;
	mAllowButtonDisownership = false;
	mTooltipRenderingModifiers = null;
	mUseMode = null;
	mCallback = null;
	mKeepTrackOfClickedIndex = false;
	mLastClickedIndex = -1;
	mActionContainerOwner = null;
	mFrontInsert = false;
	mDelayContainerUpdateOnItemUpdate = false;
	mDelayContainerUpdateEvent = null;
	mShowBindingInfo = true;
	mShowEquipmentComparison = true;
	constructor( name, rows, columns, iconSpacingX, iconSpacingY, owner, ... )
	{
		this.mActionButtonList = [];
		this.mActionButtonSlotList = [];
		this.mLockedList = [];
		this.mMovingToProperties = {};
		this.mAcceptingFromProperties = {};
		this.mTooltipRenderingModifiers = {};
		this.mActionContainerOwner = owner;
		this.mRowsVisible = rows;
		this.mColumnsVisible = columns;
		this.mContainerName = name;
		this.ContainerLists[this.mContainerName] <- this;

		if (vargc > 0)
		{
			this.mAutoPosition = vargv[0];
		}

		if (vargc > 1)
		{
			this.mSlotWidthOverride = vargv[1];
		}

		if (vargc > 2)
		{
			this.mSlotHeightOverride = vargv[2];
		}

		if (this.mAutoPosition == false)
		{
			this.mActionButtonList.resize(this.mRowsVisible * this.mColumnsVisible);
		}

		this.mBroadcaster = this.MessageBroadcaster();
		this.GUI.Container.constructor(this.GUI.BoxLayoutV(1, 1));
		this.mPageContainer = this.GUI.Component(this.GUI.BoxLayoutV(1, 1));
		local _layout = this.GUI.GridLayout(this.mRowsVisible, this.mColumnsVisible);
		_layout.setGaps(iconSpacingX, iconSpacingY);
		this.mIconContainer = this.GUI.Container(_layout);
		this.mIconContainer.setInsets(iconSpacingY, iconSpacingX, iconSpacingY, iconSpacingX);
		this.mPageInfo = this.GUI.Container(this.GUI.BorderLayout());
		this.mPageInfo.setInsets(0);
		this.mPageInfo.setSize(0, 25);
		this.mPageInfo.setPreferredSize(0, 25);
		this.mPageLabel = this.GUI.Label("");
		this.mPageLabel.setFontColor(this.Colors.white);
		this.mPageForward = this.GUI.SmallButton("RightArrow");
		this.mPageForward.addActionListener(this);
		this.mPageBack = this.GUI.SmallButton("LeftArrow");
		this.mPageBack.addActionListener(this);

		if (this.mShowDebuggingButtons)
		{
			this.mTempAdd = this.GUI.Button("Add");
			this.mTempRemove = this.GUI.Button("Remove");
		}

		this.mPageLabel.setVisible(this.mPagingInfoVisible);
		this.mPageBack.setVisible(this.mPagingInfoVisible);
		this.mPageForward.setVisible(this.mPagingInfoVisible);
		this.mPageInfo.add(this.mPageBack, this.GUI.BorderLayout.WEST);
		this.mPageInfo.add(this.mPageLabel, this.GUI.BorderLayout.CENTER);
		this.mPageInfo.add(this.mPageForward, this.GUI.BorderLayout.EAST);
		this.mPageBack.setReleaseMessage("onPageBack");
		this.mPageForward.setReleaseMessage("onPageForward");

		if (this.mShowDebuggingButtons)
		{
			this.mTempAdd.setReleaseMessage("_onActionAdd");
			this.mTempRemove.setReleaseMessage("_onIconRemove");
		}

		this.mPageBack.addActionListener(this);
		this.mPageForward.addActionListener(this);

		if (this.mShowDebuggingButtons)
		{
			this.mTempAdd.addActionListener(this);
			this.mTempRemove.addActionListener(this);
		}

		this.mPerPage = this.mRowsVisible * this.mColumnsVisible;
		this.mPageContainer.add(this.mIconContainer);

		for( local i = 0; i < this.mPerPage; i++ )
		{
			local actionButtonSlot = this.GUI.ActionButtonSlot();
			actionButtonSlot.setInfoPanelState(false);
			actionButtonSlot.setAcceptDroppedButtons(true);
			actionButtonSlot.setActionContainer(this);
			this.mActionButtonSlotList.push(actionButtonSlot);
			this.mIconContainer.add(actionButtonSlot);
			actionButtonSlot.setActionContainer(this);

			if (this.mSlotWidthOverride != -1)
			{
				actionButtonSlot.setWidth(this.mSlotWidthOverride);
			}

			if (this.mSlotHeightOverride != -1)
			{
				actionButtonSlot.setHeight(this.mSlotHeightOverride);
			}
		}

		this.add(this.mPageContainer);

		if (this.mShowDebuggingButtons)
		{
			this.add(this.mTempAdd);
			this.add(this.mTempRemove);
		}
	}

	function addAction( action, shouldUpdateContainer, ... )
	{
		local actionButton = this.GUI.ActionButton();
		actionButton.bindActionToButton(action);

		if (vargc > 0)
		{
			this.addActionFromButton(actionButton, shouldUpdateContainer, vargv[0]);
		}
		else
		{
			this.addActionFromButton(actionButton, shouldUpdateContainer);
		}

		return actionButton;
	}

	function addActionFromButton( actionButton, shouldUpdateContainer, ... )
	{
		actionButton.setPreviousActionContainer(this);

		if (this.mFrontInsert == true)
		{
			this.mActionButtonList.insert(0, actionButton);
		}
		else if (this.mAutoPosition == false)
		{
			local index = 0;

			if (vargc > 0)
			{
				index = vargv[0];
			}
			else
			{
				index = this._findNextFreeIndex();
			}

			if (index >= 0 && index < this.mActionButtonList.len())
			{
				this.mActionButtonList[index] = actionButton;
			}
		}
		else
		{
			this.mActionButtonList.push(actionButton);
		}

		if (shouldUpdateContainer)
		{
			this.updateContainer();
		}
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function addAcceptingFromProperties( actionContainerName, properties )
	{
		this.mAcceptingFromProperties[actionContainerName] <- properties;
	}

	function addMovingToProperties( actionContainerName, properties )
	{
		this.mMovingToProperties[actionContainerName] <- properties;
	}

	function attemptDisownButton( slot )
	{
		local slotIndex = this.getIndexOfSlot(slot);
		local action;
		local previousContainer;
		local button = slot.getActionButton();

		if (button)
		{
			previousContainer = button.getPreviousActionContainer();

			if (previousContainer)
			{
				action = button.getAction();
			}
		}

		if (this.mAllowButtonDisownership)
		{
			if (action)
			{
				local index = previousContainer.findSlotIndexOfAction(action);

				if (index >= 0)
				{
					previousContainer._unlockSlot(previousContainer.mActionButtonSlotList[index]);
				}
			}

			this._removeActionButton(slotIndex);

			if ("onActionButtonLost" in this.mActionContainerOwner)
			{
				this.mActionContainerOwner.onActionButtonLost(null, slot);
			}
		}
		else
		{
			this.mBroadcaster.broadcastMessage("onAttemptedItemDisown", this, action, slotIndex);
		}
	}

	function buttonDroppedOnSlot( actionButtonSlot, actionButton, oldSlotsButton )
	{
		local slotIndex = -1;

		if (this.mAutoPosition == false)
		{
			local droppedIndex = this.getIndexOfSlot(actionButtonSlot);

			if (droppedIndex >= 0)
			{
				slotIndex = (this.mCurrentPageNumber - 1) * this.mColumnsVisible + droppedIndex;
			}
		}
		else
		{
			slotIndex = this._findNextFreeIndex();
		}

		if (slotIndex >= 0)
		{
			this.mActionButtonList[slotIndex] = actionButton;
			this.mBroadcaster.broadcastMessage("onItemMovedInContainer", this, slotIndex, oldSlotsButton);
		}
		else
		{
			this.log.debug("Error - no free space in container");
		}

		this.updateContainer();
	}

	function clearActiveSelection()
	{
		if (!this.mKeepTrackOfClickedIndex)
		{
			return;
		}

		if (this.mLastClickedIndex >= 0)
		{
			local oldActionSlot = this.mActionButtonSlotList[this.mLastClickedIndex];

			if (oldActionSlot)
			{
				oldActionSlot.setBlendColor(this.Color(this.Colors["GUI Gold"]));
			}

			this.mLastClickedIndex = -1;
		}
	}

	function copyActionButton( actionButton, forquickbar, slot )
	{
		local action = actionButton.getAction();

		if (forquickbar)
		{
			action = action.getQuickBarAction();
		}

		slot.mBackground.setVisible(false);
		slot.mKeybindingLabel.setVisible(false);
		local newActionButton = this.GUI.ActionButton();
		newActionButton.setActionButtonSlot(slot);
		newActionButton.bindActionToButton(action);
		newActionButton.setSize(slot.mButtonSlotWidth, slot.mButtonSlotHeight);

		if (slot.mKeybindingLabelText)
		{
			newActionButton._setKeybindingText(slot.mKeybindingLabelText);
		}

		slot.setInfoPanelState(slot.mShowInfoPanel, slot.mMiniInfoPanelVersion);
		newActionButton.mTimer = actionButton.getTimer();
		slot.setActionButton(newActionButton);
		local slotsActionContainer = slot.getActionContainer();

		if (slotsActionContainer)
		{
			slotsActionContainer.buttonDroppedOnSlot(slot, newActionButton, actionButton);
		}

		if (slotsActionContainer && actionButton.getPreviousActionContainer() != slotsActionContainer)
		{
			local oldContainer = actionButton.getPreviousActionContainer();

			if (oldContainer)
			{
				newActionButton.setPreviousActionContainer(oldContainer);
			}
		}
		else
		{
			newActionButton.setPreviousActionContainer(actionButton.getPreviousActionContainer());
		}
	}

	function doesAcceptFromContainer( containerName )
	{
		if (containerName in this.mAcceptingFromProperties)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	function forceShowDisabledOverlayWithColor( index, color )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			local button = this.mActionButtonSlotList[index].getActionButton();

			if (button != null)
			{
				button.setDisabledOverlayColor(color);
				button.showDisabledOverlay(true);
			}
		}
	}

	function forceShowDisabledOverlayWithAppearance( index, appearance )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			local button = this.mActionButtonSlotList[index].getActionButton();

			if (button != null)
			{
				button.setDisabledOverlayAppearance(appearance);
				button.showDisabledOverlay(true);
			}
		}
	}

	function forceHideDisabledOverlay( index )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			local button = this.mActionButtonSlotList[index].getActionButton();

			if (button != null)
			{
				button.showDisabledOverlay(false);
			}
		}
	}

	function findNextFreeIndex()
	{
		local freeIndex = -1;
		local i = 0;

		for( i = 0; i < this.mActionButtonList.len(); i++ )
		{
			if (this.mActionButtonList[i] == null)
			{
				return i;
			}
		}

		return -1;
	}

	function findSlotIndexOfAction( action )
	{
		for( local i = 0; i < this.mActionButtonSlotList.len(); i++ )
		{
			if (this.mActionButtonSlotList[i].getActionButton() && action == this.mActionButtonSlotList[i].getActionButton().getAction())
			{
				return i;
			}
		}

		return -1;
	}

	function getActionButtonValid( newSlot, button )
	{
		local oldSlot = button.getActionButtonSlot();

		if (!oldSlot)
		{
			return;
		}

		local oldActionContainer = oldSlot.getActionContainer();

		if (!oldActionContainer)
		{
			return;
		}

		local oldName = oldActionContainer.getContainerName();

		if (oldName in this.mAcceptingFromProperties)
		{
			if (this.mAcceptingFromProperties[oldName].mIsValidDropSlotCallback && "onValidDropSlot" in this.mAcceptingFromProperties[oldName].mIsValidDropSlotCallback)
			{
				return this.mAcceptingFromProperties[oldName].mIsValidDropSlotCallback.onValidDropSlot(newSlot, oldSlot);
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

	function getAllActionButtons( ... )
	{
		local collapsed = vargc > 0 ? vargv[0] : true;

		if (!collapsed)
		{
			return this.mActionButtonList;
		}
		else
		{
			local list = [];

			foreach( action in this.mActionButtonList )
			{
				if (action != null)
				{
					list.append(action);
				}
			}

			return list;
		}
	}

	function getAllActionButtonSlots()
	{
		return this.mActionButtonSlotList;
	}

	function getContainerName()
	{
		return this.mContainerName;
	}

	function getIndexOfSlot( slot )
	{
		for( local i = 0; i < this.mActionButtonSlotList.len(); i++ )
		{
			if (this.mActionButtonSlotList[i] == slot)
			{
				return i;
			}
		}

		return -1;
	}

	function getIndexKeybinding( index )
	{
		if (index > this.mActionButtonSlotList.len())
		{
			return null;
		}

		return this.mActionButtonSlotList[index].getKeybinding();
	}

	function getLastSlotClickedOn()
	{
		if (this.mHighlightLastClicked)
		{
			return this.mLastSlotClicked;
		}

		return null;
	}

	function getOwner()
	{
		return this.mActionContainerOwner;
	}

	function getNumSlots()
	{
		return this.mActionButtonSlotList.len();
	}

	function getSelectedSlot()
	{
		if (this.mKeepTrackOfClickedIndex && this.mLastClickedIndex >= 0)
		{
			return this.mActionButtonSlotList[this.mLastClickedIndex];
		}
	}

	function getShowBindingInfo()
	{
		return this.mShowBindingInfo;
	}

	function getShowEquipmentComparison()
	{
		return this.mShowEquipmentComparison;
	}

	function getActionInSlot( index )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			local slot = this.mActionButtonSlotList[index];
			local actionButton = slot.getActionButton();

			if (actionButton)
			{
				return actionButton.getAction();
			}
		}

		return null;
	}

	function getSlotContents( index )
	{
		if (index >= 0 && this.mActionButtonSlotList.len() > index)
		{
			return this.mActionButtonSlotList[index];
		}

		return null;
	}

	function getActionButtonFromIndex( index )
	{
		if (this.mActionButtonList.len() > index)
		{
			return this.mActionButtonList[index];
		}

		return null;
	}

	function getPreferredSize()
	{
		local slotWidth = 0;
		local slotHeight = 0;

		if (this.mItemPanelVisible == false)
		{
			slotWidth = this.mActionButtonSlotList[0].mButtonSlotWidth;
			slotHeight = this.mActionButtonSlotList[0].mButtonSlotHeight;
		}
		else if (this.mMiniItemPanelVersion)
		{
			slotWidth = this.mActionButtonSlotList[0].mMiniInfoPanelSlotWidth;
			slotHeight = this.mActionButtonSlotList[0].mMiniInfoPanelSlotHeight;
		}
		else
		{
			slotWidth = this.mActionButtonSlotList[0].mInfoPanelSlotWidth;

			if (("itemProto" in this.mTooltipRenderingModifiers) && ("resizeInfoPanel" in this.mTooltipRenderingModifiers.itemProto) && this.mTooltipRenderingModifiers.itemProto.resizeInfoPanel == false)
			{
				slotHeight = this.mActionButtonSlotList[0].mInfoPanelSlotHeight + 25;
			}
			else
			{
				slotHeight = this.mActionButtonSlotList[0].mInfoPanelSlotHeight;
			}
		}

		local totalWidth = this.mColumnsVisible * slotWidth;
		local totalHeight = this.mRowsVisible * slotHeight;
		return {
			width = totalWidth,
			height = totalHeight
		};
	}

	function getSlotsRestriction( index )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			return this.mActionButtonSlotList[index].getEquipmentRestriction();
		}
	}

	function getTooltipRenderingModifiers( actiontype )
	{
		if (actiontype in this.mTooltipRenderingModifiers)
		{
			return this.mTooltipRenderingModifiers[actiontype];
		}
		else
		{
			return null;
		}
	}

	function handleSwapIn( oldSlot, newSlot )
	{
		local oldSlotsButton = oldSlot.getActionButton();
		local newSlotsButton = newSlot.getActionButton();
		local oldContainer = oldSlot.getActionContainer();
		local oldContainerName = oldContainer.getContainerName();
		local swappedButtons = false;
		local returnValue = {
			dropSucessful = false,
			buttonsSwapped = false
		};

		if (!this.doesAcceptFromContainer(oldContainerName))
		{
			return;
		}

		if (this.mAcceptingFromProperties[oldContainerName].mUnlockWhenDropped)
		{
			local oldAction = oldSlotsButton.getAction();

			foreach( lockedIndex in this.mLockedList )
			{
				local prevLockedSlot = this.mActionButtonSlotList[lockedIndex];
				local button = prevLockedSlot.getActionButton();
				local lockedAction = button.getAction();

				if (oldAction == lockedAction)
				{
					if (prevLockedSlot == newSlot)
					{
						this._unlockSlot(prevLockedSlot);
						returnValue.dropSucessful = true;
						returnValue.buttonsSwapped = false;
					}
					else
					{
						if (newSlotsButton)
						{
							this.copyActionButton(oldSlotsButton, false, newSlot);
							this.copyActionButton(newSlotsButton, false, prevLockedSlot);
							local newSlotIndex = this.getIndexOfSlot(newSlot);

							if (this.isIndexLocked(newSlotIndex))
							{
								this._unlockSlot(this.mActionButtonSlotList[newSlotIndex]);
							}
							else
							{
								this._unlockSlot(prevLockedSlot);
							}
						}
						else
						{
							this.copyActionButton(oldSlotsButton, false, newSlot);
							local prevSlotIndex = this.getIndexOfSlot(prevLockedSlot);
							this._removeActionButton(prevSlotIndex);
							this._unlockSlot(prevLockedSlot);
						}

						returnValue.dropSucessful = true;
						returnValue.buttonsSwapped = false;
					}
				}
			}
		}

		if (true != returnValue.dropSucessful)
		{
			if (!oldContainer.shouldCopyAction(newSlot))
			{
				returnValue.dropSucessful = true;
				returnValue.buttonsSwapped = false;
				return returnValue;
			}

			local newSlotIndex = this.getIndexOfSlot(newSlot);

			if (this.isIndexLocked(newSlotIndex))
			{
				returnValue.dropSucessful = false;
				returnValue.buttonsSwapped = false;
				return returnValue;
			}

			if (newSlotsButton && !this.mAutoPosition)
			{
				local newSlotsAction = newSlotsButton.getAction();

				if (oldSlot.isItemPlacementValid(newSlotsAction.getEquipmentType()))
				{
					this.copyActionButton(oldSlotsButton, false, newSlot);
					oldSlot.setSwapped(true);
					local oldSlotActionContainer = oldSlot.getActionContainer();

					if (oldSlotActionContainer && oldSlotActionContainer.doesAcceptFromContainer(this.mContainerName))
					{
						this.copyActionButton(newSlotsButton, false, oldSlot);
						returnValue.buttonsSwapped = true;
					}

					returnValue.dropSucessful = true;
				}
				else
				{
					this.IGIS.error("That item does not go in that slot");
					returnValue.dropSucessful = false;
					returnValue.buttonsSwapped = false;
				}
			}
			else if (!this.isContainerFull())
			{
				this.copyActionButton(oldSlotsButton, false, newSlot);
				returnValue.dropSucessful = true;
				returnValue.buttonsSwapped = false;
			}
		}

		if (true == returnValue.dropSucessful)
		{
			if (this.mAcceptingFromProperties[oldContainerName].mActionGainedCallback && "onActionButtonGained" in this.mAcceptingFromProperties[oldContainerName].mActionGainedCallback)
			{
				this.mAcceptingFromProperties[oldContainerName].mActionGainedCallback.onActionButtonGained(newSlot, oldSlot);
			}
		}

		return returnValue;
	}

	function handleSwapOut( oldSlot, newSlot, swapped )
	{
		local oldSlotsActionContainer = oldSlot.getActionContainer();
		local newSlotsActionContainer = newSlot.getActionContainer();
		local newSlotContainerName = newSlotsActionContainer.getContainerName();

		if (!(newSlotContainerName in this.mMovingToProperties))
		{
			return;
		}

		switch(this.mMovingToProperties[newSlotContainerName].mMovementType)
		{
		case this.MovementTypes.CLONE:
			break;

		case this.MovementTypes.MOVE:
			if (!swapped)
			{
				local slotIndex = this.getIndexOfSlot(oldSlot);

				if (slotIndex >= 0)
				{
					this._removeActionButton(slotIndex);
				}
			}

			break;

		case this.MovementTypes.LOCK:
			local slotIndex = this.getIndexOfSlot(oldSlot);

			if (slotIndex >= 0)
			{
				this._lockSlot(slotIndex);
			}

			break;

		case this.MovementTypes.DESTROY:
			local slotIndex = this.getIndexOfSlot(oldSlot);

			if (slotIndex >= 0)
			{
				this._removeActionButton(slotIndex);
			}

			break;
		}

		if (this.mMovingToProperties[newSlotContainerName].mActionLostCallback && "onActionButtonLost" in this.mMovingToProperties[newSlotContainerName].mActionLostCallback)
		{
			this.mMovingToProperties[newSlotContainerName].mActionLostCallback.onActionButtonLost(newSlot, oldSlot);
		}
	}

	function isIndexLocked( slot )
	{
		if (typeof slot == "integer")
		{
			foreach( lockedIndex in this.mLockedList )
			{
				if (slot == lockedIndex)
				{
					return true;
				}
			}
		}
		else
		{
			foreach( lockedIndex in this.mLockedList )
			{
				if (slot == this.mActionButtonSlotList[lockedIndex])
				{
					return true;
				}
			}
		}

		return false;
	}

	function isContainerFull()
	{
		for( local i = 0; i < this.mActionButtonSlotList.len(); i++ )
		{
			if (null == this.mActionButtonSlotList[i].getActionButton())
			{
				return false;
			}
		}

		return true;
	}

	function isSlotEmpty( index )
	{
		if (index < this.mActionButtonSlotList.len())
		{
			return this.mActionButtonSlotList[index].getActionButton() == null;
		}
	}

	function onDragActionButton( actionButton )
	{
		this.mBroadcaster.broadcastMessage("onActionButtonDragged", this, actionButton);
	}

	function onDropActionButton( actionButton )
	{
		this.mBroadcaster.broadcastMessage("onActionButtonDropped", this, actionButton);
	}

	function onPageBack( evt )
	{
		this.gotoPage(this.mCurrentPageNumber - 1);
	}

	function onPageForward( evt )
	{
		this.gotoPage(this.mCurrentPageNumber + 1);
	}

	function getCurrentPageNumber()
	{
		return this.mCurrentPageNumber;
	}

	function gotoPage( pageNumber )
	{
		this.mCurrentPageNumber = this.Math.clamp(pageNumber, 1, this.mMaxPages);
		this._updateVisibleIcons();
		this._updatePageNumText();
		this.mBroadcaster.broadcastMessage("onPageChanged", this, this.mCurrentPageNumber);
	}

	function removeAllActions( ... )
	{
		local forceUpdateContainer = true;

		if (vargc > 0)
		{
			forceUpdateContainer = vargv[0];
		}

		if (this.mAutoPosition)
		{
			this.mActionButtonList.clear();
		}
		else
		{
			for( local i = 0; i < this.mActionButtonList.len(); i++ )
			{
				this.mActionButtonList[i] = null;
			}
		}

		if (forceUpdateContainer)
		{
			this.updateContainer();
		}

		this.mPagingInfoVisible = false;
	}

	function removeAction( action )
	{
		local index = this._findIndexOfAction(action);

		if (index >= 0)
		{
			this._removeActionButton(index);
			return true;
		}
		else
		{
			return false;
		}
	}

	function removeActionInSlot( slot )
	{
		if (slot >= 0)
		{
			this._removeActionButton(slot);
		}
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function setAllowButtonDisownership( value )
	{
		this.mAllowButtonDisownership = value;
	}

	function setAllButtonsDraggable( value )
	{
		foreach( slot in this.mActionButtonSlotList )
		{
			slot.setDraggable(value);
		}
	}

	function setDelayContainerUpdate( value )
	{
		this.mDelayContainerUpdateOnItemUpdate = value;
	}

	function setCallback( callback )
	{
		this.mCallback = callback;
	}

	function setFrontInsert( value )
	{
		this.mFrontInsert = value;
	}

	function setIconContainerInsets( top, right, bottom, left )
	{
		if (this.mIconContainer)
		{
			this.mIconContainer.setInsets(top, right, bottom, left);
		}
	}

	function setShowBindingInfo( value )
	{
		this.mShowBindingInfo = value;
	}

	function setShowEquipmentComparison( value )
	{
		this.mShowEquipmentComparison = value;
	}

	function setSlotsBackgroundMaterial( index, material, ... )
	{
		local setAsEmptyMat = false;

		if (vargc > 0)
		{
			setAsEmptyMat = vargv[0];
		}

		if (this.mActionButtonSlotList.len() > index)
		{
			this.mActionButtonSlotList[index].setBackgroundMaterial(material, setAsEmptyMat);
		}
	}

	function setSlotDraggable( value, index )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			this.mActionButtonSlotList[index].setDraggable(value);
		}
	}

	function setSlotDraggableReplaceable( index, value )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			this.mActionButtonSlotList[index].setDraggable(value);
			this.mActionButtonSlotList[index].setAcceptDroppedButtons(value);
		}
	}

	function setSlotKeyBinding( slotindex, keybinding )
	{
		if (this.mActionButtonSlotList.len() > slotindex)
		{
			if (keybinding == null)
			{
				this.mActionButtonSlotList[slotindex].setKeybindingVisible(false);
			}
			else
			{
				this.mActionButtonSlotList[slotindex].setKeybinding(keybinding);
				this.mActionButtonSlotList[slotindex].setKeybindingVisible(true);
			}
		}
	}

	function setSlotRestriction( index, restriction )
	{
		if (this.mActionButtonSlotList.len() > index)
		{
			this.mActionButtonSlotList[index].setEquipmentRestriction(restriction);
		}
	}

	function setSlotSelected( slot, value )
	{
		if (!this.mKeepTrackOfClickedIndex)
		{
			return;
		}

		if (!value)
		{
			return;
		}

		local slotIndex = this.getIndexOfSlot(slot);
		this.clearActiveSelection();
		this.mLastClickedIndex = slotIndex;
		slot.setBlendColor(this.Color("37FDFC"));
		local actionButton = slot.getActionButton();

		if (actionButton)
		{
			actionButton.setSelectionVisible(true);
			this.mBroadcaster.broadcastMessage("onNewSelection", this, slot);
		}
	}

	function setSlotUseMode( slotindex, useMode )
	{
		if (this.mActionButtonSlotList.len() > slotindex)
		{
			this.mActionButtonSlotList[slotindex].setUseMode(useMode);
		}
	}

	function setTransparent()
	{
		this.mIconContainer.setAppearance("Container");

		for( local i = 0; i < this.mActionButtonSlotList.len(); i++ )
		{
			this.mActionButtonSlotList[i].mBackground.setAppearance(null);
			this.mActionButtonSlotList[i].setAppearance(null);
			this.mActionButtonSlotList[i].setBackgroundMaterial("ActionButtonOverlay", true);
			this.mActionButtonSlotList[i].setRestoreBackgroundOnEmpty(false);
		}
	}

	function setSlotVisible( slotindex, value )
	{
		if (this.mActionButtonSlotList.len() > slotindex)
		{
			this.mActionButtonSlotList[slotindex].setVisible(value);
		}
	}

	function setTooltipRenderingModifiers( actiontype, modifiers )
	{
		this.mTooltipRenderingModifiers[actiontype] <- modifiers;
	}

	function setHighlightSelectedIndex( value )
	{
		this.mKeepTrackOfClickedIndex = value;
	}

	function setItemPanelVisible( visible, ... )
	{
		this.mItemPanelVisible = visible;

		if (vargc > 0)
		{
			this.mMiniItemPanelVersion = vargv[0];
		}

		this.updateContainer();
	}

	function setMaxPages( maxPages )
	{
		this.mMaxPages = maxPages;
		this.mActionButtonList.resize(this.mRowsVisible * this.mColumnsVisible * this.mMaxPages);
	}

	function setPagingInfoEnabled( value )
	{
		this.mPagingInfoEnabled = value;
	}

	function setUseMode( useMode )
	{
		this.mUseMode = useMode;

		for( local i = 0; i < this.mActionButtonSlotList.len(); i++ )
		{
			this.mActionButtonSlotList[i].setUseMode(useMode);
		}
	}

	function setValidDropContainer( droppable )
	{
		foreach( slot in this.mActionButtonSlotList )
		{
			slot.setAcceptDroppedButtons(droppable);
		}
	}

	function setValidDropSlot( index, droppable )
	{
		if (index < this.mActionButtonSlotList.len())
		{
			this.mActionButtonSlotList[index].setAcceptDroppedButtons(droppable);
		}
	}

	function simulateButtonDrop( actionButton )
	{
		local newSlot;

		if (this.mAutoPosition)
		{
			newSlot = this.mActionButtonSlotList[0];
		}
		else if (!this.isContainerFull())
		{
			local action = actionButton.getAction();
			local slotIndex = this.findSlotIndexOfAction(action);

			if (slotIndex == -1)
			{
				slotIndex = this._findNextFreeIndex();
			}

			newSlot = this.mActionButtonSlotList[slotIndex];
		}
		else
		{
			return;
		}

		local oldSlot = actionButton.getActionButtonSlot();
		local oldActionContainer = oldSlot.getActionContainer();
		local results = this.handleSwapIn(oldSlot, newSlot);

		if (results.dropSucessful == true)
		{
			oldActionContainer.handleSwapOut(oldSlot, newSlot, results.buttonsSwapped);
		}
	}

	function simulateActionButtonSlotDrop( actionButtonSlot, oldActionButtonIndex, actionContainer, slotIndexTriedToDropTo )
	{
		local newSlot;

		if (slotIndexTriedToDropTo < this.mActionButtonSlotList.len())
		{
			newSlot = this.mActionButtonSlotList[slotIndexTriedToDropTo];
		}
		else
		{
			return;
		}

		local oldSlot = actionButtonSlot;
		local oldActionButton = oldSlot.getActionButton();

		if (oldActionButton)
		{
			local oldActionContainer = oldSlot.getActionContainer();
			local results = this.handleSwapIn(oldSlot, newSlot);

			if (results.dropSucessful == true)
			{
				oldActionContainer.handleSwapOut(oldSlot, newSlot, results.buttonsSwapped);
			}
		}
		else if (actionContainer)
		{
			local movedActionButtonSlot = this.getSlotContents(slotIndexTriedToDropTo);

			if (movedActionButtonSlot)
			{
				local movedActionButton = movedActionButtonSlot.getActionButton();

				if (movedActionButton)
				{
					local action = movedActionButton.getAction();

					if (action)
					{
						actionContainer.addAction(action, true, oldActionButtonIndex);
					}

					this.removeActionInSlot(slotIndexTriedToDropTo);
				}
			}
		}
	}

	function shouldCopyAction( slot )
	{
		local slotContainerName = slot.getActionContainer().getContainerName();

		if (slotContainerName in this.mMovingToProperties)
		{
			return this.mMovingToProperties[slotContainerName].mMovementType != this.MovementTypes.DESTROY;
		}
	}

	function slotHasButton( slotindex )
	{
		if (slotindex < this.mActionButtonSlotList.len())
		{
			return this.mActionButtonSlotList[slotindex].getActionButton() != null;
		}

		return false;
	}

	function unlockAllSlots()
	{
		for( local i = 0; i < this.mLockedList.len(); i++ )
		{
			local slot = this.mActionButtonSlotList[this.mLockedList[i]];
			slot.setDraggable(true);
			slot.setSlotDisabled(false);
			slot.setUseMode(this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
		}

		this.mLockedList.clear();
	}

	function updateContainer()
	{
		this._updateMaxPages();

		if (this.mMaxPages > 1)
		{
			if (this.mPagingInfoEnabled)
			{
				if (!this.mPagingInfoVisible)
				{
					this.mPagingInfoVisible = true;
					this.remove(this.mPageContainer);
					this.mPageContainer = this.GUI.Container(this.GUI.GridLayout(2, 1));
					local iconContainerSize = this.mIconContainer.getSize();

					if (("itemProto" in this.mTooltipRenderingModifiers) && ("resizeInfoPanel" in this.mTooltipRenderingModifiers.itemProto) && this.mTooltipRenderingModifiers.itemProto.resizeInfoPanel == false)
					{
						this.mPageContainer.getLayoutManager().setRows(iconContainerSize.height, 25);
						this.mPageInfo.setSize(iconContainerSize.width, 25);
						this.mPageInfo.setPreferredSize(iconContainerSize.width, 25);
						local pageInfoSize = this.mPageInfo.getSize();
						this.mPageContainer.setSize(iconContainerSize.width, pageInfoSize.height + iconContainerSize.height);
					}
					else
					{
						this.mPageContainer.getLayoutManager().setRows("*", 25);
					}

					this.mPageContainer.add(this.mIconContainer);
					this.mPageContainer.add(this.mPageInfo);
					this.add(this.mPageContainer);

					if (this.mShowDebuggingButtons)
					{
						this.mPageContainer.add(this.mTempAdd);
						this.mPageContainer.add(this.mTempRemove);
					}

					this.mPageLabel.setVisible(true);
					this.mPageBack.setVisible(true);
					this.mPageForward.setVisible(true);
				}
			}

			this._updatePageNumText();
		}
		else
		{
			this.mCurrentPageNumber = this.mMaxPages;

			if (this.mPagingInfoVisible)
			{
				this.remove(this.mPageContainer);
				this.mPageContainer = this.GUI.Component(this.GUI.BoxLayoutV(1, 1));
				local ipz = this.mIconContainer.getPreferredSize();
				local ppz = this.mPageInfo.getPreferredSize();

				if (("itemProto" in this.mTooltipRenderingModifiers) && ("resizeInfoPanel" in this.mTooltipRenderingModifiers.itemProto) && this.mTooltipRenderingModifiers.itemProto.resizeInfoPanel == false)
				{
				}
				else
				{
					this.mIconContainer.setPreferredSize(ipz.width, ipz.height + ppz.height);
				}

				this.mPageContainer.add(this.mIconContainer);
				this.add(this.mPageContainer);
			}
		}

		this._updateVisibleIcons();
	}

	function _delayedUpdateContainer( container )
	{
		this.mDelayContainerUpdateEvent = null;
		this.updateContents(container);
	}

	function _findNextFreeIndex()
	{
		local freeIndex = -1;
		local i = 0;

		for( i = 0; i < this.mActionButtonList.len(); i++ )
		{
			if (this.mActionButtonList[i] == null)
			{
				return i;
			}
		}

		this.mActionButtonList.append(null);
		return i;
	}

	function _findIndexOfAction( action )
	{
		for( local i = 0; i < this.mActionButtonList.len(); i++ )
		{
			if (this.mActionButtonList[i] && action == this.mActionButtonList[i].getAction())
			{
				return i;
			}
		}

		return -1;
	}

	function _lockSlot( index )
	{
		this.mActionButtonSlotList[index].setSlotDisabled(true);
		this.mActionButtonSlotList[index].setUseMode(this.GUI.ActionButtonSlot.USE_NONE);
		this.mActionButtonSlotList[index].setDraggable(false);
		this.mLockedList.append(index);
	}

	function _removeActionButton( index )
	{
		if (index > this.mActionButtonList.len() - 1)
		{
			return;
		}

		if (this.mAutoPosition == true)
		{
			this.mActionButtonList.remove(index);
		}
		else
		{
			this.mActionButtonList[index] = null;
		}

		this._updateMaxPages();

		if (this.mMaxPages == 1)
		{
			if (this.mPagingInfoVisible)
			{
				this.mPagingInfoVisible = false;
				this.remove(this.mPageContainer);
				this.mPageContainer = this.GUI.Component(this.GUI.BoxLayoutV(1, 1));
				this.mPageContainer.add(this.mIconContainer);

				if (this.mShowDebuggingButtons)
				{
					this.mPageContainer.add(this.mTempAdd);
					this.mPageContainer.add(this.mTempRemove);
				}

				this.add(this.mPageContainer);
				this.mPageLabel.setVisible(true);
				this.mPageBack.setVisible(true);
				this.mPageForward.setVisible(true);
			}
		}

		if (this.mCurrentPageNumber > this.mMaxPages)
		{
			this.mCurrentPageNumber = this.mMaxPages;
		}

		this._updatePageNumText();
		this._updateVisibleIcons();
	}

	function _onIconRemove( evt )
	{
		this._removeActionButton(0);
	}

	function _onActionAdd( evt )
	{
		local action = ::_AbilityManager.getAbilityByName("Red Potion");
		this.addAction(action, true);
	}

	function _updateMaxPages()
	{
		local lastMaxPages = this.mMaxPages;
		this.mMaxPages = (this.mActionButtonList.len().tofloat() / (this.mRowsVisible * this.mColumnsVisible) + 0.99000001).tointeger();

		if (lastMaxPages != this.mMaxPages && this.mAutoPosition == false)
		{
			this.mActionButtonList.resize(this.mRowsVisible * this.mColumnsVisible * this.mMaxPages);
		}

		if (this.mMaxPages == 0)
		{
			this.mMaxPages = 1;
		}
	}

	function _unlockSlot( slotToRemove )
	{
		for( local i = 0; i < this.mLockedList.len(); i++ )
		{
			local slot = this.mActionButtonSlotList[this.mLockedList[i]];

			if (slotToRemove == slot)
			{
				slotToRemove.setDraggable(true);
				slotToRemove.setSlotDisabled(false);
				slotToRemove.setUseMode(this.GUI.ActionButtonSlot.USE_LEFT_CLICK);
				this.mLockedList.remove(i);
				return;
			}
		}
	}

	function _updatePageNumText()
	{
		this.mPageLabel.setText("Page " + this.mCurrentPageNumber + " / " + this.mMaxPages);
		this.mPageLabel.setTextAlignment(0.5, 0.5);
	}

	function _updateVisibleIcons()
	{
		foreach( actionButtonSlot in this.mActionButtonSlotList )
		{
			actionButtonSlot.removeActionButton();
			actionButtonSlot.setInfoPanelState(this.mItemPanelVisible, this.mMiniItemPanelVersion);
		}

		local start = 0;

		if (this.mCurrentPageNumber - 1 > 0)
		{
			start = this.mPerPage * (this.mCurrentPageNumber - 1);
		}

		local end = start + this.mPerPage;
		local slotindex = 0;

		for( local i = start; i < this.mActionButtonList.len() && i < end; i++ )
		{
			local add = true;
			local amtToAddToSlotIndex = 0;

			if (this.mCallback && "shouldActionButtonBeAdded" in this.mCallback)
			{
				if (this.mActionButtonList[i])
				{
					add = this.mCallback.shouldActionButtonBeAdded(this, this.mActionButtonSlotList[slotindex], this.mActionButtonList[i], this.mActionButtonList[i].getAction());
				}
			}

			if (this.mActionButtonList[i] && add)
			{
				this.mActionButtonSlotList[slotindex].setActionButton(this.mActionButtonList[i]);
				this.mActionButtonSlotList[slotindex].setInfoPanelState(this.mItemPanelVisible, this.mMiniItemPanelVersion);

				if (this.mAutoPosition)
				{
					amtToAddToSlotIndex = 1;
				}
			}

			if (this.mCallback && "onActionButtonAdded" in this.mCallback)
			{
				if (this.mActionButtonList[i] && slotindex < this.mActionButtonSlotList.len())
				{
					add = this.mCallback.onActionButtonAdded(this, this.mActionButtonSlotList[slotindex], this.mActionButtonList[i], this.mActionButtonList[i].getAction());
				}
			}

			if (!this.mAutoPosition)
			{
				slotindex++;
			}

			slotindex += amtToAddToSlotIndex;
		}
	}

}

class this.GUI.InventoryActionContainer extends this.GUI.ActionContainer
{
	mCreatureId = null;
	mServerContainerName = null;
	constructor( name, rows, cols, iconSpacingX, iconSpacingY, owner, ... )
	{
		if (vargc >= 3)
		{
			this.GUI.ActionContainer.constructor(name, rows, cols, iconSpacingX, iconSpacingY, owner, vargv[2]);
		}
		else
		{
			this.GUI.ActionContainer.constructor(name, rows, cols, iconSpacingX, iconSpacingY, owner);
		}

		if (vargc >= 2)
		{
			this.setSourceContainer(vargv[0], vargv[1]);
		}

		::_ItemDataManager.addListener(this);
	}

	function destroy()
	{
		::_ItemDataManager.removeListener(this);
	}

	function setSourceContainer( creatureId, containerName )
	{
		this.mCreatureId = creatureId;
		this.mServerContainerName = containerName;
		local container = ::_ItemDataManager.getContents(containerName, creatureId);
		this.updateContents(container);
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (creatureId == this.mCreatureId && containerName == this.mServerContainerName)
		{
			this.updateContents(container);
		}
	}

	function onItemUpdated( itemId, item, itemdef, itemlookdef )
	{
		if (this.mServerContainerName && this.mCreatureId)
		{
			local container = ::_ItemDataManager.getContents(this.mServerContainerName, this.mCreatureId, false);

			if (container.containsItem(itemId))
			{
				if (this.mDelayContainerUpdateOnItemUpdate)
				{
					if (this.mDelayContainerUpdateEvent != null)
					{
						::_eventScheduler.cancel(this.mDelayContainerUpdateEvent);
					}

					this.mDelayContainerUpdateEvent = ::_eventScheduler.fireIn(1.5, this, "_delayedUpdateContainer", container);
				}
				else
				{
					this.updateContents(container);
				}
			}
		}
	}

	function updateContents( container )
	{
		if (this.mAutoPosition)
		{
			for( local i = 0; i < this.mActionButtonList.len();  )
			{
				local actionbutton = this.mActionButtonList[i];

				if (actionbutton != null)
				{
					local action = actionbutton.getAction();
					local itemId = action.getItemId();

					if (!container.containsItem(itemId))
					{
						this._removeActionButton(i);
					}
					else
					{
						i++;
					}
				}
				else
				{
					i++;
				}
			}
		}
		else
		{
			for( local i = 0; i < this.mActionButtonList.len(); i++ )
			{
				local actionbutton = this.mActionButtonList[i];

				if (actionbutton != null)
				{
					local action = actionbutton.getAction();
					local itemId = action.getItemId();

					if (!container.containsItem(itemId))
					{
						this._removeActionButton(i);
					}
					else
					{
						local item = ::_ItemDataManager.getItem(itemId);

						if (item.mContainerSlot != i)
						{
							this._removeActionButton(i);
						}
					}
				}
			}
		}

		foreach( itemId in container.mContents )
		{
			local found = false;

			foreach( button in this.mActionButtonList )
			{
				if (button != null)
				{
					local action = button.getAction();
					local itemIdTest = action.getItemId();

					if (itemIdTest == itemId)
					{
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				local itemAction = ::_ItemManager.getItem(itemId);

				if (this.mAutoPosition)
				{
					this.addAction(itemAction, false);
				}
				else
				{
					local item = ::_ItemDataManager.getItem(itemId);
					this.addAction(itemAction, false, item.mContainerSlot);
				}
			}
		}

		this.GUI.ActionContainer.updateContainer();
	}

}

class this.GUI.InventoryRestrictedActionContainer extends this.GUI.ActionContainer
{
	mValidContainerIds = null;
	constructor( name, rows, cols, iconSpacingX, iconSpacingY, autoPosition, ... )
	{
		this.mValidContainerIds = [];
		this.GUI.ActionContainer.constructor(name, rows, cols, iconSpacingX, iconSpacingY, autoPosition);

		if (vargc >= 2)
		{
			foreach( container in vargv[1] )
			{
				this.addPermittableSourceContainer(vargv[0], container);
			}
		}
	}

	function addPermittableSourceContainer( creatureId, containerName )
	{
		::_Connection.sendQuery("item.contents", this, [
			containerName,
			creatureId
		]);
	}

	function clearPermittableSourceContainers()
	{
		this.mValidContainerIds = [];
	}

	function getActionButtonValid( slot, button )
	{
		local action = button.getAction();

		if (action.getType() == "item")
		{
			local itemId = action.getItemId();
			local itemdata = ::_ItemDataManager.getItem(itemId);

			foreach( container in this.mValidContainerIds )
			{
				if (itemdata.mContainerItemId == container)
				{
					return true;
				}
			}
		}

		return false;
	}

	static function onQueryComplete( qa, results )
	{
		if (qa.query == "item.contents")
		{
			this.mValidContainerIds.append(results[0][0]);
		}
	}

}

class this.MoveToProperties 
{
	mMovementType = this.MovementTypes.CLONE;
	mActionLostCallback = null;
	constructor( movementType, ... )
	{
		this.mMovementType = movementType;

		if (vargc > 0)
		{
			this.mActionLostCallback = vargv[0];
		}
	}

}

class this.AcceptFromProperties 
{
	mIsValidDropSlotCallback = null;
	mActionGainedCallback = null;
	mUnlockWhenDropped = false;
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.mIsValidDropSlotCallback = vargv[0];
		}

		if (vargc > 1)
		{
			this.mActionGainedCallback = vargv[1];
		}

		if (vargc > 2)
		{
			this.mUnlockWhenDropped = vargv[2];
		}
	}

}

