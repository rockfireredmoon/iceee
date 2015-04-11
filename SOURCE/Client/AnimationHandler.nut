this.gNextEmitterID <- 0;
class this.AnimationHandler extends this.MessageBroadcaster
{
	mCurrentAnim = "";
	mSceneObject = null;
	mEntity = null;
	mAnimationStates = [];
	mStepSoundRecords = null;
	mStepEmitter = null;
	mStepSounds = [
		"Sound-Biped-Walk1.ogg",
		"Sound-Biped-Walk2.ogg",
		"Sound-Biped-Walk3.ogg",
		"Sound-Biped-Walk4.ogg",
		"Sound-Biped-Walk5.ogg",
		"Sound-Biped-Walk6.ogg",
		"Sound-Biped-Walk7.ogg",
		"Sound-Biped-Walk8.ogg"
	];
	mRunSounds = [
		"Sound-Biped-Run01.ogg",
		"Sound-Biped-Run02.ogg",
		"Sound-Biped-Run03.ogg",
		"Sound-Biped-Run04.ogg",
		"Sound-Biped-Run05.ogg",
		"Sound-Biped-Run06.ogg",
		"Sound-Biped-Run07.ogg",
		"Sound-Biped-Run08.ogg",
		"Sound-Biped-Run09.ogg",
		"Sound-Biped-Run10.ogg",
		"Sound-Biped-Run11.ogg",
		"Sound-Biped-Run12.ogg",
		"Sound-Biped-Run13.ogg",
		"Sound-Biped-Run14.ogg",
		"Sound-Biped-Run15.ogg",
		"Sound-Biped-Run16.ogg",
		"Sound-Biped-Run17.ogg",
		"Sound-Biped-Run18.ogg"
	];
	mJumping = false;
	mSwimming = false;
	mMovementType = 0;
	mMoving = false;
	mDead = false;
	mPaused = false;
	mCombat = false;
	mLastSoundTimePos = 0;
	mLastSoundAnimName = null;
	mStepEffect = null;
	constructor( pSceneObject )
	{
		::MessageBroadcaster.constructor();
		this.mSceneObject = pSceneObject;
		this.mStepSoundRecords = null;
		this.mStepEmitter = this._scene.createSoundEmitter("_Step_Emitter_" + this.gNextEmitterID++);
		this.mStepEmitter.setAmbient(true);
		this.mSceneObject.getNode().attachObject(this.mStepEmitter);
		local def = this.mSceneObject.getDef();

		if (!("AnimationHandler" in def))
		{
		}

		def.AnimationHandler <- {};
		this.mEntity = this.mSceneObject.getEntity();
		this.mAnimationStates = this.mEntity.getAnimationStates();
		this.mEntity.setAnimationUnitCount(1);
		this.mEntity.getAnimationUnit(0).setIdleState("Idle");
		this.mEntity.getAnimationUnit(0).setEnabled(true);
	}

	function getStepAnimationDef()
	{
		return this.getAnimationDef()[this.mCurrentAnim];
	}

	function getStepAnimationUnit()
	{
		return this._getPrimaryAU();
	}

	function setSwimming( swimming )
	{
		this.mSwimming = swimming;
	}

	function setCombat( value )
	{
		this.mCombat = value;
	}

	function reset()
	{
	}

	function forceAnimUpdate()
	{
	}

	function _updateStepSound()
	{
		if (this.mMoving == true)
		{
			local animDef = this.getStepAnimationDef();
			local stepAnim = this.getStepAnimationUnit();

			if (animDef == null || stepAnim == null)
			{
				return;
			}

			if ("StepSounds" in animDef)
			{
				if ("Effect" in animDef)
				{
					this.mStepEffect = animDef.Effect;
				}
				else
				{
					this.mStepEffect = null;
				}

				local currentTime = stepAnim.getTimePosition();
				local events = animDef.StepSounds;

				if (this.mStepSoundRecords == null)
				{
					this.mStepSoundRecords = [];

					for( local t = 0; t < events.len(); t++ )
					{
						this.mStepSoundRecords.append(false);
					}
				}

				for( local x = 0; x < events.len(); x++ )
				{
					local interval = events[x];

					if (currentTime >= interval && this.mStepSoundRecords[x] == true)
					{
						this.mStepEmitter.stop();

						if (this.mStepEffect == null)
						{
							this.mStepEmitter.setSound(this.Util.randomElement(this.mSpeed > 60 ? this.mRunSounds : this.mStepSounds));
							this.mStepEmitter.play();
						}
						else
						{
							this.mSceneObject.cue(this.mStepEffect);
						}

						this.mStepSoundRecords[x] = false;
					}
					else if (currentTime < interval)
					{
						this.mStepSoundRecords[x] = true;
					}
				}
			}
		}
	}

	function getCurrentAnim()
	{
		return this.mCurrentAnim;
	}

	function _getPrimaryAU()
	{
		return this.mEntity ? this.mEntity.getAnimationUnit(0) : null;
	}

	function getClassName()
	{
		return null;
	}

	function getCompatible( other )
	{
		return false;
	}

	function getCurrentAnimLength()
	{
		local au = this._getPrimaryAU();

		if (!au)
		{
			return null;
		}

		if (au.isBlending())
		{
			return au.getTargetLength();
		}

		return au.getLength();
	}

	function getCurrentAnimTimePosition()
	{
		local au = this._getPrimaryAU();

		if (!au)
		{
			return null;
		}

		if (au.isBlending())
		{
			return au.getTargetTimePosition();
		}

		return au.getTimePosition();
	}

	function getCurrentAnimTimeScale()
	{
		local au = this._getPrimaryAU();

		if (!au)
		{
			return null;
		}

		return au.getTimeScaleFactor();
	}

	function _getCustomState( vState )
	{
	}

	function onDeath()
	{
		this.mDead = true;
	}

	function onMove( pSpeed )
	{
		this.mMoving = true;
	}

	function onStop()
	{
		this.mStepSoundRecords = null;
		this.mMoving = false;
	}

	function getAnimationState()
	{
		local count = this.mEntity.getAnimationUnitCount();
		local ret = [];

		if (this.mEntity)
		{
			for( local i = 0; i < count; i++ )
			{
				local au = this.mEntity.getAnimationUnit(i);
				local unit = {
					current = au.getCurrentAnimName(),
					target = au.getTargetAnimName(),
					currentpos = au.getTimePosition(),
					targetpos = au.getTargetTimePosition()
				};
				ret.append(unit);
			}
		}

		local table = {
			states = ret,
			className = this.getClassName()
		};
		this._getCustomState(table);
		return table;
	}

