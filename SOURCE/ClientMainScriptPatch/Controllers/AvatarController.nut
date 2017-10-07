this.require("Controllers/ServerController");
class this.Controller.Avatar2 extends this.Controller.Server
{
	constructor( pSceneObject )
	{
		this.Controller.Server.constructor(pSceneObject);
	}

	function onServerPosition( pX, pY, pZ )
	{
		this.Controller.Server.onServerPosition(pX, pY, pZ);

		if (this.distanceCheck() == false)
		{
			if (!this.getMoveActive())
			{
				this.mSceneObject.mSpeed = 0.0;
				this._moveEnded();
			}
		}
	}

	function onUpdate()
	{
	}

	function canInteractWith( sceneObject )
	{
		return sceneObject.getMeta("copper_shopkeeper") || 
				sceneObject.getMeta("credit_shopkeeper") || 
				sceneObject.getMeta("essence_vendor") || 
				sceneObject.getMeta("vault") || 
				sceneObject.getMeta("auctioneer") || 
				sceneObject.getMeta("clan_registrar") || 
				sceneObject.getMeta("crafter") || 
				sceneObject.getMeta("credit_shop") || 
				sceneObject.hasStatusEffect(this.StatusEffects.TRANSFORMER) || 
				( sceneObject.getQuestIndicator() && sceneObject.getQuestIndicator().hasValidQuest() ) || 
				( sceneObject.getQuestIndicator() && sceneObject.getQuestIndicator().hasCompletedNotTurnInQuest() ) || 
				::_useableCreatureManager.isUseable(sceneObject.getID());
	}

	function _moveStarted()
	{
		local animHandler = this.mSceneObject.getAnimationHandler();

		if (animHandler)
		{
			animHandler.onMove(this.mAvatarSpeed * this.getSpeedFactor());
		}

		this.startHeartBeat();
	}

	function _isAvatarMoving( animHandler )
	{
		if (animHandler.mCurrentAnim == "Walk" || animHandler.mCurrentAnim == "Run" || animHandler.mCurrentAnim == "Jog")
		{
			return true;
		}

		foreach( type in animHandler.mCurrentAnim )
		{
			if (type && typeof type == "table" && "name" in type)
			{
				if (this.Util.startsWith(type.name, "Jog") || this.Util.startsWith(type.name, "Run") || this.Util.startsWith(type.name, "Walk") || this.Util.startsWith(type.name, "Swim"))
				{
					return true;
				}
			}
		}

		return false;
	}

	function _moveEnded()
	{
		if (this.mDirection.equals(this.Vector3(0.0, 0.0, 0.0)))
		{
			local animHandler = this.mSceneObject.getAnimationHandler();

			if (animHandler && this._isAvatarMoving(animHandler))
			{
				animHandler.onStop();
			}

			this.mSceneObject.setSpeed(0.0);
			this.stopHeartBeat();
		}
		else
		{
			this._moveStarted();
		}
	}

	function onIncreaseAvatarSpeed()
	{
		if (this.Util.hasPermission("dev"))
		{
			local maxSpeed = 250.0;
			this.mAvatarSpeed = this.Math.min(this.mAvatarSpeed + 5.0, maxSpeed);
			this.IGIS.info("Speed Increased: " + this.mAvatarSpeed + "%");
			local animHandler = this.mSceneObject.getAnimationHandler();

			if (this.getMoveActive() && animHandler)
			{
				animHandler.onMove(this.mSceneObject.mSpeed);
			}
		}
	}

	function onDecreaseAvatarSpeed()
	{
		if (this.Util.hasPermission("dev"))
		{
			this.mAvatarSpeed = this.Math.max(this.mAvatarSpeed - 5.0, 0.0);
			this.IGIS.info("Speed Decreased: " + this.mAvatarSpeed + "%");
			local animHandler = this.mSceneObject.getAnimationHandler();

			if (this.getMoveActive() && animHandler)
			{
				animHandler.onMove(this.mSceneObject.mSpeed);
			}
		}
	}

