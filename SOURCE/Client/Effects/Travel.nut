this.require("EffectDef");
class this.EffectDef.BounderDash extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BounderDash";
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
			particleSystem = "Par-BounderDash_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("BounderDashTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-BounderDash_Feet",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.BounderDashTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BounderDashTarget";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-BounderDash_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-BounderDash_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-BounderDash_Feet",
			emitterPoint = "feet"
		});
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.RunAway extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RunAway";
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
			particleSystem = "Par-RunAway_Gather",
			emitterPoint = "right_hand"
		});
		this.onSound();
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("RunAwayTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.RunAwayTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "RunAwayTarget";
	mSound = "Sound-Ability-RunAway_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Feet",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Feet2",
			emitterPoint = "feet"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-RunAway_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.5, "onDone");
	}

}

class this.EffectDef.BindWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "BindWarmup";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Bind_Hand",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Bind_Particles",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Bind_Lines",
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

class this.EffectDef.Bind extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Bind";
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
			particleSystem = "Par-Bind_Gather",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("BindTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Bind_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.BindTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BindTarget";
	mSound = "Sound-Ability-Snare_Effect.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local buff = this.createGroup("Buff", this.getSource());
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bind_Sparkle",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bind_Fill",
			emitterPoint = "node"
		});
		buff.add("ParticleSystem", {
			particleSystem = "Par-Bind_Symbol",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

class this.EffectDef.SelfPortalWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "SelfPortalWarmup";
	mMaxWarmupTime = 29.0;
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
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.SelfPortal extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SelfPortal";
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
			particleSystem = "Par-Portal_Gather",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("SelfPortalTarget", this.getSource());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Portal_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.SelfPortalTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SelfPortalTarget";
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

class this.EffectDef.PortalHadrian extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalHadrian";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Hadrian",
			emitterPoint = "node"
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

class this.EffectDef.PortalBind extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalBind";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Bind",
			emitterPoint = "node"
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

class this.EffectDef.PortalHeartwood extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalHeartwood";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Heartwood",
			emitterPoint = "node"
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

class this.EffectDef.PortalNewbadari extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalNewbadari";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Newbadari",
			emitterPoint = "node"
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

class this.EffectDef.PortalStonehenge extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalStonehenge";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Stonehenge",
			emitterPoint = "node"
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

class this.EffectDef.PortalSwineland extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalSwineland";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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
			particleSystem = "Par-Portal_Twinkle",
			emitterPoint = "node"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Portal_Swineland",
			emitterPoint = "node"
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

class this.EffectDef.PortalWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "PortalWarmup";
	mLoopSound = "Sound-Ability-Portalbind_Warmup.ogg";
	mMaxWarmupTime = 29.0;
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

class this.EffectDef.Portal extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Portal";
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
		this.getTarget().cue("PortalTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Portal_Burst",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.PortalTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "PortalTarget";
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