	function translateAnim( anim )
	{
		return anim;
	}

	function setIdleState( state )
	{
		if (this.mEntity)
		{
			local count = this.mEntity.getAnimationUnitCount();

			for( local i = 0; i < count; i++ )
			{
				local au = this.mEntity.getAnimationUnit(i);
				local anim = this.translateAnim(state);
				au.setIdleState(anim);
			}
		}
	}

	function _setCustomState( vState )
	{
		return true;
	}

	function setAnimationState( vState )
	{
		if (this.mEntity && vState.className == this.getClassName())
		{
			if (this._setCustomState(vState) == false)
			{
				return;
			}

			local states = vState.states;
			this.mEntity.setAnimationUnitCount(states.len());

			foreach( i, unit in states )
			{
				local au = this.mEntity.getAnimationUnit(i);
				au.setIdleState("Idle");
				au.setEnabled(true);
				au.animationSwitch(unit.current, true);
				au.setTimePosition(unit.currentpos);
			}
		}
	}

	function destroy()
	{
		if (this.mStepEmitter)
		{
			this.mStepEmitter.destroy();
			this.mStepEmitter = null;
		}
	}

	function _playSound( sound )
	{
		this.mSceneObject.playSound(sound);
	}

	function _processSound()
	{
		if (this.mPaused)
		{
			return;
		}

		local animDef = this.getAnimationDef();
		local animName = this.getCurrentAnim();

		if (animName in animDef)
		{
			if ("Sounds" in animDef[animName])
			{
				local lastT = this.mLastSoundTimePos;

				if (this.mLastSoundAnimName != animName)
				{
					lastT = 0;
					this.mLastSoundAnimName = animName;
				}

				local au = this._getPrimaryAU();
				local t;

				if (au.isBlending())
				{
					t = au.getTargetTimePosition();
				}
				else
				{
					t = au.getTimePosition();
				}

				local tname = au.getCurrentAnimName();
				local soundList = animDef[animName].Sounds;

				foreach( at, sound in soundList )
				{
					if (at >= lastT && at < t || at >= lastT && t < lastT || at <= t && t < lastT)
					{
						this._playSound(sound);
					}
				}

				this.mLastSoundTimePos = t;
			}
		}
	}

}

class this.AnimationHandler.Horde extends this.AnimationHandler
{
	constructor( pSceneObject )
	{
		this.AnimationHandler.constructor(pSceneObject);
	}

	function getAnimationDef()
	{
		local model = this.mSceneObject.mAssembler.mBody;
		return ::ModelDef[model].Animations;
	}

	function getModelDef()
	{
		local model = this.mSceneObject.mAssembler.mBody;
		return ::ModelDef[model];
	}

	function getAmbientDef()
	{
		local model = this.mSceneObject.mAssembler.mBody;
		return ::ModelDef[model].Ambient;
	}

	function getMovementAnimation( pSpeed )
	{
		pSpeed = this.Math.min(200, pSpeed.tointeger());
		local model = this.mSceneObject.mAssembler.mBody;
		local forSet = ::ModelDef[model].Movement;

		foreach( i, x in forSet )
		{
			if (pSpeed >= x[0] && pSpeed <= x[2])
			{
				return {
					name = i,
					meanSpeed = x[1]
				};
			}
		}

		throw "No valid movement animation for speed " + pSpeed;
	}

	function getHandlerDef()
	{
		return this.mSceneObject.getDef().AnimationHandler;
	}

	function _startAnimation( pName, pSpeed, pBlend, pLoop )
	{
		if (this.mDead && pName != "Death")
		{
			return;
		}

		local animDef = this.getAnimationDef();

		if (!(pName in animDef))
		{
			throw pName + " not a valid animation for " + this.mSceneObject.getDef().Model;
		}

		this.mEntity.getAnimationUnit(0).setTimeScaleFactor(pSpeed);
		this.mEntity.getAnimationUnit(0).animationCrossBlend(pName, pBlend, true);
		this._calcPlayHead();
		this.mCurrentAnim = pName;
		this.mAnimationSpeed = pSpeed;
		this.mAnimationLoop = pLoop;

		if (this.mEntity.getAnimationUnit(0).isBlending())
		{
			if (pLoop)
			{
				this.mAnimationDuration = this.mEntity.getAnimationUnit(0).getTargetLength();
			}
			else
			{
				this.mAnimationDuration = this.mEntity.getAnimationUnit(0).getTargetLength() - pBlend;
			}
		}
		else if (pLoop)
		{
			this.mAnimationDuration = this.mEntity.getAnimationUnit(0).getLength();
		}
		else
		{
			this.mAnimationDuration = this.mEntity.getAnimationUnit(0).getLength() - pBlend;
		}
	}

	function setPause( pBool )
	{
		if (pBool)
		{
			this.mEntity.getAnimationUnit(0).setTimeScaleFactor(0.00001);
		}
		else
		{
			this.mEntity.getAnimationUnit(0).setTimeScaleFactor(this.mAnimationSpeed);
		}

		this.mPaused = pBool;
	}

	function _processAnimation()
	{
		local animDef = this.getAnimationDef();
		local def = this.mSceneObject.getDef();
		local size = this.mSceneObject.mAssembler.mSize;
		local adjustedSpeed = 0.0;
		local handlerDef = def.AnimationHandler;

		if (size > 1.0)
		{
			adjustedSpeed = (size - 1.0) * 0.25 + 1.0;
		}
		else
		{
			adjustedSpeed = size;
		}

		adjustedSpeed *= this.mFFSpeed;

		if ("Death" in handlerDef)
		{
			if (this.mCurrentAnim != "Death" && "Death" in animDef)
			{
				this._startAnimation("Death", 1.0, this.getBlend("Death"), false);
				this.mDead = true;
			}

			return;
		}

		if ("Movement" in handlerDef)
		{
			if ("FF" in handlerDef)
			{
				delete handlerDef.FF;
			}

			local moveAnim = this.getMovementAnimation(handlerDef.Movement / size);
			local speed = handlerDef.Movement / moveAnim.meanSpeed / size;

			if ((this.mCurrentAnim != moveAnim.name || speed != this.mAnimationSpeed) && moveAnim.name in animDef)
			{
				this._startAnimation(moveAnim.name, speed, this.getBlend(moveAnim.name), true);
			}

			return;
		}

		if (this.mPlayHeadTime >= this.mAnimationDuration - this._deltat / 1000.0 && this.mAnimationLoop == false)
		{
			if (this.mCurrentAnim != "")
			{
				if ("FF" in handlerDef)
				{
					delete handlerDef.FF;
				}

				this.mCurrentAnim = "";
			}
		}

		if ("FF" in handlerDef)
		{
			if (this.mCurrentAnim != handlerDef.FF && handlerDef.FF in animDef)
			{
				if (handlerDef.FF == "Run" || handlerDef.FF == "Jog" || handlerDef.FF == "Walk" || handlerDef.FF == "Idle")
				{
					this._startAnimation(handlerDef.FF, 1.0 / adjustedSpeed, this.getBlend(handlerDef.FF), this.mFFLoop);
				}
				else
				{
					this._startAnimation(handlerDef.FF, 1.0, this.getBlend(handlerDef.FF), this.mFFLoop);
				}
			}

			return;
		}

		if (this.mCurrentAnim != "Idle" && this.mCurrentAnim != "IdleCombat")
		{
			if (this.mCombat && "IdleCombat" in animDef)
			{
				this._startAnimation("IdleCombat", 1.0 / adjustedSpeed, this.getBlend("IdleCombat"), true);
				return;
			}

			if ("Idle" in animDef)
			{
				this._startAnimation("Idle", 1.0 / adjustedSpeed, this.getBlend("Idle"), true);
			}
		}
	}

