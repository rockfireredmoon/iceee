this.require("EffectDef");
class this.EffectDef.Assault extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Assault";
	mWeapon = "";
	mSound = "Sound-Ability-Assault_Cast.ogg";
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

		this.getTarget().cork();
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
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "One_Handed_Spinning_Backswing"
		});
		this.fireIn(0.60000002, "onHitFinal");
	}

	function on2h()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onHitFinal");
	}

	function onHorde( ... )
	{
		local cast = this.createGroup("Anim1", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onHitFinal");
		}
		else if (this.mEffectsPackage == "Horde-Vespin")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onHitFinal");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onHitFinal");
		}
	}

	function onHit( ... )
	{
		this.getTarget().cue("AssaultHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.AssaultHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AssaultHit";
	mSound = "Sound-Ability-Assault_Effect.ogg";
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
			particleSystem = "Par-Assault_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Assault_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Blender1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Blender1";
	mSound = "Sound-Ability-Blender_Cast.ogg";
	mWeapon = "";
	mHits = 1;
	function onStart( ... )
	{
		if (!this._sourceCheck())
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

		if (!this._targetCheck())
		{
			return;
		}

		this._corkTargets();

		if (this.mHits > 1)
		{
			this.fireIn(0.80000001, "onHit");
		}

		if (this.mHits > 2)
		{
			this.fireIn(0.40000001, "onHit");
		}

		if (this.mHits > 3)
		{
			this.fireIn(0.60000002, "onHit");
		}

		if (this.mHits > 4)
		{
			this.fireIn(1.0, "onHit");
		}
	}

	function on1h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.fireIn(0.2, "onStartSound");
		this.fireIn(1.2, "onFinalHit");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.fireIn(0.2, "onStartSound");
		this.fireIn(1.2, "onFinalHit");
	}

	function onSmall()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Spinning_Daggers"
		});
		this.fireIn(0.2, "onStartSound");
		this.fireIn(1.2, "onFinalHit");
	}

	function onPole()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.fireIn(0.2, "onStartSound");
		this.fireIn(1.2, "onFinalHit");
	}

	function onDual()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.fireIn(0.2, "onStartSound");
		this.fireIn(1.2, "onFinalHit");
	}

	function onHit( ... )
	{
		local targets = this.getTargets();

		if (targets == null)
		{
			return;
		}

		if (targets.len() == 0)
		{
			return;
		}

		this._cueTargets("BlenderHit", true);

		foreach( x in targets )
		{
			this._cueImpactSound(x);
		}
	}

	function onStartSound( ... )
	{
		this.onSound();
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this._uncorkTargets(true);
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Blender2 extends this.EffectDef.Blender1
{
	static mEffectName = "Blender2";
	mHits = 2;
}

class this.EffectDef.Blender3 extends this.EffectDef.Blender1
{
	static mEffectName = "Blender3";
	mHits = 3;
}

class this.EffectDef.Blender4 extends this.EffectDef.Blender1
{
	static mEffectName = "Blender4";
	mHits = 4;
}

class this.EffectDef.Blender5 extends this.EffectDef.Blender1
{
	static mEffectName = "Blender5";
	mHits = 5;
}

class this.EffectDef.BlenderHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlenderHit";
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
			particleSystem = "Par-Blender_Impact",
			particleScale = 0.69999999,
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Blender_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Cripple extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Cripple";
	mWeapon = "";
	mSound = "Sound-Ability-Assault_Cast.ogg";
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

		this.getTarget().cork();
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
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "One_Handed_Spinning_Backswing"
		});
		this.fireIn(0.60000002, "onHitFinal");
	}

	function on2h()
	{
		this.onSound();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onHitFinal");
	}

	function onHit( ... )
	{
		this.getTarget().cue("CrippleHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.CrippleHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CrippleHit";
	mSound = "Sound-Ability-Assault_Effect.ogg";
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
			particleSystem = "Par-Assault_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Assault_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Challenge extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Challenge";
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
			particleSystem = "Par-Challenge_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Challenge_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.60000002, "onHitFinal");
		this.fireIn(0.2, "onInitialBurst");
		this.fireIn(0.30000001, "onInitialBurst2");
		this.fireIn(0.69999999, "onBurst");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		blast.add("ParticleSystem", {
			particleSystem = "Par-Challenge_Blast",
			emitterPoint = "node"
		});
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Challenge_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("ChallengeHit", true);
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.ChallengeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ChallengeHit";
	mSound = "Sound-Ability-Challenge_Effect.ogg";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Challenge_Fill1",
			emitterPoint = "spell_target_head"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Challenge_Fill2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.Demoralize1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Demoralize1";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mWeapon = "";
	mHit = "DemoralizeHit1";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "2h")
		{
			this.on2h();
		}
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "Exclaim"
		});
		this.onSound();
		this.fireIn(0.40000001, "onShout");
		this.fireIn(0.69999999, "onHitFinal");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Exclaim"
		});
		this.onSound();
		this.fireIn(0.40000001, "onShout");
		this.fireIn(0.69999999, "onHitFinal");
	}

	function onShout()
	{
		local shout = this.createGroup("Shout", this.getSource());
		shout.add("ParticleSystem", {
			particleSystem = "Par-Demoralize_Burst",
			emitterPoint = "spell_target_head"
		});
	}

	function onHit( ... )
	{
		this.getTarget().cue(this.mHit, this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Demoralize2 extends this.EffectDef.Demoralize1
{
	static mEffectName = "Demoralize2";
	mHit = "DemoralizeHit2";
}

class this.EffectDef.Demoralize3 extends this.EffectDef.Demoralize1
{
	static mEffectName = "Demoralize3";
	mHit = "DemoralizeHit3";
}

class this.EffectDef.DemoralizeHit1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DemoralizeHit";
	mSound = "Sound-Ability-Mystical_Effect.ogg";
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
			particleSystem = "Par-Demoralize_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Demoralize_Fill2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Demoralize_Sparkle1",
			emitterPoint = "spell_origin"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Demoralize_Fill1",
			emitterPoint = "spell_origin"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.DemoralizeHit2 extends this.EffectDef.DemoralizeHit1
{
	static mEffectName = "DemoralizeHit2";
	mDuration = 9.0;
}

class this.EffectDef.DemoralizeHit3 extends this.EffectDef.DemoralizeHit1
{
	static mEffectName = "DemoralizeHit3";
	mDuration = 12.0;
}

class this.EffectDef.EarthShaker1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "EarthShaker1_1";
	mHitName = "EarthShakerHit1_1";
	mSound = "Sound-Ability-Earth_Shaker_Cast.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul",
			speed = 1.2
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onSound");
		this._corkTargets();
		this.fireIn(0.80000001, "onHitFinal");
		this.fireIn(0.5, "onImpact");
		this.fireIn(0.69999999, "onImpact1");
		this.fireIn(0.89999998, "onImpact2");
		this.fireIn(1.1, "onImpact3");
		this.fireIn(0.60000002, "onImpactEnd");
		this.fireIn(0.69999999, "onBurst");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onImpact( ... )
	{
		local shock = this.createGroup("Shockwave");
		shock.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Ring"
		});
	}

	function onImpact1( ... )
	{
		local wave1 = this.createGroup("Wave1");
		wave1.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Wave1"
		});
		wave1.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Rocks1"
		});
	}

	function onImpact2( ... )
	{
		local wave2 = this.createGroup("Wave2");
		wave2.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Wave2"
		});
		wave2.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Rocks2"
		});
		this.get("Wave1").finish();
	}

	function onImpact3( ... )
	{
		local wave3 = this.createGroup("Wave3");
		wave3.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Wave3"
		});
		wave3.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Rocks3"
		});
		this.get("Wave2").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 80,
			orthoHeight = 80,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets(this.mHitName);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onImpactEnd( ... )
	{
		this.get("Shockwave").finish();
	}

	function onUncork( ... )
	{
		this._uncorkTargets();
	}

}

