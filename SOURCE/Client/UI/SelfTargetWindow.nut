this.require("AbilityManager");
this.require("UI/Screens");
class this.Screens.SelfTargetWindow extends this.GUI.Component
{
	static MOVEMENT_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static MOVEMENT_NORMAL = this.Color(0.31999999, 0.12, 0.44999999, 1);
	static STAT_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static STAT_NORMAL = this.Color(0.60000002, 0.12, 0.14, 1);
	static OTHER_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static OTHER_NORMAL = this.Color(0.75999999, 0.44999999, 0.1, 1);
	static BUFF_HIGHLIGHT = this.Color(1.0, 0.0, 0.0, 1);
	static BUFF_NORMAL = this.Color(0, 0.80000001, 0, 1);
	MAX_WILL_POINTS = 10;
	MAX_MIGHT_POINTS = 10;
	mHealthBarStartX = 78.5;
	mHealthBarStartY = 37;
	mMightPointsStartX = 82;
	mMightPointsStartY = 17;
	mWillPointsStartX = 82;
	mWillPointsStartY = 53.5;
	mLevelMiddleXPos = 37;
	mLevelMiddleYPos = 53;
	mMightChargeMiddleXPos = 8;
	mMightChargeStartYPos = 38;
	mWillChargeMiddleXPos = 72;
	mWillChargeStartYPos = 38;
	mHealthTextMiddleXPos = 155.5;
	mHealthTextStartYPos = 47;
	mDragging = false;
	mBackground = null;
	mSelfTargetWindowContainer = null;
	mMouseOffset = {};
	mMightCharges = null;
	mWillCharges = null;
	mPrevMightCharges = -1;
	mPrevWillCharges = -1;
	mMightChargePoints = null;
	mWillChargePoints = null;
	mLevelLabel = null;
	mHealthBar = null;
	mHealthBarText = null;
	mMightPointContainer = null;
	mWillPointContainer = null;
	mMaxHealthSize = 0;
	mPartyCastGem = null;
	mPartyLeader = null;
	mBlinkingPips = null;
	mBlinkEvent = null;
	mBlinkdownEvent = null;
	mBlinkdown = false;
	mPipBlinkOn = true;
	mMightMaxed = false;
	mWillMaxed = false;
	mInactiveGem = null;
	mStatus = null;
	mPartyMenu = null;
	mName = null;
	static MAX_CHARGES = 5;
	constructor()
	{
		this.GUI.Component.constructor(null);
		this.mMightCharges = [];
		this.mWillCharges = [];
		this.mBlinkingPips = [];
		this.mMightCharges = this.array(this.MAX_CHARGES);
		this.mWillCharges = this.array(this.MAX_CHARGES);
		this.mMightChargePoints = this.array(10);
		this.mWillChargePoints = this.array(10);
		this.setFont(this.GUI.Font("Maiandra", 12));
		this.setSize(250, 85);
		this.setPreferredSize(250, 85);
		this.setPosition(20, 24);
		this.mPartyCastGem = this.GUI.CheckBox();
		this.mPartyCastGem.setAppearance("PartyCheckBox");
		this.mPartyCastGem.setLayoutExclude(true);
		this.mPartyCastGem.setTooltip("If selected, positive spells will be cast on your party members");
		this.mPartyCastGem.setSize(31, 29);
		this.mPartyCastGem.setPreferredSize(31, 29);
		this.mPartyCastGem.setPosition(26.5, 58);
		this.mPartyCastGem.addActionListener(this);
		this.mPartyCastGem.setReleaseMessage("onPartyClicked");
		this.mPartyCastGem.setVisible(false);
		this.mSelfTargetWindowContainer = this._buildMainBackground();
		this.add(this.mSelfTargetWindowContainer);
		this.mInactiveGem = this.GUI.Component(null);
		this.mInactiveGem.setPassThru(true);
		this.mInactiveGem.setSize(31, 29);
		this.mInactiveGem.setPreferredSize(31, 29);
		this.mInactiveGem.setPosition(26.5, 58);
		this.mInactiveGem.setAppearance("PartyCheckBox/Inactive");
		this.mInactiveGem.setVisible(false);
		this.setCached(::Pref.get("video.UICache"));
		this.mPartyMenu = this.GUI.PopupMenu();
		this.mPartyMenu.addActionListener(this);
	}

