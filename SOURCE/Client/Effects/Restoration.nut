this.require("EffectDef");
class this.EffectDef.GaiasEmbrace extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "GaiasEmbrace";
	mLoopSound = "Sound-Ability-Gaiasembrace_Warmup.ogg";
	mDuration = 6.0;
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
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "resurrect.png",
			orthoWidth = 15,
			orthoHeight = 15,
			far = 150,
			offset = this.Vector3(0, 60, 0),
			additive = true
		});
		aoe.add("Spin", {
			axis = "y",
			speed = 0.2
		});

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(this.mDuration, "onAbilityCancel");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("HealingTendrilTarget");
	}

}

class this.EffectDef.GaiasEmbraceTarget extends this.EffectDef.TemplateBasic
{
	mSound = "Sound-Ability-Gaiasembrace_Effect.ogg";
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

class this.EffectDef.HealingHandWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "HealingHandWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "resurrect.png",
			orthoWidth = 15,
			orthoHeight = 15,
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

class this.EffectDef.HealingHand extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingHand";
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
		this.getTarget().cue("HealingHandTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Healing-Burst2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.HealingHandTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingHandTarget";
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

class this.EffectDef.HealingTendrilWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "HealingTendrilWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "resurrect.png",
			orthoWidth = 15,
			orthoHeight = 15,
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

class this.EffectDef.HealingTendril extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "HealingTendril";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 7.0;
	mProjs = null;
	mProjVersion = 1;
	mDebugger = null;
	function onStart( ... )
	{
		this.debug("onStart");
		this.mProjs = {};

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
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		this.fireIn(this.mDuration, "onAbilityCancel");
		this.onSound();

		if (!this._secondaryTargetCheck())
		{
			return;
		}

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		for( local i = 0; i < this.mDuration; i++ )
		{
			local time = i * 1.0 + 0.30000001;
			this.fireIn(time, "onProjectile");
		}
	}

	function _createProjectile1( group, tID, target )
	{
		this.debug("_createProjectile1 called");

		if (!(tID in this.mProjs))
		{
			this.mProjs[tID] <- [];
		}

		local proj = this.createGroup(group);
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = target,
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 4.0,
			topSpeed = 5.0,
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
			particleSystem = "Par-Tendril_Streams",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.5,
			width = 0.5,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ffffff",
			colorChange = [
				0.5,
				0.5,
				0.5,
				0.5
			]
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
		this.mProjs[tID].append(group);
	}

	function _createProjectile2( group, tID, target )
	{
		this.debug("_createProjectile2 called");

		if (!(tID in this.mProjs))
		{
			this.mProjs[tID] <- [];
		}

		local proj = this.createGroup(group);
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = target,
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 4.0,
			topSpeed = 5.0,
			orient = true,
			arcEnd = 0.5,
			arcSideAngle = 10.0,
			arcForwardAngle = 90.0
		});
		proj.add("Mesh", {
			mesh = "Item-Invisible.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Tendril_Streams",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.5,
			width = 0.5,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ffffff",
			colorChange = [
				0.5,
				0.5,
				0.5,
				0.5
			]
		});
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
		this.mProjs[tID].append(group);
	}

	function onProjectile( ... )
	{
		this.debug("onProjectile called");
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

			if (this.mProjVersion == 1)
			{
				this._createProjectile1(group, i, sec[i]);
			}
			else if (this.mProjVersion == 2)
			{
				this._createProjectile2(group, i, sec[i]);
			}
			else if (this.mProjVersion == 3)
			{
				this._createProjectile1(group, i, sec[i]);
			}
			else if (this.mProjVersion == 4)
			{
				this._createProjectile2(group, i, sec[i]);
			}
			else if (this.mProjVersion == 5)
			{
				this._createProjectile1(group, i, sec[i]);
			}
			else if (this.mProjVersion == 6)
			{
				this._createProjectile2(group, i, sec[i]);
			}
			else if (this.mProjVersion == 7)
			{
				this._createProjectile1(group, i, sec[i]);
			}

			this.mCount++;
		}

		this.mProjVersion++;
	}

	function onContact( ... )
	{
		local distance = 0;
		local targetID;
		local projID;
		local found = false;
		local sec = this.getSecondaryTargets();

		for( local i = 0; i < sec.len(); i++ )
		{
			if (i in this.mProjs)
			{
				if (this.mProjs[i].len() > 0)
				{
					for( local x = 0; x < this.mProjs[i].len(); x++ )
					{
						local newDist = this._distanceCheck(sec[i], this.mProjs[i][x]);

						if (distance >= newDist)
						{
							distance = newDist;
							targetID = i;
							projID = x;
							found = true;
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
				this.debug("ID: " + i + ", Value: " + x);
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
						try
						{
							this.get(this.mProjs[i][x]).finish();
						}
						catch( err )
						{
						}

						this.mProjs[i].remove(x);
					}
				}
			}
		}
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("HealingTendrilTarget");
	}

	function onDone( ... )
	{
		this.debug("onDone Called");

		try
		{
			this.get("Particles").finish();
		}
		catch( err )
		{
		}

		this.mProjVersion = 1;
		this.onClearProjectiles();
		this.finish();
	}

}

