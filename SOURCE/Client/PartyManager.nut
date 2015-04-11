this.require("Connection");
this.require("ServerConstants");
this.require("GUI/Panel");
this.require("GUI/BoxLayout");
this.require("GUI/Container");
this.require("GUI/Frame");
this.PARTY_COMPONENT_WIDTH <- 135;
this.PARTY_COMPONENT_HEIGHT <- 58;
class this.PartyComponent extends this.GUI.Container
{
	static MOVEMENT_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static MOVEMENT_NORMAL = this.Color(0.31999999, 0.12, 0.44999999, 1);
	static STAT_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static STAT_NORMAL = this.Color(0.60000002, 0.12, 0.14, 1);
	static OTHER_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static OTHER_NORMAL = this.Color(0.75999999, 0.44999999, 0.1, 1);
	static BUFF_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static BUFF_NORMAL = this.Color(0, 0.80000001, 0, 1);
	mFloating = false;
	mId = 0;
	mHealthBar = null;
	mCompact = null;
	mNextFreeBuffColumn = 7;
	mBuffContainer = null;
	mNextFreeDebuffColumn = 0;
	mNextFreeDebuffRow = 0;
	mDebuffContainer = null;
	mDragging = false;
	mDragged = false;
	mLastMouse = {};
	mMenu = null;
	mLeaderStar = null;
	mMemberName = null;
	mCreature = null;
	mRemovedBuffs = false;
	mRemovedDebuffs = false;
	static mHealthBarWidth = 120;
	constructor( memberInfo )
	{
		this.GUI.Container.constructor(this.GUI.GridLayout(1, 2));
		this.getLayoutManager().setColumns(135, 115);
		this.mId = memberInfo.id;
		this.mMemberName = memberInfo.name;
		this.setSize(this.PARTY_COMPONENT_WIDTH, this.PARTY_COMPONENT_HEIGHT);
		this.setOverlay("GUI/PartyOverlay");
		local partyContainer = this.GUI.Container(null);
		partyContainer.setSize(this.PARTY_COMPONENT_WIDTH, this.PARTY_COMPONENT_HEIGHT);
		partyContainer.setPreferredSize(this.PARTY_COMPONENT_WIDTH, this.PARTY_COMPONENT_HEIGHT);
		partyContainer.setBlendColor(this.Color(1, 1, 1, 0.5));
		partyContainer.setAppearance("Panel");
		local label = this.GUI.Label(memberInfo.name);
		label.setPosition(5, 3);
		partyContainer.add(label);
		this.mHealthBar = this.GUI.AnimatedBar();
		this.mHealthBar.setSize(this.mHealthBarWidth, 15);
		this.mHealthBar.setPreferredSize(this.mHealthBarWidth, 15);
		this.mHealthBar.setPosition(5, 20);
		this.mHealthBar.setAppearance("PlayerHealthBar");
		local buffHolder = this.GUI.Container();
		this.mBuffContainer = this.GUI.ActionContainer("party_buff_container", 1, 8, 0, 0, this, false, 16, 16);
		this.mBuffContainer.setTransparent();
		this.mBuffContainer.setAllButtonsDraggable(false);
		this.mBuffContainer.setPassThru(true);
		this.mBuffContainer.generateTooltipWithPassThru(true);
		buffHolder.setPosition(72, 37);
		buffHolder.add(this.mBuffContainer);
		local debuffHolder = this.GUI.Container();
		this.mDebuffContainer = this.GUI.ActionContainer("party_debuff_container", 3, 7, 0, 0, this, false, 16, 16);
		this.mDebuffContainer.setTransparent();
		this.mDebuffContainer.setAllButtonsDraggable(false);
		this.mDebuffContainer.setPassThru(true);
		this.mDebuffContainer.generateTooltipWithPassThru(true);
		debuffHolder.setPosition(0, 0);
		debuffHolder.add(this.mDebuffContainer);
		partyContainer.add(this.mHealthBar);
		partyContainer.add(buffHolder);
		this.mMenu = this.GUI.PopupMenu();
		this.mMenu.addActionListener(this);
		this._buildMenu();
		this.mLeaderStar = this.GUI.Image("Star6.png");
		this.mLeaderStar.setSize(12, 12);
		this.mLeaderStar.setPreferredSize(12, 12);
		this.mLeaderStar.setPosition(110, 7);
		this.mLeaderStar.setVisible(false);
		partyContainer.add(this.mLeaderStar);
		this.add(partyContainer);
		this.add(debuffHolder);
	}

