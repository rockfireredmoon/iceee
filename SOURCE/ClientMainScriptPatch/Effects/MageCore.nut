this.require("EffectDef");
class this.EffectDef.ArcaneAccelerationWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ArcaneAccelerationWarmup";
	mLoopSound = "Sound-Ability-MysticProtection_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Bright_Gather2",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Haste_Cast_Sparks",
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

class this.EffectDef.ArcaneAcceleration extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ArcaneAcceleration";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Humble"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Haste_Cast_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Haste_Cast_Sparks",
			emitterPoint = "casting"
		});
		this.getSource().cue("ArcaneAccelerationTarget", this.getSource());
		this.fireIn(1.0, "onBurst");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Haste_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ArcaneAccelerationTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ArcaneAccelerationTarget";
	mSound = "Sound-Ability-Arcane_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ArcaneAcceleration_Spin",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ArcaneAcceleration_Fill",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.2, "onDone");
	}

}

class this.EffectDef.Barrier extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Barrier";
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
			particleSystem = "Par-Ice_Hand4",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Barrier_Cast_Spray",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.0, "onTriggerTarget");
		this.fireIn(0.5, "onFinishCast");
	}

	function onTriggerTarget( ... )
	{
		this.getTarget().cue("BarrierTarget", this.getTarget());
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Barrier_Cast_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.BarrierTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BarrierTarget";
	mSound = "Sound-Ability-WarriorsSpirit_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff");
		buff.add("Mesh", {
			mesh = "BarrierShield.mesh",
			point = "node",
			fadeInTime = 1.0,
			fadeOutTime = 0.5
		});
		buff.add("Spin", {
			speed = 0.75,
			accel = 0.0,
			axis = this.Vector3(0.0, 1.0, 0.0)
		});
		this.onSound();
		this.fireIn(0.75, "onFill");
		this.fireIn(2.0, "onDone");
	}

	function onFill( ... )
	{
		local fill = this.createGroup("Fill", this.getSource());
		fill.add("ParticleSystem", {
			particleSystem = "Par-Barrier_Target_Fill",
			emitterPoint = "node"
		});
		fill.add("ParticleSystem", {
			particleSystem = "Par-Barrier_Target_Spray",
			emitterPoint = "node"
		});
	}

}

