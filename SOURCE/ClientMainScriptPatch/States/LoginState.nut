this.require("States/StateManager");
this.require("UI/UI");
::_username <- null;
::_password <- null;
::_authtoken <- null;
::_accountPermissionGroup <- null;

class this.States.LoginState extends this.State
{
	static mClassName = "LoginState";
	mLoggingIn = false;
	mMessageBroadcaster = null;
	mGUI = null;
	mMenu = {};
	mCS = {};
	mVersion = null;
	mNews = null;
	mConnectMessageState = null;
	mCheckConnectEvent = null;
	constructor()
	{
	}

	function onEnter()
	{
		local appearance = "EULAScreen";
		this.mGUI = ::GUI.Component();
		this.mGUI.setAppearance(appearance);
		this.mGUI.setSize(::Screen.getWidth(), ::Screen.getHeight());
		this.mVersion = this.GUI.Label();

		if (!this.Util.isDevMode())
		{
			this.mVersion.setText("Version: " + this.gVersion);
		}
		else
		{
			this.mVersion.setText("Version: Dev");
		}

		this.mVersion.setFontColor(this.Color(this.Colors["Soft Grey"]));
		::Screen.setOverlayVisible("GUI/Overlay", true);
		this.mGUI.setOverlay("GUI/Overlay");
		::Pref.setAccount(null);
		local logo = ::GUI.Component();
		logo.setAppearance("ValkalTitle");
		logo.setSticky("center", "center");
		logo.setPosition(-375, -285);
		logo.setSize(752, 235);
		this.mGUI.add(logo);
		this.mMenu = {};
		this.mCS = {};
		this._buildMenu();
		this._buildLogin();
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (("gQuickLogin" in this.getroottable()) && ::gQuickLogin == true && ::Pref.get("login.Credentials") != "")
		{
			this.onLogin(null);
		}
	}