	function endCombatBlinkdown()
	{
		this.mBlinkdown = false;
		::_eventScheduler.cancel(this.mBlinkEvent);
		::_eventScheduler.cancel(this.mBlinkdownEvent);
		this.mBlinkEvent = null;
		this.mBlinkdownEvent = null;
		this.mBlinkingPips.clear();
		this.mMightMaxed = false;
		this.mWillMaxed = false;
		this.updateMightCharges(true);
		this.updateWillCharges(true);
	}

	function endPipBlink( pip )
	{
		local index = this.Util.indexOf(this.mBlinkingPips, pip);

		if (index != null)
		{
			this.mBlinkingPips.remove(index);
		}
	}

	function fillOut()
	{
		this.updateWillCharges();
		this.updateWillPoints();
		this.updateMightCharges();
		this.updateMightPoints();

		if (this.mBlinkdown)
		{
			this.pipBlinkdown();
		}

		this.updateLevel();
		this.updateHealth();

		if (::_avatar)
		{
			local name = ::_avatar.getName();
			this.mName.setText(name);
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

	function setHealth( curHealth, maxHealth, ... )
	{
		local anim = vargc == 0 || vargv[0];
		local percent = curHealth.tofloat() / maxHealth.tofloat();

		if (percent >= 0.0 && percent <= 1.0)
		{
			local newWidth = this.mMaxHealthSize * percent;
			local healthBarHeight = this.mHealthBar.getSize().height;
			this.mHealthBar.setPosition(this.mHealthBarStartX, this.mHealthBarStartY);
			this.mHealthBar.setDestWidth(newWidth, anim);
			this.mHealthBarText.setText(curHealth.tostring() + "/" + maxHealth.tostring());
			this._centerHealthBarText();
		}
	}

	function setLevel( level )
	{
		if (level != null)
		{
			this.mLevelLabel.setText("<font size=\"18\"><b>" + level.tostring() + "</b></font>");
			this._centerLevelText();
		}
	}

	function setMightCharges( charges, ... )
	{
		local chargesToSet = charges;

		if (null == chargesToSet)
		{
			chargesToSet = 0;
		}
		else if (chargesToSet > 5)
		{
			chargesToSet = 5;
		}

		local forceUpdate = false;

		if (vargc > 0)
		{
			forceUpdate = vargv[0];
		}

		if (chargesToSet == this.mPrevMightCharges && !forceUpdate)
		{
			return;
		}

		this.mPrevMightCharges = chargesToSet;
		local mightCurMaxed = false;

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			if (i < chargesToSet)
			{
				this.mMightCharges[i].setVisible(true);
			}
			else
			{
				this.mMightCharges[i].setVisible(false);
			}
		}

		if (chargesToSet == 5)
		{
			mightCurMaxed = true;
		}

		if (this.mMightMaxed != mightCurMaxed)
		{
			this.mMightMaxed = mightCurMaxed;

			if (this.mMightMaxed)
			{
				for( local i = 0; i < this.MAX_CHARGES; i++ )
				{
					if (i < 4)
					{
						this.startPipBlink(this.mMightCharges[i], false);
					}
					else
					{
						this.startPipBlink(this.mMightCharges[i], true);
					}
				}
			}
			else if (!this.mBlinkdown)
			{
				for( local i = 0; i < this.MAX_CHARGES; i++ )
				{
					this.endPipBlink(this.mMightCharges[i]);
				}
			}
		}
	}

	function setMightPoints( points )
	{
		local pointsToSet = points;

		if (points < 0)
		{
			points = 0;
		}
		else if (points > this.MAX_MIGHT_POINTS)
		{
			points = this.MAX_MIGHT_POINTS;
		}

		local i = 0;

		while (i < points)
		{
			this.mMightChargePoints[i].setVisible(true);
			i++;
		}

		while (i < this.MAX_MIGHT_POINTS)
		{
			this.mMightChargePoints[i].setVisible(false);
			i++;
		}
	}

	function setWillCharges( charges, ... )
	{
		local chargesToSet = charges;

		if (null == chargesToSet)
		{
			chargesToSet = 0;
		}
		else if (chargesToSet > 5)
		{
			chargesToSet = 5;
		}

		local forceUpdate = false;

		if (vargc > 0)
		{
			forceUpdate = vargv[0];
		}

		if (chargesToSet == this.mPrevWillCharges && !forceUpdate)
		{
			return;
		}

		this.mPrevWillCharges = chargesToSet;
		local willCurMaxed = false;

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			if (i < chargesToSet)
			{
				this.mWillCharges[i].setVisible(true);
			}
			else
			{
				this.mWillCharges[i].setVisible(false);
			}
		}

		if (chargesToSet == 5)
		{
			willCurMaxed = true;
		}

		if (this.mWillMaxed != willCurMaxed)
		{
			this.mWillMaxed = willCurMaxed;

			if (this.mWillMaxed)
			{
				for( local i = 0; i < this.MAX_CHARGES; i++ )
				{
					if (i < 4)
					{
						this.startPipBlink(this.mWillCharges[i], false);
					}
					else
					{
						this.startPipBlink(this.mWillCharges[i], true);
					}
				}
			}
			else if (!this.mBlinkdown)
			{
				for( local i = 0; i < this.MAX_CHARGES; i++ )
				{
					this.endPipBlink(this.mWillCharges[i]);
				}
			}
		}
	}

