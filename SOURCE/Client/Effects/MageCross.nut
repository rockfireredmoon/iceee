this.require("EffectDef");
class this.EffectDef.Blazing extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Blazing";
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
			particleSystem = "Par-Blazing_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("BlazingTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Burst",
			emitterPoint = "right_hand"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Gather",
			emitterPoint = "casting"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.BlazingTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlazingTarget";
	mSound = "Sound-Ability-Blazing_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Hand",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Particles",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Blazing_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Frozen extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Frozen";
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
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("FrozenTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.FrozenTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrozenTarget";
	mSound = "Sound-Ability-Frozen_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Particles",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Hand",
			emitterPoint = "casting"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Symbol",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Frozen_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.FrostMireWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FrostMireWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Dust",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Rays",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Lens",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_BR_Ground",
			emitterPoint = "node"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FrostMire extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostMire";
	mSound = "Sound-Ability-Frostmire_Cast.ogg";
	function onStart( ... )
	{
		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Started");
		}

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": target check passed");
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Cast group setup passed");
		}

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Dust",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Rays",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Lens",
			emitterPoint = "casting"
		});

		if (this.gLogEffects)
		{
			this.log.debug(this.mEffectName + ": Particle group setup passed");
		}

		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.34999999, "onNova");
		this.fireIn(0.40000001, "onInitialBurst");
		this.fireIn(1.0, "onBurst");
		this.fireIn(0.34999999, "onRing");
	}

	function onInitialBurst( ... )
	{
		local blast = this.createGroup("Blast");
		this._detachGroup(blast, this.getSource().getNode().getPosition());
		blast.add("ParticleSystem", {
			particleSystem = "Par-Frost_Vortex",
			emitterPoint = "node"
		});
		this.onSound();
	}

	function onNova( ... )
	{
		local nova = this.createGroup("Nova");
		this._detachGroup(nova, this.getSource().getNode().getPosition());
		nova.add("ParticleSystem", {
			particleSystem = "Par-Frost_Nova_Shine",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Frost_Nova_Rays",
			emitterPoint = "node"
		});
		nova.add("ParticleSystem", {
			particleSystem = "Par-Frost_Nova_Inverse",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onNovaEnd");
		this.fireIn(0.60000002, "onInitialBurst2");
	}

	function onNovaEnd( ... )
	{
		this.get("Nova").finish();
	}

	function onRing( ... )
	{
		local ring = this.createGroup("Ring");
		this._detachGroup(ring, this.getSource().getNode().getPosition());
		ring.add("ParticleSystem", {
			particleSystem = "Par-Frostmire_Burst",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 0.5,
			startSize = 0.80000001,
			duration = 0.2
		});
		this.fireIn(0.2, "onRingEnd");
	}

	function onRingEnd( ... )
	{
		this.get("Ring").finish();
	}

	function onInitialBurst2( ... )
	{
		this.get("Blast").finish();
		local blast2 = this.createGroup("Blast2");
		this._detachGroup(blast2, this.getSource().getNode().getPosition());
		blast2.add("ParticleSystem", {
			particleSystem = "Par-Frost_Flare_Ground",
			emitterPoint = "node"
		});
		blast2.add("ParticleSystem", {
			particleSystem = "Par-Frost_Nova_Rays",
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
			duration = 1000,
			fadeIn = 100,
			fadeOut = 400,
			target = aoe.getObject(),
			textureName = "FrostMire.png",
			orthoWidth = 120,
			orthoHeight = 120,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		this._cueTargets("FrostMireHit", true);
		this.fireIn(0.80000001, "onDone");
	}

}

class this.EffectDef.FrostMireHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostMireHit";
	function onStart( ... )
	{
		local buff = this.createGroup("Buff", this.getSource());
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

class this.EffectDef.ColdsnapWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ColdsnapWarmup";
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
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Lines",
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

class this.EffectDef.Coldsnap extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Coldsnap";
	mSound = "Sound-Ability-Coldsnap_Cast.ogg";
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
			animation = "Magic_Double_Handed"
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
		this.add("Dummy");
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onContact");
		this.fireIn(0.80000001, "onDone");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
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

		this.getTarget().cue("ColdsnapHit", this.getTarget());
		this.getTarget().uncork();
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("ColdsnapSecondaryHit");
	}

}

class this.EffectDef.ColdsnapHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ColdsnapHit";
	mSound = "Sound-Ability-Coldsnap_Effect.ogg";
	mHit = true;
	mDuration = 5.0;
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
			particleSystem = "Par-Coldsnap_Explosion1",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.onSound();
		this.fireIn(0.5, "onHitStop");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Fill",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitStop( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.ColdsnapSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ColdsnapSecondaryHit";
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
			particleSystem = "Par-Coldsnap_Explosion2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Coldsnap_Burst2",
			emitterPoint = "spell_target"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.FlameSpearWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FlameSpearWarmup";
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
		warmup.add("ParticleSystem", {
			particleSystem = "Par-FlameSpear_Lines",
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

class this.EffectDef.FlameSpear extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FlameSpear";
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

		this.getTarget().cue("FlameSpearHit", this.getTarget());
		this.getTarget().uncork();
		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.FlameSpearHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FlameSpearHit";
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

class this.EffectDef.FrostSpearWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "FrostSpearWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	mMaxWarmupTime = 3.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		this.mEffectsPackage = this._getEffectsPackage(this.getSource());
		local warmup = this.createGroup("Warmup", this.getSource());

		if (this.mEffectsPackage == "Biped")
		{
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Dust",
				emitterPoint = "casting"
			});
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Rays",
				emitterPoint = "casting"
			});
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Lens",
				emitterPoint = "casting"
			});
			warmup.add("FFAnimation", {
				animation = "Magic_Casting",
				loop = true
			});
		}
		else
		{
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Dust",
				emitterPoint = "casting"
			});
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Rays",
				emitterPoint = "casting"
			});
			warmup.add("ParticleSystem", {
				particleSystem = "Par-Frost_Hands_Lens",
				emitterPoint = "casting"
			});
			warmup.add("FFAnimation", {
				animation = "Magic_Casting",
				loop = true
			});
		}

		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.FrostSpear extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostSpear";
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

		this.getTarget().cork();
	}

	function onBiped( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Sneak_Throw1"
		});
		this.onParticles();
		this.fireIn(0.30000001, "onProjectile");
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

		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Hand",
			emitterPoint = "casting"
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

		this.get("Cast").finish();
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

		this.getTarget().cue("FrostSpearHit", this.getTarget());
		this.getTarget().uncork();
		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.FrostSpearHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostSpearHit";
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
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.JarnsaxasKissWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "JarnsaxasKissWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
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
			animation = "Magic_Casting",
			loop = true
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Hand",
			emitterPoint = "casting"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Snow",
			emitterPoint = "casting"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Lines",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

	function onCreature( ... )
	{
		local cast = this.createGroup("Cast", this.getSource());

		if (this.mEffectsPackage == "Horde-Shroomie")
		{
			cast.add("FFAnimation", {
				animation = "Attack"
			});
		}
		else
		{
			cast.add("FFAnimation", {
				animation = "Attack1"
			});
		}

		cast.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Hand",
			emitterPoint = "casting"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Snow",
			emitterPoint = "casting"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.JarnsaxasKiss1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "JarnsaxasKiss1";
	mSound = "Sound-Ability-Jarnsaxaskiss_Cast.ogg";
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
			animation = "Magic_Casting"
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
			particleSystem = "Par-JarnsaxasKiss_Cast",
			emitterPoint = "casting"
		});
		this._corkTargets();
		this.onSound();
		this.fireIn(0.5, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());

		if (this.mEffectsPackage == "Biped")
		{
			sparks.add("ParticleSystem", {
				particleSystem = "Par-JarnsaxasKiss_Explosion2",
				emitterPoint = "left_hand"
			});
		}
		else
		{
			sparks.add("ParticleSystem", {
				particleSystem = "Par-JarnsaxasKiss_Explosion3",
				emitterPoint = "horde_caster"
			});
		}

		sparks.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Cast",
			emitterPoint = "casting"
		});
		this._cueTargets("JarnsaxasKissHit", true);
		this.fireIn(0.1, "onUncork");
		this.fireIn(0.80000001, "onDone");
	}

	function onUncork( ... )
	{
		this._uncorkTargets();
	}

}

