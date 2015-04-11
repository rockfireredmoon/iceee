this.require("GUI/GUI");
class this.GUI.PVPScreen extends this.GUI.Frame
{
	OVERALL_TAB = "Overall";
	BLUE_TEAM_TAB = "Blue Team";
	RED_TEAM_TAB = "Red Team";
	GREEN_TEAM_TAB = "Green Team";
	YELLOW_TEAM_TAB = "Yellow Tab";
	mTabbedPane = null;
	mRedTeam = null;
	mBlueTeam = null;
	mYellowTeam = null;
	mGreenTeam = null;
	mOverallComponent = null;
	mRedTeamComponent = null;
	mBlueTeamComponent = null;
	mYellowTeamComponent = null;
	mGreenTeamComponent = null;
	mMaxPlayersOnEachTeam = 0;
	mUpdateTimeEvent = null;
	mTimer = null;
	mLastUpdatedTime = null;
	mTimeRemaining = 0;
	mStatusTimeContainer = null;
	mStatusLabel = null;
	mTimeLabel = null;
	constructor( name, teams, maxPlayersOnEachTeam )
	{
		this.GUI.Frame.constructor(name);
		this.mTabbedPane = this.GUI.TabbedPane();
		this.mTabbedPane.addActionListener(this);
		this.mLastUpdatedTime = 0;
		this.mTimeRemaining = 0;
		this.mTimer = ::Timer();
		this.mStatusLabel = this.GUI.Label("");
		this.mTimeLabel = this.GUI.Label("");
		this.mStatusTimeContainer = this.GUI.Container(this.GUI.GridLayout(1, 4));
		this.mStatusTimeContainer.add(this.mStatusLabel);
		this.mStatusTimeContainer.add(this.GUI.Spacer());
		this.mStatusTimeContainer.add(this.GUI.Spacer());
		this.mStatusTimeContainer.add(this.mTimeLabel);
		this.mMaxPlayersOnEachTeam = maxPlayersOnEachTeam;
		this.mRedTeam = {};
		this.mBlueTeam = {};
		this.mYellowTeam = {};
		this.mGreenTeam = {};

		switch(teams)
		{
		case 0:
		case 1:
			this.mTabbedPane.addTab(this.OVERALL_TAB, this._buildOverallPage());
			break;

		case 2:
			this.mTabbedPane.addTab(this.OVERALL_TAB, this._buildOverallPage());
			this.mTabbedPane.addTab(this.BLUE_TEAM_TAB, this._buildBlueTeamPage());
			this.mTabbedPane.addTab(this.RED_TEAM_TAB, this._buildRedTeamPage());
			break;

		case 3:
			this.mTabbedPane.addTab(this.OVERALL_TAB, this._buildOverallPage());
			this.mTabbedPane.addTab(this.BLUE_TEAM_TAB, this._buildBlueTeamPage());
			this.mTabbedPane.addTab(this.RED_TEAM_TAB, this._buildRedTeamPage());
			this.mTabbedPane.addTab(this.GREEN_TEAM_TAB, this._buildGreenTeamPage());
			break;

		case 3:
			this.mTabbedPane.addTab(this.OVERALL_TAB, this._buildOverallPage());
			this.mTabbedPane.addTab(this.BLUE_TEAM_TAB, this._buildBlueTeamPage());
			this.mTabbedPane.addTab(this.RED_TEAM_TAB, this._buildRedTeamPage());
			this.mTabbedPane.addTab(this.GREEN_TEAM_TAB, this._buildGreenTeamPage());
			this.mTabbedPane.addTab(this.YELLOW_TEAM_TAB, this._buildYellowTeamPage());
			break;
		}

		this.setSize(600, 450);
		this.setContentPane(this.mTabbedPane);
	}

	function addPlayer( team, creatureId )
	{
		local teamToJoin;

		switch(team)
		{
		case this.PVPTeams.BLUE:
			teamToJoin = this.mBlueTeam;
			break;

		case this.PVPTeams.RED:
			teamToJoin = this.mRedTeam;
			break;

		case this.PVPTeams.YELLOW:
			teamToJoin = this.mYellowTeam;
			break;

		case this.PVPTeams.GREEN:
			teamToJoin = this.mGreenTeam;
			break;

		default:
			return;
		}

		if (teamToJoin.len() >= this.mMaxPlayersOnEachTeam)
		{
			return;
		}

		local newPlayer = this.GUI.PVPPlayerLine(team, this.mStatColumns);
		teamToJoin[creatureId] <- newPlayer;
		this.updateScreen();
	}

	function initPlayerStats( creatureId, name )
	{
		return false;
	}

	function handleFlagEvent( playerName, flagType, flagEvent )
	{
		return false;
	}

