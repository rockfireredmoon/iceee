this.require("Controllers/Controller");
class this.Controller.AvatarCreation extends this.Controller.Basic
{
	constructor( pSceneObject )
	{
		this.Controller.Basic.constructor(pSceneObject);
	}

	function onAvatarRightStart( ... )
	{
		if (!this.mRotationActive)
		{
			this.mHeadingChange = this.Math.ConvertPercentageToRad(-0.25);
			this.mRotationActive = true;
			this.mMotionInProgress++;
		}
	}

	function onAvatarLeftStart( ... )
	{
		if (!this.mRotationActive)
		{
			this.mHeadingChange = this.Math.ConvertPercentageToRad(0.25);
			this.mRotationActive = true;
			this.mMotionInProgress++;
		}
	}

	function onAvatarRotateStop( ... )
	{
		if (this.mRotationActive)
		{
			this.mHeadingChange = 0.0;
			local quad = this.mSceneObject.getOrientation();
			this.mSceneObject.mRotation = this.Math.ConvertQuaternionToRad(quad);
			this.mSceneObject.mHeading = this.mSceneObject.mRotation - this.mRotationOffset;
			this.mRotationActive = false;
			this.mMotionInProgress--;
		}
	}

	function onEnterFrame()
	{
		if (!this.mRotationActive && this.mHeadingChange != 0.0)
		{
			this.mHeadingChange = 0.0;
		}

		if (this.mHeadingChange != 0.0)
		{
			local frameChange = this.mHeadingChange * this._deltat / 1000.0;
			this.mSceneObject.mHeading += frameChange;
			this.mSceneObject.mRotation = this.mSceneObject.mHeading;
			this.mSceneObject.setOrientation(this.mSceneObject.mRotation);
		}
	}

}