	function getID()
	{
		return this.mId;
	}

	function getMemberName()
	{
		return this.mMemberName;
	}

	function getMemberID()
	{
		return this.mCreature.getID();
	}

	function isFloating()
	{
		return this.mFloating;
	}

	function onMenuItemPressed( menu, menuID )
	{
		switch(menuID)
		{
		case "Leave":
			::_Connection.sendQuery("party", this, [
				"quit"
			]);
			break;

		case "Kick":
			::_Connection.sendQuery("party", this, [
				"kick",
				this.mId
			]);
			break;

		case "inCharge":
			::_Connection.sendQuery("party", this, [
				"setLeader",
				this.mId
			]);
			break;

		case "Loot":
			this.partyManager.showLootWindow();
			break;

		case "Free":
			this.partyManager.setFreeDrag(true);
			break;

		case "Lock":
			this.partyManager.setFreeDrag(false);
			break;
		}
	}

	function setHealth( percent, ... )
	{
		local anim = vargc == 0 || vargv[0];

		if (percent >= 0.0 && percent <= 1.0)
		{
			local newWidth = this.mHealthBarWidth * percent;
			local healthBarHeight = this.mHealthBar.getSize().height;
			this.mHealthBar.setDestWidth(newWidth, anim);
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local pos = ::Screen.getCursorPos();
			this.mDragged = true;
			this.partyManager.drag(this, pos.x - this.mLastMouse.x, pos.y - this.mLastMouse.y);
			this.mLastMouse = pos;
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		this.mLastMouse = ::Screen.getCursorPos();
		this.mDragging = true;
		this.mDragged = false;
		evt.consume();
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.RBUTTON)
		{
			this._buildMenu();
			this.mMenu.showMenu();
		}
		else if (!this.mDragged)
		{
			local mobs = ::_sceneObjectManager.getCreatures();

			foreach( k, v in mobs )
			{
				if (v.getStat(this.Stat.DISPLAY_NAME) == this.mMemberName)
				{
					::_avatar.setResetTabTarget(true);
					::_Connection.sendSelectTarget(v.getID());
					break;
				}
			}
		}

		this.mDragging = false;
		evt.consume();
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

	function refresh( creature )
	{
		this.mCreature = creature;
		local baseHealth = creature.getStat(this.Stat.BASE_HEALTH, true);
		local curHealth = creature.getStat(this.Stat.HEALTH, true);
		local con = creature.getStat(this.Stat.CONSTITUTION, true);
		local bonusHealth = creature.getStat(this.Stat.HEALTH_MOD, true);
		local maxHealth = ::_combatEquations.calcMaxHealth(baseHealth, con, bonusHealth, 0);

		if (curHealth == null || !maxHealth || maxHealth == 0)
		{
			this.setHealth(1.0, false);
		}
		else
		{
			this.setHealth(curHealth.tofloat() / maxHealth);
		}
	}

	function removeBuffMods( updateContainer )
	{
		if (this.mNextFreeBuffColumn != 7)
		{
			this.mNextFreeBuffColumn = 7;
			this.mBuffContainer.removeAllActions(updateContainer);
			this.mRemovedBuffs = true;
		}
	}

	function removeDebuffMods( updateContainer )
	{
		if (this.mNextFreeDebuffColumn != 0 || this.mNextFreeDebuffRow != 0)
		{
			this.mNextFreeDebuffColumn = 0;
			this.mNextFreeDebuffRow = 0;
			this.mDebuffContainer.removeAllActions(updateContainer);
			this.mRemovedDebuffs = true;
		}
	}

	function setAsLeader( leader )
	{
		this.mLeaderStar.setVisible(leader);
	}

	function updateMods( mods )
	{
		local dirtyBuffContainer = false;
		local dirtyDebuffContainer = false;
		this.removeBuffMods(false);
		this.removeDebuffMods(false);

		foreach( mod in mods )
		{
			if (!(mod instanceof this.BuffDebuff) && !(mod instanceof this.StatusEffect))
			{
				continue;
			}

			local modType = mod.getBuffType();
			local actionContainer;
			local indexToPlaceActionIn = -1;

			if (modType == this.BuffType.BUFF)
			{
				indexToPlaceActionIn = this.mNextFreeBuffColumn;
				actionContainer = this.mBuffContainer;
				dirtyBuffContainer = true;
			}
			else if (modType == this.BuffType.DEBUFF)
			{
				indexToPlaceActionIn = this.mNextFreeDebuffRow * 7 + this.mNextFreeDebuffColumn;
				actionContainer = this.mDebuffContainer;
				dirtyDebuffContainer = true;
			}
			else if (modType == this.BuffType.WORLD)
			{
				return;
			}
			else
			{
				continue;
			}

			actionContainer.addAction(mod, false, indexToPlaceActionIn);

			switch(modType)
			{
			case this.BuffType.BUFF:
				this.mNextFreeBuffColumn--;

				if (this.mNextFreeBuffColumn < 0)
				{
				}

				break;

			default:
				if (modType == this.BuffType.DEBUFF)
				{
					this.mNextFreeDebuffRow++;

					if (this.mNextFreeDebuffRow >= 3)
					{
						this.mNextFreeDebuffRow = 0;
						this.mNextFreeDebuffColumn++;

						if (this.mNextFreeDebuffColumn >= 8)
						{
						}
					}
				}
			}
		}

		if (dirtyBuffContainer || this.mRemovedBuffs)
		{
			this.mBuffContainer.updateContainer();
		}

		if (dirtyDebuffContainer || this.mRemovedDebuffs)
		{
			this.mDebuffContainer.updateContainer();
		}

		this.mRemovedBuffs = false;
		this.mRemovedDebuffs = false;
	}

	function _buildMenu()
	{
		this.mMenu.removeAllMenuOptions();
		this.mMenu.addMenuOption("Leave", "Leave Party");

		if (this.partyManager.isLeader())
		{
			this.mMenu.addMenuOption("Loot", "Change Loot Setting");
			this.mMenu.addMenuOption("Kick", "Remove From Party");
			this.mMenu.addMenuOption("inCharge", "Make Leader");
		}
		else
		{
			this.mMenu.addMenuOption("Loot", "View Loot Setting");
		}

		if (!this.partyManager.isFreeDrag())
		{
			this.mMenu.addMenuOption("Free", "Free Status Bar");
		}
		else
		{
			this.mMenu.addMenuOption("Lock", "Lock Status Bar");
		}
	}

	function _createIndicator( color, x, y )
	{
		local result = this.GUI.ColorSplotch(color, false);
		result.setAppearance("ColorSplotch/Hex");
		result.setSize(11, 11);
		result.setPosition(x, y);
		return result;
	}

}

class this.LootOptions extends this.GUI.Frame
{
	mFreeForAll = null;
	mRoundRobin = null;
	mNeedBeforeGreed = null;
	mIncludeMundane = null;
	constructor()
	{
		this.GUI.Frame.constructor(this.TXT("Loot options"));
		this.setSize(170, 200);
		this.setPreferredSize(170, 200);
		this.setInsets(0, 0, 0, 0);
		local baseContainer = this.GUI.Container(this.GUI.GridLayout(7, 1));
		this.setContentPane(baseContainer);
		baseContainer.setInsets(20, 20, 20, 20);
		baseContainer.getLayoutManager().setRows(20, 10, 20, 20, 20, 5, 20);
		local op1Container = this.GUI.Container(this.GUI.GridLayout(1, 2));
		baseContainer.add(op1Container);
		op1Container.getLayoutManager().setColumns(20, "*");
		this.mFreeForAll = this.GUI.CheckBox("CheckBoxSmall");
		op1Container.add(this.mFreeForAll);
		this.mFreeForAll.addActionListener(this);
		this.mFreeForAll.setReleaseMessage("onFreeForAll");
		op1Container.add(this.GUI.Label(this.TXT("Free for All")));
		baseContainer.add(this.GUI.Spacer(10, 10));
		local op2Container = this.GUI.Container(this.GUI.GridLayout(1, 2));
		baseContainer.add(op2Container);
		op2Container.getLayoutManager().setColumns(20, "*");
		this.mRoundRobin = this.GUI.CheckBox("CheckBoxSmall");
		op2Container.add(this.mRoundRobin);
		this.mRoundRobin.addActionListener(this);
		this.mRoundRobin.setReleaseMessage("onRoundRobin");
		op2Container.add(this.GUI.Label(this.TXT("Round Robin")));
		baseContainer.add(this.GUI.Spacer(10, 10));
		local op3Container = this.GUI.Container(this.GUI.GridLayout(1, 2));
		baseContainer.add(op3Container);
		op3Container.getLayoutManager().setColumns(20, "*");
		this.mNeedBeforeGreed = this.GUI.CheckBox("CheckBoxSmall");
		op3Container.add(this.mNeedBeforeGreed);
		this.mNeedBeforeGreed.addActionListener(this);
		this.mNeedBeforeGreed.setReleaseMessage("onNeedBeforeGreed");
		op3Container.add(this.GUI.Label(this.TXT("Need before Greed")));
		baseContainer.add(this.GUI.Spacer(10, 5));
		local op4Container = this.GUI.Container(this.GUI.GridLayout(1, 2));
		baseContainer.add(op4Container);
		op4Container.getLayoutManager().setColumns(20, "*");
		this.mIncludeMundane = this.GUI.CheckBox("CheckBoxSmall");
		op4Container.add(this.mIncludeMundane);
		this.mIncludeMundane.addActionListener(this);
		this.mIncludeMundane.setReleaseMessage("onIncludeMundane");
		op4Container.add(this.GUI.Label(this.TXT("Include mundane items")));
	}

