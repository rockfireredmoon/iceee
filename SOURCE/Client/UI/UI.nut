this.require("GUI/GUI");
this.UI <- {};
this.UI.FatalError <- function ( message )
{
	this.log.error("[FATAL] " + message);
	::_Connection.close();
	::_stateManager.setState(this.States.MessageState("An internal error has occurred.<br />Please try again later.<br /><font size=\"24\" color=\"ffe29b\">Reason: " + message + "</font>", this.GUI.ConfirmationWindow.NONE));
	::Screen.setOverlayVisible("GUI/LoadScreen", false);
};
