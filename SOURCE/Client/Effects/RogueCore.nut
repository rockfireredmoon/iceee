this.require("EffectDef");
class this.EffectDef.DeathsTouchWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "DeathsTouchWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Hands",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.DeathsTouch extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathsTouch";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("DeathsTouchTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.DeathsTouchTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathsTouchTarget";
	mSound = "Sound-Ability-Deathly_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathsTouch_Hands",
			emitterPoint = "casting"
		});
		this.onSound();
		local skull = this.createGroup("Skull");
		skull.add("Mesh", {
			mesh = "Item-Skull.mesh",
			point = "node",
			fadeInTime = 0.5,
			fadeOutTime = 1.0
		});
		skull.add("Spin", {
			axis = "y",
			speed = 0.89999998,
			extraStopTime = 1.0
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.WalkInShadow extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "WalkInShadow";
	mLoopSound = "Sound-Ability-Morass_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-WalkInShadow_Gather",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-WalkInShadow_Particles",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(2.9000001, "onStopLoop");
		this.fireIn(3.0, "onComplete");
	}

	function onStopLoop( ... )
	{
		local ah = this.getSource().getAnimationHandler();

		if (ah)
		{
			ah.fullStop();
		}

		this.fireIn(0.2, "onLoopSoundStop");
	}

	function onComplete( ... )
	{
		this.get("Warmup").finish();
		this._cueTargets("WalkInShadowTarget");
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.WalkInShadowTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WalkInShadowTarget";
	mSound = "Sound-Ability-Walkinshadow_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "Magic_Prayer"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-WalkInShadow_Smoke",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.LadyLuck extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LadyLuck";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Particles",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("LadyLuckTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.LadyLuckTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LadyLuckTarget";
	mSound = "Sound-Ability-Ladyluck_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Twinkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-LadyLuck_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		local dice = this.createGroup("Dice");
		dice.add("Mesh", {
			mesh = "Item-Die.mesh",
			point = "node",
			fadeInTime = 0.5,
			fadeOutTime = 1.0
		});
		dice.add("Spin", {
			axis = "y",
			speed = 0.40000001,
			extraStopTime = 1.0
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.ShadowStrike extends this.EffectDef.TemplateMelee
{
	static mEffectName = "ShadowStrike";
	mSound = "Sound-Ability-Walkinshadow_Effect.ogg";
	mWeapon = "";
	mDuration = 8.0;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.add("Dummy");
		this.getTarget().cork();
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "7D26CD",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "One_Handed_Jumping_Thrust"
			});
			this.onSound();
			this.fireIn(0.69999999, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "One_Handed_Jumping_Thrust"
			});
			this.onSound();
			this.fireIn(0.69999999, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Jumping_Thrust"
		});
		this.onSound();
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Jumping_Thrust"
		});
		this.onSound();
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("ShadowStrikeHit", this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.get("Weapon").finish();

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onSecondaryHit( ... )
	{
		if (this.getTarget())
		{
			this.getTarget().cue("ShadowStrikeSecondaryHit", this.getTarget());
		}
	}

}

