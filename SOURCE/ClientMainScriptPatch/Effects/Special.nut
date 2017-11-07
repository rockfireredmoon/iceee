this.require("EffectDef");
this.require("Effects/MageCore");
this.EffectDef.AdminZap <- this.EffectDef.Firebolt;
this.EffectDef.AEAdminZap1 <- this.EffectDef.Pyroblast1;
this.EffectDef.AEAdminZap2 <- this.EffectDef.Frostbolt;
this.EffectDef.AEAdminZap3 <- this.EffectDef.DeepFreeze1_1;
this.EffectDef.AEAdminZap4 <- this.EffectDef.MysticMissile;
class this.EffectDef.HengeTest extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HengeTest";
	mCount = 1;
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		for( local i = 0; i < 10; i++ )
		{
			this.fireIn(i * 1.0, "onHenge");
		}
	}

	function onHenge( ... )
	{
		this.getTarget().cue("Henge" + this.mCount, this.getTarget());
		this.mCount++;
	}

}

class this.EffectDef.Henge1 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge1";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count1",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge2 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge2";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count3",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge3 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge3";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count5",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge4 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge4";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl2",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count2",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge5 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge5";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl2",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count4",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge6 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge6";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl2",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count6",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge7 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge7";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl3",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count1",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count2",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge8 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge8";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local henge = this.createGroup("Henge", this.getSource());
		henge.add("ParticleSystem", {
			particleSystem = "Par-Henge_Swirl3",
			emitterPoint = "node"
		});
		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count3",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count4",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge9 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge9";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count5",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count6",
			emitterPoint = "node"
		});
		this.fireIn(0.5, "onCount");
		this.fireIn(1.0, "onDone");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

}

class this.EffectDef.Henge10 extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Henge10";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local count = this.createGroup("Count", this.getSource());
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count1",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count2",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count3",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count4",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count5",
			emitterPoint = "node"
		});
		count.add("ParticleSystem", {
			particleSystem = "Par-Henge_Count6",
			emitterPoint = "node"
		});
		this.fireIn(0.89999998, "onCount");
		this.fireIn(1.0, "onClimax");
	}

	function onCount( ... )
	{
		this.get("Count").finish();
	}

	function onClimax( ... )
	{
		local burst = this.createGroup("Burst", this.getSource());
		burst.add("ParticleSystem", {
			particleSystem = "Par-Henge_Burst",
			emitterPoint = "node"
		});
		this.fireIn(0.1, "onDone");
	}

}

class this.EffectDef.Fix extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Fix";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hammer = this.createGroup("Hammer", this.getSource());
		hammer.add("FFAnimation", {
			animation = "Hammer_Stand"
		});
		hammer.add("Mesh", {
			mesh = "Item-Hammer.mesh",
			point = "right_hand"
		});
		this.getSource().hideHandAttachments();
		this.fireIn(3.0999999, "onDone");
	}

	function onDone( ... )
	{
		this.getSource().unHideHandAttachments();
		this.finish();
	}

}

class this.EffectDef.Daze extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Daze";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Daze_Sparkle1",
			emitterPoint = "daze_stun"
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.Stun extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "Stun";
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local warmup = this.createGroup("Warmup", this.getSource());
		warmup.add("ParticleSystem", {
			particleSystem = "Par-Stun_Sparkle1",
			emitterPoint = "daze_stun"
		});
		this.fireIn(this.mMaxWarmupTime, "onAbilityCancel");
	}

}