	function getBlend( animName )
	{
		local animDef = this.getAnimationDef();
		local modelDef = this.getModelDef();

		if (animName in animDef)
		{
			if ("Blend" in animDef[animName])
			{
				return animDef[animName].Blend;
			}
		}

		if ("Blend" in modelDef)
		{
			return modelDef.Blend;
		}
		else
		{
			return 0.2;
		}
	}

	function translateAnim( anim )
	{
		if (typeof anim != "string")
		{
			return anim;
		}

		switch(anim)
		{
		case "$MELEE$":
			return this.Util.randomElement([
				"Attack1"
			]);
			break;

		case "$HIT$":
			return this.Util.randomElement([
				"Hit"
			]);
			break;
		}

		return anim;
	}

	function _checkDeathPause()
	{
		if (this.mCurrentAnim == "Death")
		{
			local stopPosition = 0.0;

			if (this.mEntity.getAnimationUnit(0).isBlending())
			{
				stopPosition = this.mEntity.getAnimationUnit(0).getTargetLength() - 0.1;
			}
			else
			{
				stopPosition = this.mEntity.getAnimationUnit(0).getLength() - 0.1;
			}

			if (this.mPaused)
			{
				if (this.mEntity.getAnimationUnit(0).isBlending())
				{
					this.mEntity.getAnimationUnit(0).setTargetTimePosition(stopPosition);
				}
				else
				{
					this.mEntity.getAnimationUnit(0).setTimePosition(stopPosition);
				}
			}
			else if (this.mPlayHeadTime >= stopPosition || this.mOldPlayHeadTime > this.mPlayHeadTime)
			{
				if (this.mEntity.getAnimationUnit(0).isBlending())
				{
					this.mEntity.getAnimationUnit(0).setTargetTimePosition(stopPosition);
				}
				else
				{
					this.mEntity.getAnimationUnit(0).setTimePosition(stopPosition);
				}

				this.setPause(true);
			}
		}
	}

	function _calcPlayHead()
	{
		this.mOldPlayHeadTime = this.mPlayHeadTime;
		local length = 0.0;

		if (this.mEntity.getAnimationUnit(0).isBlending())
		{
			this.mPlayHeadTime = this.mEntity.getAnimationUnit(0).getTargetTimePosition();
			length = this.mEntity.getAnimationUnit(0).getTargetLength();
		}
		else
		{
			this.mPlayHeadTime = this.mEntity.getAnimationUnit(0).getTimePosition();
			length = this.mEntity.getAnimationUnit(0).getLength();
		}

		if (this.mCurrentAnim != "")
		{
			if (this.mOldPlayHeadTime == length)
			{
				this.mOldPlayHeadTime = 0.0;
			}

			if (this.mOldPlayHeadTime > this.mPlayHeadTime && this.mPlayHeadTime != length)
			{
				this.mPlayHeadTime = length;
			}
		}
		else
		{
			this.mPlayHeadTime = 0.0;
			this.mOldPlayHeadTime = 0.0;
		}
	}

	function onEnterFrame()
	{
		this._calcPlayHead();

		if (this.mDead)
		{
			this._checkDeathPause();
		}
		else
		{
			this._processAnimation();
		}

		this._updateStepSound();
		this._processSound();
	}

	function onJump()
	{
		local handlerDef = this.getHandlerDef();
		handlerDef.FF <- "Jump";
		this._processAnimation();
	}

	function onFalling()
	{
	}

	function onLand()
	{
	}

	function onDeath()
	{
		this.mDead = true;
		local handlerDef = this.getHandlerDef();
		handlerDef.Death <- true;
		this._processAnimation();
	}

	function onRes()
	{
		local handlerDef = this.getHandlerDef();

		if (this.mDead)
		{
			if ("Death" in handlerDef)
			{
				delete handlerDef.Death;
			}

			this.mDead = false;
		}

		this.setPause(false);
		this._processAnimation();
	}

	function onFF( pAnimation, ... )
	{
		if (this.mDead == true)
		{
			return;
		}

		pAnimation = this.translateAnim(pAnimation);
		local handlerDef = this.getHandlerDef();
		handlerDef.FF <- pAnimation;
		this.mFFSpeed = vargc > 0 ? vargv[0] : 1.0;
		this.mFFLoop = vargc > 1 ? vargv[1] : false;
		this._processAnimation();
		this.mFFSpeed = 1.0;
	}

	mSpeed = 0.0;
	function onMove( pSpeed )
	{
		this.mSpeed = pSpeed;
		local handlerDef = this.getHandlerDef();
		handlerDef.Movement <- pSpeed;
		this.setPause(false);
		this.AnimationHandler.onMove(pSpeed);
	}

	function onStop()
	{
		this.AnimationHandler.onStop();
		local handlerDef = this.getHandlerDef();

		if ("Movement" in handlerDef)
		{
			delete handlerDef.Movement;
		}

		this.setPause(false);
		this._processAnimation();
	}

	function fullStop()
	{
		local handlerDef = this.getHandlerDef();

		if ("FF" in handlerDef)
		{
			delete handlerDef.FF;
		}

		this.onStop();
	}

	function getClassName()
	{
		return "Horde";
	}

