this.require("UI/ActionContainer");
this.require("UI/Equipment");
this.require("UI/Screens");
class this.Screens.Inventory extends this.GUI.ContainerFrame
{
	static mClassName = "Screens.Inventory";
	mScreenContainer = null;
	mInventoryContainer = null;
	mInventoryContainerHolder = null;
	mBagContainer = null;
	mBagContainerHolder = null;
	mColumns = 8;
	mMaxBagSlots = 4;
	mRows = 0;
	mBaseInventorySize = 24;
	mCurrentInventorySize = 24;
	mDisabledSlotMaterial0 = "BG/ActionHolderPlug_64";
	mEnabledSlotMaterial0 = "BG/ActionHolder1_64";
	mEnabledSlotMaterial1 = "BG/ActionHolder2_64";
	mEnabledSlotMaterial2 = "BG/ActionHolder3_64";
	mDebuggingMode = false;
	mResize = null;
	COST_OF_BAG = 1475;
	constructor( ... )
	{
		local slots = vargc >= 1 ? vargv[0] : this.mBaseInventorySize;
		this.GUI.ContainerFrame.constructor("Inventory");
		this.mBagContainerHolder = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mBagContainerHolder.setAppearance("Inventory_BagHolder");
		this.mBagContainer = this.GUI.ActionContainer("bag_inventory", 1, this.mMaxBagSlots, 0, 0, this, false);
		this.mBagContainer.addListener(this);
		this.mBagContainer.setSlotsBackgroundMaterial(0, "EQ_Storage_64", true);
		this.mBagContainer.setSlotsBackgroundMaterial(1, "EQ_Storage_64", true);
		this.mBagContainer.setSlotsBackgroundMaterial(2, "EQ_Storage_64", true);
		this.mBagContainer.setSlotsBackgroundMaterial(3, "EQ_Storage_64", true);
		this.mBagContainer.setSlotRestriction(0, this.ItemEquipSlot.CONTAINER_0);
		this.mBagContainer.setSlotRestriction(1, this.ItemEquipSlot.CONTAINER_1);
		this.mBagContainer.setSlotRestriction(2, this.ItemEquipSlot.CONTAINER_2);
		this.mBagContainer.setSlotRestriction(3, this.ItemEquipSlot.CONTAINER_3);
		this.mBagContainer.setCallback(this);
		this.mBagContainer.setValidDropContainer(true);
		this.mBagContainer.setShowEquipmentComparison(false);
		this.mBagContainerHolder.add(this.mBagContainer);
		local preferredSize = this.mBagContainer.getPreferredSize();
		this.mBagContainerHolder.setSize(preferredSize.width + 50, preferredSize.height + 4);
		this.mBagContainerHolder.setPreferredSize(preferredSize.width + 50, preferredSize.height + 4);
		this.setPosition(400, 50);
		::_ItemDataManager.addListener(this);
		this.mScreenContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mScreenContainer.add(this.mBagContainerHolder);
		this.mScreenContainer.getLayoutManager().setGap(0);
		this.setContentPane(this.mScreenContainer);
		this.setSize(this.getPreferredSize());
		this.resizeInventory(slots);
		this.refreshInventory();
		::_Connection.addListener(this);
		this.setCached(::Pref.get("video.UICache"));
	}

	function close()
	{
		this.setVisible(false);
	}

	function disableSlot( slot )
	{
		if (this.mInventoryContainer)
		{
			this.mInventoryContainer.setSlotsBackgroundMaterial(slot, this.mDisabledSlotMaterial0, true);
			this.mInventoryContainer.setValidDropSlot(slot, false);
		}
	}

