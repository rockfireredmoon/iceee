this.require("UI/PvPScreen");
this.require("UI/Screens");
this.CTFStats <- {
	NAME = 0,
	KILLS = 1,
	DEATHS = 2,
	FLAG_CAPTURES = 3
};
class this.Screens.CTFScreen extends this.GUI.PVPScreen
{
	MAX_PLAYERS_ON_EACH_TEAM = 10;
	mStatColumns = 4;
	constructor()
	{
		this.GUI.PVPScreen.constructor("Capture the Flag", 2, this.MAX_PLAYERS_ON_EACH_TEAM);
	}

	function initPlayerStats( creatureId, name )
	{
		this._setStat(creatureId, this.CTFStats.NAME, name);
		this._setStat(creatureId, this.CTFStats.KILLS, 0);
		this._setStat(creatureId, this.CTFStats.DEATHS, 0);
		this._setStat(creatureId, this.CTFStats.FLAG_CAPTURES, 0);
	}

	function setStat( creatureId, stat, value )
	{
		if (stat >= 0 && stat <= this.CTFStats.len())
		{
			this._setStat(creatureId, stat, value);
		}
	}

	function handleFlagEvent( playerName, flagType, flagEvent )
	{
		local message = playerName + " ";
		local useExclamation = false;

		switch(flagEvent)
		{
		case this.FlagEvents.TAKEN:
			message += "took ";
			useExclamation = true;
			break;

		case this.FlagEvents.DROPPED:
			message += "dropped ";
			useExclamation = false;
			break;

		case this.FlagEvents.CAPTURED:
			message += "captured ";
			useExclamation = true;
			break;

		case this.FlagEvents.RETURNED:
			message += "returned ";
			useExclamation = false;
			break;
		}

		message += "the ";

		switch(flagType)
		{
		case this.FlagType.RED:
			message += "red flag";
			break;

		case this.FlagType.BLUE:
			message += "blue flag";
			break;
		}

		if (useExclamation)
		{
			message += "!";
		}
		else
		{
			message += ".";
		}

		this.IGIS.info(message);
	}

	function _buildFooter()
	{
		local footerContainer = this.GUI.Container(this.GUI.GridLayout(1, 3));
		footerContainer.add(this.mStatusLabel);
		footerContainer.add(this.mTimerLabel);
		return footerContainer;
	}

	function _buildHeader()
	{
		local headerContainer = this.GUI.Container(this.GUI.GridLayout(1, this.CTFStats.len()));
		headerContainer.add(this.GUI.Label("Name"));
		headerContainer.add(this.GUI.Label("Kills"));
		headerContainer.add(this.GUI.Label("Deaths"));
		headerContainer.add(this.GUI.Label("Flag Captures"));
		headerContainer.setAppearance("PVPScreenHeader");
		return headerContainer;
	}

	function _buildBlueTeamPage()
	{
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		pageContainer.getLayoutManager().setRows(20, "*", 20);
		this.mBlueTeamComponent = this.GUI.Container(this.GUI.GridLayout(this.MAX_PLAYERS_ON_EACH_TEAM, 1));
		this.mBlueTeamComponent.getLayoutManager().setRows(20, 20, 20, 20, 20, 20, 20, 20, 20, 20);
		pageContainer.add(this._buildHeader());
		pageContainer.add(this.mBlueTeamComponent);
		return pageContainer;
	}

	function _buildOverallPage()
	{
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		pageContainer.getLayoutManager().setRows(20, "*", 20);
		this.mOverallComponent = this.GUI.Container(this.GUI.GridLayout(this.MAX_PLAYERS_ON_EACH_TEAM * 4, 1));
		this.mOverallComponent.getLayoutManager().setRows(20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20);
		pageContainer.add(this._buildHeader());
		pageContainer.add(this.mOverallComponent);
		return pageContainer;
	}

	function _buildRedTeamPage()
	{
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		pageContainer.getLayoutManager().setRows(20, "*", 20);
		this.mRedTeamComponent = this.GUI.Container(this.GUI.GridLayout(this.MAX_PLAYERS_ON_EACH_TEAM, 1));
		this.mRedTeamComponent.getLayoutManager().setRows(20, 20, 20, 20, 20, 20, 20, 20, 20, 20);
		pageContainer.add(this._buildHeader());
		pageContainer.add(this.mRedTeamComponent);
		return pageContainer;
	}

}