class this.EffectDef.ShadowStrikeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShadowStrikeHit";
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
			particleSystem = "Par-ShadowStrike_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ShadowStrike_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ShadowStrikeSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShadowStrikeSecondaryHit";
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
			particleSystem = "Par-ShadowStrike_Bleed",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Assail extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Assail";
	mWeapon = "";
	mSound = "Sound-Ability-Assail_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "FFFF00",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this.fireIn(0.30000001, "onSound");
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Hamstring1"
			});
			this.fireIn(0.5, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Hamstring1"
			});
			this.fireIn(0.5, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring2"
		});
		this.fireIn(0.5, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring3"
		});
		this.fireIn(0.5, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("AssailHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.AssailHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AssailHit";
	mSound = "Sound-Ability-Assail_Effect.ogg";
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
			particleSystem = "Par-Assail_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Pierce extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Pierce";
	mWeapon = "";
	mSound = "Sound-Ability-Pierce_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "007FFF",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall( ... )
	{
		this.onSound();
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Jab1"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.60000002, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Jab1"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.60000002, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Jab2"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Jab3"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("PierceHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.PierceHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PierceHit";
	mSound = "Sound-Ability-Pierce_Effect.ogg";
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
			particleSystem = "Par-Pierce_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Pierce_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Disembowel1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Disembowel1";
	mSound = "Sound-Ability-Disembowel_Cast.ogg";
	mWeapon = "";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.mEffectsPackage = this._getEffectsPackage(this.getSource());

		if (this.mEffectsPackage == null)
		{
			return;
		}

		if (this.mEffectsPackage == "Biped")
		{
			this.onBiped();
		}
		else
		{
			this.onCreature();
		}
	}

	function onBiped( ... )
	{
		this.onWeapon();
	}

	function onCreature( ... )
	{
		this.onHorde();
	}

	function onWeapon( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "E3170D",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Eviscerate1",
				events = this._setHit(this.getSource(), "Sneak_Eviscerate1")
			});
			this.onSound();
			this.fireIn(0.60000002, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Dual_Wield_Badgers_Embrace",
				events = this._setHit(this.getSource(), "Dual_Wield_Badgers_Embrace")
			});
			this.onSound();
			this.fireIn(0.89999998, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Eviscerate2"
		});
		this.onSound();
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Eviscerate3"
		});
		this.onSound();
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onHorde( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onFinalHit");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onFinalHit");
		}
	}

	function onHit( ... )
	{
		this.getTarget().cue("DisembowelHit", this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Disembowel2 extends this.EffectDef.Disembowel1
{
	static mEffectName = "Disembowel2";
}

class this.EffectDef.Disembowel3 extends this.EffectDef.Disembowel1
{
	static mEffectName = "Disembowel3";
}

class this.EffectDef.Disembowel4 extends this.EffectDef.Disembowel1
{
	static mEffectName = "Disembowel4";
}

class this.EffectDef.Disembowel5 extends this.EffectDef.Disembowel1
{
	static mEffectName = "Disembowel5";
}

class this.EffectDef.DisembowelHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DisembowelHit";
	mSound = "Sound-Ability-Disembowel_Effect.ogg";
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
			particleSystem = "Par-Disembowel_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Disembowel_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Fade5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Fade5";
	mSound = "Sound-Ability-Walkinshadow_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fade_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fade_Particles",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("FadeTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fade_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.FadeTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FadeTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Fade_Smoke",
			emitterPoint = "node"
		});
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.Backstab1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Backstab1";
	mWeapon = "";
	mSound = "Sound-Ability-Backstab_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "FFE303",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this.onSound();
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Jab1"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.60000002, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Jab1"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.60000002, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Jab1"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Jab1"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.60000002, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("BackstabHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Backstab2 extends this.EffectDef.Backstab1
{
	static mEffectName = "Backstab2";
}

class this.EffectDef.Backstab3 extends this.EffectDef.Backstab1
{
	static mEffectName = "Backstab3";
}

class this.EffectDef.Backstab4 extends this.EffectDef.Backstab1
{
	static mEffectName = "Backstab4";
}

class this.EffectDef.Backstab5 extends this.EffectDef.Backstab1
{
	static mEffectName = "Backstab5";
}

class this.EffectDef.BackstabHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BackstabHit";
	mSound = "Sound-Ability-Backstab_Effect.ogg";
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
			particleSystem = "Par-Backstab_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Backstab_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Hemorrhage1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Hemorrhage1";
	mWeapon = "";
	mDuration = 5.0;
	mSound = "Sound-Ability-Hemorrhage_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.add("Dummy");
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "E3170D",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this.onSound();
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "One_Handed_Diagonal_Strike"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.69999999, "onFinalHit");
			this.fireIn(0.89999998, "onWeaponFinish");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Midcross1"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.5, "onFinalHit");
			this.fireIn(0.69999999, "onWeaponFinish");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Midcross2"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.5, "onFinalHit");
		this.fireIn(0.69999999, "onWeaponFinish");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Midcross3"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.5, "onFinalHit");
		this.fireIn(0.69999999, "onWeaponFinish");
	}

	function onWeaponFinish( ... )
	{
		this.get("Weapon").finish();
	}

	function onHit( ... )
	{
		this.getTarget().cue("HemorrhageHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this._uncorkTargets();
		this.onHit();
		this.getTarget().uncork();

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(this.mDuration + 1.0, "onDone");
	}

	function onSecondaryHit( ... )
	{
		this.getTarget().cue("HemorrhageSecondaryHit", this.getTarget());
	}

}

class this.EffectDef.Hemorrhage2 extends this.EffectDef.Hemorrhage1
{
	static mEffectName = "Hemorrhage2";
}

class this.EffectDef.Hemorrhage3 extends this.EffectDef.Hemorrhage1
{
	static mEffectName = "Hemorrhage3";
}

class this.EffectDef.Hemorrhage4 extends this.EffectDef.Hemorrhage1
{
	static mEffectName = "Hemorrhage4";
}

class this.EffectDef.Hemorrhage5 extends this.EffectDef.Hemorrhage1
{
	static mEffectName = "Hemorrhage5";
}