class this.EffectDef.EarthShaker1_2 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker1_2";
	mHitName = "EarthShakerHit1_2";
}

class this.EffectDef.EarthShaker1_3 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker1_3";
	mHitName = "EarthShakerHit1_3";
}

class this.EffectDef.EarthShaker1_4 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker1_4";
	mHitName = "EarthShakerHit1_4";
}

class this.EffectDef.EarthShaker1_5 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker1_5";
	mHitName = "EarthShakerHit1_5";
}

class this.EffectDef.EarthShaker2_1 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker2_1";
	mHitName = "EarthShakerHit2_1";
}

class this.EffectDef.EarthShaker2_2 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker2_2";
	mHitName = "EarthShakerHit2_2";
}

class this.EffectDef.EarthShaker2_3 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker2_3";
	mHitName = "EarthShakerHit2_3";
}

class this.EffectDef.EarthShaker2_4 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker2_4";
	mHitName = "EarthShakerHit2_4";
}

class this.EffectDef.EarthShaker2_5 extends this.EffectDef.EarthShaker1_1
{
	static mEffectName = "EarthShaker2_5";
	mHitName = "EarthShakerHit2_5";
}

class this.EffectDef.EarthShakerHit1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "EarthShakerHit1_1";
	mDuration = 5.0;
	mSound = "Sound-Ability-Earth_Shaker_Effect.ogg";
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
			particleSystem = "Par-EarthShaker_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-EarthShaker_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Fill1",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Fill2",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.EarthShakerHit1_2 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit1_2";
	mDuration = 0.40000001;
}