	function _buildMenu()
	{
		this.mMenu.container <- ::GUI.Component();
		this.mMenu.container.setAppearance("Container");
		this.mMenu.container.setSticky("center", "center");
		this.mMenu.container.setPosition(-340, -30);
		this.mMenu.container.setMaximumSize(260, null);
		this.mMenu.container.setResize(true);
		this.mMenu.container.setLayoutManager(this.GUI.BoxLayout(this.GUI.BoxLayout.VERTICAL));
		this.Screen.setOverlayVisible("GUI/Overlay2", true);
		this.mMenu.container.setOverlay("GUI/Overlay2");
		this.mMenu.faqButton <- ::GUI.LtGreenBigButton("Create Account");
		this.mMenu.container.add(this.mMenu.faqButton);
		this.mMenu.faqButton.setReleaseMessage("onCreateAccount");
		this.mMenu.faqButton.addActionListener(this);
		this.mMenu.privacyButton <- ::GUI.BigButton("Project");
		this.mMenu.container.add(this.mMenu.privacyButton);
		this.mMenu.privacyButton.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("https://github.com/rockfireredmoon/iceee/");
			}

		});
		
		
		this.mMenu.forParents <- ::GUI.BigButton("Support");
		this.mMenu.container.add(this.mMenu.forParents);
		this.mMenu.forParents.addActionListener({
			function onActionPerformed( b )
			{			
				this.System.openURL("http://www.theanubianwar.com/support");
			}

		});
		this.mMenu.forgotPassword <- ::GUI.BigButton("Forum");
		this.mMenu.container.add(this.mMenu.forgotPassword);
		this.mMenu.forgotPassword.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://www.theanubianwar.com/forum");
			}

		});
		this.mMenu.manageAccount <- ::GUI.BigButton("Manage Account");
		this.mMenu.container.add(this.mMenu.manageAccount);
		this.mMenu.manageAccount.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://www.theanubianwar.com/user");
			}

		});
		this.mMenu.downloadAll <- ::GUI.BigButton("Download All");
		this.mMenu.container.add(this.mMenu.downloadAll);
		this.mMenu.downloadAll.addActionListener({
			function onActionPerformed( b )
			{
				local callback = {
					function onActionSelected( askMessage, alt )
					{
						if (alt == "Yes")
						{
							local downloadScreen = ::Screens.get("DownloadAllMedia", true);

							if (downloadScreen)
							{
								::Screens.show("DownloadAllMedia");
								downloadScreen.startDownload();
							}
						}
					}

				};
				local message = "Planet Forever will stream necessary content in the background" + " while playing, but for users with slower connections it may be desirable" + " to download all required media packages while outside the game to ensure optimal performance.";
				local askMessage = this.GUI.MessageBox.showYesNo(message + " Are you sure you want to download all media?", callback);
			}

		});
		this.mMenu.container.add(this.mVersion);
	}

	function _buildLogin()
	{
		local baseContainer = mCS.baseContainer <- ::GUI.Container();
		baseContainer.setLayoutManager( GUI.BorderLayout() );
		baseContainer.setSticky("center", "center");
		baseContainer.setPosition( -50, -43);
		baseContainer.setSize( 425, 334 );
		baseContainer.setOverlay("GUI/Overlay2");

		local infoPanel = mCS.infoPanel <- ::GUI.Panel();
		infoPanel.setLayoutManager( GUI.BorderLayout() );
		infoPanel.setInsets( 6, 6, 6, 6 );
		infoPanel.setSize( 400, 185 );
		baseContainer.add( infoPanel, GUI.BorderLayout.NORTH );
		
		local infoTitle = mCS.infoTitle <- ::GUI.Label("NEWS");
		infoTitle.setFont( GUI.Font( "Maiandra", 18 ) );
		infoPanel.add( infoTitle, GUI.BorderLayout.NORTH );
		
		local scrollArea = mCS.scrollArea <- GUI.ScrollPanel();
		scrollArea.setPreferredSize( 400, 165 );
		infoPanel.add( scrollArea, GUI.BorderLayout.CENTER );
		
		mNews = mCS.infoArea <- ::GUI.HTML();
		mNews.setAppearance("PaperBackBorder");
		mNews.setVisible(true);
		mNews.addActionListener(this);
		mNews.setLayoutManager(::GUI.FlowLayout());
		mNews.getLayoutManager().setAlignment("left");
		mNews.getLayoutManager().setGaps(0.0, 0.0);
		mNews.setInsets( 10, 10, 10, 10 );
		mNews.setFont( GUI.Font( "Maiandra", 16 ) );
		mNews.setFontColor("2b1b00");
		mNews.setLinkStaticColor("004183");
		mNews.setLinkHoverColor("007b83");
		mNews.setChangeColorOnHover(true);
		mNews.setText("Loading...");
		scrollArea.attach( mNews );

		fetchNews(); 

		local thirdContainer = mCS.thirdContainer <- ::GUI.Container();
		thirdContainer.setLayoutManager( GUI.BorderLayout() );
		thirdContainer.setInsets( 10, 0, 0, 0 );
		baseContainer.add( thirdContainer, GUI.BorderLayout.SOUTH );

		local loginPanel = mCS.loginPanel <- ::GUI.Panel();		
		loginPanel.setLayoutManager( GUI.BorderLayout() );
		loginPanel.setSize( 400, 140 );
		loginPanel.setInsets( 6, 26, 6, 26 );
		loginPanel.setFont( GUI.Font( "Maiandra", 24 ) );
		thirdContainer.add( loginPanel, GUI.BorderLayout.SOUTH );
		
		local accountText = "Username:";
		
		// The username/password grid
		local loginGrid = mCS.loginGrid <- ::GUI.Container();
		loginGrid.setLayoutManager( GUI.GridLayout(2,2) );
		loginGrid.getLayoutManager().setColumns(
				Math.max(
					loginPanel.getFont().getTextMetrics(accountText).width,
					loginPanel.getFont().getTextMetrics("Password:").width) + 5,
				"*");
		loginGrid.getLayoutManager().setGaps(6,6);
		
		local usernameLabel = mCS.usernameLabel <- ::GUI.Label(accountText);
		loginGrid.add(usernameLabel, GUI.GridLayout.FILL);

		local usernameInput = mCS.usernameInput <- ::GUI.InputArea();
		loginGrid.add( usernameInput );

		local passwordLabel = mCS.passwordLabel <- ::GUI.Label("Password:");
		loginGrid.add(passwordLabel, GUI.GridLayout.FILL);

		local passwordInput = mCS.passwordInput <- ::GUI.InputArea();
		passwordInput.setPassword(true);
		passwordInput.addActionListener(this);
		loginGrid.add(passwordInput);
		
		local buttonContainer = mCS.buttonContainer <- ::GUI.Container();
		buttonContainer.setLayoutManager( GUI.BorderLayout() );
		buttonContainer.setInsets( 10, 0, 0, 0 );
		
		local loginButton = mCS.loginButton <- GUI.BigButton("Login");
		loginButton.setReleaseMessage( "onLogin" );
		buttonContainer.add( loginButton, GUI.BorderLayout.EAST );

		local checkBoxContainer = mCS.checkBoxContainer <- GUI.Component()
		checkBoxContainer.setLayoutManager( GUI.BoxLayout() );
		checkBoxContainer.getLayoutManager().setExpand(false);
		buttonContainer.add( checkBoxContainer, GUI.BorderLayout.WEST );

		local checkBox = mCS.checkBox <- GUI.CheckBox();	
		checkBoxContainer.add( checkBox );

		local checkLabelContainer = mCS.checkLabelContainer <- GUI.Component();
		checkLabelContainer.setLayoutManager( GUI.BoxLayout() )
		checkLabelContainer.setInsets( 3, 0, 0, 0 );
		checkBoxContainer.add( checkLabelContainer );
		
		local checkLabel = mCS.checkLabel <- GUI.Label("Remember Me");
		checkLabel.setFont( GUI.Font( "Maiandra", 16 ) );
		checkLabelContainer.add( checkLabel );
				
		loginPanel.add(loginGrid, GUI.BorderLayout.CENTER);
		loginPanel.add(buttonContainer, GUI.BorderLayout.SOUTH);

		mCS.loginButton.addActionListener( this );

		local creds = ::Pref.get( "login.Credentials" );
		if( creds != "" )
		{			
			local parts = Util.split( base64_decode( creds ), ":" );
						
			usernameInput.setText( parts[0] );
			passwordInput.setText( parts[1] );
			
			checkBox.setChecked(true);
		}

		usernameInput.setTabOrderTarget( passwordInput );
		passwordInput.setTabOrderTarget( usernameInput );	
	}
	
	
	function onLinkClicked( message, data )
	{
		if("href" in data) {
			this.System.openURL(data.href);
		}
	}

	function setNews( input )
	{
		this.log.debug("Martin: Set News Called.");

		if (this.mNews != null)
		{
			this.log.debug("Martin: News updated.");
			this.mNews.setText(input);
			return true;
		}

		return false;
	}

	function fetchNews()
	{
		local req = this.XMLHttpRequest();
		local self = this;
		req.onreadystatechange = function () : ( self )
		{
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
					local text = this.responseText;
					self.setNews(text);
				}
				else
				{
					self.setNews("News Temporarily Unavailable: " + this.status);
				}

				return;
			}
		};
		local txt = "";
		req.open("GET", "http://files.theanubianwar.com/gameinfo/valkal/in_game_news");
		req.send(txt);
	}

	function onInputComplete( input )
	{
		this.onLogin(null);
	}

	function _destroyCS()
	{
		if ("baseContainer" in this.mCS)
		{
			this.mCS.baseContainer.setOverlay(null);
			this.mCS = {};
		}
	}

	function _destroyMenu()
	{
		if ("container" in this.mMenu)
		{
			this.mMenu.container.setOverlay(null);
			this.mMenu = {};
		}
	}

	function onDestroy()
	{
		this._destroyMenu();
		this._destroyCS();
		::Screens.close("Queue");
		this.mGUI.setOverlay(null);
		this.mGUI.destroy();
	}

	function onCreateAccount( button )
	{
		if (this.System.isBrowserEmbedded())
		{
			this.System.openURL("http://www.eartheternal.com/welcome_reg");
		}
		else
		{
			this.System.openURL("http://www.theanubianwar.com/user/register");
		}
	}

	function onLogin( sender )
	{
		if (this.mCS.usernameInput.getText() == "")
		{
			this.GUI.ConfirmationWindow.showOk("Please Enter a Username");
		}
		else if (this.mCS.passwordInput.getText() == "")
		{
			this.GUI.ConfirmationWindow.showOk("Please Enter a Password");
		}
		else
		{
			local username = this.mCS.usernameInput.getText();
			local password = this.mCS.passwordInput.getText();

			if ("checkBox" in this.mCS)
			{
				local checked = this.mCS.checkBox.getChecked();

				if (this.Util.isDevMode())
				{
					::Pref.set("login.Credentials", checked == true ? this.base64_encode(username + ":" + password) : null);
				}
				else
				{
					::Pref.set("login.Credentials", checked == true ? this.base64_encode(username + ":") : null);
				}
			}

			this.doLogin(username, password);
		}
	}

	function doLogin( username, password )
	{
		::_username = username;
		::_password = password;
		this.mConnectMessageState = this.States.MessageState("Connecting to Planet Forever servers...", this.GUI.ConfirmationWindow.CANCEL, this.GUI.ProgressAnimation());
		this.States.push(this.mConnectMessageState);
		local htmlComp = this.mConnectMessageState.getHTMLComp();
		htmlComp.addActionListener(this);
		this.mCheckConnectEvent = ::_eventScheduler.fireIn(30.0, this, "updateConnectMessage");
		this.mLoggingIn = true;
		::_Connection.connect();
	}

	function updateConnectMessage()
	{
		local connectText = "Having trouble connecting? You may be behind a firewall," + " click <a href=\"http://www.eartheternal.com/help/connection-issues\">here</a> for information on what ports you need open to play Earth Eternal.";

		if (this.mConnectMessageState)
		{
			this.mConnectMessageState.updateText(connectText);
		}
	}

	function onLinkClicked( message, data )
	{
		if ("href" in data)
		{
			this.System.openURL(data.href);
		}
	}

	function event_Disconnect( junk )
	{
		if (this.mLoggingIn)
		{
			this.States.pop();
			this.States.push(this.States.MessageState("The server has disconnected.", this.GUI.ConfirmationWindow.OK));
			this.mLoggingIn = false;
		}

		::Screens.close("Queue");
	}

	function event_AuthFailure( msg )
	{
		this.States.pop();
		::States.push(::States.MessageState(msg, ::GUI.ConfirmationWindow.OK));
	}

	function event_PersonaList( results )
	{
		::Pref.setAccount(::_username);
		::Pref.download(this.Pref.ACCOUNT);
		this.States.set(this.States.LoadState("CharSelection", this.States.CharacterSelectionState(results)));
	}

	function onReturn( state, value )
	{
		if (value == false)
		{
			this.mLoggingIn = false;
			::_username = null;
			::_password = null;
			::_accountPermissionGroup = null;
			::_Connection.close();
			this.log.debug("User cancelled the connection!");
		}
	}

	function event_queueChanged( data )
	{
		::_Connection.cancelPersonaList();

		if (this.mLoggingIn)
		{
			this.mLoggingIn = false;
			this.States.pop();
		}

		if (data[0] == 0)
		{
			::_Connection.sendQuery("persona.list", this.PersonaListHandler());
		}

		if (data[0] > 5)
		{
			local queue = ::Screens.show("Queue");
			queue.setQueuePosition(data[0], data[1]);
		}
		else
		{
			::Screens.close("Queue");
		}
	}

	function onPackageComplete( name )
	{
		::_Connection.attemptToConnect();
	}

	function onPackageError( pkg, error )
	{
		this.log.debug("Error loading package " + pkg + " - " + error);
		this.onPackageComplete(pkg);
	}

	function onScreenResize( width, height )
	{
		this.mGUI.setSize(width, height);
	}

}

