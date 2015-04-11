this.require("EffectDef");
class this.EffectDef.BloodRitual5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BloodRitual5";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Casting_Glow",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Casting_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("BloodRitualTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Burst2",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.BloodRitualTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BloodRitualTarget";
	mSound = "Sound-Ability-Bloodbath_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Burst",
			emitterPoint = "chest2",
			particleScale = 0.89999998
		});
		this.onSound();
		local spinner = this.createGroup("Spinner");
		spinner.add("ParticleSystem", {
			particleSystem = "Par-BloodRitual_Spin",
			emitterPoint = "node"
		});
		spinner.add("Spin", {
			axis = "y",
			speed = 0.75
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.CanOpener1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "CanOpener1";
	mWeapon = "";
	mSound = "Sound-Ability-Canopener_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "999999",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Small")
		{
			this.onSmall();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else if (mWeapon == "Dual")
		{
			this.onDual();
		}
		else
		{
			this.onDone();
		}
	}

	function on1h()
	{
		this.fireIn(0.2, "onSound");
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Spinning_Backswing"
		});
		this.fireIn(0.60000002, "onFinalHit");
	}

	function on2h()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Spinning"
		});
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onSmall()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Midcross1"
		});
		this.fireIn(0.5, "onFinalHit");
	}

	function onPole()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Staff_SideToSide",
			speed = 1.0
		});
		this.fireIn(0.30000001, "onFinalHit");
	}

	function onDual()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Middle_Cross",
			speed = 1.2
		});
		this.fireIn(0.25, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("CanOpenerHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.CanOpener2 extends this.EffectDef.CanOpener1
{
	static mEffectName = "CanOpener2";
}

class this.EffectDef.CanOpener3 extends this.EffectDef.CanOpener1
{
	static mEffectName = "CanOpener3";
}

class this.EffectDef.CanOpener4 extends this.EffectDef.CanOpener1
{
	static mEffectName = "CanOpener4";
}

class this.EffectDef.CanOpener5 extends this.EffectDef.CanOpener1
{
	static mEffectName = "CanOpener5";
}

class this.EffectDef.CanOpenerHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CanOpenerHit";
	mDuration = 3.0;
	mSound = "Sound-Ability-Canopener_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-CanOpener_Rip",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-CanOpener_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-CanOpener_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-CanOpener_Sparkle",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.Disarm1_1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Disarm1_1";
	mHitName = "DisarmHit1_1";
	mWeapon = "";
	mSound = "Sound-Ability-Disarm_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ff0000",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Small")
		{
			this.onSmall();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else if (mWeapon == "Dual")
		{
			this.onDual();
		}
		else
		{
			this.onDone();
		}
	}

	function on1h()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Flashy"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function on2h()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.5
		});
		this.fireIn(0.2, "onFinalHit");
	}

	function onSmall()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Eviscerate1"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onPole()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Staff_Underhand",
			speed = 1.5
		});
		this.fireIn(0.2, "onFinalHit");
	}

	function onDual()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Flashy_Strike"
		});
		this.fireIn(0.40000001, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue(this.mHitName, this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Disarm1_2 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm1_2";
	mHitName = "DisarmHit1_2";
}

class this.EffectDef.Disarm1_3 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm1_3";
	mHitName = "DisarmHit1_3";
}

class this.EffectDef.Disarm1_4 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm1_4";
	mHitName = "DisarmHit1_4";
}

class this.EffectDef.Disarm1_5 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm1_5";
	mHitName = "DisarmHit1_5";
}

class this.EffectDef.Disarm2_1 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm2_1";
	mHitName = "DisarmHit2_1";
}

class this.EffectDef.Disarm2_2 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm2_2";
	mHitName = "DisarmHit2_2";
}

class this.EffectDef.Disarm2_3 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm2_3";
	mHitName = "DisarmHit2_3";
}

class this.EffectDef.Disarm2_4 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm2_4";
	mHitName = "DisarmHit2_4";
}

class this.EffectDef.Disarm2_5 extends this.EffectDef.Disarm1_1
{
	static mEffectName = "Disarm2_5";
	mHitName = "DisarmHit2_5";
}

class this.EffectDef.DisarmHit1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DisarmHit1_1";
	mDuration = 3.0;
	mSound = "Sound-Ability-Disarm_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Disarm_Hands3",
			emitterPoint = "casting"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Disarm_Hands2",
			emitterPoint = "casting"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Disarm_Impact",
			emitterPoint = "spell_target"
		});
		this.getSource().hideWeapons();
		this.getSource().hideHandAttachments();
		this.fireIn(0.1, "onHitDone");
		this.onSound();
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Disarm_Hands",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.getSource().showWeapons();
		this.finish();
	}

}

