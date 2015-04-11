this.GameEvents <- {};
this.GameEvents.onChatWindowToggle <- function ()
{
	this.Screens.toggle("ChatWindow");
};
this.GameEvents.onInventoryToggle <- function ()
{
	local toggle = ::_displayInventory.isVisible();
	::_displayInventory.setVisible(!toggle);
};
this.GameEvents.onChatListVisibleToggle <- function ()
{
	try
	{
		local toggle = ::_ChatWindow.isListVisible();
		::_ChatWindow.setListVisible(!toggle);
	}
	catch( pID )
	{
	}
};
this.GameEvents.onDebugWindowToggle <- function ()
{
	this._debug.toggleShowingFPS();
};
this.GameEvents.onStartChatInput <- function ()
{
	local currentState = ::_stateManager.peekCurrentState();

	if (currentState.mClassName == "GameState")
	{
		try
		{
			::_ChatWindow.onStartChatInput();
			::_ChatWindow.setVisible(true);
		}
		catch( pID )
		{
			this.GameEvents.onChatWindowToggle();
			::_ChatWindow.onStartChatInput();
		}
	}
	else if (currentState.mClassName == "LoginState")
	{
		if ("onLogin" in currentState)
		{
			currentState.onLogin(null);
		}
	}
};
this.GameEvents.onStartCommandInput <- function ()
{
	this.onStartChatInput();
};
this.GameEvents.onAttackAnimation <- function ()
{
	try
	{
		::_avatar.mAnimationHandler.onFF("1H_Attack");
	}
	catch( pID )
	{
	}
};
