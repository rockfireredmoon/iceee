this.require("EffectDef");
class this.EffectDef.ThunderWalkerBolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThunderWalkerBolt";
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

		this.getTarget().cork();
	}

	function onBiped( ... )
	{
		this.onDone();
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.30000001, "onProjectile");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.69999999, "onProjectile");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Ball",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Column",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Fill2",
			emitterPoint = "horde_caster"
		});
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
				this.onDone();
			}
		}

		this.get("Particles").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local proj = this.createGroup("Projectile");
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourcePoint = "horde_caster",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 8.0,
			accel = 10.0,
			topSpeed = 20.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Bolt_Long.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Lightning1",
			emitterPoint = "node"
		});
		proj.add("Spin", {
			speed = 13.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
	}

	function onContact( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		this.getTarget().cue("ThunderWalkerBoltHit", this.getTarget());
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ShakyStomp extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShakyStomp";
	mSound = "stomp.ogg";
	function onStart( ... )
	{
		this.addShaky(65, 0.25, 150.0);
		this.onSound();
	}

}

class this.EffectDef.ThunderWalkerBoltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThunderWalkerBoltHit";
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
			particleSystem = "Par-Lightning_Impact",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.30000001, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Force_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.ThunderWalkerBlast1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThunderWalkerBlast1";
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

		this.getTarget().cork();
	}

	function onBiped( ... )
	{
		this.onDone();
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.30000001, "onProjectile");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.69999999, "onProjectile");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Ball",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Column",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Fill2",
			emitterPoint = "horde_caster"
		});
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
				this.onDone();
			}
		}

		this.get("Particles").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local proj = this.createGroup("Projectile");
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourcePoint = "horde_caster",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 8.0,
			accel = 10.0,
			topSpeed = 20.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Bolt_Long.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Lightning2",
			emitterPoint = "node"
		});
		proj.add("Spin", {
			speed = 13.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
	}

	function onContact( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		this.getTarget().cue("ThunderWalkerBlastHit", this.getTarget());
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ThunderWalkerBlast2 extends this.EffectDef.ThunderWalkerBlast1
{
	static mEffectName = "ThunderWalkerBlast2";
}

class this.EffectDef.ThunderWalkerBlast3 extends this.EffectDef.ThunderWalkerBlast1
{
	static mEffectName = "ThunderWalkerBlast3";
}

class this.EffectDef.ThunderWalkerBlast4 extends this.EffectDef.ThunderWalkerBlast1
{
	static mEffectName = "ThunderWalkerBlast4";
}

class this.EffectDef.ThunderWalkerBlast5 extends this.EffectDef.ThunderWalkerBlast1
{
	static mEffectName = "ThunderWalkerBlast5";
}

class this.EffectDef.ThunderWalkerBlastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThunderWalkerBlastHit";
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
			particleSystem = "Par-Lightning_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Impact",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.30000001, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Force_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.Claw extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Claw";
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
			this.onDone();
		}
		else
		{
			this.onCreature();
		}
	}

	function onCreature()
	{
		this.onSound();
		this.getTarget().cork();
		local a = this.createGroup("Anim1", this.getSource());

		if (this.mEffectsPackage == "Boss-Dragon")
		{
			a.add("FFAnimation", {
				animation = "Claw",
				events = this._setHit(this.getSource(), "Claw")
			});
		}
		else
		{
			a.add("FFAnimation", {
				animation = "Attack1",
				events = this._setHit(this.getSource(), "Attack1")
			});
		}
	}

	function onHit( ... )
	{
		this.getTarget().cue("AssaultHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ClawHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ClawHit";
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

class this.EffectDef.FlytrapPoisonSpray1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FlytrapPoisonSpray1";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "HissBreath"
		});
		this.fireIn(0.5, "onSpray");
	}

	function onSpray( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-PoisonArc",
			emitterPoint = "horde_attacker"
		});
		this.getTarget().cue("PoisonSprayTarget", this.getTarget());
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.FlytrapPoisonSpray2 extends this.EffectDef.FlytrapPoisonSpray1
{
	static mEffectName = "FlytrapPoisonSpray2";
}

class this.EffectDef.FlytrapPoisonSpray3 extends this.EffectDef.FlytrapPoisonSpray1
{
	static mEffectName = "FlytrapPoisonSpray3";
}

class this.EffectDef.FlytrapPoisonSpray4 extends this.EffectDef.FlytrapPoisonSpray1
{
	static mEffectName = "FlytrapPoisonSpray4";
}

class this.EffectDef.FlytrapPoisonSpray5 extends this.EffectDef.FlytrapPoisonSpray1
{
	static mEffectName = "FlytrapPoisonSpray5";
}

class this.EffectDef.PoisonSprayTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PoisonSprayTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hitf", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-PoisonSprayHit",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.FireBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FireBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Fire_Breath_Warmup",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		this.get("Warmup").finish();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Breath",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("FireBreathHit", this.getTarget());
	}

}

class this.EffectDef.FireBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FireBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Breath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FrostBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Breath_Warmup",
			emitterPoint = "limbs"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Breath",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Breath2",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("FrostBreathHit", this.getTarget());
	}

}

class this.EffectDef.FrostBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Frost_Breath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Frost_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.AcidBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AcidBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Acid_Breath_Warmup",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Acid_Breath",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("AcidBreathHit", this.getTarget());
	}

}

class this.EffectDef.AcidBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AcidBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Acid_Breath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Acid_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.MysticBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Mystic_Breath_Warmup",
			emitterPoint = "primary"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Mystic_Breath",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("MysticBreathHit", this.getTarget());
	}

}

class this.EffectDef.MysticBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Mystic_Breath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Mystic_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.DeathBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Death_Breath_Warmup",
			emitterPoint = "primary"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Death_Breath",
			emitterPoint = "casting"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("DeathBreathHit", this.getTarget());
	}

}

class this.EffectDef.DeathBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Death_Breath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Death_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.LightningBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LightningBreath";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Fire"
		});
		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Breath_Warmup",
			emitterPoint = "limbs"
		});
		this.fireIn(0.69999999, "onSpray");
	}

	function onSpray( ... )
	{
		this.get("Warmup").finish();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Breath",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Breath2",
			emitterPoint = "limbs"
		});
		this.fireIn(0.69999999, "onImpact");
		this.fireIn(0.80000001, "onDone");
	}

	function onImpact( ... )
	{
		this.getTarget().cue("LightningBreathHit", this.getTarget());
	}

}

class this.EffectDef.LightningBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LightningBreathHit";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Lightning_Breath_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SpellFailure extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpellFailure";
	mSound = "Sound-SpellFailure.ogg";
	function onStart( ... )
	{
		this.onSound();
	}

}

class this.EffectDef.Mummify extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Mummify";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Mummify_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Mummify_Hand",
			emitterPoint = "right_hand"
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.69999999, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Mummify_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("MummifyHit");
	}

}