class this.EffectDef.JarnsaxasKissHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "JarnsaxasKissHit";
	mSound = "Sound-Ability-Forceblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Explosion1",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-JarnsaxasKiss_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.JarnsaxasKiss2 extends this.EffectDef.JarnsaxasKiss1
{
	static mEffectName = "JarnsaxasKiss2";
}

class this.EffectDef.JarnsaxasKiss3 extends this.EffectDef.JarnsaxasKiss1
{
	static mEffectName = "JarnsaxasKiss3";
}

class this.EffectDef.JarnsaxasKiss4 extends this.EffectDef.JarnsaxasKiss1
{
	static mEffectName = "JarnsaxasKiss4";
}

class this.EffectDef.JarnsaxasKiss5 extends this.EffectDef.JarnsaxasKiss1
{
	static mEffectName = "JarnsaxasKiss5";
}

class this.EffectDef.IncinerateWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "IncinerateWarmup";
	mLoopSound = "Sound-Ability-Firebolt_Projectile.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Lines",
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

class this.EffectDef.Incinerate1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Incinerate1";
	mSound = "Sound-Ability-Firebolt_Cast.ogg";
	mCharges = 1.0;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		if (!this._secondaryTargetCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onContact");

		for( local i = 1; i <= this.mCharges; i++ )
		{
			this.fireIn(i, "onSecondaryHit");
		}

		this.fireIn(0.5 + this.mCharges, "onDone");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
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

		this.getTarget().cue("IncinerateHit", this.getTarget());
		this.getTarget().uncork();
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("IncinerateSecondaryHit");
	}

}

