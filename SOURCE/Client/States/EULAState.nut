this.require("States/StateManager");
this.require("UI/UI");
class this.States.EULAState extends this.State
{
	static mClassName = "EULAState";
	mAgreeButton = null;
	mHTML = null;
	mGUI = null;
	constructor()
	{
	}

	function onEnter()
	{
		local appearance = "EULAScreen";
		this.mGUI = ::GUI.Component();
		this.mGUI.setAppearance(appearance);
		this.mGUI.setSize(::Screen.getWidth(), ::Screen.getHeight());
		::Screen.setOverlayVisible("GUI/Overlay", true);
		this.mGUI.setOverlay("GUI/Overlay");
		local logoGlow = ::GUI.Component();
		logoGlow.setAppearance("EELogo/Glow");
		logoGlow.setSticky("center", "center");
		logoGlow.setPosition(-430, -340);
		logoGlow.setSize(366, 366);
		this.mGUI.add(logoGlow);
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
		this.mGUI.add(panel);
		local panelPaper = ::GUI.Component(this.GUI.BorderLayout());
		panelPaper.setAppearance("Container");
		panelPaper.setSticky("left", "top");
		panelPaper.setPosition(10, 55);
		panelPaper.setSize(430, 430);
		panel.add(panelPaper);
		local scrollArea = ::GUI.ScrollPanel();
		panelPaper.add(scrollArea, this.GUI.BorderLayout.CENTER);
		this.mHTML = ::GUI.HTML();
		this.mHTML.setAppearance("PaperBackBorder");
		this.mHTML.setVisible(true);
		this.mHTML.setLayoutManager(::GUI.FlowLayout());
		this.mHTML.getLayoutManager().setAlignment("left");
		this.mHTML.getLayoutManager().setGaps(0.0, 0.0);
		this.mHTML.setInsets(10, 10, 10, 10);
		this.mHTML.setText(::EULADef);
		scrollArea.attach(this.mHTML);
		local font = ::GUI.Font("Maiandra", 32);
		local loadTitle = ::GUI.Label("");
		loadTitle.setAppearance("Label");
		loadTitle.setPosition(15, 14);
		loadTitle.setFont(font);
		loadTitle.setFontColor("ffff99");
		loadTitle.setText(::TXT("EULA"));
		panel.add(loadTitle);
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
		this.mGUI.add(quitButton);
		this.mAgreeButton = ::GUI.BigButton(::TXT("I Accept"));
		this.mAgreeButton.setLayoutExclude(true);
		this.mAgreeButton.setSticky("center", "center");
		this.mAgreeButton.setPosition(175, 235);
		this.mAgreeButton.setReleaseMessage("onEULAAgree");
		this.mAgreeButton.addActionListener(this);
		this.mGUI.add(this.mAgreeButton);
	}

	function onDestroy()
	{
		this.mGUI.destroy();
	}

	function onEULAAgree( evt )
	{
		this.States.set(this.States.LoginState());
	}

	function onScreenResize( width, height )
	{
		this.mGUI.setSize(width, height);
	}

}

