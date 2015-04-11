this.require("GUI/Component");
this.require("GUI/Manager");
class this.GUI.Cursor extends this.GUI.Component
{
	kIconSize = 44;
	kIconOffset = 0;
	constructor()
	{
		this.GUI.Component.constructor(null);
		this.setSize(35, 35);
		this.setAppearance("Cursor");
		this._exitFrameRelay.addListener(this);
		::_cursor <- this;
		this.setOverlay("GUI/CursorOverlay");
		::Screen.setOverlayVisible("GUI/CursorOverlay", false);
		::Screen.setOverlayVisible("GUI/DragOverlay", true);
		::Screen.setOverlayPassThru("GUI/CursorOverlay", true);
		::Screen.setOverlayPassThru("GUI/DragOverlay", true);
		this.mSecondaryImage = this.GUI.Component(null);
		this.mSecondaryImage.setPosition(0, 0);
		this.mSecondaryImage.setSize(35, 35);
		this.mSecondaryImage.setAppearance("Cursor/Secondary");
		this.mSecondaryImage.setVisible(false);
		this.add(this.mSecondaryImage);
	}

	function isLocked()
	{
		return this.mLocked;
	}

	function setLocked( value, ... )
	{
		if (value == this.mLocked)
		{
			return;
		}

		if (vargc > 1 && value)
		{
			this.mLockedX = vargv[0];
			this.mLockedY = vargv[1];
		}
		else if (vargc > 0 && value)
		{
			this.mLockedX = vargv[0].x;
			this.mLockedY = vargv[0].y;
		}
		else
		{
			local pos = ::Screen.getCursorPos();
			this.mLockedX = pos.x;
			this.mLockedY = pos.y;
		}

		if (value)
		{
			::_root.lockMouse(this.mLockedX, this.mLockedY);
		}
		else
		{
			::_root.unlockMouse();
		}

		::Screen.setCursorVisible(!value);
		this.mLocked = value;
	}

	function getState()
	{
		return this.mState;
	}

	function showNativeCursor()
	{
		::Screen.setOverlayVisible("GUI/CursorOverlay", false);

		if (!this.mLocked)
		{
			::Screen.setCursorVisible(true);
		}
	}

	function hideNativeCursor()
	{
		::Screen.setOverlayVisible("GUI/CursorOverlay", true);
		::Screen.setCursorVisible(false);
	}

	function setState( state )
	{
		if (state == this.mState)
		{
			return;
		}

		this.hideNativeCursor();

		switch(state)
		{
		case this.DRAG:
			this.setMaterial("SmallCursor");
			this.mSecondaryImage.setMaterial("Cursor/Drag");
			this.mSecondaryImage.setVisible(true);
			this.mState = state;
			break;

		case this.LOAD:
			this.setMaterial("SmallCursor");
			this.mSecondaryImage.setMaterial("Cursor/Load");
			this.mSecondaryImage.setVisible(true);
			this.mState = state;
			break;

		case this.USE:
			this.setMaterial("SmallCursor");
			this.mSecondaryImage.setMaterial("Cursor/Use");
			this.mSecondaryImage.setVisible(true);
			this.mState = state;
			break;

		case this.ATTACK:
			this.setMaterial("SmallCursor");
			this.mSecondaryImage.setMaterial("Cursor/Attack");
			this.mSecondaryImage.setVisible(true);
			this.mState = state;
			break;

		case this.SELECTGROUND:
			this.setMaterial("SmallCursor");
			this.mSecondaryImage.setMaterial("Cursor/Use");
			this.mSecondaryImage.setVisible(true);
			this.mState = state;
			break;

		case this.ROTATE:
			this.showNativeCursor();
			this.mState = state;
			break;

		default:
			this.showNativeCursor();
			this.mState = this.DEFAULT;
			break;
		}
	}

	function onExitFrame()
	{
		if (this.mLocked)
		{
			if (this.mIcon)
			{
				local sz = this.mIcon.getSize();
				this.mIcon.setPosition(this.mLockedX + this.kIconOffset, this.mLockedY + this.kIconOffset);
			}

			this.setPosition(this.mLockedX, this.mLockedY);
		}
		else
		{
			local pos = ::Screen.getCursorPos();
			this.setPosition(pos.x + -1, pos.y + -1);

			if (this.mIcon)
			{
				local sz = this.mIcon.getSize();
				this.mIcon.setPosition(pos.x + this.kIconOffset, pos.y + this.kIconOffset);
			}
		}
	}

	function attachAction( foreground, background )
	{
		this.kIconOffset = 0;

		if (foreground == null && this.mIcon)
		{
			this.mIcon.setOverlay(null);
			this.mIcon = null;
			this.mImage = null;
		}
		else
		{
			if (!this.mIcon)
			{
				this.mIcon = this.GUI.Image(null);
				this.mIcon.setLayoutManager(null);
				this.mIcon.setAppearance("Icon");
			}

			if (background)
			{
				this.mIcon.setImageName(background.getImageName());
			}

			if (!this.mImage)
			{
				this.mImage = this.GUI.Image(null);
				this.mImage.setLayoutManager(null);
				this.mImage.setAppearance("Icon");
			}

			if (foreground)
			{
				this.mImage.setImageName(foreground.getImageName());
			}

			this.mIcon.add(this.mImage);
			this.mIcon.setSize(this.kIconSize, this.kIconSize);
			this.mImage.setSize(this.kIconSize, this.kIconSize);
			this.mImage.setPreferredSize(this.kIconSize, this.kIconSize);
			this.mIcon.setOverlay("GUI/DragOverlay");
		}
	}

	function setIcon( material, ... )
	{
		if (material == null)
		{
			this.clearIcon();
			return;
		}

		local width = vargc > 1 ? vargv[0] : 32;
		local height = vargc > 2 ? vargv[1] : 32;

		if (!this.mIcon)
		{
			this.kIconOffset = 0;
			this.mIcon = this.GUI.Image(null);
			this.mIcon.setLayoutManager(null);
			this.mIcon.setAppearance("Icon");
			this.mIcon.setMaterial(material);
			this.mIcon.setSize(width, height);
			this.mIcon.setPassThru(true);
			this.mIcon.setOverlay("GUI/DragOverlay");
			this.Screen.setCursorVisible(false);
		}
	}

	function clearIcon()
	{
		if (this.mIcon)
		{
			this.mIcon.setOverlay(null);
			this.mIcon = null;
			this.Screen.setCursorVisible(true);
		}
	}

	mSecondaryImage = null;
	mState = 0;
	mLocked = false;
	mLockedX = 0;
	mLockedY = 0;
	mID = null;
	mIcon = null;
	mImage = null;
	static DEFAULT = 0;
	static DRAG = 1;
	static LOAD = 2;
	static USE = 3;
	static ATTACK = 4;
	static SELECTGROUND = 5;
	static ROTATE = 6;
}

::_cursor <- null;
::_cursor = ::GUI.Cursor();
::_cursor.setState(this.GUI.Cursor.DEFAULT);
