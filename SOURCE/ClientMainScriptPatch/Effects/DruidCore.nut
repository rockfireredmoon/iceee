this.require("EffectDef");
class this.EffectDef.RootWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "RootWarmup";
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
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.Root extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Root";
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

class this.EffectDef.RootTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RootTarget";
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
class this.EffectDef.TelekinesisWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "TelekinesisWarmup";
	mLoopSound = "Sound-Ability-Haste_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Telekinesis_Cast",
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

class this.EffectDef.Telekinesis extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Telekinesis";
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
		this.getTarget().cue("TelekinesisTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Root_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.TelekinesisTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TelekinesisTarget";
	mSound = "Sound-Ability-Thorns_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Telekinesis_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Telekinesis_Symbol2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Telekinesis_Symbol",
			emitterPoint = "node"
		});
		buff.add("FFAnimation", {
			animation = "Magic_Spellgasm",
			loop = true
		});
		this.onSound();
		local residual = this.createGroup("Residual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Root_Fill",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onInitialDone");
		this.fireIn(8.0, "onAnimDone");
	}

	function onInitialDone( ... )
	{
		this.get("Buff").finish();
	}
	
	function onAnimDone( ... )
	{
		this._stopAnimation();
	}

}

class this.EffectDef.HasteWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "HasteWarmup";
	mLoopSound = "Sound-Ability-Haste_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Haste_Cast_Fill",
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

class this.EffectDef.Haste extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Haste";
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
			particleSystem = "Par-Haste_Cast_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Haste_Cast_Sparks",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onBurst");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Haste_Burst",
			emitterPoint = "right_hand"
		});
		this.getTarget().cue("HasteTarget", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.HasteTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HasteTarget";
	mSound = "Sound-Ability-Haste_Effect.ogg";
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
			particleSystem = "Par-Haste_Spin2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Haste_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.WolfSpiritWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "WolfSpiritWarmup";
	mLoopSound = "Sound-Ability-Haste_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Hands2",
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

class this.EffectDef.WolfSpirit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WolfSpirit";
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
			particleSystem = "Par-WolfSpirit_Hands2",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("WolfSpiritTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.WolfSpiritTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "WolfSpiritTarget";
	mSound = "Sound-Ability-Wolfspirit_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		local residual = this.createGroup("Residual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-WolfSpirit_Hands",
			emitterPoint = "casting"
		});
		this.fireIn(1.5, "onInitialDone");
		this.fireIn(7.0, "onDone");
	}

	function onInitialDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.ShadowSpiritWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ShadowSpiritWarmup";
	mLoopSound = "Sound-Ability-DeathProtection_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Hands",
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

class this.EffectDef.ShadowSpirit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShadowSpirit";
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
			particleSystem = "Par-ShadowSpirit_Hands",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("ShadowSpiritTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onDone");
	}

}

class this.EffectDef.ShadowSpiritTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ShadowSpiritTarget";
	mSound = "Sound-Ability-Malediction_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Shadow_Spirit_Halo",
			emitterPoint = "node"
		});
		buff.add("Mesh", {
			mesh = "Item-Black_Sphere.mesh",
			point = "node",
			fadeInTime = 1.0,
			fadeOutTime = 0.5
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Fill",
			emitterPoint = "node"
		});
		this.onSound();
		local residual = this.createGroup("Residual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-ShadowSpirit_Hands",
			emitterPoint = "casting"
		});
		this.fireIn(1.5, "onInitialDone");
		this.fireIn(7.0, "onDone");
	}

	function onInitialDone( ... )
	{
		this.get("Buff").finish();
	}

}

class this.EffectDef.SnareWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SnareWarmup";
	mLoopSound = "Sound-Ability-MysticProtection_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Snare_Cast_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Snare_Cast_Swirls",
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

