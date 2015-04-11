this.require("EffectDef");
class this.EffectDef.FelinesGrace extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FelinesGrace";
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
			particleSystem = "Par-FelinesGrace_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("FelinesGraceTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.FelinesGraceTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FelinesGraceTarget";
	mSound = "Sound-Ability-Felinesgrace_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.69999999, "onSparkle");
		this.fireIn(0.89999998, "onSparkleDone");
		this.fireIn(1.5, "onDone");
	}

	function onSparkle( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local sparkle = this.createGroup("Sparkle", this.getSource());
		sparkle.add("ParticleSystem", {
			particleSystem = "Par-FelinesGrace_Symbol2",
			emitterPoint = "node"
		});
	}

	function onSparkleDone( ... )
	{
		this.get("Sparkle").finish();
	}

}

class this.EffectDef.Kick extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Kick";
	mSound = "Sound-Ability-Canopener_Cast.ogg";
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
			initialColor = "97FFFF",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local mWeapon = this._checkMelee();
		this.onAttack();
	}

	function onAttack( ... )
	{
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Kick1"
		});
		this.onSound();
		this.fireIn(0.30000001, "onHit");
		this.fireIn(0.69999999, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("KickHit", this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.KickHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "KickHit";
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
			particleSystem = "Par-Kick_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Kick_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.NotInTheFace extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NotInTheFace";
	mSound = "Sound-Ability-Notintheface_Effect.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Beg",
			speed = 1.5
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NotInTheFace_Gather",
			emitterPoint = "casting"
		});
		local spinner2 = this.createGroup("Spinner2");
		spinner2.add("ParticleSystem", {
			particleSystem = "Par-NotInTheFace_Symbol",
			emitterPoint = "node"
		});
		spinner2.add("Spin", {
			axis = "y",
			speed = 0.69999999,
			extraStopTime = 1.0
		});
		this.onSound();
		this.fireIn(1.0, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("NotInTheFaceTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-NotInTheFace_Gather",
			emitterPoint = "casting"
		});
		local spinner = this.createGroup("Spinner");
		spinner.add("ParticleSystem", {
			particleSystem = "Par-NotInTheFace_Symbol",
			emitterPoint = "node"
		});
		spinner.add("Spin", {
			axis = "y",
			speed = 0.69999999,
			extraStopTime = 1.0
		});
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.NotInTheFaceTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NotInTheFaceTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.Overstrike1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Overstrike1";
	mTargetName = "OverstrikeHit1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spell_Channel"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue(this.mTargetName, this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Burst",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.Overstrike2 extends this.EffectDef.Overstrike1
{
	static mEffectName = "Overstrike2";
	mTargetName = "OverstrikeHit12";
}

class this.EffectDef.Overstrike3 extends this.EffectDef.Overstrike1
{
	static mEffectName = "Overstrike3";
	mTargetName = "OverstrikeHit13";
}

class this.EffectDef.Overstrike4 extends this.EffectDef.Overstrike1
{
	static mEffectName = "Overstrike4";
	mTargetName = "OverstrikeHit14";
}

class this.EffectDef.Overstrike5 extends this.EffectDef.Overstrike1
{
	static mEffectName = "Overstrike5";
	mTargetName = "OverstrikeHit15";
}

class this.EffectDef.OverstrikeHit1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "OverstrikeHit1";
	mSound = "Sound-Ability-Overstrike_Effect.ogg";
	mDuration = 2.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Hands",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Overstrike_Hands",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.OverstrikeHit2 extends this.EffectDef.OverstrikeHit1
{
	static mEffectName = "OverstrikeHit2";
	mDuration = 4.0;
}

class this.EffectDef.OverstrikeHit3 extends this.EffectDef.OverstrikeHit1
{
	static mEffectName = "OverstrikeHit3";
	mDuration = 6.0;
}

class this.EffectDef.OverstrikeHit4 extends this.EffectDef.OverstrikeHit1
{
	static mEffectName = "OverstrikeHit4";
	mDuration = 8.0;
}

class this.EffectDef.OverstrikeHit5 extends this.EffectDef.OverstrikeHit1
{
	static mEffectName = "OverstrikeHit5";
	mDuration = 10.0;
}

class this.EffectDef.Frenzy1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Frenzy1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Dual_Wield_Dagger_Draw"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("FrenzyHit", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.Frenzy2 extends this.EffectDef.Frenzy1
{
	static mEffectName = "Frenzy2";
}

class this.EffectDef.Frenzy3 extends this.EffectDef.Frenzy1
{
	static mEffectName = "Frenzy3";
}

class this.EffectDef.Frenzy4 extends this.EffectDef.Frenzy1
{
	static mEffectName = "Frenzy4";
}

class this.EffectDef.Frenzy5 extends this.EffectDef.Frenzy1
{
	static mEffectName = "Frenzy5";
}

class this.EffectDef.FrenzyHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrenzyHit";
	mSound = "Sound-Ability-Frenzy_Effect.ogg";
	mDuration = 10.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Hands",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Hands",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.Debilitate1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Debilitate1_1";
	mSound = "Sound-Ability-Debilitate_Cast.ogg";
	mTargetName = "DebilitateHit1_1";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue(this.mTargetName, this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.Debilitate1_2 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate1_2";
	mTargetName = "DebilitateHit1_2";
}

class this.EffectDef.Debilitate1_3 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate1_3";
	mTargetName = "DebilitateHit1_3";
}