class this.EffectDef.MummifyHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MummifyHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-Mummify_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Mummify_Explosion2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Mummify_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.PyreBlast extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PyreBlast";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Throw"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.80000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion4",
			emitterPoint = "casting"
		});
		this.getTarget().cue("PyreBlastMainHit", this.getTarget());
		this._cueSecondary("PyreBlastHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.PyreBlastMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PyreBlastMainHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.node.getWorldPosition();
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast_Combustion_Ring"
		});
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast-Fire_Circle"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.PyreBlastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PyreBlastHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FireFallWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FireFallWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Warmup",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FireFall extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FireFall";
	mSound = "Sound-Ability-Froststorm_Effect.ogg";
	mCount = 0;
	mCTargets = null;
	mMainTargets = null;
	mInitialCount = 0;
	mMainCount = 0;
	mY = 200.0;
	mRandom = null;
	function _randLocation()
	{
		local randNumber = this.mRandom.nextInt(150);
		local x = randNumber - 75.0;
		randNumber = this.mRandom.nextInt(150);
		local z = randNumber - 75.0;
		return this.Vector3(x, 0.0, z);
	}

	function _createRandomTarget( groupName, primaryGroupName, ... )
	{
		local groundSnap = false;

		if (vargc > 0)
		{
			groundSnap = vargv[0];
		}

		local group = this.createGroup(groupName);
		local primaryPos = this.get(primaryGroupName).getObject().getNode().getWorldPosition();
		local pos = primaryPos + this._randLocation();

		if (groundSnap)
		{
			local y = this.Util.getFloorHeightAt(pos, 15.0, this.QueryFlags.FLOOR, false);
			pos = this.Vector3(pos.x, y, pos.z);
		}

		this._detachGroup(group, pos);
		return group;
	}

	function _createOffsetTarget( groupName, primaryGroupName, originGroupName )
	{
		local group = this.createGroup(groupName);
		local offsetVector = this.Vector3(50.0, 0.0, 50.0);
		local primaryPos = this.get(primaryGroupName).getObject().getNode().getWorldPosition();
		local originWorldPos = this.get(originGroupName).getObject().getNode().getWorldPosition();
		local originLocalPos = originWorldPos - primaryPos;
		local newVector = this.Vector3(offsetVector.x + originLocalPos.x, this.mY, offsetVector.z + originLocalPos.z);
		local pos = primaryPos + newVector;
		this._detachGroup(group, pos);
		return group;
	}

	function _distanceCheck( bottomGroupName, projGroupName )
	{
		local botTarget = this.get(bottomGroupName);
		local proj = this.get(projGroupName);
		local botPos = botTarget.getObject().getNode().getPosition();
		local projPos = proj.getObject().getNode().getPosition();
		return ::Math.DetermineDistanceBetweenTwoPoints(projPos, botPos);
	}

	function onStart( ... )
	{
		if (!this._positionalCheck())
		{
			return;
		}

		this.mMainTargets = [];
		local secondaries = this.getTargets();

		if (secondaries)
		{
			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = s;
					s_target.cork();
					this.mMainTargets.append(s_target);
				}
			}
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Warmup",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onStormCloud");
		this.fireIn(6.4000001, "onDone");
		this.mRandom = this.Random();
	}

	function onStormCloud( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Cast_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.1, "onBurstStop");
		local cloud = this.createGroup("Cloud");
		this._detachGroup(cloud, this.getPositionalTarget());
		cloud.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = cloud.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 300,
			orthoHeight = 300,
			offset = this.Vector3(0, 200, 0),
			far = 300,
			additive = true
		});
		local time = 0.40000001;

		while (time < 5.0)
		{
			this.fireIn(time, "onProjectile");
			time = time + 0.2;
		}

		local sound = this.createGroup("Sound");
		this._detachGroup(sound, this.getPositionalTarget());
		sound.add("Sound", {
			sound = this.mSound
		});
	}

	function onBurstStop( ... )
	{
		this.get("Burst").finish();
	}

	function onProjectile( ... )
	{
		local botTarget;

		if (this.mInitialCount < 5 || this.mMainTargets.len() <= this.mMainCount)
		{
			local botTargetParent = this._createRandomTarget("bottomTarget" + this.mCount, "Cloud", true);
			botTarget = botTargetParent;
			this.mInitialCount++;
		}
		else if (this.mMainCount < this.mMainTargets.len())
		{
			botTarget = this.createGroup("bottomTarget" + this.mCount, this.mMainTargets[this.mMainCount]);
			this.mMainCount++;
		}

		local topTarget = this._createOffsetTarget("topTarget" + this.mCount, "Cloud", "bottomTarget" + this.mCount);
		local proj = this.createGroup("projectile" + this.mCount);
		proj.add("MoveToTarget", {
			source = topTarget.getObject(),
			target = botTarget.getObject(),
			intVelocity = 6.0,
			accel = 12.0,
			topSpeed = 100.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Sphere.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Trail",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -3.0,
			width = 6.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "EE2C2C",
			colorChange = [
				2.0,
				2.0,
				2.0,
				2.0
			]
		});
		proj.add("ScaleTo", {
			size = 2.0,
			maintain = true,
			duration = 0.0
		});

		if (this.mCTargets == null)
		{
			this.mCTargets = [];
		}

		this.mCTargets.append(this.mCount);
		this.mCount++;

		if (this.mInitialCount < 5 && this.mMainCount < this.mMainTargets.len())
		{
			this.onProjectile();
		}
	}

	function onContact( ... )
	{
		for( local i = this.mCTargets.len() - 1; i > -1; i-- )
		{
			local bottomGroupName = "bottomTarget" + this.mCTargets[i];
			local projGroupName = "projectile" + this.mCTargets[i];
			local distance = this._distanceCheck(bottomGroupName, projGroupName);

			if (distance == 0)
			{
				local botTarget = this.get(bottomGroupName).getObject();

				foreach( k in this.mMainTargets )
				{
					if (botTarget == k)
					{
						botTarget.cue("FireFallHit", botTarget);
						botTarget.uncork();
					}
				}

				this.get("bottomTarget" + this.mCTargets[i]).finish();
				this.get("topTarget" + this.mCTargets[i]).finish();
				this.get("projectile" + this.mCTargets[i]).finish();
				this.mCTargets.remove(i);
			}
		}
	}

}

class this.EffectDef.FireFallHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FireFallHit";
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
			particleSystem = "Par-FireFall_Burst",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FireFall_Explosion",
			emitterPoint = "node"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FloralFodder extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FloralFodder";
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
			particleSystem = "Par-FloralFodder_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("FloralFodderTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.FloralFodderTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FloralFodderTarget";
	mSound = "Sound-Ability-Thorns_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Symbol1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.VineLashWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "VineLashWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FloralFodder_Gather",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-VineLash_Particles",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.VineLash extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VineLash";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "458B00",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Spinning_Daggers"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-VineLash_Particles",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("VineLashTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.VineLashTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VineLashTarget";
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
			particleSystem = "Par-VineLash_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-VineLash_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.DivineProtection extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DivineProtection";
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
			particleSystem = "Par-MysticProtection_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("MysticProtectionTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.DivineProtectionTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DivineProtectionTarget";
	mSound = "Sound-Ability-MysticProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Symbol1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.SweepingBlowWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SweepingBlowWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Gather",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SweepingBlow extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SweepingBlow";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ffff7e",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl1",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("SweepingBlowTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.SweepingBlowTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SweepingBlowTarget";
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
			particleSystem = "Par-SweepingBlow_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.TendonCut extends this.EffectDef.TemplateMelee
{
	static mEffectName = "TendonCut";
	mSound = "Sound-Ability-EnfeeblingBlow_Cast.ogg";
	mHitName = "TendonCutHit";
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
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Sneak_Hamstring1"
		});
		this.fireIn(0.5, "onHitFinal");
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

class this.EffectDef.TendonCutHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TendonCutHit";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 4.0;
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

