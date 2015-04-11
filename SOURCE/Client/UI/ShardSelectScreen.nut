this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.ShardSelectScreen extends this.GUI.Frame
{
	static mClassName = "Screens.ShardSelectScreen";
	mBrowser = null;
	mSelectButton = null;
	mRefreshButton = null;
	mShardIDs = null;
	constructor()
	{
		this.GUI.Frame.constructor("Shard List");
		this.mShardIDs = [];
		this.mBrowser = this.GUI.ColumnList();
		this.mBrowser.addColumn("Name", 196);
		this.mBrowser.addColumn("Players", 64);
		this.mSelectButton = this.GUI.Button("Select Shard", this, "onSelectShard");
		this.mRefreshButton = this.GUI.Button("Refresh List", this, "onRefresh");
		local actions = this.GUI.Container(this.GUI.BoxLayout());
		actions.getLayoutManager().setPackAlignment(0.5);
		actions.setInsets(0, 0, 5, 5);
		actions.add(this.mSelectButton);
		actions.add(this.mRefreshButton);
		local pane = this.GUI.Container(this.GUI.BorderLayout());
		pane.add(this.mBrowser, this.GUI.BorderLayout.CENTER);
		pane.add(actions, this.GUI.BorderLayout.SOUTH);
		pane.setInsets(5);
		this.setContentPane(pane);
		this.setSize(400, 300);
		this.getShardList();
	}

	function onSelectShard( button )
	{
		local rows = this.mBrowser.getSelectedRows();

		if (rows.len() == 0)
		{
			return;
		}

		local data = this.mShardIDs[rows[0]];

		if (::_stateManager.peekCurrentState().mClassName != "GameState")
		{
			::_Connection.sendQuery("shard.lobbyset", this, [
				data
			]);
		}
		else
		{
			::_Connection.sendQuery("shard.set", this, [
				data
			]);
		}

		this.getShardList();
	}

	function onRefresh( button )
	{
		this.getShardList();
	}

	function getShardList()
	{
		::_Connection.sendQuery("world.list", this, []);
		this.mSelectButton.setEnabled(false);
		this.mRefreshButton.setEnabled(false);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "world.list")
		{
			this.mBrowser.removeAllRows();
			this.mShardIDs = [];
			local selected = 0;
			local x = 0;

			foreach( r in results )
			{
				local id = r[0];
				local name = r[1];
				local players = r[2];
				this.mShardIDs.append(id);

				if (name[0] == 33)
				{
					name = name.slice(1);
					selected = x;
				}

				this.mBrowser.addRow([
					name,
					players
				]);
				x++;
			}

			this.mBrowser.setSelectedRows([
				selected
			]);
			this.mSelectButton.setEnabled(true);
			this.mRefreshButton.setEnabled(true);
		}
	}

	function onQueryTimeout( qa )
	{
		if (qa.query == "world.list")
		{
			this.getShardList();
		}
	}

	function setVisible( value )
	{
		local wasVisible = this.isVisible();
		this.GUI.Frame.setVisible(value);

		if (value && !wasVisible)
		{
			this.getShardList();
		}
	}

}

