this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.PartyScreen extends this.GUI.Frame
{
	static mClassName = "Screens.PartyScreen";
	constructor()
	{
		this.GUI.Frame.constructor("PartyScreen");
		::_Connection.addListener(this);
		local layout = this.GUI.Container(this.GUI.GridLayout(1, 1));
		layout.getLayoutManager().setRows("*");
		layout.getLayoutManager().setColumns("*");
		layout.setInsets(4);
		layout.add(this.GUI.Label("PartyScreen UI"));
		this.setContentPane(layout);
		this.setSize(100, 100);
		this.centerOnScreen();
		this.setCached(::Pref.get("video.UICache"));
	}

	function onPartyUpdated( callingFunction )
	{
		this.log.debug("onPartyUpdated()");
	}

}

