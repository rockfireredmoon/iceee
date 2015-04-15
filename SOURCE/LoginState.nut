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
		logo.setAppearance("EELogo/New");
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
		this.mMenu.faqButton <- ::GUI.BigButton("Player Support");
		this.mMenu.container.add(this.mMenu.faqButton);
		this.mMenu.faqButton.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://armouree.vm.bytemark.co.uk/dokuwiki/doku.php?id=issues:introduction");
			}

		});
		this.mMenu.privacyButton <- ::GUI.BigButton("Wiki");
		this.mMenu.container.add(this.mMenu.privacyButton);
		this.mMenu.privacyButton.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://armouree.vm.bytemark.co.uk/dokuwiki/doku.php?id=infowiki");
			}

		});
		this.mMenu.forParents <- ::GUI.BigButton("For Dissy");
		this.mMenu.container.add(this.mMenu.forParents);
		this.mMenu.forParents.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://upload.wikimedia.org/wikipedia/commons/5/57/Chicken_-_melbourne_show_2005.jpg");
			}

		});
		this.mMenu.forgotPassword <- ::GUI.BigButton("Forgot Password?");
		this.mMenu.container.add(this.mMenu.forgotPassword);
		this.mMenu.forgotPassword.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://iceee.servegame.com/ResetPassword.html");
			}

		});
		this.mMenu.manageAccount <- ::GUI.BigButton("Create Account");
		this.mMenu.container.add(this.mMenu.manageAccount);
		this.mMenu.manageAccount.addActionListener({
			function onActionPerformed( b )
			{
				this.System.openURL("http://iceee.servegame.com/Account.html");
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
				local message = "Earth Eternal will stream necessary content in the background" + " while playing, but for users with slower connections it may be desirable" + " to download all required media packages while outside the game to ensure optimal performance.";
				local askMessage = this.GUI.MessageBox.showYesNo(message + " Are you sure you want to download all media?", callback);
			}

		});
		this.mMenu.container.add(this.mVersion);
	}

	function _buildLogin()
	{
		this.mCS.baseContainer <- ::GUI.Container();
		this.mCS.setLayoutManager(this.GUI.BorderLayout());
		this.mCS.setSticky("center", "center");
		this.mCS.setPosition(-50, -275);
		this.mCS.setSize(395, 530);
		this.mCS.baseContainer.setOverlay("GUI/Overlay2");
		this.mCS.secondContainer <- ::GUI.Container();
		this.mCS.setLayoutManager(this.GUI.BorderLayout());
		this.mCS.setInsets(10, 0, 0, 0);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.SOUTH);
		this.mCS.accountPanel <- ::GUI.Panel();
		this.mCS.setLayoutManager(this.GUI.GridLayout(2, 1));
		this.mCS.setSize(400, 300);
		this.mCS.setInsets(10, 10, 10, 10);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.NORTH);
		this.mCS.accountTitle <- ::GUI.Label("New to Earth Eternal");
		this.mCS.setFont(this.GUI.Font("Maiandra", 32));
		this.mCS.setTextAlignment(0.5, 0.5);
		this.mCS.add(this.mCS);
		this.mCS.accountButtonContainer <- this.GUI.Container();
		this.mCS.setLayoutManager(this.GUI.BoxLayoutV());
		this.mCS.getLayoutManager().setExpand(false);
		this.mCS.add(this.mCS);
		this.mCS.accountButton <- ::GUI.BigButton("Create Account");
		this.mCS.setReleaseMessage("onCreateAccount");
		this.mCS.add(this.mCS);
		this.mCS.thirdContainer <- ::GUI.Container();
		this.mCS.setLayoutManager(this.GUI.BorderLayout());
		this.mCS.setInsets(10, 0, 0, 0);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.SOUTH);
		this.mCS.loginPanel <- ::GUI.Panel();
		this.mCS.setLayoutManager(this.GUI.BorderLayout());
		this.mCS.setSize(400, 160);
		this.mCS.setInsets(10, 10, 10, 10);
		this.mCS.setFont(this.GUI.Font("Maiandra", 32));
		this.mCS.add(this.mCS, this.GUI.BorderLayout.SOUTH);
		local accountText = "Email:";
		this.mCS.loginGrid <- ::GUI.Container();
		this.mCS.setLayoutManager(this.GUI.GridLayout(2, 2));
		this.mCS.getLayoutManager().setColumns(this.Math.max(this.mCS.getFont().getTextMetrics(accountText).width, this.mCS.getFont().getTextMetrics("Password:").width) + 5, "*");
		this.mCS.getLayoutManager().setGaps(6, 6);
		this.mCS.usernameLabel <- ::GUI.Label(accountText);
		this.mCS.add(this.mCS, this.GUI.GridLayout.FILL);
		this.mCS.usernameInput <- ::GUI.InputArea();
		this.mCS.addActionListener(this);
		this.mCS.add(this.mCS);
		this.mCS.passwordLabel <- ::GUI.Label("Password:");
		this.mCS.add(this.mCS, this.GUI.GridLayout.FILL);
		this.mCS.passwordInput <- ::GUI.InputArea();
		this.mCS.setPassword(true);
		this.mCS.addActionListener(this);
		this.mCS.add(this.mCS);
		this.mCS.buttonContainer <- ::GUI.Container();
		this.mCS.setLayoutManager(this.GUI.BorderLayout());
		this.mCS.setInsets(10, 0, 0, 0);
		this.mCS.loginButton <- this.GUI.BigButton("Login");
		this.mCS.setReleaseMessage("onLogin");
		this.mCS.add(this.mCS, this.GUI.BorderLayout.EAST);
		this.mCS.checkBoxContainer <- this.GUI.Component();
		this.mCS.setLayoutManager(this.GUI.BoxLayout());
		this.mCS.getLayoutManager().setExpand(false);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.WEST);
		this.mCS.checkBox <- this.GUI.CheckBox();
		this.mCS.add(this.mCS);
		this.mCS.checkLabelContainer <- this.GUI.Component();
		this.mCS.setLayoutManager(this.GUI.BoxLayout());
		this.mCS.setInsets(3, 0, 0, 0);
		this.mCS.add(this.mCS);
		this.mCS.checkLabel <- this.GUI.Label("Remember Me");
		this.mCS.setFont(this.GUI.Font("Maiandra", 16));
		this.mCS.add(this.mCS);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.CENTER);
		this.mCS.add(this.mCS, this.GUI.BorderLayout.SOUTH);
		this.mCS.loginButton.addActionListener(this);
		this.mCS.accountButton.addActionListener(this);
		local creds = ::Pref.get("login.Credentials");

		if (creds != "")
		{
			local parts = this.Util.split(this.base64_decode(creds), ":");
			this.mCS.setText(parts[0]);
			this.mCS.setText(parts[1]);
			this.mCS.setChecked(true);
		}

		this.mCS.setTabOrderTarget(this.mCS);
		this.mCS.setTabOrderTarget(this.mCS);
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
		req.open("GET", "http://www.eartheternal.com/in_game_news");
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
			this.System.openURL("http://www.eartheternal.com/affiliate?code=OneDownload");
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
		this.mConnectMessageState = this.States.MessageState("Connecting to Earth Eternal servers...", this.GUI.ConfirmationWindow.CANCEL, this.GUI.ProgressAnimation());
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