class this.EffectDef.Snare extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Snare";
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
			particleSystem = "Par-Snare_Cast_Swirls2",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.0099999998, "onFill");
		this.fireIn(0.5, "onBurst");
	}

	function onFill( ... )
	{
		local particles = this.get("Particles");
		particles.add("ParticleSystem", {
			particleSystem = "Par-Snare_Cast_Fill",
			emitterPoint = "casting"
		});
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Snare_Burst",
			emitterPoint = "right_hand"
		});
		this.getTarget().cue("SnareTarget", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.SnareTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SnareTarget";
	mSound = "Sound-Ability-Snare_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Snare_Burst",
			emitterPoint = "spell_target"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Snare_Target_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Snare_Feet_Swirls",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(2.5, "onDone");
	}

}

class this.EffectDef.Exodus extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Exodus";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
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
			particleSystem = "Par-Exodus_Gather",
			emitterPoint = "right_hand"
		});
		this.onLoopSound();
		this.fireIn(9.0, "onFinishCast");
	}

	function onAbilityWarmupComplete( ... )
	{
	}

	function onFinishCast( ... )
	{
		this.log.debug("Martin: on finish called");
		this.get("Particles").finish();
		this._cueSecondary("ExodusTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("FFAnimation", {
			animation = "Victory"
		});
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Exodus_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onAbilityCancel");
	}

}

