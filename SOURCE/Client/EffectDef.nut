this.require("Effects");

if (!("EffectDef" in this.getroottable()))
{
	this.EffectDef <- {};
}

this.log.debug("Loading EffectDefs...");
class this.EffectDef.TemplateBasic extends this.EffectScript
{
	static mEffectName = "TemplateBasic";
	mSound = "";
	mLoopSound = "";
	mEffectsPackage = "";
	function debug( message )
	{
		this.fxLog.debug(this.mEffectName + ": " + message);
	}

	function _stopAnimation()
	{
		local ah = this.getSource().getAnimationHandler();

		if (ah)
		{
			ah.fullStop();
		}
	}

	function _checkCreature( so )
	{
		local assembler = so.getAssembler();

		if (!assembler)
		{
			return null;
		}

		local body = assembler.mBody;

		if (!body)
		{
			return null;
		}

		if (body.find("Biped-", 0) != null)
		{
			return "Biped";
		}

		if (this.type(body) == "string")
		{
			if (body in ::ModelDef)
			{
				return body;
			}
			else
			{
				return null;
			}
		}
	}

	function _filterBipedCreatures( so )
	{
		local body = this._checkCreature(so);

		if (!body)
		{
			return null;
		}

		if (body == "Biped")
		{
			return body;
		}

		if ("AnimationHandler" in ::ModelDef[body])
		{
			local attach = ::ModelDef[body].AnimationHandler;

			if (attach == "Biped")
			{
				return "Biped";
			}
			else
			{
				return body;
			}
		}
		else
		{
			return null;
		}
	}

	function _getEffectsPackage( so )
	{
		local body = this._checkCreature(so);

		if (!body)
		{
			return null;
		}

		if (body == "Biped")
		{
			return body;
		}

		if ("AnimationHandler" in ::ModelDef[body])
		{
			local attach = ::ModelDef[body].AnimationHandler;

			if (attach == "Biped")
			{
				return "Biped";
			}
			else
			{
				return body;
			}
		}
		else
		{
			if (body in ::ModelDef)
			{
				if ("EffectsPackage" in ::ModelDef[body])
				{
					return ::ModelDef[body].EffectsPackage;
				}
			}

			return null;
		}
	}

	function _sourceCheck()
	{
		if (!this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No source");
			}

			return false;
		}

