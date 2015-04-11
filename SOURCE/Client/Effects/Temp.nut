this.require("EffectDef");
class this.EffectDef.Temp_CastPrep extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Temp_CastPrep";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hands = this.createGroup("CastPrep", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Casting",
			loop = true
		});
		this.fireIn(20.0, "onAbilityCancel");
	}

	function onAbilityWarmupComplete( ... )
	{
		this.onDone();
	}

	function onAbilityCancel( ... )
	{
		local ah = this.getSource().getAnimationHandler();

		if (ah)
		{
			ah.fullStop();
		}

		this.onDone();
	}

}

class this.EffectDef.Temp_ChannelRecive extends this.EffectScript
{
	static mEffectName = "Temp_ChannelRecive";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local hands = this.createGroup("CastPrep", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Spell_Channel",
			loop = true
		});
		local channel = this.createGroup("channel", this.getSource());
		channel.add("ParticleSystem", {
			particleSystem = "Par-Temp-Suck2",
			emitterPoint = "core"
		});
		local steal = this.createGroup("steal", this.getTarget());
		steal.add("ParticleSystem", {
			particleSystem = "Par-Temp-Steal",
			emitterPoint = "core"
		});
		this.fireIn(4.0, "onAbilityComplete");
		this.fireIn(30.0, "onDone");
	}

	function onAbilityCancel( ... )
	{
		local ah = this.getSource().getAnimationHandler();

		if (ah)
		{
			ah.fullStop();
		}

		this.onDone();
	}

	function onAbilityComplete( ... )
	{
		this.onAbilityCancel();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_QuickCast extends this.EffectScript
{
	static mEffectName = "Temp_QuickCast";
	function onStart( ... )
	{
		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_GetHitMagical extends this.EffectScript
{
	static mEffectName = "Temp_GetHitMagical";
	function onStart( ... )
	{
		local core = this.createGroup("GetHitMagical", this.getSource());
		core.add("FFAnimation", {
			animation = "$HIT$"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-Temp-HitFlash",
			emitterPoint = "core"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-Temp-Magic_Explosion",
			emitterPoint = "core"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_GetHit extends this.EffectScript
{
	static mEffectName = "Temp_GetHit";
	function onStart( ... )
	{
		local core = this.createGroup("GetHit", this.getSource());
		core.add("FFAnimation", {
			animation = "$HIT$"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_MagicalHeal extends this.EffectScript
{
	static mEffectName = "Temp_MagicalHeal";
	function onStart( ... )
	{
		local core = this.createGroup("MagicalHeal", this.getTarget());
		core.add("ParticleSystem", {
			particleSystem = "Par-Temp-Imbue",
			emitterPoint = "core"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_MagicalDebuff extends this.EffectScript
{
	static mEffectName = "Temp_MagicalDebuff";
	function onStart( ... )
	{
		local core = this.createGroup("MagicalDebuff", this.getSource());
		core.add("ParticleSystem", {
			particleSystem = "Par-Temp-Debuff",
			emitterPoint = "core"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE5 extends this.EffectScript
{
	static mEffectName = "Temp_AOE5";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE5", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 100,
			orthoHeight = 100,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY,
			queryFlags = this.QueryFlags.FLOOR
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE10 extends this.EffectScript
{
	static mEffectName = "Temp_AOE10";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE10", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 200,
			orthoHeight = 200,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE15 extends this.EffectScript
{
	static mEffectName = "Temp_AOE15";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE15", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 300,
			orthoHeight = 300,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE20 extends this.EffectScript
{
	static mEffectName = "Temp_AOE20";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE20", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 400,
			orthoHeight = 400,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE25 extends this.EffectScript
{
	static mEffectName = "Temp_AOE25";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE25", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 500,
			orthoHeight = 500,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_AOE30 extends this.EffectScript
{
	static mEffectName = "Temp_AOE30";
	function onStart( ... )
	{
		local aoe = this.createGroup("AOE30", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 600,
			orthoHeight = 600,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(10.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastInstantDamage extends this.EffectScript
{
	static mEffectName = "Temp_CastInstantDamage";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		this.getTarget().cork();
		this.fireIn(0.5, "onHit");
		this.fireIn(30.0, "onDone");
	}

	function onHit( ... )
	{
		this.get("QuickCast").finish();
		this.getTarget().cue("Temp_GetHitMagical", this.getTarget());
		this.onDone();
	}

	function onDone( ... )
	{
		this.getTarget().uncork();
		this.finish();
	}

}

class this.EffectDef.Temp_CastHeal extends this.EffectScript
{
	static mEffectName = "Temp_CastHeal";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		this.fireIn(0.5, "onHeal");
		this.fireIn(30.0, "onDone");
	}

	function onHeal( ... )
	{
		this.get("QuickCast").finish();
		this.getTarget().cue("Temp_MagicalHeal", this.getTarget());
		this.onDone();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastDebuff extends this.EffectScript
{
	static mEffectName = "Temp_CastDebuff";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		this.fireIn(0.5, "onDebuff");
		this.fireIn(30.0, "onDone");
	}

	function onDebuff( ... )
	{
		this.get("QuickCast").finish();
		this.getTarget().cue("Temp_MagicalDebuff", this.getTarget());
		this.onDone();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_InstantCastDebuffAOE15 extends this.EffectScript
{
	static mEffectName = "Temp_InstantCastDebuffAOE15";
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

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		this.fireIn(0.5, "onDebuff");
	}

	function onDebuff( ... )
	{
		this.get("QuickCast").finish();
		local secondaries = this.getTargets();

		if (this.type(secondaries) == "array")
		{
			if (this.gLogEffects)
			{
				this.log.debug("target count: " + secondaries.len());
			}

			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = [
						s
					];
					s.cue("Temp_MagicalDebuff", s_target);
				}
			}
		}

		local aoe = this.createGroup("AOE", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 300,
			orthoHeight = 300,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_InstantCastDamageAOE15 extends this.EffectScript
{
	static mEffectName = "Temp_InstantCastDamageAOE15";
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

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Temp-Cast_Prep",
			emitterPoint = "casting"
		});
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		this.fireIn(0.5, "onDamage");
		this.fireIn(30.0, "onDone");
	}

	function onDamage( ... )
	{
		this.get("QuickCast").finish();
		local secondaries = this.getTargets();

		if (this.type(secondaries) == "array")
		{
			if (this.gLogEffects)
			{
				this.log.debug("target count: " + secondaries.len());
			}

			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = [
						s
					];
					s.cue("Temp_GetHitMagical", s_target);
					s.uncork();
				}
			}
		}

		local aoe = this.createGroup("AOE", this.getSource());
		aoe.add("Projector", {
			duration = 10000,
			fadeIn = 500,
			fadeOut = 500,
			target = this.getSource(),
			textureName = "magic-ring.png",
			orthoWidth = 300,
			orthoHeight = 300,
			offset = this.Vector3(0, 60, 0),
			additive = true,
			visibilityFlags = this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectile extends this.EffectScript
{
	static mEffectName = "Temp_CastProjectile";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("CueEffect", {
			narrative = "Temp_QuickCast",
			target = this.getSource()
		});
		this.getTarget().cork();
		this.fireIn(0.40000001, "onProjectile");
		this.fireIn(30.0, "onDone");
	}

	function onProjectile( ... )
	{
		local target = this.getTarget();
		local proj = this.createGroup("projectile");
		proj.add("MoveToTarget", {
			source = this.getSource(),
			sourceBone = "Bone-RightHand",
			target = this.getTarget(),
			targetBone = "Bone-Back",
			intVelocity = 2.0,
			accel = 5.0,
			topSpeed = 100.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Sphere.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Temp-Projectile",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			colorChange = [
				0.60000002,
				0.60000002,
				0.60000002,
				0.0
			]
		});
		proj.add("ScaleTo", {
			size = 0.5,
			maintain = true,
			duration = 0.0
		});
	}

	function onContact( ... )
	{
		this.get("projectile").finish();
		this.getTarget().cue("Temp_GetHitMagical", this.getTarget());
		this.getTarget().uncork();
		this.onDone();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE5 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE5";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE5", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
		this.fireIn(30.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE10 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE10";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE10", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE15 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE15";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE15", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
		this.fireIn(30.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE20 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE20";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE20", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE25 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE25";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE25", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
		this.fireIn(30.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Temp_CastProjectileAOE30 extends this.EffectDef.Temp_CastProjectile
{
	static mEffectName = "Temp_CastProjectileAOE30";
	function onContact( ... )
	{
		this.getTarget().cue("Temp_AOE30", this.getTarget());
		this.EffectDef.Temp_InstantCastProjectile.onContact();
		this.fireIn(30.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.MeleeEnhanced extends this.EffectScript
{
	static mEffectName = "MeleeEnhanced";
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
				this.log.debug(this.mEffectName + ": No Target");
			}

			return;
		}

		local anim = this.createGroup("Animation", this.getSource());
		anim.add("FFAnimation", {
			animation = "$MELEE$",
			events = {
				[1.0] = "onDone"
			}
		});
		anim.add("WeaponRibbon", {});
		this.fireIn(0.40000001, "onHit");
		this.fireIn(30.0, "onDone");
	}

	function onHit( ... )
	{
		this.getTarget().cue("Temp_GetHit", this.getTarget());
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.TraumaBug1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TraumaBug1";
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

class this.EffectDef.TraumaBug2 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TraumaBug2";
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
			particleSystem = "Par-Trauma_Impact",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onDone");
	}

}