class this.EffectDef.Incinerate2 extends this.EffectDef.Incinerate1
{
	static mEffectName = "Incinerate2";
	mCharges = 2.0;
}

class this.EffectDef.Incinerate3 extends this.EffectDef.Incinerate1
{
	static mEffectName = "Incinerate3";
	mCharges = 3.0;
}

class this.EffectDef.Incinerate4 extends this.EffectDef.Incinerate1
{
	static mEffectName = "Incinerate4";
	mCharges = 4.0;
}

class this.EffectDef.Incinerate5 extends this.EffectDef.Incinerate1
{
	static mEffectName = "Incinerate5";
	mCharges = 5.0;
}

class this.EffectDef.IncinerateHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "IncinerateHit";
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
			particleSystem = "Par-Incinerate_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.IncinerateSecondaryHit extends this.EffectDef.IncinerateHit
{
	static mEffectName = "IncinerateSecondaryHit";
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
			particleSystem = "Par-Incinerate_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Incinerate_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.DeepFreezeWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "DeepFreezeWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Dust",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Rays",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Lens",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Frost_BR_Ground",
			emitterPoint = "node"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.DeepFreeze1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeepFreeze1_1";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mTargetEffect = "DeepFreezeTarget1_1";
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
			particleSystem = "Par-DeepFreeze_Cast",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue(this.mTargetEffect, this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Bright_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.DeepFreeze1_2 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze1_2";
	mTargetEffect = "DeepFreezeTarget1_2";
}

class this.EffectDef.DeepFreeze1_3 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze1_3";
	mTargetEffect = "DeepFreezeTarget1_3";
}

class this.EffectDef.DeepFreeze1_4 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze1_4";
	mTargetEffect = "DeepFreezeTarget1_4";
}

class this.EffectDef.DeepFreeze1_5 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze1_5";
	mTargetEffect = "DeepFreezeTarget1_5";
}

class this.EffectDef.DeepFreeze2_1 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze2_1";
	mTargetEffect = "DeepFreezeTarget2_1";
}

class this.EffectDef.DeepFreeze2_2 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze2_2";
	mTargetEffect = "DeepFreezeTarget2_2";
}

class this.EffectDef.DeepFreeze2_3 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze2_3";
	mTargetEffect = "DeepFreezeTarget2_3";
}

class this.EffectDef.DeepFreeze2_4 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze2_4";
	mTargetEffect = "DeepFreezeTarget2_4";
}

class this.EffectDef.DeepFreeze2_5 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze2_5";
	mTargetEffect = "DeepFreezeTarget2_5";
}

class this.EffectDef.DeepFreeze3_1 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze3_1";
	mTargetEffect = "DeepFreezeTarget3_1";
}

class this.EffectDef.DeepFreeze3_2 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze3_2";
	mTargetEffect = "DeepFreezeTarget3_2";
}

class this.EffectDef.DeepFreeze3_3 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze3_3";
	mTargetEffect = "DeepFreezeTarget3_3";
}

class this.EffectDef.DeepFreeze3_4 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze3_4";
	mTargetEffect = "DeepFreezeTarget3_4";
}

