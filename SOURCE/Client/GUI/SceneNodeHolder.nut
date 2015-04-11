this.require("GUI/GUI");
class this.GUI.SceneNodeHolder extends this.GUI.Component
{
	static mClassName = "SceneNodeHolder";
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setAppearance("Panel");
		this.mSceneNode = this._scene.createSceneNode();
	}

	function destroy()
	{
		this.mSceneNode.destroy();
		this.mSceneNode = null;
		this.GUI.Component.destroy();
	}

	function getSceneNode()
	{
		return this.mSceneNode;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.log.debug("Adding  " + this.mSceneNode.getName());
		this.Screen.addOverlaySceneNode(this.mSceneNodeOverlay, this.mSceneNode);
	}

	function _removeNotify()
	{
		this.GUI.Component._removeNotify();
		this.log.debug("Removing  " + this.mSceneNode.getName());
		this.Screen.removeOverlaySceneNode(this.mSceneNodeOverlay, this.mSceneNode);
	}

	function _visibilityNotify()
	{
		this.GUI.Component._visibilityNotify();
		this.log.debug("Vis  " + this.mSceneNode.getName() + " = " + this.mIsVisible);

		if (this.mIsVisible)
		{
			if (this.mSceneNode.getParent() == null)
			{
				this.Screen.addOverlaySceneNode(this.mSceneNodeOverlay, this.mSceneNode);
			}
		}
		else if (this.mSceneNode.getParent())
		{
			this.Screen.removeOverlaySceneNode(this.mSceneNodeOverlay, this.mSceneNode);
			this.log.debug(" removed, parent = " + this.mSceneNode.getParent());
			this.mSceneNode.setPosition(this.Vector3(0, -10000000, 0));
		}
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();

		if (!this.mSceneNode)
		{
			return;
		}

		local pos = this.getScreenPosition();
		local dx = pos.x - this.Screen.getWidth() / 2 + this.mWidth / 2;
		local dy = pos.y - this.Screen.getHeight() / 2 + this.mHeight / 2;
		this.mSceneNode.setScale(this.Vector3(1, 1, 1));
		local bb = this.mSceneNode.getBoundingBox();
		local size = bb.getMaximum() - bb.getMinimum();
		local midPoint = bb.getMinimum() + size / 2 - this.mSceneNode.getPosition();
		local near = this._camera.getNearClipDistance();
		local far = this._camera.getFarClipDistance();
		local d = near + size.z / 2;
		local halfW = this.Screen.getWidth() / 2;
		local halfH = this.Screen.getHeight() / 2;
		local _x = dx * d / near;
		local _y = dy * d / near / this.Screen.getHeight();
		local _x = 0;
		local h = near * this.tan(this._camera.getFOVy());
		local h2 = d * this.tan(this._camera.getFOVy());
		this.log.debug("size.y = " + size.y + ", h = " + h + " h2=" + h2);
		local projH = h / h2 * (h / (size.y / 5));
		projH = h / size.y;
		this.log.debug("projH = " + projH);
		local scale = this.mHeight.tofloat() / this.Screen.getHeight() * projH;
		this.log.debug("gah = " + scale);
		this.mSceneNode.setScale(this.Vector3(scale, scale, scale));
		this.mSceneNode.setPosition(this.Vector3(-midPoint.x * scale + _x, -midPoint.y * scale + _y, -near - size.z));
	}

	mSceneNodeOverlay = this.GUI.POPUP_OVERLAY;
	mSceneNode = null;
}

