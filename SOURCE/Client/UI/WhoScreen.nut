this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.WhoScreen extends this.GUI.Frame
{
	mOutputList = null;
	mUpdateEvent = null;
	mSelectedPlyId = null;
	mPlayers = null;
	USER_NAME = 0;
	CHARACTER_NAME = 1;
	ZONE = 2;
	IP = 3;
	IP_FULL = 4;
	PLY_ID = 5;
	EMAIL = 6;
	constructor()
	{
		this.GUI.Frame.constructor("Results");
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		this.setContentPane(cmain);
		this.setSize(400, 400);
		this.setPosition(50, 50);
		this.mOutputList = this.GUI.ColumnList();
		this.mOutputList.addColumn("Name", 100);
		this.mOutputList.addColumn("Zone", 50);
		this.mOutputList.addColumn("Email", 150);
		this.mOutputList.addActionListener(this);
		cmain.add(this.GUI.ScrollPanel(this.mOutputList), this.GUI.BorderLayout.CENTER);
	}

	function onRefresh( WhoList )
	{
		this.mOutputList.removeAllRows();
		this.mPlayers = [];
		local CharacterCounter = 0;
		local RestoredRowSelection = false;

		foreach( character in WhoList )
		{
			this.mOutputList.addRow([
				character[this.CHARACTER_NAME],
				character[this.ZONE],
				character[this.EMAIL]
			]);
			local plyId = character[this.PLY_ID];
			this.mPlayers.append(plyId);

			if (!RestoredRowSelection && this.mSelectedPlyId == plyId)
			{
				this.mOutputList.setSelectedRows([
					CharacterCounter
				]);
				RestoredRowSelection = true;
			}

			CharacterCounter++;
		}

		if (!RestoredRowSelection)
		{
			this.mOutputList.setSelectedRows(null);
		}
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
	}

	function _removeNotify()
	{
		this.GUI.Frame._removeNotify();
		::_eventScheduler.cancel(this.mUpdateEvent);
	}

	function onQueryComplete( Query, Result )
	{
		if (Query.query == "who")
		{
			this.onRefresh(Result);
		}
	}

	function ResendQuery( args )
	{
		this._Connection.sendQuery("who", this);
	}

	function onRowSelectionChanged( list, row, selected )
	{
		this.mSelectedPlyId = this.mPlayers[row];
	}

	function onDoubleClick( list, evt )
	{
		this._Connection.sendQuery("go", this.DefaultActionHandler(), [
			"Player/" + this.mSelectedPlyId
		]);
	}

	function isVisible()
	{
		return this.GUI.Frame.isVisible();
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);
		this.ResendQuery("who");
	}

	function destroy()
	{
		return this.GUI.Frame.destroy();
	}

}

