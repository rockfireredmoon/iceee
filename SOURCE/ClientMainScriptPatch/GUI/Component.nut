this.require("GUI/LayoutManager");
this.require("GUI/Manager");
this.require("GUI/Font");
this.require("GUI/GUI");
class this.GUI.Component 
{
	mName = "";
	mDebugName = null;
	mWidth = 0;
	mHeight = 0;
	mMinimumSize = null;
	mPreferredSize = null;
	mMaxWidth = null;
	mMaxHeight = null;
	mIsResize = false;
	mX = 0;
	mY = 0;
	mVAlign = "top";
	mHAlign = "left";
	mIsVisible = false;
	mIsValid = false;
	mIsRealized = false;
	mIsSticky = false;
	mParentComponent = null;
	mTooltip = null;
	mChildTooltips = null;
	mChildrenInheritTooltip = false;
	mFont = null;
	mFontColor = null;
	mAppearance = null;
	mWidget = null;
	mSkin = null;
	mIsEnabled = true;
	mBlendColor = null;
	mCached = false;
	mData = null;
	mIsDefaultPositonSet = false;
	mDefaultXPosition = 0;
	mDefaultYPosition = 0;
	components = null;
	insets = null;
	mLayoutConstraints = null;
	mLayoutManager = null;
	mOverlay = null;
	mSelection = null;
	mLayoutExclude = false;
	mMaterial = null;
	mMaterialIsSkinned = null;
	mScroll = null;
	mPassThru = false;
	mGenTooltipWithPassThru = false;
	mMessageBroadcaster = null;
	mHidden = null;
	mPressedLeft = false;
	mKeyFocus = false;
	static mClassName = "Component";
	constructor( ... )
	{
		this.mName = this.mClassName + "_" + this.GUI._NextID++;
		this.mWidth = 0;
		this.mHeight = 0;
		this.mX = 0;
		this.mY = 0;
		this.mIsValid = false;
		this.mIsVisible = true;
		this.mIsRealized = false;
		this.mIsSticky = false;
		this.mParentComponent = null;
		this.mAppearance = null;
		this.mTooltip = null;
		this.mChildTooltips = [];
		this.mFont = null;
		this.mFontColor = null;
		this.mWidget = null;
		this.mSkin = this.GUI.CurrentSkin;
		this.mBlendColor = null;
		this.mPassThru = false;
		this.mChildrenInheritTooltip = false;
		this.mMaterial = null;
		this.mMaterialIsSkinned = true;

		if (vargc > 0 && vargv[0] != null && (vargv[0] instanceof this.GUI.LayoutManager))
		{
			this.mLayoutManager = vargv[0];
		}
		else
		{
			this.mLayoutManager = null;
		}

		this.components = [];
		this.insets = {
			top = 0,
			right = 0,
			bottom = 0,
			left = 0
		};
	}

	function setOpts( opts )
	{
		if ("height" in opts)
		{
			this.mHeight = opts.height;
		}

		if ("width" in opts)
		{
			this.mWidth = opts.width;
		}

		if ("x" in opts)
		{
			this.mX = opts.x;
		}

		if ("y" in opts)
		{
			this.mY = opts.y;
		}

		if ("font" in opts)
		{
			this.setFont(opts.font);
		}

		if ("fontColor" in opts)
		{
			this.setFontColor(opts.fontColor);
		}

		if ("blendColor" in opts)
		{
			this.setBlendColor(opts.blendColor);
		}

		if ("enabled" in opts)
		{
			this.setEnabled(opts.enabled);
		}

		if ("visible" in opts)
		{
			this.setVisible(opts.visible);
		}

		if ("appearance" in opts)
		{
			this.setAppearance(opts.appearance);
		}

		if ("selection" in opts)
		{
			this.setSelection(opts.selection);
		}
	}

	static function _create( component )
	{
		if (component.mIsRealized)
		{
			return false;
		}

		component._addNotify();

		if (!component.mIsRealized)
		{
			throw this.Exception("_addNotify() did not realize component");
		}

		return true;
	}

	function _recreate()
	{
		if (this.mIsRealized)
		{
			this._removeNotify();
			this._addNotify();
		}
	}

	function setAppearance( appearance )
	{
		this.setSelection(false);
		this.mAppearance = appearance;
		this._recreate();
	}

	function getAppearance()
	{
		return this.mAppearance;
	}

	function setSkin( name )
	{
		this.mSkin = name;
		this._recreate();
	}

	function setWidth( width )
	{
		this.setSize(width, this.mHeight);
	}