	function setWillPoints( points )
	{
		local pointsToSet = points;

		if (points < 0)
		{
			points = 0;
		}
		else if (points > this.MAX_WILL_POINTS)
		{
			points = this.MAX_WILL_POINTS;
		}

		local i = 0;

		while (i < points)
		{
			this.mWillChargePoints[i].setVisible(true);
			i++;
		}

		while (i < this.MAX_WILL_POINTS)
		{
			this.mWillChargePoints[i].setVisible(false);
			i++;
		}
	}

	function showPartyGem( vis )
	{
		this.mPartyCastGem.setVisible(vis);
		this.mPartyCastGem.setChecked(false);
		this.mInactiveGem.setVisible(vis);
	}

	function startCombatEndBlinkdown()
	{
		foreach( mightCharge in this.mMightCharges )
		{
			this.endPipBlink(mightCharge);
		}

		foreach( willCharge in this.mWillCharges )
		{
			this.endPipBlink(willCharge);
		}

		this.updateMightCharges(true);
		this.updateWillCharges(true);
		this.mBlinkdownEvent = ::_eventScheduler.fireIn(7.0, this, "pipBlinkdown");
	}

	function startPipBlink( pip, ... )
	{
		local index = this.Util.indexOf(this.mBlinkingPips, pip);

		if (index == null)
		{
			this.mBlinkingPips.push(pip);
			local scheduleIfNeeded = true;

			if (vargc > 0)
			{
				scheduleIfNeeded = vargv[0];
			}

			if (this.mBlinkEvent == null && scheduleIfNeeded)
			{
				this.mBlinkEvent = ::_eventScheduler.repeatIn(0.0, 0.5, this, "pipBlink");
			}
		}
	}

	function uncheckPartyGem()
	{
		this.mPartyCastGem.setChecked(false);
		this.mInactiveGem.setVisible(true);
	}

	function showPartyLeader( vis )
	{
		this.mPartyLeader.setVisible(vis);
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging)
		{
			local newpos = ::Screen.getCursorPos();
			local deltax = newpos.x - this.mMouseOffset.x;
			local deltay = newpos.y - this.mMouseOffset.y;
			local pos = this.getPosition();
			pos.x += deltax;
			pos.y += deltay;
			this.mMouseOffset = newpos;
			this.setPosition(pos);
			this.fitToScreen();
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		this.mMouseOffset = ::Screen.getCursorPos();
		this.mDragging = true;
		evt.consume();
	}

	function onMouseReleased( evt )
	{
		this.mDragging = false;

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			evt.consume();
		}
		else if (evt.button == this.MouseEvent.RBUTTON)
		{
			if (::partyManager.isInParty())
			{
				this.mPartyMenu.removeAllMenuOptions();
				this.mPartyMenu.addMenuOption("Leave", "Leave Party");

				if (::partyManager.isLeader())
				{
					this.mPartyMenu.addMenuOption("Loot", "Change Loot Setting");
					this.mPartyMenu.showMenu();
				}
				else
				{
					this.mPartyMenu.addMenuOption("Loot", "View Loot Setting");
					::partyManager.isCharacterInParty(::_avatar.getID(), this.mPartyMenu);
				}
			}
		}
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