class this.EffectDef.EarthShakerHit1_3 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit1_3";
	mDuration = 0.60000002;
}

class this.EffectDef.EarthShakerHit1_4 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit1_4";
	mDuration = 0.80000001;
}

class this.EffectDef.EarthShakerHit1_5 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit1_5";
	mDuration = 1.0;
}

class this.EffectDef.EarthShakerHit2_1 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit2_1";
	mDuration = 0.40000001;
}

class this.EffectDef.EarthShakerHit2_2 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit2_2";
	mDuration = 0.80000001;
}

class this.EffectDef.EarthShakerHit2_3 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit2_3";
	mDuration = 1.2;
}

class this.EffectDef.EarthShakerHit2_4 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit2_4";
	mDuration = 1.6;
}

class this.EffectDef.EarthShakerHit2_5 extends this.EffectDef.EarthShakerHit1_1
{
	static mEffectName = "EarthShakerHit2_5";
	mDuration = 2.0;
}

class this.EffectDef.EnfeeblingBlowWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "EnfeeblingBlowWarmup";
	mLoopSound = "Sound-Ability-MysticProtection_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill2",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.EnfeeblingBlow1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "EnfeeblingBlow1";
	mSound = "Sound-Ability-EnfeeblingBlow_Cast.ogg";
	mHitName = "EnfeeblingBlowHit1";
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
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onHitFinal");
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

class this.EffectDef.EnfeeblingBlow2 extends this.EffectDef.EnfeeblingBlow1
{
	static mEffectName = "EnfeeblingBlow2";
	mHitName = "EnfeeblingBlowHit2";
}

class this.EffectDef.EnfeeblingBlow3 extends this.EffectDef.EnfeeblingBlow1
{
	static mEffectName = "EnfeeblingBlow3";
	mHitName = "EnfeeblingBlowHit3";
}

class this.EffectDef.EnfeeblingBlowHit1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "EnfeeblingBlowHit1";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 2.0;
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
			particleSystem = "Par-Enfeebling_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Burst",
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

class this.EffectDef.EnfeeblingBlowHit2 extends this.EffectDef.EnfeeblingBlowHit1
{
	static mEffectName = "EnfeeblingBlowHit2";
	mDuration = 4.0;
}

class this.EffectDef.EnfeeblingBlowHit3 extends this.EffectDef.EnfeeblingBlowHit1
{
	static mEffectName = "EnfeeblingBlowHit3";
	mDuration = 6.0;
}

