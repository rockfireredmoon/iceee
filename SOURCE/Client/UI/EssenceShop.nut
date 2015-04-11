this.require("UI/Screens");
class this.Screens.EssenceShop extends this.GUI.Frame
{
	static mClassName = "Screens.EssenceShop";
	mScreenContainer = null;
	mHeaderContainer = null;
	mEssenceCostContainer = null;
	mItemContainer = null;
	mItemCostHolder = null;
	mButtonContainer = null;
	mMerchantId = null;
	mScreenWidth = 300;
	mScreenHeight = 490;
	mAcceptButton = null;
	mCancelButton = null;
	mCosts = null;
	mItemActionContainer = null;
	mEssenceCostActionContainer = null;
	mEssenceId = -1;
	ITEMS_PER_PAGE = 8;
	constructor()
	{
		this.GUI.Frame.constructor("Essence Shop");
		this.setPosition(50, 50);
		this.setSize(this.mScreenWidth, this.mScreenHeight);
		this.mEssenceCostContainer = [];
		this.mCosts = [];
		this.mScreenContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		this.mScreenContainer.getLayoutManager().setRows(40, 370, 40);
		this.mScreenContainer.setInsets(5);
		this.mHeaderContainer = this.GUI.Container(this.GUI.GridLayout(1, 3));
		this.mHeaderContainer.getLayoutManager().setColumns(150, 95, 32);
		this.mHeaderContainer.add(this.GUI.Spacer(150, 1));
		local essenceCostLabel = this.GUI.Label("Token Required:");
		essenceCostLabel.setTextAlignment(0.0, 0.40000001);
		essenceCostLabel.setFontColor(this.Colors.white);
		this.mHeaderContainer.add(essenceCostLabel);
		local centerContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mHeaderContainer.add(centerContainer);
		this.mEssenceCostActionContainer = this.GUI.ActionContainer("essence_needed", 1, 1, 0, 0, this);
		this.mEssenceCostActionContainer.setAllButtonsDraggable(false);
		centerContainer.add(this.mEssenceCostActionContainer);
		this.mScreenContainer.add(this.mHeaderContainer);
		this.mItemCostHolder = this.GUI.InnerPanel(this.GUI.GridLayout(1, 2));
		this.mItemCostHolder.getLayoutManager().setColumns(232, 50);
		this.mItemCostHolder.add(this._buildLootSection());
		this.mItemCostHolder.add(this._buildCostSection());
		this.mScreenContainer.add(this.mItemCostHolder);
		this.mAcceptButton = this.GUI.NarrowButton("Accept");
		this.mAcceptButton.setFixedSize(72, 32);
		this.mAcceptButton.setReleaseMessage("onAccept");
		this.mAcceptButton.addActionListener(this);
		this.mCancelButton = this.GUI.RedNarrowButton("Cancel");
		this.mCancelButton.setFixedSize(72, 32);
		this.mCancelButton.setReleaseMessage("onCancel");
		this.mCancelButton.addActionListener(this);
		this.mButtonContainer = this.GUI.Container(this.GUI.BoxLayout());
		this.mButtonContainer.setInsets(10, 5, 0, 5);
		this.mButtonContainer.add(this.mCancelButton);
		this.mButtonContainer.add(this.GUI.Spacer(130, 1));
		this.mButtonContainer.add(this.mAcceptButton);
		this.mScreenContainer.add(this.mButtonContainer);
		this.setContentPane(this.mScreenContainer);
		::_ItemDataManager.addListener(this);
		::_ItemManager.addListener(this);
		this.setCached(::Pref.get("video.UICache"));
	}

	function setMerchantId( creatureId )
	{
		this.mMerchantId = creatureId;

		if (creatureId != null)
		{
			this.mEssenceCostActionContainer.removeAllActions();
			local sceneObject = ::_sceneObjectManager.getCreatureByID(creatureId);
			this.setTitle(sceneObject.getName());
			this.mItemActionContainer.removeAllActions();
			::_Connection.sendQuery("essenceShop.contents", this, [
				this.mMerchantId
			]);
		}
	}

	function onAccept( button )
	{
		local lastClickedOn = this.mItemActionContainer.getSelectedSlot();

		if (!lastClickedOn)
		{
			return;
		}

		local actionButton = lastClickedOn.getActionButton();

		if (actionButton)
		{
			local action = actionButton.getAction();

			if (action)
			{
				local callback = {
					essenceShop = this,
					mMerchantId = this.mMerchantId,
					action = action,
					function onActionSelected( mb, alt )
					{
						if (alt == "Continue")
						{
							::_Connection.sendQuery("trade.essence", this.essenceShop, [
								this.mMerchantId,
								this.action.getProto()
							]);
						}
					}

				};
				local showPopup = ::Pref.get("other.BindPopup");

				if (showPopup && action._getItemDef() && action._getItemDef().getBindingType() == this.ItemBindingType.BIND_ON_PICKUP)
				{
					this.GUI.MessageBox.showEx("This item will be bound to you permanently if you choose to pick it up.", [
						"Continue",
						"Cancel"
					], callback);
				}
				else
				{
					::_Connection.sendQuery("trade.essence", this, [
						this.mMerchantId,
						action.getProto()
					]);
				}
			}
		}
	}

	function onCancel( button )
	{
		this.setVisible(false);
	}