		return true;
	}

	function addShaky( strength, duration, radius )
	{
		local source = this.getSource();

		if (source)
		{
			::_playTool.addShaky(source.getPosition(), strength, duration, radius);
		}
	}

	function _targetCheck()
	{
		if (!this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No source");
			}

			return false;
		}
		else if (!this.getTarget())
		{
			if (("gTestEffectGrabTarget" in this.getroottable()) && ::gTestEffectGrabTarget)
			{
				this.getNarrative().setTargets([
					this.getSource().getTargetObject()
				]);
			}
		}

		this.log.debug("Martin: Target - " + this.getTarget());

		if (!this.getTarget())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No Target");
			}

			return false;
		}

		return true;
	}

	function _secondaryTargetCheck()
	{
		if (!this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No source");
			}

			return false;
		}
		else if (!this.getSecondaryTargets())
		{
			if (("gTestEffectGrabTarget" in this.getroottable()) && ::gTestEffectGrabTarget)
			{
				this.getNarrative().setSecondaryTargets([
					this.getSource().getTargetObject()
				]);
			}
		}

		this.log.debug("Martin: Secondary Target - " + this.getSecondaryTargets());

		if (!this.getSecondaryTargets())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No Secondary Targets");
			}

			return false;
		}

		return true;
	}

	function _positionalCheck()
	{
		if (!this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No source");
			}

			return false;
		}
		else if (!this.getPositionalTarget())
		{
			if (("gTestEffectGrabTarget" in this.getroottable()) && ::gTestEffectGrabTarget)
			{
				this.getNarrative().mPosTarget = this.getSource().getTargetObject().getNode().getWorldPosition();
			}
		}

		if (this.gLogEffects)
		{
			this.log.debug("Martin: " + this.getNarrative().mPosTarget);
		}

		if (!this.getPositionalTarget())
		{
			if (this.gLogEffects)
			{
				this.log.debug(this.mEffectName + ": No Positional Target");
			}

			return false;
		}

		return true;
	}

	function _corkTargets( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getTargets();

		if (targets)
		{
			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.cork();
					}
				}
				else if (t)
				{
					t.cork();
				}
			}
		}
	}

	function _corkSecondary( ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getSecondaryTargets();

		if (targets)
		{
			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.cork();
					}
				}
				else if (t)
				{
					t.cork();
				}
			}
		}
	}

	function _uncorkTargets( ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getTargets();

		if (targets)
		{
			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.uncork();
					}
				}
				else if (t)
				{
					t.uncork();
				}
			}
		}
	}

	function _uncorkSecondary( ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getSecondaryTargets();

		if (targets)
		{
			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.uncork();
					}
				}
				else if (t)
				{
					t.uncork();
				}
			}
		}
	}

	function _cueTargets( hit, ... )
	{
		if (!this._targetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getTargets();

		if (targets)
		{
			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.cue(hit, t);
					}
				}
				else if (t)
				{
					t.cue(hit, t);
				}
			}
		}
	}

	function _cueSecondary( hit, ... )
	{
		if (!this._secondaryTargetCheck())
		{
			return;
		}

		local excludeSource = false;

		if (vargc > 0)
		{
			excludeSource = vargv[0];
		}

		local targets = this.getSecondaryTargets();

		if (targets)
		{
			if (this.gLogEffects)
			{
				this.log.debug("Martin - Number of secondary targets to process: " + targets.len());
			}

			foreach( t in targets )
			{
				if (excludeSource)
				{
					if (t && t != this.getSource())
					{
						t.cue(hit, t);
					}
				}
				else if (t)
				{
					t.cue(hit, t);
				}
			}
		}
		else if (this.gLogEffects)
		{
			this.log.debug("Martin - No secondary targets found");
		}
	}

	function onSound( ... )
	{
		if (this.mSound == "")
		{
			return;
		}

		local soundName = "Sound";

		if (vargc > 0)
		{
			soundName = vargv[0];
		}

		local sound = this.createGroup(soundName, this.getSource());
		sound.add("Sound", {
			sound = this.mSound
		});
	}

	function onLoopSound()
	{
		if (this.mLoopSound == "")
		{
			return;
		}

		local sound = this.createGroup("LoopSound", this.getSource());
		sound.add("Sound", {
			sound = this.mLoopSound
		});
	}

	function onLoopSoundStop()
	{
		if (this.mLoopSound == "")
		{
			return;
		}

		try
		{
			this.get("LoopSound").destroy();
		}
		catch( err )
		{
		}
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.TemplateMultiProjectile extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TemplateMultiProjectile";
	mProjs = null;
	mCount = 0;
	function _distanceCheck( target, projGroupName )
	{
		local proj;

		try
		{
			proj = this.get(projGroupName);
		}
		catch( err )
		{
			return null;
		}

		local targetPos = this._resolvePos(target, "spell_target", null);
		local projPos = proj.getObject().getNode().getPosition();
		return ::Math.DetermineDistanceBetweenTwoPoints(projPos, targetPos);
	}

	function _resolvePos( so, point, bone )
	{
		if (point)
		{
			local ap = so.getAttachPointDef(point);

			if (ap == null || !("bone" in ap))
			{
				return this.Util.getBoneWorldPosition(so, bone);
			}

			return this.Util.getBoneWorldPosition(so, ap.bone, ap);
		}

		return this.Util.getBoneWorldPosition(so, bone);
	}

}

class this.EffectDef.TemplateMelee extends this.EffectDef.TemplateBasic
{
	static mEffectName = "TemplateMelee";
	mWeapon = "";
	function _checkMelee()
	{
		local weaponGroups = this.getSource().getHandAttachments();
		local result = "";

		foreach( a in weaponGroups )
		{
			local wt = a.getWeaponType();

			if (wt == "1h")
			{
				if (result != "")
				{
					result = "Dual";
				}
				else
				{
					result = "1h";
				}
			}
			else if (wt == "2h")
			{
				return "2h";
			}
			else if (wt == "Small")
			{
				if (result != "")
				{
					result = "Dual";
				}
				else
				{
					result = "Small";
				}
			}
			else if (wt == "Staff")
			{
				return "Pole";
			}
		}

		return result;
	}

