this.require("UI/Screens");
this.require("GUI/Frame");
class this.Screens.MorphItemScreen extends this.GUI.Frame
{
	static mClassName = "Screens.MorphItemScreen";
	mStats = null;
	mLook = null;
	mResult = null;
	mOkBtn = null;
	mMorphBtn = null;
	mCost = null;
	mStatType = this.ItemEquipType.NONE;
	mLookType = this.ItemEquipType.NONE;
	mStatCost = 0;
	mLookCost = 0;
	mStatId = 0;
	mLookId = 0;
	mMorpherId = -1;
	mCheckDistanceEvent = null;
	constructor()
	{
		this.GUI.Frame.constructor(this.TXT("Refashion Item"));
		this.setSize(285, 250);
		this.setInsets(0, 0, 0, 0);
		local firstPanel = this.GUI.InnerPanel(null);
		firstPanel.setSize(92, 132);
		firstPanel.setPreferredSize(92, 132);
		firstPanel.setPosition(5, 3);
		local tempContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		tempContainer.setSize(32, 32);
		tempContainer.setPreferredSize(32, 32);
		tempContainer.setPosition(26, 40);
		tempContainer.setAppearance(null);
		firstPanel.add(tempContainer);
		local sourceContainer = this.GUI.ActionContainer("morph_stats", 1, 1, 0, 0, this, false);
		sourceContainer.setItemPanelVisible(false);
		sourceContainer.setValidDropContainer(true);
		sourceContainer.setAllowButtonDisownership(false);
		sourceContainer.setSlotDraggable(false, 0);
		sourceContainer.addListener(this);
		tempContainer.add(sourceContainer);
		local holdFirstTextComp = this.GUI.Container(this.GUI.BoxLayoutV());
		holdFirstTextComp.setSize(40, 40);
		holdFirstTextComp.setPosition(23, 83);
		holdFirstTextComp.getLayoutManager().setGap(-3);
		firstPanel.add(holdFirstTextComp);
		local origLabel = this.GUI.Label("Original");
		origLabel.setFontColor(this.Colors.white);
		holdFirstTextComp.add(origLabel);
		local itemLabel = this.GUI.Label("Item");
		itemLabel.setFontColor(this.Colors.white);
		holdFirstTextComp.add(itemLabel);
		local secondPanel = this.GUI.InnerPanel(null);
		secondPanel.setSize(92, 132);
		secondPanel.setPreferredSize(92, 132);
		secondPanel.setPosition(95, 3);
		local tempSecondContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		tempSecondContainer.setSize(32, 32);
		tempSecondContainer.setPreferredSize(32, 32);
		tempSecondContainer.setPosition(28, 40);
		tempSecondContainer.setAppearance(null);
		secondPanel.add(tempSecondContainer);
		local destContainer = this.GUI.ActionContainer("morph_look", 1, 1, 0, 0, this, false);
		destContainer.setItemPanelVisible(false);
		destContainer.setValidDropContainer(true);
		destContainer.setAllowButtonDisownership(false);
		destContainer.setSlotDraggable(false, 0);
		destContainer.addListener(this);
		tempSecondContainer.add(destContainer);
		local holdSecondTextComp = this.GUI.Container(this.GUI.BoxLayoutV());
		holdSecondTextComp.setSize(92, 40);
		holdSecondTextComp.setPreferredSize(92, 40);
		holdSecondTextComp.setPosition(0, 83);
		holdSecondTextComp.getLayoutManager().setGap(-3);
		holdSecondTextComp.getLayoutManager().setAlignment(0.5);
		secondPanel.add(holdSecondTextComp);
		local label = this.GUI.Label("New");
		label.setFontColor(this.Colors.white);
		holdSecondTextComp.add(label);
		label = this.GUI.Label("Equipment");
		label.setFontColor(this.Colors.white);
		holdSecondTextComp.add(label);
		label = this.GUI.Label("Look");
		label.setFontColor(this.Colors.white);
		holdSecondTextComp.add(label);
		this.mStats = sourceContainer;
		this.mLook = destContainer;
		local size = sourceContainer.getPreferredSize();
		sourceContainer.setSize(size.width, size.height);
		sourceContainer.setPosition(18, 18);
		destContainer.setSize(size.width, size.height);
		destContainer.setPosition(83, 18);
		sourceContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		destContainer.addAcceptingFromProperties("inventory", this.AcceptFromProperties(this));
		local thirdPanel = this.GUI.InnerPanel(null);
		thirdPanel.setSize(92, 132);
		thirdPanel.setPreferredSize(92, 132);
		thirdPanel.setPosition(185, 3);
		local img = this.GUI.Image();
		img.setAppearance("Icon");
		img.setSize(32, 32);
		img.setPosition(28, 40);
		this.mResult = img;
		thirdPanel.add(img);
		local holdThirdTextComp = this.GUI.Container(this.GUI.BoxLayoutV());
		holdThirdTextComp.setSize(92, 40);
		holdThirdTextComp.setPreferredSize(92, 40);
		holdThirdTextComp.setPosition(0, 83);
		holdThirdTextComp.getLayoutManager().setGap(-3);
		holdThirdTextComp.getLayoutManager().setAlignment(0.5);
		thirdPanel.add(holdThirdTextComp);
		local label = this.GUI.Label("Original Item");
		label.setFontColor(this.Colors.white);
		holdThirdTextComp.add(label);
		label = this.GUI.Label("with New Look");
		label.setFontColor(this.Colors.white);
		holdThirdTextComp.add(label);
		local plusImg = this.GUI.Container();
		plusImg.setSize(26, 27);
		plusImg.setAppearance("Refashion/Plus");
		plusImg.setPosition(-14, 43);
		secondPanel.add(plusImg);
		local equalsImg = this.GUI.Container();
		equalsImg.setAppearance("Refashion/Equal");
		equalsImg.setSize(26, 27);
		equalsImg.setPosition(-14, 43);
		thirdPanel.add(equalsImg);
		local okBtn = this.mOkBtn = this.GUI.NarrowButton("Finalize");
		okBtn.setFixedSize(72, 32);
		okBtn.setPosition(195, 190);
		okBtn.setEnabled(false);
		okBtn.setReleaseMessage("onProceed");
		okBtn.addActionListener(this);
		local morphBtn = this.mMorphBtn = this.GUI.NarrowButton("Refashion");
		morphBtn.setFixedSize(72, 32);
		morphBtn.setPosition(195, 143);
		morphBtn.setEnabled(false);
		morphBtn.setReleaseMessage("onMorph");
		morphBtn.addActionListener(this);
		local cancelBtn = this.GUI.RedNarrowButton("Cancel");
		cancelBtn.setFixedSize(72, 32);
		cancelBtn.setPosition(11, 143);
		cancelBtn.setReleaseMessage("onCancel");
		cancelBtn.addActionListener(this);
		local hSpacer = this.GUI.Container(null);
		hSpacer.setSize(272, 10);
		hSpacer.setPosition(5, 172);
		hSpacer.setAppearance("HSliderBar");
		local cost = this.mCost = this.GUI.Currency(1000, this.TXT("Cost:"));
		cost.setPosition(11, 190);
		cost.setSize(100, 30);
		cost.setCurrentValue(0);
		cost.setVisible(false);
		local container = this.GUI.Container(null);
		container.setSize(285, 250);
		container.add(thirdPanel);
		container.add(secondPanel);
		container.add(firstPanel);
		container.add(okBtn);
		container.add(morphBtn);
		container.add(cancelBtn);
		container.add(hSpacer);
		container.add(cost);
		this.setContentPane(container);
		this.setCached(::Pref.get("video.UICache"));
	}

