this.require("EffectDef");
class this.EffectDef.SpiritOfSolomon extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpiritOfSolomon";
	mSound = "Sound-Ability-Mystical_Effect.ogg";
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
			particleSystem = "Par-SpiritOfSolomon_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-SpiritOfSolomon_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("SpiritOfSolomonTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-SpiritOfSolomon_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.SpiritOfSolomonTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SpiritOfSolomonTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-SpiritOfSolomon_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-SpiritOfSolomon_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-SpiritOfSolomon_Symbol",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Mystical extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Mystical";
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
			particleSystem = "Par-Mystical_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Mystical_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("MysticalTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Mystical_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.MysticalTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticalTarget";
	mSound = "Sound-Ability-Mystical_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Mystical_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Mystical_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Mystical_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Deathly extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Deathly";
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
		this.getTarget().cue("DeathlyTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Deathly_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.DeathlyTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathlyTarget";
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

class this.EffectDef.NefritarisAura extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "NefritarisAura1";
	mLoopSound = "Sound-Ability-Nefritarisaura_Warmup.ogg";
	mDuration = 5.0;
	mCount = 0.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-NefritarisAura_Lines",
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

		this._cueSecondary("NefritarisAuraHit");
		this.mCount++;
		this.fireIn(1.0, "onHit");
	}

	function onAbilityCancel( ... )
	{
		this.mCount = this.mDuration;
		::EffectDef.TemplateWarmup.onAbilityCancel();
	}

}

class this.EffectDef.NefritarisAuraHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "NefritarisAuraHit";
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

class this.EffectDef.MorassWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "MorassWarmup";
	mLoopSound = "Sound-Ability-Morass_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Morass_Lines",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Morass_Cast",
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

class this.EffectDef.Morass extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Morass";
	mSound = "Sound-Ability-Morass_Effect.ogg";
	function onStart( ... )
	{
		if (!this._positionalCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Morass_Lines",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Morass_Cast",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.5, "onHitFinal");
		this.fireIn(0.69999999, "onBurst");
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
			textureName = "Morass.png",
			orthoWidth = 200,
			orthoHeight = 200,
			far = 200,
			offset = this.Vector3(0, 100, 0),
			additive = true
		});
		aoe.add("ParticleSystem", {
			particleSystem = "Par-Morass_Ring",
			emitterPoint = "node"
		});
		this._cueTargets("MorassHit", true);
		this.fireIn(2.0, "onDone");
	}

}

class this.EffectDef.MorassHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MorassHit";
	mDuration = 30.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Morass_Fill2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Morass_Hit1",
			emitterPoint = "casting"
		});
		this.fireIn(0.1, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Morass_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Morass_Hit1",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.DamnationWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "DamnationWarmup";
	mLoopSound = "Sound-Ability-Morass_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Morass_Lines",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Morass_Cast",
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

class this.EffectDef.Damnation extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Damnation";
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
			particleSystem = "Par-Damnation_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Damnation_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("DamnationTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Damnation_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.DamnationTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DamnationTarget";
	mSound = "Sound-Ability-Damnation_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Damnation_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Damnation_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Damnation_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

	function onInitialDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.SwarmWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SwarmWarmup";
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
			emitterPoint = "casting"
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

class this.EffectDef.Swarm extends this.EffectDef.TemplateBasic
{
	mDuration = 8.0;
	mSound = "Sound-Ability-Swarm_Effect.ogg";
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
			particleSystem = "Par-Swarm_Hand",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Particles",
			emitterPoint = "right_hand"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Hand",
			emitterPoint = "horde_caster"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Swarm_Particles",
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
			textureName = "Swarm.png",
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
		local sound = this.createGroup("Sound");
		this._detachGroup(sound, this.getPositionalTarget());
		sound.add("Sound", {
			sound = this.mSound
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("SwarnHit");
	}

}

class this.EffectDef.SwarnHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SwarnHit";
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
			particleSystem = "Par-Swarm_Hit",
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

class this.EffectDef.SyphonWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SyphonWarmup";
	mLoopSound = "Sound-Ability-Syphon_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Syphon_Cast",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Syphon_Lines",
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

class this.EffectDef.Syphon extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Syphon";
	mSound = "Sound-Ability-Syphon_Effect.ogg";
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
			animation = "Magic_Spell_Channel",
			loop = true
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Syphon_Cast",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Syphon_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(this.mDuration, "onFinishCast");

		if (!this._secondaryTargetCheck())
		{
			return;
		}

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}
	}

	function onAbilityWarmupComplete( ... )
	{
	}

	function onFinishCast( ... )
	{
		try
		{
			this.get("Particles").finish();
		}
		catch( err )
		{
		}

		this.onAbilityCancel();
	}

	function onSecondaryHit( ... )
	{
		this.getSource().cue("SyphonSelf", this.getSource());
		this._cueSecondary("SyphonTarget");
	}

}

