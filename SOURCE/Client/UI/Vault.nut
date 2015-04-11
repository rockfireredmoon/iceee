this.require("UI/ActionContainer");
this.require("UI/Equipment");
this.require("UI/Screens");
class this.Screens.Vault extends this.GUI.BigFrame
{
	static mClassName = "Screens.Vault";
	mScreenContainer = null;
	mVaultContainer = null;
	mVaultContainerHolder = null;
	mColumns = 8;
	mRows = 15;
	mDeliveryColumns = 1;
	mDeliveryRows = 3;
	mMaxVaultSize = 120;
	mCurrentVaultSize = -1;
	mExpandVaultButton = null;
	mCurrentVaultId = -1;
	mDisabledSlotMaterial0 = "BG/ActionHolderPlug_64";
	mEnabledSlotMaterial0 = "BG/ActionHolder1_64";
	mEnabledSlotMaterial1 = "BG/ActionHolder2_64";
	mEnabledSlotMaterial2 = "BG/ActionHolder3_64";
	mNewsAndDeliveryHolder = null;
	mDeliveryBox = null;
	mDeliveryBoxAC = null;
	mNewsBox = null;
	mCheckDistanceEvent = null;
	mCheckPurchaseEvent = null;
	constructor()
	{
		this.GUI.BigFrame.constructor("Vault", true, {
			x = 373,
			y = 1
		});
		this.mScreenContainer = this.GUI.Container(this.GUI.BoxLayout());
		this.setPosition(400, 50);
		::_ItemDataManager.addListener(this);
		this.setContentPane(this.mScreenContainer);
		this.mScreenContainer.setInsets(5);
		::_Connection.addListener(this);
		this.mTitleBar.setAppearance("VaultTop");
		this.mContentPane.setAppearance("VaultSides");
		this.setCached(::Pref.get("video.UICache"));
	}

	function close()
	{
		this.setVisible(false);
	}

	function disableSlot( slot )
	{
		if (this.mVaultContainer)
		{
			this.mVaultContainer.setSlotsBackgroundMaterial(slot, this.mDisabledSlotMaterial0, true);
			this.mVaultContainer.setValidDropSlot(slot, false);
		}
	}

	function enableSlot( slot )
	{
		if (this.mVaultContainer)
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

			this.mVaultContainer.setSlotsBackgroundMaterial(slot, texture, true);
			this.mVaultContainer.setValidDropSlot(slot, true);
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
		return this.mCurrentVaultSize - this.mVaultContainer.getAllActionButtons().len();
	}

	function getMovable()
	{
		return this.mMovable;
	}

	function getActionContainer()
	{
		return this.mVaultContainer;
	}

	function onAcceptExpand( source, value )
	{
		if (value == "Proceed" && this.mExpandVaultButton)
		{
			this.mExpandVaultButton.setEnabled(false);
			::_Connection.sendQuery("vault.expand", this, [
				this.mCurrentVaultId
			]);
		}
	}