class this.EffectDef.HemorrhageHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HemorrhageHit";
	mSound = "Sound-Ability-Hemorrhage_Effect.ogg";
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
			particleSystem = "Par-Hemorrhage_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Hemorrhage_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.HemorrhageSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HemorrhageSecondaryHit";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Hemorrhage_Bleed",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Spinstrike1_1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Spinstrike1_1";
	mTargetName = "SpinstrikeHit1_1";
	mWeapon = "";
	mSound = "Sound-Ability-Spinstrike_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "00B2EE",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this.onSound();
		this._corkTargets();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Hamstring1"
			});
			this.fireIn(0.5, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Sneak_Hamstring1"
			});
			this.fireIn(0.5, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring2"
		});
		this.fireIn(0.5, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring3"
		});
		this.fireIn(0.5, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue(this.mTargetName, this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.SpinstrikeHit1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpinstrikeHit1_1";
	mDuration = 6.0;
	mSound = "Sound-Ability-Spinstrike_Effect.ogg";
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
			particleSystem = "Par-Spinstrike_Hit",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Spinstrike_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Spinstrike_Daze",
			emitterPoint = "spell_target_head"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Spinstrike_Fill",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.Untouchable1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Untouchable1";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mTargetName = "UntouchableHit1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue(this.mTargetName, this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Lines",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.Untouchable2 extends this.EffectDef.Untouchable1
{
	static mEffectName = "Untouchable2";
	mTargetName = "UntouchableHit2";
}

class this.EffectDef.Untouchable3 extends this.EffectDef.Untouchable1
{
	static mEffectName = "Untouchable3";
	mTargetName = "UntouchableHit3";
}

class this.EffectDef.Untouchable4 extends this.EffectDef.Untouchable1
{
	static mEffectName = "Untouchable4";
	mTargetName = "UntouchableHit4";
}

class this.EffectDef.Untouchable5 extends this.EffectDef.Untouchable1
{
	static mEffectName = "Untouchable5";
	mTargetName = "UntouchableHit5";
}

class this.EffectDef.UntouchableHit1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "UntouchableHit1";
	mSound = "Sound-Ability-Untouchable_Effect.ogg";
	mDuration = 0.5;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Fill3",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Fill2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.1, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Fill",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Untouchable_Fill2",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.UntouchableHit2 extends this.EffectDef.UntouchableHit1
{
	static mEffectName = "UntouchableHit2";
	mDuration = 1.0;
}

class this.EffectDef.UntouchableHit3 extends this.EffectDef.UntouchableHit1
{
	static mEffectName = "UntouchableHit3";
	mDuration = 1.5;
}

class this.EffectDef.UntouchableHit4 extends this.EffectDef.UntouchableHit1
{
	static mEffectName = "UntouchableHit4";
	mDuration = 2.0;
}

class this.EffectDef.UntouchableHit5 extends this.EffectDef.UntouchableHit1
{
	static mEffectName = "UntouchableHit5";
	mDuration = 2.5;
}

class this.EffectDef.Rend extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Rend";
	mWeapon = "";
	mDuration = 2.0;
	mSound = "Sound-Ability-Rend_Cast.ogg";
	function onStart( ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		this.add("Dummy");
		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "EE7621",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		this.mWeapon = this._checkMelee();

		if (this.mWeapon == "Small" || this.mWeapon == "Dual" || this.mWeapon == "1h")
		{
			this.onSmall();
		}
		else
		{
			this.onDone();
		}
	}

	function onSmall()
	{
		this.onSound();
		this._corkSecondary();
		local smallType = this._checkSmallType();

		if (smallType == "Claw")
		{
			this.onClaw();
		}
		else if (smallType == "Katar")
		{
			this.onKatar();
		}
		else
		{
			this.onDagger();
		}
	}

	function onDagger( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mWeapon == "Small" || this.mWeapon == "1h")
		{
			a.add("FFAnimation", {
				animation = "One_Handed_Diagonal_Strike"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(0.60000002, "onFinalHit");
		}
		else if (this.mWeapon == "Dual")
		{
			a.add("FFAnimation", {
				animation = "Dual_Wield_Jump_Flip"
			});
			this.fireIn(0.2, "onHit");
			this.fireIn(1.3, "onFinalHit");
		}
	}

	function onClaw( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Jump_Flip"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(1.3, "onFinalHit");
	}

	function onKatar( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Jump_Flip"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(1.3, "onFinalHit");
	}

	function onHit( ... )
	{
		this._cueSecondary("RendSecondaryHit");
		this._cueImpactSound(this.getSecondaryTarget());
	}

	function onFinalHit( ... )
	{
		this.fireIn(0.40000001, "onDone");
		this._uncorkSecondary();
		this.onHit();
	}

}

class this.EffectDef.RendSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RendSecondaryHit";
	mSound = "Sound-Ability-Rend_Effect.ogg";
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
			particleSystem = "Par-Rend_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Rend_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