class this.EffectDef.Bright extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Bright";
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
			particleSystem = "Par-Bright_Gather2",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Bright_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("BrightTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Bright_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.BrightTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BrightTarget";
	mSound = "Sound-Ability-Bright_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bright_Beams3",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bright_Twinkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bright_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bright_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.CataclysmWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "CataclysmWarmup";
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

class this.EffectDef.Cataclysm extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Cataclysm";
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
		this.getTarget().cue("CataclysmMainHit", this.getTarget());
		this._cueSecondary("CataclysmHit");
		this.fireIn(0.5, "onFinish");
	}

	function onFinish()
	{
		this.getTarget().uncork();
		this._uncorkSecondary();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.CataclysmMainHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CataclysmMainHit";
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
			particleSystem = "Par-Fire_Combustion_Ring"
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
			particleSystem = "Par-Cataclysm-Fire_Circle"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.CataclysmHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CataclysmHit";
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

class this.EffectDef.Pyroblast1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Pyroblast1";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
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
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Throw",
			speed = 1.6
		});
		this.fireIn(0.30000001, "onStopCastFire");
		this.onParticles();
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Vulture" || this.mEffectsPackage == "Horde-Behemoth" || this.mEffectsPackage == "Horde-Wolf" || this.mEffectsPackage == "Horde-Vampire")
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Lion" || this.mEffectsPackage == "Horde-Snail" || this.mEffectsPackage == "Horde-Chicken" || this.mEffectsPackage == "Horde-Revenant" || this.mEffectsPackage == "Horde-Salamander" || this.mEffectsPackage == "Horde-Spibear" || this.mEffectsPackage == "Horde-Vespin")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Spider")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.69999999, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Stag")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Boar" || this.mEffectsPackage == "Horde-Biter")
		{
			cast.add("FFAnimation", {
				animation = "RearHit"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Rabbit" || this.mEffectsPackage == "Horde-Treekin")
		{
			this.fireIn(0.5, "onStopCastFire");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}

		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lens_Flare_Ring",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Specs",
			emitterPoint = "casting"
		});
		this.onSound();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion4",
			emitterPoint = "casting"
		});
		this.getTarget().cue("PyroblastHit", this.getTarget());
		this.fireIn(0.2, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Small",
			emitterPoint = "casting"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.Pyroblast2 extends this.EffectDef.Pyroblast1
{
	static mEffectName = "Pyroblast2";
}

class this.EffectDef.Pyroblast3 extends this.EffectDef.Pyroblast1
{
	static mEffectName = "Pyroblast3";
}

class this.EffectDef.Pyroblast4 extends this.EffectDef.Pyroblast1
{
	static mEffectName = "Pyroblast4";
}

class this.EffectDef.Pyroblast5 extends this.EffectDef.Pyroblast1
{
	static mEffectName = "Pyroblast5";
}

class this.EffectDef.PyroblastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PyroblastHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		this._detachGroup(hit, this.getSource().getNode().getPosition());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.34999999, "onDone");
		local burst = this.createGroup("Burst");
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Burst_Ring",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Ground_Large",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Ground",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Burst",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Pieces",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ring_Flare",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Large",
			emitterPoint = "node"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare_Large",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onBurstDone");
		local buff = this.createGroup("Buff");
		buff.add("Spin", {
			speed = 1.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 1.0, 0.0)
		});
		buff.add("ScaleTo", {
			size = 1.0,
			startSize = 0,
			duration = 0.69999999
		});
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 500,
			fadeIn = 150,
			fadeOut = 300,
			target = aoe.getObject(),
			textureName = "HotSpot.png",
			orthoWidth = 80,
			orthoHeight = 80,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.Firebolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Firebolt";
	mSound = "Sound-Ability-Firebolt_Cast.ogg";
	mLoopSound = "Sound-Ability-Firebolt_Projectile.ogg";
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
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed",
			speed = 1.4
		});
		this.onSound();
		this.onParticles();
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Vulture" || this.mEffectsPackage == "Horde-Behemoth" || this.mEffectsPackage == "Horde-Wolf" || this.mEffectsPackage == "Horde-Vampire")
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Lion" || this.mEffectsPackage == "Horde-Snail" || this.mEffectsPackage == "Horde-Chicken" || this.mEffectsPackage == "Horde-Revenant" || this.mEffectsPackage == "Horde-Salamander" || this.mEffectsPackage == "Horde-Spibear" || this.mEffectsPackage == "Horde-Vespin")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Spider")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.69999999, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Stag")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Boar" || this.mEffectsPackage == "Horde-Biter")
		{
			cast.add("FFAnimation", {
				animation = "RearHit"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Rabbit" || this.mEffectsPackage == "Horde-Treekin")
		{
			this.fireIn(0.5, "onStopCastFire");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}

		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lens_Flare_Ring",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Specs",
			emitterPoint = "casting"
		});
		this.fireIn(0.2, "onStopCastFire");
		this.fireIn(0.25, "onProjectile");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare_Small",
			emitterPoint = "right_hand"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Small",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Sparks").finish();
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
		this._detachGroup(proj, this.getSource().getNode().getPosition());

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
			mesh = "Item-Sphere.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Twin",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Embers",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Specs_Trail",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 4.0,
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
			speed = 4.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
		proj.add("ScaleTo", {
			size = 0.44999999,
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

		this.getTarget().cue("FireboltHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.FireboltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FireboltHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
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
		this.fireIn(0.34999999, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR",
			emitterPoint = "spell_target"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare",
			emitterPoint = "spell_target"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Ground",
			emitterPoint = "spell_target"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Burst",
			emitterPoint = "spell_target"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Fire_Pieces",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.ForceBlast1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ForceBlast1";
	mSound = "Sound-Ability-Forceblast_Cast.ogg";
	mLoopSound = "Sound-Ability-Forcebolt_Projectile.ogg";
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

		this.fireIn(10.0, "onAbilityCancel");
		this.getTarget().cork();
	}

	function onBiped( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Wand_Point"
		});
		this.fireIn(0.40000001, "onProjectile");
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Circle2",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Particles",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Column3",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Fill",
			emitterPoint = "right_hand"
		});
		this.onSound();
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
			this.fireIn(0.40000001, "onProjectile");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Circle3",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Particles2",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Column5",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Fill2",
			emitterPoint = "horde_caster"
		});
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

		this.get("Particles").finish();

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
			particleSystem = "Par-Wand_Aura3",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Force_Column4",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.80000001,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "00b4ff",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		this.onLoopSound(proj);
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

		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.getTarget().cue("ForceBlastHit", this.getTarget());
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ForceBlast2 extends this.EffectDef.ForceBlast1
{
	static mEffectName = "ForceBlast2";
}