	function setAvatarSpeed( speed )
	{
		this.mAvatarSpeed = speed;
		local animHandler = this.mSceneObject.getAnimationHandler();

		if (this.getMoveActive() && animHandler)
		{
			animHandler.onMove(this.mSceneObject.mSpeed);
		}
	}

	function isForwardDirectionStopped()
	{
		return this.mDirection.z == 0.0;
	}

	function onAvatarForwardStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			if (this.mDirection.z == 0.0)
			{
				this.mDirection.z += 1;
			}

			this._moveStarted();
		}
	}

	function onAvatarForwardStop()
	{
		if (this.mMovingToTarget || this.mMovingToPoint)
		{
			return;
		}

		if (this.mDirection.z > 0.0)
		{
			this.mDirection.z -= 1;
		}

		this._moveEnded();
	}

	function onAvatarBackwardStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			if (this.mDirection.z == 0.0)
			{
				this.mDirection.z -= 1;
			}

			this._moveStarted();
		}
	}

	function onAvatarBackwardStop()
	{
		if (this.mDirection.z < 0.0)
		{
			this.mDirection.z += 1;
		}

		this._moveEnded();
	}

	function onAvatarForwardBackwardStop()
	{
		if (this.mDirection.z != 0.0)
		{
			this.mDirection.z = 0;
		}

		this._moveEnded();
	}

	function onAvatarStrafeLeftStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			if (this.mDirection.x == 0.0)
			{
				this.mDirection.x += 1.0;
			}

			this._moveStarted();
		}
	}

	function onAvatarStrafeLeftStop()
	{
		if (this.mDirection.x > 0.0)
		{
			this.mDirection.x -= 1.0;
		}

		this._moveEnded();
	}

	function onAvatarStrafeRightStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			if (this.mDirection.x == 0.0)
			{
				this.mDirection.x -= 1.0;
			}

			this._moveStarted();
		}
	}

	function onAvatarStrafeRightStop()
	{
		if (this.mDirection.x < 0.0)
		{
			this.mDirection.x += 1.0;
		}

		this._moveEnded();
	}

	function onAvatarLeftRightStop()
	{
		if (this.mDirection.x != 0.0)
		{
			this.mDirection.x = 0.0;
		}

		this._moveEnded();
	}

	function onAvatarRightStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			this.startHeartBeat();
			this.mTurnDir = -1.0;
		}
	}

	function onAvatarLeftStart()
	{
		if (::_avatar.isGMFrozen())
		{
			return;
		}

		this.stopFollowing();

		if (this.mEnabled)
		{
			this.startHeartBeat();
			this.mTurnDir = 1.0;
		}
	}

	function onAvatarRotateStop()
	{
		this.mTurnDir = 0.0;
	}

	function setEnabled( which )
	{
		if (which != this.mEnabled)
		{
			this.mEnabled = which;

			if (this.mEnabled == false)
			{
				this.mDirection.x = 0.0;
				this.mDirection.y = 0.0;
				this.mDirection.z = 0.0;
				this._moveEnded();
			}
		}
	}

	function onAvatarJump()
	{
		this.stopFollowing();

		if (this.mSwimming)
		{
			return;
		}

		local floor = this.Util.getFloorHeightAt(this.mSceneObject.getPosition(), 10.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true);

		if (floor != null)
		{
			if (floor.normal.dot(this.Vector3(0, 1, 0)) > this.gMaxSlope && this.mSceneObject.getDistanceToFloor() <= 1.5 && this.mJumpTimer == null)
			{
				local ab = this._AbilityManager.getAbilityByName("Jump");

				if (ab)
				{
					ab.sendActivationRequest();
				}

				this.mSceneObject.setDistanceToFloor(0.0);
				this.mJumpTimer = 0.0;
				this.onJump();
			}
		}
	}

	function _avatarJumpCheck()
	{
		if (this.mJumpTimer != null)
		{
			this.mJumpTimer += this._deltat;

			if (this.mJumpTimer >= 150)
			{
				this.mJumpTimer = null;
			}
		}
	}

	function onServerVelocity( pServerHeading, pServerRotation, pServerSpeed )
	{
		this.Controller.Server.onServerVelocity(pServerHeading, pServerRotation, pServerSpeed);

		if (!this.mSceneObject.mLastServerUpdate)
		{
			this.onHeadingChanged();
			::_playTool.setYaw(this.Math.rad2deg(this.mSceneObject.getRotation()) + 180);
		}
	}

	function onEnterFrame()
	{
		this._avatarJumpCheck();

		if (this.mMovingToTarget)
		{
			this.updateAutoFollow();
		}
		else if (this.mMovingToPoint)
		{
			this.updateMovingToPoint();
		}

		if (this.mEnabled && this.getMoveActive())
		{
			this.mSceneObject.setSpeed(this.mAvatarSpeed * this.getSpeedFactor());
			local qrot = this.Quaternion(this.mRotation, this.Vector3(0.0, 1.0, 0.0));
			local dir = qrot.rotate(this.mDirection);
			local yRot = this.Math.polarAngle(dir.x, dir.z);
			this.mSceneObject.updateHeading(yRot);
		}

		local delta = this._deltat / 1000.0;

		if (!this.mMovingToTarget && !this.mMovingToPoint)
		{
			this.mRotation += delta * this.mTurnDir * this.mTurnSpeed;
		}

		this.mSceneObject.setRotation(this.mRotation);
	}

	function onHeadingChanged()
	{
		this.mRotation = this.mSceneObject.mHeading;
		this.mSceneObject.setRotation(this.mRotation);
	}

	function getSpeedFactor()
	{
		if (this.mDirection.z < 0.0)
		{
			return 0.5;
		}

		return 1.0;
	}

	function getMovingForward()
	{
		return this.mDirection.z > 0.0;
	}

	function getMovementAnimationName()
	{
		if (this.mDirection.z < 0.0)
		{
			if (this.mSwimming)
			{
				return ::BipedAnimationDef.Swim_Backward;
			}

			if (this.mDirection.x > 0.0)
			{
				return ::BipedAnimationDef.Backward_Left;
			}
			else if (this.mDirection.x < 0.0)
			{
				return ::BipedAnimationDef.Backward_Right;
			}

			return ::BipedAnimationDef.Backward;
		}

		if (this.mDirection.x > 0.0)
		{
			if (this.mDirection.z > 0.0)
			{
				return this.mSwimming ? ::BipedAnimationDef.Swim_Forward_Left : ::BipedAnimationDef.Forward_Left;
			}

			return this.mSwimming ? ::BipedAnimationDef.Swim_Left : ::BipedAnimationDef.StrafeLeft;
		}

		if (this.mDirection.x < 0.0)
		{
			if (this.mDirection.z > 0.0)
			{
				return this.mSwimming ? ::BipedAnimationDef.Swim_Forward_Right : ::BipedAnimationDef.Forward_Right;
			}

			return this.mSwimming ? ::BipedAnimationDef.Swim_Right : ::BipedAnimationDef.StrafeRight;
		}

		return this.mSwimming ? ::BipedAnimationDef.Swim_Forward : ::BipedAnimationDef.Forward;
	}

	function getRotationActive()
	{
		return this.mTurnDir != 0.0;
	}

	function getMoveActive()
	{
		return this.mDirection.equals(this.Vector3(0.0, 0.0, 0.0)) == false;
	}

	function setStrafeOverride( which )
	{
		this.mStrafeOverride = which;
	}

	function getTurnSpeed()
	{
		return this.mTurnSpeed;
	}

	function startFollowing( creature, keepTryingToUse )
	{
		this.stopFollowing();

		if (creature == null)
		{
			return;
		}

		this.mMovingToTarget = true;
		this.mTargetToMoveTo = creature;
		this.mRepeatUseageAttempt = keepTryingToUse;
	}

	function startMovingToPoint( point )
	{
		this.stopFollowing();
		this.mPositionToMoveTo = point;
		this.mMovingToPoint = true;
	}

	function stopFollowing()
	{
		this.mMovingToTarget = false;
		this.mTargetToMoveTo = null;
		this.mAutoFollowMoving = false;
		this.mRepeatUseageAttempt = false;
		this.mRepeatUseageAttempt = false;
		this.mPositionToMoveTo = null;
		this.mMovingToPoint = false;
	}

	function isAutoMoving()
	{
		return this.mAutoFollowMoving == true || this.mMovingToPoint == true;
	}

	function BetweenZeroAnd360( val )
	{
		while (val < 0)
		{
			val += 2 * this.Math.PI;
		}

		while (val > 2 * this.Math.PI)
		{
			val -= 2 * this.Math.PI;
		}

		return val;
	}

	function updateMovingToPoint()
	{
		local targetPos = this.mPositionToMoveTo;

		if (targetPos == null)
		{
			this.stopFollowing();
		}

		local d = this.Math.manhattanDistanceXZ(::_avatar.getPosition(), targetPos);

		if (d < 5.0)
		{
			this.stopFollowing();
			this.onAvatarForwardStop();
			return;
		}

		local tdir = this.Math.DetermineRadAngleBetweenTwoPoints(::_avatar.getPosition(), targetPos);
		local tq = this.Quaternion(tdir, this.Vector3().UNIT_Y);
		local aq = this.Quaternion(this.mRotation, this.Vector3().UNIT_Y);
		local rq = aq.slerp(0.1, tq);
		this.mRotation = this.Math.ConvertQuaternionToRad(rq);

		if (!this.mAutoFollowMoving && d > 5.0)
		{
			this.mAutoFollowMoving = true;
			this.mDirection.z += 1;
			this._moveStarted();
		}
	}

	function isMouseMoving()
	{
		return this.mMovingToTarget || this.mMovingToPoint;
	}

	function updateAutoFollow()
	{
		local targetPos = this.mTargetToMoveTo.getPosition();

		if (targetPos == null)
		{
			this.stopFollowing();
			return;
		}

		local d = this.Math.manhattanDistanceXZ(::_avatar.getPosition(), targetPos);

		if (d < this.AUTO_FOLLOW_DISTANCE)
		{
			if (this.mRepeatUseageAttempt)
			{
				local useSuccessful = ::_avatar.useCreature(this.mTargetToMoveTo);

				if (useSuccessful)
				{
					this.stopFollowing();
				}
			}

			if (this.mAutoFollowMoving)
			{
				this.mAutoFollowMoving = false;
				this.mMovingToTarget = false;
				this.onAvatarForwardStop();
				this.mMovingToTarget = true;
			}

			return;
		}

		local tdir = this.Math.DetermineRadAngleBetweenTwoPoints(::_avatar.getPosition(), targetPos);
		local tq = this.Quaternion(tdir, this.Vector3().UNIT_Y);
		local aq = this.Quaternion(this.mRotation, this.Vector3().UNIT_Y);
		local rq = aq.slerp(0.1, tq);
		this.mRotation = this.Math.ConvertQuaternionToRad(rq);

		if (!this.mAutoFollowMoving && d > this.AUTO_FOLLOW_DISTANCE * 2.5)
		{
			this.mAutoFollowMoving = true;
			this.mDirection.z += 1;
			this._moveStarted();
		}
	}

	mStrafeOverride = false;
	mDirection = this.Vector3();
	mAvatarSpeed = 120.0;
	mJumpTimer = null;
	mTurnSpeed = 1.5;
	mTurnDir = 0.0;
	mEnabled = true;
	mPreviousRotation = 0.0;
	mAutoFollowMoving = false;
	mTargetToMoveTo = null;
	mMovingToTarget = false;
	mPositionToMoveTo = null;
	mMovingToPoint = false;
	mRepeatUseageAttempt = false;
}

