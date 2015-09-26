this.require("States/StateManager");
class this.States.WebAuthState extends this.State
{
	mConnectMessageState = null;
	mCheckConnectEvent = null;
	mComponent = null;
	mBackground = null;
	constructor()
	{
		print("ICE! Creating WebAuthState\n");
	}

	function onEnter()
	{
		print("ICE! ENTER WEBAUTH " + ::_args.web_auth_token + "\n");
		
		this.mComponent = this.GUI.Component();
		this.mComponent.setAppearance("GoldBorder");
		this.mComponent.setSize(::Screen.getWidth(), ::Screen.getHeight());
		this.mBackground = ::GUI.Component();
		this.mBackground.setAppearance("LoadScreen");
		this.mBackground.setSticky("center", "center");
		local width = ::Screen.getHeight() * 1.7708;
		this.mBackground.setSize(width.tointeger(), ::Screen.getHeight());
		this.mBackground.setPosition(this.mBackground.getWidth() / 2 * -1, this.mBackground.getHeight() / 2 * -1);
		this.mBackground.setVisible(true);
		this.mComponent.add(this.mBackground);
		this.mComponent.setVisible(true);
		this.mComponent.setOverlay(this.GUI.OVERLAY);
		this.mConnectMessageState = this.States.MessageState("Connecting to Earth Eternal servers...", this.GUI.ConfirmationWindow.CANCEL, this.GUI.ProgressAnimation());
		this.States.push(this.mConnectMessageState);
		local htmlComp = this.mConnectMessageState.getHTMLComp();
		htmlComp.addActionListener(this);
		this.mCheckConnectEvent = ::_eventScheduler.fireIn(30.0, this, "updateConnectMessage");
		::_Connection.setAuthToken(::_args.web_auth_token);
		::_Connection.connect();
	}

	function onReturn( state, value )
	{
		::_Connection.close();
	}

	function onDestroy()
	{
		::Screens.close("Queue");
		this.mComponent.destroy();
		this.mComponent = null;
	}

	function event_AuthFailure( msg )
	{
		this.event_Disconnect(msg);
	}

	function event_Disconnect( junk )
	{
		this.States.set(this.States.LoginState());
		::Screens.close("Queue");
	}

	function event_PersonaList( results )
	{
		if (::_username)
		{
			::Pref.setAccount(::_username);
			::Pref.download(this.Pref.ACCOUNT);
		}

		this.States.set(this.States.LoadState("CharSelection", this.States.CharacterSelectionState(results)));
	}

	function onScreenResize()
	{
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
		local width = ::Screen.getHeight() * 1.7708;
		this.mBackground.setSize(width.tointeger(), ::Screen.getHeight());
		this.mBackground.setPosition(this.mBackground.getWidth() / 2 * -1, this.mBackground.getHeight() / 2 * -1);
	}

	function updateConnectMessage()
	{
		local connectText = "Having trouble connecting? You may be behind a firewall," + " click <a href=\"http://www.eartheternal.com/help/connection-issues\">here</a> for information on what ports you need open to play Earth Eternal.";

		if (this.mConnectMessageState)
		{
			this.mConnectMessageState.updateText(connectText);
		}
	}

}

