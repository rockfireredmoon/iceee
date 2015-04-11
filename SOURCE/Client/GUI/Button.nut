this.require("GUI/Container");
class this.GUI.Button extends this.GUI.Container
{
	mInitAppearance = null;
	mInitFontColor = null;
	mDisabledFontColor = null;
	mIcon = null;
	mLabel = null;
	mPressed = false;
	mHover = false;
	mUseMouseOverEffect = true;
	mUseOffsetEffect = true;
	mOffset = false;
	mPressMessage = "";
	mReleaseMessage = "";
	mOpenMenuMessage = "";
	mFixedSize = null;
	mRadioGroup = null;
	mToggled = false;
	mAutoRepeat = false;
	mAutoRepeatTimer = null;
	mAutoRepeatDelay = 100;
	mMouseExitMessage = "onExitButton";
	static mClassName = "Button";
	constructor( label, ... )
	{
		this.GUI.Container.constructor();
		this.mAppearance = "Button";
		this.mInitAppearance = "Button";
		this.setInsets(3, 5, 3, 5);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.mIcon = null;
		this.mLabel = this.GUI.Label(label);
		this.mLabel.setTextAlignment(0.5, 0.5);
		this.add(this.mLabel, this.GUI.BorderLayout.CENTER);
		this.setDisabledFontColor("CCCCCC");
		this.setSize(this.getPreferredSize());
		this.setSelection(true);
		this.mInitFontColor = this.Color(this.GUI.DefaultButtonFontColor);
		this.mReleaseMessage = "onActionPerformed";
		this.mOpenMenuMessage = "onOpenMenu";
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
			this.mReleaseMessage = vargv[1];
		}

