this.require("UI/MainScreenElement");
this.require("UI/Currency");
this.require("UI/Screens");
class this.Screens.CurrencyOverview extends this.GUI.MainScreenElement
{
	mContainerWidth = 78;
	mContainerHeight = 25;
	mReagentsOverview = null;
	mCreditsOverview = null;
	mCopperOverview = null;
	mComponentWidth = 16;
	mComponentHeight = 16;
	mReagentStartingX = 5;
	mReagentStartingY = 5;
	mCreditsStartingX = 31;
	mCreditsStartingY = 5;
	mCopperStartingX = 57;
	mCopperStartingY = 5;
	constructor()
	{
		this.GUI.MainScreenElement.constructor(null);
		this.setSize(this.mContainerWidth, this.mContainerHeight);
		this.setAppearance("Panel");
		local screenWidth = ::Screen.getWidth();
		local screenHeight = ::Screen.getHeight();
		this.setPosition(screenWidth / 2 - this.getSize().width * 2, screenHeight - this.getSize().height - 20);
		this.mReagentsOverview = this.GUI.ReagentsOverview();
		this.mReagentsOverview.setSize(this.mComponentWidth, this.mComponentHeight);
		this.mReagentsOverview.setPosition(this.mReagentStartingX, this.mReagentStartingY);
		this.mCreditsOverview = this.GUI.CreditsOverview();
		this.mCreditsOverview.setSize(this.mComponentWidth, this.mComponentHeight);
		this.mCreditsOverview.setPosition(this.mCreditsStartingX, this.mCreditsStartingY);
		this.mCopperOverview = this.GUI.CopperOverview();
		this.mCopperOverview.setSize(this.mComponentWidth, this.mComponentHeight);
		this.mCopperOverview.setPosition(this.mCopperStartingX, this.mCopperStartingY);
		this.add(this.mReagentsOverview);
		this.add(this.mCreditsOverview);
		this.add(this.mCopperOverview);
		this.setCached(::Pref.get("video.UICache"));
	}

}

class this.GUI.CopperOverview extends this.GUI.Container
{
	constructor()
	{
		this.GUI.Container.constructor();
		this.setSize(16, 16);
		this.setPreferredSize(16, 16);
		this.setAppearance("Money/Copper");
	}

	function onMouseEnter( evt )
	{
		this.setTooltip(this._buildTooltip());
	}

	function onMouseExit( evt )
	{
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _buildTooltip()
	{
		local tooltipContainer = this.GUI.Container(this.GUI.BoxLayout());
		local currency = this.GUI.Currency();
		local copperAmt = 0;
		currency.setCurrentValue(copperAmt);

		if (::_avatar)
		{
			copperAmt = ::_avatar.getStat(this.Stat.COPPER);
		}

		if (copperAmt)
		{
			currency.setCurrentValue(copperAmt);
		}

		tooltipContainer.add(currency);
		return tooltipContainer;
	}

}

class this.GUI.CreditsOverview extends this.GUI.Container
{
	constructor()
	{
		this.GUI.Container.constructor();
		this.setSize(16, 16);
		this.setPreferredSize(16, 16);
		this.setAppearance("Credit");
	}

	function onMouseEnter( evt )
	{
		this.setTooltip(this._buildTooltip());
	}

	function onMouseExit( evt )
	{
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _buildTooltip()
	{
		local tooltipContainer = this.GUI.Container(this.GUI.BoxLayout());
		local credits = this.GUI.Credits();
		local creditAmt = 0;

		if (::_avatar)
		{
			creditAmt = ::_avatar.getStat(this.Stat.CREDITS);
		}

		credits.setCurrentValue(creditAmt);
		tooltipContainer.add(credits);
		return tooltipContainer;
	}

}

class this.GUI.ReagentsOverview extends this.GUI.Container
{
	constructor()
	{
		this.GUI.Container.constructor();
		this.setSize(13, 13);
		this.setPreferredSize(13, 13);
		this.setAppearance("MainReagent");
	}

	function onMouseEnter( evt )
	{
		this.setTooltip(this._buildTooltip());
	}

	function onMouseExit( evt )
	{
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _buildTooltip()
	{
		local tooltipContainer = this.GUI.Container(this.GUI.BoxLayout());
		local reagents = this.GUI.Reagents();
		tooltipContainer.add(reagents);
		return tooltipContainer;
	}

}