class this.EffectDef.SyphonSelf extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SyphonSelf";
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
			particleSystem = "Par-Syphon-Dual",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		buff.add("Spin", {
			speed = this.mSpinSpeed
		});
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-Syphon-Imbue",
			emitterPoint = "core",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		this.fireIn(this.mParticleLife, "onParticleDone");
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.SyphonTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SyphonTarget";
	mSound = "Sound-Ability-Syphon_Effect.ogg";
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
			particleSystem = "Par-Syphon-Dual",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		buff.add("Spin", {
			speed = this.mSpinSpeed
		});
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-Syphon-Imbue2",
			emitterPoint = "core",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		this.onSound();
		this.fireIn(this.mParticleLife, "onParticleDone");
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Wither extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Wither";
	mSound = "Sound-Ability-Wither_Cast.ogg";
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
		this.onParticles();
		this.fireIn(0.5, "onCastSound");
		this.fireIn(0.60000002, "onStopCastFire");
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
			particleSystem = "Par-Wither_Gather",
			emitterPoint = "casting"
		});
	}

	function onCastSound( ... )
	{
		this.onSound();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Wither_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cue("WitherHit", this.getTarget());
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.WitherHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WitherHit";
	mSound = "Sound-Ability-Wither_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Wither_Explosion",
			emitterPoint = "node"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Wither_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.75, "onDone");
	}

}

class this.EffectDef.MysticMissile extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticMissile";
	mSound = "Sound-Ability-Mysticmissile_Cast.ogg";
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
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-MysticMissile_Cast",
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
			particleSystem = "Par-MysticMissile_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-MysticMissile_Tail",
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

		this.getTarget().cue("MysticMissileHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.MysticMissileHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MysticMissileHit";
	mSound = "Sound-Ability-Mysticmissile_Effect.ogg";
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
			particleSystem = "Par-MysticMissile_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MysticMissile_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.SoulBurst1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SoulBurst1";
	mSound = "Sound-Ability-Soulburst_Cast.ogg";
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
			animation = "Magic_Throw"
		});
		this.onSound();
		this.fireIn(0.60000002, "onStopCastFire");
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
			particleSystem = "Par-SoulBurst_Cast",
			emitterPoint = "casting"
		});
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-SoulBurst_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("SoulBurstHit", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.SoulBurst2 extends this.EffectDef.SoulBurst1
{
	static mEffectName = "SoulBurst2";
}

class this.EffectDef.SoulBurst3 extends this.EffectDef.SoulBurst1
{
	static mEffectName = "SoulBurst3";
}

class this.EffectDef.SoulBurst4 extends this.EffectDef.SoulBurst1
{
	static mEffectName = "SoulBurst4";
}

class this.EffectDef.SoulBurst5 extends this.EffectDef.SoulBurst1
{
	static mEffectName = "SoulBurst5";
}

class this.EffectDef.SoulBurstHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SoulBurstHit";
	mSound = "Sound-Ability-Soulburst_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-SoulBurst_Explosion2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-SoulBurst_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.MaliceBlast1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MaliceBlast1";
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
			animation = "Magic_Throw"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-MaliceBlast_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.80000001, "onStopCastFire");
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

		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-MaliceBlast_Cast",
			emitterPoint = "casting"
		});
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-MaliceBlast_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("MaliceBlastHit", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.MaliceBlast2 extends this.EffectDef.MaliceBlast1
{
	static mEffectName = "MaliceBlast2";
}

class this.EffectDef.MaliceBlast3 extends this.EffectDef.MaliceBlast1
{
	static mEffectName = "MaliceBlast3";
}

class this.EffectDef.MaliceBlast4 extends this.EffectDef.MaliceBlast1
{
	static mEffectName = "MaliceBlast4";
}

class this.EffectDef.MaliceBlast5 extends this.EffectDef.MaliceBlast1
{
	static mEffectName = "MaliceBlast5";
}

class this.EffectDef.MaliceBlastHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MaliceBlastHit";
	mSound = "Sound-Ability-Maliceblast_Effect.ogg";
	function onStart( ... )
	{
		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MaliceBlast_Explosion2",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-MaliceBlast_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.LifeLeech1 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "LifeLeech1";
	mLoopSound = "Sound-Ability-Lifeleech_Warmup.ogg";
	mDuration = 1.0;
	mCount = 0.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Cast",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Particles",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Spell_Channel",
			loop = true
		});
		this.onLoopSound();
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

		this.getSource().cue("LifeLeechSource", this.getSource());
		this._cueSecondary("LifeLeechTarget");
		this.mCount++;
		this.fireIn(1.0, "onHit");
	}

	function onAbilityCancel( ... )
	{
		this.mCount = this.mDuration;
		::EffectDef.TemplateWarmup.onAbilityCancel();
	}

}

