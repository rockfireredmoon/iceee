this.require("GUI/Button");
class this.GUI.Link extends this.GUI.Label
{
	mClickedMessage = "onLinkClicked";
	mColor = "44AAFF";
	mHoverColor = "AADDFF";
	mAllowHoverColorChange = true;
	static mClassName = "Link";
	constructor( label, ... )
	{
		this.GUI.Label.constructor(label);
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			if (typeof vargv[0] == "table" || typeof vargv[0] == "instance")
			{
				this.addActionListener(vargv[0]);
			}
		}

		if (vargc > 1 && typeof vargv[1] == "string")
		{
			this.mClickedMessage = vargv[1];
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function setChangeColorOnHover( value )
	{
		this.mAllowHoverColorChange = value;
	}

	function onMouseReleased( evt )
	{
		this._fireActionPerformed(this.mClickedMessage);
		evt.consume();
	}

	function _addNotify()
	{
		this.GUI.Label._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.GUI.Label._removeNotify();

		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}
	}

	function onMouseEnter( evt )
	{
		this._fireActionPerformed("onLinkMouseEnter");
		this.showHover(true);
	}

	function onMouseExit( evt )
	{
		this._fireActionPerformed("onLinkMouseExit");
		this.showHover(false);
	}

	function showHover( val )
	{
		if (this.mAllowHoverColorChange)
		{
			if (val)
			{
				this.setFontColor(this.mHoverColor);
			}
			else
			{
				this.setFontColor(this.mColor);
			}
		}
	}

	function setHoverColor( color )
	{
		this.mHoverColor = color;
	}

	function setStaticColor( color )
	{
		this.mColor = color;
	}

}

