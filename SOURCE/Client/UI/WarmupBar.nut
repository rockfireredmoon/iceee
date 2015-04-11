class this.GUI.WarmupBar extends this.GUI.Panel
{
	mBar = null;
	mLabel = "";
	mAction = null;
	mDuration = 0.0;
	constructor( action )
	{
		this.GUI.Panel.constructor(this.GUI.BoxLayout());
		this._enterFrameRelay.addListener(this);
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
		this.mAction = action;
		this.mDuration = this.mAction.getWarmupDuration();
		this.add(this.mBar);
		this._exitGameStateRelay.addListener(this);
	}

	function onEnterFrame()
	{
		local timeLeft = this.mAction.getWarmupTimeLeft();

		if (timeLeft >= 0.0)
		{
			local startTime = this.mAction.getWarmupStartTime();
			local endTime = this.mAction.getWarmupEndTime();
			local curTime = endTime - timeLeft;
			local progress = (endTime - curTime).tofloat() / (endTime - startTime).tofloat();
			this.mBar.setCurrent(100.0 * progress);
			this.mBar.setAnimated(true);
		}
		else
		{
			this.destroy();
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

	function onExitGame()
	{
		this.destroy();
	}

}

class this.GUI.ChannelBar extends this.GUI.Panel
{
	mBar = null;
	mLabel = "";
	mAction = null;
	mDuration = 0.0;
	constructor( action )
	{
		this.GUI.Panel.constructor(this.GUI.BoxLayout());
		this._enterFrameRelay.addListener(this);
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
		this.mAction = action;
		this.mDuration = this.mAction.getDuration();
		this.add(this.mBar);
		this._exitGameStateRelay.addListener(this);
	}

	function onExitGame()
	{
		this.destroy();
	}

	function onEnterFrame()
	{
		local timeLeft = this.mAction.getChannelTimeLeft();

		if (timeLeft >= 0.0)
		{
			local startTime = this.mAction.getChannelStartTime();
			local endTime = this.mAction.getChannelEndTime();
			local curTime = endTime - timeLeft;
			local progress = (endTime - curTime).tofloat() / (endTime - startTime).tofloat();
			this.mBar.setCurrent(100.0 * progress);
			this.mBar.setAnimated(true);
		}
		else
		{
			this.destroy();
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