class this.EffectDef.HealingTendrilTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingTendrilTarget";
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

class this.EffectDef.HealingBreezeWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "HealingBreezeWarmup";
	mLoopSound = "Sound-Ability-Frostspells_Warmup.ogg";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "resurrect.png",
			orthoWidth = 15,
			orthoHeight = 15,
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

class this.EffectDef.HealingBreeze1t extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingBreeze1t";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mDuration = 8.0;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		this.add("Dummy");
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

		for( local i = 1; i <= this.mDuration; i++ )
		{
			local time = i * 1.0;
			this.fireIn(time, "onSecondaryHit");
		}

		this.fireIn(0.40000001, "onFinishCast");
		this.fireIn(this.mDuration + 0.5, "onDone");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		this.getTarget().cue("HealingBreezeTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
	}

	function onSecondaryHit( ... )
	{
		this._cueSecondary("HealingBreezeSecondaryTarget");
	}

}

class this.EffectDef.HealingBreeze2t extends this.EffectDef.HealingBreeze1t
{
	static mEffectName = "HealingBreeze2t";
	mDuration = 10.0;
}

class this.EffectDef.HealingBreezeTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingBreezeTarget";
	mSound = "Sound-Ability-Healingbreeze_Effect.ogg";
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

class this.EffectDef.HealingBreezeSecondaryTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HealingBreezeSecondaryTarget";
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

class this.EffectDef.ResurrectWarmup extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "ResurrectWarmup";
	mLoopSound = "Sound-Ability-Resurrect_Warmup.ogg";
	mMaxWarmupTime = 8.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Fill",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Leaves",
			emitterPoint = "casting"
		});
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Healing_Warmup_Lines",
			emitterPoint = "casting"
		});
		warmup.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.onLoopSound();
		local aoe = this.createGroup("AOE");
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 250,
			target = aoe.getObject(),
			textureName = "resurrect.png",
			orthoWidth = 15,
			orthoHeight = 15,
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

class this.EffectDef.Resurrect extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Resurrect";
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
		this._cueTargets("ResurrectTarget");
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.ResurrectTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "ResurrectTarget";
	mSound = "Sound-Ability-Vigorofthesargon_Effect.ogg";
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

class this.EffectDef.VigorOfTheSargon extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VigorOfTheSargon";
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
		this.getTarget().cue("VigorOfTheSargonTarget", this.getTarget());
		local sparks = this.createGroup("Sparks", this.getSource());
		sparks.add("ParticleSystem", {
			particleSystem = "Par-Hatred_Gather2",
			emitterPoint = "right_hand",
			particleScale = 0.75
		});
		this.fireIn(0.40000001, "onDone");
	}

}

class this.EffectDef.VigorOfTheSargonTarget extends this.EffectDef.TemplateBasic
{
	static mEffectName = "VigorOfTheSargonTarget";
	mSound = "Sound-Ability-Vigorofthesargon_Effect.ogg";
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
			particleSystem = "Par-Healing-2GlowingGlobesRed",
			particleScale = this.getSource().getScale().y,
			particleScaleProps = this.PSystemFlags.SIZE | this.PSystemFlags.VELOCITY
		});
		buff.add("Spin", {
			speed = this.mSpinSpeed
		});
		local heal = this.createGroup("GreenEnergy", this.getSource());
		heal.add("ParticleSystem", {
			particleSystem = "Par-VigorOfTheSargon-Imbue",
			emitterPoint = "node"
		});
		this.onSound();
		this.fireIn(this.mParticleLife, "onParticleDone");
		this.fireIn(1.5, "onDone");
	}

}