class this.EffectDef.LifeLeech2 extends this.EffectDef.LifeLeech1
{
	static mEffectName = "LifeLeech2";
	mDuration = 2.0;
}

class this.EffectDef.LifeLeech3 extends this.EffectDef.LifeLeech1
{
	static mEffectName = "LifeLeech3";
	mDuration = 3.0;
}

class this.EffectDef.LifeLeech4 extends this.EffectDef.LifeLeech1
{
	static mEffectName = "LifeLeech4";
	mDuration = 4.0;
}

class this.EffectDef.LifeLeech5 extends this.EffectDef.LifeLeech1
{
	static mEffectName = "LifeLeech5";
	mDuration = 5.0;
}

class this.EffectDef.LifeLeechSource extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LifeLeechSource";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Sparkle2",
			emitterPoint = "node"
		});
		this.fireIn(0.2, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Fill",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.LifeLeechTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LifeLeechTarget1";
	mSound = "Sound-Ability-Lifeleech_Effect.ogg";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Sparkle3",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Sparkle4",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(0.2, "onBuffDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-LifeLeech_Fill",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onBuffDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.CreepingDeathWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "CreepingDeathWarmup";
	mLoopSound = "Sound-Ability-Creepingdeath_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-CreepingDeath_Cast",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-CreepingDeath_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-CreepingDeath_Lines",
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

class this.EffectDef.CreepingDeath1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "CreepingDeath1";
	mHit = "CreepingDeathTarget1";
	mSound = "Sound-Ability-Default_Cast.ogg";
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
			animation = "Victory"
		});
		this.onParticles();
		this.fireIn(0.5, "onCastSound");
		this.fireIn(0.60000002, "onStopCastFire");
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
			particleSystem = "Par-Wither_Gather",
			emitterPoint = "casting"
		});
	}

	function onCastSound( ... )
	{
		this.onSound();
	}

	function onStopCastFire( ... )
	{
		this.get("Particles").finish();
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Wither_Cast",
			emitterPoint = "casting"
		});
		this.getTarget().cue(this.mHit, this.getTarget());
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.CreepingDeath2 extends this.EffectDef.CreepingDeath1
{
	static mEffectName = "CreepingDeath2";
	mHit = "CreepingDeathTarget2";
}

class this.EffectDef.CreepingDeath3 extends this.EffectDef.CreepingDeath1
{
	static mEffectName = "CreepingDeath3";
	mHit = "CreepingDeathTarget3";
}

