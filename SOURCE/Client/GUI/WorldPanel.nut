this.require("GUI/AnchorPanel");
class this.GUI.WorldPanelAnchor 
{
	constructor( so )
	{
		if (so != null)
		{
			this.mSceneObject = so.weakref();
		}
		else
		{
			this.log.warn("Created a WorldPanelAnchor that began as null");
			this.mSceneObject = null;
		}
	}

	function getScreenPosition()
	{
		if (this.mSceneObject == null)
		{
			return null;
		}

		local node = this.mSceneObject.getNode();

		if (!node)
		{
			return null;
		}

		local pos = node.getWorldPosition();

		if (this.mSceneObject.mNamePlatePosition)
		{
			pos += this.mSceneObject.mNamePlatePosition;
			pos.y += 1;
		}
		else
		{
			local bbox = node.getBoundingBox();
			local height = bbox.getMaximum().y - bbox.getMinimum().y;
			pos.y += height;
		}

		if (pos.x == 0 && pos.y == 0 && pos.z == 0)
		{
			return null;
		}

		local spos = this.Screen.worldToScreen(pos);
		return spos;
	}

	function getSize()
	{
		return {
			width = 0,
			height = 0
		};
	}

	function getSceneObject()
	{
		return this.mSceneObject;
	}

	mSceneObject = null;
}

class this.GUI.WorldPanel extends this.GUI.AnchorPanel
{
	constructor( so )
	{
		this.GUI.AnchorPanel.constructor(this.GUI.WorldPanelAnchor(so));
		this.setAppearance("Container");
	}

	function _anchorPosition()
	{
		local pt = {
			x = this.mAnchorPos.x,
			y = this.mAnchorPos.y
		};

		if (this.mAnchor)
		{
			local tpt = this.mAnchor.getScreenPosition();

			if (tpt == null)
			{
				this.setVisible(false);
			}
			else
			{
				this.setVisible(true);
				pt = tpt;
			}
		}

		this.mAnchorPos = pt;
	}

	static mClassName = "WorldPanel";
}