	function enableSlot( slot )
	{
		if (this.mInventoryContainer)
		{
			local textureNum = slot % 3;
			local texture;

			if (textureNum == 0)
			{
				texture = this.mEnabledSlotMaterial0;
			}
			else if (textureNum == 1)
			{
				texture = this.mEnabledSlotMaterial1;
			}
			else
			{
				texture = this.mEnabledSlotMaterial2;
			}

			this.mInventoryContainer.setSlotsBackgroundMaterial(slot, texture, true);
			this.mInventoryContainer.setValidDropSlot(slot, true);
		}
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function getFreeSlotsRemaining()
	{
		return this.mCurrentInventorySize - this.mInventoryContainer.getAllActionButtons().len();
	}

	function getMovable()
	{
		return this.mMovable;
	}

	function getMyActionContainer()
	{
		return this.mInventoryContainer;
	}

	function getBagActionContainer()
	{
		return this.mBagContainer;
	}

	function onActionButtonAdded( container, actionbuttonslot, actionbutton, action )
	{
		local disabled = false;

		if (container == this.mInventoryContainer)
		{
			local itemdef = action._getItemDef();
			local profession = this.Professions[::_avatar.getStat(this.Stat.PROFESSION)].name;
			local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);

			if (itemdef && itemdef.getType() == this.ItemType.BASIC)
			{
				disabled = false;
			}
			else
			{
				local classrestrictions = itemdef.getClassRestrictions();

				if (avatarlevel < itemdef.getMinUseLevel())
				{
					disabled = true;
				}

				if (!classrestrictions[profession.tolower()])
				{
					disabled = true;
				}
			}
		}

		actionbuttonslot.setSlotDisabled(disabled);
		return true;
	}

	function onActionButtonDragged( container, actionButton )
	{
		local equipScreen = this.Screens.get("Equipment", false);

		if (equipScreen)
		{
			equipScreen.highlightSlots(actionButton);
		}
	}

	function onActionButtonDropped( actionContainer, actionButton )
	{
		local equipScreen = this.Screens.get("Equipment", false);

		if (equipScreen)
		{
			equipScreen.handleClearHighlight();
		}
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (!::_avatar)
		{
			return;
		}

		local proff = ::_avatar.getStat(this.Stat.PROFESSION);

		if (proff == null)
		{
			return;
		}

		local profession = this.Professions[::_avatar.getStat(this.Stat.PROFESSION)].name;
		local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);
		local visibleItemCount = this.mInventoryContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = this.mInventoryContainer.getSlotContents(i);