class this.EffectDef.ExodusTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ExodusTarget";
	mSound = "Sound-Ability-Ladyluck_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Exodus_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Exodus_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Exodus_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Sting extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Sting";
	mSound = "Sound-Ability-Sting_Cast.ogg";
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
		this.fireIn(0.2, "onArrowSound");
		this.fireIn(0.30000001, "onAddArrow");
		this.fireIn(1.0, "onProjectile");
	}

	function onArrowSound( ... )
	{
		this.onSound();
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
		cast.add("ParticleSystem", {
			particleSystem = "Par-Sting_Gather",
			emitterPoint = "right_hand_Arrow"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-Sting_Fill",
			emitterPoint = "right_hand_Arrow"
		});
	}

	function onProjectile( ... )
	{
		this.get("Cast").finish();
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
		proj.add("Ribbon", {
			offset = -0.5,
			width = 1.0,
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
		proj.add("ParticleSystem", {
			particleSystem = "Par-Sting_Projectile",
			emitterPoint = "node"
		});
	}

	function onContact( ... )
	{
		this.getTarget().cue("StingHit", this.getTarget());
		this.getTarget().uncork();
		local proj = this.get("Projectile");
		proj.add("ParticleSystem", {
			particleSystem = "Par-Sting_Impact",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Sting_Smoke",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Sting_Shot_Burst",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.StingHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "StingHit";
	mSound = "Sound-Ability-Sting_Effect.ogg";
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
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.SoulNeedlesWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SoulNeedles";
	mLoopSound = "Sound-Ability-Soulneedles_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Warmup_Glow",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Warmup_Lines",
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

class this.EffectDef.SoulNeedles extends this.EffectDef.TemplateMultiProjectile
{
	static mEffectName = "SoulNeedles";
	mSound = "Sound-Ability-Soulneedles_Cast.ogg";
	mLoopSound = "Sound-Ability-Soulneedles_Projectile.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.mEffectsPackage = this._getEffectsPackage(this.getSource());
		this.add("Dummy");
		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local part = this.createGroup("CastParticles", this.getSource());
		part.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Warmup_Glow",
			emitterPoint = "casting"
		});
		part.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Warmup_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this._corkTargets();
		this.fireIn(0.5, "onProjectile");
	}

	function _createProjectile( group, arcEnd, arcSideAngle, topVelocity, intVelocity )
	{
		local target = this.getTarget();
		local proj = this.createGroup(group);

		if (this.mEffectsPackage == "Biped")
		{
			proj.add("ArcToTarget", {
				source = this.getSource(),
				sourcePoint = "right_hand",
				target = this.getTarget(),
				targetPoint = "spell_target",
				intVelocity = intVelocity,
				accel = 4.0,
				topSpeed = topVelocity,
				orient = true,
				arcEnd = arcEnd,
				arcSideAngle = arcSideAngle,
				arcForwardAngle = 10.0
			});
		}
		else
		{
			proj.add("ArcToTarget", {
				source = this.getSource(),
				sourcePoint = "horde_caster",
				target = this.getTarget(),
				targetPoint = "spell_target",
				intVelocity = intVelocity,
				accel = 4.0,
				topSpeed = topVelocity,
				orient = true,
				arcEnd = arcEnd,
				arcSideAngle = arcSideAngle,
				arcForwardAngle = 10.0
			});
		}

		proj.add("Mesh", {
			mesh = "Item-Shard.mesh",
			FadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Ring",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.0,
			width = 2.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "00a2ff",
			colorChange = [
				3.0,
				3.0,
				3.0,
				0.75
			]
		});
		proj.add("Ribbon", {
			offset = 1.5,
			width = 1.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "cc00ff",
			colorChange = [
				3.0,
				3.0,
				3.0,
				0.75
			]
		});
		proj.add("Ribbon", {
			offset = -2.5,
			width = 1.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "cc00ff",
			colorChange = [
				3.0,
				3.0,
				3.0,
				0.75
			]
		});
		proj.add("ScaleTo", {
			size = 0.5,
			maintain = true,
			duration = 0.0
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
		this.get("CastParticles").finish();

		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
			}

			this.onDone();
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		this.mProjs = [];
		this._createProjectile("proj1", 0.0, 0.0, 200.0, 9.0);
		this.mProjs.append("proj1");
		this._createProjectile("proj2", 0.1, 90.0, 175.0, 6.0);
		this.mProjs.append("proj2");
		this._createProjectile("proj3", 0.2, -90.0, 150.0, 3.0);
		this.mProjs.append("proj3");
		this.onLoopSound(this.get(this.mProjs[2]));
	}

	function clearProjectiles( ... )
	{
		for( local i = this.mProjs.len() - 1; i > -1; i-- )
		{
			local groupName = this.mProjs[i];
			local proj = this.get("groupName");

			if (proj)
			{
				this.get(groupName).finish();
				this.mProjs.remove(i);
				this.mCount++;
			}
		}
	}

	function onContact( ... )
	{
		if (!this.getSource())
		{
			if (this.getTarget())
			{
				this.getTarget().uncork();
			}

			this.onDone();
		}

		if (!this.getTarget())
		{
			this.onDone();
		}

		for( local i = this.mProjs.len() - 1; i > -1; i-- )
		{
			local groupName = this.mProjs[i];
			local distance = this._distanceCheck(this.getTarget(), groupName);

			if (distance == 0)
			{
				this.getTarget().cue("SoulNeedlesHit", this.getTarget());

				if (groupName == "proj3")
				{
					this.onLoopSoundStop();
				}

				this.get(groupName).finish();
				this.mProjs.remove(i);
				this.mCount++;
			}
		}

		if (this.mProjs.len() == 0 && this.mCount == 3)
		{
			this._uncorkTargets();
			this.fireIn(0.1, "onDone");
		}
	}

	function onDone( ... )
	{
		this.clearProjectiles();
		this.finish();
	}

}

class this.EffectDef.SoulNeedlesHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SoulNeedlesHit";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$",
			speed = 3.0
		});
		this.fireIn(0.30000001, "onDone");
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Burst",
			emitterPoint = "spell_target"
		});
		burst.add("ParticleSystem", {
			particleSystem = "Par-Soul_Needles_Burst2",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onBurstDone");
	}

	function onBurstDone( ... )
	{
		if (!this.getSource())
		{
			this.onDone();
		}

		this.get("Burst").finish();
	}

}