class this.EffectDef.StabbingFrenzy extends this.EffectDef.TemplateBasic
{
	static mEffectName = "StabbingFrenzy";
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
		this.getSource().cue("StabbingFrenzyHit", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Frenzy_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.StabbingFrenzyHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "StabbingFrenzyHit";
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

class this.EffectDef.SleetStormWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SleetStormWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-SleetStorm_Gather",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-SleetStorm_Gather2",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SleetStorm extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SleetStorm";
	mSound = "Sound-Ability-Frostmire_Cast.ogg";
	function onStart( ... )
	{
		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Started");
		}

		if (!this._targetCheck())
		{
			return;
		}

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": target check passed");
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Cast group setup passed");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Lines",
			emitterPoint = "right_hand"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Particle group setup passed");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.5, "onInitialBurst");
		this.fireIn(0.60000002, "onInitialBurst2");
		this.fireIn(0.69999999, "onBurst");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		blast.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast2",
			emitterPoint = "node"
		});
		this.onSound();
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
		local blast2 = this.createGroup("Blast2");
		blast2.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast1",
			emitterPoint = "node"
		});
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		this.get("Blast2").finish();
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
			textureName = "FrostMire.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("SleetStormHit", true);
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.SleetStormHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SleetStormHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-SleetStorm_Explosion",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FrozenHeart extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrozenHeart";
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
			particleSystem = "Par-FrostProtection_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("FrostProtectionTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.FrozenHeartTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrozenHeartTarget";
	mSound = "Sound-Ability-FrostProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Symbol1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.LawmansGrip extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LawmansGrip";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Hand",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.2, "onNet");
		this.fireIn(0.30000001, "onBurst");
		this.fireIn(0.40000001, "onHit");
		this.fireIn(0.89999998, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onNet( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Net",
			emitterPoint = "node"
		});
	}

	function onHit( ... )
	{
		this._cueSecondary("LawmansGripHit");
	}

}

class this.EffectDef.LawmansGripHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LawmansGripHit";
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
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Daze_Sparkle1",
			emitterPoint = "daze_stun"
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.LawmansWrathWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "LawmansWrathWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Gather",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Gather2",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.LawmansWrath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LawmansWrath";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Hand",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.80000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("LawmansWrathHit", this.getTarget());
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.LawmansWrathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LawmansWrathHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.node.getWorldPosition();
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Ring"
		});
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "burst_add.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.ChaoticReleaseWarmup1 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ChaoticReleaseWarmup1";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
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
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.ChaoticReleaseWarmup2 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ChaoticReleaseWarmup2";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.ChaoticReleaseWarmup3 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ChaoticReleaseWarmup3";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Abolish_Particles",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.ChaoticRelease extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ChaoticRelease";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spellgasm"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.80000001, "onExplodingRing");
	}

	function onExplodingRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("Ring").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("ChaoticReleaseHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.ChaoticReleaseHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ChaoticReleaseHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SwellingVines extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SwellingVines";
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
			particleSystem = "Par-Root_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("RootTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Root_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.SwellingVinesTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SwellingVinesTarget";
	mSound = "Sound-Ability-Thorns_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		local residual = this.createGroup("Residual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Root_Fill",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onInitialDone");
		this.fireIn(7.0, "onDone");
	}

	function onInitialDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.AlimatFirebolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFirebolt";
	mSound = "Sound-Ability-Firebolt_Cast.ogg";
	mLoopSound = "Sound-Ability-Firebolt_Projectile.ogg";
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
		this.onSound();
		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onProjectile");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("Cast").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
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
			mesh = "Item-Sphere.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Embers",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ffb400",
			colorChange = [
				4.0,
				4.0,
				4.0,
				4.0
			]
		});
		proj.add("Spin", {
			speed = 3.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
		this.onLoopSound(proj);
	}

	function onLoopSound( proj )
	{
		if (this.mLoopSound == "")
		{
			return;
		}

		local sound = this.createGroup("LoopSound");
		local node = sound.getObject().getNode();
		node.getParent().removeChild(node);
		proj.getObject().getNode().addChild(node);
		node.setPosition(this.Vector3(0, 0, 0));
		sound.add("Sound", {
			sound = this.mLoopSound
		});
	}

	function onContact( ... )
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

		this.getTarget().cue("AlimatFireboltHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.AlimatFireboltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFireboltHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
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
			particleSystem = "Par-Fire_Explosion3",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.AlimatFireAOE extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFireAOE";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Throw"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.80000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion4",
			emitterPoint = "casting"
		});
		this.getTarget().cue("AlimatFireAOEMainHit", this.getTarget());
		this._cueSecondary("AlimatFireAOEHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.AlimatFireAOEMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFireAOEMainHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast_Combustion_Ring"
		});
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast-Fire_Circle"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.AlimatFireAOEHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFireAOEHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-PyreBlast_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.AlimatFrostbolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFrostbolt";
	mSound = "Sound-Ability-Forceblast_Cast.ogg";
	mLoopSound = "Sound-Ability-Forcebolt_Projectile.ogg";
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
		this.onSound();
		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Hand",
			emitterPoint = "casting"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onProjectile");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("Cast").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
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
			mesh = "Item-Spear.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Snow",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "AFEEEE",
			colorChange = [
				4.0,
				4.0,
				4.0,
				4.0
			]
		});
		proj.add("Spin", {
			speed = 3.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
		this.onLoopSound(proj);
	}

	function onLoopSound( proj )
	{
		if (this.mLoopSound == "")
		{
			return;
		}

		local sound = this.createGroup("LoopSound");
		local node = sound.getObject().getNode();
		node.getParent().removeChild(node);
		proj.getObject().getNode().addChild(node);
		node.setPosition(this.Vector3(0, 0, 0));
		sound.add("Sound", {
			sound = this.mLoopSound
		});
	}

	function onContact( ... )
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

		this.getTarget().cue("AlimatFrostboltHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.AlimatFrostboltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFrostboltHit";
	mSound = "Sound-Ability-Forceblast_Effect.ogg";
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
			particleSystem = "Par-FrostSpear_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Burst2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.AlimatFrostAOE extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFrostAOE";
	mSound = "Sound-Ability-Forceblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Throw"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Hand",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.80000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cue("AlimatFrostAOEMainHit", this.getTarget());
		this.fireIn(0.1, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.AlimatFrostAOEMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AlimatFrostAOEMainHit";
	mSound = "Sound-Ability-Forceblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Explosion1",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "FrostMire.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast1"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.FlamingBreathWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FlamingBreathWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlamingBreath_Warmup",
			emitterPoint = "horde_attacker"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FlamingBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FlamingBreath";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Ambient"
		});
		this.onSound();
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("FlamingBreathTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-FlamingBreath_Breath",
			emitterPoint = "horde_attacker"
		});
		this.fireIn(0.89999998, "onDone");
	}

}

class this.EffectDef.FlamingBreathTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FlamingBreathTarget";
	mSound = "Sound-Ability-FireProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Abolish_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Abolish_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Abolish_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.CinderSlash extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CinderSlash";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ffff7e",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl2",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("SweepingBlowTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.CinderSlashTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CinderSlashTarget";
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
			particleSystem = "Par-FlameSpear_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.LastHowl extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LastHowl";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Ambient"
		});
		this.onSound();
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("LastHowlTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-LastHowl_Cast",
			emitterPoint = "horde_attacker"
		});
		this.fireIn(0.89999998, "onDone");
	}

}

class this.EffectDef.LastHowlTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LastHowlTarget";
	mSound = "Sound-Ability-FireProtection_Effect.ogg";
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
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-LastHowl_Impact",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.VoodooCurse extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VoodooCurse";
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
			particleSystem = "Par-Deathly_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("VoodooCurseTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-VoodooCurse_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.VoodooCurseTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VoodooCurseTarget";
	mSound = "Sound-Ability-Deathly_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Hands",
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

