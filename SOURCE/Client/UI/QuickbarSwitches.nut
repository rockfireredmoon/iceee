this.require("UI/QuickBar");
this.require("UI/Screens");
class this.Screens.QuickbarSwitches extends this.GUI.MainScreenElement
{
	mContainerWidth = 227;
	mContainerHeight = 31;
	mQuickbarNumberWidth = 23;
	mQuickbarNumberHeight = 20;
	mXSpacing = 9;
	mYSpacing = 5;
	mXInitialXSpacing = 6;
	mQuickbarSwitchContainers = null;
	mQuickbarSwitchLabels = null;
	MAX_QUICKBAR_SWITCHES = 8;
	constructor()
	{
		this.GUI.MainScreenElement.constructor(null);
		this.mQuickbarSwitchContainers = this.array(this.MAX_QUICKBAR_SWITCHES);
		this.mQuickbarSwitchLabels = this.array(this.MAX_QUICKBAR_SWITCHES);
		this.setSize(this.mContainerWidth, this.mContainerHeight);
		this.setAppearance("QuickbarSwitchBackground");
		local screenWidth = ::Screen.getWidth();
		local screenHeight = ::Screen.getHeight();
		this.setPosition(screenWidth / 2 + 33, screenHeight - this.getSize().height - 45);

		for( local i = 0; i < this.MAX_QUICKBAR_SWITCHES; i++ )
		{
			this.mQuickbarSwitchContainers[i] = this.GUI.Button(i.tostring());

			if (i != 0)
			{
				this.mQuickbarSwitchContainers[i].setAppearance("QuickbarSwitchButton");
				this.mQuickbarSwitchContainers[i].setFontColor(this.Colors.white);
				this.mQuickbarSwitchContainers[i].setSize(this.mQuickbarNumberWidth, this.mQuickbarNumberHeight);
				this.mQuickbarSwitchContainers[i].setPreferredSize(this.mQuickbarNumberWidth, this.mQuickbarNumberHeight);
				this.mQuickbarSwitchContainers[i].setPosition(this.mQuickbarNumberWidth * (i - 1) + (i - 1) * this.mXSpacing + this.mXInitialXSpacing, this.mYSpacing);
				this.mQuickbarSwitchContainers[i].setTooltip("Quick Bar " + i.tostring());
				this.add(this.mQuickbarSwitchContainers[i]);
				this.mQuickbarSwitchContainers[i].setReleaseMessage("onQuickbarSwitchPressed");
				this.mQuickbarSwitchContainers[i].addActionListener(this);
			}

			::_quickBarManager.addListener(this);
		}

		this.setCached(::Pref.get("video.UICache"));
	}

	function onQuickbarLoaded()
	{
		if (::_quickBarManager.isQuickbarVisible(this.i))
		{
			this.mQuickbarSwitchContainers[this.i].setToggled(true);
		}
	}

	function onQuickbarSwitchPressed( button )
	{
		local i = 1;

		while (i < this.MAX_QUICKBAR_SWITCHES)
		{
			if (this.mQuickbarSwitchContainers[i] == button)
			{
				break;
			}

			i++;
		}

		if (i == this.MAX_QUICKBAR_SWITCHES)
		{
			return;
		}

		button.setToggled(!button.getToggled());
		::_quickBarManager.toggleQuickbar(i);
	}

	function onQuickbarUnserialized( sender, quickbar )
	{
		local quickbarIndex = quickbar.getIndex();

		if (::_quickBarManager.isQuickBarVisible(quickbarIndex))
		{
			this.mQuickbarSwitchContainers[quickbarIndex].setToggled(true);
		}
	}

}