	function setStrategy( newMode )
	{
		if (newMode == this.LootModes.FREE_FOR_ALL)
		{
			this.mRoundRobin.setChecked(false);
			this.mFreeForAll.setChecked(true);
		}
		else if (newMode == this.LootModes.ROUND_ROBIN)
		{
			this.mRoundRobin.setChecked(true);
			this.mFreeForAll.setChecked(false);
		}
	}

	function setStrategyFlags( newFlags )
	{
		this.mNeedBeforeGreed.setChecked((newFlags & this.LootFlags.NEED_B4_GREED) != 0);
		this.mIncludeMundane.setChecked((newFlags & this.LootFlags.MUNDANE) != 0);
	}

	function onFreeForAll( evt, checked )
	{
		if (checked)
		{
			::_Connection.sendQuery("party", this, [
				"loot.mode",
				this.LootModes.FREE_FOR_ALL
			]);
			this.mRoundRobin.setChecked(false);
		}
		else
		{
			this.mFreeForAll.setChecked(true);
		}
	}

	function onRoundRobin( evt, checked )
	{
		if (checked)
		{
			::_Connection.sendQuery("party", this, [
				"loot.mode",
				this.LootModes.ROUND_ROBIN
			]);
			this.mFreeForAll.setChecked(false);
		}
		else
		{
			this.mRoundRobin.setChecked(true);
		}
	}