	function onTabSelected( tabPane, tab )
	{
		if (this.mTabbedPane != tabPane)
		{
			return;
		}

		this.updateScreen(tab);
		tab.component.add(this.mStatusTimeContainer);
	}

	function pvpTimeUpdate()
	{
		local currentTime = this.mTimer.getMilliseconds();
		local timeSinceLastUpdate = currentTime - this.mLastUpdatedTime;

		if (this.mTimeRemaining > 0)
		{
			this.setRemainingTime(this.mTimeRemaining - timeSinceLastUpdate);
		}
		else
		{
			this.setRemainingTime(0);
		}
	}

	function removePlayer( creatureId )
	{
		local team = this._findTeamPlayerIsOn(creatureId);

		if (!team)
		{
			return false;
		}

		local teamComponent = this._findTeamComponentPlayerIsOn(creatureId);

		if (!teamComponent)
		{
			return false;
		}

		teamComponent.remove(team[creatureId]);
		this.mOverallComponent.remove(team[creatureId]);
		delete team[creatureId];
		return true;
	}

	function setRemainingTime( value )
	{
		if (value <= 0)
		{
			this.mTimeRemaining = 0;
			this.mTimeLabel.setText("");
		}
		else
		{
			this.mTimeRemaining = value;
			this.mLastUpdatedTime = this.mTimer.getMilliseconds();
			local timeLeftTable = this.Util.paraseMiliToTable(value);
			local timeLeftString = "";

			if (timeLeftTable.h > 0)
			{
				timeLeftString += timeLeftTable.h + ":";
			}

			if (timeLeftTable.m > 0)
			{
				if (timeLeftTable.m <= 9)
				{
					timeLeftString += "0";
				}

				timeLeftString += timeLeftTable.m + ":";
			}
			else
			{
				timeLeftString += "00:";
			}

			if (timeLeftTable.s <= 9)
			{
				timeLeftString += "0";
			}

			timeLeftString += timeLeftTable.s;
			this.mTimeLabel.setText("Time remaining: " + timeLeftString);
		}
	}

	function setVisible( value )
	{
		if (value == true)
		{
			if (this.mTimeRemaining > 0 && this.mUpdateTimeEvent == null)
			{
				this.mUpdateTimeEvent = ::_eventScheduler.repeatIn(0.0, 0.5, this, "pvpTimeUpdate");
			}

			local tab = this.mTabbedPane.getSelectedTab();

			switch(tab.name)
			{
			case this.OVERALL_TAB:
				this._updateOverall();
				break;

			case this.BLUE_TEAM_TAB:
				this._updateTeam(this.mBlueTeam, this.mBlueTeamComponent);
				break;

			case this.RED_TEAM_TAB:
				this._updateTeam(this.mRedTeam, this.mRedTeamComponent);
				break;

			case this.GREEN_TEAM_TAB:
				this._updateTeam(this.mGreenTeam, this.mGreenTeamComponent);
				break;

			case this.YELLOW_TEAM_TAB:
				this._updateTeam(this.mYellowTeam, this.mYellowTeamComponent);
				break;
			}
		}
		else if (this.mUpdateTimeEvent)
		{
			::_eventScheduler.cancel(this.mUpdateTimeEvent);
			this.mUpdateTimeEvent = null;
		}

		this.GUI.Frame.setVisible(value);
	}

	function updateScreen( ... )
	{
		local tab;

		if (vargc > 0)
		{
			tab = vargv[0];
		}
		else
		{
			tab = this.mTabbedPane.getSelectedTab();
		}

		switch(tab.name)
		{
		case this.OVERALL_TAB:
			this._updateOverall();
			break;

		case this.BLUE_TEAM_TAB:
			this._updateTeam(this.mBlueTeam, this.mBlueTeamComponent);
			break;

		case this.RED_TEAM_TAB:
			this._updateTeam(this.mRedTeam, this.mRedTeamComponent);
			break;

		case this.GREEN_TEAM_TAB:
			this._updateTeam(this.mGreenTeam, this.mGreenTeamComponent);
			break;

		case this.YELLOW_TEAM_TAB:
			this._updateTeam(this.mYellowTeam, this.mYellowTeamComponent);
			break;
		}
	}

	function updateState( gameState )
	{
		switch($[stack offset 1])
		{
		case this.PVPGameState.WAITING_TO_START:
			foreach( id, value in this.mRedTeam )
			{
				this._resetCreatureStats(id);
			}

			foreach( id, value in this.mBlueTeam )
			{
				this._resetCreatureStats(id);
			}

			foreach( id, value in this.mYellowTeam )
			{
				this._resetCreatureStats(id);
			}

			foreach( id, value in this.mGreenTeam )
			{
				this._resetCreatureStats(id);
			}

			this.mStatusLabel.setText("Waiting for additional players.");
			break;

		case this.PVPGameState.WAITING_TO_CONTINUE:
			this.mStatusLabel.setText("Waiting to continue.");
			break;

		case this.PVPGameState.PLAYING:
			this.mStatusLabel.setText("Playing");
			break;

		case this.PVPGameState.POST_GAME_LOBBY:
			this.mStatusLabel.setText("Game has completed");
			this.updateTime(0);
			break;

		default:
			return;
		}
	}