	function onValidDropSlot( newSlot, oldSlot )
	{
		local actionBtn = oldSlot.getActionButton();
		local action = actionBtn.getAction();
		local container = newSlot.getActionContainer();
		local item = action.mItemDefData;
		local lookData = action.mLookDefData != null ? action.mLookDefData : action.mItemDefData;
		local eqslot = item ? item.mEquipType : this.ItemEquipType.NONE;
		local cost = item ? item._getPrice() : 0;
		local itemId = item ? action.getItemId() : 0;
		local sourceAction = this.mStats.getActionInSlot(0);
		local sourceActionItemId = -1;

		if (sourceAction && "mItemDefData" in sourceAction)
		{
			sourceActionItemId = sourceAction.mItemDefData.mID;
		}

		local destAction = this.mLook.getActionInSlot(0);
		local destActionItemId = -1;

		if (destAction && "mItemDefData" in destAction)
		{
			destActionItemId = destAction.mItemDefData.mID;
		}

		if (eqslot == this.ItemEquipType.NONE || eqslot == this.ItemEquipType.FOCUS_FIRE || eqslot == this.ItemEquipType.FOCUS_FROST || eqslot == this.ItemEquipType.FOCUS_MYSTIC || eqslot == this.ItemEquipType.FOCUS_DEATH || eqslot == this.ItemEquipType.CONTAINER || eqslot == this.ItemEquipType.COSEMETIC_SHOULDER || eqslot == this.ItemEquipType.COSEMETIC_HIP || eqslot == this.ItemEquipType.RED_CHARM || eqslot == this.ItemEquipType.GREEN_CHARM || eqslot == this.ItemEquipType.BLUE_CHARM || eqslot == this.ItemEquipType.ORANGE_CHARM || eqslot == this.ItemEquipType.YELLOW_CHARM || eqslot == this.ItemEquipType.PURPLE_CHARM || eqslot == this.ItemEquipType.WEAPON_1H || eqslot == this.ItemEquipType.WEAPON_1H_UNIQUE || eqslot == this.ItemEquipType.WEAPON_1H_MAIN || eqslot == this.ItemEquipType.WEAPON_1H_OFF || eqslot == this.ItemEquipType.WEAPON_2H || eqslot == this.ItemEquipType.WEAPON_RANGED || eqslot == this.ItemEquipType.ARMOR_RING || eqslot == this.ItemEquipType.ARMOR_RING_UNIQUE || eqslot == this.ItemEquipType.ARMOR_AMULET || eqslot == this.ItemEquipType.ARMOR_SHIELD)
		{
			this.IGIS.error("You can only morph armor items.");
			return false;
		}
		else if (container == this.mStats)
		{
			if (eqslot == this.mLookType || this.mLookType == this.ItemEquipType.NONE)
			{
				this.mStatType = eqslot;
				this.mStatId = itemId;
				this.mResult.setTooltip(actionBtn.getTooltip());
				this._setCost(cost, this.mLookCost);
			}
			else
			{
				this.IGIS.error("You can only morph items equipped in the same slot");
				return false;
			}
		}
		else if (eqslot == this.mStatType || this.mStatType == this.ItemEquipType.NONE)
		{
			this.mLookType = eqslot;
			this.mLookId = itemId;
			this._setCost(this.mStatCost, cost);
			local icons = this.Util.split(lookData.mIcon, "|");

			if (icons.len() == 2)
			{
				this.mResult.setImageName(icons[0]);
			}
			else
			{
				this.mResult.setImageName("");
			}
		}
		else
		{
			this.IGIS.error("You can only morph items equipped in the same slot");
			return false;
		}

		if (this.mStatType != this.ItemEquipType.NONE && this.mLookType != this.ItemEquipType.NONE)
		{
			this.mMorphBtn.setEnabled(true);
			this.mOkBtn.setEnabled(true);
		}

		return true;
	}