		case "Loot":
			::partyManager.showLootWindow();
			break;
		}
	}

	function onPartyClicked( evt, checked )
	{
		::_avatar.setResetTabTarget(true);
		this._avatar.setTargetObject(null);
		::_AbilityManager.setPartyCasting(checked);
		this.mInactiveGem.setVisible(!checked);
	}

	function pipBlink()
	{
		this.mPipBlinkOn = !this.mPipBlinkOn;

		foreach( pip in this.mBlinkingPips )
		{
			pip.setVisible(this.mPipBlinkOn);
		}
	}

	function pipBlinkdown()
	{
		if (!::_avatar)
		{
			return;
		}

		this.mBlinkdown = true;
		local mightCharges = ::_avatar.getStat(this.Stat.MIGHT_CHARGES);
		local willCharges = ::_avatar.getStat(this.Stat.WILL_CHARGES);

		switch(mightCharges)
		{
		case 5:
			this.startPipBlink(this.mMightCharges[mightCharges - 1]);
			break;

		case 4:
		case 3:
		case 2:
		case 1:
			for( local i = 4; i >= mightCharges; i-- )
			{
				this.endPipBlink(this.mMightCharges[i]);
			}

			this.startPipBlink(this.mMightCharges[mightCharges - 1]);
			break;

		case 0:
			for( local i = 4; i >= 0; i-- )
			{
				this.endPipBlink(this.mMightCharges[i]);
			}

			break;
		}

		switch(willCharges)
		{
		case 5:
			this.startPipBlink(this.mWillCharges[willCharges - 1]);
			break;

		case 4:
		case 3:
		case 2:
		case 1:
			for( local i = 4; i >= willCharges; i-- )
			{
				this.endPipBlink(this.mWillCharges[i]);
			}

			this.startPipBlink(this.mWillCharges[willCharges - 1]);
			break;

		case 0:
			for( local i = 4; i >= 0; i-- )
			{
				this.endPipBlink(this.mWillCharges[i]);
			}

			break;
		}
	}

	function updateLevel()
	{
		if (::_avatar)
		{
			this.setLevel(::_avatar.getStat(this.Stat.LEVEL, true));
		}
	}

	function updateMightCharges( ... )
	{
		if (::_avatar)
		{
			local forceUpdate = false;

			if (vargc > 0)
			{
				forceUpdate = vargv[0];
			}

			this.setMightCharges(::_avatar.getStat(this.Stat.MIGHT_CHARGES, true), forceUpdate);
		}
	}

	function updateMightPoints()
	{
		if (::_avatar)
		{
			this.setMightPoints(::_avatar.getStat(this.Stat.MIGHT, true));
		}
	}

	function updateHealth()
	{
		if (!::_avatar)
		{
			return;
		}

		local con = ::_avatar.getStat(this.Stat.CONSTITUTION, true);
		local baseHealth = ::_avatar.getStat(this.Stat.BASE_HEALTH, true);
		local bonusHealth = ::_avatar.getStat(this.Stat.HEALTH_MOD, true);
		local maxHealth = ::_combatEquations.calcMaxHealth(baseHealth, con, bonusHealth, 0);
		local curHealth = ::_avatar.getStat(this.Stat.HEALTH, true);

		if (maxHealth != 0)
		{
			if (curHealth > 0 || curHealth == 0 && ::_avatar.isDead())
			{
				this.setHealth(curHealth, maxHealth);
			}
		}
	}

	function updateWillCharges( ... )
	{
		if (::_avatar)
		{
			local forceUpdate = false;

			if (vargc > 0)
			{
				forceUpdate = vargv[0];
			}

			this.setWillCharges(::_avatar.getStat(this.Stat.WILL_CHARGES, true), forceUpdate);
		}
	}

	function updateWillPoints()
	{
		if (::_avatar)
		{
			local will = ::_avatar.getStat(this.Stat.WILL, true);

			if (will != null)
			{
				this.setWillPoints(will);
			}
		}
	}

	function _buildMainBackground()
	{
		local container = this.GUI.Container(null);
		container.setSize(257, 71);
		container.setPreferredSize(257, 71);
		this.mBackground = this.GUI.Container(null);
		this.mBackground.setSize(250, 76);
		this.mBackground.setPreferredSize(250, 76);
		this.mMaxHealthSize = 158;
		this.mBackground.setAppearance("PlayerBG");
		container.add(this.mBackground);
		local targetNameFont = this.GUI.Font("MaiandraOutline", 22);
		this.mName = this.GUI.Label("");
		this.mName.setFontColor(this.Colors.white);
		this.mName.setFont(targetNameFont);
		this.mName.setPosition(66, 0);
		this.mName.setSize(180, 20);
		this.mName.setPreferredSize(180, 20);
		this.mName.setAutoFit(true);
		this.mName.setTextAlignment(0.5, 1.0);
		this.mBackground.add(this.mName);

		if (::_avatar)
		{
			local name = ::_avatar.getName();
			this.mName.setText(name);
		}

		local CHARGES_SIZE = 13;
		local mightPositions = {
			[0] = {
				x = 13,
				y = 42
			},
			[1] = {
				x = 4,
				y = 34
			},
			[2] = {
				x = 3,
				y = 23
			},
			[3] = {
				x = 8.5,
				y = 13
			},
			[4] = {
				x = 15,
				y = 4
			}
		};
		local willPositions = {
			[0] = {
				x = 55,
				y = 42
			},
			[1] = {
				x = 61,
				y = 34
			},
			[2] = {
				x = 63,
				y = 23
			},
			[3] = {
				x = 57,
				y = 13
			},
			[4] = {
				x = 50,
				y = 4
			}
		};

		for( local i = 0; i < this.MAX_CHARGES; i++ )
		{
			this.mMightCharges[i] = this.GUI.Component();
			this.mMightCharges[i].setTooltip("Melee Charge");
			this.mMightCharges[i].setSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mMightCharges[i].setPreferredSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mMightCharges[i].setPosition(mightPositions[i].x, mightPositions[i].y);
			this.mMightCharges[i].setAppearance("PlayerMight");
			this.mBackground.add(this.mMightCharges[i]);
			this.mWillCharges[i] = this.GUI.Container();
			this.mWillCharges[i].setTooltip("Magic Charge");
			this.mWillCharges[i].setSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mWillCharges[i].setPreferredSize(CHARGES_SIZE, CHARGES_SIZE);
			this.mWillCharges[i].setPosition(willPositions[i].x, willPositions[i].y);
			this.mWillCharges[i].setAppearance("PlayerWill");
			this.mBackground.add(this.mWillCharges[i]);
		}

		this.mHealthBar = this.GUI.AnimatedBar();
		this.mHealthBar.setSize(158, 15);
		this.mHealthBar.setPreferredSize(158, 15);
		this.mHealthBar.setPosition(this.mHealthBarStartX, this.mHealthBarStartY);
		this.mHealthBar.setAppearance("PlayerHealthBar");
		this.mHealthBar.setTooltip("Health bar");
		this.mLevelLabel = this.GUI.HTML("");
		this.mLevelLabel.setSize(15, 20);
		this.mLevelLabel.setPreferredSize(15, 20);
		this.mLevelLabel.setTooltip("Level");
		this.mLevelLabel.setChildrenInheritTooltip(true);
		local healthBarFont = this.GUI.Font("MaiandraOutline", 22);
		this.mHealthBarText = this.GUI.Label("");
		this.mHealthBarText.setFont(healthBarFont);
		this.mHealthBarText.setFontColor(this.Colors.white);
		this.mHealthBarText.setTooltip("Health bar");
		this.mMightPointContainer = this._buildMightChargeContainer();
		this.mMightPointContainer.setPosition(this.mMightPointsStartX, this.mMightPointsStartY);
		this.mWillPointContainer = this._buildWillChargeContainer();
		this.mWillPointContainer.setPosition(this.mWillPointsStartX, this.mWillPointsStartY);
		this.mBackground.add(this.mHealthBar);
		this.mBackground.add(this.mLevelLabel);
		this.mBackground.add(this.mHealthBarText);
		this.mBackground.add(this.mMightPointContainer);
		this.mBackground.add(this.mWillPointContainer);
		this.mPartyLeader = this.GUI.Image("PartyLeaderCrown");
		this.mPartyLeader.setLayoutExclude(true);
		this.mPartyLeader.setTooltip("Party Leader");
		this.mPartyLeader.setSize(22, 19);
		this.mPartyLeader.setPreferredSize(22, 19);
		this.mPartyLeader.setPosition(29.5, -3);
		this.mPartyLeader.setVisible(false);
		this.mBackground.add(this.mPartyLeader);
		this.mStatus = this.GUI.Image();
		this.mStatus.setImageName("Player_Status-Normal.png");
		this.mStatus.setSize(64, 64);
		this.mStatus.setPosition(7, -4);
		this.mBackground.add(this.mStatus);
		return container;
	}

	function _buildMightChargeContainer()
	{
		local PIP_HOLDER_SIZE = 12;
		local PIP_SIZE = 6;
		local NUM_HOLDERS = 10;
		local container = this.GUI.Container(this.GUI.GridLayout(1, NUM_HOLDERS));
		container.setSize((PIP_HOLDER_SIZE + 2) * NUM_HOLDERS, 18);
		container.setPreferredSize((PIP_HOLDER_SIZE + 2) * NUM_HOLDERS, 18);
		container.getLayoutManager().setColumns(PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE);
		container.getLayoutManager().setRows(14);
		container.getLayoutManager().setGaps(1, 0);

		for( local i = 0; i < 10; i++ )
		{
			local holder = this.GUI.Container(null);
			holder.setAppearance("PointHolder");
			holder.setTooltip("Might");
			local mightCharge = this.GUI.Container(null);
			mightCharge.setAppearance("MightPoint");
			mightCharge.setSize(PIP_SIZE, PIP_SIZE);
			mightCharge.setPreferredSize(PIP_SIZE, PIP_SIZE);
			mightCharge.setPosition(3, 4);
			mightCharge.setTooltip("Might");
			holder.add(mightCharge);
			holder.setSize(PIP_HOLDER_SIZE, 14);
			holder.setPreferredSize(PIP_HOLDER_SIZE, 14);
			container.add(holder);
			this.mMightChargePoints[i] = mightCharge;
		}

		return container;
	}

	function _buildWillChargeContainer()
	{
		local PIP_HOLDER_SIZE = 12;
		local PIP_SIZE = 6;
		local NUM_HOLDERS = 10;
		local container = this.GUI.Container(this.GUI.GridLayout(1, NUM_HOLDERS));
		container.setSize((PIP_HOLDER_SIZE + 2) * NUM_HOLDERS, 18);
		container.setPreferredSize((PIP_HOLDER_SIZE + 2) * NUM_HOLDERS, 18);
		container.getLayoutManager().setColumns(PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE, PIP_HOLDER_SIZE);
		container.getLayoutManager().setRows(14);
		container.getLayoutManager().setGaps(1, 0);

		for( local i = 0; i < 10; i++ )
		{
			local holder = this.GUI.Component(null);
			holder.setAppearance("PointHolder");
			holder.setTooltip("Will");
			local willCharge = this.GUI.Component(null);
			willCharge.setAppearance("WillPoint");
			willCharge.setSize(PIP_SIZE, PIP_SIZE);
			willCharge.setPreferredSize(PIP_SIZE, PIP_SIZE);
			willCharge.setPosition(3, 4);
			willCharge.setTooltip("Will");
			holder.add(willCharge);
			holder.setSize(PIP_HOLDER_SIZE, 14);
			holder.setPreferredSize(PIP_HOLDER_SIZE, 14);
			container.add(holder);
			this.mWillChargePoints[i] = willCharge;
		}

		return container;
	}

	function _centerHealthBarText()
	{
		local font = this.mHealthBarText.getFont();
		local fontMetrics = font.getTextMetrics(this.mHealthBarText.getText());
		this.mHealthBarText.setPosition(this.mHealthTextMiddleXPos - fontMetrics.width / 2, this.mHealthTextStartYPos - fontMetrics.height / 2);
	}

	function _centerLevelText()
	{
		local font = this.mLevelLabel.getFont();
		local fontMetrics = font.getTextMetrics(this.mLevelLabel.getNoTagsText());
		this.mLevelLabel.setPosition(this.mLevelMiddleXPos - fontMetrics.width / 2, this.mLevelMiddleYPos - fontMetrics.height / 2);
	}

	function _createIndicator( color, x, y )
	{
		local result = this.GUI.ColorSplotch(color, false);
		result.setAppearance("ColorSplotch/Hex");
		result.setSize(5, 5);
		result.setPosition(x, y);
		return result;
	}

	function updateStatus()
	{
		if (::_avatar)
		{
			local inCombat = ::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT);
			local isDead = ::_avatar.hasStatusEffect(this.StatusEffects.DEAD);
			local pvpTeam = ::_avatar.getStat(this.Stat.PVP_TEAM);
			local isPVPGame = pvpTeam && pvpTeam != this.PVPTeams.NONE;
			local isSanctuary = false;
			local name = ::_avatar.getName();
			this.mName.setText(name);

			if (isDead)
			{
				this.mStatus.setImageName("Player_Status-Dead.png");
			}
			else if (inCombat)
			{
				this.mStatus.setImageName("Player_Status-Combat.png");
			}
			else if (isPVPGame)
			{
				this.mStatus.setImageName("Player_Status-Pvp.png");
			}
			else if (isSanctuary)
			{
				this.mStatus.setImageName("Player_Status-Sanctuary.png");
			}
			else
			{
				this.mStatus.setImageName("Player_Status-Normal.png");
			}
		}
	}

}