class this.EffectDef.DeepFreeze3_5 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze3_5";
	mTargetEffect = "DeepFreezeTarget3_5";
}

class this.EffectDef.DeepFreeze4_1 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze4_1";
	mTargetEffect = "DeepFreezeTarget4_1";
}

class this.EffectDef.DeepFreeze4_2 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze4_2";
	mTargetEffect = "DeepFreezeTarget4_2";
}

class this.EffectDef.DeepFreeze4_3 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze4_3";
	mTargetEffect = "DeepFreezeTarget4_3";
}

class this.EffectDef.DeepFreeze4_4 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze4_4";
	mTargetEffect = "DeepFreezeTarget4_4";
}

class this.EffectDef.DeepFreeze4_5 extends this.EffectDef.DeepFreeze1_1
{
	static mEffectName = "DeepFreeze4_5";
	mTargetEffect = "DeepFreezeTarget4_5";
}

class this.EffectDef.DeepFreezeTarget1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeepFreezeTarget1_1";
	mSound = "Sound-Ability-Deepfreeze_Effect.ogg";
	mDuration = 0.5;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Burst",
			emitterPoint = "spell_target",
			particleScale = 1.2
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Fill",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.1, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Daze",
			emitterPoint = "spell_target_head"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Ice",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.DeepFreezeTarget1_2 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget1_2";
	mDuration = 1.0;
}

class this.EffectDef.DeepFreezeTarget1_3 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget1_3";
	mDuration = 1.5;
}

class this.EffectDef.DeepFreezeTarget1_4 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget1_4";
	mDuration = 2.0;
}

class this.EffectDef.DeepFreezeTarget1_5 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget1_5";
	mDuration = 2.5;
}

class this.EffectDef.DeepFreezeTarget2_1 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget2_1";
	mDuration = 1.5;
}

class this.EffectDef.DeepFreezeTarget2_2 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget2_2";
	mDuration = 2.0;
}

class this.EffectDef.DeepFreezeTarget2_3 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget2_3";
	mDuration = 2.5;
}

class this.EffectDef.DeepFreezeTarget2_4 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget2_4";
	mDuration = 3.0;
}

class this.EffectDef.DeepFreezeTarget2_5 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget2_5";
	mDuration = 3.5;
}

class this.EffectDef.DeepFreezeTarget3_1 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget3_1";
	mDuration = 2.5;
}

class this.EffectDef.DeepFreezeTarget3_2 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget3_2";
	mDuration = 3.0;
}

class this.EffectDef.DeepFreezeTarget3_3 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget3_3";
	mDuration = 3.5;
}

class this.EffectDef.DeepFreezeTarget3_4 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget3_4";
	mDuration = 4.0;
}

class this.EffectDef.DeepFreezeTarget3_5 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget3_5";
	mDuration = 4.5;
}

class this.EffectDef.DeepFreezeTarget4_1 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget4_1";
	mDuration = 3.5;
}

class this.EffectDef.DeepFreezeTarget4_2 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget4_2";
	mDuration = 4.0;
}

class this.EffectDef.DeepFreezeTarget4_3 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget4_3";
	mDuration = 4.5;
}

class this.EffectDef.DeepFreezeTarget4_4 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget4_4";
	mDuration = 5.0;
}

class this.EffectDef.DeepFreezeTarget4_5 extends this.EffectDef.DeepFreezeTarget1_1
{
	static mEffectName = "DeepFreezeTarget4_5";
	mDuration = 5.5;
}

