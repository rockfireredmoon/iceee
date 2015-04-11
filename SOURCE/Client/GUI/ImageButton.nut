this.require("GUI/Component");
this.require("GUI/Image");
class this.GUI.ImageButton extends this.GUI.Image
{
	constructor()
	{
		this.GUI.Container.constructor(null);
		this.setAppearance("Icon");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setPreferredSize(32, 32);
		this.mGlowImage = ::GUI.Component();
		this.mGlowImage.setAppearance("Icon");
		this.mGlowImage.setBlendColor(this.Color(0.89999998, 0.89999998, 0.89999998, 0.0));
		this.add(this.mGlowImage);
		this.setGlow(0.0);
	}

	function destroy()
	{
		::_enterFrameRelay.removeListener(this);
		::GUI.Container.destroy();
	}

	function _addNotify()
	{
		::GUI.Container._addNotify();
		this.mWidget.addListener(this);
		this.mWidget.setChildProcessingEvents(false);
		::_enterFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.mHover = false;
		::GUI.Container._removeNotify();
	}

	function _reshapeNotify()
	{
		::GUI.Container._reshapeNotify();
		this.mGlowImage.setSize(this.getSize());
	}

	function addActionListener( obj )
	{
		this.mMessageBroadcaster.addListener(obj);
	}

	function removeActionListener( obj )
	{
		this.mMessageBroadcaster.removeListener(obj);
	}

	function onMouseEnter( evt )
	{
		this.mHover = true;
		::_enterFrameRelay.addListener(this);
	}

	function onMouseExit( evt )
	{
		this.mHover = false;
		this.mPressed = false;
		::_enterFrameRelay.addListener(this);
	}

	function onMousePressed( evt )
	{
		if (this.mIsEnabled && evt.button == this.MouseEvent.LBUTTON)
		{
			this.mPressed = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mIsEnabled && this.mPressed)
		{
			this.onPressed();
			this.mPressed = false;
			evt.consume();
		}
	}

	function onPressed()
	{
		this.mMessageBroadcaster.broadcastMessage(this.mPressMessage, this);

		if (this.mRadioGroup && this.mPressMessage)
		{
			this.mRadioGroup.setSelected(this);
		}

		if (this.mToggled && this.mGlowEnabled)
		{
			this.mGlowImage.setBlendColor(this.Color(1, 1, 1, 1));
		}
	}

	function onEnterFrame()
	{
		local delta = this._deltat / 1000.0;
		local fadeSpeed = 3.5;
		local color = this.mGlowImage.getBlendColor();

		if (this.mHover == false || this.mIsEnabled == false)
		{
			if (this.mPulsate)
			{
				color.a += delta * this.mPulseSpeed * this.mPulseDir;

				if (color.a > 1.0)
				{
					color.a = 1.0;
					this.mPulseDir = -1;
				}
				else if (color.a < 0.0)
				{
					color.a = 0.0;
					this.mPulseDir = 1;
				}
			}
			else
			{
				color.a = this.Math.max(0.0, color.a - delta * fadeSpeed);
			}
		}
		else
		{
			color.a = this.Math.min(0.89999998, color.a + delta * fadeSpeed);
		}

		if (this.mGlowEnabled && this.mGlowConstant == false && !(this.mToggled && this.mGlowEnabled))
		{
			this.mGlowImage.setBlendColor(this.Color(1, 1, 1, color.a));
			local blendColorString = color.a.tostring();
		}

		if (this.lastAlpha == color.a)
		{
			::_enterFrameRelay.removeListener(this);
		}

		if (this.mToggled && this.mGlowEnabled)
		{
			this.mGlowImage.setBlendColor(this.Color(1, 1, 1, 1));
		}

		this.lastAlpha = color.a;
	}

	function setGlowEnabled( value )
	{
		this.mGlowEnabled = value;
	}

	function setSelection( value )
	{
		if (value)
		{
			if (this.mRadioGroup)
			{
				this.mRadioGroup.setSelected(this);
			}
		}
		else
		{
		}

		if (this.mToggled && this.mGlowEnabled)
		{
			this.mGlowImage.setBlendColor(this.Color(1, 1, 1, 1));
		}
	}

	function setGlowConstant( which )
	{
		this.mGlowConstant = which;
	}

	function getGlowConstant()
	{
		return this.mGlowConstant;
	}

	function setGlow( value )
	{
		if (this.mGlowImage)
		{
			this.mGlowImage.setBlendColor(this.Color(1.0, 1.0, 1.0, value));
		}
	}

	function getGlow()
	{
		if (this.mGlowImage)
		{
			return this.mGlowImage.getBlendColor().a;
		}
		else
		{
			return 0;
		}
	}

	function setPressMessage( name )
	{
		this.mPressMessage = name;
	}

	function getPressMessage()
	{
		return this.mPressMessage;
	}

	function setGlowImageName( glowImageName, ... )
	{
		this._updateMaterial(glowImageName, this.mGlowImage, "/Icon/Selection");
	}

	function setPulsate( which )
	{
		this.mPulsate = which;
	}

	function getPulsate()
	{
		return this.mPulsate;
	}

	function setPulseSpeed( value )
	{
		this.mPulseSpeed = value;
	}

	function getPulseSpeed()
	{
		return this.mPulseSpeed;
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

	function getToggled()
	{
		return this.mToggled;
	}

	function setToggled( value )
	{
		this.mToggled = value;
		this.setSelectionVisible(this.mWidget != null && (this.mToggled || this.mHover));
		::_enterFrameRelay.addListener(this);
	}

	lastAlpha = null;
	mPulseDir = 1;
	mPulseSpeed = 0.5;
	mGlowConstant = false;
	mGlowEnabled = true;
	mPressMessage = "onPressed";
	mMessageBroadcaster = null;
	mImageName = null;
	mGlowImage = null;
	mPressed = true;
	mPulsate = false;
	mHover = false;
	mRadioGroup = null;
	mToggled = false;
}