	mNewFrame = false;
	mAnimationDuration = 0.0;
	mAnimationSpeed = 0.0;
	mPlayHeadTime = 0.0;
	mOldPlayHeadTime = 0.0;
	mPauseTime = -1.0;
	mDead = false;
	mAnimationLoop = false;
	mFFSpeed = 1.0;
	mFFLoop = false;
	mLogTimer = null;
	mLogO = null;
	mTimer = null;
	mAmbientTimer = null;
}

class this.AnimationHandler.Biped extends this.AnimationHandler
{
	mPosture = "standard";
	mGender = "male";
	mGrip = false;
	mJumping = 0;
	mFF = null;
	mFFIndex = 0;
	mFFSpeed = 1.0;
	mFFLoop = false;
	mSpeed = 0.0;
	mStepTick = 0;
	static SWITCHWEAPON_NONE = 0;
	static SWITCHWEAPON_FROM = 1;
	static SWITCHWEAPON_TO = 2;
	mSwitchingWeaponState = 0;
	mToWeapon = null;
	mFromWeapon = null;
	mWeaponCallback = null;
	mNewAnim = {};
	mLastSoundPlayHead = 0;
	mLastSoundAnimName = null;
	function getClassName()
	{
		return "Biped";
	}

	function _getCustomState( table )
	{
		table.posture <- this.mPosture;
		table.gender <- this.mGender;
	}

	function _setCustomState( table )
	{
		if (table.posture != this.mPosture)
		{
			return false;
		}

		if (table.gender != this.mGender)
		{
			return false;
		}

		return true;
	}

	constructor( pSceneObject )
	{
		this.AnimationHandler.constructor(pSceneObject);
		this.mStepTick = 0;
		this.mEntity.setAnimationUnitCount(3);
		local abottom = this.mEntity.getAnimationUnit(0);
		abottom.setTimeScaleFactor(1.0);
		abottom.setEnabled(true);
		local atop = this.mEntity.getAnimationUnit(1);
		atop.setTimeScaleFactor(1.0);
		atop.setEnabled(true);
		local ahands = this.mEntity.getAnimationUnit(2);
		ahands.setTimeScaleFactor(1.0);
		ahands.setEnabled(true);
		this.mCurrentAnim = {
			top = {
				name = "Idle",
				animUnit = atop,
				speed = 1.0,
				loop = true
			},
			bottom = {
				name = "Idle",
				animUnit = abottom,
				speed = 1.0,
				loop = true
			},
			hands = {
				name = "Idle",
				animUnit = ahands,
				speed = 1.0,
				loop = true
			}
		};
		this.mNewAnim = {
			top = null,
			bottom = null,
			hands = null
		};
		abottom.animationSwitch("Idle_b", true);
		atop.animationSwitch("Idle_t", true);

		if (this.mSceneObject.hasItemInHand())
		{
			this.mCurrentAnim.hands.name = "Idle_Combat_1h";
			this.mNewAnim.hands = this.mCurrentAnim.hands;
			this.mNewAnim.hands.blend <- 0.0;
		}
		else
		{
			ahands.animationSwitch("Idle_h", true);
		}

		this._processAnimation();
	}

	function _startAnimation( pName, pSpeed, pBlend, pLoop, pAnimUnitName )
	{
		if (this.mSwimming && pName.find("Swim") == null && pName != "Idle")
		{
			return;
		}

		local animDef = this.getAnimationDef();

		if (!(pName in animDef))
		{
			throw pName + " not found in animDef.";
		}

		local animUnit = this.mCurrentAnim[pAnimUnitName].animUnit;
		local postFix = "";

		if (pAnimUnitName == "top")
		{
			postFix = "_t";
		}
		else if (pAnimUnitName == "bottom")
		{
			postFix = "_b";
		}
		else if (pAnimUnitName == "hands")
		{
			postFix = "_h";
		}

		animUnit.setTimeScaleFactor(pSpeed);
		animUnit.animationCrossBlend(pName + postFix, pBlend, true);
		this.mCurrentAnim[pAnimUnitName].name <- pName;
		this.mCurrentAnim[pAnimUnitName].speed <- pSpeed;
		this.mCurrentAnim[pAnimUnitName].loop <- pLoop;
		this.mCurrentAnim[pAnimUnitName].stopped <- false;
		local otherAnimUnit;

		if (pAnimUnitName == "top")
		{
			otherAnimUnit = "bottom";
		}
		else
		{
			otherAnimUnit = "top";
		}

		if (this.mCurrentAnim[otherAnimUnit].name == pName)
		{
			local syncTime = 0.0;

			if (this.mCurrentAnim[otherAnimUnit].animUnit.isBlending())
			{
				syncTime = this.mCurrentAnim[otherAnimUnit].animUnit.getTargetTimePosition();
			}
			else
			{
				syncTime = this.mCurrentAnim[otherAnimUnit].animUnit.getTimePosition();
			}

			if (animUnit.isBlending())
			{
				animUnit.setTargetTimePosition(syncTime);
			}
			else
			{
				animUnit.setTimePosition(syncTime);
			}
		}

		if (animUnit.isBlending())
		{
			if (pLoop)
			{
				this.mCurrentAnim[pAnimUnitName].duration <- animUnit.getTargetLength() - animUnit.getTargetTimePosition();
			}
			else
			{
				this.mCurrentAnim[pAnimUnitName].duration <- animUnit.getTargetLength() - animUnit.getTargetTimePosition() - pBlend;
			}

			this.mCurrentAnim[pAnimUnitName].lastTime <- animUnit.getTargetTimePosition();
		}
		else
		{
			if (pLoop)
			{
				this.mCurrentAnim[pAnimUnitName].duration <- animUnit.getLength() - animUnit.getTimePosition();
			}
			else
			{
				this.mCurrentAnim[pAnimUnitName].duration <- animUnit.getLength() - animUnit.getTimePosition() - pBlend;
			}

			this.mCurrentAnim[pAnimUnitName].lastTime <- animUnit.getTimePosition();
		}

		animUnit.setLooping(this.mCurrentAnim[pAnimUnitName].loop);
	}

	function reset()
	{
		this.mNewAnim.bottom <- {
			name = "Idle",
			speed = 1.0,
			blend = 0.0,
			loop = true
		};
		this.mNewAnim.top <- {
			name = "Idle",
			speed = 1.0,
			blend = 0.0,
			loop = true
		};
		this.mNewAnim.hands <- {
			name = "Idle",
			speed = 1.0,
			blend = 0.0,
			loop = true
		};

		if (this.mNewAnim.bottom != null)
		{
			this._startAnimation(this.mNewAnim.bottom.name, this.mNewAnim.bottom.speed, this.mNewAnim.bottom.blend, this.mNewAnim.bottom.loop, "bottom");
		}

		if (this.mNewAnim.top != null)
		{
			this._startAnimation(this.mNewAnim.top.name, this.mNewAnim.top.speed, this.mNewAnim.top.blend, this.mNewAnim.top.loop, "top");
		}

		if (this.mNewAnim.hands != null)
		{
			this._startAnimation(this.mNewAnim.hands.name, this.mNewAnim.hands.speed, this.mNewAnim.hands.blend, this.mNewAnim.hands.loop, "hands");
		}
	}