	function onNeedBeforeGreed( evt, checked )
	{
		::_Connection.sendQuery("party", this, [
			"loot.flags",
			this.LootFlags.NEED_B4_GREED,
			checked
		]);
	}

	function onIncludeMundane( evt, checked )
	{
		::_Connection.sendQuery("party", this, [
			"loot.flags",
			this.LootFlags.MUNDANE,
			checked
		]);
	}

	function setDefaults()
	{
		this.mFreeForAll.setChecked(true);
		this.mRoundRobin.setChecked(false);
		this.mNeedBeforeGreed.setChecked(false);
		this.mIncludeMundane.setChecked(false);
	}

	function doEnable( value )
	{
		this.mFreeForAll.setEnabled(value);
		this.mRoundRobin.setEnabled(value);
		this.mNeedBeforeGreed.setEnabled(value);
		this.mIncludeMundane.setEnabled(value);
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

}

class this.LootOfferComponent extends this.GUI.Frame
{
	static MAX_TIME = 60 * 1000;
	mLootTag = null;
	mItemDefId = null;
	mTimer = null;
	mTimerLabel = null;
	mNeedButton = null;
	mGreedButton = null;
	mPassButton = null;
	mLabel = null;
	mWaiting = false;
	mLootOfferCont = null;
	mCastingBarBG = null;
	mCastingBar = null;
	mIsButtonsDisabled = false;
	constructor( lootTag, itemDefId, needed )
	{
		this.GUI.Frame.constructor(this.TXT("Loot available"));
		this.mLootTag = lootTag;
		this.mItemDefId = itemDefId;
		this.mTimer = ::Timer();
		local itemdef = ::_ItemManager.getItemDef(this.mItemDefId);
		this.setSize(256, 125);
		this.setPreferredSize(256, 125);
		local cmain = this.GUI.Container(null);
		this.setContentPane(cmain);
		local tempContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		tempContainer.setPosition(22, 5);
		tempContainer.setSize(200, 60);
		tempContainer.setPreferredSize(200, 60);
		cmain.add(tempContainer);
		this.mLootOfferCont = this.GUI.ActionContainer("lootOffer_container", 1, 1, 0, 0, this, false);
		this.mLootOfferCont.setItemPanelVisible(true);
		this.mLootOfferCont.setAllButtonsDraggable(false);
		tempContainer.add(this.mLootOfferCont);
		local actionButton = this.mLootOfferCont.addAction(itemdef, true);
		local need = this.mNeedButton = this.GUI.NarrowButton("Need");
		cmain.add(need);
		need.setFixedSize(72, 32);
		need.setPosition(12, 60);
		need.setEnabled(needed);
		need.setReleaseMessage("onNeed");
		need.addActionListener(this);
		local greed = this.mGreedButton = this.GUI.NarrowButton("Greed");
		cmain.add(greed);
		greed.setFixedSize(72, 32);
		greed.setPosition(90, 60);
		greed.setReleaseMessage("onGreed");
		greed.addActionListener(this);
		local pass = this.mPassButton = this.GUI.NarrowButton("Pass");
		cmain.add(pass);
		pass.setFixedSize(72, 32);
		pass.setPosition(170, 60);
		pass.setReleaseMessage("onPass");
		pass.addActionListener(this);
		this.mCastingBarBG = this.GUI.Container(null);
		this.mCastingBarBG.setSize(162, 21);
		this.mCastingBarBG.setPreferredSize(162, 21);
		this.mCastingBarBG.setPosition(40, 85);
		this.mCastingBarBG.setAppearance("DefaultTargetAndCastingBarBG");
		cmain.add(this.mCastingBarBG);
		this.mCastingBar = this.GUI.Container(null);
		this.mCastingBar.setSize(114, 10);
		this.mCastingBar.setPreferredSize(114, 10);
		this.mCastingBar.setPosition(45, 6);
		this.mCastingBar.setAppearance("DefaultTargetAndCastingBar");
		this.mCastingBarBG.add(this.mCastingBar);
		local timerLabel = this.mTimerLabel = this.GUI.HTML();
		this.mCastingBarBG.add(timerLabel);
		timerLabel.setPosition(7, 3);
		timerLabel.setText("<font color=\"FFFFFF\"><b>" + this.TXT("00::60 Seconds") + "</b> " + "</font>");
		this._enterFrameRelay.addListener(this);
	}