class this.EffectDef.DisarmHit1_2 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit1_2";
	mDuration = 4.0;
}

class this.EffectDef.DisarmHit1_3 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit1_3";
	mDuration = 5.0;
}

class this.EffectDef.DisarmHit1_4 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit1_4";
	mDuration = 2.0;
}

class this.EffectDef.DisarmHit1_5 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit1_5";
	mDuration = 2.5;
}

class this.EffectDef.DisarmHit2_1 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit2_1";
	mDuration = 1.0;
}

class this.EffectDef.DisarmHit2_2 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit2_2";
	mDuration = 2.0;
}

class this.EffectDef.DisarmHit2_3 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit2_3";
	mDuration = 3.0;
}

class this.EffectDef.DisarmHit2_4 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit2_4";
	mDuration = 4.0;
}

class this.EffectDef.DisarmHit2_5 extends this.EffectDef.DisarmHit1_1
{
	static mEffectName = "DisarmHit2_5";
	mDuration = 5.0;
}

class this.EffectDef.FatalCrescent extends this.EffectDef.TemplateMelee
{
	static mEffectName = "FatalCrescent";
	mSound = "Sound-Ability-FatalCrescent_Cast.ogg";
	mWeapon = "";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local mWeapon = this._checkMelee();

		if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else
		{
			this.onDone();
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "8DEEEE",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.fireIn(0.80000001, "onDone");
	}

	function on2h( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Side_Cuts", true, 1.3);
		this.onSound();
		this._corkTargets(true);
	}

	function onPole( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Side_Cuts", true, 1.3);
		this.onSound();
		this._corkTargets(true);
	}

	function onHit( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local targets = this.getTargets();
		this._cueTargets("FatalCrescentHit", true);

		foreach( x in targets )
		{
			this._cueImpactSound(x);
		}
	}

	function onHitFinal( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.onHit();
		this._uncorkTargets(true);
		this.onDone();
	}

}

class this.EffectDef.FatalCrescentHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FatalCrescentHit";
	mSound = "Sound-Ability-ThorsMightyBlow_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FatalCrescent_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FatalCrescent_Impact",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Spellbreaker1_1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Spellbreaker1_1";
	mSound = "Sound-Ability-Spellbreaker_Cast.ogg";
	mHitName = "SpellbreakerHit1_1";
	mWeapon = "";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "1B3F8B",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Small")
		{
			this.onSmall();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else if (mWeapon == "Dual")
		{
			this.onDual();
		}
		else
		{
			this.onDone();
		}
	}

	function on1h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Flashy"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.5
		});
		this.fireIn(0.2, "onFinalHit");
	}

	function onSmall()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Eviscerate1"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onPole()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Staff_Underhand",
			speed = 1.5
		});
		this.fireIn(0.2, "onFinalHit");
	}

	function onDual()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Flashy_Strike"
		});
		this.fireIn(0.40000001, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue(this.mHitName, this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onSound();
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Spellbreaker1_2 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker1_2";
	mHitName = "SpellbreakerHit1_2";
}

class this.EffectDef.Spellbreaker1_3 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker1_3";
	mHitName = "SpellbreakerHit1_3";
}

class this.EffectDef.Spellbreaker1_4 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker1_4";
	mHitName = "SpellbreakerHit1_4";
}

class this.EffectDef.Spellbreaker1_5 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker1_5";
	mHitName = "SpellbreakerHit1_5";
}

class this.EffectDef.Spellbreaker2_1 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker2_1";
	mHitName = "SpellbreakerHit2_1";
}

class this.EffectDef.Spellbreaker2_2 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker2_2";
	mHitName = "SpellbreakerHit2_2";
}

class this.EffectDef.Spellbreaker2_3 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker2_3";
	mHitName = "SpellbreakerHit2_3";
}

class this.EffectDef.Spellbreaker2_4 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker2_4";
	mHitName = "SpellbreakerHit2_4";
}

class this.EffectDef.Spellbreaker2_5 extends this.EffectDef.Spellbreaker1_1
{
	static mEffectName = "Spellbreaker2_5";
	mHitName = "SpellbreakerHit2_5";
}