	function _evaluateAnimation( pName, pSpeed, pLoop, ... )
	{
		local size = this.mSceneObject.getAssembler().getSize();
		local blend = this.getBlend(pName) / size;

		if (vargc > 0)
		{
			blend = vargv[0];
		}

		if (this.mCurrentAnim.bottom.name != pName)
		{
			if (this.mNewAnim.bottom != null)
			{
				if (this.mNewAnim.bottom.name != pName && this._getAnimPriority(this.mNewAnim.bottom.name, "bottom") <= this._getAnimPriority(pName, "bottom"))
				{
					this.mNewAnim.bottom <- {
						name = pName,
						speed = pSpeed,
						blend = blend,
						loop = pLoop
					};
				}
			}
			else if (this._getAnimPriority(this.mCurrentAnim.bottom.name, "bottom") <= this._getAnimPriority(pName, "bottom") || this._isAnimationComplete("bottom"))
			{
				this.mNewAnim.bottom <- {
					name = pName,
					speed = pSpeed,
					blend = blend,
					loop = pLoop
				};
			}
		}
		else
		{
			this.mCurrentAnim.bottom.animUnit.setTimeScaleFactor(pSpeed);
		}

		if (this.mCurrentAnim.top.name != pName)
		{
			if (this.mNewAnim.top != null)
			{
				if (this.mNewAnim.top.name != pName && this._getAnimPriority(this.mNewAnim.top.name, "top") <= this._getAnimPriority(pName, "top"))
				{
					this.mNewAnim.top <- {
						name = pName,
						speed = pSpeed,
						blend = blend,
						loop = pLoop
					};
					this.checkHideWeapons(this.mCurrentAnim.top, this.mNewAnim.top);

					if (!this.mGrip)
					{
						this.mNewAnim.hands <- {
							name = pName,
							speed = pSpeed,
							blend = blend,
							loop = pLoop
						};
					}
					else
					{
						this.mNewAnim.hands <- {
							name = "Idle_Combat_1h",
							speed = pSpeed,
							blend = blend,
							loop = pLoop
						};
					}
				}
			}
			else if (this._getAnimPriority(this.mCurrentAnim.top.name, "top") <= this._getAnimPriority(pName, "top") || this._isAnimationComplete("top"))
			{
				this.mNewAnim.top <- {
					name = pName,
					speed = pSpeed,
					blend = blend,
					loop = pLoop
				};
				this.checkHideWeapons(this.mCurrentAnim.top, this.mNewAnim.top);

				if (!this.mGrip)
				{
					this.mNewAnim.hands <- {
						name = pName,
						speed = pSpeed,
						blend = blend,
						loop = pLoop
					};
				}
				else
				{
					this.mNewAnim.hands <- {
						name = "Idle_Combat_1h",
						speed = pSpeed,
						blend = blend,
						loop = pLoop
					};
				}
			}
		}
		else
		{
			this.mCurrentAnim.top.animUnit.setTimeScaleFactor(pSpeed);
			this.mCurrentAnim.hands.animUnit.setTimeScaleFactor(pSpeed);
		}
	}

	function checkHideWeapons( oldAnim, newAnim )
	{
		local animDef = this.getAnimationDef();
		local unhideWeapons = false;

		if (!(newAnim.name in animDef))
		{
			throw newAnim.name + " not found in animDef.";
		}

		local unhideWeapons = false;

		if (("HideWeapons" in animDef[oldAnim.name]) && true == animDef[oldAnim.name].HideWeapons)
		{
			unhideWeapons = true;
		}

		if (("HideWeapons" in animDef[newAnim.name]) && true == animDef[newAnim.name].HideWeapons)
		{
			if (!unhideWeapons)
			{
				this.mSceneObject.hideHandAttachments();
				this.mGrip = true;
			}
			else
			{
				unhideWeapons = false;
			}
		}

		if (unhideWeapons)
		{
			this.mSceneObject.unHideHandAttachments();
		}

		this.mSceneObject.updateGrip();
	}

	function switchingWeapons()
	{
		if (this.mSwitchingWeaponState != this.SWITCHWEAPON_NONE)
		{
			return true;
		}

		return false;
	}

	function _weaponsSwitched()
	{
		this.mSwitchingWeaponState = this.SWITCHWEAPON_NONE;

		if (this.mWeaponCallback)
		{
			this.mWeaponCallback.weaponsSwitched();
		}

		this.mWeaponCallback = null;
	}

	function switchWeapons( old, new, ... )
	{
		if (this.mSwitchingWeaponState != this.SWITCHWEAPON_NONE)
		{
			return;
		}

		this.mToWeapon = new;
		this.mFromWeapon = old;
		this.mWeaponCallback = null;

		if (vargc > 0)
		{
			this.mWeaponCallback = vargv[0];
		}

		if (old > this.VisibleWeaponSet.NONE)
		{
			this.mSwitchingWeaponState = this.SWITCHWEAPON_FROM;
		}
		else if (new > this.VisibleWeaponSet.NONE)
		{
			this.mSwitchingWeaponState = this.SWITCHWEAPON_TO;
		}
		else
		{
			return false;
		}

		this._processAnimation();
		return true;
	}