	function getWidth()
	{
		return this.mWidth;
	}

	function setHeight( height )
	{
		this.setSize(this.mWidth, height);
	}

	function getHeight()
	{
		return this.mHeight;
	}

	function move( x, y )
	{
		local pos = this.getPosition();
		this.setPosition(pos.x + x, pos.y + y);
	}

	function setPosition( ... )
	{
		local x;
		local y;

		if (vargc == 0)
		{
			return;
		}

		if (vargc == 1 && typeof vargv[0] == "table")
		{
			x = vargv[0].x;
			y = vargv[0].y;
		}
		else if (vargc == 2)
		{
			x = vargv[0];
			y = vargv[1];
		}
		else
		{
			return;
		}

		local new_x = x.tointeger();
		local new_y = y.tointeger();

		if (new_x == this.mX && new_y == this.mY)
		{
			return;
		}

		this.mX = new_x;
		this.mY = new_y;

		if (!this.mIsDefaultPositonSet)
		{
			this.mIsDefaultPositonSet = true;
			this.mDefaultXPosition = this.mX;
			this.mDefaultYPosition = this.mY;
		}

		if (this.mIsRealized)
		{
			this._reshapeNotify();
		}
	}

	function getPosition()
	{
		return {
			x = this.mX,
			y = this.mY
		};
	}

	function onScreenDefaultPositon()
	{
		if (this.mIsDefaultPositonSet)
		{
			this.setPosition(this.mDefaultXPosition, this.mDefaultYPosition);
		}
	}

	function onScreenResize()
	{
		if (this.USE_OLD_SCREEN)
		{
			this.oldScreenResize();
		}
		else
		{
			this.onNewScreenResize();
		}
	}

	function oldScreenResize()
	{
		local oldPos = this.getPosition();
		local screenWidth = ::Screen.getWidth().tofloat();
		local screenHeight = ::Screen.getHeight().tofloat();
		local width = this.getWidth().tofloat();
		local height = this.getHeight().tofloat();
		local oldW = this._screenResizeRelay.getOldWidth().tofloat() - width;
		local oldH = this._screenResizeRelay.getOldHeight().tofloat() - height;
		local xPerc = oldW > 0.0 ? oldPos.x / oldW : 0.0;
		local yPerc = oldH > 0.0 ? oldPos.y / oldH : 0.0;
		local nX = (xPerc * (screenWidth - width)).tointeger();
		local nY = (yPerc * (screenHeight - height)).tointeger();
		this.setPosition(nX, nY);
		this.keepOnScreen();
	}

	function onNewScreenResize()
	{
		local newPositon = ::Util.getUpdateResizePosition(::_screenResizeRelay.getOldWidth().tofloat(), ::_screenResizeRelay.getOldHeight().tofloat(), this.getPosition().x.tofloat(), this.getPosition().y.tofloat(), this.getWidth().tofloat(), this.getHeight().tofloat() / 2);
		this.setPosition(this.newPosition.x, this.newPosition.y);
		this.keepOnScreen();
	}

	function containsCursorPos( cursorPos )
	{
		local pos = this.getScreenPosition();

		if (cursorPos.x < pos.x)
		{
			return false;
		}

		if (cursorPos.y < pos.y)
		{
			return false;
		}

		if (cursorPos.x >= pos.x + this.mWidth)
		{
			return false;
		}

		if (cursorPos.y >= pos.y + this.mHeight)
		{
			return false;
		}

		return true;
	}

	function getScreenPosition()
	{
		local ScreenPos = {
			x = 0,
			y = 0
		};
		local Pos = this.getPosition();
		local component = this;
		local parentComp = this.mParentComponent;
		local ptsz;
		local scsz = {
			width = ::Screen.getWidth(),
			height = ::Screen.getHeight()
		};

		while (component.mParentComponent)
		{
			switch(component.getStickyHorz())
			{
			case "left":
				ScreenPos.x += Pos.x;
				break;

			case "right":
				ptsz = parentComp.getSize();
				ScreenPos.x += Pos.x + ptsz.width;
				break;

			case "center":
				ptsz = parentComp.getSize();
				ScreenPos.x += Pos.x + ptsz.width / 2;
				break;
			}

			switch(component.getStickyVert())
			{
			case "top":
				ScreenPos.y += Pos.y;
				break;

			case "bottom":
				ptsz = parentComp.getSize();
				ScreenPos.y += Pos.y + ptsz.height;
				break;

			case "center":
				ptsz = parentComp.getSize();
				ScreenPos.y += Pos.y + ptsz.height / 2;
				break;
			}

			component = parentComp;
			Pos = component.getPosition();
			parentComp = component.mParentComponent;
		}

		switch(component.getStickyHorz())
		{
		case "left":
			ScreenPos.x += Pos.x;
			break;

		case "right":
			ScreenPos.x += Pos.x + scsz.width;
			break;

		case "center":
			ScreenPos.x += Pos.x + scsz.width / 2;
			break;
		}

		switch(component.getStickyVert())
		{
		case "top":
			ScreenPos.y += Pos.y;
			break;

		case "bottom":
			ScreenPos.y += Pos.y + scsz.height;
			break;

		case "center":
			ScreenPos.y += Pos.y + scsz.height / 2;
			break;
		}

		return ScreenPos;
	}