class this.EffectDef.SpellbreakerHit1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpellbreakerHit1_1";
	mDuration = 0.5;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Spellbreaker_Hands3",
			emitterPoint = "casting"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Spellbreaker_Hands2",
			emitterPoint = "casting"
		});
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Spellbreaker_Hands",
			emitterPoint = "casting"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Spellbreaker_Fill",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.SpellbreakerHit1_2 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit1_2";
	mDuration = 1.0;
}

class this.EffectDef.SpellbreakerHit1_3 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit1_3";
	mDuration = 1.5;
}

class this.EffectDef.SpellbreakerHit1_4 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit1_4";
	mDuration = 2.0;
}

class this.EffectDef.SpellbreakerHit1_5 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit1_5";
	mDuration = 2.5;
}

class this.EffectDef.SpellbreakerHit2_1 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit2_1";
	mDuration = 1.0;
}

class this.EffectDef.SpellbreakerHit2_2 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit2_2";
	mDuration = 2.0;
}

class this.EffectDef.SpellbreakerHit2_3 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit2_3";
	mDuration = 3.0;
}

class this.EffectDef.SpellbreakerHit2_4 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit2_4";
	mDuration = 4.0;
}

class this.EffectDef.SpellbreakerHit2_5 extends this.EffectDef.SpellbreakerHit1_1
{
	static mEffectName = "SpellbreakerHit2_5";
	mDuration = 5.0;
}

class this.EffectDef.Taunt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Taunt";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Point",
			speed = 1.5
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Gather",
			emitterPoint = "left_hand"
		});
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("TauntTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Burst",
			emitterPoint = "left_hand"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Gather",
			emitterPoint = "left_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.TauntTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TauntTarget";
	mSound = "Sound-Ability-Taunt_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Fill1",
			emitterPoint = "spell_target_head"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Fill2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Taunt_Burst2",
			emitterPoint = "spell_target_head"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.ThorsMightyBlow1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "ThorsMightyBlow1";
	mWeapon = "";
	mSound = "Sound-Ability-ThorsMightyBlow_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "90eeff",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local mWeapon = this._checkMelee();

		if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else
		{
			this.onDone();
		}
	}

	function on2h( ... )
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onPole( ... )
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("ThorsMightyBlowHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ThorsMightyBlow2 extends this.EffectDef.ThorsMightyBlow1
{
	static mEffectName = "ThorsMightyBlow2";
}

class this.EffectDef.ThorsMightyBlow3 extends this.EffectDef.ThorsMightyBlow1
{
	static mEffectName = "ThorsMightyBlow3";
}

class this.EffectDef.ThorsMightyBlow4 extends this.EffectDef.ThorsMightyBlow1
{
	static mEffectName = "ThorsMightyBlow4";
}

class this.EffectDef.ThorsMightyBlow5 extends this.EffectDef.ThorsMightyBlow1
{
	static mEffectName = "ThorsMightyBlow5";
}

class this.EffectDef.ThorsMightyBlowHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThorsMightyBlowHit";
	mSound = "Sound-Ability-ThorsMightyBlow_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ThorsMightBlow-Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ThorsMightBlow-Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.WarriorsSpirit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WarriorsSpirit";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-WarriorsSpirit_Gather1",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("WarriorsSpiritTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-WarriorsSpirit_Cast",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.WarriorsSpiritTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WarriorsSpiritTarget";
	mSound = "Sound-Ability-WarriorsSpirit_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-WarriorsSpirit_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-WarriorsSpirit_Fill1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-WarriorsSpirit_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Concussion extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Concussion";
	mSound = "Sound-Ability-EnfeeblingBlow_Cast.ogg";
	mHitName = "ConcussionHit";
	mWeapon = "";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "528B8B",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Small")
		{
			this.onSmall();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
		}
		else if (mWeapon == "Dual")
		{
			this.onDual();
		}
		else
		{
			this.onDone();
		}
	}

	function on1h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Jumping_Thrust"
		});
		this.fireIn(0.69999999, "onHitFinal");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul",
			speed = 1.2
		});
		this.fireIn(0.69999999, "onHitFinal");
	}

	function onSmall()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring1"
		});
		this.fireIn(0.5, "onHitFinal");
	}

	function onPole()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Staff_SideToSide",
			speed = 1.2
		});
		this.fireIn(0.2, "onHitFinal");
	}

	function onDual()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Flashy_Strike"
		});
		this.fireIn(0.40000001, "onHitFinal");
	}

	function onHit( ... )
	{
		this.onSound();
		this.getTarget().cue(this.mHitName, this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ConcussionHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ConcussionHit";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 6.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ShieldBash_Impact2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ShieldBash_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill2",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