class this.EffectDef.VoodooCurseSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VoodooCurseSecondaryHit";
	mSound = "Sound-Ability-Deathly_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Sparkle",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.WitchyRevive extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WitchyRevive";
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
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("WitchyReviveTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Healing-Burst2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.WitchyReviveTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WitchyReviveTarget";
	mSound = "Sound-Ability-Healinghand_Effect.ogg";
	static mParticleLife = 0.69999999;
	static mSpinSpeed = 1.2;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff");
		buff.add("ParticleSystem", {
			particleSystem = "Par-Healing-2GlowingGlobes",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		buff.add("Spin", {
			speed = this.mSpinSpeed
		});
		this.onSound();
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-Healing-Imbue",
			emitterPoint = "core",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		this.fireIn(this.mParticleLife, "onParticleDone");
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.BlightBite extends this.EffectDef.TemplateMelee
{
	static mEffectName = "BlightBite";
	mSound = "Sound-Ability-Assault_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Attack1"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.80000001, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("BlightBiteHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.BlightBiteHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlightBiteHit";
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

class this.EffectDef.ConstrictingWebs extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ConstrictingWebs";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});
		this._corkTargets(true);
		this.fireIn(0.30000001, "onExplodingRing");
	}

	function onExplodingRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		this.fireIn(0.1, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Ring").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("ConstrictingWebsHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.ConstrictingWebsHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ConstrictingWebsHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Impact",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.80000001, "onBuff");
		this.fireIn(1.5, "onDone");
	}

	function onBuff( ... )
	{
		this.get("Hit").finish();
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Fill",
			emitterPoint = "node"
		});
	}

}

class this.EffectDef.ConstrictingWebsSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ConstrictingWebsSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ConstrictingWeb_Fill",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.DenmothersRage extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DenmothersRage";
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
		this.onSound();
		this.getTarget().cue("DenmothersRageTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.DenmothersRageTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DenmothersRageTarget";
	mSound = "Sound-Ability-MysticProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-DenmothersRage_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DenmothersRage_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.FrostyFreezeWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FrostyFreezeWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FrostyFreeze_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FrostyFreeze_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FrostyFreeze extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostyFreeze";
	mSound = "Sound-Ability-Frostmire_Cast.ogg";
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
			particleSystem = "Par-FrostyFreeze_Hand",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostyFreeze_Lines",
			emitterPoint = "right_hand"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Particle group setup passed");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.5, "onInitialBurst");
		this.fireIn(0.60000002, "onInitialBurst2");
		this.fireIn(0.69999999, "onBurst");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		blast.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast2",
			emitterPoint = "node"
		});
		this.onSound();
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
		local blast2 = this.createGroup("Blast2");
		blast2.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast1",
			emitterPoint = "node"
		});
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		this.get("Blast2").finish();
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
			textureName = "FrostMire.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("FrostyFreezeHit", true);
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.FrostyFreezeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostyFreezeHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "feet"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FrostyFreezeSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostyFreezeSecondaryHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.ShatteringHailstorm extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShatteringHailstorm";
	mSound = "Sound-Ability-Froststorm_Effect.ogg";
	mCount = 0;
	mCTargets = null;
	mMainTargets = null;
	mInitialCount = 0;
	mMainCount = 0;
	mY = 200.0;
	mRandom = null;
	function _randLocation()
	{
		local randNumber = this.mRandom.nextInt(150);
		local x = randNumber - 75.0;
		randNumber = this.mRandom.nextInt(150);
		local z = randNumber - 75.0;
		return this.Vector3(x, 0.0, z);
	}

	function _createRandomTarget( groupName, primaryGroupName, ... )
	{
		local groundSnap = false;

		if (vargc > 0)
		{
			groundSnap = vargv[0];
		}

		local group = this.createGroup(groupName);
		local primaryPos = this.get(primaryGroupName).getObject().getNode().getWorldPosition();
		local pos = primaryPos + this._randLocation();

		if (groundSnap)
		{
			local y = this.Util.getFloorHeightAt(pos, 15.0, this.QueryFlags.FLOOR, false);
			pos = this.Vector3(pos.x, y, pos.z);
		}

		this._detachGroup(group, pos);
		return group;
	}

	function _createOffsetTarget( groupName, primaryGroupName, originGroupName )
	{
		local group = this.createGroup(groupName);
		local offsetVector = this.Vector3(50.0, 0.0, 50.0);
		local primaryPos = this.get(primaryGroupName).getObject().getNode().getWorldPosition();
		local originWorldPos = this.get(originGroupName).getObject().getNode().getWorldPosition();
		local originLocalPos = originWorldPos - primaryPos;
		local newVector = this.Vector3(offsetVector.x + originLocalPos.x, this.mY, offsetVector.z + originLocalPos.z);
		local pos = primaryPos + newVector;
		group.getObject().getNode().setPosition(pos);
		this._detachGroup(group, pos);
		return group;
	}

	function _distanceCheck( bottomGroupName, projGroupName )
	{
		local botTarget = this.get(bottomGroupName);
		local proj = this.get(projGroupName);
		local botPos = botTarget.getObject().getNode().getPosition();
		local projPos = proj.getObject().getNode().getPosition();
		return ::Math.DetermineDistanceBetweenTwoPoints(projPos, botPos);
	}

	function onStart( ... )
	{
		if (!this._positionalCheck())
		{
			return;
		}

		this.mMainTargets = [];
		local secondaries = this.getTargets();

		if (secondaries)
		{
			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = s;
					s_target.cork();
					this.mMainTargets.append(s_target);
				}
			}
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostyFreeze_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostyFreeze_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onStormCloud");
		this.fireIn(6.4000001, "onDone");
		this.mRandom = this.Random();
	}

	function onStormCloud( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Cast_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.1, "onBurstStop");
		local cloud = this.createGroup("Cloud");
		this._detachGroup(cloud, this.getPositionalTarget());
		cloud.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Cloud2",
			emitterPoint = "node"
		});
		cloud.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = cloud.getObject(),
			textureName = "FrostStorm_Ring.png",
			orthoWidth = 300,
			orthoHeight = 300,
			offset = this.Vector3(0, 200, 0),
			far = 300,
			additive = true
		});
		local time = 0.40000001;

		while (time < 5.0)
		{
			this.fireIn(time, "onProjectile");
			time = time + 0.2;
		}

		local sound = this.createGroup("Sound");
		this._detachGroup(sound, this.getPositionalTarget());
		sound.add("Sound", {
			sound = this.mSound
		});
	}

	function onBurstStop( ... )
	{
		this.get("Burst").finish();
	}

	function onProjectile( ... )
	{
		local botTarget;

		if (this.mInitialCount < 5 || this.mMainTargets.len() <= this.mMainCount)
		{
			local botTargetParent = this._createRandomTarget("bottomTarget" + this.mCount, "Cloud", true);
			botTarget = botTargetParent;
			this.mInitialCount++;
		}
		else if (this.mMainCount < this.mMainTargets.len())
		{
			botTarget = this.createGroup("bottomTarget" + this.mCount, this.mMainTargets[this.mMainCount]);
			this.mMainCount++;
		}

		local topTarget = this._createOffsetTarget("topTarget" + this.mCount, "Cloud", "bottomTarget" + this.mCount);
		local proj = this.createGroup("projectile" + this.mCount);
		proj.add("MoveToTarget", {
			source = topTarget.getObject(),
			target = botTarget.getObject(),
			intVelocity = 6.0,
			accel = 12.0,
			topSpeed = 100.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Shard.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Snow",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -3.0,
			width = 6.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "009cff",
			colorChange = [
				2.0,
				2.0,
				2.0,
				2.0
			]
		});
		proj.add("ScaleTo", {
			size = 2.0,
			maintain = true,
			duration = 0.0
		});

		if (this.mCTargets == null)
		{
			this.mCTargets = [];
		}

		this.mCTargets.append(this.mCount);
		this.mCount++;

		if (this.mInitialCount < 5 && this.mMainCount < this.mMainTargets.len())
		{
			this.onProjectile();
		}
	}

	function onContact( ... )
	{
		for( local i = this.mCTargets.len() - 1; i > -1; i-- )
		{
			local bottomGroupName = "bottomTarget" + this.mCTargets[i];
			local projGroupName = "projectile" + this.mCTargets[i];
			local distance = this._distanceCheck(bottomGroupName, projGroupName);

			if (distance == 0)
			{
				local botTarget = this.get(bottomGroupName).getObject();

				foreach( k in this.mMainTargets )
				{
					if (botTarget == k)
					{
						botTarget.cue("ShatteringHailstormHit", botTarget);
						botTarget.uncork();
					}
				}

				this.get("bottomTarget" + this.mCTargets[i]).finish();
				this.get("topTarget" + this.mCTargets[i]).finish();
				this.get("projectile" + this.mCTargets[i]).finish();
				this.mCTargets.remove(i);
			}
		}
	}

}

