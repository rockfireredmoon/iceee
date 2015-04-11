this.require("GUI/Component");
class this.GUI.MiniMapSticker extends this.GUI.Component
{
	constructor( name, data, worldx, worldz, map )
	{
		this.GUI.Component.constructor();
		this.mName = name;
		this.mData = data;
		this.mWorldX = worldx;
		this.mWorldZ = worldz;
		this.mMMap = map;
		this.mSceneObject = null;
		this.mFollowing = false;
		this.mSPP = this.GUI.StretchproofPanel();
		this.mSPP.setTextureSize(64, 64);
		this.mSPP.setMaterial(this.mData.material);
		this.mSPP.setTextureOffsets(this.mData.left, this.mData.top);
		this.mSPP.setSize(this.mData.width, this.mData.height);
		this.mSPP.setLayoutExclude(true);
		this.mSPP.setPosition(0, 0);
		this.add(this.mSPP);
	}

	function setFollow( ob )
	{
		if (ob == null)
		{
			this.mSceneObject = null;
			this.mFollowing = false;
		}
		else
		{
			this.mSceneObject = ob.weakref();
			this.mFollowing = true;
		}

		this.update();
	}

	function update()
	{
		if (this.mSceneObject)
		{
			if (!this.mSceneObject.getNode())
			{
				this.setVisible(false);
				this.mMMap.removeSticker(this);
				return;
			}

			local obpos = this.mSceneObject.getPosition();
			this.mWorldX = obpos.x;
			this.mWorldZ = obpos.z;
		}
		else if (this.mFollowing)
		{
			this.print("sticker " + this.mName + " lost target, destroying.");
			this.setVisible(false);
			this.mMMap.removeSticker(this);
			return;
		}

		local center = this.mMMap.viewToPixel(this.mWorldX, this.mWorldZ);
		center.x += this.mData.xoffset;
		center.y += this.mData.yoffset;

		if (center.x + this.mData.width < 0 || center.x > this.mMMap.getWidth() || center.y + this.mData.height < 0 || center.y > this.mMMap.getHeight())
		{
			this.setVisible(false);
			return;
		}

		local nheight = this.mSceneObject.getNode().getBoundingBox().getSize().y;
		local pos = this.mSceneObject.getNode().getPosition();
		local fheight = this.Util.getFloorHeightAt(pos, 1000.0, this.QueryFlags.ANY, false, this.mSceneObject.getNode());
		local under = fheight && fheight > pos.y + nheight;

		if (center.x >= 0 && center.x + this.mData.width <= this.mMMap.getWidth() && center.y >= 0 && center.y + this.mData.height <= this.mMMap.getHeight())
		{
			this.mSPP.setTextureOffsets(this.mData.left, this.mData.top + (under ? 16 : 0));
			this.mSPP.setSize(this.mData.width, this.mData.height);
			this.mSPP.setPosition(center.x, center.y);
			this.setVisible(true);
			return;
		}

		local xoff = 0;
		local yoff = 0;
		local xdim = this.mData.width;
		local ydim = this.mData.height;

		if (center.x < 0)
		{
			xoff = -center.x;
			xdim = this.mData.width + center.x;
			center.x = 0;
		}
		else if (center.x + this.mData.width + 1 > this.mMMap.getWidth())
		{
			xoff = 0;
			xdim = this.mData.width - (center.x + this.mData.width - this.mMMap.getWidth());
		}

		if (center.y < 0)
		{
			yoff = -center.y;
			ydim = this.mData.height + center.y;
			center.y = 0;
		}
		else if (center.y + this.mData.height + 1 > this.mMMap.getHeight())
		{
			yoff = 0;
			ydim = this.mData.height - (center.y + this.mData.height - this.mMMap.getHeight());
		}

		this.mSPP.setTextureOffsets(this.mData.left + xoff, this.mData.top + yoff + (under ? 16 : 0));
		this.mSPP.setPosition(center.x, center.y);
		this.mSPP.setSize(xdim, ydim);
		this.setVisible(true);
	}

	mFollowing = false;
	mSceneObject = null;
	mData = null;
	mWorldX = 0.0;
	mWorldZ = 0.0;
	mSPP = null;
	mMMap = null;
	mName = null;
}