	function _processAnimation()
	{
		local size = this.mSceneObject.getAssembler().getSize();
		local animDef = this.getAnimationDef();
		local index = 0;

		if (this.mFFIndex > 0)
		{
			index = this.mFFIndex - 1;
		}

		if (this.mFF)
		{
			if (this.type(this.mFF) == "array")
			{
				if (this.mFFIndex == this.mFF.len())
				{
					this.mFF = null;
					this.mFFIndex = 0;
					this.mFFSpeed = 1.0;
					this.mFFLoop = false;
				}
				else if (this.mCurrentAnim.top.name != this.mFF[index] || this._isAnimationComplete("top"))
				{
					if (this.mCurrentAnim.top.name == "Walk" || this.mCurrentAnim.top.name == "Run" || this.mCurrentAnim.top.name == "Jog" || this.mCurrentAnim.top.name == "Idle")
					{
						this._evaluateAnimation(this.mFF[this.mFFIndex], 1.0 / size * this.mFFSpeed, this.mFFLoop);
					}
					else
					{
						this._evaluateAnimation(this.mFF[this.mFFIndex], 1.0 * this.mFFSpeed, this.mFFLoop);
					}

					this.mFFIndex++;
					index = this.mFFIndex - 1;
				}
			}
			else
			{
				if (this.mFF == "Walk" || this.mFF == "Run" || this.mFF == "Jog" || this.mFF == "Idle")
				{
					this._evaluateAnimation(this.mFF, 1.0 / size * this.mFFSpeed, this.mFFLoop);
				}
				else
				{
					this._evaluateAnimation(this.mFF, 1.0 * this.mFFSpeed, this.mFFLoop);
				}

				this.mFF = null;
				this.mFFSpeed = 1.0;
				this.mFFLoop = false;
			}
		}

		if (this.mJumping > 0)
		{
			local jumpAnim = this._getJumpAnimation();
			this._evaluateAnimation(jumpAnim.name, 1.0, true, jumpAnim.blend);
		}
		else if (this.mMoving)
		{
			local moveAnim = this.getMovementAnimation(this.mSpeed / size);

			if (moveAnim)
			{
				this._evaluateAnimation(moveAnim.name, this.mSpeed / moveAnim.meanSpeed / size, true);

				if (this.mSwitchingWeaponState != 0)
				{
					local switchAnim = this._getSwitchWeaponAnim();

					if (switchAnim)
					{
						this._evaluateAnimation(switchAnim.name, 1.0, false);
					}
				}
			}
		}
		else if (this.mSwitchingWeaponState != 0)
		{
			local switchAnim = this._getSwitchWeaponAnim();

			if (switchAnim)
			{
				this._evaluateAnimation(switchAnim.name, 1.0, false);
			}
		}
		else if (!this.mDead)
		{
			this._evaluateAnimation(this._getIdleName(), 1.0 / size * this.mFFSpeed, true);
		}

		if (this.type(this.mFF) == "array")
		{
			if (this.mNewAnim.top != null && (this.mCurrentAnim.top.name != this.mFF[index] || this._isAnimationComplete("top")))
			{
				if (this.mNewAnim.top.name != this.mFF[index])
				{
					this._processAnimation();
				}
			}
		}

		if (this.mNewAnim.bottom != null)
		{
			this._startAnimation(this.mNewAnim.bottom.name, this.mNewAnim.bottom.speed, this.mNewAnim.bottom.blend, this.mNewAnim.bottom.loop, "bottom");
		}

		if (this.mNewAnim.top != null)
		{
			this._startAnimation(this.mNewAnim.top.name, this.mNewAnim.top.speed, this.mNewAnim.top.blend, this.mNewAnim.top.loop, "top");
		}

		if (this.mNewAnim.hands != null)
		{
			this._startAnimation(this.mNewAnim.hands.name, this.mNewAnim.hands.speed, this.mNewAnim.hands.blend, this.mNewAnim.hands.loop, "hands");
		}

		this.mNewAnim = {
			top = null,
			bottom = null,
			hands = null
		};
	}

	function _getAnimPriority( pName, pUnitName )
	{
		local animDef = this.getAnimationDef();

		if (!(pName in animDef))
		{
			throw "Animation def does not container animation " + pName;
		}

		if (!("Catagory" in animDef[pName]))
		{
			throw "Catagory missing for animation " + pName;
		}

		local catagory = animDef[pName].Catagory;

		if (pUnitName.tolower() == "bottom")
		{
			return ::BipedAnimationDef.Catagories[catagory].Bottom;
		}
		else
		{
			return ::BipedAnimationDef.Catagories[catagory].Top;
		}
	}

	function _getIdleName()
	{
		local custom_idle;

		if (this.mSceneObject)
		{
			custom_idle = this.mSceneObject.getStat(this.Stat.ALTERNATE_IDLE_ANIM, true);
		}

		if (custom_idle && custom_idle != "")
		{
			return custom_idle;
		}

		if (this.mCombat)
		{
			return this.translateAnim("$IDLE_COMBAT$");
		}

		if (this.mSwimming)
		{
			return "Swim_Tread";
		}

		local name = "Idle";

		if (this.mPosture == "flowing")
		{
			name += "_Flowing";
		}
		else if (this.mPosture == "hulking")
		{
			name += "_Hulking";
		}

		if (this.mGender == "female")
		{
			name += "_Female";
		}

		return name;
	}

	function _getJumpAnimation()
	{
		local rOffset = this.Math.ConvertRadToFloatPercentage(this.mSceneObject.mController.mRotationOffset);
		local size = this.mSceneObject.getAssembler().getSize();

		if (false && this.mMoving)
		{
			if (rOffset <= 0.1875 || rOffset >= 0.8125)
			{
				local test = true;
			}
		}
		else
		{
			local vel = this.mSceneObject.mVerticalSpeed;
			local acc = this.mSceneObject.mDownwardAcceleration;

			if (this.mJumping == 1)
			{
				this.mJumping = 2;
				return {
					name = "Jump_Standing_Recover",
					blend = this.getBlend("Jump_Standing_Recover")
				};
			}
			else if (this.mJumping == 2)
			{
				this.mJumping = 3;
				return {
					name = "Jump_Standing_Up",
					blend = this.getBlend("Jump_Standing_Up")
				};
			}
			else if (vel > 0.0 && this.mJumping == 3)
			{
				local blendTime = vel / acc;
				return {
					name = "Jump_Standing_Top",
					blend = blendTime
				};
			}
			else if (vel <= 0.0 && this.mJumping == 3 || this.mJumping == 4)
			{
				this.mJumping = 4;
				return {
					name = "Jump_Standing_Down",
					blend = this.getBlend("Jump_Standing_Down")
				};
			}
			else if (this.mJumping == 5)
			{
				this.mJumping = 0;
				return {
					name = "Jump_Standing_Recover",
					blend = this.getBlend("Jump_Standing_Recover")
				};
			}
		}
	}

