this.require("UI/UI");
this.require("UI/CurrencyOverview");
this.require("UI/Screens");
class this.Screens.MainScreen extends this.GUI.Container
{
	static nClassName = "MainScreen";
	MAIN_SCREEN_WIDTH = 768;
	MAIN_SCREEN_HEIGHT = 73;
	mExperienceBar = null;
	mLuckHeroism = null;
	constructor( ... )
	{
		this.GUI.Container.constructor(null);
		this.setSize(this.MAIN_SCREEN_WIDTH, this.MAIN_SCREEN_HEIGHT);
		this.setPreferredSize(this.MAIN_SCREEN_WIDTH, this.MAIN_SCREEN_HEIGHT);
		local screenWidth = ::Screen.getWidth();
		local screenHeight = ::Screen.getHeight();
		this.setSticky("center", "bottom");
		this.setPosition(-this.MAIN_SCREEN_WIDTH * 0.5 + 11, -this.MAIN_SCREEN_HEIGHT);
		this.setAppearance("MainScreenUI");
		this.setOverlay("GUI/MainUIOverlay");
		local functionBar = this.GUI.FunctionsBar();
		functionBar.setPosition(404, 32);
		this.add(functionBar);
		local creditShop = this.addShopIcon();
		creditShop.setPosition(658, 28.5);
		this.add(creditShop);
		local quickBar = this.addActionBarIcon();
		quickBar.setPosition(57, 31);
		this.add(quickBar);
		this.mLuckHeroism = this.GUI.LuckHeroismBar();
		this.mLuckHeroism.setPosition(344, -8);
		this.add(this.mLuckHeroism);
		this.mExperienceBar = this.GUI.ExperienceBar();
		this.mExperienceBar.setPosition(62, 66);
		this.add(this.mExperienceBar);
		this.addCurrencyComp();
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Container._removeNotify();
	}

	function addQuickBar( quickBar )
	{
		quickBar.setPosition(84, 26);
		quickBar.setSize((32 + 2) * 8, 32);
		quickBar.setPreferredSize((32 + 2) * 8, 32);
		quickBar.setAppearance(null);
		quickBar.setVisible(true);
		quickBar.mShowContextMenu = false;
		this.add(quickBar);
	}

	function addShopIcon()
	{
		local ICON_SIZE_WIDTH = 52;
		local ICON_SIZE_HEIGHT = 36;
		local button = this.GUI.ImageButton();
		button.setSize(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT);
		button.setPreferredSize(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT);
		button.setAppearance("CashShopIcon");
		button.setGlowImageName("CashShopIcon/Selection");
		button.setTooltip("Credit Shop");
		button.addActionListener(this);
		button.setPressMessage("_onToggleCreditScreen");
		return button;
	}

	function addActionBarIcon()
	{
		local ICON_SIZE_WIDTH = 28;
		local ICON_SIZE_HEIGHT = 28;
		local button = this.GUI.ImageButton();
		button.setSize(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT);
		button.setPreferredSize(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT);
		button.setAppearance("ActionBarGem");
		button.setGlowImageName("ActionBarGem/Selection");
		button.setTooltip("Quick Bars");
		button.addActionListener(this);
		button.setPressMessage("_onToggleQuickBarScreen");
		return button;
	}

	function addCurrencyComp()
	{
		local COMP_WIDTH = 16;
		local COMP_HEIGHT = 16;
		local reagentsOverview = this.GUI.ReagentsOverview();
		reagentsOverview.setSize(13, 13);
		reagentsOverview.setPosition(724, 38);
		local creditsOverview = this.GUI.CreditsOverview();
		creditsOverview.setSize(COMP_WIDTH, COMP_HEIGHT);
		creditsOverview.setPosition(711.5, 25);
		local copperOverview = this.GUI.CopperOverview();
		copperOverview.setSize(COMP_WIDTH, COMP_HEIGHT);
		copperOverview.setPosition(711.5, 49);
		this.add(reagentsOverview);
		this.add(creditsOverview);
		this.add(copperOverview);
	}

	function _onToggleQuickBarScreen( button )
	{
		this.Screens.toggle("QuickbarSwitches");
	}

	function _onToggleCreditScreen( button )
	{
		this.Screens.toggle("CreditShop");
	}

	function fillOut()
	{
		this.mExperienceBar.fillOut();
		this.mLuckHeroism.fillOut();
	}

}

