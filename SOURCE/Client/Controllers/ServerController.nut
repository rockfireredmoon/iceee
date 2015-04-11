this.require("Globals");
this.require("Controllers/Controller");
class this.Controller.Server extends this.Controller.Basic
{
	mLastServerPos = null;
	constructor( so )
	{
		::Controller.Basic.constructor(so);
		this.mSphere = null;
	}

	function _updatePositionDebugObjects()
	{
		if (this.gPositionDebugObjects && !this.mSphere)
		{
			local name = this.mSceneObject.getNodeName() + "/Debug/";
			this.mSphere = this._scene.getRootSceneNode().createChildSceneNode(name + "Sphere");
			local sphereEntity;
			sphereEntity = this._scene.createEntity(name + "Sphere", "Manipulator-SpawnPoint.mesh");
			this.mSphere.attachObject(sphereEntity);
		}

		if (this.mSphere)
		{
			if (!this.gPositionDebugObjects)
			{
				this.mSphere.destroy();
				this.mSphere = null;
			}
			else
			{
				this.mSphere.setPosition(this.mServerPosition);
			}
		}
	}

	function _updateRotationDebugObjects()
	{
		if (this.gPositionDebugObjects && !this.mSphere)
		{
			local name = this.mSceneObject.getNodeName() + "/Debug/";
			this.mSphere = this._scene.getRootSceneNode().createChildSceneNode(name + "Sphere");
			local sphereEntity;
			sphereEntity = this._scene.createEntity(name + "Sphere", "Manipulator-SpawnPoint.mesh");
			this.mSphere.attachObject(sphereEntity);
		}

		if (this.mSphere)
		{
			if (!this.gPositionDebugObjects)
			{
				this.mSphere.destroy();
				this.mSphere = null;
			}
			else
			{
				local value = this.mServerRotation;
				value -= 90 * 3.1400001 / 180;

				if (typeof value == "float" || typeof value == "integer")
				{
					value = this.Quaternion(value, this.Vector3().UNIT_Y);
				}

				this.mSphere.setOrientation(value);
			}
		}
	}

	function onServerPosition( pX, pY, pZ )
	{
		local pos;

		if (!this.mSceneObject.mLastServerUpdate)
		{
			pos = this.Util.safePointOnFloor(this.Vector3(pX, pY, pZ), this.mSceneObject.getNode());
			this.mServerPosition = pos;
			this.mLastServerPos = pos;
			this.mSceneObject.setPosition(pos);
		}
		else
		{
			pos = this.Vector3(pX, pY, pZ);
			this.mLastServerPos = this.mServerPosition;
			this.mServerPosition = pos;
		}

		this._updatePositionDebugObjects();
	}

	function startHeartBeat()
	{
		if (this.mUpdateTimer == null)
		{
			this.mUpdateTimer = ::_messageTimerManager.createHeartBeat(0.0, "", 0.25, "serverVelosityUpdate");
			this.mUpdateTimer.addListener(this.mSceneObject);
			this.mSceneObject.serverVelosityUpdate();
		}
	}

	function stopHeartBeat()
	{
		if (this.mUpdateTimer != null && this.mMotionInProgress == 0)
		{
			this.mUpdateTimer.removeListener(this.mSceneObject);
			this.mUpdateTimer = ::_messageTimerManager.removeTimer(this.mUpdateTimer);
			this.mSceneObject.serverVelosityUpdate();
		}
	}

	function onServerVelocity( pServerHeading, pServerRotation, pServerSpeed )
	{
		if (!this.mSceneObject.mLastServerUpdate)
		{
			this.mServerHeading = pServerHeading;
			this.mSceneObject.mHeading = this.mServerHeading;
			this.mServerRotation = pServerRotation;
			this.mSceneObject.mRotation = this.mServerHeading;
			local rotationQuat = this.Quaternion(this.mSceneObject.mRotation, this.Vector3().UNIT_Y);
			this.mSceneObject.mNode.setOrientation(rotationQuat);
		}
		else
		{
			this.mServerHeading = pServerHeading;
			this.mServerRotation = pServerRotation;
			this.mRotationOffset = pServerRotation - pServerHeading;
			this.mServerSpeed = pServerSpeed;
		}

		this._updateRotationDebugObjects();
	}