		if (vargc > 2 && typeof vargv[2] == "string")
		{
			this.mOpenMenuMessage = vargv[2];
		}
	}

	function getLabel()
	{
		return this.mLabel;
	}

	function setAppearance( appearance )
	{
		this.mInitAppearance = appearance;

		if (this.mIsEnabled)
		{
			this.GUI.Container.setAppearance(appearance);
		}
		else if (!this.mIsEnabled)
		{
			if (appearance)
			{
				this.GUI.Container.setAppearance(appearance + "/Disabled");
			}
		}

		if (this.mIsRealized)
		{
			this._removeNotify();
			this._addNotify();
		}
	}

	function setOpts( opts )
	{
		this.GUI.Container.setOpts(opts);

		if ("releaseMessage" in opts)
		{
			this.setReleaseMessage(opts.releaseMessage);
		}

		if ("pressMessage" in opts)
		{
			this.setReleaseMessage(opts.pressMessage);
		}

		this.mLabel.setOpts(opts);
	}

	function setText( text )
	{
		this.mLabel.setText(text);
	}

	function getText()
	{
		return this.mLabel.getText();
	}

	function setValue( text )
	{
	}

	function setFixedSize( width, height )
	{
		this.mFixedSize = {
			width = width,
			height = height
		};
		this.setSize(width, height);
	}

	function getPreferredSize()
	{
		if (this.mFixedSize)
		{
			return this.mFixedSize;
		}

		return this._addInsets(this.mLabel.getPreferredSize());
	}

	function getMinimumSize()
	{
		if (this.mFixedSize)
		{
			return this.mFixedSize;
		}

		return this.GUI.Container.getMinimumSize();
	}

	function setIcon( icon )
	{
		if (this.mIcon)
		{
			this.remove(this.mIcon);
			this.mIcon = null;
		}

		if (icon)
		{
			if (typeof icon == "instance" && (icon instanceof this.GUI.Panel))
			{
				this.mIcon = icon;
			}
			else if (typeof icon == "string")
			{
				this.mIcon = this.GUI.Image();
				this.mIcon.setImageName(icon);
			}
			else
			{
				this.print("Invalid icon in Button.setIcon(): " + icon);
			}

			if (this.mIcon)
			{
				this.insert(0, this.mIcon);
			}
		}

		this.invalidate();
	}

	function getIcon()
	{
		return this.mIcon;
	}

	function setFontColor( pColor )
	{
		if (pColor != null)
		{
			if (typeof pColor == "string")
			{
				this.mInitFontColor = ::Color(pColor);
			}
			else if (typeof pColor == "instance" && (pColor instanceof this.Color))
			{
				this.mInitFontColor = ::Color(pColor.toHexString());
			}
			else
			{
				throw this.Exception("Invalid color value: " + pColor);
			}
		}
		else
		{
			this.mInitFontColor = null;
		}

		if (this.mIsEnabled)
		{
			this.setSwitchFontColor(this.mInitFontColor);
		}

		this.invalidate();
	}

	function getFontColor()
	{
		if (this.mInitFontColor)
		{
			return this.mInitFontColor;
		}

		if (this.mParentComponent)
		{
			return this.mParentComponent.getFontColor();
		}

		return ::Color(this.GUI.DefaultFontColor);
	}

	function setSwitchFontColor( pColor )
	{
		if (pColor)
		{
			this.mLabel.mFontColor = pColor;
		}
		else if (this.mParentComponent)
		{
			this.mLabel.mFontColor = this.mParentComponent.getFontColor();
		}
		else
		{
			this.mLabel.mFontColor = ::Color(this.GUI.DefaultFontColor);
		}

		if (this.mWidget != null)
		{
			local fontColorString = this.mLabel.mFontColor.r.tostring() + ", " + this.mLabel.mFontColor.g.tostring() + ", " + this.mLabel.mFontColor.b.tostring();
			this.mLabel.mWidget.setParam("colour_top", fontColorString);
			this.mLabel.mWidget.setParam("colour_bottom", fontColorString);
		}

		this._recreate();
	}

	function setDisabledFontColor( pColor )
	{
		if (pColor != null)
		{
			if (typeof pColor == "string")
			{
				this.mDisabledFontColor = ::Color(pColor);
			}
			else if (typeof pColor == "instance" && (pColor instanceof this.Color))
			{
				this.mDisabledFontColor = ::Color(pColor.toHexString());
			}
			else
			{
				throw this.Exception("Invalid color value: " + pColor);
			}
		}
		else
		{
			this.mDisabledFontColor = null;
		}

		if (!this.mIsEnabled)
		{
			this.setSwitchFontColor(this.mDisabledFontColor);
		}

		this.invalidate();
	}

	function _setOffset( pBool )
	{
		if (this.mOffset && !pBool)
		{
			this.mOffset = pBool;

			if (this.mIsRealized)
			{
				this._reshapeNotify();
			}
		}
		else if (!this.mOffset && pBool)
		{
			this.mOffset = pBool;

			if (this.mIsRealized)
			{
				this._reshapeNotify();
			}
		}
	}

	function setReleaseMessage( pString )
	{
		this.mReleaseMessage = pString;
	}

	function setMouseExitMessage( messageString )
	{
		this.mMouseExitMessage = messageString;
	}

	function setOpenMenuMessage( pString )
	{
		this.mOpenMenuMessage = pString;
	}

	function getReleaseMessage()
	{
		return this.mReleaseMessage;
	}

	function getMouseExitMessage()
	{
		return this.mMouseExitMessage;
	}

	function getOpenMenuMessage()
	{
		return this.mOpenMenuMessage;
	}

	function setPressMessage( pString )
	{
		this.mPressMessage = pString;
	}

	function getPressMessage()
	{
		return this.mPressMessage;
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

		if (this.mRadioGroup && pMessage == this.mReleaseMessage || this.mRadioGroup && pMessage == this.mPressMessage && this.mReleaseMessage == "onActionPerformed")
		{
			this.mRadioGroup.setSelected(this);
		}
	}

	function onMousePressed( evt )
	{
		if (!this.mIsEnabled)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mPressed = true;

			if (this.mUseOffsetEffect)
			{
				this._setOffset(true);
			}

			if (this.mPressMessage)
			{
				this._fireActionPerformed(this.mPressMessage);
			}

			if (this.mAutoRepeat)
			{
				this.mAutoRepeatTimer = ::Timer();
				this.mAutoRepeatTimer.reset();
			}

			this.GUI._Manager.releaseKeyboardFocus();
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (!this.mIsEnabled)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mAutoRepeatTimer = null;

			if (this.mHover && this.mPressed)
			{
				if (this.mUseOffsetEffect)
				{
					this._setOffset(false);
				}

				::Audio.playSound("Sound-UiClick.ogg");
				this._fireActionPerformed(this.mReleaseMessage);
				evt.consume();
			}

			this.mPressed = false;
		}
		else if (evt.button == this.MouseEvent.RBUTTON)
		{
			this._fireActionPerformed(this.mOpenMenuMessage);
		}
	}

	function onMouseEnter( evt )
	{
		if (!this.mIsEnabled)
		{
			return;
		}

		this.mHover = true;

		if (this.mUseMouseOverEffect && this.mWidget != null && (this.mSelection || this.mToggled))
		{
			this.setSelectionVisible(true);
		}
	}

	function onMouseExit( evt )
	{
		this.mHover = false;
		this.mPressed = false;

		if (this.mUseOffsetEffect)
		{
			this._setOffset(false);
		}

		if (this.mUseMouseOverEffect && this.mWidget != null && this.mSelection && !this.mToggled)
		{
			this.setSelectionVisible(false);
		}

		this._fireActionPerformed(this.mMouseExitMessage);
	}

	function onEnterFrame()
	{
		if (this.mAutoRepeatTimer && this.mAutoRepeatTimer.getMilliseconds() > this.mAutoRepeatDelay)
		{
			this._fireActionPerformed(this.mPressMessage);
			this.mAutoRepeatTimer.reset();
		}
	}

	function getToggled()
	{
		return this.mToggled;
	}

	function setToggled( value )
	{
		this.mToggled = value;
		this.setSelectionVisible(this.mWidget != null && (this.mToggled || this.mHover));
	}

	function setToggledExplicit( value )
	{
		this.mToggled = value;
		this.setSelectionVisible(value);
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.addListener(this);
		this.mWidget.setChildProcessingEvents(false);

		if (this.mOffset)
		{
			this.mWidget.setPosition(this.mX + 1, this.mY + 1);
		}
		else
		{
			this.mWidget.setPosition(this.mX, this.mY);
		}

		if (this.mAutoRepeat)
		{
			::_root.addListener(this);
		}
	}

	function _removeNotify()
	{
		if (this.mAutoRepeat == true)
		{
			::_root.removeListener(this);
		}

		this.GUI.Component._removeNotify();

		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.onMouseExit(null);
	}

	function _reshapeNotify()
	{
		this.GUI.Container._reshapeNotify();

		if (this.mWidget != null && this.mOffset)
		{
			this.mWidget.setPosition(this.mX + 1, this.mY + 1);
		}
	}

	function _debugstring()
	{
		return this.GUI.Container._debugstring() + ", \"" + this.getText() + "\"";
	}

	function setRadioGroup( radio )
	{
		if (this.mRadioGroup == radio)
		{
			return;
		}

		if (this.mRadioGroup != null)
		{
			this.mRadioGroup._removeButton(this);
		}

		this.mRadioGroup = radio;

		if (this.mRadioGroup != null)
		{
			this.mRadioGroup._addButton(this);
		}
	}

	function getRadioGroup()
	{
		return this.mRadioGroup;
	}

	function setEnabled( bool )
	{
		if (bool == this.mIsEnabled)
		{
			return;
		}

		if (bool)
		{
			this.mAppearance = this.mInitAppearance;
			this.setSwitchFontColor(this.mInitFontColor);
		}
		else
		{
			this.onMouseExit(null);

			if (this.mInitAppearance)
			{
				this.mAppearance = this.mInitAppearance + "/Disabled";
			}

			this.setSwitchFontColor(this.mDisabledFontColor);
		}

		this.GUI.Container.setEnabled(bool);
		this._recreate();
	}

	function setUseOffsetEffect( bool )
	{
		this.mUseOffsetEffect = bool;
	}

	function setAutoRepeat( bool )
	{
		if (this.mWidget != null && this.mAutoRepeat == true && bool == false)
		{
			::_root.removeListener(this);
		}

		if (this.mWidget != null && this.mAutoRepeat == false && bool == true)
		{
			::_root.addListener(this);
		}

		this.mAutoRepeat = bool;
	}

	function setAutoRepeatDelay( delay )
	{
		this.mAutoRepeatDelay = delay;
	}

	function setUseMouseOverEffect( value )
	{
		this.mUseMouseOverEffect = value;
	}

}