	function getToplevelParent()
	{
		if (this.mParentComponent == null)
		{
			return null;
		}

		local p = this.mParentComponent;

		while (p.mParentComponent)
		{
			p = p.mParentComponent;
		}

		return p;
	}

	function setFontColor( pColor )
	{
		if (pColor != null)
		{
			if (typeof pColor == "string")
			{
				this.mFontColor = ::Color(pColor);
			}
			else if (typeof pColor == "instance" && (pColor instanceof this.Color))
			{
				this.mFontColor = ::Color(pColor.toHexString());
			}
			else
			{
				throw this.Exception("Invalid color value: " + pColor);
			}

			if (this.mWidget != null)
			{
				local fontColorString = this.mFontColor.r.tostring() + ", " + this.mFontColor.g.tostring() + ", " + this.mFontColor.b.tostring() + ", " + this.mFontColor.a.tostring();
				this.mWidget.setParam("colour_top", fontColorString);
				this.mWidget.setParam("colour_bottom", fontColorString);
			}
		}
		else
		{
			this.mFontColor = null;
		}

		this.invalidate();
	}

	function getFontColor()
	{
		if (this.mFontColor)
		{
			return this.mFontColor;
		}

		if (this.mParentComponent)
		{
			return this.mParentComponent.getFontColor();
		}

		return ::Color(this.GUI.DefaultFontColor);
	}

	function setBlendColor( pColor )
	{
		if (pColor != null)
		{
			if (typeof pColor == "string")
			{
				this.mBlendColor = pColor;
			}
			else if (typeof pColor == "instance" && (pColor instanceof this.Color))
			{
				this.mBlendColor = pColor;
			}
			else
			{
				throw this.Exception("Invalid color value: " + pColor);
			}

			if (this.mWidget != null)
			{
				local blendColorString = this.mBlendColor.r.tostring() + ", " + this.mBlendColor.g.tostring() + ", " + this.mBlendColor.b.tostring() + ", " + this.mBlendColor.a.tostring();
				this.mWidget.setParam("color", blendColorString);
			}
		}
		else
		{
			this.mBlendColor = null;
		}

		this.invalidate();
	}

	function getBlendColor()
	{
		if (this.mBlendColor == null)
		{
			return this.Color(1.0, 1.0, 1.0, 1.0);
		}

		return this.mBlendColor;
	}

	function setSticky( pHorz, pVert )
	{
		if (pHorz == null)
		{
			this.mHAlign = null;
		}
		else if (pHorz.tolower() == "left" || pHorz.tolower() == "center" || pHorz.tolower() == "right" || pHorz.tolower() == null)
		{
			this.mHAlign = pHorz.tolower();
		}

		if (pVert == null)
		{
			this.mVAlign = null;
		}
		else if (pVert.tolower() == "top" || pVert.tolower() == "center" || pVert.tolower() == "bottom" || pVert.tolower() == null)
		{
			this.mVAlign = pVert.tolower();
		}

		if (!this.mHAlign)
		{
			this.mHAlign = "left";
		}

		if (!this.mVAlign)
		{
			this.mVAlign = "top";
		}

		if (this.mWidget != null)
		{
			this.mWidget.setParam("horz_align", "" + this.mHAlign);
			this.mWidget.setParam("vert_align", "" + this.mVAlign);
		}

		if (this.mHAlign || this.mVAlign)
		{
			this.mIsSticky = true;
		}

		this.invalidate();
	}

	function isSticky()
	{
		return this.mIsSticky;
	}

	function getStickyHorz()
	{
		return this.mHAlign;
	}

	function getStickyVert()
	{
		return this.mVAlign;
	}

