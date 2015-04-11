this.require("UI/UI");
this.require("UI/Screens");
class this.GUI.CreatureBrowser extends this.GUI.Container
{
	static mClassName = "CreatureBrowser";
	mTypeDropdown = null;
	mFilterInput = null;
	mList = null;
	mSearchButton = null;
	mCurrentResult = null;
	mResults = null;
	static mTypeMap = {
		C = "Creature",
		P = "Package"
	};
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.BorderLayout());
		this.mResults = [];
		this.mTypeDropdown = this.GUI.DropDownList();
		this.mTypeDropdown.addChoice("-All-");
		this.mTypeDropdown.addChoice("Creatures");
		this.mTypeDropdown.addChoice("Spawn Packages");
		this.mTypeDropdown.addChoice("Incomplete Creatures");
		this.mTypeDropdown.addSelectionChangeListener(this);
		this.mFilterInput = this.GUI.InputArea();
		this.mFilterInput.addActionListener(this);
		this.mSearchButton = this.GUI.Button("Search", this, "onSearchPressed");
		this.mList = this.GUI.ColumnList();
		this.mList.addColumn("ID", 60);
		this.mList.addColumn("Name", 240);
		this.mList.addColumn("Type", 100);
		this.mList.addActionListener(this);
		local top = this.GUI.Container(this.GUI.BorderLayout());
		top.add(this.GUI.Container(this.GUI.GridLayout(1, 2), this.mFilterInput, this.mTypeDropdown), this.GUI.BorderLayout.CENTER);
		top.add(this.mSearchButton, this.GUI.BorderLayout.EAST);
		top.setInsets(0, 0, 5, 0);
		this.add(top, this.GUI.BorderLayout.NORTH);
		this.add(this.GUI.ScrollPanel(this.mList), this.GUI.BorderLayout.CENTER);
	}

	function onQueryComplete( qa, results )
	{
		this.mList.removeAllRows();
		this.mResults = [];

		foreach( row in results )
		{
			local r = {
				id = row[0],
				name = row[1],
				type = this.mTypeMap[row[2]]
			};
			this.mResults.append(r);
			this.mList.addRow([
				r.id,
				r.name,
				r.type
			]);
		}
	}

	function onSelectionChange( list )
	{
		this.refresh();
	}

	function onRowSelectionChanged( list, index, selected )
	{
		if (index >= this.mResults.len())
		{
			return;
		}

		if (selected)
		{
			this.mCurrentResult = this.mResults[index];
		}
		else
		{
			this.mCurrentResult = null;
		}
	}

	function onInputComplete( button )
	{
		this.refresh();
	}

	function getSelection()
	{
		return this.mCurrentResult;
	}

	function refresh()
	{
		local type = this.mTypeDropdown.getCurrent();

		if (type == "-All-")
		{
			type = "";
		}
		else if (type == "Incomplete Creatures")
		{
			type = "NoAppearanceCreatures";
		}
		else if (type == "Spawn Packages")
		{
			type = "Packages";
		}

		local filter = this.mFilterInput.getText();

		if (filter.len() < 2)
		{
			this.mList.removeAllRows();
			this.mResults = [];
			this.mList.addRow([
				"",
				"Search string must be at least 2 characters",
				""
			]);
			return;
		}

		this._Connection.sendQuery("spawn.list", this, [
			type,
			filter
		]);
	}

	function onSearchPressed( button )
	{
		this.refresh();
	}

}

class this.Screens.CreatureBrowserScreen extends this.GUI.Frame
{
	static mClassName = "Screens.CreatureBrowserScreen";
	mBrowser = null;
	mSpawnButton = null;
	mTweakButton = null;
	constructor()
	{
		this.GUI.Frame.constructor("Browse Spawnables");
		this.mBrowser = this.GUI.CreatureBrowser();
		this.mBrowser.setInsets(5);
		this.mSpawnButton = this.GUI.Button("Create Spawn", this, "onCreateSpawn");
		this.mTweakButton = this.GUI.Button("Tweak", this, "onTweakPressed");
		local actions = this.GUI.Container(this.GUI.BoxLayout());
		actions.getLayoutManager().setPackAlignment(0.5);
		actions.setInsets(0, 0, 5, 0);
		actions.add(this.mSpawnButton);
		actions.add(this.mTweakButton);
		local pane = this.GUI.Container(this.GUI.BorderLayout());
		pane.add(this.mBrowser, this.GUI.BorderLayout.CENTER);
		pane.add(actions, this.GUI.BorderLayout.SOUTH);
		this.setContentPane(pane);
		this.setSize(this.getPreferredSize());
		this.centerOnScreen();
	}

	function setVisible( value )
	{
		local wasVisible = this.isVisible();
		this.GUI.Frame.setVisible(value);

		if (value && !wasVisible)
		{
			this.mBrowser.refresh();
		}
	}

	function onTweakPressed( button )
	{
		local row = this.mBrowser.getSelection();

		if (row == null)
		{
			return;
		}

		local ct = this.Screens.show("CreatureTweakScreen");
		ct.setCurrentType(row.id);
	}

	function onCreateSpawn( button )
	{
		local c = this.mBrowser.getSelection();

		if (c)
		{
			if (c.type == "Creature")
			{
				this._Connection.sendQuery("spawn.create", this, [
					"CREATURE",
					c.id
				]);
			}
			else
			{
				local av_pos = ::_avatar.getPosition();
				local str_pos = av_pos.x.tostring() + " " + av_pos.y.tostring() + " " + av_pos.z.tostring();
				this._Connection.sendQuery("spawn.create", this, [
					"PACKAGE",
					c.name,
					str_pos
				]);
			}
		}
	}

	function onQueryComplete( qa, results )
	{
		this.setVisible(false);
	}

}