	function onActionButtonAdded( container, actionbuttonslot, actionbutton, action )
	{
		local disabled = false;

		if (container == this.mVaultContainer)
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

	function onAttemptedItemDisown( actionContainer, action, slotIndex )
	{
		if (actionContainer == this.mVaultContainer)
		{
			local callback = {
				currAction = action,
				slotNum = slotIndex,
				currActionContainer = this.mVaultContainer,
				currentVaultId = this.mCurrentVaultId,
				function onActionSelected( mb, alt )
				{
					if (alt == "Yes")
					{
						if (this._Connection)
						{
							this._Connection.sendQuery("item.delete", this, [
								this.currAction.getItemId(),
								this.currentVaultId
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
			if (containerName == "bank")
			{
				if (this.mVaultContainer)
				{
					this.mVaultContainer.removeAllActions();

					foreach( itemId in container.mContents )
					{
						local item = ::_ItemManager.getItem(itemId);
						this.mVaultContainer.addAction(item, false, item.mItemData.mContainerSlot);
					}

					this.mVaultContainer.updateContainer();
				}
			}
		}
	}

	function onClosePressed()
	{
		this.close();
	}

	function onExpandVaultPressed( button )
	{
		this.GUI.MessageBox.showEx("You are about to expand your Vault by 8 slots. You will be charged 200 Credits.", [
			"Proceed",
			"Cancel"
		], this, "onAcceptExpand");
	}

	function onRightButtonReleased( actionButton, evt )
	{
		local abSlot = actionButton.getActionButtonSlot();

		if (abSlot)
		{
			local actionContainer = abSlot.getActionContainer();

			if (actionContainer == this.mDeliveryBoxAC)
			{
				local slotIndex = this.mDeliveryBoxAC.getIndexOfSlot(abSlot);

				if (slotIndex != null)
				{
					slotIndex = slotIndex + (this.mDeliveryBoxAC.getCurrentPageNumber() - 1) * this.mDeliveryRows * this.mDeliveryColumns;
					::_Connection.sendQuery("vault.lootdeliveryitem", this, [
						this.mCurrentVaultId,
						slotIndex
					]);
					return false;
				}
			}
			else if (actionContainer == this.mVaultContainer)
			{
				if (this.Key.isDown(this.Key.VK_CONTROL))
				{
					local inventory = this.Screens.get("Inventory", true);

					if (inventory)
					{
						local freeslots = inventory.getFreeSlotsRemaining();

						if (freeslots > 0)
						{
							local inventoryAC = inventory.getMyActionContainer();
							inventoryAC.simulateButtonDrop(actionButton);
						}
					}
				}
			}
		}

		return true;
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
		local visibleItemCount = this.mVaultContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = this.mVaultContainer.getSlotContents(i);

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

		if (container == this.mVaultContainer)
		{
			if (item.getSwapped() == true)
			{
				item.setSwapped(false);
			}
			else
			{
				queryArgument.append(itemID);
				queryArgument.append("bank");
				queryArgument.append(slotIndex);

				if (::_Connection.getProtocolVersionId() >= 19)
				{
					queryArgument.append(oldSlotContainerName);
					queryArgument.append(oldSlotIndex);
				}

				queryArgument.append(::_avatar.getID());
				queryArgument.append(this.mCurrentVaultId);
				this._Connection.sendQuery("item.move", this, queryArgument);
			}
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
		local visibleItemCount = this.mVaultContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = this.mVaultContainer.getSlotContents(i);

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

	function onQueryComplete( qa, rows )
	{
		switch(qa.query)
		{
		case "vault.expand":
			this.mCheckPurchaseEvent = ::_eventScheduler.fireIn(2.0, this, "_delayVaultButtonValid");
			break;

		case "vault.size":
			this.mCurrentVaultSize = rows[0][0].tointeger();
			this._buildNewsAndDelivery();
			this._buildVault();
			this.refreshVault();
			local ndSize = this.mNewsAndDeliveryHolder.getSize();
			local vaultSize = this.mVaultContainerHolder.getSize();
			this.setSize(ndSize.width + vaultSize.width + 8, ndSize.height + 32);
			this.setPreferredSize(ndSize.width + vaultSize.width + 8, ndSize.height + 32);
			this.mScreenContainer.setSize(ndSize.width + vaultSize.width, ndSize.height);
			this.mScreenContainer.setPreferredSize(ndSize.width + vaultSize.width, ndSize.height);
			::_Connection.sendQuery("vault.deliverycontents", this, [
				this.mCurrentVaultId
			]);
			this.GUI.Frame.setVisible(true);
			break;

		case "vault.deliverycontents":
			for( local i = 0; i < rows.len(); i++ )
			{
				this.mDeliveryBoxAC.addAction(this.ItemProtoAction(rows[i][0]), i == rows.len() - 1);
			}

			break;

		case "vault.lootdeliveryitem":
			if (rows[0][0] == "OK")
			{
				local slotIndex = qa.args[1];
				this.mDeliveryBoxAC.removeActionInSlot(slotIndex);
			}

			break;
		}
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);

		switch(qa.query)
		{
		case "item.move":
			::Util.handleMoveItemBack(qa);
			break;

		case "vault.size":
			break;

		case "vault.expand":
			this.mCheckPurchaseEvent = ::_eventScheduler.fireIn(2.0, this, "_delayVaultButtonValid");
			break;
		}
	}

	function _delayVaultButtonValid()
	{
		if (this.mCheckPurchaseEvent)
		{
			::_eventScheduler.cancel(this.mCheckPurchaseEvent);
		}

		if (this.mExpandVaultButton)
		{
			this.mExpandVaultButton.setEnabled(true);
		}
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
	}

	function refreshVault()
	{
		local vault = ::_ItemDataManager.getContents("bank");
		local bags = ::_ItemDataManager.getContents("eq");
		this.onContainerUpdated("bank", ::_avatar.getID(), vault);
		this.onContainerUpdated("eq", ::_avatar.getID(), bags);
	}

	function setContainerMoveProperties()
	{
		this.mVaultContainer.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mVaultContainer.addMovingToProperties("vault", this.MoveToProperties(this.MovementTypes.MOVE));
		this.mVaultContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		this.mVaultContainer.addAcceptingFromProperties("vault", this.AcceptFromProperties(this));
	}

	function setVaultSize( newVaultSize )
	{
		for( local i = this.mCurrentVaultSize; i < newVaultSize; i++ )
		{
			this.enableSlot(i);
		}

		this.mCurrentVaultSize = newVaultSize;

		if (this.mCurrentVaultSize >= this.mMaxVaultSize)
		{
			if (this.mExpandVaultButton)
			{
				this.mVaultContainerHolder.remove(this.mExpandVaultButton);
				local vaultSize = this.mVaultContainerHolder.getSize();
				local buttonSize = this.mExpandVaultButton.getSize();
				this.mVaultContainerHolder.setSize(vaultSize.width + 4, vaultSize.height - buttonSize.height);
				this.mVaultContainerHolder.setPreferredSize(vaultSize.width + 4, vaultSize.height - buttonSize.height);
				this.mExpandVaultButton.destroy();
				this.mExpandVaultButton = null;
			}
		}
	}

	function setVaultId( id )
	{
		this.mCurrentVaultId = id;
	}

	function setVisible( visible )
	{
		if (visible)
		{
			::Audio.playSound("Sound-InventoryOpen.ogg");

			if (this.mCurrentVaultSize == -1)
			{
				::_Connection.sendQuery("vault.size", this, [
					this.mCurrentVaultId
				]);
			}
			else
			{
				this.GUI.Frame.setVisible(true);
				this.mDeliveryBoxAC.removeAllActions();
				::_Connection.sendQuery("vault.deliverycontents", this, [
					this.mCurrentVaultId
				]);
			}

			if (!this.mCheckDistanceEvent)
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "vaultDistanceCheck");
			}

			this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "vaultDistanceCheck");
		}
		else
		{
			this.mCurrentVaultId = -1;
			this.GUI.Frame.setVisible(false);
			::Audio.playSound("Sound-InventoryClose.ogg");

			if (this.mCheckDistanceEvent)
			{
				::_eventScheduler.cancel(this.mCheckDistanceEvent);
				this.mCheckDistanceEvent = null;
			}
		}
	}

	function vaultDistanceCheck()
	{
		if (!this.isVisible())
		{
			return;
		}

		local vault = ::_sceneObjectManager.getCreatureByID(this.mCurrentVaultId);

		if (vault)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), vault.getPosition()) > this.Util.getRangeOffset(this._avatar, vault) + this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the vault to continue using it.");
				this.setVisible(false);
				this.mCurrentVaultId = -1;
			}
			else
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "vaultDistanceCheck");
			}
		}
		else
		{
			this.IGIS.error("This vault no longer exists.");
			this.setVisible(false);
			this.mCurrentVaultId = -1;
		}
	}

	function _addNotify()
	{
		this.GUI.ContainerFrame._addNotify();
	}

	function _buildVault()
	{
		this.mVaultContainerHolder = this.GUI.Container(this.GUI.BorderLayout());
		local titleBar = this.GUI.TitleBar(null, "Character Vault", false);
		titleBar.setAppearance("FrameTopBarSilver");
		this.mVaultContainer = this.GUI.ActionContainer("vault", this.mRows, this.mColumns, 0, 0, this, false);
		this.mVaultContainer.setUseMode(this.GUI.ActionButtonSlot.USE_LEFT_DOUBLE_CLICK);
		this.mVaultContainer.setCallback(this);
		this.mVaultContainer.setAllowButtonDisownership(false);
		local vaultPanel = this.GUI.Panel(this.GUI.BoxLayoutV());
		vaultPanel.add(this.mVaultContainer);
		vaultPanel.setAppearance("SilverBorder");
		this.mVaultContainerHolder.add(titleBar, this.GUI.BorderLayout.NORTH);
		this.mVaultContainerHolder.add(vaultPanel, this.GUI.BorderLayout.CENTER);
		local size = this.mVaultContainer.getPreferredSize();

		if (this.mCurrentVaultSize < this.mMaxVaultSize)
		{
			this.mExpandVaultButton = this.GUI.NarrowButton("Expand Vault");
			this.mExpandVaultButton.addActionListener(this);
			this.mExpandVaultButton.setReleaseMessage("onExpandVaultPressed");
			this.mVaultContainerHolder.add(this.mExpandVaultButton, this.GUI.BorderLayout.SOUTH);
			local buttonSize = this.mExpandVaultButton.getPreferredSize();
			size.height += buttonSize.height;
		}

		local titleBarSize = titleBar.getSize();
		size.height += titleBarSize.height;
		this.mVaultContainerHolder.setSize(size.width + 15, size.height + 42);
		this.mVaultContainerHolder.setPreferredSize(size.width + 15, size.height + 42);
		this.setContainerMoveProperties();
		this.mVaultContainer.addListener(this);
		this.mVaultContainer.setItemPanelVisible(false);
		this.mVaultContainer.setValidDropContainer(true);
		local vault = ::_ItemDataManager.getContents("bank");
		this.onContainerUpdated("bank", ::_avatar.getID(), vault);
		local i = 0;

		for( i = 0; i < this.mCurrentVaultSize; i++ )
		{
			this.enableSlot(i);
		}

		while (i < this.mMaxVaultSize)
		{
			this.disableSlot(i);
			i++;
		}

		this.mScreenContainer.add(this.mVaultContainerHolder);
	}

	function _buildNewsAndDelivery()
	{
		this.mNewsAndDeliveryHolder = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mNewsBox = this._buildNewsBox();
		this.mDeliveryBox = this._buildDeliveryBox();
		local newsSize = this.mNewsBox.getSize();
		local deliverySize = this.mDeliveryBox.getSize();
		this.mNewsAndDeliveryHolder.add(this.mNewsBox);
		this.mNewsAndDeliveryHolder.add(this.mDeliveryBox);
		this.mScreenContainer.add(this.mNewsAndDeliveryHolder);
		this.mNewsAndDeliveryHolder.setSize(newsSize.width, newsSize.height + deliverySize.height);
	}

	function _buildNewsBox()
	{
		local newsBox = this.GUI.Panel();
		newsBox.setAppearance("VaultImage");
		newsBox.setSize(254, 315);
		newsBox.setPreferredSize(254, 315);
		return newsBox;
	}

	function _buildDeliveryBox()
	{
		local deliveryBox = this.GUI.Container(this.GUI.BorderLayout());
		local containerHolder = this.GUI.Panel(this.GUI.BoxLayoutV());
		containerHolder.setAppearance("SilverBorder");
		local titleBar = this.GUI.TitleBar(null, "Delivery Box", false);
		titleBar.setAppearance("FrameTopBarSilver");
		this.mDeliveryBoxAC = this.GUI.ActionContainer("deliveryBox", this.mDeliveryRows, this.mDeliveryColumns, 0, 0, this);
		this.mDeliveryBoxAC.setAllButtonsDraggable(false);
		this.mDeliveryBoxAC.setItemPanelVisible(true);
		this.mDeliveryBoxAC.setTooltipRenderingModifiers("itemProto", {
			hideValue = true,
			resizeInfoPanel = false
		});
		containerHolder.add(this.mDeliveryBoxAC);
		deliveryBox.add(titleBar, this.GUI.BorderLayout.NORTH);
		deliveryBox.add(containerHolder, this.GUI.BorderLayout.CENTER);
		deliveryBox.setPreferredSize(250, 250);
		deliveryBox.setSize(250, 250);
		return deliveryBox;
	}

	function _removeNotify()
	{
		this.close();
		this.GUI.ContainerFrame._removeNotify();
	}

}

