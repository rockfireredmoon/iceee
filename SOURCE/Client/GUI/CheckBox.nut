this.require("GUI/Button");
class this.GUI.CheckBox extends this.GUI.Button
{
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor("", vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor("");
		}

		this.mAppearance = "CheckBox";
		this.mUseMouseOverEffect = false;
		this.mUseOffsetEffect = false;
		this.mReleaseMessage = "onActionPerformed";
		this.setSize(30, 31);
		this.setFixedSize(30, 31);
		this.mCheckMark = ::GUI.Component();
		this.mCheckMark.setLayoutExclude(true);
		this.mCheckMark.setAppearance(this.mAppearance + "/CheckMark");
		this.mCheckMark.setVisible(false);
		this.mCheckMark.setSize(30, 31);
		this.add(this.mCheckMark);
	}

	function setAppearance( appearance )
	{
		this.GUI.Button.setAppearance(appearance);
		this.mCheckMark.setAppearance(this.mAppearance + "/CheckMark");
	}

	function setCheckMarkAppearance( appearance )
	{
		this.mCheckMark.setAppearance(appearance);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this, this.mChecked);
		}
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
			w = vargv[0].tointeger();
			h = vargv[1].tointeger();
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setSize()");
		}

		::GUI.Button.setSize(w, h);

		if (this.mCheckMark)
		{
			this.mCheckMark.setSize(w, h);
		}
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();

		if (this.mCheckMark)
		{
			this.mCheckMark.setPosition(0, 0);
			this.mCheckMark.setSize(this.mWidth, this.mHeight);
		}
	}

	function setChecked( pBool )
	{
		if (this.mChecked != pBool)
		{
			this.mChecked = pBool;
			this.mCheckMark.setVisible(this.mChecked);
		}
	}

	function getChecked()
	{
		return this.mChecked;
	}

	function onMouseReleased( evt )
	{
		if (this.mHover && this.mPressed)
		{
			if (this.isEnabled())
			{
				this.setChecked(!this.mChecked);
			}
		}

		this.GUI.Button.onMouseReleased(evt);
	}

	mCheckMark = null;
	mChecked = false;
	static mClassName = "CheckBox";
}

