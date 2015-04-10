require("States/LoginState")

/* Apparently the login screen, where it says "Email:" confuses nearly everyone.
   Here's a new experimental hack to change the UI element after it has been
   loaded into the table, so it says "Username:" instead.
*/

function States::LoginState::onEnterHack()
{

try
{

	onEnterOriginal();
	if(typeof mCS == "table")
	{
		if("usernameLabel" in mCS)
		{
			mCS["usernameLabel"].setText("Username:");
		}
	}


}
catch(e)
{
}


}

States.LoginState.onEnterOriginal <- States.LoginState.onEnter;
States.LoginState.onEnter <- States.LoginState.onEnterHack;