class this.EffectDef.Hatred extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Hatred";
	mSound = "Sound-Ability-Hatred_Cast.ogg";
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
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("HatredTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.HatredTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HatredTarget";
	mSound = "Sound-Ability-Hatred_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Mist",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.HitMe5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HitMe5";
	mSound = "Sound-Ability-Hitme_Cast.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Exclaim"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("HitMeHit", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.HitMeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HitMeHit";
	mSound = "Sound-Ability-Hitme_Effect.ogg";
	mDuration = 8.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-HitMe_Burst",
			emitterPoint = "casting",
			particleScale = 1.3
		});
		this.onSound();
		this.fireIn(0.80000001, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-HitMe_Sparkle",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-HitMe_Fill2",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.Provoke extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Provoke";
	mSound = "Sound-Ability-Provoke_Cast.ogg";
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
			particleSystem = "Par-Provoke_Gather",
			emitterPoint = "left_hand"
		});
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("ProvokeTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Provoke_Burst",
			emitterPoint = "left_hand"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Provoke_Gather",
			emitterPoint = "left_hand"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.ProvokeTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ProvokeTarget";
	mSound = "Sound-Ability-Provoke_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Provoke_Fill1",
			emitterPoint = "spell_target_head"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Provoke_Burst2",
			emitterPoint = "spell_target_head"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Provoke_Fill2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.RageOfAres5 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "RageOfAres5";
	mWeapon = "";
	mSound = "Sound-Ability-RageOfAres_Cast2.ogg";
	mSoundCount = 0;
	function onStart( ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		this.getSecondaryTarget().cork();
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
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "One_Handed_Diagonal_Strike",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "One_Handed_Diagonal_Strike", false, 1.3);
		this.fireIn(1.25, "on1h2");
	}

	function on1h2()
	{
		local a = this.createGroup("Anim2", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Spinning_Backswing",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "One_Handed_Spinning_Backswing", false, 1.3);
		this.fireIn(1.5, "on1h3");
	}

	function on1h3()
	{
		local a = this.createGroup("Anim3", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Diagonal_Strike",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "One_Handed_Diagonal_Strike", true, 1.3);
		this.fireIn(1.2, "onDone");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Diagonal1",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Diagonal1", false, 1.3);
		this.fireIn(1.2, "on2h2");
	}

	function on2h2()
	{
		local a = this.createGroup("Anim2", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Diagonal2",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Diagonal2", false, 1.3);
		this.fireIn(1.5, "on2h3");
	}

	function on2h3()
	{
		local a = this.createGroup("Anim3", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Spinning",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Spinning", true, 1.3);
		this.fireIn(1.5, "onDone");
	}

	function onHit( ... )
	{
		local soundName = "Sound" + this.mSoundCount;
		this.onSound(soundName);
		this.mSoundCount++;
		this.getSecondaryTarget().cue("RageOfAresHit", this.getSecondaryTarget());
		this._cueImpactSound(this.getSecondaryTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getSecondaryTarget().uncork();
	}

}

class this.EffectDef.RageOfAresHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RageOfAresHit";
	mSound = "Sound-Ability-Rageofares_Effect.ogg";
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
			particleSystem = "Par-RageOfAres_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-RageOfAres_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.RampageWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "RampageWarmup";
	mLoopSound = "Sound-Ability-MysticProtection_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Abolish_Hand",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.Rampage extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Rampage";
	mSound = "Sound-Ability-Rampage_Cast.ogg";
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
			this.on1h();
		}
		else if (mWeapon == "Dual")
		{
			this.on1h();
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
			animation = "One_Handed_Spinning_Backswing",
			speed = 1.6
		});
		this._setFireInHit(this.getSource(), "One_Handed_Spinning_Backswing", false, 1.6);
		this.fireIn(1.0, "on1h2");
	}

	function on1h2()
	{
		local a = this.createGroup("Anim2", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Jumping_Thrust",
			speed = 1.6
		});
		this._setFireInHit(this.getSource(), "One_Handed_Jumping_Thrust", true, 1.6);
		this.fireIn(0.60000002, "onDone");
	}

	function onHit( ... )
	{
		this.getTarget().cue("RampageHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onSound();
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.RampageHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RampageHit";
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
			particleSystem = "Par-Rampage_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Rampage_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ShieldBash1 extends this.EffectDef.TemplateMelee
{
	static mEffectName = "ShieldBash1";
	mSound = "Sound-Ability-Shieldbash_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "FF6600",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Shield_Counter"
		});
		this._corkTargets();
		this.fireIn(0.050000001, "onShieldHit");
		this.fireIn(0.2, "onHit");
		this.fireIn(0.89999998, "onHitFinal");
	}

	function onShieldHit( ... )
	{
		local shield = this.createGroup("Shield", this.getSource());
		shield.add("ParticleSystem", {
			particleSystem = "Par-ShieldBash_Impact",
			emitterPoint = "left_hand_Shield"
		});
		this.getTarget().cue("ShieldBashHit", this.getTarget());
		this.fireIn(0.2, "onShieldStop");
	}

	function onShieldStop( ... )
	{
		this.get("Shield").finish();
		this.onSound();
	}

	function onHit( ... )
	{
		this.getTarget().cue("ShieldBashHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this._uncorkTargets();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ShieldBash2 extends this.EffectDef.ShieldBash1
{
	static mEffectName = "ShieldBash2";
}

class this.EffectDef.ShieldBash3 extends this.EffectDef.ShieldBash1
{
	static mEffectName = "ShieldBash3";
}

class this.EffectDef.ShieldBash4 extends this.EffectDef.ShieldBash1
{
	static mEffectName = "ShieldBash4";
}

class this.EffectDef.ShieldBash5 extends this.EffectDef.ShieldBash1
{
	static mEffectName = "ShieldBash5";
}

class this.EffectDef.ShieldBashHit extends this.EffectDef.TemplateMelee
{
	static mEffectName = "ShieldBashHit";
	mSound = "Sound-Ability-Shieldbash_Effect.ogg";
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
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.TaurianMight extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TaurianMight";
	mSound = "Sound-Ability-TaurianMight_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Throw",
			speed = 1.3
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Gather",
			emitterPoint = "chest2",
			particleScale = 1.5
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Gather",
			emitterPoint = "casting",
			particleScale = 0.80000001
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("TaurianMightTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Burst",
			emitterPoint = "casting",
			particleScale = 0.80000001
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.TaurianMightTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TaurianMightTarget";
	mSound = "Sound-Ability-TaurianMight_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Mist",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Burst",
			emitterPoint = "chest2",
			particleScale = 0.89999998
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-TaurianMight_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Turtle5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Turtle5";
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
			particleSystem = "Par-Turtle_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Turtle_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("TurtleTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Turtle_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.TurtleTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TurtleTarget";
	mSound = "Sound-Ability-Turtle_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff");
		buff.add("ParticleSystem", {
			particleSystem = "Par-TurtleTarget_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-TurtleTarget_Fill2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-TurtleTarget_Lines",
			emitterPoint = "node"
		});
		buff.add("Mesh", {
			mesh = "Item-Turtle_Shield.mesh",
			point = "node",
			fadeInTime = 1.0,
			fadeOutTime = 0.5
		});
		buff.add("Spin", {
			axis = "y",
			speed = 0.75,
			extraStopTime = 1.0
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Whiplash extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Whiplash";
	mHitName = "WhiplashHit";
	mWeapon = "";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "528B8B",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
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
		else if (mWeapon == "Dual")
		{
			this.on1h();
		}
		else
		{
			this.onDone();
		}

		this._corkTargets(true);
	}

	function on1h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Spinning_Backswing"
		});
		this._setFireInHit(this.getSource(), "One_Handed_Spinning_Backswing");
	}

	function on2h()
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.3
		});
		this._setFireInHit(this.getSource(), "Two_Handed_Side_Cuts", true, 1.3);
	}

	function onHit( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this._cueTargets(this.mHitName, true);

		foreach( x in this.getTargets() )
		{
			this._cueImpactSound(x);
		}
	}

	function onHitFinal( ... )
	{
		if (this._targetCheck())
		{
			this.onHit();
			this._uncorkTargets(true);
		}

		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Whiplash2t extends this.EffectDef.Whiplash
{
	static mEffectName = "Whiplash2t";
	mHitName = "WhiplashHit2t";
}

class this.EffectDef.WhiplashHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WhiplashHit";
	mSound = "Sound-Ability-Whiplash_Effect.ogg";
	mDuration = 10.0;
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
			particleSystem = "Par-Whiplash_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Whiplash_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Whiplash_Fill",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Whiplash_Fill2",
			emitterPoint = "attacking"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.WhiplashHit2t extends this.EffectDef.WhiplashHit
{
	static mEffectName = "WhiplashHit2t";
	mDuration = 15.0;
}

