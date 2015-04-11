this.require("UI/Screens");
local wasVisible = this.Screens.close("GMPetitionScreen");
class this.Screens.GMPetitionScreen extends this.GUI.Frame
{
	mainScreen = null;
	mMyPetitionList = null;
	mPendingPetitionList = null;
	mMyPetitionClose = null;
	mMyPetitionUntake = null;
	mPendingPetitionTake = null;
	mRefreshPetitions = null;
	mPetitionPlayerInfo = null;
	mPetitionProblem = null;
	mPendingPetitions = null;
	mMyPetitions = null;
	constructor()
	{
		this.GUI.Frame.constructor("GM Petition Screen");
		this.mainScreen = this._buildMainScreen();
		this.setSize(650, 550);
		this.setContentPane(this.mainScreen);
	}

	function refreshPetitions()
	{
		::_Connection.sendQuery("petition.list", this);
		this.mPendingPetitions = [];
		this.mMyPetitions = [];
		this.mMyPetitionList.removeAllRows();
		this.mPendingPetitionList.removeAllRows();
		this.mPendingPetitionList.setSelectedRows(null);
		this.mMyPetitionClose.setEnabled(false);
		this.mMyPetitionUntake.setEnabled(false);
		this.mMyPetitionList.setSelectedRows(null);
		this.mPendingPetitionTake.setEnabled(false);
		this.mPetitionPlayerInfo.setText("");
		this.mPetitionProblem.setText("");
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value)
		{
			this.refreshPetitions();
		}

