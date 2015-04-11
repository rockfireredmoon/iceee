this.require("GUI/Component");
class this.GUI.AnchorPanel extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();

		if (vargc == 0 || typeof vargv[0] != "instance")
		{
			this.mAnchor = null;
		}
		else
		{
			this.mAnchor = vargv[0];
		}

		this.mGap = {
			x = 3,
			y = 3
		};
		this.mAnchorPos = {
			x = 0,
			y = 0
		};
		this.mAlignment = {
			horz = "left",
			vert = "top"
		};
		this.mOffset = {
			x = 0,
			y = 0
		};
		this.setAppearance("Panel");
		::_root.addListener(this);
		this.setOverlay(this.GUI.POPUP_OVERLAY);
	}

	function setGap( ... )
	{
		if (typeof vargv[0] == "table")
		{
			this.mGap = {
				x = vargv[0].x,
				y = vargv[0].y
			};
		}
		else if (typeof vargv[0] == "integer")
		{
			this.mGap = {
				x = vargv[0],
				y = vargv[1]
			};
		}

		this._forceCheck();
	}

	function getGap()
	{
		return {
			x = this.mGap.x,
			y = this.mGap.y
		};
	}

	function setAlignment( ... )
	{
		if (typeof vargv[0] == "table")
		{
			this.mAlignment = {
				horz = vargv[0].horz,
				vert = vargv[0].vert
			};
		}
		else if (typeof vargv[0] == "string")
		{
			this.mAlignment = {
				horz = vargv[0],
				vert = vargv[1]
			};
		}

		this._forceCheck();
	}

	function _forceCheck()
	{
		this.mAnchorPos.x = -1;
		this.mAnchorPos.y = -1;
		this.update();
	}

	function getAlignment()
	{
		return {
			horz = this.mAlignment.horz,
			vert = this.mAlignment.vert
		};
	}

	function getHorzAlignment()
	{
		return this.mAlignment.horz;
	}

	function getVertAlignment()
	{
		return this.mAlignment.vert;
	}

	function setAnchor( pAnchor )
	{
		this.mAnchor = pAnchor;
		this._forceCheck();
	}

	function getAnchor()
	{
		return this.mAnchor;
	}

	function setAddAnchorSize( pWidthBool, pHeightBool )
	{
		this.mAddAnchorWidth = pWidthBool;
		this.mAddAnchorHeight = pHeightBool;
		this._forceCheck();
	}

	function getAddAnchorWidth()
	{
		return this.mAddAnchorWidth;
	}

	function getAddAnchorHeight()
	{
		return this.mAddAnchorHeight;
	}

	function setReverseAlignment( pHorzBool, pVertBool )
	{
		this.mReverseHorz = pHorzBool;
		this.mReverseVert = pVertBool;
		this._forceCheck();
	}

	function getReverseAlignment( pBool )
	{
		return this.mReverseAlignment;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
	}

	function _removeNotify()
	{
		this.GUI.Component._removeNotify();
	}

	function _anchorPosition()
	{
		local pt = {
			x = this.mAnchorPos.x,
			y = this.mAnchorPos.y
		};

		if (this.mAnchor)
		{
			pt = this.mAnchor.getScreenPosition();
		}

		this.mAnchorPos = pt;
	}

	function _calcOffset( horz, vert )
	{
		local sz = this.getSize();
		local psz = {
			width = 0,
			height = 0
		};

		if (this.mAnchor)
		{
			psz = this.mAnchor.getSize();
		}

		switch(horz)
		{
		case "left":
			if (this.mAddAnchorWidth)
			{
				this.mOffset.x = this.mGap.x + psz.width;
			}
			else
			{
				this.mOffset.x = this.mGap.x;
			}

			break;

		case "center":
			if (this.mAddAnchorWidth)
			{
				this.mOffset.x = psz.width / 2 - sz.width / 2;
			}
			else
			{
				this.mOffset.x = sz.width / 2 * -1;
			}

			break;

		case "right":
			this.mOffset.x = (sz.width + this.mGap.x) * -1;
			break;
		}

		switch(vert)
		{
		case "top":
			if (this.mAddAnchorHeight)
			{
				this.mOffset.y = this.mGap.y + psz.height;
			}
			else
			{
				this.mOffset.y = this.mGap.y;
			}

			break;

		case "center":
			if (this.mAddAnchorHeight)
			{
				this.mOffset.y = psz.height / 2 - sz.height / 2 * -1;
			}
			else
			{
				this.mOffset.y = sz.height / 2 * -1;
			}

			break;

		case "bottom":
			this.mOffset.y = (sz.height + this.mGap.y) * -1;
			break;
		}
	}

	function _stayOnScreen()
	{
		local pt = {
			x = this.mAnchorPos.x,
			y = this.mAnchorPos.y
		};
		local sz = this.getSize();
		local psz;
		local horz = this.mAlignment.horz;
		local vert = this.mAlignment.vert;
		this.mOnScreen = true;
		psz = {
			width = ::Screen.getWidth(),
			height = ::Screen.getHeight()
		};

		if (this.mReverseHorz)
		{
			switch(this.mAlignment.horz)
			{
			case "left":
				if (pt.x + this.mOffset.x + sz.width > psz.width)
				{
					horz = "right";
				}

				break;

			case "right":
				if (pt.x + this.mOffset.x < 0)
				{
					horz = "left";
				}

				break;
			}
		}

		if (this.mReverseVert)
		{
			switch(this.mAlignment.vert)
			{
			case "top":
				if (pt.y + this.mOffset.y + sz.height > psz.height)
				{
					vert = "bottom";
				}

				break;

			case "bottom":
				if (pt.y + this.mOffset.y < 0)
				{
					vert = "top";
				}

				break;
			}
		}

		this._calcOffset(horz, vert);
		pt.x = pt.x + this.mOffset.x;
		pt.y = pt.y + this.mOffset.y;

		if (pt.x + sz.width > psz.width)
		{
			pt.x = psz.width - sz.width;
			this.mOnScreen = false;
		}
		else if (pt.x < 0)
		{
			pt.x = 0;
			this.mOnScreen = false;
		}

		if (pt.y + sz.height > psz.height)
		{
			pt.y = psz.height - sz.height;
			this.mOnScreen = false;
		}
		else if (pt.y < 0)
		{
			pt.y = 0;
			this.mOnScreen = false;
		}

		return pt;
	}

	function isObjectOnScreen()
	{
		return this.mOnScreen;
	}

	function onMouseMoved( evt )
	{
		if (!this.mAnchor)
		{
			this.mAnchorPos = {
				x = evt.x,
				y = evt.y
			};
			this.update();
		}
	}

	function setSize( ... )
	{
		local oldW = this.mWidth;
		local oldH = this.mHeight;
		local w;
		local h;

		if (vargc == 1 && (typeof vargv[0] == "table" || typeof vargv[0] == "instance"))
		{
			w = vargv[0].width;
			h = vargv[0].height;
		}
		else if (vargc == 2)
		{
			w = vargv[0];
			h = vargv[1];
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setSize()");
		}

		this.GUI.Component.setSize(w, h);
		this._forceCheck();
	}

	function update()
	{
		if (!this.mIsRealized)
		{
			return;
		}

		local oldX = -1;
		local oldY = -1;

		if (this.mAnchorPos != null)
		{
			oldX = this.mAnchorPos.x;
			oldY = this.mAnchorPos.y;
		}

		this._anchorPosition();

		if (oldX != this.mAnchorPos.x || oldY != this.mAnchorPos.y)
		{
			local pt = this._stayOnScreen();
			this.setPosition(pt.x, pt.y);
		}
	}

	function onOutsideClick( evt )
	{
		this.setVisible(false);
	}

	function onExitFrame()
	{
		this.update();
	}

	function validate()
	{
		if (this.mAlignment)
		{
			this.update();
		}

		this.GUI.Component.validate();
	}

	function destroy()
	{
		this.setOverlay(null);
		this.GUI.Component.destroy();
		::_root.removeListener(this);
		return null;
	}

	mAnchorPos = null;
	mAddAnchorWidth = true;
	mAddAnchorHeight = false;
	mGap = null;
	mOffset = null;
	mAnchor = null;
	mAlignment = null;
	mReverseHorz = true;
	mReverseVert = true;
	mOnScreen = true;
	static mClassName = "AnchorPanel";
}