	function _getSwitchWeaponAnim()
	{
		switch(this.mSwitchingWeaponState)
		{
		case this.SWITCHWEAPON_FROM:
			switch(this.mFromWeapon)
			{
			case this.VisibleWeaponSet.MELEE:
				return {
					name = "One_Handed_Side_Sheathe",
					blend = this.getBlend("One_Handed_Side_Sheathe")
				};

			case this.VisibleWeaponSet.RANGED:
				return {
					name = "Dual_Wield_Back_Sheathe",
					blend = this.getBlend("Dual_Wield_Back_Sheathe")
				};
			}

			break;

		case this.SWITCHWEAPON_TO:
			switch(this.mToWeapon)
			{
			case this.VisibleWeaponSet.MELEE:
				return {
					name = "One_Handed_Side_Draw",
					blend = this.getBlend("One_Handed_Side_Draw")
				};

			case this.VisibleWeaponSet.RANGED:
				return {
					name = "Dual_Wield_Back_Draw",
					blend = this.getBlend("Dual_Wield_Back_Draw")
				};

			default:
				break;
			}
		}

		return null;
	}

	function _isAnimationComplete( animUnitName )
	{
		if (this.mDead == true)
		{
			return false;
		}

		if (animUnitName in this.mCurrentAnim)
		{
			if (!("stopped" in this.mCurrentAnim[animUnitName]))
			{
				return true;
			}

			if (this.mCurrentAnim[animUnitName].stopped)
			{
				return true;
			}

			if (this.mCurrentAnim[animUnitName].loop)
			{
				return false;
			}
			else
			{
				local animUnit = this.mCurrentAnim[animUnitName].animUnit;
				local lastTime = this.mCurrentAnim[animUnitName].lastTime;
				local duration = this.mCurrentAnim[animUnitName].duration;
				local currentTime = 0.0;

				if (animUnit.isBlending())
				{
					currentTime = animUnit.getTargetTimePosition();
				}
				else
				{
					currentTime = animUnit.getTimePosition();
				}

				if (lastTime > currentTime)
				{
					return true;
				}

				if (currentTime >= duration)
				{
					return true;
				}

				this.mCurrentAnim[animUnitName].lastTime = currentTime;
				return false;
			}
		}
		else
		{
			throw "invalide animation unit name provided to _isAnimationComplete";
		}
	}

	function getMovementAnimation( pSpeed )
	{
		local controller = this.mSceneObject.getController();

		if (!controller)
		{
			return null;
		}

		local forSet = controller.getMovementAnimationName();

		foreach( i, x in forSet )
		{
			if (pSpeed >= x[0] && pSpeed <= x[2])
			{
				return {
					name = i,
					meanSpeed = x[1]
				};
			}
		}

		return null;
	}

	function _getPrimaryAU()
	{
		if ("top" in this.mCurrentAnim)
		{
			if ("animUnit" in this.mCurrentAnim.top)
			{
				return this.mCurrentAnim.top.animUnit;
			}
		}

		return null;
	}

	function getCurrentAnim()
	{
		if ("top" in this.mCurrentAnim)
		{
			if ("name" in this.mCurrentAnim.top)
			{
				return this.mCurrentAnim.top.name;
			}
		}

		return null;
	}

	function getBlend( animName )
	{
		local animDef = this.getAnimationDef();
		local bipedDef = this.getBipedDef();

		if (animName in animDef)
		{
			if ("Blend" in animDef[animName])
			{
				return animDef[animName].Blend;
			}
		}

		if ("Blend" in bipedDef)
		{
			return bipedDef.Blend;
		}
		else
		{
			return 0.2;
		}
	}

	function getStepAnimationDef()
	{
		local bottomAnim = this.mCurrentAnim.bottom;
		local animDef = this.getAnimationDef();
		return animDef[bottomAnim.name];
	}

	function getStepAnimationUnit()
	{
		return this.mCurrentAnim.bottom.animUnit;
	}

	function onEnterFrame()
	{
		local animDef = this.getAnimationDef();
		local top = this.mCurrentAnim.top.name;
		local bottom = this.mCurrentAnim.bottom.name;

		if (animDef[top].Catagory == "Jump" && (this.mJumping < 4 || this.mJumping == 5))
		{
			if (!this.mCurrentAnim.top.animUnit.isBlending())
			{
				this.mCurrentAnim.top.stopped = true;
			}
		}

		if (animDef[bottom].Catagory == "Jump" && (this.mJumping < 4 || this.mJumping == 5))
		{
			if (!this.mCurrentAnim.bottom.animUnit.isBlending())
			{
				this.mCurrentAnim.bottom.stopped = true;
			}
		}

		if (this.mSwitchingWeaponState != this.SWITCHWEAPON_NONE)
		{
			local switchAnim = this._getSwitchWeaponAnim();

			if (!switchAnim || this.mCurrentAnim.top.name != switchAnim.name && this.mCurrentAnim.bottom.name != switchAnim.name || this._isAnimationComplete("top") || this._isAnimationComplete("bottom"))
			{
				switch(this.mSwitchingWeaponState)
				{
				case this.SWITCHWEAPON_FROM:
					this.mSceneObject.hideWeapons();

					if (this.mToWeapon != this.VisibleWeaponSet.INVALID || this.mToWeapon != this.VisibleWeaponSet.NONE)
					{
						this.mSwitchingWeaponState = this.SWITCHWEAPON_TO;
					}
					else
					{
						this._weaponsSwitched();
					}

					break;

				case this.SWITCHWEAPON_TO:
					this.mSceneObject.showWeapons();
					this._weaponsSwitched();
					break;
				}

				this._processAnimation();
			}
		}
		else if (this._isAnimationComplete("top") || this._isAnimationComplete("bottom"))
		{
			this._processAnimation();
		}

		this._updateStepSound();
		this._processSound();
	}

	function setPosture( pPosture )
	{
		if (pPosture == "" || pPosture == null)
		{
			this.mPosture = "standard";
		}
		else if (pPosture.tolower() == "standard" || pPosture.tolower() == "flowing" || pPosture.tolower() == "hulking")
		{
			this.mPosture = pPosture.tolower();
		}
		else
		{
			throw "AnimationHandler.Biped.setPosture: invalide posture set " + pPosture;
		}

		this._processAnimation();
	}

	function getPosture()
	{
		return this.mPosture;
	}

	function setCombat( pBool )
	{
		this.mCombat = pBool;
		this._processAnimation();
	}

	function getCombat( pBool )
	{
		return this.mCombat;
	}

	function setGrip( pBool )
	{
		this.mGrip = pBool;

		if (this.mGrip)
		{
			this.mNewAnim.hands = "Idle_Combat_1h";
			this.mNewAnim.hands <- {
				name = "Idle_Combat_1h",
				speed = 1.0,
				blend = 0.2,
				loop = true
			};
		}
		else
		{
			this.mNewAnim.hands = "Idle";
			this.mNewAnim.hands <- {
				name = "Idle",
				speed = 1.0,
				blend = 0.2,
				loop = true
			};
		}

		this._processAnimation();
	}