class this.EffectDef.Debilitate1_4 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate1_4";
	mTargetName = "DebilitateHit1_4";
}

class this.EffectDef.Debilitate1_5 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate1_5";
	mTargetName = "DebilitateHit1_5";
}

class this.EffectDef.Debilitate2_1 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate2_1";
	mTargetName = "DebilitateHit2_1";
}

class this.EffectDef.Debilitate2_2 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate2_2";
	mTargetName = "DebilitateHit2_2";
}

class this.EffectDef.Debilitate2_3 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate2_3";
	mTargetName = "DebilitateHit2_3";
}

class this.EffectDef.Debilitate2_4 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate2_4";
	mTargetName = "DebilitateHit2_4";
}

class this.EffectDef.Debilitate2_5 extends this.EffectDef.Debilitate1_1
{
	static mEffectName = "Debilitate2_5";
	mTargetName = "DebilitateHit2_5";
}

class this.EffectDef.DebilitateHit1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DebilitateHit1_1";
	mSound = "Sound-Ability-Debilitate_Effect.ogg";
	mDuration = 0.40000001;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Burst2",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Impact",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Debilitate_Sparkle",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.DebilitateHit1_2 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit1_2";
	mDuration = 0.80000001;
}

class this.EffectDef.DebilitateHit1_3 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit1_3";
	mDuration = 1.2;
}

class this.EffectDef.DebilitateHit1_4 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit1_4";
	mDuration = 1.6;
}

class this.EffectDef.DebilitateHit1_5 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit1_5";
	mDuration = 2.0;
}

class this.EffectDef.DebilitateHit2_1 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit2_1";
	mDuration = 0.60000002;
}

class this.EffectDef.DebilitateHit2_2 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit2_2";
	mDuration = 1.2;
}

class this.EffectDef.DebilitateHit2_3 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit2_3";
	mDuration = 1.8;
}

class this.EffectDef.DebilitateHit2_4 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit2_4";
	mDuration = 2.4000001;
}

class this.EffectDef.DebilitateHit2_5 extends this.EffectDef.DebilitateHit1_1
{
	static mEffectName = "DebilitateHit2_5";
	mDuration = 3.0;
}