class this.EffectDef.ShatteringHailstormHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShatteringHailstormHit";
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
			particleSystem = "Par-Frost_Storm_Ice_Burst",
			emitterPoint = "node"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.ObliteratingBlow extends this.EffectDef.TemplateMelee
{
	static mEffectName = "ObliteratingBlow";
	mSound = "Sound-Ability-Assault_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Attack1"
		});
		a.add("ParticleSystem", {
			particleSystem = "Par-ObliteratingBlow_Hand",
			emitterPoint = "attacking"
		});
		this.fireIn(0.2, "onHit");
		this.fireIn(0.80000001, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("ObliteratingBlowHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.onHit();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.ObliteratingBlowHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ObliteratingBlowHit";
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

class this.EffectDef.BoneShaker extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BoneShaker";
	mHitName = "BoneShakerHit";
	mSound = "Sound-Ability-Earth_Shaker_Cast.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2",
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
		this.fireIn(0.60000002, "onImpact");
		this.fireIn(0.69999999, "onImpact1");
		this.fireIn(0.89999998, "onImpact2");
		this.fireIn(1.1, "onImpact3");
		this.fireIn(0.69999999, "onImpactEnd");
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
			orthoWidth = 120,
			orthoHeight = 120,
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

class this.EffectDef.BoneShakerHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BoneShakerHit";
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

class this.EffectDef.GoliathScream extends this.EffectDef.TemplateBasic
{
	static mEffectName = "GoliathScream";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Ambient2"
		});
		this.onSound();
		this.fireIn(0.89999998, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("GoliathScreamTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-GoliathScream_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.GoliathScreamTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "GoliathScreamTarget";
	mSound = "Sound-Ability-FireProtection_Effect.ogg";
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
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-LastHowl_Impact",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.SpreadingRot extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpreadingRot";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 2.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "right_hand"
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.69999999, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("SpreadingRotMainHit");
	}

}

class this.EffectDef.SpreadingRotMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpreadingRotMainHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.SpreadingRotSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpreadingRotSecondaryHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.MarkOfDeathWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "MarkOfDeathWarmup";
	mLoopSound = "Sound-Ability-Morass_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Lines",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-MarkOfDeath_Symbol2",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-MarkOfDeath_Symbol1",
			emitterPoint = "node"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		local skull = this.createGroup("Skull");
		skull.add("Mesh", {
			mesh = "Item-Skull2.mesh",
			point = "node",
			size = 3.0,
			fadeInTime = 0.5,
			fadeOutTime = 1.0
		});
		skull.add("Spin", {
			axis = "y",
			speed = 0.89999998,
			extraStopTime = 1.0
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.MarkOfDeath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MarkOfDeath";
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
			particleSystem = "Par-Deathly_Hands",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("MarkOfDeathTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.MarkOfDeathTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MarkOfDeathTarget";
	mSound = "Sound-Ability-Deathly_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-MarkOfDeath_Symbol2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-MarkOfDeath_Symbol1",
			emitterPoint = "node"
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

class this.EffectDef.Shellify extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Shellify";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Shellify_Sparkle",
			emitterPoint = "node"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Symbol2",
			emitterPoint = "node",
			particleScale = 0.60000002
		});
		this.getTarget().cue("ShellifyTarget", this.getTarget());
		this.onSound();
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.ShellifyTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShellifyTarget";
	mSound = "Sound-Ability-MysticProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-FireProtection_Symbol1",
			emitterPoint = "node",
			particleScale = 0.60000002
		});
		this.onSound();
		this.fireIn(10, "onDone");
	}

}

