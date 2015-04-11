this.Controller <- {};
class this.Controller.Basic extends this.MessageBroadcaster
{
	constructor( pSceneObject )
	{
		::MessageBroadcaster.constructor();
		this.mSceneObject = pSceneObject;
	}

	function setFalling( pBool )
	{
		if (this.mFalling != pBool)
		{
			local animHandler = this.mSceneObject.getAnimationHandler();

			if (pBool)
			{
				if (this.mSceneObject.getDistanceToFloor() > 1.0)
				{
					if (animHandler)
					{
						animHandler.onFalling();
					}
				}
			}
			else
			{
				if (animHandler)
				{
					animHandler.onLand();
				}

				if (this.mJumping)
				{
					this.mJumping = false;
					::Audio.playSound("Sound-Land.ogg");
				}

				this.mSceneObject.serverVelosityUpdate();
			}

			this.mFalling = pBool;
		}

		if (!this.mFalling && this.mSceneObject.mSpeed > 0.0)
		{
			local animHandler = this.mSceneObject.getAnimationHandler();

			if (animHandler)
			{
				animHandler.onMove(this.mSceneObject.mSpeed);
			}
		}
	}

	function setEnabled( which )
	{
	}

	function onJump()
	{
		this.mSceneObject.setDistanceToFloor(0.25);
		this.mSceneObject.setVerticalSpeed(this.mJumpSpeed);
		local animHandler = this.mSceneObject.getAnimationHandler();

		if (animHandler)
		{
			animHandler.onJump();
		}

		this.mJumping = true;
		::Audio.playSound("Sound-Jump.ogg");
		this.setFalling(true);
	}

	function onStartSwimming( mWaterElevation )
	{
		this.mSwimming = true;

		if (this.mSceneObject == ::_avatar)
		{
			::_Connection.sendInWater(true);
		}

		this.setFalling(false);
		local animHandler = this.mSceneObject.getAnimationHandler();

		if (animHandler != null)
		{
			animHandler.setSwimming(true);
		}
	}

	function onStopSwimming()
	{
		this.mSwimming = false;

		if (this.mSceneObject == ::_avatar)
		{
			::_Connection.sendInWater(false);
		}

		local animHandler = this.mSceneObject.getAnimationHandler();

		if (animHandler != null)
		{
			animHandler.setSwimming(false);
		}
	}

	function getData()
	{
		return {
			mHeadingTarget = this.mHeadingTarget,
			mHeadingChange = this.mHeadingChange,
			mRotation = this.mRotation
		};
	}

	function canInteractWith( sceneObject )
	{
		return false;
	}

	function setData( data )
	{
		this.mHeadingTarget = data.mHeadingTarget;
		this.mHeadingChange = data.mHeadingChange;
		this.mRotation = data.mRotation;
	}

	function onUpdate()
	{
	}

	function destroy()
	{
	}

	function getMoveActive()
	{
		return this.mMoveActive;
	}

	function getRotationActive()
	{
		return this.mRotationActive;
	}

	function getInMotion()
	{
		return this.getMoveActive() || this.getRotationActive();
	}

	function getMovementAnimationName()
	{
		return ::BipedAnimationDef.Forward;
	}

	mSpeed = 0.0;
	mHeading = 0.0;
	mHeadingTarget = 0.0;
	mHeadingChange = 0.0;
	mRotation = 0.0;
	mRotationOffset = 0.0;
	mRotationTarget = 0.0;
	mRotationChange = 0.0;
	mJumping = false;
	mJumpSpeed = 96.0;
	mFalling = false;
	mSwimming = false;
	mMotionInProgress = 0;
	mMoveActive = false;
	mRotationActive = false;
	mSceneObject = null;
}

