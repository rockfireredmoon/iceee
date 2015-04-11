this.require("UI/Screens");
class this.Screens.CreatureActionCountdown extends this.GUI.Panel
{
	mBar = null;
	mLabel = "";
	mActionName = "";
	mDuration = 0.0;
	mStartTime = 0.0;
	mEndTime = 0.0;
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
		this.mTimer = ::Timer();
		this.add(this.mBar);
	}

	function interruptAction()
	{
		this.mBar.setAnimated(false);
		this.Screens.hide("CreatureActionCountdown");
		this._enterFrameRelay.removeListener(this);
	}

	function setAction( duration, actionName )
	{
		local width = this.mBar.getWidth() / 2;
		local height = this.mBar.getHeight() / 2;
		this.setPosition(::Screen.getWidth() / 2 - width, ::Screen.getHeight() * 0.85000002 - height);
		this.mDuration = duration;
		this.mActionName = actionName;
		this.mStartTime = this.mTimer.getMilliseconds();
		this.mEndTime = this.mStartTime + duration;
		this.setText(this.mActionName);
		this._enterFrameRelay.addListener(this);
	}

	function onEnterFrame()
	{
		local curTime = this.mTimer.getMilliseconds();

		if (curTime < this.mEndTime)
		{
			local progress = (this.mEndTime - curTime).tofloat() / (this.mEndTime - this.mStartTime).tofloat();
			this.mBar.setCurrent(100.0 * progress);
			this.mBar.setAnimated(true);
		}
		else
		{
			this.mBar.setAnimated(false);
			this.Screens.hide("CreatureActionCountdown");
			this._enterFrameRelay.removeListener(this);
		}
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