	function setGender( pGender )
	{
		this.mGender = pGender.tolower();
		this.processAnimation();
	}

	function getGender()
	{
		return this.mGender;
	}

	function getBipedDef()
	{
		return ::BipedAnimationDef;
	}

	function getAnimationDef()
	{
		local model = this.mSceneObject.mAssembler.mBody;

		if ((model in ::ModelDef) && "Animations" in ::ModelDef[model])
		{
			return ::ModelDef[model].Animations;
		}

		return ::BipedAnimationDef.Animations;
	}

	function _getWieldedWeaponGroup()
	{
		local weaponGroup = "";
		local count = 0;

		foreach( a in this.mSceneObject.getHandAttachments() )
		{
			local wt = a.getWeaponType();

			if (wt == "Shield")
			{
				wt = "";
			}

			if (wt == "2h" || wt == "Bow" || wt == "Staff" || wt == "Wand")
			{
				return wt;
			}

			if (wt != "")
			{
				count++;
			}

			if (count == 2)
			{
				return "Dual";
			}

			if (wt != "")
			{
				weaponGroup = wt;
			}
		}

		return weaponGroup;
	}

	function translateAnim( anim )
	{
		if (typeof anim != "string")
		{
			return anim;
		}

		switch(anim)
		{
		case "$MELEE$":
			local weaponGroup = this._getWieldedWeaponGroup();
			this.log.debug("$MELEE$ Animation weapon group: " + weaponGroup);

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

			case "Staff":
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
					"One_Handed_Shield_Counter"
				]);
				break;
			}

			break;

		case "$IDLE_COMBAT$":
			local weaponGroup = this._getWieldedWeaponGroup();

			switch(weaponGroup)
			{
			case "2h":
				return "Idle_Combat_Two_Handed";

			case "Staff":
				return "Idle_Combat_Staff";

			case "Wand":
				return "Idle_Combat_Magic";

			case "Bow":
				return "Idle_Combat_Bow";
				break;

			default:
				if (weaponGroup == "1h")
				{
				}
			}

			return "Idle_Combat_1h";
			break;

		case "$HIT$":
			local weaponGroup = this._getWieldedWeaponGroup();

			switch(weaponGroup)
			{
			case "2h":
				return "Hit_Two_Handed";

			case "Staff":
				return "Hit_Staff";

			case "Bow":
				return "Hit_Bow";
				break;

			default:
				if (weaponGroup == "1h")
				{
				}
			}

			return "Hit_Dual_Wield";
			break;
		}

		return anim;
	}

	function onFF( animData, ... )
	{
		if (this.mDead == true)
		{
			return;
		}

		animData = this.translateAnim(animData);

		if (this.type(animData).tolower() == "string")
		{
			animData = this.Util.toFirstCaps(animData);

			if (animData in this.getAnimationDef())
			{
				this.mFF = animData;
				this.mFFIndex = 0;
				this.mFFSpeed = vargc > 0 ? vargv[0] : 1.0;
				this.mFFLoop = vargc > 1 ? vargv[1] : false;
				this._processAnimation();
			}
		}
		else if (this.type(animData).tolower() == "array")
		{
			local subData = [];
			local animDef = this.getAnimationDef();

			foreach( i, x in animData )
			{
				if (x in animDef)
				{
					subData.append(x);
				}
			}

			if (subData.len() > 0)
			{
				this.mFF = animData;
				this.mFFIndex = 0;
				this.mFFSpeed = vargc > 0 ? vargv[0] : 1.0;
				this.mFFLoop = vargc > 1 ? vargv[1] : false;
				this._processAnimation();
			}
		}
	}

	function onMove( pSpeed )
	{
		this.AnimationHandler.onMove(pSpeed);
		this.mSpeed = pSpeed;
		this._processAnimation();
	}

	function stopAnim( anim )
	{
		if (this.mCurrentAnim.top.name == anim)
		{
			this.mCurrentAnim.top.stopped = true;
			this._processAnimation();
		}
		else if (this.mCurrentAnim.bottom.name == anim)
		{
			this.mCurrentAnim.bottom.stopped = true;
			this._processAnimation();
		}
	}

	function onStop()
	{
		if (!this.mMoving)
		{
			return;
		}

		this.mSpeed = 0.0;
		local animDef = this.getAnimationDef();
		local top = this.mCurrentAnim.top.name;
		local bottom = this.mCurrentAnim.bottom.name;

		if (animDef[top].Catagory == "Movement")
		{
			this.mCurrentAnim.top.stopped = true;
		}

		if (animDef[bottom].Catagory == "Movement")
		{
			this.mCurrentAnim.bottom.stopped = true;
		}

		this.mFF = null;
		this.mFFSpeed = 1.0;
		this.mFFLoop = false;
		this.AnimationHandler.onStop();
		this._processAnimation();
	}

	function forceAnimUpdate()
	{
		this._processAnimation();
	}

	function onDeath()
	{
		this.mDead = true;
		this.mNewAnim.top <- {
			name = "Death_Backward",
			speed = 1.0,
			blend = 0.2,
			loop = false
		};
		this.mNewAnim.bottom <- {
			name = "Death_Backward",
			speed = 1.0,
			blend = 0.2,
			loop = false
		};
		this._processAnimation();
	}

	function onRes()
	{
		this.mDead = false;
		this._processAnimation();
	}

	function fullStop()
	{
		this.mStepSoundRecords = null;
		this.mMoving = false;
		this.mSpeed = 0.0;
		this.mCurrentAnim.bottom.stopped <- true;
		this.mCurrentAnim.top.stopped <- true;
		this.mCurrentAnim.hands.stopped <- true;
		this.mFF = null;
		this.mFFSpeed = 1.0;
		this.mFFLoop = false;
		this._processAnimation();
	}

	function onJump()
	{
		if (this.mJumping == 0 || this.mJumping == 5)
		{
			this.mJumping = 1;
			this._processAnimation();
		}
	}

	function onFalling()
	{
		if (this.mJumping == 0)
		{
			this.mJumping = 3;
			this._processAnimation();
		}
	}

	function onLand()
	{
		if (this.mJumping > 0)
		{
			this.mJumping = 5;
			local animDef = this.getAnimationDef();
			local top = this.mCurrentAnim.top.name;
			local bottom = this.mCurrentAnim.bottom.name;

			if (animDef[top].Catagory == "Jump")
			{
				this.mCurrentAnim.top.stopped = true;
			}

			if (animDef[bottom].Catagory == "Jump")
			{
				this.mCurrentAnim.bottom.stopped = true;
			}

			this._processAnimation();
		}
	}

}

