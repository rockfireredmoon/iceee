this.require("UI/PvPScreen");
this.require("UI/Screens");
this.TeamSlayerStats <- {
	NAME = 0,
	KILLS = 1,
	DEATHS = 2
};
class this.Screens.TeamSlayerScreen extends this.GUI.PVPScreen
{
	MAX_PLAYERS_ON_EACH_TEAM = 10;
	mStatColumns = 3;
	constructor()
	{
		this.GUI.PVPScreen.constructor("Team Slayer", 2, this.MAX_PLAYERS_ON_EACH_TEAM);
	}

	function initPlayerStats( creatureId, name )
	{
		this._setStat(creatureId, this.TeamSlayerStats.NAME, name);
		this._setStat(creatureId, this.TeamSlayerStats.KILLS, 0);
		this._setStat(creatureId, this.TeamSlayerStats.DEATHS, 0);
	}

	function setStat( creatureId, stat, value )
	{
		if (stat >= 0 && stat <= this.TeamSlayerStats.len())
		{
			this._setStat(creatureId, stat, value);
		}
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
		local headerContainer = this.GUI.Container(this.GUI.GridLayout(1, this.TeamSlayerStats.len()));
		headerContainer.add(this.GUI.Label("Name"));
		headerContainer.add(this.GUI.Label("Kills"));
		headerContainer.add(this.GUI.Label("Deaths"));
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

	function _buildGreenTeamPage()
	{
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		pageContainer.getLayoutManager().setRows(20, "*", 20);
		this.mGreenTeamComponent = this.GUI.Container(this.GUI.GridLayout(this.MAX_PLAYERS_ON_EACH_TEAM, 1));
		this.mGreenTeamComponent.getLayoutManager().setRows(20, 20, 20, 20, 20, 20, 20, 20, 20, 20);
		pageContainer.add(this._buildHeader());
		pageContainer.add(this.mGreenTeamComponent);
		return pageContainer;
	}

	function _buildYellowTeamPage()
	{
		local pageContainer = this.GUI.Container(this.GUI.GridLayout(3, 1));
		pageContainer.getLayoutManager().setRows(20, "*", 20);
		this.mYellowTeamComponent = this.GUI.Container(this.GUI.GridLayout(this.MAX_PLAYERS_ON_EACH_TEAM, 1));
		this.mYellowTeamComponent.getLayoutManager().setRows(20, 20, 20, 20, 20, 20, 20, 20, 20, 20);
		pageContainer.add(this._buildHeader());
		pageContainer.add(this.mYellowTeamComponent);
		return pageContainer;
	}

}