class this.EffectDef.Trauma extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Trauma";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
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

	function on1h( ... )
	{
		this.getTarget().cork();
		local a = this.createGroup("Anim", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Side_Cuts",
			speed = 1.3
		});
		this.fireIn(0.25, "onBurst");
		this.fireIn(0.44999999, "onImpact");
		this.fireIn(1.0, "onDone");
		a.add("WeaponRibbon", {
			initialColor = "e2ff80",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
	}

	function onPole( ... )
	{
		this.getTarget().cork();
		local Am = this.createGroup("Anim", this.getSource());
		Am.add("FFAnimation", {
			animation = "Staff_Spinning_Smash"
		});
		this.fireIn(0.2, "onUp");
		this.fireIn(0.5, "onReturn");
		this.fireIn(0.55000001, "onPause");
		this.fireIn(0.57999998, "onBurst");
		this.fireIn(0.66000003, "onImpact");
		this.fireIn(1.0, "onDone");
		Am.add("WeaponRibbon", {
			initialColor = "e2ff80",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
	}

	function onUp( ... )
	{
		local Dt = this.createGroup("Dirt");
		Dt.add("ParticleSystem", {
			particleSystem = "Par-Trauma_Dust"
		});
	}

	function onReturn( ... )
	{
		local Dt = this.get("Dirt");
		Dt.stop();
	}

	function onPause( ... )
	{
		this.get("Spin").finish();
	}

	function onBurst( ... )
	{
		this.getTarget().cue("TraumaHit", this.getTarget());
		this.getTarget().uncork();
	}

	function onImpact( ... )
	{
		this.get("Anim").finish();
	}

}

class this.EffectDef.TraumaHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TraumaHit";
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
			particleSystem = "Par-Trauma_Impact",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.DeadlyShot1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeadlyShot1";
	mSound = "Sound-Ability-Deadlyshot_Cast.ogg";
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
		this.onSound();
		this.fireIn(0.30000001, "onAddArrow");
		this.fireIn(1.0, "onProjectile");
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
		cast.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Arrow_Head",
			emitterPoint = "right_hand_Arrow"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Sparks",
			emitterPoint = "right_hand_Arrow"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Fill",
			emitterPoint = "right_hand_Arrow"
		});
	}

	function onProjectile( ... )
	{
		this.get("Cast").finish();
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
		proj.add("Ribbon", {
			offset = -0.5,
			width = 1.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ff0000",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Arrow_Head",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Sparks",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Fill",
			emitterPoint = "node"
		});
	}

	function onContact( ... )
	{
		this.getTarget().cue("DeadlyShotHit", this.getTarget());
		this.getTarget().uncork();
		local proj = this.get("Projectile");
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Impact",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-DeadlyShot_Skulls",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.DeadlyShot2 extends this.EffectDef.DeadlyShot1
{
	static mEffectName = "DeadlyShot2";
}

class this.EffectDef.DeadlyShot3 extends this.EffectDef.DeadlyShot1
{
	static mEffectName = "DeadlyShot3";
}

class this.EffectDef.DeadlyShot4 extends this.EffectDef.DeadlyShot1
{
	static mEffectName = "DeadlyShot4";
}

class this.EffectDef.DeadlyShot5 extends this.EffectDef.DeadlyShot1
{
	static mEffectName = "DeadlyShot5";
}

class this.EffectDef.DeadlyShotHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "DeadlyShotHit";
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
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.ArrowStorm5 extends this.EffectDef.TemplateMultiProjectile
{
	static mEffectName = "ArrowStorm";
	mSound = "Sound-Ability-Arrowstorm_Cast2.ogg";
	function onStart( ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		this._corkSecondary();
		local ongoing = this.createGroup("OnGoing", this.getSource());
		ongoing.add("ParticleSystem", {
			particleSystem = "Par-ArrowStorm_ArrowHead",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.0, "onThrow");
		this.fireIn(0.80000001, "onThrow");
		this.fireIn(1.6, "onThrow");
		this.fireIn(2.4000001, "onThrow");
		this.fireIn(3.2, "onThrow");
		this.mProjs = [];
	}

	function onThrow( ... )
	{
		local hands = this.createGroup("QuickCast" + this.mCount, this.getSource());
		hands.add("FFAnimation", {
			animation = "Bow_Attack_Release",
			speed = 5.0
		});
		this.fireIn(0.1, "onShotSound");
		this.fireIn(0.1, "onThrowAddArrow");
		this.fireIn(0.2, "onProjectile");
	}

	function onShotSound( ... )
	{
		this.onSound("Sound" + this.mCount);
	}

	function onThrowAddArrow( ... )
	{
		local hands = this.get("QuickCast" + this.mCount);
		hands.add("Mesh", {
			mesh = "Item-Arrow.mesh",
			point = "right_hand",
			fadeInTime = 0.5,
			fadeOutTime = 0.0099999998
		});
	}

	function onProjectile( ... )
	{
		this.get("QuickCast" + this.mCount).finish();

		if (this.gLogEffects)
		{
			this.log.debug("Effect " + this.mEffectName + ": onProjectile called");
		}

		local projGroupName = "Projectile" + this.mCount;
		local proj = this.createGroup(projGroupName);
		this.mProjs.append(projGroupName);
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getSecondaryTarget(),
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
		proj.add("Ribbon", {
			offset = -0.5,
			width = 1.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "79cbff",
			colorChange = [
				6.0,
				6.0,
				6.0,
				6.0
			]
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-ArrowStorm_ArrowHead",
			emitterPoint = "node"
		});
		this.mCount++;
	}

	function onContact( ... )
	{
		for( local i = this.mProjs.len() - 1; i > -1; i-- )
		{
			local groupName = this.mProjs[i];
			local distance = this._distanceCheck(this.getSecondaryTarget(), groupName);

			if (distance == 0)
			{
				this.getSecondaryTarget().cue("ArrowStormHit", this.getSecondaryTarget());
				this.get(groupName).finish();
				this.mProjs.remove(i);
			}
		}

		if (this.mProjs.len() == 0 && this.mCount == 5)
		{
			this._uncorkSecondary();
			this.fireIn(0.1, "onDone");
		}
	}

}

class this.EffectDef.ArrowStormHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ArrowStormHit";
	mSound = "Sound-Ability-Arrowstorm_Effect1.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck)
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "$HIT$"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-ArrowStorm_Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.MaledictionWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "MaledictionWarmup";
	mLoopSound = "Sound-Ability-Malediction_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Malediction_Gather",
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

class this.EffectDef.Malediction1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Malediction1";
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
			particleSystem = "Par-Malediction_Lines",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Malediction_Glow",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.80000001, "onBurst");
	}

	function onBurst( ... )
	{
		this.get("Particles").finish();
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Malediction_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("MaledictionTarget", this.getTarget());
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.Malediction2 extends this.EffectDef.Malediction1
{
	static mEffectName = "Malediction2";
}

class this.EffectDef.Malediction3 extends this.EffectDef.Malediction1
{
	static mEffectName = "Malediction3";
}

class this.EffectDef.Malediction4 extends this.EffectDef.Malediction1
{
	static mEffectName = "Malediction4";
}

class this.EffectDef.Malediction5 extends this.EffectDef.Malediction1
{
	static mEffectName = "Malediction5";
}

class this.EffectDef.MaledictionTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MaledictionTarget";
	mSound = "Sound-Ability-Malediction_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Malediction_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Malediction_Fill",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.Sacrifice1 extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Sacrifice1";
	mLoopSound = "Sound-Ability-Lifeleech_Warmup.ogg";
	mDuration = 8.0;
	mCount = 0.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Warmup_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Warmup_Sparkle",
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

		this.getSource().cue("SacrificeSelfDrain", this.getSource());
		this._cueSecondary("SacrificeTarget");
		this.mCount++;
		this.fireIn(1.0, "onHit");
	}

	function onAbilityCancel( ... )
	{
		this.mCount = this.mDuration;
		::EffectDef.TemplateWarmup.onAbilityCancel();
	}

}