	function _checkSmallType()
	{
		local weaponGroups = this.getSource().getHandAttachments();
		local result = "";

		foreach( a in weaponGroups )
		{
			local wt = a.getMeshName();

			if (wt.find("-Dagger", 0) != null)
			{
				return "Dagger";
			}

			if (wt.find("-Katar", 0) != null)
			{
				return "Katar";
			}

			if (wt.find("-Claw", 0) != null)
			{
				return "Claw";
			}
		}

		return result;
	}

	function _checkWeaponSoundType()
	{
		local weaponGroups = this.getSource().getHandAttachments();
		local result = "Blunt";
		local meleeWeapon = this._checkMelee();

		if (meleeWeapon == "Small" || meleeWeapon == "Dual")
		{
			return "Sharp";
		}

		foreach( a in weaponGroups )
		{
			local wt = a.getMeshName();

			if (wt.find("Sword", 0) != null || wt.find("Axe", 0) != null)
			{
				return "Sharp";
			}
		}

		return result;
	}

	function _cueImpactSound( so )
	{
		if (!so)
		{
			return;
		}

		if (this._checkWeaponSoundType() == "Sharp")
		{
			so.cue("BladeImpact", so);
		}
		else
		{
			so.cue("BluntImpact", so);
		}
	}

	function _setFireInHit( so, anim, ... )
	{
		local useFinalHit = true;

		if (vargc > 0)
		{
			useFinalHit = vargv[0];
		}

		local speed = 1.0;

		if (vargc > 1)
		{
			speed = vargv[1];
		}

		local events = this._setHit(so, anim, useFinalHit);

		if (!events)
		{
			return false;
		}

		foreach( i, x in events )
		{
			this.fireIn(i / speed, x);
		}

		return true;
	}

	function _setHit( so, anim, ... )
	{
		local useFinalHit = true;

		if (vargc > 0)
		{
			useFinalHit = vargv[0];
		}

		local type = this._filterBipedCreatures(so);

		if (!type)
		{
			return false;
		}

		local impacts;

		if (type == "Biped")
		{
			if (!(anim in ::BipedAnimationDef.Animations))
			{
				return false;
			}

			if (!("Impacts" in ::BipedAnimationDef.Animations[anim]))
			{
				this.fireIn(0.40000001, "onHit");
				return false;
			}

			impacts = this.BipedAnimationDef.Animations[anim].Impacts;
		}
		else if (type in ::ModelDef)
		{
			if (!(anim in ::ModelDef[type].Animations))
			{
				return false;
			}

			if (!("Impacts" in ::ModelDef[type].Animations[anim]))
			{
				this.fireIn(0.40000001, "onHit");
				return false;
			}

			impacts = this.ModelDef[type].Animations[anim].Impacts;
		}
		else
		{
			return false;
		}

		local events = {};

		for( local i = 0; i < impacts.len(); i++ )
		{
			if (i == impacts.len() - 1 && useFinalHit)
			{
				events[impacts[i]] <- "onHitFinal";
			}
			else
			{
				events[impacts[i]] <- "onHit";
			}
		}

		return events;
	}

}

class this.EffectDef.TemplateWarmup extends this.EffectDef.TemplateMultiProjectile
{
	static mEffectName = "TemplateWarmup";
	mMaxWarmupTime = 10.0;
	function onAbilityWarmupComplete( ... )
	{
		this.debug("onAbilityWarmupComplete called");
		this.onDone();
	}

	function onAbilityCancel( ... )
	{
		this.debug("onAbilityCancel called");
		this._stopAnimation();
		this.onDone();
	}

	function onDone( ... )
	{
		this.debug("onDone called");
		this.fireIn(0.2, "onFinalDone");
	}

	function onFinalDone( ... )
	{
		this.debug("onFinalDone called");
		this.onLoopSoundStop();
		this.finish();
	}

}

