this.require("Relay");
this.GUI <- {
	_NextID = 0,
	CurrentSkin = "DefaultSkin",
	DefaultFont = null,
	DefaultFontColor = "ffffff",
	DefaultButtonFontColor = "FFFFFF",
	OVERLAY = "GUI/Overlay",
	CONFIRMATION_OVERLAY = "GUI/ConfirmationOverlay",
	POPUP_OVERLAY = "GUI/PopupOverlay",
	ROLLOUT_OVERLAY = "GUI/RolloutOverlay",
	TOOLTIP_OVERLAY = "GUI/TooltipOverlay",
	CURSOR_OVERLAY = "GUI/CursorOverlay",
	RegExp = {
		Letter = this.regexp("^([^<>\\s]{1})"),
		Word = this.regexp("^([^<>\\s]+)"),
		WordNoSlash = this.regexp("^([^<>/\\s]+)"),
		Space = this.regexp("^(\\s)"),
		Slash = this.regexp("^([/]{1})"),
		NewLine = this.regexp("^([\\n\\r\\t]+)"),
		AnyOne = this.regexp("^(.{1})"),
		AnyWord = this.regexp("^([^\\s]+)"),
		AnyLetter = this.regexp("^([^\\s]{1})"),
		Tag = this.regexp("^<([^>]+)/?>"),
		Attributes = this.regexp("\\s*(\\w+)\\s*=\\s*\\\"?([^\"]+)\\\"?\\s*")
	}
};
this.GUI.ToggleOverlayVisibility <- function ( pName )
{
	::Assert.isStringBlank(pName, "GUI.ToggleOverlayVisibility", "pName");
	local vis = ::Screen.isOverlayVisible(pName);

	if (::Screen.setOverlayVisible(pName, !vis))
	{
		return true;
	}
	else
	{
		::print("***ERROR*** GUI.ToggleOverlayVisible: The overlay " + pName + " was likely not found");
		return false;
	}
};
this.gDebugGUILayout <- false;
this.GUI._LayoutDebugDepth <- 0;
this.GUI.DebugLayout <- function ( text, ... )
{
	if (vargc > 0 && vargv[0] < 0)
	{
		this.GUI._LayoutDebugDepth -= 1;

		if (this.GUI._LayoutDebugDepth < 0)
		{
			this.GUI._LayoutDebugDepth = 0;
		}
	}

	local str = "[Layout] ";

	for( local i = 0; i < this.GUI._LayoutDebugDepth; i++ )
	{
		str += "   ";
	}

	str += text;
	this.print(str);

	if (vargc > 0 && vargv[0] > 0)
	{
		this.GUI._LayoutDebugDepth += 1;
	}
};
class this.GUI.Adjustable 
{
	function setAdjustmentValues( min, value, max, visible )
	{
	}

	function getAdjustmentValue()
	{
	}

	function setAdjustmentValue( value )
	{
	}

	function getAdjustmentValues()
	{
	}

}