class this.EffectDef.ForceBlast3 extends this.EffectDef.ForceBlast1
{
	static mEffectName = "ForceBlast3";
}

class this.EffectDef.ForceBlast4 extends this.EffectDef.ForceBlast1
{
	static mEffectName = "ForceBlast4";
}

class this.EffectDef.ForceBlast5 extends this.EffectDef.ForceBlast1
{
	static mEffectName = "ForceBlast5";
}

class this.EffectDef.ForceBlastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ForceBlast";
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
			particleSystem = "Par-Force_Ring",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Force_Ring2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Force_Burst2",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.Forcebolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Forcebolt";
	mLoopSound = "Sound-Ability-Forcebolt_Projectile.ogg";
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
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Wand_Point"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Circle",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Particles",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Column",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Fill",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onProjectile");
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Vulture" || this.mEffectsPackage == "Horde-Behemoth" || this.mEffectsPackage == "Horde-Wolf" || this.mEffectsPackage == "Horde-Vampire")
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.60000002, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Lion" || this.mEffectsPackage == "Horde-Snail" || this.mEffectsPackage == "Horde-Chicken" || this.mEffectsPackage == "Horde-Revenant" || this.mEffectsPackage == "Horde-Salamander" || this.mEffectsPackage == "Horde-Spibear" || this.mEffectsPackage == "Horde-Vespin")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Spider")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.69999999, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Stag")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Boar" || this.mEffectsPackage == "Horde-Biter")
		{
			cast.add("FFAnimation", {
				animation = "RearHit"
			});
			this.fireIn(0.5, "onProjectile");
		}
		else if (this.mEffectsPackage == "Horde-Rabbit" || this.mEffectsPackage == "Horde-Treekin")
		{
			this.fireIn(0.5, "onProjectile");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onProjectile");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Circle3",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Force_Particles2",
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

		local target = this.getTarget();
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
			particleSystem = "Par-Wand_Aura2",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Force_Column2",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.80000001,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "00b4ff",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
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
				this.getTarget().uncork();
				this.onDone();
			}
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.getTarget().cue("ForceboltHit", this.getTarget());
		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.ForceboltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ForceboltHit";
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
			particleSystem = "Par-Force_Ring",
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

class this.EffectDef.FrostShield1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostShield1";
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
		local sound = this.createGroup("Sound", this.getSource());
		sound.add("Sound", {
			sound = "Sound-Ability-Frostshield_Cast.ogg"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand4",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-IceShield-Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.5, "onBurst");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-IceShield-Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.25, "onShower");
		this.fireIn(0.5, "onShield");
	}

	function onShower( ... )
	{
		local shower = this.createGroup("Shower", this.getSource());
		shower.add("ParticleSystem", {
			particleSystem = "Par-IceShield-Twinkle",
			emitterPoint = "node"
		});
		this.fireIn(3, "onShowerStop");
	}

	function onShield( ... )
	{
		this.get("Burst").finish();
		local shield = this.createGroup("Shield", this.getSource());
		shield.add("ParticleSystem", {
			particleSystem = "Par-IceShield-Circle",
			emitterPoint = "node"
		});
		shield.add("ParticleSystem", {
			particleSystem = "Par-IceShield-Mist",
			emitterPoint = "node"
		});
		local sound = this.createGroup("Sound2", this.getSource());
		sound.add("Sound", {
			sound = "Frostshieldeffect.ogg"
		});
		this.fireIn(5.0, "onDone");
	}

	function onShowerStop( ... )
	{
		this.get("Shower").finish();
	}

	function onAbilityDone( ... )
	{
		this.onDone();
	}

}

class this.EffectDef.FrostShield2 extends this.EffectDef.FrostShield1
{
	static mEffectName = "FrostShield2";
}