class this.EffectDef.LevelDing extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LevelDing";
	mSound = "Sound-Ability-Levelupding_Cast.ogg";
	function onStart( ... )
	{
		local level = this.createGroup("Level", this.getSource());
		level.add("ParticleSystem", {
			particleSystem = "Par-Level_Lines",
			emitterPoint = "node"
		});
		level.add("FFAnimation", {
			animation = "Excited"
		});
		this.onSound();
		this.fireIn(0.1, "onGlyph");
		this.fireIn(0.30000001, "onGlyphEnd");
		this.fireIn(2.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

	function onGlyph( ... )
	{
		local glyph = this.createGroup("Glyph", this.getSource());
		glyph.add("ParticleSystem", {
			particleSystem = "Par-Level_Glyph",
			emitterPoint = "node"
		});
	}

	function onGlyphEnd( ... )
	{
		this.get("Glyph").finish();
	}

}


class this.EffectDef.GuildDing extends this.EffectDef.LevelDing
{
	static mEffectName = "GuildDing";

}

class this.EffectDef.Snowball extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Snowball";
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
			animation = "Sneak_Throw1"
		});
		cast.add("ParticleSystem", {
			particleSystem = "Par-Snowball-Hands",
			emitterPoint = "right_hand"
		});
		this.fireIn(0.30000001, "onProjectile");
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

		this.get("Cast").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
		local proj = this.createGroup("Projectile");
		this._detachGroup(proj, this.getSource().getNode().getPosition());
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 3.0,
			topSpeed = 80.0,
			orient = true,
			arcEnd = 0.40000001,
			arcSideAngle = 0.0,
			arcForwardAngle = 85.0
		});
		proj.add("Mesh", {
			mesh = "Item-Sphere.mesh",
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

		this.getTarget().cue("SnowballHit", this.getTarget());
		this.getTarget().uncork();
		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.SnowballHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "SnowballHit";
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
			particleSystem = "Par-Snowball-Hit",
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

class this.EffectDef.Heart extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Heart";
	mSound = "Sound-Ability-Default_Cast.ogg";
	mLoopSound = "Sound-Ability-Forcebolt_Projectile.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Magic_Spellgasm"
		});
		this.onSound();
		this.onParticles();
	}

	function onParticles( ... )
	{
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Heart_Hands",
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
			sourcePoint = "spell_target",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 4.0,
			accel = 5.0,
			topSpeed = 40.0,
			orient = true
		});
		proj.add("Mesh", {
			mesh = "Item-Heart.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Heart_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Heart_Trail",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "EE2C2C",
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

		this.getTarget().cue("HeartHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.HeartHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HeartHit";
	mSound = "Sound-Ability-Bright_Effect.ogg";
	mDuration = 2.0;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("FFAnimation", {
			animation = "Excited"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Heart_Explosion",
			emitterPoint = "spell_target"
		});
		hit.add("ParticleSystem", {
			particleSystem = "Par-Heart_Explosion2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.1, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Heart_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Heart_Sparkle",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.Tomato extends this.EffectDef.TemplateBasic
{
	static mEffectName = "Tomato";
	mSound = "Sound-Ability-Tomato_Cast.ogg";
	mLoopSound = "Sound-Ability-Forcebolt_Projectile.ogg";
	function onStart( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local cast = this.createGroup("Cast", this.getSource());
		cast.add("FFAnimation", {
			animation = "Sneak_Throw4"
		});
		this.onSound();
		this.onParticles();
	}

	function onParticles( ... )
	{
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
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 3.0,
			topSpeed = 80.0,
			orient = true,
			arcEnd = 0.40000001,
			arcSideAngle = 0.0,
			arcForwardAngle = 85.0
		});
		proj.add("Mesh", {
			mesh = "Item-Tomato.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Tomato_Trail",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -1.5,
			width = 3.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "EE2C2C",
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

		this.getTarget().cue("TomatoHit", this.getTarget());
		this.getTarget().uncork();
		this.get("Projectile").finish();
		this.onLoopSoundStop();
		this.onDone();
	}

}

class this.EffectDef.TomatoHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TomatoHit";
	mSound = "Sound-Ability-Tomato_Effect.ogg";
	mDuration = 3.0;
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
			particleSystem = "Par-Tomato_Explosion",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.5, "onHitDone");
		local residual = this.createGroup("Risidual", this.getSource());
		residual.add("ParticleSystem", {
			particleSystem = "Par-Tomato_Fill",
			emitterPoint = "node"
		});
		residual.add("ParticleSystem", {
			particleSystem = "Par-Tomato_Sparkle",
			emitterPoint = "node"
		});
		this.fireIn(this.mDuration, "onDone");
	}

	function onHitDone( ... )
	{
		this.get("Hit").finish();
	}

}

class this.EffectDef.BlueFirework extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlueFirework";
	mSound = "Sound-Ability-Firework_Cast.ogg";
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
		this.fireIn(0.30000001, "onProjectile");
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

		this.get("Cast").finish();

		if (!this.getTarget())
		{
			this.onDone();
		}

		local target = this.getTarget();
		local proj = this.createGroup("Projectile");
		proj.add("ArcToTarget", {
			source = this.getSource(),
			sourcePoint = "right_hand",
			target = this.getTarget(),
			targetPoint = "spell_target",
			intVelocity = 3.0,
			accel = 3.0,
			topSpeed = 80.0,
			orient = true,
			arcEnd = 0.40000001,
			arcSideAngle = 0.0,
			arcForwardAngle = 85.0
		});
		proj.add("Mesh", {
			mesh = "Item-Bolt_Long.mesh",
			fadeInTime = 0.050000001
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-BlueFirework_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-BlueFirework_Trail",
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

		this.getTarget().cue("BlueFireworkHit", this.getTarget());
		this.getTarget().uncork();
		this.onLoopSoundStop();
		this.get("Projectile").finish();
		this.onDone();
	}

}

class this.EffectDef.BlueFireworkHit extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BlueFireworkHit";
	mSound = "Sound-Ability-Firework_Effect.ogg";
	mDuration = 0.69999999;
	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return;
		}

		local hit = this.createGroup("Hit", this.getSource());
		hit.add("ParticleSystem", {
			particleSystem = "Par-BlueFirework-Burst",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(0.30000001, "onBurst2");
	}

	function onBurst2( ... )
	{
		this.get("Hit").finish();
		local stream = this.createGroup("Stream", this.getSource());
		stream.add("ParticleSystem", {
			particleSystem = "Par-BlueFirework_Burst3",
			emitterPoint = "spell_target"
		});
		this.fireIn(this.mDuration, "onDone");
	}

}

class this.EffectDef.LightRain extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LightRain";

	function onStart( ... )
	{
		this.mMaxTime = -1;
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Rain1",
			emitterPoint = "head_particles"
		});
	}

}

class this.EffectDef.MediumRain extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MediumRain";
	
	function onStart( ... )
	{
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Rain2",
			emitterPoint = "head_particles"
		});
	}

}

class this.EffectDef.HeavyRain extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HeavyRain";
	
	function onStart( ... )
	{
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Rain3",
			emitterPoint = "head_particles"
		});
	}

}

class this.EffectDef.LightSnow extends this.EffectDef.TemplateBasic
{
	static mEffectName = "LightSnow";

	function onStart( ... )
	{
		this.mMaxTime = -1;
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Snow1",
			emitterPoint = "head_particles"
		});
	}

}

class this.EffectDef.MediumSnow extends this.EffectDef.TemplateBasic
{
	static mEffectName = "MediumSnow";
	
	function onStart( ... )
	{
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Snow2",
			emitterPoint = "head_particles"
		});
	}

}

class this.EffectDef.HeavySnow extends this.EffectDef.TemplateBasic
{
	static mEffectName = "HeavySnow";
	
	function onStart( ... )
	{
		local rain = this.createGroup("Weather", this.getSource());
		rain.add("ParticleSystem", {
			particleSystem = "Par-Weather_Snow3",
			emitterPoint = "head_particles"
		});
	}

}