	function setSize( ... )
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
			w = vargv[0];
			h = vargv[1];
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setSize()");
		}

		local new_w = w.tointeger();
		local new_h = h.tointeger();

		if (this.mWidth == new_w && this.mHeight == new_h)
		{
			return;
		}

		this.mWidth = new_w;
		this.mHeight = new_h;

		if (this.mIsRealized)
		{
			this._reshapeNotify();
		}

		if (this.components.len() > 0 || this.mOverlay)
		{
			this.invalidate();
		}
	}

	function setMaximumSize( ... )
	{
		local w;
		local h;

		if (vargc == 1 && (typeof vargv[0] == "table" || typeof vargv[0] == "instance"))
		{
			if (vargv[0].width != null)
			{
				w = vargv[0].width;
			}
			else
			{
				w = null;
			}

			if (vargv[0].height != null)
			{
				h = vargv[1];
			}
			else
			{
				h = vargv[1];
			}
		}
		else if (vargc == 2)
		{
			if (vargv[0] != null)
			{
				w = vargv[0];
			}
			else
			{
				w = null;
			}

			if (vargv[1] != null)
			{
				h = vargv[1];
			}
			else
			{
				h = vargv[1];
			}
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setSize()");
		}

		if (w != null)
		{
			w = w.tointeger();
		}

		if (h != null)
		{
			h = h.tointeger();
		}

		this.mMaxWidth = w;
		this.mMaxHeight = h;
		this.invalidate();
	}

	function getMaxSize()
	{
		if (this.isResize())
		{
			local w = this.mMaxWidth;
			local h = this.mMaxHeight;
			return {
				width = w,
				height = h
			};
		}
		else
		{
			return {
				width = 60000,
				height = 60000
			};
		}
	}

	function getSize()
	{
		return {
			width = this.mWidth,
			height = this.mHeight
		};
	}

	function setResize( pBool )
	{
		this.mIsResize = pBool;
	}

	function isResize()
	{
		return this.mIsResize;
	}

	function setSelection( pBool )
	{
		if (pBool)
		{
			if (this.mSelection)
			{
				this.remove(this.mSelection);
			}

			this.mSelection = ::GUI.Component();
			this.mSelection.setAppearance(this.mAppearance + "/Selection");
			this.mSelection.setPosition(0, 0);
			this.mSelection.setSize(this.mWidth, this.mHeight);
			this.mSelection.setLayoutExclude(true);
			this.setSelectionVisible(false);
			this.add(this.mSelection);
		}
		else if (this.mSelection && !pBool)
		{
			this.remove(this.mSelection);
			this.mSelection = null;
		}
	}

	function getSelection()
	{
		if (this.mSelection)
		{
			return this.mSelection;
		}
	}

	function setSelectionVisible( pBool )
	{
		if (this.mSelection)
		{
			this.mSelection.setVisible(pBool);
		}
	}

	function isSelectionVisible()
	{
		if (this.mSelection)
		{
			return this.mSelection.isVisible();
		}
	}

	function setMaterial( pMaterial, ... )
	{
		if (vargc > 0)
		{
			this.mMaterialIsSkinned = vargv[0];
		}
		else
		{
			this.mMaterialIsSkinned = true;
		}

		this.mMaterial = pMaterial;

		if (this.mIsRealized)
		{
			local materialName;

			if (this.mSkin && this.mMaterialIsSkinned)
			{
				materialName = this.mSkin + "/" + this.mMaterial;
			}
			else
			{
				materialName = this.mMaterial;
			}

			this.mWidget.setMaterialName(materialName);
		}
	}

	function getMaterial()
	{
		return this.mMaterial;
	}

	function invalidate()
	{
		if (this.mIsValid)
		{
			this.mIsValid = false;

			if (this.mParentComponent)
			{
				this.mParentComponent.invalidate();
			}

			if (this.mOverlay)
			{
				this.GUI._Manager.queueValidation(this);
			}
		}
	}

	function isLayoutManaged()
	{
		return !this.mLayoutExclude && (this.mIsVisible || this.mHidden);
	}

	function setLayoutExclude( pBool )
	{
		this.mLayoutExclude = pBool;
	}

	function getLayoutExclude()
	{
		return this.mLayoutExclude;
	}

	function setVisible( value )
	{
		if (typeof value != "bool")
		{
			throw this.Exception("Invalid visibility value: " + value);
		}

		if (this.mHidden != null)
		{
			this.mHidden = value;
			return;
		}

		if (this.mIsVisible == value)
		{
			return;
		}

		this.mIsVisible = value;

		if (this.mIsVisible)
		{
			if (this.mParentComponent)
			{
				if (this.mParentComponent.mIsRealized && !this.mIsRealized)
				{
					this._addNotify();

					if (!this.mIsRealized)
					{
						throw this.Exception("_addNotify() did not set realized!");
					}
				}

				this.mParentComponent.invalidate();
			}
			else
			{
				this._addToOverlay();
			}
		}
		else if (this.mIsRealized)
		{
			this._removeNotify();
		}

		if (this.mIsRealized)
		{
			this._visibilityNotify();
		}
	}

	function isVisible()
	{
		return this.mIsVisible;
	}

	function setEnabled( value )
	{
		this.mIsEnabled = value;
	}

	function isEnabled()
	{
		return this.mIsEnabled;
	}

	function getPreferredSize()
	{
		if (this.mPreferredSize != null)
		{
			return this.mPreferredSize;
		}

		if (this.mLayoutManager)
		{
			return this.mLayoutManager.preferredLayoutSize(this);
		}

		return this.getMinimumSize();
	}

	function setPreferredSize( ... )
	{
		if (vargc == 1 && vargv[0] == null)
		{
			this.mPreferredSize = vargv[0];
		}
		else if (vargc == 1 && typeof vargv[0] == "table")
		{
			this.mPreferredSize = vargv[0];
		}
		else if (vargc == 2)
		{
			this.mPreferredSize = {
				width = vargv[0],
				height = vargv[1]
			};
		}
		else
		{
			throw this.Exception("Invalid argument(s)");
		}
	}

	function getMinimumSize()
	{
		if (this.mMinimumSize != null)
		{
			return this.mMinimumSize;
		}

		if (this.mLayoutManager)
		{
			return this.mLayoutManager.minimumLayoutSize(this);
		}

		return {
			width = 0,
			height = 0
		};
	}

	function setMinimumSize( ... )
	{
		if (vargc == 1 && vargv[0] == null)
		{
			this.mMinimumSize = vargv[0];
		}
		else if (vargc == 1 && typeof vargv[0] == "table")
		{
			this.mMinimumSize = vargv[0];
		}
		else if (vargc == 2)
		{
			this.mMinimumSize = {
				width = vargv[0],
				height = vargv[1]
			};
		}
		else
		{
			throw this.Exception("Invalid argument(s)");
		}
	}

	function getFont()
	{
		if (this.mFont)
		{
			return this.mFont;
		}

		if (this.mParentComponent)
		{
			return this.mParentComponent.getFont();
		}

		return this.GUI.DefaultFont;
	}

	function _areaDebug()
	{
		this.setSkin("GUI");
		this.setAppearance("AreaDebug");
	}

	function setFont( font )
	{
		this.mFont = font;

		if (this.mWidget != null)
		{
			font = this.getFont();
			this.mWidget.setParam("font_name", font.getFullFace());
			this.mWidget.setParam("char_height", "" + font.height);
		}
	}

	function setCached( which )
	{
		this.mCached = which;

		if (this.mWidget != null)
		{
			this.mWidget.setCached(which);
		}
	}

	function _addNotify()
	{
		if (this.mIsRealized)
		{
			throw this.Exception("attempt to re-realize");
			return;
		}

		local appearance = this.mAppearance;

		if (!appearance && this.components.len() > 0)
		{
			appearance = "Container";
		}

		if (appearance)
		{
			local tmpl;

			if (this.mSkin)
			{
				tmpl = this.mSkin + "/" + appearance;
			}
			else
			{
				tmpl = appearance;
			}

			local w = this.Widget.createFromTemplate(tmpl, this.mName);

			if (!w)
			{
				throw this.Exception("skin element not found: " + tmpl);
			}

			this.mWidget = w;

			if (this.mMaterial)
			{
				try
				{
					local materialName;

					if (this.mSkin && this.mMaterialIsSkinned)
					{
						materialName = this.mSkin + "/" + this.mMaterial;
					}
					else
					{
						materialName = this.mMaterial;
					}

					this.mWidget.setMaterialName(materialName);
				}
				catch( err )
				{
					this.log.error("Problem setting " + this.mWidget + " material: " + err);
				}
			}

			this.mWidget.setVisible(this.mIsVisible);
			local font = this.getFont();
			this.mWidget.setParam("font_name", font.getFullFace());
			this.mWidget.setParam("char_height", "" + font.height);
			this.mWidget.setCached(this.mCached);

			if (this.mFontColor != null)
			{
				local fontColorString = this.mFontColor.r.tostring() + ", " + this.mFontColor.g.tostring() + ", " + this.mFontColor.b.tostring() + ", " + this.mFontColor.a.tostring();
				this.mWidget.setParam("colour_top", fontColorString);
				this.mWidget.setParam("colour_bottom", fontColorString);
			}

			if (this.mBlendColor != null)
			{
				local blendColorString = this.mBlendColor.r.tostring() + ", " + this.mBlendColor.g.tostring() + ", " + this.mBlendColor.b.tostring() + ", " + this.mBlendColor.a.tostring();
				this.mWidget.setParam("color", blendColorString);
			}

			if (this.mHAlign != null)
			{
				this.mWidget.setParam("horz_align", "" + this.mHAlign);
			}

			if (this.mVAlign != null)
			{
				this.mWidget.setParam("vert_align", "" + this.mVAlign);
			}

			if (this.mParentComponent)
			{
				this.mWidget.setParent(this.mParentComponent.mWidget);
			}

			if (this.mKeyFocus)
			{
				this.mWidget.requestKeyboardFocus();
			}

			this.mWidget.setUserData(this);
			this._reshapeNotify();
		}
		else
		{
			this.mWidget = null;
		}

		this.mIsRealized = true;

		if (this.components.len() > 0)
		{
			if (this.mWidget == null)
			{
				throw this.Exception("Container needs a Widget (skin error)");
			}

			if (!this.mWidget.isContainer())
			{
				throw this.Exception("Container Widget isn\'t a container (skin error)");
			}

			local c;

			foreach( c in this.components )
			{
				c._addNotify();

				if (!c.mIsRealized)
				{
					throw this.Exception("Child _addNotify() did not realize!");
				}
			}
		}

		if (this.mWidget != null && this.mParentComponent == null)
		{
			this.mWidget.addListener(this);
		}
	}

	function requestKeyboardFocus()
	{
		this.mKeyFocus = true;

		if (this.mWidget)
		{
			this.mWidget.requestKeyboardFocus();
		}
	}

	function _removeNotify()
	{
		if (this.mWidget != null && this.mParentComponent == null)
		{
			this.mWidget.removeListener(this);
		}

		local c;

		foreach( c in this.components )
		{
			if (c.mIsRealized)
			{
				c._removeNotify();
			}
		}

		if (this.mWidget != null)
		{
			this.GUI._Manager.releaseKeyboardFocus(this);
			this.mWidget.destroy();
			this.mWidget = null;
		}

		this.mIsRealized = false;
	}

	function _reshapeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.setPosition(this.mX, this.mY);
			this.mWidget.setSize(this.mWidth, this.mHeight);
		}

		if (this.mSelection)
		{
			this.mSelection.setPosition(0, 0);
			this.mSelection.setSize(this.mWidth, this.mHeight);
		}
	}

	function _visibilityNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.setVisible(this.mIsVisible);
		}
	}

	function setLayoutManager( layoutMgr )
	{
		this.mLayoutManager = layoutMgr;
		this.invalidate();
	}

	function getLayoutManager()
	{
		return this.mLayoutManager;
	}

	function getComponentIndex( component )
	{
		local i = 0;

		foreach( c in this.components )
		{
			if (c == component)
			{
				return i;
			}

			i++;
		}

		return -1;
	}

	function onRequestedKeyboardFocus()
	{
		if (this.mWidget != null && !this.mKeyFocus)
		{
			this.mWidget.requestKeyboardFocus();
		}

		this.mKeyFocus = true;
	}

	function onReleasedKeyboardFocus()
	{
		this.mKeyFocus = false;
	}

	function setScroll( pScroll )
	{
		this.mScroll = pScroll;
	}

	function getScroll()
	{
		return this.mScroll;
	}

	function destroy()
	{
		local c;

		foreach( c in this.components )
		{
			c.destroy();
		}

		this.mParentComponent = null;

		if (this.mMessageBroadcaster != null)
		{
			this.mMessageBroadcaster.removeAllListeners();
		}

		this.setVisible(false);

		if (this.mOverlay != null)
		{
			this.setOverlay(null);
		}

		this.mWidget = null;
	}

	function getParent()
	{
		return this.mParentComponent;
	}

	function add( component, ... )
	{
		if (vargc > 0)
		{
			this.insert(this.components.len(), component, vargv[0]);
		}
		else
		{
			this.insert(this.components.len(), component);
		}
	}

	function insert( index, component, ... )
	{
		if (!this.IsInstanceOf(component, this.GUI.Component))
		{
			throw this.Exception("Invalid component: " + component);
		}

		if (component.mParentComponent)
		{
			component.mParentComponent.remove(component);
		}
		else if (component.mOverlay)
		{
			component.setOverlay(null);
		}

		if (index < 0)
		{
			index = 0;
		}
		else if (index > this.components.len())
		{
			index = this.components.len();
		}

		this.components.insert(index, component);
		component.mParentComponent = this;

		if (this.mIsRealized)
		{
			component._addNotify();


			if (!component.mIsRealized)
			{
				throw this.Exception("Child _addNotify() did not realize!");
			}
		}

		if (vargc > 0)
		{
			if (!this.mLayoutConstraints)
			{
				this.mLayoutConstraints = {};
			}

			this.mLayoutConstraints[component.mName] <- vargv[0];
		}

		this.invalidate();
	}

	function remove( component )
	{
		local idx = this.getComponentIndex(component);

		if (idx >= 0)
		{
			this.components.remove(idx);

			if (this.mLayoutConstraints && component.mName in this.mLayoutConstraints)
			{
				delete this.mLayoutConstraints[component.mName];
			}

			component.mParentComponent = null;

			if (component.mIsRealized)
			{
				component._removeNotify();
			}

			this.invalidate();
		}
	}

	function removeAll()
	{
		while (this.components.len() > 0)
		{
			this.remove(this.components[0]);
		}
	}

	function findComponentAt( x, y )
	{
		local c;

		foreach( c in this.components )
		{
			if (c.isVisible())
			{
				if (x >= c.mX && x <= c.mX + c.mWidth && y >= c.mY && y <= c.mY + c.mHeight)
				{
					return c;
				}
			}
		}

		return null;
	}

	function validate()
	{
		if (!this.mIsValid)
		{
			if (this.mLayoutManager)
			{
				if (this.isResize())
				{
					this.setSize(this.mLayoutManager.preferredLayoutSize(this));
				}

				if (::gDebugGUILayout)
				{
					::GUI.DebugLayout("BEGIN " + this.mLayoutManager + " of " + this, 1);
				}

				this.mLayoutManager.layoutContainer(this);
			}
			else
			{
				this._reshapeNotify();
			}

			local c;

			foreach( c in this.components )
			{
				if (c.isVisible())
				{
					c.validate();
				}
			}

			this.mIsValid = true;

			if (::gDebugGUILayout && this.mLayoutManager)
			{
				local c;

				foreach( c in this.components )
				{
					::GUI.DebugLayout("laid out " + c);
				}

				::GUI.DebugLayout("END " + this.mLayoutManager + " of " + this, -1);
			}
		}
	}

	function _addToOverlay()
	{
		if (!this.mIsRealized)
		{
			this._addNotify();

			if (!this.mIsRealized)
			{
				throw this.Exception("_addNotify() did not realize!");
			}
		}

		this.validate();

		if (this.mOverlay)
		{
			this.mWidget.setOverlay(this.mOverlay);
		}
	}

	function _setHidden( value )
	{
		if (value)
		{
			if (this.mHidden == null)
			{
				local vis = this.mIsVisible;
				this.setVisible(false);
				this.mHidden = vis;
			}
		}
		else if (this.mHidden != null)
		{
			local vis = this.mHidden;
			this.mHidden = null;
			this.setVisible(vis);
		}
	}

	function setOverlay( overlayName )
	{
		this.mOverlay = overlayName;

		if (this.mOverlay)
		{
			if (this.mIsVisible)
			{
				this._addToOverlay();
			}

			if (!this.mIsSticky)
			{
				this._screenResizeRelay.addListener(this);
				this._screenDefaultPositionRelay.addListener(this);
			}
		}
		else if (!this.mOverlay)
		{
			if (this.mIsRealized)
			{
				this._removeNotify();
			}

			if (!this.mIsSticky)
			{
				this._screenResizeRelay.removeListener(this);
				this._screenDefaultPositionRelay.removeListener(this);
			}
		}
	}

	function getOverlay()
	{
		if (this.mOverlay == null && this.mParentComponent)
		{
			return this.mParentComponent.getOverlay();
		}

		return this.mOverlay;
	}

	function centerOnScreen()
	{
		if (this.mParentComponent != null)
		{
			throw this.Exception("Cannot center a component that is part of a container");
		}

		local sz = this.getSize();
		this.setPosition((this.Screen.getWidth() - sz.width) / 2, (this.Screen.getHeight() - sz.height) / 2);
	}

	function keepOnScreen()
	{
		local pos = this.getScreenPosition();
		local topParent = this.getToplevelParent();

		if (topParent == null)
		{
			topParent = this;
		}

		local dx = 0;
		local dy = 0;
		local screenW = this.Screen.getWidth();
		local screenH = this.Screen.getHeight();

		if (pos.x < 0)
		{
			dx = -pos.x;
		}
		else if (pos.x + this.mWidth > screenW)
		{
			dx = screenW - (pos.x + this.mWidth);
		}
		else
		{
			dx = 0;
		}

		if (pos.y < 0)
		{
			dy = -pos.y;
		}
		else if (pos.y + this.mHeight > screenH)
		{
			dy = screenH - (pos.y + this.mHeight);
		}
		else
		{
			dy = 0;
		}

		if (dx == 0 && dy == 0)
		{
			return;
		}

		topParent.setPosition(pos.x + dx, pos.y + dy);
	}

	function setInsets( ... )
	{
		this.insets = {};

		if (vargc == 1)
		{
			this.insets.top <- vargv[0].tointeger();
			this.insets.right <- vargv[0].tointeger();
			this.insets.bottom <- vargv[0].tointeger();
			this.insets.left <- vargv[0].tointeger();
		}
		else if (vargc == 4)
		{
			this.insets.top <- vargv[0].tointeger();
			this.insets.right <- vargv[1].tointeger();
			this.insets.bottom <- vargv[2].tointeger();
			this.insets.left <- vargv[3].tointeger();
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setInsets()");
		}
	}

	function _addInsets( size )
	{
		size.width += this.insets.left + this.insets.right;
		size.height += this.insets.top + this.insets.bottom;
		return size;
	}

	function getDebugLayoutString( ... )
	{
		local depth = vargc > 0 ? vargv[0] : 0;
		local str = "";
		local c;

		for( c = 0; c < depth; c++ )
		{
			str += "  ";
		}

		str += "" + this + "\n";

		foreach( c in this.components )
		{
			str += c.getDebugLayoutString(depth + 1);
		}

		return str;
	}

	function _debugstring()
	{
		local str = "";

		if (this.mAppearance)
		{
			str += "<" + this.mAppearance + "> ";
		}

		str += "pos=" + this.mX + "," + this.mY + " sz=" + this.mWidth + "x" + this.mHeight;

		if (this.mHidden != null)
		{
			if (this.mHidden)
			{
				str += " visible (clipped)";
			}
			else
			{
				str += " hidden (clipped)";
			}
		}
		else if (this.mIsVisible)
		{
			str += " visible";
		}
		else
		{
			str += " hidden";
		}

		if (this.mClassName != "Spacer")
		{
			if (this.mWidget != null && !this.mIsRealized)
			{
				str += " [ack, realization out of sync! (extra widget)] ";
			}
			else if (this.mWidget == null && this.mIsRealized)
			{
				str += " [ack, realization out of sync! (no widget)] ";
			}
		}

		str += (this.mIsRealized ? " realized" : "") + (this.mIsValid ? "" : " invalid");

		if (this.mLayoutManager && this.components.len() > 0)
		{
			str += " (" + this.mLayoutManager + ")";
		}

		return str;
	}

	function setData( value )
	{
		this.mData = value;
	}

	function getData()
	{
		return this.mData;
	}

	function _tostring()
	{
		return (this.mDebugName ? this.mDebugName : this.mName) + "[" + this._debugstring() + "]";
	}

	function onMousePressed( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function onMouseEnter( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function onMouseExit( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function onMouseWheel( evt )
	{
		if (this.mParentComponent == null && this.mPassThru == false)
		{
			evt.consume();
		}
	}

	function setPassThru( bool )
	{
		this.mPassThru = bool;
	}

	function generateTooltipWithPassThru( bool )
	{
		this.mGenTooltipWithPassThru = bool;
	}

	function getToolTipPassThruOverride()
	{
		return this.mGenTooltipWithPassThru;
	}

	function getPassThru()
	{
		return this.mPassThru;
	}

	function getTooltip()
	{
		return this.mTooltip;
	}

	function getCursor()
	{
		return null;
	}

	function setTooltip( tip )
	{
		this.mTooltip = tip;
	}

	function addChildtootip( tip )
	{
		this.mChildTooltips.append(tip);
	}

	function getChildTooltips()
	{
		return this.mChildTooltips;
	}

	function clearChildtooltips()
	{
		this.mChildTooltips.clear();
	}

	function setChildrenInheritTooltip( bool )
	{
		this.mChildrenInheritTooltip = bool;
	}

	function getChildrenInheritTooltip()
	{
		return this.mChildrenInheritTooltip;
	}

}