class this.EffectDef.DreadedClawWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "DreadedClawWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-DreadedClaw_Gather",
			emitterPoint = "crabclaw1"
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.DreadedClaw extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DreadedClaw";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Claw"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-DreadedClaw_Gather",
			emitterPoint = "crabclaw1"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-DreadedClaw_Sparkle",
			emitterPoint = "crabclaw1"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("DreadedClawTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.DreadedClawTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DreadedClawTarget";
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

class this.EffectDef.SlowingFreeze extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SlowingFreeze";
	mSound = "Sound-Ability-Frostmire_Cast.ogg";
	function onStart( ... )
	{
		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Started");
		}

		if (!this._targetCheck())
		{
			return;
		}

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": target check passed");
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Cast group setup passed");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Lines",
			emitterPoint = "casting"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Particle group setup passed");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.5, "onInitialBurst");
		this.fireIn(0.60000002, "onInitialBurst2");
		this.fireIn(0.69999999, "onBurst");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		blast.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast2",
			emitterPoint = "node"
		});
		this.onSound();
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
		local blast2 = this.createGroup("Blast2");
		blast2.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast1",
			emitterPoint = "node"
		});
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		this.get("Blast2").finish();
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
			textureName = "FrostMire.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("SlowingFreezeHit", true);
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.SlowingFreezeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SlowingFreezeHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "feet"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SlowingFreezeSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SlowingFreezeSecondaryHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SwirlingMixupWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SwirlingMixupWarmup";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SwirlingMixup extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SwirlingMixup";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("SwirlingMixupTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Portal_Burst",
			emitterPoint = "casting"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Earthshaker_Ring",
			emitterPoint = "node",
			particleScale = 0.5
		});
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.SwirlingMixupTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SwirlingMixupTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Symbol",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.SpiritLock extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpiritLock";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Hand",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.5, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("SpiritLockMainHit", this.getTarget());
		this._cueSecondary("SpiritLockHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.SpiritLockMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpiritLockMainHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Circle"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SpiritLockHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpiritLockHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Symbol2",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MysticProtection_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.PsycheLock extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PsycheLock";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Hand",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.5, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("PsycheLockMainHit", this.getTarget());
		this._cueSecondary("PsycheLockHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.PsycheLockMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PsycheLockMainHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FireProtection_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Ring"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.PsycheLockHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PsycheLockHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FireProtection_Symbol2",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.RoundABash extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RoundABash";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ffff7e",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Two_Handed_Furys_Eye"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl1",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("RoundABashTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.RoundABashTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RoundABashTarget";
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
			particleSystem = "Par-SweepingBlow_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.NauseatingBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NauseatingBreath";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Potion_Drink"
		});
		cast.add("Mesh", {
			mesh = "Item-PotionConstitution.mesh",
			point = "left_hand",
			fadeInTime = 0.2,
			fadeOutTime = 0.2
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NauseatingBreath_Cast",
			emitterPoint = "left_hand"
		});
		this._corkTargets(true);
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Morass.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Morass_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("NauseatingBreathHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.NauseatingBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NauseatingBreathHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NauseatingBreat_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NauseatingBreath_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FocusedRageWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FocusedRageWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Hand",
			emitterPoint = "right_hand"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Particles",
			emitterPoint = "right_hand"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FocusedRage extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FocusedRage";
	mSound = "Sound-Ability-Firebolt_Cast.ogg";
	mLoopSound = "Sound-Ability-Firebolt_Projectile.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Sneak_Throw1"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Hand",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.30000001, "onProjectile");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("Cast").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
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
			mesh = "Item-Spear.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Embers",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ffb400",
			colorChange = [
				4.0,
				4.0,
				4.0,
				4.0
			]
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
		this.onLoopSound(proj);
	}

	function onLoopSound( proj )
	{
		if (this.mLoopSound == "")
		{
			return;
		}

		local sound = this.createGroup("LoopSound");
		local node = sound.getObject().getNode();
		node.getParent().removeChild(node);
		proj.getObject().getNode().addChild(node);
		node.setPosition(this.Vector3(0, 0, 0));
		sound.add("Sound", {
			sound = this.mLoopSound
		});
	}

	function onContact( ... )
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

		this.getTarget().cue("FocusedRageHit", this.getTarget());
		this.getTarget().uncork();
		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.FocusedRageHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FocusedRageHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
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
			particleSystem = "Par-FlameSpear_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.FocusedRageSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FocusedRageSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.DistortedRage extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DistortedRage";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spellgasm"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.30000001, "onExplodingRing");
	}

	function onExplodingRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		this.fireIn(0.2, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("Ring").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("DistortedRageHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.DistortedRageHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DistortedRageHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.DistortedRageSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DistortedRageSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.UnsettledMind extends this.EffectDef.TemplateBasic
{
	static mEffectName = "UnsettledMind";
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
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("UnsettledMindTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.UnsettledMindTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "UnsettledMindTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Portal_Symbol",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.PoisonousPest extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PoisonousPest";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "right_hand"
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.69999999, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("PoisonousPestHit");
	}

}

class this.EffectDef.PoisonousPestMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PoisonousPestMainHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.PoisonousPestSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PoisonousPestSecondaryHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.PoisonousPestHeal extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PoisonousPestHeal";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
	function onStart( ... )
	{
		local heal = this.createGroup("Heal", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-PoisonousPestHeal_Symbol",
			emitterPoint = "casting"
		});
		heal.add("ParticleSystem", {
			particleSystem = "Par-PoisonousPestHeal_Symbol2",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.MysticalMuteWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "MysticalMuteWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Cast",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.MysticalMute extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticalMute";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Bite"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cork();
		this._corkSecondary();
		this.fireIn(0.80000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cue("MysticalMuteMainHit", this.getTarget());
		this._cueSecondary("MysticalMuteHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.MysticalMuteMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticalMuteMainHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Hit"
		});
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		ground.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Circle"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.MysticalMuteHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticalMuteHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Symbol",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MysticalMute_Hit",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.SproutedObstruction1Warmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SproutedObstruction1Warmup";
	mLoopSound = "Sound-Ability-Haste_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Root_Cast",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SproutedObstruction1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SproutedObstruction1";
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
			particleSystem = "Par-Root_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("SproutedObstruction1Hit", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Root_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.SproutedObstruction1Hit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SproutedObstruction1Hit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol2",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		local ground = this.createGroup("Ground");
		local primaryPos = this.get("Hit").getObject().getNode().getWorldPosition();
		this._detachGroup(ground, primaryPos);
		ground.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = ground.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.SproutedObstruction2Channel extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SproutedObstruction2Channel";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 150,
			orthoHeight = 150,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SproutedObstruction2Hit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SproutedObstruction2Hit";
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
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.FrelonSwarmWarmup extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrelonSwarmWarmup";
	mLoopSound = "Sound-Ability-Swarm_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Hand",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Particles",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FrelonSwarm extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrelonSwarm";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Burst",
			emitterPoint = "casting"
		});
		this.fireIn(0.2, "onNet");
		this.fireIn(0.30000001, "onBurst");
		this.fireIn(0.40000001, "onHit");
		this.fireIn(1.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onNet( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Ring",
			emitterPoint = "node"
		});
	}

	function onHit( ... )
	{
		this._cueSecondary("StingingWebshotHit");
	}

}

class this.EffectDef.FrelonSwarmHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrelonSwarmHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
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
			particleSystem = "Par-Swarm_Hit",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.Enslave extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Enslave";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Lines",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("EnslaveHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.EnslaveHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "EnslaveHit";
	mSound = "Sound-Ability-Thorns_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		local residual = this.createGroup("Residual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Root_Fill",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onInitialDone");
		this.fireIn(7.0, "onDone");
	}

}

class this.EffectDef.PunishChannel extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PunishChannel";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("FFAnimation", {
			animation = "Attack2",
			loop = true
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "casting",
			particleScale = 0.2
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 150,
			orthoHeight = 150,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.PunishHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PunishHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.Combust extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Combust";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spellgasm"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.80000001, "onExplodingRing");
	}

	function onExplodingRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("Ring").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("CombustHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.CombustHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CombustHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.CombustSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CombustSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.Dismissed extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Dismissed";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ffff7e",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Dual_Wield_Figure_8"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl1",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("DismissedTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.DismissedTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DismissedTarget";
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
			particleSystem = "Par-Portal_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Fill",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Symbol",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill2",
			emitterPoint = "feet"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.SpeedTrap extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpeedTrap";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local weapon = this.createGroup("Weapon", this.getSource());
		weapon.add("WeaponRibbon", {
			initialColor = "ffff7e",
			colorChange = [
				8.0,
				8.0,
				8.0,
				8.0
			]
		});
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("ParticleSystem", {
			particleSystem = "Par-FrostProtection_Gather",
			emitterPoint = "casting"
		});
		cast.add("FFAnimation", {
			animation = "Spinning_Daggers"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl1",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("SpeedTrapTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.SpeedTrapTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpeedTrapTarget";
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
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill2",
			emitterPoint = "casting"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Enfeebling_Fill2",
			emitterPoint = "feet"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.IcyBreath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "IcyBreath";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Exclaim"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand",
			emitterPoint = "casting",
			particleScale = 0.5
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.69999999, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "FrostMire.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("IcyBreathHit");
	}

}

class this.EffectDef.IcyBreathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "IcyBreathHit";
	mSound = "Sound-Ability-Frozen_Effect.ogg";
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
			particleSystem = "Par-Coldsnap_Explosion1",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Normalize extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Normalize";
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
			particleSystem = "Par-Supress_Gather",
			emitterPoint = "right_hand",
			particleScale = 0.5
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("NormalizeTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Supress_Lines",
			emitterPoint = "right_hand",
			particleScale = 0.5
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.NormalizeTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NormalizeTarget";
	mSound = "Sound-Ability-Supress_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Supress_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Supress_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Supress_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.BlazingBurstWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "BlazingBurstWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.BlazingBurst extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlazingBurst";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spellgasm"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.80000001, "onExplodingRing");
	}

	function onExplodingRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("Ring").finish();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Inferno.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("BlazingBurstHit", true);
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.BlazingBurstHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlazingBurstHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.BlazingBurstSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlazingBurstSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.FreezingFlareWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FreezingFlareWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Hand",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Particles",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Lines",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FreezingFlare extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FreezingFlare";
	mSound = "Sound-Ability-Frostmire_Cast.ogg";
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
			particleSystem = "Par-FrostMire_Hand",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Lines",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.5, "onInitialBurst");
		this.fireIn(0.60000002, "onInitialBurst2");
		this.fireIn(0.69999999, "onBurst");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		blast.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast2",
			emitterPoint = "node"
		});
		this.onSound();
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
		local blast2 = this.createGroup("Blast2");
		blast2.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Blast1",
			emitterPoint = "node"
		});
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		this.get("Blast2").finish();
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
			textureName = "FrostMire.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("FreezingFlareHit", true);
		this.fireIn(0.60000002, "onDone");
	}

}

