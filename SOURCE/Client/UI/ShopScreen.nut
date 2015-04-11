this.require("UI/Screens");
this.require("GUI/Frame");
this.require("EventScheduler");
class this.Screens.ItemShop extends this.GUI.Frame
{
	static mClassName = "Screens.ItemShop";
	mMerchantId = null;
	mShopContainer = null;
	mBuybackContainer = null;
	mCurrencyLabel = null;
	mCreditLabel = null;
	mMyLevelCheck = null;
	mMyCoreClassCheck = null;
	mTooltipValue = null;
	mShopScreenType = this.CurrencyCategory.COPPER;
	mBuybackTab = null;
	mTabs = null;
	mSoldItems = null;
	mBuyItems = null;
	mCurrencyContainer = null;
	mCheckDistanceEvent = null;
	static INSET_SIZE = 10;
	static BASE_WIDTH = 534;
	static BASE_HEIGHT = 478;
	constructor()
	{
		this.GUI.Frame.constructor(this.TXT("Barter"));
		this.setSize(this.BASE_WIDTH, this.BASE_HEIGHT);
		this.mSoldItems = {};
		this.mBuyItems = {};
		local basecontainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		basecontainer.getLayoutManager().setColumns(this.BASE_WIDTH - this.INSET_SIZE * 2);
		basecontainer.getLayoutManager().setRows(20, this.BASE_HEIGHT - this.INSET_SIZE * 2 - 60, 20);
		basecontainer.setInsets(this.INSET_SIZE / 2, this.INSET_SIZE, this.INSET_SIZE, this.INSET_SIZE);
		this.setContentPane(basecontainer);
		local filtercontainer = this.GUI.Container(this.GUI.BoxLayout());
		filtercontainer.getLayoutManager().setGap(10);
		basecontainer.add(filtercontainer);
		local usablelabel = this.GUI.Label(this.TXT("Sort by:") + " ");
		filtercontainer.add(usablelabel);
		local levelContainer = this.GUI.Container(this.GUI.BoxLayout());
		filtercontainer.add(levelContainer);
		local mylevellabel = this.GUI.Label(this.TXT("Level") + " ");
		levelContainer.add(mylevellabel);
		this.mMyLevelCheck = this.GUI.CheckBox(this);
		this.mMyLevelCheck.setAppearance("CheckBoxSmall");
		this.mMyLevelCheck.setFixedSize(16, 16);
		levelContainer.add(this.mMyLevelCheck);
		local coreClassContainer = this.GUI.Container(this.GUI.BoxLayout());
		filtercontainer.add(coreClassContainer);
		local mycoreclasslabel = this.GUI.Label(this.TXT("Core Class") + " ");
		coreClassContainer.add(mycoreclasslabel);
		this.mMyCoreClassCheck = this.GUI.CheckBox(this);
		this.mMyCoreClassCheck.setAppearance("CheckBoxSmall");
		this.mMyCoreClassCheck.setFixedSize(16, 16);
		coreClassContainer.add(this.mMyCoreClassCheck);
		local shoptab = this.GUI.Container(this.GUI.BoxLayoutV());
		shoptab.getLayoutManager().setExpand(true);
		shoptab.setInsets(0, 0, 0, 0);
		this.mBuybackTab = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mBuybackTab.getLayoutManager().setExpand(true);
		this.mBuybackTab.setInsets(0, 0, 0, 0);
		this.mTabs = this.GUI.TabbedPane();
		this.mTabs.setSize(this.BASE_WIDTH - this.INSET_SIZE * 2, this.BASE_HEIGHT - this.INSET_SIZE * 2);
		this.mTabs.setPreferredSize(this.BASE_WIDTH - this.INSET_SIZE * 2, this.BASE_HEIGHT - this.INSET_SIZE * 2);
		this.mTabs.setTabPlacement("bottom");
		this.mTabs.addTab(this.TXT("Shop"), shoptab);
		this.mTabs.addTab(this.TXT("Buyback"), this.mBuybackTab);
		basecontainer.add(this.mTabs);
		this.mShopContainer = this.GUI.InventoryActionContainer("item_shop", 8, 2, 0, 0, this);
		this.mShopContainer.setItemPanelVisible(true);
		this.mShopContainer.setCallback(this);
		this.mShopContainer.setTooltipRenderingModifiers("itemProto", {
			showBuyValue = true
		});
		shoptab.add(this.mShopContainer);
		local helpLabel = this.GUI.Label("To buy, Ctrl-Right Click or drag to your backpack. To sell, drag from your backpack to this screen");
		shoptab.add(helpLabel);
		this.mBuybackContainer = this.GUI.InventoryActionContainer("buyback_shop", 8, 2, 0, 0, this);
		this.mBuybackContainer.setItemPanelVisible(true);
		this.mBuybackContainer.setCallback(this);
		this.mBuybackContainer.setFrontInsert(true);
		this.mBuybackContainer.setDelayContainerUpdate(true);
		this.mBuybackContainer.setSourceContainer(::_avatar.getID(), "buyback");
		this.mBuybackContainer.setTooltipRenderingModifiers("itemProto", {
			showBuyValue = true
		});
		this.mBuybackTab.add(this.mBuybackContainer);
		this.mCurrencyContainer = this.GUI.Component(this.GUI.BorderLayout());
		basecontainer.add(this.mCurrencyContainer);
		this.mCurrencyLabel = this.GUI.Currency(0, this.TXT("Your Money: "));
		this.mCurrencyContainer.add(this.mCurrencyLabel, this.GUI.BorderLayout.EAST);
		this.mCreditLabel = this.GUI.Credits(0, this.TXT("Your Money: "));

		if (::_avatar)
		{
			local playerCurrency = ::_avatar.getStat(this.Stat.COPPER);
			this.mCurrencyLabel.setCurrentValue(playerCurrency);
			local playerCredit = ::_avatar.getStat(this.Stat.CREDITS);
			this.mCreditLabel.setCurrentValue(playerCredit);
		}

		this.setContainerMoveProperties();
		::_Connection.addListener(this);
		::_ItemDataManager.addListener(this);
		this.mShopContainer.addListener(this);
		this.mBuybackContainer.addListener(this);
		this.setCached(::Pref.get("video.UICache"));
	}