class this.EffectDef.HandOfHeraclitus1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HandOfHeraclitus1";
	mSound = "Sound-Ability-Pyroblast_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed",
			speed = 1.5
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-HandOfHeraclitus_Cast",
			emitterPoint = "casting"
		});
		this.onSound();
		local spinner = this.createGroup("Spinner");
		spinner.add("ParticleSystem", {
			particleSystem = "Par-HandOfHeraclitus_Swirl",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		spinner.add("Spin", {
			axis = "y",
			speed = 1.5,
			extraStopTime = 1.0
		});
		this.fireIn(0.2, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-HandOfHeraclitus_Explosion2",
			emitterPoint = "casting"
		});
		this.getTarget().cue("HandOfHeraclitusHit", this.getTarget());
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.HandOfHeraclitus2 extends this.EffectDef.HandOfHeraclitus1
{
	static mEffectName = "HandOfHeraclitus2";
}

class this.EffectDef.HandOfHeraclitus3 extends this.EffectDef.HandOfHeraclitus1
{
	static mEffectName = "HandOfHeraclitus3";
}

class this.EffectDef.HandOfHeraclitus4 extends this.EffectDef.HandOfHeraclitus1
{
	static mEffectName = "HandOfHeraclitus4";
}

class this.EffectDef.HandOfHeraclitus5 extends this.EffectDef.HandOfHeraclitus1
{
	static mEffectName = "HandOfHeraclitus5";
}

class this.EffectDef.HandOfHeraclitusHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HandOfHeraclitusHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-HandOfHeraclitus_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-HandOfHeraclitus_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.WintersCarress1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WintersCarress1";
	mSound = "Sound-Ability-Winterscaress_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed",
			speed = 1.5
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-WintersCarress_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.2, "onStopCastFire");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-WintersCarress_Touch",
			emitterPoint = "casting"
		});
		this.onSound();
		this.getTarget().cue("WintersCarressHit", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.WintersCarress2 extends this.EffectDef.WintersCarress1
{
	static mEffectName = "WintersCarress2";
}

class this.EffectDef.WintersCarress3 extends this.EffectDef.WintersCarress1
{
	static mEffectName = "WintersCarress3";
}

class this.EffectDef.WintersCarress4 extends this.EffectDef.WintersCarress1
{
	static mEffectName = "WintersCarress4";
}

class this.EffectDef.WintersCarress5 extends this.EffectDef.WintersCarress1
{
	static mEffectName = "WintersCarress5";
}

class this.EffectDef.WintersCarressHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WintersCarressHit";
	mSound = "Sound-Ability-Coldsnap_Effect.ogg";
	mHit = true;
	mDuration = 5.0;
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
			hit.add("ParticleSystem", {
				particleSystem = "Par-WintersCarress_Explosion",
				emitterPoint = "spell_target"
			});
			hit.add("ParticleSystem", {
				particleSystem = "Par-WintersCarress_Burst",
				emitterPoint = "spell_target"
			});
			hit.add("Sound", {
				sound = "Firebolt_Impact.ogg"
			});
			this.onSound();
			this.fireIn(0.5, "onHitStop");
		}

		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeepFreeze_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitStop( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.WintersCarressSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WintersCarressSecondaryHit";
	mHit = false;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Burst2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FrostMire_Hand2",
			emitterPoint = "casting"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.fireIn(8.0, "onDone");
	}

}

class this.EffectDef.Hellfire1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Hellfire1";
	mSound = "Sound-Ability-Hellfire_Cast.ogg";
	mCastSoundDone = false;
	mContactDone = false;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed",
			speed = 1.3
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Hand",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Particles",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Sparkle",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.34999999, "onStopCastFire");
		this.fireIn(0.30000001, "onProjectile");
		this.fireIn(0.30000001, "onSpiral");
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
			intVelocity = 4.0,
			accel = 12.0,
			topSpeed = 100.0,
			orient = true
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
			particleSystem = "Par-Hellfire_Ball2",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Smoke",
			emitterPoint = "node"
		});
		proj.add("Spin", {
			speed = 3.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
		proj.add("ScaleTo", {
			size = 0.30000001,
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

		this.getTarget().cue("HellfireHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.mContactDone = true;

		if (this.mCastSoundDone)
		{
			this.onDone();
		}
	}

}

class this.EffectDef.Hellfire2 extends this.EffectDef.Hellfire1
{
	static mEffectName = "Hellfire2";
}

class this.EffectDef.Hellfire3 extends this.EffectDef.Hellfire1
{
	static mEffectName = "Hellfire3";
}

class this.EffectDef.Hellfire4 extends this.EffectDef.Hellfire1
{
	static mEffectName = "Hellfire4";
}

class this.EffectDef.Hellfire5 extends this.EffectDef.Hellfire1
{
	static mEffectName = "Hellfire5";
}

class this.EffectDef.HellfireHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HellfireHit";
	mSound = "Sound-Ability-Hellfire_Effect.ogg";
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
			particleSystem = "Par-Hellfire_Impact_Fire",
			emitterPoint = "node"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_Nova",
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
			particleSystem = "Par-Fire_BR_Ground_Large",
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
			orthoWidth = 120,
			orthoHeight = 120,
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
			particleSystem = "Par-Hellfire_Impact_Ring",
			emitterPoint = "node"
		});
		ring.add("ParticleSystem", {
			particleSystem = "Par-Hellfire_Impact_FireRing",
			emitterPoint = "node"
		});
		ring.add("ScaleTo", {
			size = 0.2,
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

class this.EffectDef.ShatterWill1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShatterWill1_1";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mTargetEffect = "ShatterWillTarget1_1";
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
			particleSystem = "Par-ShatterWill_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue(this.mTargetEffect, this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.ShatterWill1_2 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill1_2";
	mTargetEffect = "ShatterWillTarget1_2";
}

class this.EffectDef.ShatterWill1_3 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill1_3";
	mTargetEffect = "ShatterWillTarget1_3";
}

class this.EffectDef.ShatterWill1_4 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill1_4";
	mTargetEffect = "ShatterWillTarget1_4";
}