class this.EffectDef.CreepingDeath4 extends this.EffectDef.CreepingDeath1
{
	static mEffectName = "CreepingDeath4";
	mHit = "CreepingDeathTarget4";
}

class this.EffectDef.CreepingDeath5 extends this.EffectDef.CreepingDeath1
{
	static mEffectName = "CreepingDeath5";
	mHit = "CreepingDeathTarget5";
}

class this.EffectDef.CreepingDeathTarget1 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "CreepingDeathTarget1";
	mDuration = 1.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		for( local i = 0; i < this.mDuration; i++ )
		{
			this.fireIn(i, "onFire");
		}

		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onFire()
	{
		this.getSource().cue("CreepingDeathTargetEffect");
	}

}

class this.EffectDef.CreepingDeathTargetEffect extends this.EffectDef.TemplateBasic
{
	mSound = "Sound-Ability-Creepingdeath_Effect.ogg";
	static mParticleLife = 0.69999999;
	static mSpinSpeed = 1.2;
	function onStart()
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff");
		buff.add("ParticleSystem", {
			particleSystem = "Par-CreepingDeath-2GlowingGlobes",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		buff.add("Spin", {
			speed = this.mSpinSpeed
		});
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-CreepingDeath-Hit",
			emitterPoint = "core",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.CreepingDeathTarget2 extends this.EffectDef.CreepingDeathTarget1
{
	static mEffectName = "CreepingDeathTarget2";
	mDuration = 2.0;
}

class this.EffectDef.CreepingDeathTarget3 extends this.EffectDef.CreepingDeathTarget1
{
	static mEffectName = "CreepingDeathTarget3";
	mDuration = 3.0;
}

class this.EffectDef.CreepingDeathTarget4 extends this.EffectDef.CreepingDeathTarget1
{
	static mEffectName = "CreepingDeathTarget4";
	mDuration = 4.0;
}

class this.EffectDef.CreepingDeathTarget5 extends this.EffectDef.CreepingDeathTarget1
{
	static mEffectName = "CreepingDeathTarget5";
	mDuration = 5.0;
}

class this.EffectDef.ReflectiveWard1 extends this.EffectDef.TemplateBasic
{
	mDuration = 30.0;
	static mEffectName = "ReflectiveWard";
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
			particleSystem = "Par-ReflectiveWard_Hand",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("ReflectiveWardTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-ReflectiveWard_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.ReflectiveWard2 extends this.EffectDef.ReflectiveWard1
{
	static mEffectName = "ReflectiveWard2";
	mDuration = 30.0;
}

class this.EffectDef.ReflectiveWard3 extends this.EffectDef.ReflectiveWard1
{
	static mEffectName = "ReflectiveWard3";
	mDuration = 30.0;
}

class this.EffectDef.ReflectiveWard4 extends this.EffectDef.ReflectiveWard1
{
	static mEffectName = "ReflectiveWard4";
	mDuration = 30.0;
}

class this.EffectDef.ReflectiveWard5 extends this.EffectDef.ReflectiveWard1
{
	static mEffectName = "ReflectiveWard5";
	mDuration = 30.0;
}

class this.EffectDef.ReflectiveWardTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ReflectiveWardTarget";
	mSound = "Sound-Ability-MysticProtection_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-ReflectiveWard_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ReflectiveWard_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ReflectiveWard_Symbol1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ReflectiveWard_Symbol2",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.DeathlyDart extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathlyDart";
	mSound = "Sound-Ability-Deathly_Dart_Cast.ogg";
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
			particleSystem = "Par-Deathly_Hands",
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
			particleSystem = "Par-DeathlyDart_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeathlyDart_Ball2",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "BA55D3",
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

		this.getTarget().cue("DeathlyDartHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.DeathlyDartHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeathlyDartHit";
	mSound = "Sound-Ability-Deathly_Dart_Effect.ogg";
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
			particleSystem = "Par-DeathlyDart_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-DeathlyDart_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeathlyDart_Fill",
			emitterPoint = "spell_target"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-DeathlyDart_Fill2",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