	function onCopperUpdated( value )
	{
		this.mCurrencyLabel.setCurrentValue(value);
	}

	function onCreditUpdated( value )
	{
		this.mCreditLabel.setCurrentValue(value);
	}

	function isShopOrBuybackContainer( container )
	{
		if (container == this.mShopContainer || container == this.mBuybackContainer)
		{
			return true;
		}

		return false;
	}

	function setContainerMoveProperties()
	{
		this.mShopContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		this.mBuybackContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
	}

	function setMerchantId( creatureId )
	{
		this.mMerchantId = creatureId;

		if (creatureId != null)
		{
			this.mShopContainer.removeAllActions();
			::_Connection.sendQuery("shop.contents", this, [
				this.mMerchantId
			]);
		}
	}

	function onSellItem( action )
	{
		if (this.mShopScreenType == this.CurrencyCategory.CREDITS)
		{
			this.IGIS.error("You can only purchase items from a credit shop.");
			return;
		}

		if (action instanceof this.ItemAction)
		{
			if (!action.isValid())
			{
				return;
			}

			local itemDef = action._getItemDef();

			if (!itemDef)
			{
				return;
			}

			if (itemDef.mValueType == this.CurrencyCategory.CREDITS)
			{
				this.IGIS.error("Items worth credits cannot be sold");
				return;
			}
		}

		local merchant = ::_sceneObjectManager.getCreatureByID(this.mMerchantId);

		if (merchant)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), merchant.getPosition()) > this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the item merchant to sell items.");
				return;
			}
		}

		local paramaters = [];
		paramaters.append(this.mMerchantId);
		paramaters.append("sell");
		local itemid = action.getItemId();
		paramaters.append(itemid);

		if (paramaters.len() > 0)
		{
			local query = ::_Connection.sendQuery("trade.shop", this, paramaters);
			local soldItem = {
				itemDefId = action.getItemDefId(),
				count = action.getNumStacks()
			};
			this.mSoldItems[query.correlationId] <- soldItem;
		}
	}

	function onBuyItem( buttonAction )
	{
		local merchant = ::_sceneObjectManager.getCreatureByID(this.mMerchantId);

		if (merchant)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), merchant.getPosition()) > this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the item merchant to buy items.");
				return;
			}
		}

		local isItemAction = false;
		local paramaters = [];
		paramaters.append(this.mMerchantId);

		if (buttonAction instanceof this.ItemProtoAction)
		{
			local proto = buttonAction.getProto();
			paramaters.append(proto);
		}
		else if (buttonAction instanceof this.ItemAction)
		{
			local id = buttonAction.getItemId();
			paramaters.append(id);
			isItemAction = true;
		}

		if (paramaters.len() > 0)
		{
			local query = ::_Connection.sendQuery("trade.shop", this, paramaters);

			if (isItemAction)
			{
				local buyItem = {
					itemDefId = buttonAction.getItemDefId(),
					count = buttonAction.getNumStacks()
				};
				this.mBuyItems[query.correlationId] <- buyItem;
			}
		}
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "trade.shop")
		{
			this.IGIS.error(this.TXT(error));
		}
	}

	function onQueryTimeout( qa )
	{
		this.log.warn("Query " + qa.query + " [" + qa.correlationId + "] timed out");
		this.IGIS.error(qa.query + " timed out.");
	}

	function _getItemDefIdAndCount( item )
	{
		local itemInfo = [];
		local itemData = this.split(item, ":");

		if (itemData.len() > 2)
		{
			local itemDefId = this.Util.replace(itemData[0], "item", "");
			itemInfo.append(itemDefId.tointeger());
			local count = itemData[2].tointeger();

			if (count == 0)
			{
				count = 1;
			}

			itemInfo.append(count);
		}

		return itemInfo;
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "trade.shop")
		{
			local result = results[0][0];

			if (result == "OK")
			{
				::Audio.playSound("Sound-BuyOrSellComplete.ogg");

				if (qa.args.len() > 2)
				{
					if (qa.args[1] == "sell" && qa.correlationId in this.mSoldItems)
					{
						local soldItem = this.mSoldItems[qa.correlationId];
						local id = soldItem.itemDefId;
						local count = soldItem.count;

						if (count == 0)
						{
							count = 1;
						}

						local itemDef = ::_ItemDataManager.getItemDef(id);
						local message = "You have sold " + count + " " + itemDef.getDisplayName() + " to the shop.";
						::_ChatManager.addMessage("sys/info", message);
						delete this.mSoldItems[qa.correlationId];
					}
				}
				else if (qa.args.len() > 1)
				{
					local id = 1;
					local count = 1;

					if (qa.correlationId in this.mBuyItems)
					{
						local buyItem = this.mBuyItems[qa.correlationId];
						id = buyItem.itemDefId;
						count = buyItem.count;
						delete this.mBuyItems[qa.correlationId];
					}
					else
					{
						local item = qa.args[1];
						local itemData = this._getItemDefIdAndCount(item);

						if (itemData.len() > 1)
						{
							id = itemData[0];
							count = itemData[1];
						}
					}

					if (count == 0)
					{
						count = 1;
					}

					local itemDef = ::_ItemDataManager.getItemDef(id);
					::_tutorialManager.onItemGained(itemDef, id);
					local message = "You have bought " + count + " " + itemDef.getDisplayName() + ".";
					::_ChatManager.addMessage("sys/info", message);
				}
			}
			else
			{
				local notify = this.GUI.ConfirmationWindow();
				notify.setConfirmationType(this.GUI.ConfirmationWindow.OK);
				notify.setText(this.TXT("Your transaction was refused."));
			}
		}
		else if (qa.query == "shop.contents")
		{
			this.mShopContainer.removeAllActions();
			local resultLen = results.len();
			local i = 0;

			foreach( res in results )
			{
				local proto = res[0];

				if (i >= resultLen - 1)
				{
					this.mShopContainer.addAction(this.ItemProtoAction(proto), true);
				}
				else
				{
					this.mShopContainer.addAction(this.ItemProtoAction(proto), false);
				}

				i++;
			}
		}
	}

	function onActionButtonAdded( container, actionButtonSlot, actionButton, action )
	{
		if (container == this.mShopContainer || container == this.mBuybackContainer)
		{
			if (actionButton)
			{
				local action = actionButton.getAction();

				if (action.getIsValid())
				{
					local profession = this.Professions[::_avatar.getStat(this.Stat.PROFESSION)].name;
					local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);
					local disabled = false;
					local itemdef = action._getItemDef();
					local classrestrictions = itemdef.getClassRestrictions();

					if (avatarlevel < itemdef.getMinUseLevel())
					{
						disabled = true;
					}

					if (!classrestrictions[profession.tolower()])
					{
						disabled = true;
					}

					actionButtonSlot.setSlotDisabled(disabled);
				}
			}
		}
	}

	function onRightButtonReleased( actionButton, evt )
	{
		local currentSlot = actionButton.getActionButtonSlot();

		if (currentSlot && currentSlot.getActionContainer() && this.Key.isDown(this.Key.VK_CONTROL))
		{
			local currentcontainer = currentSlot.getActionContainer();

			switch(currentcontainer)
			{
			case this.mShopContainer:
			case this.mBuybackContainer:
				this.onBuyItem(actionButton.getAction());
				evt.consume();
				break;

			default:
				break;
			}
		}
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local container = newSlot.getActionContainer();
		local oldContainer = oldSlot.getActionContainer();
		local action = oldSlot.getActionButton().getAction();

		if (container == this.mShopContainer || container == this.mBuybackContainer)
		{
			this.onSellItem(action);
		}
		else if (oldContainer == this.mShopContainer || oldContainer == this.mBuybackContainer)
		{
			this.onBuyItem(action);
		}

		return false;
	}

	function shopScreenDistanceCheck()
	{
		if (!this.isVisible())
		{
			return;
		}

		local merchant = ::_sceneObjectManager.getCreatureByID(this.mMerchantId);

		if (merchant)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), merchant.getPosition()) > this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the merchant to continue buying or selling items.");
				this.setVisible(false);
				this.mMerchantId = -1;
			}
			else
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "shopScreenDistanceCheck");
			}
		}
		else
		{
			this.IGIS.error("This merchant no longer exists.");
			this.setVisible(false);
			this.mMerchantId = -1;
		}
	}

	function setVisible( visible )
	{
		this.GUI.Frame.setVisible(visible);

		if (!visible)
		{
			::Screens.get("Inventory", true).unlockAllActions();

			if (this.mCheckDistanceEvent)
			{
				::_eventScheduler.cancel(this.mCheckDistanceEvent);
				this.mCheckDistanceEvent = null;
			}
		}
		else
		{
			local value = ::_avatar.getStat(this.Stat.COPPER);
			this.onCopperUpdated(value);
			local creditValue = ::_avatar.getStat(this.Stat.CREDITS);
			this.onCreditUpdated(creditValue);
			::_ItemDataManager.getContents("buyback", ::_avatar.getID(), true);

			if (!this.mCheckDistanceEvent)
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "shopScreenDistanceCheck");
			}
		}
	}

	function setShopScreenCategory( shopType )
	{
		if (shopType != this.mShopScreenType)
		{
			this.mShopScreenType = shopType;

			if (this.mShopScreenType == this.CurrencyCategory.COPPER)
			{
				this.mTabs.addTab(this.TXT("Buyback"), this.mBuybackTab);
				this.mCurrencyContainer.remove(this.mCreditLabel);
				this.mCurrencyContainer.add(this.mCurrencyLabel, this.GUI.BorderLayout.EAST);
			}
			else
			{
				local buybackTab = this.mTabs._findTab("Buyback");

				if (buybackTab)
				{
					this.mTabs.remove(buybackTab);
				}

				this.mCurrencyContainer.remove(this.mCurrencyLabel);
				this.mCurrencyContainer.add(this.mCreditLabel, this.GUI.BorderLayout.EAST);
			}
		}
	}

	function onActionPerformed( checkbox, checked )
	{
		this.mShopContainer.updateContainer();
		this.mBuybackContainer.updateContainer();
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (!::_avatar)
		{
			return;
		}

		if (!::_avatar.getStat(this.Stat.PROFESSION))
		{
			return;
		}

		local profession = this.Professions[::_avatar.getStat(this.Stat.PROFESSION)].name;
		local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);
		local visibleItemCount = this.mShopContainer.getNumSlots();

		for( local i = 0; i < visibleItemCount; i++ )
		{
			local actionButtonSlot = this.mShopContainer.getSlotContents(i);

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
						local classrestrictions = itemdef.getClassRestrictions();

						if (avatarlevel < itemdef.getMinUseLevel())
						{
							disabled = true;
						}

						if (!classrestrictions[profession.tolower()])
						{
							disabled = true;
						}

						actionButtonSlot.setSlotDisabled(disabled);
					}
				}
			}
		}
	}

	function shouldActionButtonBeAdded( container, actionbuttonslot, actionbutton, action )
	{
		local profession = this.Professions[::_avatar.getStat(this.Stat.PROFESSION)].name;
		local avatarlevel = ::_avatar.getStat(this.Stat.LEVEL);

		if (container == this.mShopContainer || container == this.mBuybackContainer)
		{
			local itemdef = action._getItemDef();
			local classrestrictions = itemdef.getClassRestrictions();

			if (avatarlevel < itemdef.getMinUseLevel())
			{
				if (this.mMyLevelCheck.getChecked())
				{
					return false;
				}
			}

			if (!classrestrictions[profession.tolower()])
			{
				if (this.mMyCoreClassCheck.getChecked())
				{
					return false;
				}
			}
		}

		return true;
	}

}