	function close()
	{
		this._enterFrameRelay.removeListener(this);
		this.mTimer = null;
		this.mLootOfferCont.removeAllActions();
		this.GUI.Frame.close();
	}

	function onEnterFrame()
	{
		local msecs = this.MAX_TIME - this.mTimer.getMilliseconds();
		local barSec = this.MAX_TIME.tofloat() - this.mTimer.getMilliseconds().tofloat();

		if (barSec > 0)
		{
			local percent = barSec.tofloat() / this.MAX_TIME.tofloat();
			local castingBarSize = this.mCastingBar.getSize();
			local newWidth = 114.0 * percent;
			this.mCastingBar.setSize(newWidth, castingBarSize.height);
		}

		if (msecs <= 0)
		{
			this.onPass(null);
			return;
		}

		local secs = msecs / 1000;
		local mins = secs / 60;
		secs -= mins * 60;
		this.mTimerLabel.setText("<font color=\"FFFFFF\"><b>00:" + secs.tostring() + "</b>" + this.TXT("Seconds") + "</font>");
	}

	function onNeed( evt )
	{
		::_Connection.sendQuery("party", this, [
			"loot.need",
			this.mLootTag
		]);
		this._disable();
	}

	function onGreed( evt )
	{
		::_Connection.sendQuery("party", this, [
			"loot.greed",
			this.mLootTag
		]);
		this._disable();
	}

	function onPass( evt )
	{
		::_Connection.sendQuery("party", this, [
			"loot.pass",
			this.mLootTag
		]);
		this.partyManager._closeRollWindow(this.mLootTag);
	}

	function _disable()
	{
		this.mIsButtonsDisabled = true;
		this.mNeedButton.setEnabled(false);
		this.mGreedButton.setEnabled(false);
		this.mPassButton.setEnabled(false);
	}