	function onPageChanged( actionContainer, newPage )
	{
		if (this.mItemActionContainer == actionContainer)
		{
			this.mItemActionContainer.clearActiveSelection();
			local startIndex = (newPage - 1) * this.ITEMS_PER_PAGE;

			for( local i = 0; i < this.ITEMS_PER_PAGE; i++ )
			{
				if (startIndex + i >= this.mCosts.len())
				{
					while (i < this.ITEMS_PER_PAGE)
					{
						this.mEssenceCostContainer[i].setText("");
						i++;
					}

					return;
				}
				else
				{
					this.mEssenceCostContainer[i].setText(this.mCosts[startIndex + i]);
				}
			}
		}
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value == true)
		{
			this.mItemActionContainer.clearActiveSelection();
		}
	}

	function onItemDefUpdated( itemDefId, itemdef )
	{
		if (this.mEssenceId == -1 || itemDefId != this.mEssenceId)
		{
			return;
		}

		this._updateCountText(this.mEssenceId);
	}

	function onContainerUpdated( containerName, creatureId, container )
	{
		if (this.mEssenceId != -1 && containerName == "inv" && ::_avatar && ::_avatar.getID() == creatureId)
		{
			this._updateCountText(this.mEssenceId);
		}
	}

	function onStacksUpdated( sender, itemAction, mNumUses )
	{
		local itemData = itemAction.mItemData;
		local itemDefId = itemData.mItemDefId;

		if (itemDefId == 0 || itemDefId != this.mEssenceId)
		{
			return;
		}

		this._updateCountText(itemDefId);
	}

	function _updateCountText( itemDefId )
	{
		local numStacks = ::_ItemDataManager.getNumItems(itemDefId);
		local actionButton = this.mEssenceCostActionContainer.getActionButtonFromIndex(0);

		if (actionButton)
		{
			actionButton.setCountText("" + numStacks);
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "trade.essence")
		{
			local result = results[0][0];

			if (result == "OK")
			{
				local numEssence = 1;
				local itemSelected = "a item selected from loot bag";
				local displayName = "essence";

				if (results[0].len() > 1)
				{
					numEssence = results[0][1].tointeger();
				}

				if (qa.args.len() > 1)
				{
					local proto = qa.args[1];
					local attributes = this.Util.split(proto, ":");
					local itemIndex = attributes[0].find("item");

					if (itemIndex != null)
					{
						local itemId = attributes[0].slice(itemIndex + 4);
						local itemDef = ::_ItemDataManager.getItemDef(itemId.tointeger());
						itemSelected = itemDef.getDisplayName();
					}
				}

				if (this.mEssenceId != -1)
				{
					local essenceDef = ::_ItemManager.getItemDef(this.mEssenceId);

					if (essenceDef)
					{
						displayName = essenceDef.getName();
					}
				}

				this.IGIS.info("You have traded " + numEssence + " " + displayName + " for " + itemSelected + ".");
				this.mItemActionContainer.clearActiveSelection();
			}
			else
			{
				local notify = this.GUI.ConfirmationWindow();
				notify.setConfirmationType(this.GUI.ConfirmationWindow.OK);
				notify.setText(this.TXT(result));
			}
		}
		else if (qa.query == "essenceShop.contents")
		{
			this.mEssenceId = -1;
			local essenceDef;
			local i = 0;

			foreach( res in results )
			{
				if (this.mEssenceId == -1)
				{
					this.mEssenceId = res[0].tointeger();
					essenceDef = this._ItemManager.getItemDef(this.mEssenceId);
					this.mEssenceCostActionContainer.addAction(essenceDef, true);
				}
				else
				{
					local proto = res[0];
					local essencesNeeded = res[1];
					this.mItemActionContainer.addAction(this.ItemProtoAction(proto), true);
					this.mCosts.push(essencesNeeded);

					if (i < this.ITEMS_PER_PAGE)
					{
						this.mEssenceCostContainer[i].setText(essencesNeeded);
					}

					i++;
				}
			}
		}
	}

	function _buildCostSection()
	{
		local costContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		local costLabel = this.GUI.Label("Cost");
		costLabel.setFontColor(this.Colors.white);
		costContainer.add(costLabel);
		local yPos = 5;
		costContainer.add(this.GUI.Spacer(0, 3));

		for( local i = 0; i < this.ITEMS_PER_PAGE; i++ )
		{
			local label = this.GUI.Label("");
			label.setFont(this.GUI.Font("Maiandra", 28));
			label.setFontColor(this.Colors.white);
			this.mEssenceCostContainer.push(label);
			costContainer.add(this.mEssenceCostContainer[i]);
			costContainer.add(this.GUI.Spacer(0, 11));
		}

		return costContainer;
	}

	function _buildLootSection()
	{
		this.mItemContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mItemContainer.setSize(283, 370);
		this.mItemContainer.setPreferredSize(283, 370);
		local itemLabel = this.GUI.Label("Item");
		itemLabel.setFontColor(this.Colors.white);
		this.mItemContainer.add(itemLabel);
		this.mItemActionContainer = this.GUI.ActionContainer("essence_items", this.ITEMS_PER_PAGE, 1, 0, 0, this);
		this.mItemActionContainer.setTooltipRenderingModifiers("itemProto", {
			hideValue = true
		});
		this.mItemActionContainer.addListener(this);
		this.mItemActionContainer.setAllButtonsDraggable(false);
		this.mItemActionContainer.setHighlightSelectedIndex(true);
		this.mItemActionContainer.setItemPanelVisible(true);
		this.mItemContainer.add(this.mItemActionContainer);
		return this.mItemContainer;
	}

}

