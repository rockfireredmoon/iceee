this.require("UI/Screens");
class this.Screens.InvisibilityScreen extends this.GUI.Panel
{
	mBar = null;
	mLabel = "";
	mDistance = 0.0;
	mDuration = 0;
	mTimer = null;
	constructor()
	{
		this.GUI.Panel.constructor(this.GUI.BoxLayout());
		this.setResize(true);
		this.setPassThru(true);
		local appearance = "LoadScreen";
		this.setBlendColor(this.Color(1, 1, 1, 1.0));
		this.mBar = this.GUI.ProgressBar(appearance + "/MainProgressBar");
		this.mBar.setSize(200, 10);
		this.mBar.setFontColor("ffff99");
		this.mBar.setVisible(true);
		this.mBar.setMaskVisible(false);
		this.mBar.setLabelVisible(true);
		this.mBar.setLabelMode(this.GUI.ProgressBar.FRACTION);
		this.mBar.setLabelVisible(false);
		this.mBar.setMax(100);
		this.mBar.setCurrent(100);
		this.mBar.setAnimated(false);
		this.mLabel = this.GUI.Label();
		this.mLabel.setText("");
		this.mBar.add(this.mLabel);
		this.mBar.setCurrent(100.0);
		this.mBar.setAnimated(true);
		this.add(this.mBar);
		this.setText((this.mDuration / 1000).tostring());
		this.setOverlay(this.GUI.POPUP_OVERLAY);
		this.updatePosition();
	}

	function updatePosition()
	{
		local width = this.getWidth() / 2.0;
		local height = this.getHeight() / 2.0;
		this.setPosition(::Screen.getWidth() / 2 - width, ::Screen.getHeight() * 0.85000002 - height);
	}

	function onEnterFrame()
	{
		if (::_avatar == null)
		{
			return;
		}

		this.mDuration = this.Math.max(0, this.mDuration - ::_deltat);
		local distance = ::_avatar.getStat(this.Stat.INVISIBILITY_DISTANCE);

		if (distance)
		{
			if (distance >= 0.0)
			{
				local progress = distance / this.mDistance;
				this.mBar.setCurrent(100.0 * progress);
				this.mBar.setAnimated(true);
				local time = this.Math.max(0, (this.mDuration / 1000).tointeger());
				this.setText(time.tostring());
			}
			else
			{
				this.mBar.setAnimated(false);
				this.setVisible(false);
			}
		}
	}

	function destroy()
	{
		::GUI.Panel.destroy();
	}

	function setVisible( value )
	{
		if (value)
		{
			::_enterFrameRelay.addListener(this);
			::_screenResizeRelay.addListener(this);
			this.updatePosition();
		}
		else
		{
			::_enterFrameRelay.removeListener(this);
			::_screenResizeRelay.removeListener(this);
		}

		::GUI.Panel.setVisible(value);
	}

	function onScreenResize()
	{
		this.updatePosition();
	}

	function reset()
	{
		if (::_avatar.hasStatusEffect(this.StatusEffects.WALK_IN_SHADOWS))
		{
			local dex = ::_avatar.getStat(this.Stat.DEXTERITY);

			if (dex < 50)
			{
				dex = 50;
			}

			this.mDistance = dex * 10;
		}
		else if (::_avatar.hasStatusEffect(this.StatusEffects.INVISIBLE))
		{
			local pysche = ::_avatar.getStat(this.Stat.PSYCHE);

			if (pysche < 50)
			{
				pysche = 50;
			}

			this.mDistance = pysche * 10;
		}

		this.mDuration = 120 * 1000;
		this.setText((this.mDuration / 1000).tostring());
		this.mBar.setAnimated(false);
		this.mBar.setSize(200, 10);
		this.mBar.setCurrent(100);
		this.mBar.setAnimated(true);
	}

	function setText( text )
	{
		this.mLabel.setText(text);
		local barSize = this.mBar.getPreferredSize();
		local labelSize = this.mLabel.getPreferredSize();
		local Width = barSize.width / 2.0 - labelSize.width / 2.0;
		local Height = barSize.height / 2.0 - labelSize.height / 2.0;
		this.mLabel.setPosition(Width, Height);
	}

}

