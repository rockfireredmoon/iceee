this.require("GUI/Panel");
class this.GUI.StretchproofPanel extends this.GUI.Panel
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.FlowLayout());
		this.mAppearance = "StretchproofPanel";
		this.setTextureSize(64, 64);

		if (vargc > 0)
		{
			if (typeof vargv[0] == "instance" && (vargv[0] instanceof this.GUI.LayoutManager))
			{
				this.setLayoutManager(vargv[0]);
			}
			else if (typeof vargv[0] == "string")
			{
				this.mAppearance = vargv[0];
			}
		}

		this.mStretchproofMaterial = "DefaultSkin/StretchproofPanel";
	}

	function setMaterial( pMaterial )
	{
		this.mStretchproofMaterial = pMaterial;

		if (this.mIsRealized)
		{
			this.mWidget.setParam("border_material", this.mStretchproofMaterial);
		}
	}

	function setTextureSize( ... )
	{
		local w;
		local h;

		if (vargc == 1 && (typeof vargv[0] == "table" || typeof vargv[0] == "instance"))
		{
			w = vargv[0].width;
			h = vargv[0].height;
		}
		else if (vargc == 2)
		{
			w = vargv[0].tointeger();
			h = vargv[1].tointeger();
		}
		else
		{
			throw this.Exception("Invalid arguments to StretchProofPanel.setTextureSize()");
		}

		if (w <= 0)
		{
			w = 1;
		}

		if (h <= 0)
		{
			h = 1;
		}

		this.mTextureWidth = w;
		this.mTextureHeight = h;
	}

	function _reshapeNotify()
	{
		this.GUI.Panel._reshapeNotify();
		this._recalc();
	}

	function _recalc()
	{
		if (this.mWidget != null)
		{
			this.mWidget.setParam("border_size", "0 " + this.getWidth() + " 0 0");
			this.mWidget.setParam("border_right_uv", this.format("%0.4f", 1.0 * this.mXOffset / this.mTextureWidth) + " " + this.format("%0.4f", 1.0 * this.mYOffset / this.mTextureHeight) + " " + this.format("%0.4f", 1.0 * (this.mClampSize ? this.mClampX : this.getWidth()) / this.mTextureWidth + 1.0 * this.mXOffset / this.mTextureWidth) + " " + this.format("%0.4f", 1.0 * (this.mClampSize ? this.mClampY : this.getHeight()) / this.mTextureHeight + 1.0 * this.mYOffset / this.mTextureHeight));
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.setParam("border_material", this.mStretchproofMaterial);
		this._recalc();
	}

	function setEffectiveSize( w, h )
	{
		if (w <= 0 || h <= 0)
		{
			this.mClampSize = false;
			this._recalc();
		}
		else
		{
			this.mClampX = w;
			this.mClampY = h;
			this.mClampSize = true;
			this._recalc();
		}
	}

	function setTextureOffsets( x, y )
	{
		this.mXOffset = x;
		this.mYOffset = y;
		this._recalc();
	}

	mClampSize = false;
	mClampX = 1;
	mClampY = 1;
	mXOffset = 0.0;
	mYOffset = 0.0;
	mTextureWidth = 1;
	mTextureHeight = 1;
	mStretchproofMaterial = null;
	static mClassName = "StretchproofPanel";
}

