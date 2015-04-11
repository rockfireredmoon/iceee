require("InputCommands");

class Screens.DngScale extends GUI.Frame
{
	static mClassName = "Screens.DngScale";

	mButtonClear = null;
	mButtonSelect = null;
	mColumnListResults = null;
	mHTMLDesc = null;
	mProfileList = [];

	constructor()
	{
		GUI.Frame.constructor("Dungeon Scaling");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(GUI.Spacer(0, 15));
		cmain.add(GUI.Label("Double click a profile to see its description."));
		cmain.add(_buildProfileList());
		cmain.add(GUI.Label("Description:"));
		cmain.add(_buildDescription());
		cmain.add(GUI.Spacer(0, 8));
		cmain.add(_buildButtonBar());
		cmain.add(GUI.Label("You may also use the chat command /dngscale <profile>"));
		cmain.add(GUI.Label("Or use the command with no profile to turn scaling off."));
		setContentPane(cmain);
		setSize(300, 480);

		FetchProfiles();
	}
	function _buildButtonBar()
	{
		local container = GUI.Container();
		mButtonClear = _createButton("Turn Off Scaling", "Instances you create will no longer be scaled.");
		mButtonSelect = _createButton("Choose Selected Profile", "New instances that belong to you (solo or as<br>party leader) will be scaled.");
		container.add(mButtonClear);
		container.add(mButtonSelect);
		return container;
	}

	function _createButton(name, ...)
	{
		//Optional second parameter = tooltip
		local button = GUI.Button(name);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		if(vargc > 0)
			button.setTooltip(vargv[0].tostring());
		return button;
	}

	function _buildProfileList()
	{
		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(250);
		container.getLayoutManager().setRows(180);

		mColumnListResults = GUI.ColumnList();
		mColumnListResults.addColumn("Scaling Profiles", 250);
		mColumnListResults.addActionListener(this);

		container.add(GUI.ScrollPanel(mColumnListResults));
		return container;
	}
	function _buildDescription()
	{
		mHTMLDesc = GUI.HTML();
		mHTMLDesc.setMaximumSize(218, 120);
		mHTMLDesc.setResize( true );
		mHTMLDesc.setText("");
		mHTMLDesc.addActionListener(this);
		mHTMLDesc.setInsets(8, 8, 8, 8);

		local width = mHTMLDesc.getSize().width - 32;
		local font = mHTMLDesc.getFont();
		mHTMLDesc.setWrapText(true, font, width);


		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(250);
		container.getLayoutManager().setRows(120);
		container.setAppearance("GoldBorder");
		container.add(mHTMLDesc);

		container.add(GUI.ScrollPanel(mHTMLDesc));
		return container;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonClear)
		{
			::_Connection.sendQuery("dngscale", this, [] );
		}
		else if(button == mButtonSelect)
		{
			local prof = GetSelectedProfile();
			if(!prof)
			{
				IGIS.info("You have not selected a profile.");
				return;
			}
			::_Connection.sendQuery("dngscale", this, [prof.name] );
		}
	}
	function FetchProfiles()
	{
		::_Connection.sendQuery("mod.getdungeonprofiles", this, [] );
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.getdungeonprofiles")
		{
			mProfileList.clear();
			foreach(i, r in results)
			{
				local name = r[0];
				local desc = r[1];
				mProfileList.append({name = name, desc = desc});
			}

			mColumnListResults.removeAllRows();
			foreach(i, d in mProfileList)
			{
				mColumnListResults.addRow([d.name]);
			}
		}
	}
	function onDoubleClick(list, evt)
	{
		if(list == mColumnListResults)
		{
			local prof = GetSelectedProfile();
			if(prof)
			{
				mHTMLDesc.setText(prof.desc);
			}
			else
			{
				mHTMLDesc.setText("No profile selected.");
			}
		}
	}

	function GetSelectedProfile()
	{
		local rows = mColumnListResults.getSelectedRows();
		if(rows.len() == 0)
			return null;
		local index = rows[0].tointeger();
		if(index in mProfileList)
		{
			return mProfileList[index];
		}
		return null;
	}
}

function InputCommands::Dng(args)
{
	Screens.toggle("DngScale");
}







