this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.EULAScreen extends this.GUI.Component
{
	constructor()
	{
		::print("EULA Screen Activated");
		local appearance = "EULAScreen";
		::GUI.Component.constructor();
		this.setAppearance(appearance);
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
		::Screen.setOverlayVisible("GUI/Overlay", true);
		this.setOverlay("GUI/Overlay");
		local logoGlow = ::GUI.Component();
		logoGlow.setAppearance("EELogo/Glow");
		logoGlow.setSticky("center", "center");
		logoGlow.setPosition(-430, -340);
		logoGlow.setSize(366, 366);
		this.add(logoGlow);
		local logo = ::GUI.Component();
		logo.setAppearance("EELogo");
		logo.setPosition(0, 0);
		logo.setSize(366, 366);
		logoGlow.add(logo);
		local panel = ::GUI.Component();
		panel.setAppearance("GoldBorder");
		panel.setSticky("center", "center");
		panel.setPosition(-75, -275);
		panel.setSize(450, 500);
		this.add(panel);
		local panelPaper = ::GUI.Component();
		panelPaper.setAppearance("PaperBack");
		panelPaper.setSticky("left", "top");
		panelPaper.setPosition(10, 55);
		panelPaper.setSize(390, 430);
		panel.add(panelPaper);
		this.mHTML = ::GUI.HTML();
		this.mHTML.setAppearance("Container");
		this.mHTML.setPosition(20, 20);
		this.mHTML.setSize(360, 405);
		this.mHTML.setVisible(true);
		this.mHTML.setLayoutManager(::GUI.FlowLayout());
		this.mHTML.getLayoutManager().setAlignment("left");
		this.mHTML.getLayoutManager().setGaps(0.0, 0.0);
		this.mHTML.setText(::EULADef);
		panelPaper.add(this.mHTML);
		local font = ::GUI.Font("Maiandra", 32);
		local loadTitle = ::GUI.Label("");
		loadTitle.setAppearance("Label");
		loadTitle.setPosition(15, 14);
		loadTitle.setFont(font);
		loadTitle.setFontColor("ffff99");
		loadTitle.setText(::TXT("EULA"));
		panel.add(loadTitle);
		local scroll = ::GUI.ScrollButtons();
		scroll.setLayoutExclude(true);
		scroll.setSize(25, 50);
		scroll.setIndent(0);
		scroll.setGap(13);
		scroll.attach(this.mHTML);
		local quitButton = ::GUI.BigButton(::TXT("Quit"));
		quitButton.setLayoutExclude(true);
		quitButton.setSticky("center", "center");
		quitButton.setPosition(-75, 235);
		quitButton.addActionListener({
			function onActionPerformed( b )
			{
				this.System.exit();
			}

		});
		this.add(quitButton);
		this.mAgreeButton = ::GUI.BigButton(::TXT("Proceed"));
		this.mAgreeButton.setLayoutExclude(true);
		this.mAgreeButton.setSticky("center", "center");
		this.mAgreeButton.setPosition(200, 235);
		this.mAgreeButton.setReleaseMessage("onEULAAgree");
		this.add(this.mAgreeButton);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		::_root.addListener(this);
	}

	function _removeNotify()
	{
		this.GUI.Component._removeNotify();
		::_root.removeListener(this);
	}

	function onScreenResize()
	{
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
	}

	mAgreeButton = null;
	mHTML = null;
	static mClassName = "EULAScreen";
}