class this.EffectDef.ShatterWill1_5 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill1_5";
	mTargetEffect = "ShatterWillTarget1_5";
}

class this.EffectDef.ShatterWill2_1 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill2_1";
	mTargetEffect = "ShatterWillTarget2_1";
}

class this.EffectDef.ShatterWill2_2 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill2_2";
	mTargetEffect = "ShatterWillTarget2_2";
}

class this.EffectDef.ShatterWill2_3 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill2_3";
	mTargetEffect = "ShatterWillTarget2_3";
}

class this.EffectDef.ShatterWill2_4 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill2_4";
	mTargetEffect = "ShatterWillTarget2_4";
}

class this.EffectDef.ShatterWill2_5 extends this.EffectDef.ShatterWill1_1
{
	static mEffectName = "ShatterWill2_5";
	mTargetEffect = "ShatterWillTarget2_5";
}

class this.EffectDef.ShatterWillTarget1_1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShatterWillTarget1_1";
	mSound = "Sound-Ability-Shatterwill_Effect.ogg";
	mDuration = 0.40000001;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Impact",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Burst3",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Burst2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onBuffDone");
		function onImpact( ... )
		{
			local shock = this.createGroup("Shockwave");
			shock.add("ParticleSystem", {
				particleSystem = "Par-Earthshaker_Ring"
			});
		}

		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-ShatterWill_Fill2",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.ShatterWillTarget1_2 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget1_2";
	mDuration = 0.80000001;
}

class this.EffectDef.ShatterWillTarget1_3 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget1_3";
	mDuration = 1.2;
}

class this.EffectDef.ShatterWillTarget1_4 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget1_4";
	mDuration = 1.6;
}

class this.EffectDef.ShatterWillTarget1_5 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget1_5";
	mDuration = 2.0;
}

class this.EffectDef.ShatterWillTarget2_1 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget2_1";
	mDuration = 0.60000002;
}

class this.EffectDef.ShatterWillTarget2_2 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget2_2";
	mDuration = 1.2;
}

class this.EffectDef.ShatterWillTarget2_3 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget2_3";
	mDuration = 1.8;
}

class this.EffectDef.ShatterWillTarget2_4 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget2_4";
	mDuration = 2.4000001;
}

class this.EffectDef.ShatterWillTarget2_5 extends this.EffectDef.ShatterWillTarget1_1
{
	static mEffectName = "ShatterWillTarget2_5";
	mDuration = 3.0;
}

class this.EffectDef.IgniteWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "IgniteWarmup";
	mLoopSound = "Sound-Ability-Firespells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Lines",
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

class this.EffectDef.Ignite1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Ignite1";
	mSound = "Sound-Ability-Firebolt_Cast.ogg";
	mCharges = 1.0;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Cast",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onContact");

		for( local i = 1; i <= this.mCharges; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.getTarget().cork();
		this.fireIn(0.5 + this.mCharges, "onDone");
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
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

		this.getTarget().cue("IgniteHit", this.getTarget());
		this.getTarget().uncork();
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("IgniteSecondaryHit");
	}

}

class this.EffectDef.Ignite2 extends this.EffectDef.Ignite1
{
	static mEffectName = "Ignite2";
	mCharges = 2.0;
}

class this.EffectDef.Ignite3 extends this.EffectDef.Ignite1
{
	static mEffectName = "Ignite3";
	mCharges = 3.0;
}

class this.EffectDef.Ignite4 extends this.EffectDef.Ignite1
{
	static mEffectName = "Ignite4";
	mCharges = 4.0;
}

