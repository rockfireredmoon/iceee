this.require("UI/Screens");
class this.Screens.DPSMeter extends this.GUI.MainScreenElement
{
	mTimer = null;
	mDamageOut = null;
	mDamageIn = null;
	mSwingsOut = 0;
	mSwingsOutHit = 0;
	mSwingsIn = 0;
	mSwingsInHit = 0;
	mHitRateOutLabel = null;
	mHitRateInLabel = null;
	mUpdateEvent = null;
	mTotalTimeInCombat = 0;
	mLastTimeUpdated = 0;
	mCurrentStatusLabel = null;
	mInCombat = false;
	mResetButton = null;
	mPrintButton = null;
	constructor()
	{
		this.GUI.MainScreenElement.constructor(this.GUI.GridLayout(19, 2));
		this.setAppearance("ChatWindow");
		this.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.30000001));
		this.mTimer = ::Timer();
		this.setSize(300, 300);
		this.setPosition(200, 200);
		this.mDamageOut = {};
		this.mDamageIn = {};
		this.mCurrentStatusLabel = this.GUI.Label("Not in combat");
		this.add(this.GUI.Label("Current Status"));
		this.add(this.mCurrentStatusLabel);
		this.mCurrentStatusLabel.setFontColor(this.Colors.red);
		this.add(this.GUI.Label("Total outgoing damage:"));
		this.mDamageOut.Total <- this.GUI.DamageLabelTypeCombo(-1);
		this.add(this.mDamageOut.Total);
		this.add(this.GUI.Label(" Hit %:"));
		this.mHitRateOutLabel = this.GUI.Label("");
		this.add(this.mHitRateOutLabel);
		this.add(this.GUI.Label("      Physical:"));
		this.mDamageOut.Melee <- this.GUI.DamageLabelTypeCombo(this.DamageType.MELEE);
		this.add(this.mDamageOut.Melee);
		local fireLabel = this.GUI.Label("      Fire:");
		fireLabel.setFontColor(this.Colors.red);
		this.add(fireLabel);
		this.mDamageOut.Fire <- this.GUI.DamageLabelTypeCombo(this.DamageType.FIRE, this.Colors.red);
		this.add(this.mDamageOut.Fire);
		local frostLabel = this.GUI.Label("      Frost:");
		frostLabel.setFontColor(this.Colors.blue);
		this.add(frostLabel);
		this.mDamageOut.Frost <- this.GUI.DamageLabelTypeCombo(this.DamageType.FROST, this.Colors.blue);
		this.add(this.mDamageOut.Frost);
		local mysticLabel = this.GUI.Label("      Mystic:");
		mysticLabel.setFontColor(this.Colors["GUI Gold"]);
		this.add(mysticLabel);
		this.mDamageOut.Mystic <- this.GUI.DamageLabelTypeCombo(this.DamageType.MYSTIC, this.Colors["GUI Gold"]);
		this.add(this.mDamageOut.Mystic);
		local deathLabel = this.GUI.Label("      Death:");
		deathLabel.setFontColor(this.Colors.purple);
		this.add(deathLabel);
		this.mDamageOut.Death <- this.GUI.DamageLabelTypeCombo(this.DamageType.DEATH, this.Colors.purple);
		this.add(this.mDamageOut.Death);
		local unblockableLabel = this.GUI.Label("      Unblockable:");
		unblockableLabel.setFontColor(this.Colors.silver);
		this.add(unblockableLabel);
		this.mDamageOut.Unblockable <- this.GUI.DamageLabelTypeCombo(this.DamageType.UNBLOCKABLE, this.Colors.silver);
		this.add(this.mDamageOut.Unblockable);
		this.add(this.GUI.Label("Total Incoming damage:"));
		this.mDamageIn.Total <- this.GUI.DamageLabelTypeCombo(-1);
		this.add(this.mDamageIn.Total);
		this.add(this.GUI.Label(" Hit %:"));
		this.mHitRateInLabel = this.GUI.Label("");
		this.add(this.mHitRateInLabel);
		this.add(this.GUI.Label("      Physical:"));
		this.mDamageIn.Melee <- this.GUI.DamageLabelTypeCombo(this.DamageType.MELEE);
		this.add(this.mDamageIn.Melee);
		fireLabel = this.GUI.Label("      Fire:");
		fireLabel.setFontColor(this.Colors.red);
		this.add(fireLabel);
		this.mDamageIn.Fire <- this.GUI.DamageLabelTypeCombo(this.DamageType.FIRE, this.Colors.red);
		this.add(this.mDamageIn.Fire);
		frostLabel = this.GUI.Label("      Frost:");
		frostLabel.setFontColor(this.Colors.blue);
		this.add(frostLabel);
		this.mDamageIn.Frost <- this.GUI.DamageLabelTypeCombo(this.DamageType.FROST, this.Colors.blue);
		this.add(this.mDamageIn.Frost);
		mysticLabel = this.GUI.Label("       Mystic:");
		mysticLabel.setFontColor(this.Colors["GUI Gold"]);
		this.add(mysticLabel);
		this.mDamageIn.Mystic <- this.GUI.DamageLabelTypeCombo(this.DamageType.MYSTIC, this.Colors["GUI Gold"]);
		this.add(this.mDamageIn.Mystic);
		deathLabel = this.GUI.Label("       Death:");
		deathLabel.setFontColor(this.Colors.purple);
		this.add(deathLabel);
		this.mDamageIn.Death <- this.GUI.DamageLabelTypeCombo(this.DamageType.DEATH, this.Colors.purple);
		this.add(this.mDamageIn.Death);
		unblockableLabel = this.GUI.Label("      Unblockable:");
		unblockableLabel.setFontColor(this.Colors.silver);
		this.add(unblockableLabel);
		this.mDamageIn.Unblockable <- this.GUI.DamageLabelTypeCombo(this.DamageType.UNBLOCKABLE, this.Colors.silver);
		this.add(this.mDamageIn.Unblockable);
		this.mResetButton = this.GUI.Button("Reset");
		this.mResetButton.setReleaseMessage("onReset");
		this.mResetButton.addActionListener(this);
		this.add(this.mResetButton);
		this.mPrintButton = this.GUI.Button("Print to log");
		this.mPrintButton.setReleaseMessage("onPrint");
		this.mPrintButton.addActionListener(this);
		this.add(this.mPrintButton);
		this._enterFrameRelay.addListener(this);
	}

	function addDamageIn( amount, damageType )
	{
		this.mDamageIn[this.DamageTypeNameMapping[damageType]].addDamage(amount);
		this.mDamageIn.Total.addDamage(amount);
	}

	function addDamageOut( amount, damageType )
	{
		this.mDamageOut[this.DamageTypeNameMapping[damageType]].addDamage(amount);
		this.mDamageOut.Total.addDamage(amount);
	}

	function addSwingIn( hit )
	{
		if (hit)
		{
			this.mSwingsInHit++;
		}

		this.mSwingsIn++;
	}

	function addSwingOut( hit )
	{
		if (hit)
		{
			this.mSwingsOutHit++;
		}

		this.mSwingsOut++;
	}

	function dps_update()
	{
		local combatTime = this.mTotalTimeInCombat;

		if (combatTime > 0)
		{
			combatTime = combatTime.tofloat() / 1000.0;

			if (this.mSwingsOut > 0)
			{
				this.mHitRateOutLabel.setText(this.Util.limitSignificantDigits(this.mSwingsOutHit.tofloat() / this.mSwingsOut.tofloat() * 100, 2).tostring() + "%");
			}

			local totalDamageDone = this.mDamageOut.Total.getDamage();

			foreach( k, v in this.mDamageOut )
			{
				local dpsOut = this.Util.limitSignificantDigits(v.getDamage().tofloat() / combatTime, 2);

				if (k == "Total")
				{
					v.setText(dpsOut.tostring());
				}
				else if (totalDamageDone > 0)
				{
					local totalPercantageDone = this.Util.limitSignificantDigits(v.getDamage().tofloat() / totalDamageDone.tofloat() * 100.0, 2);
					v.setText(dpsOut.tostring() + " (" + totalPercantageDone + "%)");
				}
				else
				{
					v.setText(dpsOut.tostring());
				}
			}

			if (this.mSwingsIn > 0)
			{
				this.mHitRateInLabel.setText(this.Util.limitSignificantDigits(this.mSwingsInHit.tofloat() / this.mSwingsIn.tofloat() * 100, 2).tostring() + "%");
			}

			local totalDamageTaken = this.mDamageIn.Total.getDamage();

			foreach( k, v in this.mDamageIn )
			{
				local dpsIn = this.Util.limitSignificantDigits(v.getDamage().tofloat() / combatTime, 2);

				if (k == "Total")
				{
					v.setText(dpsIn.tostring());
				}
				else if (totalDamageTaken > 0)
				{
					local totalPercantageTaken = this.Util.limitSignificantDigits(v.getDamage().tofloat() / totalDamageTaken.tofloat() * 100.0, 2);
					v.setText(dpsIn.tostring() + " (" + totalPercantageTaken + "%)");
				}
				else
				{
					v.setText(dpsIn.tostring());
				}
			}
		}
	}

	function onEnterFrame()
	{
		if (!::_avatar)
		{
			return;
		}

		local currentTime = this.mTimer.getMilliseconds();
		local inCombat = ::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT_STAND);

		if (inCombat)
		{
			this.mTotalTimeInCombat += currentTime - this.mLastTimeUpdated;

			if (inCombat != this.mInCombat)
			{
				this.mInCombat = inCombat;

				if (this.mInCombat == true)
				{
					this.mCurrentStatusLabel.setText("In combat");
					this.mCurrentStatusLabel.setFontColor(this.Colors.green);
				}
			}
		}
		else if (this.mInCombat != inCombat)
		{
			this.mCurrentStatusLabel.setText("Not in combat");
			this.mCurrentStatusLabel.setFontColor(this.Colors.red);
			this.mInCombat = inCombat;
		}

		this.mLastTimeUpdated = currentTime;
	}

	function onPrint( button )
	{
		this.log.debug("-----DPS Stats----");
		local totalDamageDone = this.mDamageOut.Total.getDamage();
		local totalDamageTaken = this.mDamageIn.Total.getDamage();
		this.log.debug("Total damage done : " + totalDamageDone);
		this.log.debug("Total damage taken : " + totalDamageTaken);
		this.log.debug("Total combat time : " + this.mTotalTimeInCombat.tofloat() / 1000.0 + " seconds");

		if (this.mSwingsOut > 0)
		{
			this.log.debug("Hit percentage Out: " + this.Util.limitSignificantDigits(this.mSwingsOutHit.tofloat() / this.mSwingsOut.tofloat() * 100, 2).tostring() + "% (" + this.mSwingsOutHit + "\\" + this.mSwingsOut + ")");
		}
		else
		{
			this.log.debug("No swings were delt");
		}

		if (this.mSwingsIn > 0)
		{
			this.log.debug("Hit percentage In: " + this.Util.limitSignificantDigits(this.mSwingsInHit.tofloat() / this.mSwingsIn.tofloat() * 100, 2).tostring() + "% (" + this.mSwingsInHit + "\\" + this.mSwingsIn + ")");
		}
		else
		{
			this.log.debug("No swings were taken");
		}

		local combatTime = this.mTotalTimeInCombat;

		if (combatTime > 0)
		{
			combatTime = this.mTotalTimeInCombat;
			combatTime = combatTime.tofloat() / 1000.0;
			this.log.debug("Total DPS Out: " + this.Util.limitSignificantDigits(totalDamageDone.tofloat() / combatTime, 2));

			foreach( k, v in this.mDamageOut )
			{
				local dpsOut = this.Util.limitSignificantDigits(v.getDamage().tofloat() / combatTime, 2);

				if (k == "Total")
				{
				}
				else if (totalDamageDone > 0)
				{
					local totalPercantageDone = this.Util.limitSignificantDigits(v.getDamage().tofloat() / totalDamageDone.tofloat() * 100.0, 2);
					this.log.debug("      " + this.DamageTypeNameMapping[v.getType()] + " DPS: " + dpsOut.tostring() + " (" + totalPercantageDone + "%)" + " (" + v.getDamage() + ")");
				}
				else
				{
					this.log.debug("      " + this.DamageTypeNameMapping[v.getType()] + " DPS: " + dpsOut.tostring() + " (" + v.getDamage() + ")");
				}
			}

			this.log.debug("Total DPS In: " + this.Util.limitSignificantDigits(totalDamageTaken.tofloat() / combatTime, 2));

			foreach( k, v in this.mDamageIn )
			{
				local dpsIn = this.Util.limitSignificantDigits(v.getDamage().tofloat() / combatTime, 2);

				if (k == "Total")
				{
				}
				else if (totalDamageTaken > 0)
				{
					local totalPercantageTaken = this.Util.limitSignificantDigits(v.getDamage().tofloat() / totalDamageTaken.tofloat() * 100.0, 2);
					this.log.debug("      " + this.DamageTypeNameMapping[v.getType()] + " DPS: " + dpsIn.tostring() + " (" + totalPercantageTaken + "%)" + " (" + v.getDamage() + ")");
				}
				else
				{
					this.log.debug("      " + this.DamageTypeNameMapping[v.getType()] + " DPS: " + dpsIn.tostring() + " (" + v.getDamage() + ")");
				}
			}
		}

		this.log.debug("------------------");
	}

	function onReset( button )
	{
		this.mTotalTimeInCombat = 0;

		foreach( k, v in this.mDamageOut )
		{
			v.setText("0");
			v.reset();
		}

		this.mSwingsOut = 0;
		this.mSwingsOutHit = 0;
		this.mHitRateOutLabel.setText("");

		foreach( k, v in this.mDamageIn )
		{
			v.setText("0");
			v.reset();
		}

		this.mSwingsIn = 0;
		this.mSwingsInHit = 0;
		this.mHitRateInLabel.setText("");
	}

	function setVisible( value )
	{
		if (value == true)
		{
			this.mUpdateEvent = ::_eventScheduler.repeatIn(0.0, 1.0, this, "dps_update");
		}
		else if (this.mUpdateEvent)
		{
			::_eventScheduler.cancel(this.mUpdateEvent);
		}

		this.GUI.MainScreenElement.setVisible(value);
	}

}

class this.GUI.DamageLabelTypeCombo extends this.GUI.Label
{
	mType = null;
	mDamage = 0;
	constructor( type, ... )
	{
		this.GUI.Label.constructor();
		this.mType = type;

		if (vargc > 0)
		{
			this.setFontColor(vargv[0]);
		}
	}

	function getType()
	{
		return this.mType;
	}

	function addDamage( damage )
	{
		this.mDamage += damage;
	}

	function getDamage()
	{
		return this.mDamage;
	}

	function reset()
	{
		this.mDamage = 0;
	}

}