			if (actionButtonSlot)
			{
				local actionButton = actionButtonSlot.getActionButton();

				if (actionButton)
				{
					local action = actionButton.getAction();
					local disabled = false;
					local itemdef = action._getItemDef();

					if (itemdef == itemdef)
					{
						if (itemdef && itemdef.getType() == this.ItemType.BASIC)
						{
							disabled = false;
						}
						else
						{
							local classrestrictions = itemdef.getClassRestrictions();

							if (avatarlevel < itemdef.getMinUseLevel())
							{
								disabled = true;
							}

							if (!classrestrictions[profession.tolower()])
							{
								disabled = true;
							}
						}

						actionButtonSlot.setSlotDisabled(disabled);
					}
				}
			}
		}
	}

	function onAttemptedItemDisown( actionContainer, action, slotIndex )
	{
		if (actionContainer == this.mInventoryContainer)
		{
			local callback = {
				currAction = action,
				slotNum = slotIndex,
				currActionContainer = this.mInventoryContainer,
				function onActionSelected( mb, alt )
				{
					if (alt == "Yes")
					{
						if (this._Connection)
						{
							this._Connection.sendQuery("item.delete", this, [
								this.currAction.getItemId()
							]);
							this.currActionContainer._removeActionButton(this.slotNum);
							this._quickBarManager.removeItemFromQuickbar(this.currAction);
							local craftWindow = ::Screens.get("CraftingWindow", false);

							if (craftWindow)
							{
								craftWindow.onCraftReceipeRemove(this.currAction);
							}
						}
					}
				}

			};
			local def = action._getItemDef();

			if (def.getType() == this.ItemType.QUEST)
			{
				this.GUI.MessageBox.showYesNo("This item is used in the quest <font color=\"red\"><b>\"" + def.getQuestName() + "\"</b></font>.<br />Are you sure you want to destroy it?", callback);
			}
			else
			{
				this.GUI.MessageBox.showYesNo("This item will be destroyed.  Are you sure you want to destroy it? ", callback);
			}
		}
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (!container.hasAllItems())
		{
			return;
		}

		if (creatureId == ::_avatar.getID())
		{
			if (containerName == "inv")
			{
				if (this.mInventoryContainer)
				{
					this.mInventoryContainer.removeAllActions();

					foreach( itemId in container.mContents )
					{
						local item = ::_ItemManager.getItem(itemId);
						this.mInventoryContainer.addAction(item, false, item.mItemData.mContainerSlot);
					}

					this.mInventoryContainer.updateContainer();
				}
			}
			else if (containerName == "eq")
			{
				if (this.mBagContainer)
				{
					this.mBagContainer.removeAllActions();
					local newInventorySize = this.mBaseInventorySize;

					foreach( itemId in container.mContents )
					{
						local item = ::_ItemManager.getItem(itemId);
						local slot = -1;

						switch(item.mItemData.mContainerSlot)
						{
						case this.ItemEquipSlot.CONTAINER_0:
							slot = 0;
							break;

						case this.ItemEquipSlot.CONTAINER_1:
							slot = 1;
							break;

						case this.ItemEquipSlot.CONTAINER_2:
							slot = 2;
							break;

						case this.ItemEquipSlot.CONTAINER_3:
							slot = 3;
							break;
						}

						if (slot >= 0 && slot <= 3)
						{
							this.mBagContainer.addAction(item, false, slot);
							newInventorySize += item.mItemDefData.mContainerSlots;
						}
					}

					if (newInventorySize != this.mCurrentInventorySize)
					{
						this.resizeInventory(newInventorySize);
					}

					this.mBagContainer.updateContainer();
				}
			}
		}
	}

	function onClosePressed()
	{
		this.close();
	}

	function onItemMovedInContainer( container, slotIndex, oldSlotsButton )
	{
		local item = container.getSlotContents(slotIndex);
		local itemID = item.getActionButton().getAction().mItemId;
		local queryArgument = [];
		local previousSlotContainer = oldSlotsButton.getPreviousActionContainer();
		local oldActionButtonSlot = oldSlotsButton.getActionButtonSlot();
		local oldSlotContainerName = "";
		local oldSlotIndex = 0;

		if (previousSlotContainer && oldActionButtonSlot)
		{
			oldSlotContainerName = previousSlotContainer.getContainerName();
			oldSlotIndex = previousSlotContainer.getIndexOfSlot(oldActionButtonSlot);
		}

		if (container == this.mInventoryContainer)
		{
			if (item.getSwapped() == true)
			{
				item.setSwapped(false);
			}
			else
			{
				queryArgument.append(itemID);
				queryArgument.append("inv");
				queryArgument.append(slotIndex);

				if (::_Connection.getProtocolVersionId() >= 19)
				{
					queryArgument.append(oldSlotContainerName);
					queryArgument.append(oldSlotIndex);
				}

				this._Connection.sendQuery("item.move", this, queryArgument);
			}

			this.onActionButtonDropped(null, null);
		}
		else if (container == this.mBagContainer)
		{
			local serverIndex = 0;

			switch(slotIndex)
			{
			case 0:
				serverIndex = this.ItemEquipSlot.CONTAINER_0;
				break;

			case 1:
				serverIndex = this.ItemEquipSlot.CONTAINER_1;
				break;

			case 2:
				serverIndex = this.ItemEquipSlot.CONTAINER_2;
				break;

			case 3:
				serverIndex = this.ItemEquipSlot.CONTAINER_3;
				break;

			default:
				this.log.debug("Invalid slot specified");
				return;
			}

			queryArgument.append(itemID);
			queryArgument.append("eq");
			queryArgument.append(serverIndex);

			if (::_Connection.getProtocolVersionId() >= 19)
			{
				queryArgument.append(oldSlotContainerName);
				queryArgument.append(oldSlotIndex);
			}

			this._Connection.sendQuery("item.move", this, queryArgument);
			this.onActionButtonDropped(null, null);
		}
	}

	function onLevelUpdate( level )
	{
		if (!::_avatar)
		{
			return;
		}

		local proff = ::_avatar.getStat(this.Stat.PROFESSION);

		if (proff == null)
		{
			return;
		}

		local profession = this.Professions[proff].name;
		local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);
		local visibleItemCount = this.mInventoryContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = this.mInventoryContainer.getSlotContents(i);

			if (actionButtonSlot)
			{
				local actionButton = actionButtonSlot.getActionButton();

				if (actionButton)
				{
					local action = actionButton.getAction();
					local disabled = false;
					local itemdef = action._getItemDef();

					if (itemdef && itemdef.getType() == this.ItemType.BASIC)
					{
						disabled = false;
					}
					else
					{
						local classrestrictions = itemdef.getClassRestrictions();

						if (avatarlevel < itemdef.getMinUseLevel())
						{
							disabled = true;
						}

						if (!classrestrictions[profession.tolower()])
						{
							disabled = true;
						}
					}

					actionButtonSlot.setSlotDisabled(disabled);
				}
			}
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "item.move")
		{
		}
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);

		if (qa.args.len() < 5)
		{
			return;
		}

		if (qa.query == "item.move")
		{
			::Util.handleMoveItemBack(qa);
		}
		else
		{
		}
	}

	function onRightButtonReleased( actionButton, evt )
	{
		local retValue = true;
		local itemShopScreen = ::Screens.get("ItemShop", false);

		if (itemShopScreen && itemShopScreen.isVisible() && this.Key.isDown(this.Key.VK_CONTROL))
		{
			itemShopScreen.onSellItem(actionButton.getAction());
			evt.consume();
			return false;
		}

		if (this.Key.isDown(this.Key.VK_CONTROL))
		{
			local vault = this.Screens.get("Vault", false);

			if (vault && vault.isVisible())
			{
				local freeslots = vault.getFreeSlotsRemaining();

				if (freeslots > 0)
				{
					local vaultAC = vault.getActionContainer();
					vaultAC.simulateButtonDrop(actionButton);
					retValue = false;
				}
			}
			else
			{
				retValue = this._handleRightClickEquip(actionButton, evt);
			}
		}

		return retValue;
	}

	function handleAutoEquip( item, serverIndex, oldSlotContainerName, oldSlotIndex )
	{
		local queryArgument = [];
		queryArgument.append(item.getItemId());
		queryArgument.append("eq");
		queryArgument.append(serverIndex);

		if (::_Connection.getProtocolVersionId() >= 19)
		{
			queryArgument.append(oldSlotContainerName);
			queryArgument.append(oldSlotIndex);
		}

		if ("mItemDefData" in item)
		{
			local itemDefData = item.mItemDefData;
			local weaponType = itemDefData.getWeaponType();

			if (weaponType == this.WeaponType.TWO_HAND)
			{
				local equipment = this.Screens.get("Equipment", true);

				if (equipment)
				{
					local eqOffhand = equipment.findMatchingContainerGivenName("eq_off_hand");

					if (eqOffhand)
					{
						local offhandButton = eqOffhand.getActionButtonFromIndex(0);

						if (offhandButton != null)
						{
							this.mInventoryContainer.simulateButtonDrop(offhandButton);
						}
					}
				}
			}
		}

		this._Connection.sendQuery("item.move", this, queryArgument);
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local oldActionContainer = oldSlot.getActionContainer();

		if (!oldActionContainer)
		{
			return false;
		}

		local newActionContainer = newSlot.getActionContainer();

		if (!newActionContainer)
		{
			return false;
		}

		if (::_avatar && ::_avatar.isDead())
		{
			this.IGIS.error("You cannot move items when you are dead.");
			return false;
		}

		local shopScreen = this.Screens.get("ItemShop", false);

		if (shopScreen && shopScreen.isVisible() && shopScreen.isShopOrBuybackContainer(oldActionContainer))
		{
			return shopScreen.onValidDropSlot(newSlot, oldSlot);
		}

		if (oldActionContainer == this.mBagContainer && newActionContainer == this.mInventoryContainer)
		{
			local oldSlotsActionButton = oldSlot.getActionButton();

			if (!oldSlotsActionButton)
			{
				return;
			}

			local oldSlotsAction = oldSlotsActionButton.getAction();

			if (!oldSlotsAction)
			{
				return;
			}

			local numSlotsToRemove = oldSlotsAction.mItemDefData.mContainerSlots;
			local newSlotsActionButton = newSlot.getActionButton();

			if (newSlotsActionButton)
			{
				local newSlotsAction = newSlotsActionButton.getAction();

				if (newSlotsAction)
				{
					if (newSlotsAction.mItemDefData.mContainerSlots > oldSlotsAction.mItemDefData.mContainerSlots)
					{
						return true;
					}
					else
					{
						numSlotsToRemove = oldSlotsAction.mItemDefData.mContainerSlots - newSlotsAction.mItemDefData.mContainerSlots;
					}
				}
			}

			local newSlotIndex = newActionContainer.getIndexOfSlot(newSlot);

			if (newSlotIndex >= this.mCurrentInventorySize - numSlotsToRemove)
			{
				this.IGIS.error("You cannot place your bag in a slot that will be unavailable when removing this bag.");
				return false;
			}

			for( local i = this.mCurrentInventorySize - numSlotsToRemove; i < this.mCurrentInventorySize; i++ )
			{
				local empty = this.mInventoryContainer.isSlotEmpty(i);

				if (!empty)
				{
					this.IGIS.error("You cannot reduce the capacity of the inventory because there are items occupying those slots.");
					return false;
				}
			}

			return true;
		}
		else if (newActionContainer == this.mBagContainer)
		{
			if (oldActionContainer == this.mBagContainer)
			{
				return true;
			}
			else if (oldActionContainer == this.mInventoryContainer)
			{
				local oldSlotsActionButton = oldSlot.getActionButton();

				if (!oldSlotsActionButton)
				{
					return;
				}

				local oldSlotsAction = oldSlotsActionButton.getAction();

				if (!oldSlotsAction)
				{
					return;
				}

				local newSlotsActionButton = newSlot.getActionButton();

				if (!newSlotsActionButton)
				{
					return true;
				}

				local newSlotsAction = newSlotsActionButton.getAction();

				if (!newSlotsAction)
				{
					return true;
				}

				if (oldSlotsAction.mItemDefData.mContainerSlots >= newSlotsAction.mItemDefData.mContainerSlots)
				{
					return true;
				}
				else
				{
					local numSlotsToRemove = newSlotsAction.mItemDefData.mContainerSlots - oldSlotsAction.mItemDefData.mContainerSlots;

					for( local i = this.mCurrentInventorySize - numSlotsToRemove; i < this.mCurrentInventorySize; i++ )
					{
						local empty = this.mInventoryContainer.isSlotEmpty(i);

						if (!empty)
						{
							this.IGIS.error("You cannot reduce the capacity of the inventory because there are items occupying those slots.");
							return false;
						}
					}
				}
			}
		}
	}

	function refreshInventory()
	{
		local inventory = ::_ItemDataManager.getContents("inv");
		local bags = ::_ItemDataManager.getContents("eq");
		this.onContainerUpdated("inv", ::_avatar.getID(), inventory);
		this.onContainerUpdated("eq", ::_avatar.getID(), bags);
	}

	function resizeInventory( slots )
	{
		local MAX_SLOTS = 112;
		this.mCurrentInventorySize = slots;

		for( local i = this.mCurrentInventorySize - 8; i < this.mCurrentInventorySize; i++ )
		{
			this.enableSlot(i);
		}

		this.mRows = (slots.tofloat() / this.mColumns.tofloat() + 0.99000001).tointeger();

		if (this.mInventoryContainer)
		{
			this.mScreenContainer.remove(this.mInventoryContainerHolder);
			this.mScreenContainer.remove(this.mBagContainerHolder);
		}

		this.mInventoryContainerHolder = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mInventoryContainerHolder.setAppearance("Inventory");
		this.mInventoryContainer = this.GUI.ActionContainer("inventory", this.mRows, this.mColumns, 0, 0, this, false);
		this.mInventoryContainer.setUseMode(this.GUI.ActionButtonSlot.USE_LEFT_DOUBLE_CLICK);
		this.mInventoryContainer.setCallback(this);
		this.mInventoryContainer.setAllowButtonDisownership(false);
		local showMoreBagButton = false;
		local extraHeight = 0;
		this.mCurrentInventorySize <= MAX_SLOTS;
		  // [078]  OP_JMP            5      4    0    0
		showMoreBagButton = true;
		local size = this.mInventoryContainer.getPreferredSize();
		this.mInventoryContainerHolder.setSize(size.width + 4, size.height + 3 + extraHeight);
		this.mInventoryContainerHolder.setPreferredSize(size.width + 4, size.height + 3 + extraHeight);
		this.setContainerMoveProperties();
		this.mInventoryContainer.addListener(this);
		this.mInventoryContainer.setItemPanelVisible(false);
		this.mInventoryContainer.setValidDropContainer(true);
		local totalSlots = this.mRows * this.mColumns;
		local unavailableSlots = totalSlots - this.mCurrentInventorySize;

		for( local i = 0; i < totalSlots; i++ )
		{
			this.enableSlot(i);
		}

		for( local i = totalSlots - unavailableSlots; i < totalSlots; i++ )
		{
			this.disableSlot(i);
		}

		local inventory = ::_ItemDataManager.getContents("inv");
		this.onContainerUpdated("inv", ::_avatar.getID(), inventory);
		this.mInventoryContainerHolder.add(this.mInventoryContainer);
		
		this.mScreenContainer.add(this.mInventoryContainerHolder);
		this.mScreenContainer.add(this.mBagContainerHolder);
		this.setSize(this.getPreferredSize());
	}

	function onCreditUpdated( value )
	{
	}

	function setContainerMoveProperties()
	{
		this.mInventoryContainer.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE));

		foreach( name in this.EquipmentContainerNames )
		{
			this.mInventoryContainer.addMovingToProperties(name, this.MoveToProperties(this.MovementTypes.MOVE));
		}

		this.mInventoryContainer.addMovingToProperties("trade_avatar", this.MoveToProperties(this.MovementTypes.LOCK));
		this.mInventoryContainer.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mInventoryContainer.addMovingToProperties("creaturetweak_inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addMovingToProperties("bag_inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addMovingToProperties("morph_stats", this.MoveToProperties(this.MovementTypes.LOCK));
		this.mInventoryContainer.addMovingToProperties("morph_look", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mInventoryContainer.addMovingToProperties("crafting_list_container", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mInventoryContainer.addMovingToProperties("vault", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addMovingToProperties("stamps", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addMovingToProperties("delivery", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addMovingToProperties("market_edit", this.MoveToProperties(this.MovementTypes.CLONE));
		this.mBagContainer.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mBagContainer.addMovingToProperties("bag_inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mInventoryContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties());
		this.mInventoryContainer.addAcceptingFromProperties("creaturetweak_inventory", this.AcceptFromProperties());

		foreach( name in this.EquipmentContainerNames )
		{
			this.mInventoryContainer.addAcceptingFromProperties(name, this.AcceptFromProperties());
		}

		this.mInventoryContainer.addAcceptingFromProperties("item_shop", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("buyback_shop", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("trade_avatar", this.AcceptFromProperties(null, null, true));
		this.mInventoryContainer.addAcceptingFromProperties("bag_inventory", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("loot", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("vault", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("delivery", this.AcceptFromProperties(this));
		this.mInventoryContainer.addAcceptingFromProperties("stamps", this.AcceptFromProperties(this));

		foreach( equipSlotName in this.EquipmentContainerNames )
		{
			this.mInventoryContainer.addAcceptingFromProperties(equipSlotName, this.AcceptFromProperties(this));
			this.mBagContainer.addAcceptingFromProperties(equipSlotName, this.AcceptFromProperties(this));
		}

		this.mBagContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		this.mBagContainer.addAcceptingFromProperties("bag_inventory", this.AcceptFromProperties());
	}

	function setVisible( visible )
	{
		this.GUI.Frame.setVisible(visible);

		if (visible)
		{
			::Audio.playSound("Sound-InventoryOpen.ogg");
		}
		else
		{
			::Audio.playSound("Sound-InventoryClose.ogg");
		}
	}

	function _addNotify()
	{
		this.GUI.ContainerFrame._addNotify();
	}

	function _removeNotify()
	{
		this.close();
		this.GUI.ContainerFrame._removeNotify();
	}

	function unlockAllActions()
	{
		this.mInventoryContainer.unlockAllSlots();
	}

	function _handleRightClickEquip( actionButton, evt )
	{
		if (actionButton && actionButton.getAction() && actionButton.getActionButtonSlot() && actionButton.getActionButtonSlot().getActionContainer())
		{
			local item = actionButton.getAction();
			local equipScreen = this.Screens.get("Equipment", true);

			if (equipScreen)
			{
				local foundActionButtonSlot = equipScreen.getActionButtonEquipSlot(item);

				if (foundActionButtonSlot)
				{
					local item = actionButton.getAction();
					local actionButtonSlot = actionButton.getActionButtonSlot();
					local actionContainer = actionButtonSlot.getActionContainer();
					local oldSlotContainerName = actionContainer.getContainerName();
					local oldSlotIndex = actionContainer.getIndexOfSlot(actionButtonSlot);
					local container = foundActionButtonSlot.getActionContainer();

					if (container)
					{
						local serverIndex = container.getSlotsRestriction(0);

						foreach( key, actionButtonSlot in container.getAllActionButtonSlots() )
						{
							if (!(actionButtonSlot && actionButtonSlot.getActionButton() && actionButtonSlot.getActionButton().getAction()))
							{
								serverIndex = container.getSlotsRestriction(key);
								break;
							}
						}

						local callback = {
							item = item,
							serverIndex = serverIndex,
							oldSlotContainerName = oldSlotContainerName,
							oldSlotIndex = oldSlotIndex,
							function onActionSelected( mb, alt )
							{
								if (alt == "Continue")
								{
									local inventory = ::Screens.get("Inventory", true);

									if (inventory)
									{
										inventory.handleAutoEquip(this.item, this.serverIndex, this.oldSlotContainerName, this.oldSlotIndex);
									}
								}
							}

						};
						local showPopup = ::Pref.get("other.BindPopup");

						if (showPopup && ("mItemData" in item) && item.mItemData && !item.mItemData.mBound && item._getItemDef() && item._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_EQUIP)
						{
							this.GUI.MessageBox.showEx("This item will be bound to you permanently if you choose to equip it.", [
								"Continue",
								"Cancel"
							], callback);
						}
						else
						{
							this.handleAutoEquip(item, serverIndex, oldSlotContainerName, oldSlotIndex);
						}

						evt.consume();
						return false;
					}
				}
			}
		}

		return true;
	}

}

class this.Inventory 
{
	mSize = 0;
	mInventoryOpen = false;
	mInventoryScreen = null;
	constructor( size )
	{
		this.mSize = size;
	}

	function toggleInventory()
	{
		if (this.mInventoryOpen)
		{
			this.mInventoryOpen = false;
			this.mInventoryScreen.setVisible(false);
		}
		else
		{
			if (!this.mInventoryScreen)
			{
				this.mInventoryScreen = this.Screens.Inventory(this.mSize);
			}

			this.mInventoryScreen.setOverlay(this.GUI.OVERLAY);
			this.mInventoryScreen.setPosition(200, 100);
			this.mInventoryScreen.setVisible(true);
		}
	}

	function resizeInventory( size )
	{
		this.mSize = size;

		if (this.mInventoryScreen)
		{
			this.mInventoryScreen._resize(this.mSize);
		}
	}

}