	function distanceCheck()
	{
		if (this.mServerPosition == null)
		{
			return;
		}

		if (!this.mSceneObject || !this.mSceneObject.mNode)
		{
			return;
		}

		local horzPosition = this.mSceneObject.mNode.getPosition();
		horzPosition.y = this.mServerPosition.y;
		local distance = this.Math.DetermineDistanceBetweenTwoPoints(horzPosition, this.mServerPosition);

		if (distance >= this.mSnapDistance)
		{
			this.mSceneObject.setPosition(this.mServerPosition);
			this.mSceneObject.mHeading = this.mServerHeading;
			this.mSceneObject.mRotation = this.mServerRotation;
			this.mRotationTarget = this.mSceneObject.mRotation;
			this.mRotationChange = 0.0;
			this.mSceneObject.mSpeed = this.mServerSpeed;
			this.mSceneObject.onEnterFrame();
			return false;
		}

		return true;
	}

	function getData()
	{
		return {
			mServerPosition = this.mServerPosition,
			mServerHeading = this.mServerHeading,
			mServerRotation = this.mServerRotation,
			mServerSpeed = this.mServerSpeed,
			mHeadingTarget = this.mHeadingTarget,
			mHeadingChange = this.mHeadingChange,
			mRotation = this.mRotation
		};
	}

	function setData( data )
	{
		this.mServerPosition = data.mServerPosition;
		this.mServerHeading = data.mServerHeading;
		this.mServerRotation = data.mServerRotation;
		this.mServerSpeed = data.mServerSpeed;
		this.mHeadingTarget = data.mHeadingTarget;
		this.mHeadingChange = data.mHeadingChange;
		this.mRotation = data.mRotation;
		this.distanceCheck();
	}

	function destroy()
	{
		this.Controller.Basic.destroy();

		if (this.mUpdateTimer)
		{
			this.mUpdateTimer.removeListener(this.mSceneObject);
			this.mUpdateTimer = ::_messageTimerManager.removeTimer(this.mUpdateTimer);
		}

		if (this.mSphere)
		{
			this.mSphere.destroy();
			this.mSphere = null;
		}
	}

	function _getRoundedNumber( number )
	{
		if (number >= 0.5)
		{
			return 1;
		}
		else if (number < -0.5)
		{
			return -1;
		}

		return 0;
	}

	function getMovementAnimationName()
	{
		local so = this.Math.rad2deg(this.mServerRotation);
		local sh = this.Math.rad2deg(this.mServerHeading);
		local dif = sh - so;

		if (dif < 0)
		{
			dif += 360;
		}

		if (dif > 22.5 && dif <= 67.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Forward_Left : ::BipedAnimationDef.Forward_Left;
		}

		if (dif > 67.5 && dif <= 112.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Left : ::BipedAnimationDef.StrafeLeft;
		}

		if (dif > 112.5 && dif <= 157.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Backward : ::BipedAnimationDef.Backward_Left;
		}

		if (dif > 157.5 && dif <= 202.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Backward : ::BipedAnimationDef.Backward;
		}

		if (dif > 202.5 && dif <= 247.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Backward : ::BipedAnimationDef.Backward_Right;
		}

		if (dif > 247.5 && dif <= 292.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Right : ::BipedAnimationDef.StrafeRight;
		}

		if (dif > 292.5 && dif <= 337.5)
		{
			return this.mSwimming ? ::BipedAnimationDef.Swim_Forward_Right : ::BipedAnimationDef.Forward_Right;
		}

		return this.mSwimming ? ::BipedAnimationDef.Swim_Forward : ::BipedAnimationDef.Forward;
	}

	mServerPosition = null;
	mServerHeading = 0.0;
	mServerRotation = 0.0;
	mServerSpeed = 0.0;
	mUpdateTimer = null;
	mSnapDistance = this.gServerCreaturePositionTolerance;
	mSphere = null;
}