class this.EffectDef.CreatureFocus_Death extends this.EffectScript
{
	static mEffectName = "CreatureFocus_Death";
	function onStart( ... )
	{
		local hands = this.createGroup("DeathEffect", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Death_Hand2",
			emitterPoint = "casting"
		});
		hands.add("ParticleSystem", {
			particleSystem = "Par-Death_Hand_Smoke3",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.CreatureFocus_Mystic extends this.EffectScript
{
	static mEffectName = "CreatureFocus_Mystic";
	function onStart( ... )
	{
		local hands = this.createGroup("MysticEffect", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Mystic_Hand",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.CreatureFocus_Nature extends this.EffectScript
{
	static mEffectName = "CreatureFocus_Nature";
	function onStart( ... )
	{
		local hands = this.createGroup("NatureEffect", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Nature_Hand4",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.CreatureFocus_Ice extends this.EffectScript
{
	static mEffectName = "CreatureFocus_Ice";
	function onStart( ... )
	{
		local hands = this.createGroup("HandFrost", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand3",
			emitterPoint = "casting"
		});
		hands.add("ParticleSystem", {
			particleSystem = "Par-Ice_Hand2-Snow",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.CreatureFocus_Fire extends this.EffectScript
{
	static mEffectName = "CreatureFocus_Fire";
	function onStart( ... )
	{
		local hands = this.createGroup("HandFlame", this.getSource());
		hands.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand4",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Melee extends this.EffectDef.TemplateMelee
{
	static mEffectName = "Melee";
	mHit = true;
	function _randomMelee()
	{
		local modelName = this._filterBipedCreatures(this.getSource());

		if (modelName == "Biped")
		{
			local weaponGroup = this._checkMelee();

			switch(weaponGroup)
			{
			case "2h":
				return this.Util.randomElement([
					"Two_Handed_Diagonal1",
					"Two_Handed_Diagonal2",
					"Two_Handed_Side_Cuts"
				]);
				break;

			case "Dual":
				return this.Util.randomElement([
					"Dual_Wield_Diagonal",
					"Dual_Wield_Flashy_Strike",
					"Dual_Wield_Side_Cuts"
				]);
				break;

			case "Pole":
				return this.Util.randomElement([
					"Staff_SideToSide",
					"Staff_Side_Jabs",
					"Staff_Underhand"
				]);
				break;

			case "1h":
			default:
				return this.Util.randomElement([
					"One_Handed_Diagonal_Strike",
					"One_Handed_Flashy",
					"One_Handed_Attack1"
				]);
				break;
			}
		}

		if ("BaseMeleeAnimations" in ::ModelDef[modelName])
		{
			return this.Util.randomElement(::ModelDef[modelName].BaseMeleeAnimations);
		}
		else
		{
			return this.Util.randomElement([
				"Attack1",
				"Attack2"
			]);
		}
	}

	function onStart( ... )
	{
		if (this.mHit)
		{
			if (!this._targetCheck())
			{
				return;
			}
		}
		else if (!this._sourceCheck())
		{
			return;
		}

		local anim = this.createGroup("Animation", this.getSource());
		local randAnim = this._randomMelee();
		anim.add("FFAnimation", {
			animation = randAnim,
			events = this._setHit(this.getSource(), randAnim)
		});
		this.fireIn(10.0, "onDone");
	}

	function onHit( ... )
	{
		if (this.getTarget() != null)
		{
			if (this.mHit)
			{
				this.getTarget().cue("Temp_GetHit", this.getTarget());
				this._cueImpactSound(this.getTarget());
			}
		}
	}

	function onHitFinal( ... )
	{
		this.onHit();
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.MeleeMiss extends this.EffectDef.Melee
{
	static mEffectName = "MeleeMiss";
	mHit = false;
}

class this.EffectDef.BluntImpact extends this.EffectDef.TemplateBasic
{
	static mEffectName = "BluntImpact";
	mSound = "Blunt";
	mTotalBluntSounds = 20;
	mTotalBladeSounds = 10;
	function randomSound( ... )
	{
		if (this.mSound == "Sharp")
		{
			local bladeSounds = [];

			for( local i = 1; i <= this.mTotalBladeSounds; i++ )
			{
				bladeSounds.append("Sound-Impact-Blade" + i + ".ogg");
			}

			return this.Util.randomElement(bladeSounds);
		}
		else
		{
			local bluntSounds = [];

			for( local i = 1; i <= this.mTotalBluntSounds; i++ )
			{
				bluntSounds.append("Sound-Impact-Blunt" + i + ".ogg");
			}

			return this.Util.randomElement(bluntSounds);
		}
	}

	function onStart( ... )
	{
		if (!this._sourceCheck())
		{
			return null;
		}

		this.add("Dummy");
		local sound = this.createGroup("ImpactSound", this.getSource());
		local randImpact = this.randomSound();
		sound.add("Sound", {
			sound = randImpact
		});
		this.fireIn(0.5, "onDone");
	}

}

class this.EffectDef.BladeImpact extends this.EffectDef.BluntImpact
{
	static mEffectName = "BladeImpact";
	mSound = "Sharp";
}

class this.EffectDef.RangedAttack extends this.EffectDef.TemplateWarmup
{
	static mEffectName = "RangedAttack";
	mWeaponGroup = "";
	mAttachment = null;
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
	}

	function randomSound( soundPrefix, soundCount, soundSuffix )
	{
		local ranNumber = this.Util.randomRange(0.5, soundCount);
		ranNumber = (ranNumber + 0.5).tointeger();
		local fileName = soundPrefix + ranNumber + soundSuffix;
		return fileName;
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

		this.add("Dummy");
		local sound = this.createGroup("ImpactSound", this.getSource());
		local soundFileName = this.randomSound("Sound-Ability-Bow_Autoattack", 4, "_Cast.ogg");
		sound.add("Sound", {
			sound = soundFileName
		});
		local anim = this.createGroup("Animation", this.getSource());
		anim.add("FFAnimation", {
			animation = "Bow_Attack_Release_v2",
			speed = 2.5,
			events = {
				[0.30000001] = "onAddArrow",
				[0.69999999] = "onBowProjectile"
			}
		});
		this.fireIn(10.0, "onDone");
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

		this.add("Dummy");
		local sound = this.createGroup("ImpactSound", this.getSource());
		local soundFileName = this.randomSound("Sound-Ability-Wand_Autoattack", 4, "_Cast.ogg");
		sound.add("Sound", {
			sound = soundFileName
		});
		local anim = this.createGroup("Animation", this.getSource());
		anim.add("FFAnimation", {
			animation = "Magic_Wand_Point",
			events = {
				[0.5] = "onWandProjectile"
			}
		});
		local ra = this.createGroup("RangedAttack", this.getSource());
		ra.add("ParticleSystem", {
			particleSystem = "Par-Wand_Charge2",
			emitterPoint = "right_hand"
		});
		this.fireIn(10.0, "onDone");
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
			particleSystem = "Par-Wand_Aura2",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -0.80000001,
			width = 1.6,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ddbeff",
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
			animation = "Sneak_Throw4",
			events = {
				[0.5] = "onThrownProjectile"
			}
		});
		this.fireIn(10.0, "onDone");
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

		this.getSource().unHideHandAttachments();

		if (this.mWeaponGroup == "1h")
		{
			this.get("Projectile").getObject().getNode().removeChild(this.get("Mesh").getObject().getNode());
			this.get("Mesh").finish();
		}

		this.get("Projectile").finish();
		local core = this.createGroup("GetHit", this.getTarget());
		core.add("FFAnimation", {
			animation = "$HIT$"
		});

		if (this.mWeaponGroup == "Wand")
		{
			core.add("ParticleSystem", {
				particleSystem = "Par-Wand_Burst2",
				emitterPoint = "spell_target"
			});
		}

		this.getTarget().uncork();
		this.fireIn(0.1, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.FireStorm extends this.EffectScript
{
	static mEffectName = "FrostStorm";
	mCount = 0;
	mTargets = null;
	mMainTargets = null;
	mInitialCount = 0;
	mMainCount = 0;
	mY = 200.0;
	mRandom = null;
	function _randLocation()
	{
		local randNumber = this.mRandom.nextInt(400);
		local x = randNumber - 200.0;
		randNumber = this.mRandom.nextInt(400);
		local z = randNumber - 200.0;
		return this.Vector3(x, this.mY, z);
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
			local y = this.Util.getFloorHeightAt(pos, 0.5, this.QueryFlags.KEEP_ON_FLOOR, true);
			pos = this.Vector3(pos.x, y.pos.y, pos.z);
		}

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

		this.mMainTargets = [];

		if (this.getTarget())
		{
			this.getTarget().cork();
		}

		this.mMainTargets.append(this.getTarget());
		local secondaries = this.getSecondaryTargets();

		if (secondaries)
		{
			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = [
						s
					];
					s_target.cork();
					this.mMainTargets.append(s_target);
				}
			}
		}

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("FFAnimation", {
			animation = "Victory"
		});
		local fire = this.createGroup("HandFire", this.getSource());
		fire.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.40000001, "onStormCloud");
		this.fireIn(7.4000001, "onDone");
		this.mRandom = this.Random();
	}

	function onStormCloud( ... )
	{
		this.get("HandFire").finish();
		local cloud = this.createGroup("Cloud");
		this._detachGroup(cloud, this.getTarget().getNode().getPosition());
		cloud.add("ParticleSystem", {
			particleSystem = "Par-Frost_Storm_Cloud2",
			emitterPoint = "node"
		});

		for( local time = 0.40000001; time < 5.0; time = time + 0.2 )
		{
			this.fireIn(time, "onProjectile");
		}
	}

	function onProjectile( ... )
	{
		local topTarget = this._createRandomTarget("topTarget" + this.mCount, "Cloud");
		topTarget = topTarget;
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
			particleSystem = "Par-Fire_Ball",
			emitterPoint = "node"
		});
		proj.add("ParticleSystem", {
			particleSystem = "Par-Fire_Embers",
			emitterPoint = "node"
		});
		proj.add("Ribbon", {
			offset = -3.0,
			width = 6.0,
			height = 5.0,
			maxSegments = 32,
			initialColor = "ffb400",
			colorChange = [
				2.0,
				2.0,
				2.0,
				2.0
			]
		});
		proj.add("Spin", {
			speed = 3.0,
			accel = 0.0,
			axis = this.Vector3(0.0, 0.0, 1.0)
		});
		proj.add("ScaleTo", {
			size = 2.0,
			maintain = true,
			duration = 0.0
		});

		if (this.mTargets == null)
		{
			this.mTargets = [];
		}

		this.mTargets.append(this.mCount);
		this.mCount++;

		if (this.mInitialCount < 5 && this.mMainCount < this.mMainTargets.len())
		{
			this.onProjectile();
		}
	}

	function onContact( ... )
	{
		for( local i = this.mTargets.len() - 1; i > -1; i-- )
		{
			local bottomGroupName = "bottomTarget" + this.mTargets[i];
			local projGroupName = "projectile" + this.mTargets[i];
			local distance = this._distanceCheck(bottomGroupName, projGroupName);

			if (distance == 0)
			{
				foreach( k in this.mMainTargets )
				{
					local botTarget = this.get(bottomGroupName).getObject();

					if (botTarget == k)
					{
						botTarget.cue("CombustionHit", botTarget);
					}
				}

				this.get("topTarget" + this.mTargets[i]).finish();
				this.get("bottomTarget" + this.mTargets[i]).finish();
				this.get("projectile" + this.mTargets[i]).finish();
				this.mTargets.remove(i);
			}
		}
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Wildfire extends this.EffectScript
{
	static mEffectName = "Wildfire";
	mCount = 0;
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

		local hands = this.createGroup("QuickCast", this.getSource());
		hands.add("FFAnimation", {
			animation = "Magic_Double_Handed"
		});
		local fire = this.createGroup("HandFire", this.getSource());
		fire.add("ParticleSystem", {
			particleSystem = "Par-Fire_Cast",
			emitterPoint = "casting"
		});
		this.fireIn(0.55000001, "onStopCastFire");
		this.fireIn(0.5, "onContact");
		this.getTarget().cork();
	}

	function onStopCastFire( ... )
	{
		this.get("HandFire").finish();
	}

	function _projectile( source, target, sourcePoint )
	{
		local proj = this.createGroup("projectile" + this.mCount);
		proj.add("ArcToTarget", {
			source = source,
			sourcePoint = sourcePoint,
			target = target,
			targetPoint = "spell_target",
			intVelocity = 6.0,
			accel = 5.0,
			topSpeed = 100.0,
			orient = true,
			arcEnd = 0.5,
			arcSideAngle = 0.0,
			arcForwardAngle = 60.0
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
		proj.add("ScaleTo", {
			size = 0.40000001,
			maintain = true,
			duration = 0.0
		});
	}

	function onProjectile( ... )
	{
		local source = this.getSource();
		local target = this.getTarget();

		switch(this.mCount)
		{
		case 0:
			break;

		case 1:
			this._projectile(target, source, "spell_target");
			this.get("GetHitMagical" + 0).finish();
			break;

		case 2:
			this._projectile(source, target, "spell_target");
			this.get("GetHitMagical" + 1).finish();
			break;

		case 3:
			this._projectile(target, source, "spell_target");
			this.get("GetHitMagical" + 2).finish();
			break;
		}
	}

	function _contact( target )
	{
		local core = this.createGroup("GetHitMagical" + this.mCount, target);
		core.add("FFAnimation", {
			animation = "$HIT$"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-Fire_Combustion_Ring",
			emitterPoint = "node"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-Fire_Spark_Burst",
			emitterPoint = "spell_target"
		});
	}

	function onContact( ... )
	{
		if (this.mCount != 0)
		{
			this.get("projectile" + this.mCount).finish();
		}

		if (this.mCount == 0 || this.mCount == 2)
		{
			this._contact(this.getTarget());
			this.getTarget().uncork();
			this.mCount++;
			this.fireIn(5.0, "onProjectile");
		}
		else if (this.mCount == 1)
		{
			this._contact(this.getSource());
			this.getSource().uncork();
			this.mCount++;
			this.fireIn(5.0, "onProjectile");
		}
		else if (this.mCount == 3)
		{
			this._contact(this.getSource());
			this.getSource().uncork();
			this.mCount++;
			this.fireIn(5.0, "onDone");
		}
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Thorns extends this.EffectScript
{
	static mEffectName = "Thorns";
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

		local anim = this.createGroup("QuickCast", this.getSource());
		anim.add("FFAnimation", {
			animation = "Victory"
		});
		local particles = this.createGroup("Particles", this.getSource());
		particles.add("ParticleSystem", {
			particleSystem = "Par-Thorns_Cast_Fill",
			emitterPoint = "casting"
		});
		particles.add("ParticleSystem", {
			particleSystem = "Par-Thorns_Cast_Leaves",
			emitterPoint = "casting"
		});
		this.fireIn(0.44999999, "onFinishCast");
	}

	function onFinishCast( ... )
	{
		this.get("Particles").finish();
		local hand = this.createGroup("Sparks", this.getSource());
		hand.add("ParticleSystem", {
			particleSystem = "Par-Thorns_Burst2",
			emitterPoint = "right_hand"
		});
		this.getTarget().cue("ThornsTarget", this.getTarget());
		this.fireIn(0.30000001, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.ThornsTarget extends this.EffectScript
{
	static mEffectName = "ThornsTarget";
	mSound = "Sound-Ability-Thorns_Effect.ogg";
	function onStart( ... )
	{
		local glow = this.createGroup("Glow", this.getSource());
		glow.add("ParticleSystem", {
			particleSystem = "Par-Thorns_Glow",
			emitterPoint = "node"
		});
		glow.add("ParticleSystem", {
			particleSystem = "Par-Thorns_Burst",
			emitterPoint = "spell_target"
		});
		local core = this.createGroup("GetHitMagical", this.getSource());
		core.add("ParticleSystem", {
			particleSystem = "Par-Thorns",
			emitterPoint = "spell_target"
		});
		core.add("ParticleSystem", {
			particleSystem = "Par-Thorns2",
			emitterPoint = "spell_target"
		});
		this.onSound();
		this.fireIn(1.0, "onPartialStop");
		this.fireIn(2.5, "onDone");
	}

	function onPartialStop( ... )
	{
		this.get("Glow").finish();
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.FireSpellCastWarmup extends this.EffectScript
{
	static mEffectName = "FireSpellCastWarmup";
	function onStart( ... )
	{
		local g = this.createGroup("glowyHands", this.getSource());
		g.add("FFAnimation", {
			animation = "Victory"
		});
		this.fireIn(0.60000002, "onDone");
		g.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand",
			emitterPoint = "casting"
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.FireballToTarget extends this.EffectScript
{
	static mEffectName = "FireballToTarget";
	function onStart( ... )
	{
		if (!this.getTarget())
		{
			return;
		}

		local secondaries = this.getSecondaryTargets();

		if (secondaries)
		{
			foreach( s in secondaries )
			{
				if (s)
				{
					local s_target = [
						s
					];
					s.cue("FlameMail1", s_target);
				}
			}
		}

		this.getTarget().cork();
		local fbg = this.createGroup("fireball");
		fbg.add("MoveToTarget", {
			source = this.getSource(),
			sourceBone = "Bone-RightHand",
			target = this.getTarget(),
			targetBone = "Bone-Back"
		});
		fbg.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand"
		});
		fbg.add("Mesh", {
			mesh = "Item-Fireball.mesh"
		});
	}

	function onContact( ... )
	{
		this.getTarget().uncork();
		this.finish();
	}

}

class this.EffectDef.RedImpactExplosion extends this.EffectScript
{
	static mEffectName = "RedImpactExplosion";
	function onStart( ... )
	{
		local target = this.getTarget();

		if (target)
		{
			local impact = this.createGroup("impact", target);
			impact.add("ParticleSystem", {
				particleSystem = "Par-Fire_Explosion1",
				emitterPoint = "chest"
			});
			impact.add("ParticleSystem", {
				particleSystem = "Par-Fire_Hand"
			});
			impact.add("DiffusePulse", {
				color1 = "ff0000",
				color2 = "000000",
				rate = 2.0,
				inherit = true
			});
			impact.add("AmbientPulse", {
				color1 = "ff0000",
				color2 = "000000",
				rate = 2.0,
				inherit = true
			});
		}

		this.fireIn(15.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Poof extends this.EffectScript
{
	static mEffectName = "Poof";
	function onStart( ... )
	{
		local group = this.createGroup("poof", this.getSource());
		group.add("ParticleSystem", {
			particleSystem = "Par-Poof"
		});
		this.fireIn(1.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Fireball extends this.EffectScript
{
	static mEffectName = "FireBall";
	function onStart( ... )
	{
		if (!this.getSource())
		{
			this.stop();
			return;
		}

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

		local g = this.createGroup("glowyHands", this.getSource());
		g.add("FFAnimation", {
			animation = "Magic_Spell_Channel",
			speed = 0.5
		});
		this.fireIn(2.0, "onFireBall");
		g.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand2",
			emitterPoint = "casting"
		});
		g.add("Sound", {
			sound = "fire.ogg"
		});
	}

	function onFireBall( ... )
	{
		if (!this.getTarget() || !this.getSource())
		{
			if (this.gLogEffects)
			{
				this.log.debug("FX Fireball has no target! Stopping.");
			}

			this.stop();
			return;
		}

		local ghg = this.get("glowyHands");
		ghg.stop();
		local fbg = this.createGroup("fireball");
		fbg.add("MoveToTarget", {
			source = this.getSource(),
			sourceBone = "Bone-LeftHand",
			target = this.getTarget(),
			targetBone = "Bone-Back"
		});
		fbg.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand"
		});
		fbg.add("Mesh", {
			mesh = "Item-Fireball.mesh"
		});
		fbg.add("Spin", {
			speed = 0.44999999,
			accel = 1.5
		});
		local fcst = this.createGroup("firecast", this.getSource());
		fcst.add("FFAnimation", {
			animation = "Magic_Throw",
			speed = 2.0
		});
		fcst.add("Sound", {
			sound = "flame_fire.ogg"
		});
	}

	function onContact( ... )
	{
		if (!this.getTarget())
		{
			this.stop();
			return;
		}

		local fbg = this.get("fireball");
		fbg.stop();
		local impact = this.createGroup("impact", this.getTarget());
		impact.add("FFAnimation", {
			animation = "Hit_Dual_Wield"
		});
		impact.add("Sound", {
			sound = "small_explosion.ogg"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Explosion2",
			emitterPoint = "chest"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Hand"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Ring2",
			emitterPoint = "chest"
		});
		impact.add("ParticleSystem", {
			particleSystem = "Par-Fire_Burst",
			emitterPoint = "chest"
		});
		impact.add("DiffusePulse", {
			color1 = "ff0000",
			color2 = "000000",
			rate = 2.0,
			inherit = true
		});
		impact.add("AmbientPulse", {
			color1 = "ff0000",
			color2 = "000000",
			rate = 2.0,
			inherit = true
		});
		this.fireIn(15.0, "onDone");
	}

	function onDone( ... )
	{
		this.finish();
	}

}

class this.EffectDef.Fireball_Death extends this.EffectScript
{
	static mEffectName = "Fireball_Death";
	function onStart( ... )
	{
		local Thanim = this.createGroup("Fb_hit", this.getSource());
		this.Fb_hit.add("FFAnimation", {
			animation = "Hit_Dual_Wield",
			events = {
				[1.0] = "onDie"
			}
		});
	}

	function onDie( ... )
	{
		local Thanim = this.createGroup("Fb_die", this.getSource());
		this.Fb_die.add("FFAnimation", {
			animation = "Hit_Death_Backward",
			events = {
				[1.0] = "onDone"
			}
		});
	}

	function onDone( ... )
	{
		this.finish();
	}

}