class this.EffectDef.Ignite5 extends this.EffectDef.Ignite1
{
	static mEffectName = "Ignite5";
	mCharges = 5.0;
}

class this.EffectDef.IgniteHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "IgniteHit";
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
			particleSystem = "Par-Ignite_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Ignite_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.IgniteSecondaryHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "IgniteSecondaryHit";
	mSound = "Sound-Ability-Pyroblast_Effect.ogg";
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
			particleSystem = "Par-Ignite_Burst",
			emitterPoint = "spell_target"
		});
		hit.add("Sound", {
			sound = "Firebolt_Impact.ogg"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.Cryoblast1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Cryoblast1";
	mSound = "Sound-Ability-Cyroblast_Cast.ogg";
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
		this.fireIn(0.60000002, "onStopCastFire");
		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Cryoblast_Cast",
			emitterPoint = "casting"
		});
		this.onSound();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Cryoblast_Cast2",
			emitterPoint = "casting"
		});
		this.getTarget().cue("CryoblastHit", this.getTarget());
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.Cryoblast2 extends this.EffectDef.Cryoblast1
{
	static mEffectName = "Cryoblast2";
}

class this.EffectDef.Cryoblast3 extends this.EffectDef.Cryoblast1
{
	static mEffectName = "Cryoblast3";
}

class this.EffectDef.Cryoblast4 extends this.EffectDef.Cryoblast1
{
	static mEffectName = "Cryoblast4";
}

class this.EffectDef.Cryoblast5 extends this.EffectDef.Cryoblast1
{
	static mEffectName = "Cryoblast5";
}

class this.EffectDef.CryoblastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CryoblastHit";
	mSound = "Sound-Ability-Cyroblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Cryoblast_Blast",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Cryoblast_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
		this.fireIn(0.0099999998, "onBurst");
	}

	function onBurst( ... )
	{
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Frost_Nova",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		this.get("Burst").finish();
	}

}

class this.EffectDef.Frostbolt extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Frostbolt";
	mSound = "Sound-Ability-Frostbolt_Cast.ogg";
	mLoopSound = "Sound-Ability-Frostbolt_Cast.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed",
			speed = 1.5
		});
		this.onSound();
		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Dust",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Rays",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Frost_Hands_Lens",
			emitterPoint = "casting"
		});
		this.fireIn(0.25, "onStopCastFire");
		this.fireIn(0.25, "onProjectile");
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
			particleSystem = "Par-Frostbolt_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-FrostSpear_Snow",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Frost_Projectile_Dust",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = 0.5,
			width = 3.0,
			height = 10.0,
			maxSegments = 64,
			initialColor = "488cd1",
			colorChange = [
				4.0,
				4.0,
				4.0,
				4.0
			]
		});
		proj.add("Ribbon", {
			offset = -0.5,
			width = -3.0,
			height = -10.0,
			maxSegments = 64,
			initialColor = "488cd1",
			colorChange = [
				4.0,
				4.0,
				4.0,
				4.0
			]
		});
		proj.add("Spin", {
			speed = 2.0,
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

		this.getTarget().cue("FrostboltHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.FrostboltHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FrostboltHit";
	mSound = "Sound-Ability-Forceblast_Effect.ogg";
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
			particleSystem = "Par-Frost_Impact_Star",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Frost_Flare",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Frost_Impact_Flare",
			emitterPoint = "spell_target"
		});
		local hit2 = this.createGroup("Hit2", this.getSource());
		hit2.add("ParticleSystem", {
			particleSystem = "Par-Frost_Impact_Star",
			emitterPoint = "spell_target"
		});
		hit2.add("ParticleSystem", {
			particleSystem = "Par-Frost_Shine",
			emitterPoint = "spell_target"
		});
		hit2.add("ParticleSystem", {
			particleSystem = "Par-Frost_Impact_Dust",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		this.fireIn(0.40000001, "onHit2Done");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Frostbolt_Fill",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Frostbolt_Fill2",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

	function onHit2Done( ... )
	{
		this.get("Hit2").finish();
		local hit3 = this.createGroup("Hit3", this.getSource());
		hit3.add("ParticleSystem", {
			particleSystem = "Par-Frost_BR",
			emitterPoint = "spell_target"
		});
		hit3.add("ParticleSystem", {
			particleSystem = "Par-Frost_Impact_Flare_Small",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onHit3Done");
	}

	function onHit3Done( ... )
	{
		this.get("Hit3").finish();
	}

}