		this.mPendingPetitions = [];
		this.mMyPetitions = [];
	}

	function onRowSelectionChanged( columnList, index, selected )
	{
		if (selected)
		{
			if (columnList == this.mMyPetitionList)
			{
				this.mPendingPetitionList.setSelectedRows(null);
				this.mMyPetitionClose.setEnabled(true);
				this.mMyPetitionUntake.setEnabled(true);
				this.mPendingPetitionTake.setEnabled(false);
				this.setPetitionInfo(this.mMyPetitions[index]);
			}
			else if (columnList == this.mPendingPetitionList)
			{
				this.mMyPetitionList.setSelectedRows(null);
				this.setPetitionInfo(this.mPendingPetitions[index]);
				this.mMyPetitionClose.setEnabled(false);
				this.mMyPetitionUntake.setEnabled(false);
				this.mPendingPetitionTake.setEnabled(true);
			}
		}
	}

	function setPetitionInfo( petition )
	{
		this.mPetitionProblem.setText(petition.description);
		local htmlString = "<b>Reporter:</b><br>" + petition.name + "<br>";
		htmlString += "<br><b>Other characters:</b> <br>";
		local rawCharacters = petition.otherChars;
		rawCharacters = this.Util.replace(rawCharacters, "[", "");
		rawCharacters = this.Util.replace(rawCharacters, "]", "");
		rawCharacters = this.Util.replace(rawCharacters, " ", "");
		local splitCharacters = this.Util.split(rawCharacters, ",");

		foreach( characterName in splitCharacters )
		{
			htmlString += characterName + "<br>";
		}

		local html = this.GUI.HTML(htmlString);
		this.mPetitionPlayerInfo.setText(html.getText());
	}

	function onActionSelected( mb, alt )
	{
		if (alt == "Yes")
		{
			local currentSelected = this.mMyPetitionList.getSelectedRows();

			if (currentSelected.len() > 0)
			{
				::_Connection.sendQuery("petition.doaction", this, [
					this.mMyPetitions[currentSelected[0]].id,
					"close"
				]);
			}
		}
	}

	function onClosePetition( button )
	{
		if (button == this.mMyPetitionClose)
		{
			this.GUI._Manager.addTransientToplevel(this.GUI.MessageBox.showYesNo("This will close the petition.  Has the issue been resolved?", this));
		}
	}

	function onRefreshPetitions( button )
	{
		if (button == this.mRefreshPetitions)
		{
			this.refreshPetitions();
		}
	}

	function onTakePetition( button )
	{
		if (button == this.mPendingPetitionTake)
		{
			local currentSelected = this.mPendingPetitionList.getSelectedRows();

			if (currentSelected.len() > 0)
			{
				::_Connection.sendQuery("petition.doaction", this, [
					this.mPendingPetitions[currentSelected[0]].id,
					"take"
				]);
			}
		}
	}

	function onUntakePetition( button )
	{
		if (button == this.mMyPetitionUntake)
		{
			local currentSelected = this.mMyPetitionList.getSelectedRows();

			if (currentSelected.len() > 0)
			{
				::_Connection.sendQuery("petition.doaction", this, [
					this.mMyPetitions[currentSelected[0]].id,
					"untake"
				]);
			}
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "petition.list")
		{
			local petitionList = [];

			foreach( result in results )
			{
				local petitionStatus = result[0];
				local petitionerName = result[1];
				local petitionId = result[2].tointeger();
				local petitionerOtherChars = result[3];
				local petitionCategory = result[4].tointeger();
				local petitionDescription = result[5];
				local petitionScore = result[6].tointeger();
				local petitionTimestamp = result[7];
				petitionList.append({
					status = petitionStatus,
					name = petitionerName,
					otherChars = petitionerOtherChars,
					id = petitionId,
					category = petitionCategory,
					description = petitionDescription,
					score = petitionScore,
					timestamp = petitionTimestamp
				});
			}

			this.Util.bubbleSort(petitionList, this.sortPetition);

			foreach( petition in petitionList )
			{
				if (petition.status == "mine")
				{
					this.mMyPetitionList.addRow([
						this.PetitionCategory[petition.category].category + " - " + petition.name + "  (" + petition.timestamp + ")"
					]);
					this.mMyPetitions.append(petition);
				}
				else if (petition.status == "pending")
				{
					this.mPendingPetitionList.addRow([
						this.PetitionCategory[petition.category].category + " - " + petition.name + "  (" + petition.timestamp + ")"
					]);
					this.mPendingPetitions.append(petition);
				}
			}
		}
		else if (qa.query == "petition.doaction")
		{
			this.refreshPetitions();
		}
	}

	function onQueryError( qa, results )
	{
		this.IGIS.error(results);
	}

	function sortPetition( p1, p2 )
	{
		if (p1 == p2)
		{
			return 0;
		}

		if (p1.score > p2.score)
		{
			return -1;
		}

		if (p1.score < p2.score)
		{
			return 1;
		}

		return 0;
	}

	function _buildMainScreen()
	{
		local container = this.GUI.Container(this.GUI.BoxLayoutV());
		local petitions = this._buildPetitions();
		local petitionInfo = this._buildPetitionInfo();
		container.add(petitions);
		container.add(petitionInfo);
		return container;
	}

	function _buildPetitions()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(3, 2));
		container.getLayoutManager().setRows(100, 10, 200);
		container.getLayoutManager().setColumns(550, "*");
		container.setInsets(5, 10, 5, 20);
		this.mMyPetitionList = this.GUI.ColumnList();
		this.mMyPetitionList.setSize(100, 200);
		this.mMyPetitionList.addColumn("My Petitions", 550);
		this.mMyPetitionList.addRow([
			"hi"
		]);
		this.mMyPetitionList.addActionListener(this);
		container.add(this.mMyPetitionList);
		local myPetitionButtonContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mMyPetitionClose = this.GUI.Button("Close");
		this.mMyPetitionClose.addActionListener(this);
		this.mMyPetitionClose.setReleaseMessage("onClosePetition");
		this.mMyPetitionUntake = this.GUI.Button("Untake");
		this.mMyPetitionUntake.addActionListener(this);
		this.mMyPetitionUntake.setReleaseMessage("onUntakePetition");
		myPetitionButtonContainer.add(this.mMyPetitionClose);
		myPetitionButtonContainer.add(this.mMyPetitionUntake);
		container.add(myPetitionButtonContainer);
		container.add(this.GUI.Spacer(1, 1), {
			span = 2
		});
		this.mPendingPetitionList = this.GUI.ColumnList();
		this.mPendingPetitionList.setSize(100, 200);
		this.mPendingPetitionList.addColumn("Pending Petitions", 550);
		this.mPendingPetitionList.addRow([
			"hi"
		]);
		this.mPendingPetitionList.addActionListener(this);
		local pendingPeitionScroll = this.GUI.ScrollPanel(this.mPendingPetitionList, this.GUI.BorderLayout.EAST);
		container.add(pendingPeitionScroll);
		local pendingPetitionButtonContainer = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mPendingPetitionTake = this.GUI.Button("Take");
		this.mPendingPetitionTake.addActionListener(this);
		this.mPendingPetitionTake.setReleaseMessage("onTakePetition");
		pendingPetitionButtonContainer.add(this.mPendingPetitionTake);
		this.mRefreshPetitions = this.GUI.Button("Refresh");
		this.mRefreshPetitions.addActionListener(this);
		this.mRefreshPetitions.setReleaseMessage("onRefreshPetitions");
		pendingPetitionButtonContainer.add(this.mRefreshPetitions);
		container.add(pendingPetitionButtonContainer);
		return container;
	}

	function _buildPetitionInfo()
	{
		local container = this.GUI.Container(this.GUI.GridLayout(1, 3));
		container.setInsets(5, 10, 5, 20);
		container.getLayoutManager().setColumns(225, 1, 350);
		this.mPetitionPlayerInfo = this.GUI.HTML();
		this.mPetitionPlayerInfo.setText("");
		local petitionInfoScroll = this.GUI.ScrollPanel(this.mPetitionPlayerInfo, this.GUI.BorderLayout.EAST);
		container.add(petitionInfoScroll);
		local sep = this.GUI.Spacer(1, 1);
		sep.setAppearance("ColumnList/HeadingDivider");
		container.add(sep);
		this.mPetitionProblem = this.GUI.HTML();
		this.mPetitionProblem.setText("");
		petitionInfoScroll = this.GUI.ScrollPanel(this.mPetitionProblem, this.GUI.BorderLayout.EAST);
		container.add(petitionInfoScroll);
		container.setSize(750, 300);
		return container;
	}

}


if (wasVisible)
{
	this.Screens.toggle("GMPetitionScreen");
}