class this.EffectDef.FreezingFlareHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FreezingFlareHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.DazingTwirlChannel extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "DazingTwirlChannel";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("FFAnimation", {
			animation = "Spinning_Daggers",
			loop = true
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-DazingTwirl_Particles",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Hand",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "FrostMire.png",
			orthoWidth = 150,
			orthoHeight = 150,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.DazingTwirlHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DazingTwirlHit";
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
			particleSystem = "Par-Coldsnap_Explosion1",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.OpenWounds extends this.EffectDef.TemplateBasic
{
	static mEffectName = "OpenWounds";
	mSound = "Sound-Ability-EnfeeblingBlow_Cast.ogg";
	mHitName = "OpenWoundsHit";
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
			initialColor = "ee0000",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Dual_Wield_Middle_Cross"
		});
		a.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onHitFinal");
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

class this.EffectDef.OpenWoundsHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "OpenWoundsHit";
	mSound = "Sound-Ability-Hatred_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("FFAnimation", {
			animation = "$HIT$"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Mist",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Assail_Burst",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-OpenWounds_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.KingsBattlecryChannel extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "KingsBattlecryChannel";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("FFAnimation", {
			animation = "Exclaim",
			loop = true
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "casting",
			particleScale = 0.2
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather",
			emitterPoint = "casting",
			particleScale = 0.5
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "Challenge.png",
			orthoWidth = 150,
			orthoHeight = 150,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.KingsBattlecryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "KingsBattlecryHit";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 4.0;
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

class this.EffectDef.RootedRot extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RootedRot";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Gather",
			emitterPoint = "horde_attacker"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Lines",
			emitterPoint = "horde_attacker"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Gather",
			emitterPoint = "horde_attacker2"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Root_Lines",
			emitterPoint = "horde_attacker2"
		});
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 150,
			orthoHeight = 150,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("RootedRotHit", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Root_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.RootedRotHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RootedRotHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol2",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Root_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.RootedRotSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RootedRotSecondaryHit";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.RotSpread extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RotSpread";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "horde_attacker"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "horde_attacker2"
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-RotSpread_Rings",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("RotSpreadTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.RotSpreadTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RotSpreadTarget";
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
			particleSystem = "Par-Portal_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Fill",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.AcidicBite extends this.EffectDef.TemplateMelee
{
	static mEffectName = "AcidicBite";
	mWeapon = "";
	mSound = "Sound-Ability-Assault_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local a = this.createGroup("A", this.getSource());
		a.add("FFAnimation", {
			animation = "Attack2"
		});
		a.add("ParticleSystem", {
			particleSystem = "Par-AcidicBite_Drip",
			emitterPoint = "horde_attacker"
		});
		this.fireIn(0.5, "onHit");
		this.fireIn(0.40000001, "onFinalHit");
	}

	function onHit( ... )
	{
		this.getTarget().cue("AcidicBiteHit", this.getTarget());
		this._cueImpactSound(this.getTarget());
	}

	function onFinalHit( ... )
	{
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.AcidicBiteHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AcidicBiteHit";
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
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.AcidicBiteSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AcidicBiteSecondaryHit";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.NaturesIntuition extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NaturesIntuition";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mTargetName = "NaturesIntuitionHit";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Ambient"
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
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.NaturesIntuitionHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NaturesIntuitionHit";
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

class this.EffectDef.UnnaturalResilience extends this.EffectDef.TemplateBasic
{
	static mEffectName = "UnnaturalResilience";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Ambient"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Gather",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("UnnaturalResilienceTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Burst",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.UnnaturalResilienceTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "UnnaturalResilienceTarget";
	mSound = "Sound-Ability-DeathProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Symbol1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeathProtection_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.StingingWebshot extends this.EffectDef.TemplateBasic
{
	static mEffectName = "StingingWebshot";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Claw"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Burst",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Hand",
			emitterPoint = "casting"
		});
		this.fireIn(0.2, "onNet");
		this.fireIn(0.30000001, "onBurst");
		this.fireIn(0.40000001, "onHit");
		this.fireIn(0.89999998, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onNet( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Net",
			emitterPoint = "node"
		});
	}

	function onHit( ... )
	{
		this._cueSecondary("StingingWebshotHit");
	}

}

class this.EffectDef.StingingWebshotHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "StingingWebshotHit";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 4.0;
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
			particleSystem = "Par-ConstrictingWeb_Impact",
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

class this.EffectDef.SilkSoarWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SilkSoarWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Gather",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LawmansWrath_Gather2",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("Spin", {
			axis = "y",
			speed = 0.2
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SilkSoar extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SilkSoar";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 5.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Claw"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Burst",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Hand",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.2, "onNet");
		this.fireIn(0.30000001, "onBurst");
		this.fireIn(0.40000001, "onHit");
		this.fireIn(0.89999998, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onNet( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Web1.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-LawmansGrip_Net",
			emitterPoint = "node"
		});
	}

	function onHit( ... )
	{
		this._cueSecondary("SilkSoarHit");
	}

}

class this.EffectDef.SilkSoarHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SilkSoarHit";
	mSound = "Sound-Ability-EnfeeblingBlow_Effect.ogg";
	mDuration = 4.0;
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
			particleSystem = "Par-Portal_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Fill",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Portal_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.SealedCocoon extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SealedCocoon";
	mSound = "Sound-Ability-EnfeeblingBlow_Cast.ogg";
	mHitName = "SealedCocoonHit";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Attack1"
		});
		this.fireIn(0.80000001, "onCast");
		this.fireIn(1.0, "onHitFinal");
	}

	function onCast( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("ParticleSystem", {
			particleSystem = "Par-SealedCocoon_Cast",
			emitterPoint = "horde_attacker"
		});
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

class this.EffectDef.SealedCocoonHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SealedCocoonHit";
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
			particleSystem = "Par-ConstrictingWeb_Impact",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.2, "onHitDone");
		this.fireIn(0.30000001, "onResidual");
		this.fireIn(10.0, "onDone");
	}

	function onResidual( ... )
	{
		this.get("Hit").finish();
		local cocoon = this.createGroup("Cocoon", this.getSource());
		cocoon.add("Mesh", {
			mesh = "Item-Cocoon.mesh",
			point = "node",
			fadeInTime = 1.0,
			fadeOutTime = 0.2
		});
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.SealedCocoonSelfHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SealedCocoonSelfHit";
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
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.RapidDecay extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RapidDecay";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.60000002, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("RapidDecayHit");
	}

}

class this.EffectDef.RapidDecayHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RapidDecayHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.RapidDecaySecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RapidDecaySecondaryHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-NefritarisAura_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.RotRoot extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RotRoot";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.60000002, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "Earthshaker.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("RotRootHit");
	}

}

class this.EffectDef.RotRootHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RotRootHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
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
			particleSystem = "Par-Supress_Sparkle",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Supress_Fill",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Supress_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.ChargedCrushWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ChargedCrushWarmup";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Gather",
			emitterPoint = "horde_attacker",
			particleScale = 0.40000001
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Gather",
			emitterPoint = "horde_attacker2",
			particleScale = 0.40000001
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.ChargedCrush extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ChargedCrush";
	mSound = "Sound-Ability-Default_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack1"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Gather",
			emitterPoint = "horde_attacker",
			particleScale = 0.40000001
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Gather",
			emitterPoint = "horde_attacker2",
			particleScale = 0.40000001
		});
		this.onSound();
		this.fireIn(0.30000001, "onSpinStart");
		this.fireIn(0.60000002, "onFinishCast");
	}

	function onSpinStart( ... )
	{
		local spin = this.createGroup("Spin");
		spin.add("ParticleSystem", {
			particleSystem = "Par-AreaSwirl2",
			emitterPoint = "node"
		});
	}

	function onFinishCast( ... )
	{
		this.getTarget().cue("ChargedCrushTarget", this.getTarget());
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.ChargedCrushTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ChargedCrushTarget";
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
			particleSystem = "Par-SweepingBlow_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-SweepingBlow_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.DeathSeed extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathSeed";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Attack2"
		});
		this.onSound();
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Burst",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "horde_attacker2",
			particleScale = 0.34999999
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.60000002, "onBurst");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
	}

	function onHitFinal( ... )
	{
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "NefritarisAura.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Ring",
			emitterPoint = "node"
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("DeathSeedHit");
	}

}