class this.EffectDef.FrostShield3 extends this.EffectDef.FrostShield1
{
	static mEffectName = "FrostShield3";
}

class this.EffectDef.FrostShield4 extends this.EffectDef.FrostShield1
{
	static mEffectName = "FrostShield4";
}

class this.EffectDef.FrostShield5 extends this.EffectDef.FrostShield1
{
	static mEffectName = "FrostShield5";
}

class this.EffectDef.FrostStormWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FrostStormWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand4",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Warmup_Snow",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Warmup_Lines",
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

class this.EffectDef.FrostStorm extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostStorm";
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
			particleSystem = "Par-Ice_Hand4",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand_Mist",
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
						botTarget.cue("FrostStormHit", botTarget);
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

class this.EffectDef.FrostStormHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostStormHit";
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

class this.EffectDef.InfernoWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "InfernoWarmup";
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
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Lines",
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

class this.EffectDef.Inferno1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Inferno1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory",
			speed = 1.5
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Lens_Flare_Ring",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Specs",
			emitterPoint = "casting"
		});
		this._corkTargets(true);
		this.fireIn(0.2, "onExplodingRing");
		this.fireIn(0.2, "onNova");
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
			size = 1.0,
			startSize = 0,
			duration = 0.69999999
		});
		this.fireIn(0.40000001, "onStopCastFire");
	}

	function onNova( ... )
	{
		local nova = this.createGroup("Nova", this.getSource());
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Nova",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Nova_Rays",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Nova_Rays_Small",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ring_Flare",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion2",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Large",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_BR_Small",
			emitterPoint = "right_hand"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Fire_Flare_Small",
			emitterPoint = "right_hand"
		});
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getPosition());
		aoe.add("Projector", {
			duration = 600,
			fadeIn = 200,
			fadeOut = 400,
			target = aoe.getObject(),
			textureName = "HotSpot.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("InfernoHit", true);
		this.fireIn(0.1, "onNovaEnd");
	}

	function onNovaEnd( ... )
	{
		this.get("Nova").finish();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		this.get("Ring").finish();
		this.fireIn(0.5, "onUncork");
		this.fireIn(1.0, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets(true);
	}

}

class this.EffectDef.Inferno2 extends this.EffectDef.Inferno1
{
	static mEffectName = "Inferno2";
}

class this.EffectDef.Inferno3 extends this.EffectDef.Inferno1
{
	static mEffectName = "Inferno3";
}

class this.EffectDef.Inferno4 extends this.EffectDef.Inferno1
{
	static mEffectName = "Inferno4";
}

class this.EffectDef.Inferno5 extends this.EffectDef.Inferno1
{
	static mEffectName = "Inferno5";
}

class this.EffectDef.InfernoHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "InfernoHit";
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
			particleSystem = "Par-Fire_Swirl",
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

class this.EffectDef.Invisibility extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Invisibility";
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
		this._cueTargets("InvisibilityTarget");
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.InvisibilityTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "InvisibilityTarget";
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

class this.EffectDef.TheftOfWill1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TheftOfWill";
	mSound = "Sound-Ability-Default_Cast.ogg";
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
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Cast_Spray",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Cast_Glow",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.80000001, "onBurst");
		this.fireIn(0.80000001, "onProjectile");
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
		proj.add("ArcToTarget", {
			source = target,
			sourcePoint = "spell_target",
			target = this.getSource(),
			targetPoint = "right_hand",
			intVelocity = 3.0,
			accel = 4.0,
			topSpeed = 100.0,
			orient = true,
			arcEnd = 0.30000001,
			arcSideAngle = 0.0,
			arcForwardAngle = 80.0
		});
		proj.add("Mesh", {
			mesh = "Item-Invisible.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Stream",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 1.0,
			height = 1.0,
			maxSegments = 32,
			initialColor = "8EE5EE",
			colorChange = [
				1.0,
				1.0,
				1.0,
				1.0
			]
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
	}

	function onContact( ... )
	{
		this.get("Projectile").finish();
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("TheftOfWillTarget", this.getTarget());
		this.fireIn(0.30000001, "onBurstEnd");
	}

	function onBurstEnd( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.TheftOfWill2 extends this.EffectDef.TheftOfWill1
{
	static mEffectName = "TheftOfWill2";
}

class this.EffectDef.TheftOfWill3 extends this.EffectDef.TheftOfWill1
{
	static mEffectName = "TheftOfWill3";
}

class this.EffectDef.TheftOfWill4 extends this.EffectDef.TheftOfWill1
{
	static mEffectName = "TheftOfWill4";
}

class this.EffectDef.TheftOfWill5 extends this.EffectDef.TheftOfWill1
{
	static mEffectName = "TheftOfWill5";
}

class this.EffectDef.TheftOfWillTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TheftOfWillTarget";
	mSound = "Sound-Ability-Theftofwill_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Target_Spary",
			emitterPoint = "spell_target_head"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Target_Glow",
			emitterPoint = "spell_target_head"
		});
		this.onSound();
		this.fireIn(1.3, "onLoss");
		this.fireIn(1.4, "onLossDone");
		this.fireIn(1.5, "onDone");
	}

	function onLoss( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local loss = this.createGroup("Loss", this.getSource());
		loss.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Loss",
			emitterPoint = "node"
		});
	}

	function onLossDone( ... )
	{
		this.get("Loss").finish();
	}

}

