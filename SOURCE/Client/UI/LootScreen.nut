this.require("UI/ActionContainer");
this.require("UI/Screens");
class this.Screens.LootScreen extends this.GUI.Frame
{
	static mScreenName = "LootScreen";
	mLootContainer = null;
	mLootAllButton = null;
	mScreenContainer = null;
	mCurrentlyLootingId = -1;
	mMaxColumns = 4;
	mMaxRows = 6;
	mAutoLoot = false;
	constructor()
	{
		this.GUI.Frame.constructor("Loot");
		this.setSize(236, 304);
		this.mScreenContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mScreenContainer.setInsets(3, 0, 3, 0);
		this.setContentPane(this.mScreenContainer);
		local outterFrame = this.GUI.InnerPanel(this.GUI.BoxLayoutV());
		outterFrame.setSize(220, 230);
		outterFrame.setPreferredSize(220, 230);
		outterFrame.setInsets(8);
		this.mScreenContainer.add(outterFrame);
		this.mLootContainer = this.GUI.InventoryActionContainer("loot", 5, 1, 0, 0, this);
		this.mLootContainer.setSize(222, 212);
		this.mLootContainer.setPreferredSize(222, 212);
		this.mLootContainer.setItemPanelVisible(true);
		this.mLootContainer.setAllButtonsDraggable(false);
		this.setContainerMoveProperties();
		this.mLootAllButton = this.GUI.NarrowButton("Loot All");
		this.mLootAllButton.addActionListener(this);
		this.mLootAllButton.setReleaseMessage("onLootAll");
		this.mLootAllButton.setFixedSize(72, 32);
		::_Connection.addListener(this);
		outterFrame.add(this.mLootContainer);
		local spacer = this.GUI.Spacer(0, 5);
		this.mScreenContainer.add(spacer);
		this.mScreenContainer.add(this.mLootAllButton);
		this.add(this.mScreenContainer);
		this.centerOnScreen();
		::_ItemDataManager.addListener(this);
		this.setCached(::Pref.get("video.UICache"));
	}

	function checkLootingPermissions( creature )
	{
		if (!::_avatar)
		{
			return false;
		}

		local lootableIDsString = creature.getStat(this.Stat.LOOTABLE_PLAYER_IDS);
		local lootableIDs = this.Util.split(lootableIDsString, ",");
		local avatarId = ::_avatar.getCreatureDef().getID();

		foreach( id in lootableIDs )
		{
			if (avatarId == id.tointeger())
			{
				return true;
			}
		}

		this.IGIS.error("You do not have permission to loot that corpse.");
		return false;
	}

	function onActionButtonLost( newSlot, oldSlot )
	{
		local oldActionContainer = oldSlot.getActionContainer();

		if (oldActionContainer == this.mLootContainer)
		{
			if (this.mLootContainer.getAllActionButtons(true).len() == 0)
			{
				this.setVisible(false);
			}
		}
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (containerName == "corpse")
		{
			this.mLootContainer.removeAllActions();

			foreach( itemId in container.mContents )
			{
				local item = ::_ItemManager.getItem(itemId);
				this.mLootContainer.addAction(item, true, item.mItemData.mContainerSlot);
			}
		}

		if (this.mAutoLoot == true)
		{
			this.onLootAll(null);
			this.setAutoLoot(false);
		}
	}

	function onLootGone( creature, slot )
	{
		if (creature == this.mCurrentlyLootingId)
		{
			this.mLootContainer.removeActionInSlot(slot);
		}
	}

	function onLootAll( evt )
	{
		local callback = {
			lootScreen = this,
			function onActionSelected( mb, alt )
			{
				if (alt == "Continue")
				{
					if (this.lootScreen)
					{
						this.lootScreen.handleLootAll();
					}
				}
			}

		};
		local bindOnPickup = false;
		local lootItems = this.mLootContainer.getAllActionButtons();

		foreach( item in lootItems )
		{
			local itemDef = item.getAction()._getItemDef();

			if (itemDef.getBindingType() == this.ItemBindingType.BIND_ON_PICKUP)
			{
				bindOnPickup = true;
				break;
			}
		}

		local showPopup = ::Pref.get("other.BindPopup");

		if (showPopup && bindOnPickup)
		{
			this.GUI.MessageBox.showEx("This item will be bound to you permanently if you choose to pick it up.", [
				"Continue",
				"Cancel"
			], callback);
		}
		else
		{
			this.handleLootAll();
		}
	}

	function handleLootAll()
	{
		local lootItems = this.mLootContainer.getAllActionButtons();

		foreach( item in lootItems )
		{
			::_Connection.sendQuery("loot.item", this, [
				this.mCurrentlyLootingId,
				item.getAction().getItemDefId()
			]);
		}

		this.setVisible(false);
	}

	function onQueryComplete( qa, results )
	{
		switch(qa.query)
		{
		case "loot.list":
			this.mLootContainer.removeAllActions();

			if (results.len() > 0)
			{
				if (results[0][0] == "FAIL")
				{
					this.IGIS.error(results[0][1]);
				}
				else
				{
					local shouldVisible = false;

					foreach( proto in results[0] )
					{
						if (proto != "")
						{
							shouldVisible = true;
							local newItem = this.GUI.ActionButton();
							newItem.fillOutFromProto(proto);
							this.mLootContainer.addActionFromButton(newItem, true);
						}
					}

					this.setVisible(shouldVisible);
				}
			}

			break;

		default:
			switch(qa.query)
			{
			case "loot.item":
				if (results[0][0] == "FAIL")
				{
					this.IGIS.error(results[0][1]);
				}
				else
				{
					local id = qa.args[1];
					local itemDef = ::_ItemDataManager.getItemDef(id);
					this.IGIS.info("You have looted: " + itemDef.getDisplayName());
					::Audio.playSound("Sound-Loot.ogg");
					::_tutorialManager.onItemGained(itemDef, id);
					this.mLootContainer.removeActionInSlot(results[0][1].tointeger());

					if (this.mLootContainer.isSlotEmpty(0))
					{
						this.setVisible(false);
					}
				}

				break;

			default:
				if (qa.query == "loot.exit")
				{
				}
			}
		}
	}

	function onRightButtonReleased( actionButton, evt )
	{
		::_Connection.sendQuery("loot.item", this, [
			this.mCurrentlyLootingId,
			actionButton.getAction().getItemDefId()
		]);
		return false;
	}

	function populateLoot( creatureId )
	{
		this.mCurrentlyLootingId = creatureId;
		this.mLootContainer.removeAllActions();
		this._Connection.sendQuery("loot.list", this, creatureId);
	}

	function setAutoLoot( value )
	{
		this.mAutoLoot = value;
	}

	function setContainerMoveProperties()
	{
		this.mLootContainer.addMovingToProperties("inventory", this.MoveToProperties(this.MovementTypes.MOVE, this));
		this.mLootContainer.addMovingToProperties("quickbar", this.MoveToProperties(this.MovementTypes.MOVE, this));
	}

	function setVisible( visible, ... )
	{
		this.GUI.Frame.setVisible(visible);

		if (visible)
		{
		}
		else
		{
			local overrideMessage = false;

			if (vargc > 0)
			{
				overrideMessage = vargv[0];
			}

			if (this.mCurrentlyLootingId != -1 && !overrideMessage)
			{
				::_Connection.sendQuery("loot.exit", this, []);
				this.mCurrentlyLootingId = -1;
			}
		}
	}

}