class this.EffectDef.Sacrifice2 extends this.EffectDef.Sacrifice1
{
	static mEffectName = "Sacrifice2";
	mDuration = 8.0;
}

class this.EffectDef.Sacrifice3 extends this.EffectDef.Sacrifice1
{
	static mEffectName = "Sacrifice3";
	mDuration = 8.0;
}

class this.EffectDef.Sacrifice4 extends this.EffectDef.Sacrifice1
{
	static mEffectName = "Sacrifice4";
	mDuration = 8.0;
}

class this.EffectDef.Sacrifice5 extends this.EffectDef.Sacrifice1
{
	static mEffectName = "Sacrifice5";
	mDuration = 8.0;
}

class this.EffectDef.SacrificeSelfDrain extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SacrificeSelfDrain";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Sparkle1",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Sparkle3",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Fill2",
			emitterPoint = "node"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.SacrificeTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SacrificeTarget";
	mSound = "Sound-Ability-Sacrifice_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Sparkle2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Sparkle4",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Sacrifice_Fill1",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.HeartOfGaia5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HeartOfGaia5";
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
			particleSystem = "Par-GaiaHeart_Gather",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-GaiaHeart_Lines",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.5, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getSource().cue("HeartOfGaiaTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-GaiaHeart_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.HeartOfGaiaTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HeartOfGaiaTarget";
	mSound = "Sound-Ability-Heartofgaia_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff");
		buff.add("ParticleSystem", {
			particleSystem = "Par-GaiaHeart_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-GaiaHeart_Fill2",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-GaiaHeart_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.TheftOfMight1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TheftOfMight1";
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
			particleSystem = "Par-Theft_Might_Cast_Spray",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Theft_Will_Cast_Glow",
			emitterPoint = "casting"
		});
		this.onSound();
		this.fireIn(0.80000001, "onProjectile");
		this.fireIn(0.80000001, "onBurst");
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
			particleSystem = "Par-Theft_Might_Stream",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 1.0,
			height = 1.0,
			maxSegments = 32,
			initialColor = "EE0000",
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
			particleSystem = "Par-Theft_Might_Burst",
			emitterPoint = "casting"
		});
		this.getTarget().cue("TheftOfMightTarget", this.getTarget());
		this.fireIn(0.80000001, "onDone");
	}

}