class this.EffectDef.WildFireWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "WildFireWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand4",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Warmup_Snow",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Warmup_Lines",
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

class this.EffectDef.WildFire extends this.EffectDef.TemplateMultiProjectile
{
	static mEffectName = "WildFire";
	mCharges = 1.0;
	mProjs = null;
	mProjVersion = 1;
	mRingCount = 0;
	function onStart( ... )
	{
		if (!this._positionalCheck())
		{
			return;
		}

		this.mProjs = {};
		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onBurst");
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(1.0, "onHit");
		this.fireIn(1.0, "onGroundStop");

		if (this.mCharges > 1.0)
		{
			for( local i = 0.0; i < this.mCharges; i++ )
			{
				this.fireIn(i * 2 + 1.0, "onSecondaryHit");
			}
		}

		this._corkTargets();
		this.fireIn(this.mCharges * 2 + 1.5, "onDone");
	}

	function onBurst( ... )
	{
		local ground = this.createGroup("Ground");
		this._detachGroup(ground, this.getPositionalTarget());
		local ground2 = this.createGroup("Ground2");
		this._detachGroup(ground2, this.getPositionalTarget());
		ground2.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		ground2.add("ScaleTo", {
			size = 2.0,
			maintain = true,
			duration = 0.0
		});
		this.onStartRing();
	}

	function onStartRing( ... )
	{
		local ring = this.createGroup("Ring" + this.mRingCount++);
		this._detachGroup(ring, this.getPositionalTarget());
		ring.add("ScaleTo", {
			size = 1,
			startSize = 0,
			duration = 0.40000001
		});
		ring.add("ParticleSystem", {
			particleSystem = "Par-Fire_Inferno_Burst",
			emitterPoint = "node"
		});
		this.fireIn(0.60000002, "onStopRing", ring);
	}

	function onStopRing( ring )
	{
		ring.finish();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
	}

	function onGroundStop( ... )
	{
		this.get("Ground").finish();
	}

	function onHit( ... )
	{
		this._cueTargets("WildFireHit");
		this.fireIn(0.1, "onUncork");
	}

	function onUncork( ... )
	{
		this._uncorkTargets();
	}

	function _createProjectile( group, tID, target )
	{
		local ground = this.get("Ground2");

		if (!(tID in this.mProjs))
		{
			this.mProjs[tID] <- [];
		}

		local proj = this.createGroup(group);
		proj.add("ArcToTarget", {
			source = ground.getObject(),
			sourcePoint = "node",
			target = target,
			targetPoint = "spell_target",
			intVelocity = 10.0,
			accel = 4.0,
			topSpeed = 30.0,
			orient = true,
			arcEnd = 0.5,
			arcSideAngle = 0.0,
			arcForwardAngle = 0.5
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
			size = 1.0,
			maintain = true,
			duration = 0.0
		});
		this.mProjs[tID].append(group);
	}