class this.EffectDef.ImpactShot extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ImpactShot";
	mWeaponGroup = "";
	mAttachment = null;
	mSound = "Sound-Ability-Impactshot_Cast.ogg";
	mDuration = 2.0;
	function onStart( ... )
	{
		if (!this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No source");
			}

			return;
		}
		else if (!this.getTarget())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": Target set to " + this.getSource().getTargetObject());
			}

			this.getNarrative().setTargets([
				this.getSource().getTargetObject()
			]);

			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": Target set to " + this.getTarget());
			}
		}

		if (!this.getTarget())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No Target");
			}

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

		this.fireIn(30.0, "onDone");
		this.onSound();
	}

	function onBiped( ... )
	{
		local weaponGroups = this.getSource().getHandAttachments();

		foreach( a in weaponGroups )
		{
			local wt = a.getWeaponType();

			if (wt == "Small")
			{
				wt = "1h";
			}

			if (wt == "Bow" || wt == "Wand" || wt == "1h")
			{
				this.mWeaponGroup = wt;

				if (wt == "1h")
				{
					this.mAttachment = a;
				}
			}
		}

		switch(this.mWeaponGroup)
		{
		case "Bow":
			this.getTarget().cork();
			this.onBow();
			break;

		case "Wand":
			this.getTarget().cork();
			this.onWand();
			break;

		case "1h":
			this.getTarget().cork();
			this.onThrown();
			break;

		default:
			if (this.gLogEffects)
			{
				this.log.debug("Effect " + this.mEffectName + ": No range attachment found.");
			}

			this.onDone();
		}
	}

	function onCreature( ... )
	{
		local anim = this.createGroup("Animation", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			anim.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.30000001, "onWandProjectile");
		}
		else
		{
			this.fireIn(0.40000001, "onWandProjectile");
		}

		local ra = this.createGroup("RangedAttack", this.getSource());
		ra.add("ParticleSystem", {
			particleSystem = "Par-Wand_Charge3",
			emitterPoint = "horde_caster"
		});
	}

	function onBow( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		local anim = this.createGroup("Animation", this.getSource());
		anim.add("FFAnimation", {
			animation = "Bow_Attack_Release_v2",
			speed = 1.8,
			events = {
				[0.30000001] = "onAddArrow",
				[0.80000001] = "onBowProjectile"
			}
		});
	}

	function onAddArrow( ... )
	{
		local ra = this.createGroup("RangedAttack", this.getSource());
		ra.add("Mesh", {
			mesh = "Item-Arrow.mesh",
			point = "right_hand_Arrow",
			fadeInTime = 0.5,
			fadeOutTime = 0.0099999998
		});
		ra.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Ball",
			emitterPoint = "left_hand"
		});
	}

	function onBowProjectile( ... )
	{
		this.get("RangedAttack").finish();

		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("RangedAttack").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		if (this.gLogEffects)
		{
			this.log.debug("Effect " + this.mEffectName + ": onBowProjectile called");
		}

		local proj = this.createGroup("Projectile");
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 6.0,
			accel = 12.0,
			topSpeed = 100.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Arrow.mesh",
			fadeInTime = 0.0099999998
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Ball",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.30000001,
			width = 0.60000002,
			height = 5.0,
			maxSegments = 32,
			initialColor = "fff3ca",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
	}

	function onWand( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		local anim = this.createGroup("Animation", this.getSource());
		anim.add("FFAnimation", {
			animation = "Magic_Wand_Point"
		});
		local ra = this.createGroup("RangedAttack", this.getSource());
		ra.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Wand",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.5, "onWandProjectile");
	}

	function onWandProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("RangedAttack").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local proj = this.createGroup("Projectile");

		if (this.mEffectsPackage == "Biped")
		{
			proj.add("MoveToTarget", {
				source = this.getSource(),
				sourcePoint = "right_hand",
				target = this.getTarget(),
				targetPoint = "spell_target",
				intVelocity = 6.0,
				accel = 12.0,
				topSpeed = 100.0,
				orient = true
			});
		}
		else
		{
			proj.add("MoveToTarget", {
				source = this.getSource(),
				sourcePoint = "horde_caster",
				target = this.getTarget(),
				targetPoint = "spell_target",
				intVelocity = 6.0,
				accel = 12.0,
				topSpeed = 100.0,
				orient = true
			});
		}

		proj.add("Mesh", {
			mesh = "Item-Bolt_Long.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Ball",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.80000001,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "FFE600",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
	}

	function onThrown( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		local ra = this.createGroup("RangedAttack", this.getSource());
		ra.add("FFAnimation", {
			animation = "Sneak_Throw4"
		});
		ra.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Ball",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.55000001, "onThrownProjectile");
	}

	function onThrownProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		if (this.gLogEffects)
		{
			this.log.debug("Effect " + this.mEffectName + ": onWandProjectile called");
		}

		local proj = this.createGroup("Projectile");
		proj.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Ball",
			emitterPoint = "node"
		});
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 3.0,
			topSpeed = 100.0,
			orient = true,
			arcEnd = 0.40000001,
			arcSideAngle = 0.0,
			arcForwardAngle = 60.0
		});
		local mesh = this.createGroup("Mesh");
		mesh.detach();
		local meshNode = mesh.getObject().getNode();
		local projNode = proj.getObject().getNode();

		if (meshNode.getParent())
		{
			meshNode.getParent().removeChild(meshNode);
		}

		projNode.addChild(meshNode);
		meshNode.rotate(this.Vector3(0, 1, 0), 1.5700001);
		mesh.add("Spin", {
			axis = "z",
			speed = 3
		});
		mesh.add("Mesh", {
			mesh = this.mAttachment.getMeshName() + ".mesh",
			texture = this.mAttachment.getTextureName()
		});
		mesh.add("Ribbon", {
			offset = 1.0,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "fff3ca",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		mesh.add("Ribbon", {
			offset = -2.5999999,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "fff3ca",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		this.getSource().hideHandAttachments();
	}

	function onContact( ... )
	{
		this.getTarget().cue("ImpactShotHit", this.getTarget());
		this.finish();
	}

}

class this.EffectDef.ImpactShotHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ImpactShotHit";
	mSound = "Sound-Ability-Impactshot_Effect.ogg";
	mDuration = 10.0;
	function onStart( ... )
	{
		local core = this.createGroup("GetHit", this.getTarget());
		core.add("FFAnimation", {
			animation = "$HIT$"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onStopCastFire");
		this.fireIn(0.0099999998, "onParticles");
		this.onParticles();
		this.getTarget().uncork();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getTarget());
		particles.add("ParticleSystem", {
			particleSystem = "Par-ImpactShot_Burst",
			emitterPoint = "spell_target"
		});
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("GetHit").finish();
	}

}