class this.EffectDef.TheftOfMight2 extends this.EffectDef.TheftOfMight1
{
	static mEffectName = "TheftOfMight2";
}

class this.EffectDef.TheftOfMight3 extends this.EffectDef.TheftOfMight1
{
	static mEffectName = "TheftOfMight3";
}

class this.EffectDef.TheftOfMight4 extends this.EffectDef.TheftOfMight1
{
	static mEffectName = "TheftOfMight4";
}

class this.EffectDef.TheftOfMight5 extends this.EffectDef.TheftOfMight1
{
	static mEffectName = "TheftOfMight5";
}

class this.EffectDef.TheftOfMightTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TheftOfMightTarget";
	mSound = "Sound-Ability-Theftofmight_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Theft_Might_Target_Spary",
			emitterPoint = "spell_target_head"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Theft_Might_Target_Glow",
			emitterPoint = "spell_target_head"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.FocusedStrike extends this.EffectDef.TemplateMelee
{
	static mEffectName = "FocusedStrike";
	mWeapon = "";
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
		local mWeapon = this._checkMelee();

		if (mWeapon == "1h")
		{
			this.on1h();
		}
		else if (mWeapon == "Pole")
		{
			this.onPole();
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
		this.getTarget().cork();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Spinning_Backswing"
		});
		this._setFireInHit(this.getSource(), "One_Handed_Spinning_Backswing");
	}

	function onPole()
	{
		this.getTarget().cork();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "One_Handed_Spinning_Backswing"
		});
		this._setFireInHit(this.getSource(), "One_Handed_Spinning_Backswing");
	}

	function on2h()
	{
		this.getTarget().cork();
		local a = this.createGroup("Anim1", this.getSource());
		a.add("FFAnimation", {
			animation = "Two_Handed_Thunder_Maul"
		});
		this.fireIn(0.69999999, "onHitFinal");
	}

	function onHit( ... )
	{
		this.getTarget().cue("FocusedStrikeHit", this.getTarget());
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.getTarget().uncork();
		this.fireIn(0.2, "onDone");
	}

}

class this.EffectDef.FocusedStrikeHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "FocusedStrikeHit";
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
			particleSystem = "Par-FocusedStrike_Impact",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-FocusedStrike_Burst",
			emitterPoint = "spell_target"
		});
		this.fireIn(0.1, "onDone");
	}

}