	function onProjectile( ... )
	{
		local sec = this.getSecondaryTargets();

		if (!sec)
		{
			return;
		}

		if (sec.len() == 0)
		{
			return;
		}

		for( local i = 0; i < sec.len(); i++ )
		{
			local group = i + ":" + this.mCount;
			this._createProjectile(group, i, sec[i]);
			this.mCount++;
		}

		this.mProjVersion++;
	}

	function onContact( ... )
	{
		local distance = 10000.0;
		local targetID;
		local projID;
		local found = false;
		local sec = this.getSecondaryTargets();

		if (sec == null)
		{
			return;
		}

		for( local i = 0; i < sec.len(); i++ )
		{
			if (i in this.mProjs)
			{
				if (this.mProjs[i].len() > 0)
				{
					for( local x = 0; x < this.mProjs[i].len(); x++ )
					{
						local newDist = this._distanceCheck(sec[i], this.mProjs[i][x]);

						if (distance > newDist)
						{
							distance = newDist;
							targetID = i;
							projID = x;
							found = true;
							sec[i].cue("WildFireSecondaryHit");

							if (this.gLogEffects)
							{
								this.log.debug("MARTIN: projectile id: " + x);
							}
						}
					}
				}
			}
		}

		if (found)
		{
			this.get(this.mProjs[targetID][projID]).finish();
			this.mProjs[targetID].remove(projID);

			foreach( i, x in this.mProjs[targetID] )
			{
				if (this.gLogEffects)
				{
					this.log.debug("MARTIN: ID: " + i + ", Value: " + x);
				}
			}
		}

		if (targetID)
		{
			if (this.gLogEffects)
			{
				this.log.debug("MARTIN: Number of projectiles remaining: " + this.mProjs[targetID].len());
			}
		}
	}

	function onClearProjectiles( ... )
	{
		local sec = this.getSecondaryTargets();

		if (!sec)
		{
			return;
		}

		for( local i = 0; i < sec.len(); i++ )
		{
			if (i in this.mProjs)
			{
				if (this.mProjs[i].len() > 0)
				{
					for( local x = this.mProjs[i].len() - 1; x >= 0; x-- )
					{
						this.get(this.mProjs[i][x]).finish();
						this.mProjs[i].remove(x);
					}
				}
			}
		}
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("WildFireSecondaryHit");
		this.onStartRing();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.WildFire1 extends this.EffectDef.WildFire
{
}

class this.EffectDef.WildFire2 extends this.EffectDef.WildFire1
{
	static mEffectName = "WildFire2";
	mCharges = 2.0;
}

class this.EffectDef.WildFire3 extends this.EffectDef.WildFire1
{
	static mEffectName = "WildFire3";
	mCharges = 3.0;
}

class this.EffectDef.WildFire4 extends this.EffectDef.WildFire1
{
	static mEffectName = "WildFire4";
	mCharges = 4.0;
}

class this.EffectDef.WildFire5 extends this.EffectDef.WildFire1
{
	static mEffectName = "WildFire5";
	mCharges = 5.0;
}

class this.EffectDef.WildFireHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WildFireHit";
	mSound = "Sound-Ability-Firebolt_Impact.ogg";
	mHit = true;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());

		if (this.mHit)
		{
			hit.add("FFAnimation", {
				animation = "$HIT$"
			});
		}

		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion3",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.WildFireSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WildFireSecondaryHit";
	mHit = false;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());

		if (this.mHit)
		{
			hit.add("FFAnimation", {
				animation = "$HIT$"
			});
		}

		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion3",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.BringingDownTheHouseWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "BringingDownTheHouseWarmup";
	mLoopSound = "Sound-Ability-Energy2_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Elemental_Earth_Core",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-House_Down_Warmup_Rocks",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-House_Down_Warmup_Smoke",
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

class this.EffectDef.BringingDownTheHouse extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BringingDownTheHouse";
	mSound = "Sound-Ability-HouseDown_Effect.ogg";
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
			particleSystem = "Par-Rock_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Elemental_Earth_Core",
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
			particleSystem = "Par-WalkInShadow_Burst",
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
			textureName = "bigcracks.png",
			orthoWidth = 500,
			orthoHeight = 500,
			offset = this.Vector3(0, 200, 0),
			far = 500,
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
		this.addShaky(65, 0.5, 300.0);

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
			intVelocity = 2.0,
			accel = 6.0,
			topSpeed = 30.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Orbit_Rock1.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-House_Down_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-House_Down_Bits",
			emitterPoint = "node"
		});
		/*proj.add("Ribbon", {
			offset = -3.0,
			width = 6.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "7788aa",
			colorChange = [
				2.0,
				2.0,
				2.0,
				2.0
			]
		});*/
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
						botTarget.cue("BringingDownTheHouseHit", botTarget);
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