	function updateTime( timeLeft )
	{
		this.setRemainingTime(timeLeft);

		if (this.mUpdateTimeEvent == null && timeLeft > 0)
		{
			this.mUpdateTimeEvent = ::_eventScheduler.repeatIn(0.0, 0.5, this, "pvpTimeUpdate");
		}
	}

	function _buildBlueTeamPage()
	{
		return null;
	}

	function _buildOverallPage()
	{
		return null;
	}

	function _buildRedTeamPage()
	{
		return null;
	}

	function _buildGreenTeamPage()
	{
		return null;
	}

	function _buildYellowTeamPage()
	{
		return null;
	}

	function _findTeamPlayerIsOn( creatureId )
	{
		local team;

		if (creatureId in this.mRedTeam)
		{
			team = this.mRedTeam;
		}
		else if (creatureId in this.mBlueTeam)
		{
			team = this.mBlueTeam;
		}
		else if (creatureId in this.mYellowTeam)
		{
			team = this.mYellowTeam;
		}
		else if (creatureId in this.mGreenTeam)
		{
			team = this.mGreenTeam;
		}

		return team;
	}

	function _findTeamComponentPlayerIsOn( creatureId )
	{
		local component;

		if (creatureId in this.mRedTeam)
		{
			component = this.mRedTeamComponent;
		}
		else if (creatureId in this.mBlueTeam)
		{
			component = this.mBlueTeamComponent;
		}
		else if (creatureId in this.mYellowTeam)
		{
			component = this.mYellowTeamComponent;
		}
		else if (creatureId in this.mGreenTeam)
		{
			component = this.mGreenTeamComponent;
		}

		return component;
	}

	function _updateOverall()
	{
		if (this.mRedTeam != null)
		{
			foreach( player in this.mRedTeam )
			{
				this.mOverallComponent.add(player);
			}
		}

		if (this.mBlueTeam != null)
		{
			foreach( player in this.mBlueTeam )
			{
				this.mOverallComponent.add(player);
			}
		}

		if (this.mYellowTeam != null)
		{
			foreach( player in this.mYellowTeam )
			{
				this.mOverallComponent.add(player);
			}
		}

		if (this.mGreenTeam != null)
		{
			foreach( player in this.mGreenTeam )
			{
				this.mOverallComponent.add(player);
			}
		}
	}

	function _resetCreatureStats( creatureId )
	{
		local team = this._findTeamPlayerIsOn(creatureId);

		if (team == null)
		{
			return;
		}

		local totalStats = team[creatureId].getTotalNumStats();

		for( local i = 1; i < totalStats; i++ )
		{
			team[creatureId].setStatText(i, "0");
		}
	}

	function _setStat( creatureId, column, value )
	{
		local team = this._findTeamPlayerIsOn(creatureId);

		if (team == null)
		{
			return;
		}

		team[creatureId].setStatText(column, value.tostring());
	}

	function _updateTeam( team, tabComponent )
	{
		if (team != null)
		{
			foreach( player in team )
			{
				tabComponent.add(player);
			}
		}
	}

}

class this.GUI.PVPPlayerLine extends this.GUI.Container
{
	mStats = null;
	mTeam = null;
	constructor( team, columns )
	{
		this.GUI.Container.constructor(this.GUI.GridLayout(1, columns));
		this.mStats = [];

		for( local i = 0; i < columns; i++ )
		{
			this.mStats.append(this.GUI.Label(""));
			this.add(this.mStats[i]);
		}

		this.mTeam = team;

		switch(this.mTeam)
		{
		case this.PVPTeams.RED:
			this.setAppearance("PVPRedPlayerHighlight");
			break;

		case this.PVPTeams.BLUE:
			this.setAppearance("PVPBluePlayerHighlight");
			break;

		case this.PVPTeams.GREEN:
			this.setAppearance("PVPGreenPlayerHighlight");
			break;

		case this.PVPTeams.YELLOW:
			this.setAppearance("PVPYellowPlayerHighlight");
			break;
		}
	}

	function getTotalNumStats()
	{
		return this.mStats.len();
	}

	function setStatText( column, text )
	{
		if (column < this.mStats.len())
		{
			this.mStats[column].setText(text);
		}
	}

}