	function onClosePressed()
	{
		if (!this.mIsButtonsDisabled)
		{
			this.onPass(null);
		}
		else
		{
			this.setVisible(false);
		}
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

}

class this.PartyManager 
{
	mMemberComponents = [];
	mOfferComponents = [];
	mDialogs = [];
	mMembers = [];
	mLeaderID = 0;
	mMemberID = 0;
	mHooked = false;
	mFreeDrag = false;
	mLootWindow = null;
	mIsInParty = false;
	mMessageBroadcaster = null;
	constructor()
	{
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function addListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function onCreatureUpdated( callingFunction, creature )
	{
		local name = creature.getStat(this.Stat.DISPLAY_NAME);

		if (name == null)
		{
			return;
		}

		foreach( mc in this.mMemberComponents )
		{
			if (mc.getMemberName() == name)
			{
				mc.refresh(creature);
				this.mMessageBroadcaster.broadcastMessage("updatePartyPosition", creature, name);
			}
		}
	}

	function isCreaturePartyMember( creature )
	{
		local name = creature.getStat(this.Stat.DISPLAY_NAME);

		if (name == null)
		{
			return false;
		}

		foreach( mc in this.mMemberComponents )
		{
			if (mc.getMemberName() == name)
			{
				return true;
			}
		}
	}

	function isLeader()
	{
		return this.mLeaderID == this.mMemberID;
	}

	function getMemberComponent( memberID )
	{
		foreach( partyMember in this.mMemberComponents )
		{
			if (partyMember.getMemberID() == memberID)
			{
				return partyMember;
			}
		}

		return null;
	}

	function isFreeDrag()
	{
		return this.mFreeDrag;
	}

	function setFreeDrag( free )
	{
		this.mFreeDrag = free;

		if (!this.mFreeDrag)
		{
			this._updatePositions();
		}
	}

	function invited( name, id )
	{
		local callback = {
			leaderId = id,
			function onActionSelected( mb, alt )
			{
				if (alt == "Yes")
				{
					::_Connection.sendQuery("party", this, [
						"accept.invite",
						this.leaderId
					]);
				}
				else
				{
					::_Connection.sendQuery("party", this, [
						"reject.invite",
						this.leaderId
					]);
				}

				this.partyManager.removeDialog(mb);
			}

			function onQueryError( qa, reason )
			{
				this.IGIS.error(reason);
			}

		};
		::_tutorialManager.onSocialEvent("PartyInvite");
		local mb = this.GUI.MessageBox.showYesNo(name + " has invited you to a party.", callback);
		::Audio.playSound("Sound-Partyinvite.ogg");
		this._addDialog(mb);
	}

	function invitationRejected( name )
	{
		this.IGIS.info(name + " has rejected your party invitation.");
	}

	function joinParty( party, leaderID, memberID )
	{
		this.mIsInParty = true;

		if (!this.mHooked)
		{
			this.mHooked = true;
			::_Connection.addListener(this);
		}

		this.clear();
		this.mMemberID = memberID;
		this.setLeader(leaderID);

		foreach( pm in party )
		{
			this.addMember(pm);
		}

		local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

		if (selfTargetWindow)
		{
			selfTargetWindow.showPartyGem(true);

			if (this.isLeader())
			{
				selfTargetWindow.showPartyLeader(true);
			}
		}

		this._buildLootWindow();
		this.mLootWindow.setDefaults();
		this._updatePositions();
	}

	function leaveParty()
	{
		this.mIsInParty = false;
		this.clear();
		local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

		if (selfTargetWindow)
		{
			selfTargetWindow.showPartyGem(false);
			selfTargetWindow.showPartyLeader(false);
		}

		this.IGIS.info("You have left your party.");
	}

	function setLeader( leaderID )
	{
		this.mLeaderID = leaderID;
		local leader = this.isLeader();
		this._buildLootWindow();
		this.mLootWindow.doEnable(leader);

		foreach( mc in this.mMemberComponents )
		{
			mc.setAsLeader(mc.getID() == leaderID);
		}

		local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

		if (selfTargetWindow)
		{
			selfTargetWindow.showPartyLeader(leader);
		}
	}

	function inCharge( leaderId, leaderName )
	{
		this.setLeader(leaderId);
		local leader = this.isLeader();

		if (leader)
		{
			this.IGIS.info("You are the leader of your party.");
		}
		else
		{
			this.IGIS.info(leaderName + " is the leader of the party.");
		}
	}

	function drag( cmp, dx, dy )
	{
		if (this.mFreeDrag)
		{
			this._doDrag(cmp, dx, dy);
		}
		else
		{
			foreach( mc in this.mMemberComponents )
			{
				this._doDrag(mc, dx, dy);
			}
		}
	}

	function _doDrag( cmp, dx, dy )
	{
		local pos = cmp.getPosition();
		cmp.setPosition(pos.x + dx, pos.y + dy);
		this.fitToScreen(cmp);
	}

	function fitToScreen( cmp )
	{
		local pos = cmp.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		local right = pos.x + cmp.getWidth();
		local bottom = pos.y + cmp.getHeight();
		pos.x = right > ::Screen.getWidth() ? ::Screen.getWidth() - cmp.getWidth() : pos.x;
		pos.y = bottom > ::Screen.getHeight() ? ::Screen.getHeight() - cmp.getHeight() : pos.y;
		cmp.setPosition(pos);
	}

	function getPartyComponent( memberId )
	{
		foreach( partyComp in this.mMemberComponents )
		{
			if (partyComp.getID() == memberId)
			{
				return partyComp;
			}
		}

		return null;
	}

	function addMember( memberInfo )
	{
		local cmp = this.getPartyComponent(memberInfo.id);

		if (!cmp)
		{
			cmp = this.PartyComponent(memberInfo);
			this.mMemberComponents.append(cmp);
			this.mMembers.append(memberInfo);
		}

		cmp.setVisible(true);
		local creature = ::_sceneObjectManager.getCreatureByID(memberInfo.id);

		if (creature)
		{
			cmp.refresh(creature);
		}

		cmp.setAsLeader(cmp.getID() == this.mLeaderID);
		this.mMessageBroadcaster.broadcastMessage("updatePartyMarkers");

		if (!this.mFreeDrag)
		{
			this._updatePositions();
		}
	}

	function getMemberSceneObject( memberNumber )
	{
		if (this.mMemberComponents.len() >= memberNumber)
		{
			local memberComponent = this.mMemberComponents[memberNumber];

			if (memberComponent)
			{
				local mobs = ::_sceneObjectManager.getCreatures();
				local memberName = memberComponent.getMemberName();

				foreach( k, v in mobs )
				{
					if (v.getStat(this.Stat.DISPLAY_NAME) == memberName)
					{
						return v;
					}
				}
			}
		}
	}

	function removeMember( memberId )
	{
		local x;

		for( x = 0; x < this.mMembers.len(); x++ )
		{
			local m = this.mMembers[x];

			if (m.id == memberId)
			{
				this.mMembers.remove(x);
				break;
			}
		}

		for( x = 0; x < this.mMemberComponents.len(); x++ )
		{
			local m = this.mMemberComponents[x];

			if (m.getID() == memberId)
			{
				this.mMemberComponents.remove(x);
				m.setVisible(false);
				m.destroy();
				break;
			}
		}

		this.mMessageBroadcaster.broadcastMessage("updatePartyMarkers");

		if (!this.mFreeDrag)
		{
			this._updatePositions();
		}
	}

	function isMemberInParty( memberId )
	{
		local x;

		for( x = 0; x < this.mMembers.len(); x++ )
		{
			local m = this.mMembers[x];

			if (m.id == memberId)
			{
				return true;
			}
		}

		return false;
	}

	function getPartyMembers()
	{
		return this.mMembers;
	}

	function isCharacterInParty( characterId, mMenu )
	{
		local queryCallback = {
			mMenu = mMenu,
			function onQueryComplete( qa, results )
			{
				if (::Util.atob(results[0][0]) == true)
				{
					this.mMenu.showMenu();
				}
			}

		};
		::_Connection.sendQuery("party.ismember", queryCallback, [
			::_avatar.getCreatureDef().getID(),
			characterId
		]);
	}

	function isInParty()
	{
		return this.mIsInParty;
	}

	function proposeInvitation( proposeeId, proposeeName, proposerId, proposerName )
	{
		local callback = {
			id = proposeeId,
			function onActionSelected( mb, alt )
			{
				if (alt == "Yes")
				{
					::_Connection.sendQuery("party", this, [
						"invite.player",
						this.id
					]);
				}

				this.partyManager.removeDialog(mb);
			}

			function onQueryComplete( qa, results )
			{
			}

			function onQueryError( qa, reason )
			{
				this.IGIS.error(reason);
			}

		};
		local mb = this.GUI.MessageBox.showYesNo(proposerName + " has suggested you invite " + proposeeName + " to join your party.", callback);
		this._addDialog(mb);
	}

	function strategyChange( newMode )
	{
		this._buildLootWindow();
		this.mLootWindow.setStrategy(newMode);
	}

	function strategyFlagsChange( newMode )
	{
		this._buildLootWindow();
		this.mLootWindow.setStrategyFlags(newMode);
	}

	function offerLoot( lootTag, itemDefId, needed )
	{
		local cmp = this.LootOfferComponent(lootTag, itemDefId, needed);
		cmp.setVisible(true);
		local existing = this.mOfferComponents.len();
		local pos;

		if (existing > 0)
		{
			local last = this.mOfferComponents[existing - 1];
			pos = last.getPosition();
			cmp.setPosition(pos.x, pos.y + last.getHeight());
		}
		else if (this.mLootWindow && this.mLootWindow.isVisible())
		{
			pos = this.mLootWindow.getPosition();
			cmp.setPosition(pos.x, pos.y + this.mLootWindow.getHeight());
		}
		else
		{
			cmp.setPosition(134, 140);
		}

		this.mOfferComponents.append(cmp);
	}

	function lootWon( lootTag, originalTag, winner )
	{
		foreach( oc in this.mOfferComponents )
		{
			if (oc.mLootTag == originalTag)
			{
				local itemdef = ::_ItemDataManager.getItemDef(oc.mItemDefId);
				this.IGIS.info(winner + " won " + itemdef.mDisplayName);
			}
		}

		this._closeRollWindow(originalTag);
	}

	function _closeRollWindow( lootTag )
	{
		local idx = 0;

		foreach( oc in this.mOfferComponents )
		{
			if (oc.mLootTag == lootTag)
			{
				oc.close();
				oc.setVisible(false);
				oc.destroy();
				this.mOfferComponents.remove(idx);
				break;
			}

			idx++;
		}
	}

	function _addDialog( mb )
	{
		this.mDialogs.append(mb);
		this._updatePositions();
	}

	function _updatePositions()
	{
		local currPos = {
			x = 0,
			y = 140
		};

		foreach( mc in this.mMemberComponents )
		{
			if (!mc.isFloating())
			{
				mc.setPosition(currPos.x, currPos.y);
				currPos.y += mc.getHeight();
			}
		}

		foreach( dlg in this.mDialogs )
		{
			dlg.setPosition(currPos.x, currPos.y);
			currPos.y += dlg.getHeight();
		}
	}

	function clear()
	{
		this.closeDialogs();

		foreach( mc in this.mMemberComponents )
		{
			mc.setVisible(false);
			mc.destroy();
		}

		foreach( oc in this.mOfferComponents )
		{
			oc.close();
			oc.setVisible(false);
			oc.destroy();
		}

		this.mMembers = [];
		this.mMemberComponents = [];
		this.mOfferComponents = [];
		this.mLeaderID = 0;
		this.mMemberID = 0;
		this.mMessageBroadcaster.broadcastMessage("updatePartyMarkers");
	}

	function removeDialog( mb )
	{
		local idx = -1;
		local curr = 0;

		foreach( dlg in this.mDialogs )
		{
			if (dlg == mb)
			{
				idx = curr;
				break;
			}

			curr++;
		}

		if (idx >= 0)
		{
			this.mDialogs.remove(idx);
		}

		this._updatePositions();
	}

	function closeDialogs()
	{
		foreach( d in this.mDialogs )
		{
			d.close();
		}

		this.mDialogs = [];
	}

	function showLootWindow()
	{
		this._buildLootWindow();
		this.mLootWindow.setVisible(true);
	}

	function invite( creatureId )
	{
		::_Connection.sendQuery("party", this, [
			"invite",
			creatureId
		]);
	}

	function _buildLootWindow()
	{
		if (!this.mLootWindow)
		{
			this.mLootWindow = this.LootOptions();
			this.mLootWindow.setPosition(134, 140);
			this.mLootWindow.doEnable(this.isLeader());
			this.mLootWindow.setDefaults();
		}
	}

	function onQueryError( qa, reason )
	{
		this.IGIS.error(reason);
	}

}