	function onProceed( evt )
	{
		local totalCost = ((this.mStatCost + this.mLookCost) / 2.0).tointeger();
		local copper = ::_avatar.getStat(this.Stat.COPPER);

		if (totalCost > copper)
		{
			this.IGIS.error("Insufficient funds to complete armor refashioning.");
			return;
		}

		if (::_Connection.getProtocolVersionId() >= 10)
		{
			local morpher = ::_sceneObjectManager.getCreatureByID(this.mMorpherId);

			if (morpher)
			{
				if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), morpher.getPosition()) <= this.MAX_USE_DISTANCE)
				{
					this._Connection.sendQuery("item.morph", this, [
						this.mStatId,
						this.mLookId,
						this.mMorpherId
					]);
					this.close();
				}
				else
				{
					this.IGIS.error("You are too far away from the item morpher to morph items.");
				}
			}
		}
		else
		{
			this._Connection.sendQuery("item.morph", this, [
				this.mStatId,
				this.mLookId
			]);
			this.close();
		}
	}

	function onCancel( evt )
	{
		this.close();
	}

	function onMorph( evt )
	{
		this.mResult.setVisible(true);
	}

	function setMorpherId( id )
	{
		this.mMorpherId = id;
	}

	function getMorpherId()
	{
		return this.mMorpherId;
	}

	function reset()
	{
		this.mStats.removeAllActions();
		this.mLook.removeAllActions();
		this.mResult.setVisible(false);
		this.mResult.setImageName("");
		this.mResult.setTooltip(null);
		this.mOkBtn.setEnabled(false);
		this.mMorphBtn.setEnabled(false);
		this.mCost.setVisible(false);
		this.mStatType = this.ItemEquipType.NONE;
		this.mLookType = this.ItemEquipType.NONE;
		this.mStatCost = 0;
		this.mLookCost = 0;
	}

	function morphingScreenDistanceCheck()
	{
		local morpher = ::_sceneObjectManager.getCreatureByID(this.mMorpherId);

		if (morpher)
		{
			if (this.Math.manhattanDistanceXZ(::_avatar.getPosition(), morpher.getPosition()) > this.MAX_USE_DISTANCE)
			{
				this.IGIS.error("You are too far away from the item morpher to continue morphing.");
				this.setVisible(false);
				this.mMorpherId = -1;
			}
			else
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "morphingScreenDistanceCheck");
			}
		}
		else
		{
			this.IGIS.error("This morpher no longer exists.");
			this.setVisible(false);
			this.mMorpherId = -1;
		}
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value)
		{
			if (!this.mCheckDistanceEvent)
			{
				this.mCheckDistanceEvent = ::_eventScheduler.fireIn(0.5, this, "morphingScreenDistanceCheck");
			}
		}
		else
		{
			local inventory = this.Screens.get("Inventory", false);

			if (inventory)
			{
				inventory.unlockAllActions();
			}

			if (this.mCheckDistanceEvent)
			{
				::_eventScheduler.cancel(this.mCheckDistanceEvent);
				this.mCheckDistanceEvent = null;
			}
		}
	}

	function _setCost( statCost, lookCost )
	{
		this.mStatCost = statCost;
		this.mLookCost = lookCost;
		this.mCost.setVisible(true);
		this.mCost.setCurrentValue(((this.mStatCost + this.mLookCost) / 2).tointeger());
	}

}