class this.EffectDef.DeathSeedHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathSeedHit";
	mSound = "Sound-Ability-Nefritarisaura_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Hands",
			emitterPoint = "casting"
		});
		this.onSound();
		local skull = this.createGroup("Skull");
		skull.add("Mesh", {
			mesh = "Item-Skull2.mesh",
			point = "node",
			fadeInTime = 0.5,
			fadeOutTime = 1.0
		});
		skull.add("Spin", {
			axis = "y",
			speed = 0.40000001,
			extraStopTime = 1.0
		});
		this.fireIn(180.0, "onDone");
	}

}

class this.EffectDef.AnubCatapultFire extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AnubCatapultFire";
	mSound = "Sound-Ability-Hellfire_Cast.ogg";
	mCastSoundDone = false;
	mContactDone = false;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local load = this.createGroup("load", this.getSource());
		load.add("Mesh", {
			mesh = "Hellfire_Sphere.mesh",
			fadeInTime = 0,
			point = "basket",
			bone = "basket"
		});
		load.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Ball",
			emitterPoint = "basket"
		});
		load.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Fire",
			emitterPoint = "basket"
		});
		load.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Smoke",
			emitterPoint = "basket"
		});
		local particles = this.createGroup("Particles", this.getSource());
		this.fireIn(0.80000001, "onAnim");
		this.fireIn(0.98000002, "onProjectile");
		this.getTarget().cork();
	}

	function onAnim( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local anim = this.createGroup("anim", this.getSource());
		anim.add("FFAnimation", {
			animation = "Attack1"
		});
		this.onSound();
	}

	function onProjectile( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork;
				this.onDone();
			}
		}

		this.get("load").finish();
		this.get("anim").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
		local proj = this.createGroup("Projectile");
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "basket",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 10.0,
			accel = 2.0,
			arcSideAngle = 0.0,
			arcForwardAngle = 45.0,
			arcEnd = 0.30000001
		});
		proj.add("Mesh", {
			mesh = "Hellfire_Sphere.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Fire",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Smoke",
			emitterPoint = "node"
		});
		proj.add("ScaleTo", {
			size = 0.69999999,
			maintain = true,
			duration = 0.0
		});
		proj.add("Sound", {
			sound = "Firebolt_Projectile.ogg"
		});
	}

	function onContact( ... )
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

		this.getTarget().cue("AnubCatapultHit", this.getTarget());
		this.getTarget().uncork();
		::_playTool.addShaky(this.getTarget().getPosition(), 170, 1.2, 400.0);
		this.get("Projectile").finish();
		this.mContactDone = true;

		if (this.mCastSoundDone)
		{
			this.onDone();
		}
	}

}

class this.EffectDef.AnubCatapultHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AnubCatapultHit";
	mSound = "flame_explosion.ogg";
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
		this.onSound();
		this.fireIn(0.0, "onImpact");
		this.fireIn(0.0099999998, "onRing");
		this.fireIn(5.5, "onDone");
	}

	function onImpact( ... )
	{
		local impact = this.createGroup("Impact", this.getSource());
		impact.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_Shine",
			emitterPoint = "spell_target"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_Rays",
			emitterPoint = "spell_target"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_Flare",
			emitterPoint = "spell_target"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Streaks",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Shine",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Rays",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Nova",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Smoke",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Burst_Ring",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Ground",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Burst",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Pieces",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ring_Flare",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Large",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare_Large",
			emitterPoint = "node"
		});
		impact.add("Projector", {
			duration = 1500,
			fadeIn = 100,
			fadeOut = 500,
			target = impact.getObject(),
			textureName = "HotSpot.png",
			orthoWidth = 180,
			orthoHeight = 180,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(0.1, "onImpactDone");
	}

	function onImpactDone( ... )
	{
		this.get("Impact").finish();
	}

	function onRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_FireRing",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 0.30000001,
			startSize = 0,
			duration = 0.30000001
		});
		ring.add("Spin", {
			speed = 1.0,
			axis = this.Vector3(0.0, 1.0, 0.0)
		});
		this.fireIn(0.25, "onRingEnd");
	}

	function onRingEnd( ... )
	{
		this.get("Ring").finish();
	}

}

class this.EffectDef.AnubCatapultDeath extends this.EffectDef.TemplateBasic
{
	static mEffectName = "AnubCatapultDeath";
	function onStart( ... )
	{
		this.fireIn(0.1, "onBurst");

		if (!this._sourceCheck())
		{
			return;
		}

		local death = this.createGroup("Death", this.getSource());
		death.add("Sound", {
			sound = "Sound-Ability-Pyroblast_Effect.ogg"
		});
		death.add("FFAnimation", {
			animation = "Death",
			speed = -2.5,
			maintain = true
		});
		death.add("ParticleSystem", {
			particleSystem = "Par-Anub_Catapult_Burn",
			emitterPoint = "node"
		});
		this.addShaky(170, 1.2, 300.0);
		this.fireIn(6.0, "onDone");
	}

	function onBurst( ... )
	{
		local burst = this.createGroup("Burst");
		burst.add("ParticleSystem", {
			particleSystem = "Par-Anub_Catapult_Smoke",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Anub_Catapult_Burst",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Streaks",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Shine",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Rays",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-AnubCatapult_Impact_Smoke",
			emitterPoint = "node"
		});
		burst.add("Projector", {
			duration = 1500,
			fadeIn = 100,
			fadeOut = 500,
			target = burst.getObject(),
			textureName = "HotSpot.png",
			orthoWidth = 180,
			orthoHeight = 180,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.ArrowTargetFire extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ArrowTargetFire";
	mSound = "Arrow";
	mTotalArrowSounds = 4;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.getTarget().cork();
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Bow_Attack_Release",
			speed = 1.8
		});
		this.add("Dummy");
		local sound = this.createGroup("ImpactSound", this.getSource());
		local randImpact = this.randomSound();
		sound.add("Sound", {
			sound = randImpact
		});
		this.fireIn(0.30000001, "onAddArrow");
		this.fireIn(1.0, "onProjectile");
	}

	function randomSound( ... )
	{
		local arrowSounds = [];

		for( local i = 1; i <= this.mTotalArrowSounds; i++ )
		{
			arrowSounds.append("Sound-Ability-Bow_Autoattack" + i + "_Cast.ogg");
		}

		return this.Util.randomElement(arrowSounds);
	}

	function onAddArrow( ... )
	{
		local cast = this.get("Cast");
		cast.add("Mesh", {
			mesh = "Item-Arrow.mesh",
			point = "right_hand_Arrow",
			fadeInTime = 0.5,
			fadeOutTime = 0.0099999998
		});
	}

	function onProjectile( ... )
	{
		this.get("Cast").finish();
		local targetPoint = this.createGroup("targetPoint");
		local targetPos = this.getTarget().getPosition();
		local newPos = targetPos + this.Vector3(0.0, 15.0, 0.0);
		this._detachGroup(targetPoint, newPos);
		local proj = this.createGroup("Projectile");
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = targetPoint.getObject(),
			targetPoint = "node",
			intVelocity = 6.0,
			accel = 12.0,
			topSpeed = 100.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Arrow.mesh",
			fadeInTime = 0.0099999998,
			fadeOutTime = 2.0
		});
		proj.add("Ribbon", {
			offset = -0.25,
			width = 0.5,
			height = 5.0,
			maxSegments = 32,
			initialColor = "e2ff80",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
	}

	function onContact( ... )
	{
		this.getTarget().uncork();
		local proj = this.get("Projectile");
		this.fireIn(0.1, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