class this.EffectDef.BringingDownTheHouseHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BringingDownTheHouseHit";
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
			particleSystem = "Par-PelletHit",
			emitterPoint = "node"
		});
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.BloodBath extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "BloodBath";
	mLoopSound = "Sound-Ability-Nefritarisaura_Warmup.ogg";
	mDuration = 10.0;
	mCount = 0.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-BloodBath_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-BloodBath_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Spell_Channel",
			loop = true
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		this._detachGroup(aoe, this.getSource().getNode().getWorldPosition());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "BloodBath.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-BloodBath_Ring",
			emitterPoint = "node"
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
		this.onHit();
	}

	function onHit( ... )
	{
		if (this.mDuration <= this.mCount)
		{
			this.onAbilityCancel();
			return;
		}

		this._cueSecondary("BloodBathHit");
		this.mCount++;
		this.fireIn(1.0, "onHit");
	}

	function onAbilityCancel( ... )
	{
		this.mCount = this.mDuration;
		::EffectDef.TemplateWarmup.onAbilityCancel();
	}

}

class this.EffectDef.BloodBathHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BloodBathHit";
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
			particleSystem = "Par-BloodBath_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-BloodBath_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.ThousandBatsWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ThousandBats";
	mLoopSound = "Sound-Ability-Swarm_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-ThousandBats_Hand",
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

class this.EffectDef.ThousandBats extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThousandBats";
	mDuration = 8.0;
	mSound = "Sound-Ability-ThousandBats_Effect.ogg";
	mScreamSound = "Sound-Ability-Healingscream_Effect.ogg";
	
	function onStart( ... )
	{
		if (!this._positionalCheck())
		{
			return;
		}

		this.add("Dummy");
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
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		this.onParticles();
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Vulture" || this.mEffectsPackage == "Horde-Behemoth" || this.mEffectsPackage == "Horde-Wolf" || this.mEffectsPackage == "Horde-Vampire")
		{
			cast.add("FFAnimation", {
				animation = "Attack2"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Lion" || this.mEffectsPackage == "Horde-Snail" || this.mEffectsPackage == "Horde-Chicken" || this.mEffectsPackage == "Horde-Revenant" || this.mEffectsPackage == "Horde-Salamander" || this.mEffectsPackage == "Horde-Spibear" || this.mEffectsPackage == "Horde-Vespin")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Spider")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.69999999, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Stag")
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Boar" || this.mEffectsPackage == "Horde-Biter")
		{
			cast.add("FFAnimation", {
				animation = "RearHit"
			});
			this.fireIn(0.5, "onStopCastFire");
		}
		else if (this.mEffectsPackage == "Horde-Rabbit" || this.mEffectsPackage == "Horde-Treekin")
		{
			this.fireIn(0.5, "onStopCastFire");
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
			this.fireIn(0.60000002, "onStopCastFire");
		}

		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-ThousandBats_Hand",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-ThousandBats_Hand",
			emitterPoint = "horde_caster"
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
		this._detachGroup(aoe, this.getPositionalTarget());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = aoe.getObject(),
			textureName = "ThousandBats.png",
			orthoWidth = 300,
			orthoHeight = 300,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-ThousandBats_Ring",
			emitterPoint = "node"
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-BloodBath_Ring",
			emitterPoint = "node"
		});
		local sound = this.createGroup("Sound");
		this._detachGroup(sound, this.getPositionalTarget());
		sound.add("Sound", {
			sound = this.mSound
		});
		sound.add("Scream", {
			sound = this.mScreamSound
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("ThousandBatsHit");
	}

}

class this.EffectDef.ThousandBatsHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ThousandBatsHit";
	mSound = "Sound-Ability-Soulburst_Effect.ogg";
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
			particleSystem = "Par-ThousandBats_Hit",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}
