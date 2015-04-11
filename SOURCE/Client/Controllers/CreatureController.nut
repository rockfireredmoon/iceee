this.require("Globals");
this.require("Controllers/ServerController");
class this.Controller.Creature extends this.Controller.Server
{
	constructor( pSceneObject )
	{
		::Controller.Server.constructor(pSceneObject);
		this.mRing = null;
		this._updatePositionDebugRing();
	}

	function _updatePositionDebugRing()
	{
		if (this.gPositionDebugObjects && !this.mRing)
		{
			local name = this.mSceneObject.getNodeName() + "/Debug/";
			this.mRing = this._scene.getRootSceneNode().createChildSceneNode(name + "Ring");
			local ringEntity;
			ringEntity = this._scene.createEntity(name + "Ring", "Ring.mesh");
			this.mRing.attachObject(ringEntity);
		}
		else if (!this.gPositionDebugObjects && this.mRing)
		{
			this.mRing.destroy();
			this.mRing = null;
		}
	}

	function onServerPosition( pX, pY, pZ )
	{
		this.Controller.Server.onServerPosition(pX, pY, pZ);
		this.updatePrediction();
		this.distanceCheck();
	}

	function onServerVelocity( pServerHeading, pServerRotation, pServerSpeed )
	{
		this.mLastServerHeading = this.mServerHeading;
		this.Controller.Server.onServerVelocity(pServerHeading, pServerRotation, pServerSpeed);
		this.mRotationTarget = this.mServerRotation;
		this.mRotationChange = ::Math.CalcuateShortestBetweenTwoRads(this.mSceneObject.mRotation, this.mRotationTarget) / 0.25;
		this.mRotationOffset = this.mSceneObject.mRotation - this.mSceneObject.mHeading;
		this.updatePrediction();
	}

	function onUpdate()
	{
		this.updatePrediction();
	}

	function updatePrediction()
	{
		if (!this.mServerPosition)
		{
			this.log.warn("No server position for " + this.mSceneObject);
			return;
		}

		local horzPosition = this.mSceneObject.mNode.getPosition();

		if (!this.mSceneObject.isPlayer() && this.mServerPosition.y - horzPosition.y > 15)
		{
			local newPos = this.Vector3(horzPosition.x, this.mServerPosition.y, horzPosition.z);
			this.mSceneObject.setPosition(::Util.safePointOnFloor(newPos));
		}

		if (!this.distanceCheck())
		{
			return;
		}

		horzPosition.y = this.mServerPosition.y;

		if (!this.mSceneObject.mLastServerUpdate)
		{
			horzPosition = this.Util.safePointOnFloor(this.Vector3(horzPosition.x, horzPosition.y, horzPosition.z), this.mSceneObject.getNode());
			this.mSceneObject.mHeading = this.mServerHeading;
			this.mSceneObject.mSpeed = this.mServerSpeed;
			this.mSceneObject.setPosition(this.mServerPosition);
		}
		else
		{
			local predictionVector = ::Math.ConvertRadToVector(this.mServerHeading);
			predictionVector = predictionVector * this.mServerSpeed * this.mPredictionDistance;
			local mPredictionPosition = predictionVector + this.mServerPosition;
			local rawPredictionDistance = ::Math.DetermineDistanceBetweenTwoPoints(horzPosition, mPredictionPosition);
			local predictionDistance = rawPredictionDistance * 0.75;
			local lastHeading = this.mSceneObject.mHeading;
			this.mSceneObject.mHeading = rawPredictionDistance > 0.0099999998 ? ::Math.DetermineRadAngleBetweenTwoPoints(this.mSceneObject.getPosition(), mPredictionPosition) : this.mServerHeading;

			if (this.fabs(predictionDistance) <= 4.0 && this.mServerSpeed == 0)
			{
				this.mSceneObject.mSpeed = 0.0;
			}
			else if (this.mServerSpeed >= 0)
			{
				this.mSceneObject.mSpeed = this.fabs(predictionDistance) / this.mPredictionDistance;
			}
			else if (this.mServerSpeed < 0)
			{
				this.mSceneObject.mSpeed = this.fabs(predictionDistance) / this.mPredictionDistance * -1;
			}

			this._updatePositionDebugRing();

			if (this.mRing)
			{
				local ringPos = this.Vector3(mPredictionPosition.x, 10000.0, mPredictionPosition.z);
				local y = this.Util.getFloorHeightAt(ringPos, 10.0, this.QueryFlags.FLOOR);

				if (y != null)
				{
					ringPos.y = y;
				}

				this.mRing.setPosition(ringPos);
			}
		}

		if (this.mSceneObject.mAnimationHandler)
		{
			if (this.mSceneObject.mSpeed > 0.0 && !this.mSceneObject.mAnimationHandler.mMoving)
			{
				this.mSceneObject.mAnimationHandler.onMove(this.mSceneObject.mSpeed);
			}
			else if (this.mSceneObject.mSpeed == 0.0 && this.mSceneObject.mAnimationHandler.mMoving)
			{
				this.mSceneObject.mAnimationHandler.onStop();
			}
		}
	}

	function onEnterFrame()
	{
		if (this.mServerPosition == null)
		{
			return;
		}

		local horzPosition = this.mSceneObject.getPosition();
		horzPosition.y = this.mServerPosition.y;

		if (this.mSceneObject.isDead() || this.mServerSpeed < 0.001 && ::Math.DetermineDistanceBetweenTwoPoints(horzPosition, this.mServerPosition) < 5.0 && this.mSceneObject.getSpeed() != 0.0)
		{
			if (this.mSceneObject.mAnimationHandler)
			{
				this.mSceneObject.mAnimationHandler.onStop();
			}

			this.mSceneObject.mSpeed = 0.0;
		}

		if (this.mRotationChange != 0.0)
		{
			local frameChange = this.mRotationChange * (this._deltat / 1000.0);
			this.mSceneObject.mRotation += frameChange;
			local remaining = this.Math.CalcuateShortestBetweenTwoRads(this.mSceneObject.mRotation, this.mRotationTarget);

			if (this.fabs(remaining) <= this.fabs(frameChange))
			{
				this.mSceneObject.mRotation = this.mRotationTarget;
				this.mRotationChange = 0.0;
			}

			this.mRotationOffset = this.mServerRotation - this.mSceneObject.mHeading;
			this.mSceneObject.setOrientation(this.mSceneObject.mRotation);

			if (this.mSceneObject.mAnimationHandler && this.mSceneObject.mSpeed != 0.0)
			{
				this.mSceneObject.mAnimationHandler.onMove(this.mSceneObject.mSpeed);
			}
		}
	}

	function setData( data )
	{
		this.Controller.Server.setData(data);
		this.updatePrediction();
	}

	function destroy()
	{
		this.Controller.Server.destroy();

		if (this.mRing)
		{
			this.mRing.destroy();
			this.mRing = null;
		}
	}

	mInterpolateSpeed = 25.0;
	mLastServerHeading = 0.0;
	mPredictionDistance = 0.5;
	mPredictionPosition = null;
	mSnapDistance = this.gServerCreaturePositionTolerance;
	mRing = null;
}

