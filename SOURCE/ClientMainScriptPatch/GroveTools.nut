require("InputCommands");
require("UI/UI");
require("UI/Screens");

class ModQueryHandler extends DefaultQueryHandler
{
	function onQueryComplete(qa, rows)
	{
	}
	function onQueryError(qa, error)
	{
		IGIS.error(error);
	}
}

ValidATSList <- [
"ATS-Bremen",
"ATS-Bremen_Low",
"ATS-Broccan",
"ATS-Burned",
"ATS-Camelot",
"ATS-Camelot_Low",
"ATS-Carrionite_Nest",
"ATS-Castle",
"ATS-Castle_Camelot",
"ATS-Castle_Underground",
"ATS-Cave_Cold",
"ATS-Cave_Dirt",
"ATS-Cave_Dvergar",
"ATS-Cave_Ice",
"ATS-Cave_Lava",
"ATS-Cave_Moss",
"ATS-Cave_Rock",
"ATS-Cave_Skrill",
"ATS-Cave_Strata",
"ATS-Cave_Subway",
"ATS-Cave_Warren",
"ATS-Church",
"ATS-DotC_Cottage",
"ATS-DotC_Tents",
"ATS-Dungeon",
"ATS-Dungeon_Castle",
"ATS-Dungeon_Crypt",
"ATS-Dungeon_Fire",
"ATS-Dungeon_Ice",
"ATS-Dungeon_Mystarch",
"ATS-Dungeon_Spanish",
"ATS-Dungeon_Temple",
"ATS-Dungeon_Vampire",
"ATS-Evil_Castle",
"ATS-Garden",
"ATS-Greek",
"ATS-Nordic",
"ATS-Primitive_Mud",
"ATS-Primitive_Rock",
"ATS-Roman",
"ATS-Roman_Low",
"ATS-Rough_Cottage",
"ATS-Ruins",
"ATS-Spanish",
"ATS-Stonelion_Tents",
"ATS-Stone_Cottage",
"ATS-Taurian",
"ATS-Undead",
"ATS-Urban",
"ATS-Warpig",
"ATS-Wooden",
"ATS-Wooden_Cottage"
];

ValidLightList <- ["Light-Intense", "Light-Huge", "Light-Large",
"Light-Medium", "Light-Small", "Light-Tiny" ];

ValidSoundList <- ["Sound-Ambient-Bastion.ogg",
"Sound-Ambient-Camelot.ogg",
"Sound-Ambient-Chatterloop.ogg",
"Sound-Ambient-Citycrowds.ogg",
"Sound-Ambient-Citymarket.ogg",
"Sound-Ambient-Citytavern.ogg",
"Sound-Ambient-Citywind.ogg",
"Sound-Ambient-Citywindgust.ogg",
"Sound-Ambient-Cloudyday.ogg",
"Sound-Ambient-Cloudyday2.ogg",
"Sound-Ambient-Corsica.ogg",
"Sound-Ambient-Crowcalls.ogg",
"Sound-Ambient-Desertloop.ogg",
"Sound-Ambient-Desertnightloop.ogg",
"Sound-Ambient-Distantcrowds.ogg",
"Sound-Ambient-Earthrise.ogg",
"Sound-Ambient-Firesynth.ogg",
"Sound-Ambient-Firewind.ogg",
"Sound-Ambient-Forestday.ogg",
"Sound-Ambient-Forestmorning.ogg",
"Sound-Ambient-Indoor_house.ogg",
"Sound-Ambient-Marketloop.ogg",
"Sound-Ambient-Newbwind.ogg",
"Sound-Ambient-Ocean.ogg",
"Sound-Ambient-Oceanloop.ogg",
"Sound-Ambient-Rotted.ogg",
"Sound-Ambient-Spookydrone.ogg",
"Sound-Ambient-Spookywoods.ogg",
"Sound-Ambient-Swampdayloop.ogg",
"Sound-Ambient-Swampnightloop.ogg",
"Sound-Ambient-Taverncrowds.ogg",
"Sound-Ambient-Townnight.ogg",
"Sound-Ambient-Vile_Grove.ogg",
"Sound-Ambient-Waterstream1.ogg",
"Sound-Ambient-Wave1.ogg",
"Sound-Ambient-Wave2.ogg",
"Sound-Ambient-Wave3.ogg",
"Sound-Ambient-Wave4.ogg",
"Sound-Ambient-Windloop1.ogg",
"Sound-Dungeon-Ambient1.ogg",
"Sound-Dungeon-Bigwind.ogg",
"Sound-Dungeon-Heartbeat.ogg",
"Alchemybubbles.ogg",
"Brick_Oven.ogg",
"Cauldron.ogg",
"crackling_fire_loop.ogg",
"fountain.ogg",
"Mushroom1.ogg",
"Mushroom2.ogg",
"Mushroom3.ogg",
"Waterfall.ogg",
"windy1.ogg"
"Sound-Ability-Creepingdeath_Warmup.ogg",
"Sound-Ability-Energy2_Warmup.ogg",
"Sound-Ability-Energy3_Warmup.ogg",
"Sound-Ability-Energy4_Warmup.ogg",
"Sound-Ability-Firespells_Warmup.ogg",
"Sound-Ability-Free_Warmup.ogg",
"Sound-Ability-Frostspells_Warmup.ogg",
"Sound-Ability-Gaiasembrace_Warmup.ogg",
"Sound-Ability-Haste_Warmup.ogg",
"Sound-Ability-Lifeleech_Warmup.ogg",
"Sound-Ability-Malediction_Warmup.ogg",
"Sound-Ability-Morass_Warmup.ogg",
"Sound-Ability-MysticProtection_Warmup.ogg",
"Sound-Ability-Nefritarisaura_Warmup.ogg",
"Sound-Ability-Portalbind_Warmup.ogg",
"Sound-Ability-Purge_Warmup.ogg",
"Sound-Ability-Resurrect_Warmup.ogg",
"Sound-Ability-Soulneedles_Warmup.ogg",
"Sound-Ability-Swarm_Warmup.ogg" ];

class Screens.PropGenerator extends GUI.Frame
{
	static mClassName = "Screens.PropGenerator";

	mButtonClearEditBox = null;
	/*
		mButtonClearEditBox = GUI.Button("Clear Asset Name");
		mButtonClearEditBox.addActionListener(this);
		mButtonClearEditBox.setReleaseMessage("onButtonPressed");
		container.add(mButtonClearEditBox);
		*/

	mDropDownLight = null;
	mColorPickerLight = null;
	mButtonCreateLight = null;

	mInputAreaSeededDensity = null;
	mInputAreaSeededSeed = null;
	mInputAreaSeededSize = null;
	mButtonCreateSeeded = null;
	mDropDownSeeded = null;

	mInputAreaSoundGain = null;
	mDropDownSound = null;
	mButtonCreateSound = null;

	mDropDownEnvironment = null;
	mButtonCreateEnvironment = null;

	mButtonSpawnPoint = null;
	mButtonPathNode = null;

	constructor()
	{
		GUI.Frame.constructor("Prop Generator");
		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(_buildLightCreator());
		cmain.add(GUI.Spacer(0, 8));
		cmain.add(_buildSeededCreator());
		cmain.add(GUI.Spacer(0, 8));
		cmain.add(_buildSoundCreator());
		cmain.add(GUI.Spacer(0, 8));
		cmain.add(_buildEnvironmentCreator());
		cmain.add(GUI.Spacer(0, 4));
		cmain.add(_buildExtraButtons());
		setContentPane(cmain);
		setSize(550, 328);
	}
	function _buildLightCreator()
	{
		local container = GUI.Container();
		mDropDownLight = GUI.DropDownList();
		foreach(i, d in ValidLightList)
		{
			mDropDownLight.addChoice(d);
		}

		mColorPickerLight = GUI.ColorPicker( "Light Color", "Light Color", ColorRestrictionType.DEVELOPMENT);
		mColorPickerLight.setColor("FFFFFF");

		mButtonCreateLight = _createButton("Generate Light Asset");

		container.add(mDropDownLight);
		container.add(mColorPickerLight);
		container.add(mButtonCreateLight);

		return container;
	}
	function _buildSeededCreator()
	{
		local container = GUI.Container();
		local grid = GUI.Container(GUI.GridLayout(4,2));
		grid.getLayoutManager().setColumns(140, 180);
		grid.getLayoutManager().setRows(20, 20, 20, 20);

		mDropDownSeeded = GUI.DropDownList();
		foreach(i, d in ::Vegetation)
		{
			mDropDownSeeded.addChoice(i);
		}
		mDropDownSeeded.getChoices().sort();

		mInputAreaSeededDensity = ::GUI.InputArea("100");
		mInputAreaSeededDensity.setTooltip("The density of the randomized props within this area.<br>Takes a percentage value between 1 and 100%");
		mInputAreaSeededSeed = ::GUI.InputArea("1");
		mInputAreaSeededSeed.setTooltip("The randomizer seed.  Use any arbitrary number.  Each value<br>produces a unique randomized appearance.");
		mInputAreaSeededSize = ::GUI.InputArea("200");
		mInputAreaSeededSize.setTooltip("The area unit size.  Will cover a square region.<br>10 units = 1 meter");
		mInputAreaSeededDensity.setAllowOnlyNumbers(true);
		mInputAreaSeededSeed.setAllowOnlyNumbers(true);
		mInputAreaSeededSize.setAllowOnlyNumbers(true);

		grid.add(GUI.Label("Template"));
		grid.add(mDropDownSeeded);

		grid.add(GUI.Label("Density (1 to 100%)"));
		grid.add(mInputAreaSeededDensity);

		grid.add(GUI.Label("Seed (randomizer)"));
		grid.add(mInputAreaSeededSeed);

		grid.add(GUI.Label("Size (square unit coverage)"));
		grid.add(mInputAreaSeededSize);

		mButtonCreateSeeded = _createButton("Generate Seeded Asset");

		container.add(grid);
		container.add(mButtonCreateSeeded);

		return container;
	}

	function _buildSoundCreator()
	{
		local container = GUI.Container();

		mDropDownSound = GUI.DropDownList();
		foreach(i, d in ValidSoundList)
		{
			mDropDownSound.addChoice(d);
		}

		mInputAreaSoundGain = ::GUI.InputArea("100");
		mInputAreaSoundGain.setSize(40, 15);
		mInputAreaSoundGain.setAllowOnlyNumbers(true);
		//mInputAreaSoundGain.setTooltip("The volume level for this sound.<br>Takes a percentage value between 1 and 100%");

		mButtonCreateSound = _createButton("Generate Sound Asset");

		container.add(mDropDownSound);
		container.add(GUI.Label("Volume (1-100%):"));
		container.add(mInputAreaSoundGain);
		container.add(mButtonCreateSound);

		return container;
	}
	function _buildEnvironmentCreator()
	{
		mDropDownEnvironment = GUI.DropDownList();
		foreach(i, d in ::Environments)
		{
			mDropDownEnvironment.addChoice(i);
		}
		mDropDownEnvironment.getChoices().sort();

		mButtonCreateEnvironment = _createButton("Generate Environment Marker");
		mButtonCreateEnvironment.setTooltip("Environment markers are domes that change the environment(skybox,<nr> ambient sounds) when the players enter them.  The environment will<br>return to normal when the player leaves.");

		local container = GUI.Container();
		container.add(mDropDownEnvironment);
		container.add(mButtonCreateEnvironment);
		return container;
	}
	function _buildExtraButtons()
	{
		mButtonSpawnPoint = _createButton("SpawnPoint");
		mButtonSpawnPoint.setTooltip("SpawnPoints allow you to place NPCs.  See the spawn tutorial for<br>more information.");

		mButtonPathNode = _createButton("PathNode");
		mButtonPathNode.setTooltip("PathNodes allow you to create patrol points for NPCs.  Select two<br>props (a SpawnPoint or another PathNode) and press P to create a link.<br>Press U to unlink.  See the spawn tutorial for more information.");

		local container = GUI.Container();
		container.add(mButtonSpawnPoint);
		container.add(mButtonPathNode);
		container.add(GUI.Spacer(60, 0));
		return container;
	}

	function _createButton(name)
	{
		local button = GUI.Button(name);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		return button;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonClearEditBox)
			ClearEditBox();
		else if(button == mButtonCreateLight)
			CreateLight();
		else if(button == mButtonCreateSeeded)
			CreateSeeded();
		else if(button == mButtonCreateSound)
			CreateSound();
		else if(button == mButtonCreateEnvironment)
			CreateEnvironment();
		else if(button == mButtonSpawnPoint)
			SetCurrentAsset("SpawnPoint");
		else if(button == mButtonPathNode)
			SetCurrentAsset("PathNode");
	}
	function ClearEditBox()
	{
		local screen = Screens.get("BuildScreen", false);
		if(!screen)
		{
			::IGIS.info("The build screen must be open (use /toggleBuilding)");
			return;
		}
		screen.setInsertAsset("");
	}
	function CreateLight()
	{
		local asset = mDropDownLight.getCurrent() + "?Group=1&COLOR=" + mColorPickerLight.getCurrent();
		SetCurrentAsset(asset);
	}
	function CreateSeeded()
	{
		local asset = mDropDownSeeded.getCurrent();
		local d = mInputAreaSeededDensity.getValue();
		local seed = mInputAreaSeededSeed.getValue();
		local sz = mInputAreaSeededSize.getValue();
		d = ::Math.clamp(d, 1, 100);
		seed = ::Math.clamp(seed, 0, 2147483648);
		sz = ::Math.clamp(sz, 1, 1920);
		mInputAreaSeededDensity.setText(d.tostring());
		mInputAreaSeededSeed.setText(seed.tostring());
		mInputAreaSeededSize.setText(sz.tostring());
		d = d.tofloat() / 100.0;
		local assetStr = asset + "?d=" + d + "&s=" + seed + "&sz=" + sz;
		SetCurrentAsset(assetStr);
	}
	function CreateSound()
	{
		local gain = mInputAreaSoundGain.getValue();
		gain = ::Math.clamp(gain, 1, 100);
		mInputAreaSeededDensity.setText(gain.tostring());
		local asset = mDropDownSound.getCurrent();

		gain = gain.tofloat() / 100.0;
		local assetStr = "Sound?SOUND=" + asset + "&GAIN=" + gain;
		SetCurrentAsset(assetStr);
	}
	function CreateEnvironment()
	{
		local asset = "Environment-Marker?type=" + mDropDownEnvironment.getCurrent();
		SetCurrentAsset(asset);
	}
	function SetCurrentAsset(assetStr)
	{
		local screen = Screens.get("BuildScreen", false);
		if(screen)
			screen.setInsertAsset(assetStr);
		else
			IGIS.info("The build tool must be open.");
	}
	function onQueryComplete(qa, results)
	{
	}
	function onQueryError(qa, error)
	{
	}
}

class Screens.EasyATS extends GUI.Frame
{
	static mClassName = "Screens.EasyATS";
	mDropDownATS = null;
	mButtonApplyATSSelected = null;
	mButtonApplyATSEditBox = null;

	constructor()
	{
		GUI.Frame.constructor("Easy ATS");
		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(_buildAutoATS());
		cmain.add(_buildLabel());
		setContentPane(cmain);
		setSize(430, 60);
	}
	function _buildAutoATS()
	{
		local container = GUI.Container();

		mDropDownATS = GUI.DropDownList();
		foreach(i, d in ValidATSList)
		{
			mDropDownATS.addChoice(d);
		}

		mButtonApplyATSEditBox = GUI.Button("Update Asset Name");
		mButtonApplyATSEditBox.addActionListener(this);
		mButtonApplyATSEditBox.setReleaseMessage("onButtonPressed");

		mButtonApplyATSSelected = GUI.Button("Apply to Selected Props");
		mButtonApplyATSSelected.addActionListener(this);
		mButtonApplyATSSelected.setReleaseMessage("onButtonPressed");

		container.add(mDropDownATS);
		container.add(mButtonApplyATSEditBox);
		container.add(mButtonApplyATSSelected);

		return container;
	}
	function _buildLabel()
	{
		local label = GUI.Label("Note: ATS-Cave intended for Cav props.  ATS-Dungeon for Dng props.  Others for Bldg props.");
		local container = GUI.Container();
		container.setPreferredSize(430, 18);
		container.add(label);
		return container;
	}

	function onButtonPressed(button)
	{
		if(button == mButtonApplyATSSelected)
			SendApplyATS();
		else if(button == mButtonApplyATSEditBox)
			SetEditBoxATS();
	}
	function GetSelectedObjectIDs()
	{
		local objects = ::_buildTool.getSelectedObjects();
		local idList = [];
		foreach (o in objects)
		{
			if(o.isScenery())
			{
				idList.push(o.getID());
				idList.push(o.getType().tostring());
			}
		}
		return idList;
	}
	function SendApplyATS()
	{
		local idList = GetSelectedObjectIDs();
		if(idList.len() == 0)
		{
			::IGIS.info("There are no objects selected (use /SceneryBrowser)");
			return;
		}
		local output = [mDropDownATS.getCurrent()];
		foreach (i, d in idList)
		{
			output.push(d);
		}
		::_Connection.sendQuery("mod.setats", ModQueryHandler(), output);
	}

	function SetEditBoxATS()
	{
		local screen = Screens.get("BuildScreen", false);
		if(!screen)
		{
			::IGIS.info("The build screen must be open (use /toggleBuilding)");
			return;
		}
		local asset = screen.mInsertAsset.getText();
		if(asset == null)
			return;
		if(asset.len() == 0)
			return;
		local pos = asset.find("?ATS=", 0);
		if(pos != null)
			asset = asset.slice(0, pos + 5);
		else
			asset += "?ATS=";

		asset += mDropDownATS.getCurrent();
		screen.setInsertAsset(asset);

	}
}

class Screens.GroveTools extends GUI.Frame
{
	static mClassName = "Screens.GroveTools";

	mButtonPropSearch = null;
	mButtonPropGenerator = null;
	mButtonEasyATS = null;
	mButtonInstanceScript = null;

	mButtonSetStart = null;

	//Environment
	mButtonEnvironment = null;
	mDropDownEnvironment = null;
	mButtonEnvironmentCycle = null;

	//Prop stash
	mColumnListPropStash = null;
	mButtonStashAdd = null;
	mButtonStashRemove = null;
	mButtonStashRemoveAll = null;
	mButtonStashSetAsset = null;
	mStashList = [];

	constructor()
	{
		GUI.Frame.constructor("Grove Tools");
		local cmain = GUI.Container(GUI.BoxLayoutV());

		cmain.add(_buildToolRow());
		cmain.add(_buildGroveSettings());
		cmain.add(_buildGroveSettings2());
		cmain.add(_buildPropStash());
		setContentPane(cmain);
		setSize(470, 360);
	}
	function _buildToolRow()
	{
		mButtonPropSearch = _createButton("Prop Search");
		mButtonPropGenerator = _createButton("Prop Generator");
		mButtonEasyATS = _createButton("Easy ATS");
		mButtonInstanceScript = _createButton("Script");

		local container = GUI.Container();
		container.setPreferredSize(470, 24);
		container.add(mButtonPropSearch);
		container.add(mButtonPropGenerator);
		container.add(mButtonEasyATS);
		container.add(mButtonInstanceScript);
		return container;
	}
	function _buildGroveSettings()
	{
		mButtonSetStart = _createButton("Set Starting Location");
		mButtonSetStart.setTooltip("This will assign your grove's starting location to<br>wherever your character is currently standing.<br>When anyone warps to your grove, that is where<br>they will appear.");

		mDropDownEnvironment = GUI.DropDownList();
		foreach(i, d in ::Environments)
		{
			mDropDownEnvironment.addChoice(i);
		}
		mDropDownEnvironment.getChoices().sort();
		mButtonEnvironment = _createButton("Set Environment");
		mButtonEnvironment.setTooltip("The environment changes the sky template for<br>the entire grove.  Note that day/night cycling may<br>alter this environment if the effect is enabled.");

		local container = GUI.Container();
		container.setPreferredSize(470, 28);
		container.add(mButtonSetStart);
		container.add(GUI.Spacer(60, 0));
		container.add(mDropDownEnvironment);
		container.add(mButtonEnvironment);
		return container;
	}
	function _buildGroveSettings2()
	{
		mButtonEnvironmentCycle = _createButton("Toggle Day/Night Cycles");
		mButtonEnvironmentCycle.setTooltip("This will toggle usage of the day/night cycle<br>used in gameplay regions.  Note that this may<br>alter a custom environment assigned from the<br>above dropdown list.");
		mButtonEnvironmentCycle.setFixedSize(132, 18);

		local container = GUI.Container();
		container.setPreferredSize(132, 20);
		container.add(mButtonEnvironmentCycle);
		return container;
	}
	function _buildPropStash()
	{
		mColumnListPropStash = GUI.ColumnList();
		mColumnListPropStash.addColumn("Prop Stash", 200);
		mColumnListPropStash.addActionListener(this);
		mButtonStashAdd = _createButton("Add Working Asset to List");
		mButtonStashAdd.setFixedSize(140, 24);
		mButtonStashRemove = _createButton("Remove Entry");
		mButtonStashRemove.setFixedSize(140, 24);
		mButtonStashRemoveAll = _createButton("Remove All Entries");
		mButtonStashRemoveAll.setFixedSize(140, 24);
		mButtonStashSetAsset = _createButton("Set Working Asset");
		mButtonStashSetAsset.setFixedSize(140, 24);

		local left = GUI.Container(GUI.GridLayout(1,1));
		left.getLayoutManager().setColumns(250);
		left.getLayoutManager().setRows(250);
		left.add(GUI.ScrollPanel(mColumnListPropStash), GUI.BorderLayout.CENTER);

		local right = GUI.Container(GUI.BoxLayoutV());
		right.add(mButtonStashSetAsset);
		right.add(GUI.Spacer(5, 20));
		right.add(mButtonStashAdd);
		right.add(GUI.Spacer(5, 20));
		right.add(mButtonStashRemove);
		right.add(mButtonStashRemoveAll);

		local container = GUI.Container();
		container.add(left);
		container.add(right);

		LoadStashContents();
		RefreshStashList();

		return container;
	}
	function _createButton(name)
	{
		local button = GUI.Button(name);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		return button;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonPropSearch)
			OpenPropSearch();
		else if(button == mButtonPropGenerator)
			OpenPropGenerator();
		else if(button == mButtonEasyATS)
			OpenEasyATS();
		else if(button == mButtonSetStart)
			SendSetStart();
		else if(button == mButtonStashSetAsset)
			StashSetAsset();
		else if(button == mButtonStashAdd)
			StashAdd();
		else if(button == mButtonStashRemove)
			StashRemove();
		else if(button == mButtonStashRemoveAll)
			StashRemoveAll();
		else if(button == mButtonEnvironment)
			GroveSetEnvironment();
		else if(button == mButtonEnvironmentCycle)
			ToggleEnvironmentCycle();
		else if(button == mButtonInstanceScript)
			InstanceScript();
	}
	function onDoubleClick(list, evt)
	{
		if(list == mColumnListPropStash)
		{
			StashSetAsset();
		}
	}
	function OpenEasyATS()
	{
		Screens.show("EasyATS");
	}
	function OpenPropGenerator()
	{
		Screens.show("PropGenerator");
	}
	function InstanceScript()
	{
		Screens.show("InstanceScript");
	}
	function OpenPropSearch()
	{
		Screens.show("PropSearch");
	}
	function SendSetStart()
	{
		::_Connection.sendQuery("mod.setgrovestart", ModQueryHandler(), [] );
	}
	function GroveSetEnvironment()
	{
		local env = mDropDownEnvironment.getCurrent();
		::_Connection.sendQuery("mod.setenvironment", ModQueryHandler(), [env] );
	}
	function ToggleEnvironmentCycle()
	{
		::_Connection.sendQuery("mod.grove.togglecycle", ModQueryHandler(), [] );
	}
	function StashGetSelectedIndex()
	{
		local rows = mColumnListPropStash.getSelectedRows();
		if(rows.len() == 0)
			return null;
		return rows[0];
	}
	function StashSetAsset()
	{
		local index = StashGetSelectedIndex();
		if(index == null)
		{
			IGIS.info("Select an entry from the list to use.");
			return;
		}
		local asset = mColumnListPropStash.mRowContents[index][0];
		local screen = Screens.get("BuildScreen", false);
		if(screen)
			screen.setInsertAsset(asset);
	}
	function StashAdd()
	{
		local screen = Screens.get("BuildScreen", false);
		if(!screen)
			return;

		if(screen.mInsertAsset.isValidAsset() == false)
		{
			::GUI.MessageBox.show("There is no asset in the build tool, or the asset doesn't exist.");
			return;
		}

		local asset = ::Util.trim(screen.mInsertAsset.getText());
		local search = asset.tolower();
		foreach(i, d in mStashList)
		{
			if(d.tolower() == search)
			{
				IGIS.info("That asset is already in the list.");
				return;
			}
		}
		mStashList.append(asset);
		RefreshStashList();
		SaveStashContents();
	}
	function StashRemove()
	{
		local index = StashGetSelectedIndex();
		if(index == null)
		{
			IGIS.info("Select an entry from the list to remove.");
			return;
		}
		if(index < 0 || index >= mStashList.len())
			return;
		mStashList.remove(index);
		RefreshStashList();
		SaveStashContents();
	}
	function StashRemoveAll()
	{
		if(mStashList.len() == 0)
			return;
		local callback =
		{
			function onActionSelected(mb, alt)
			{
				if( alt == "Yes" )
				{
					local scr = Screens.get("GroveTools", false);
					if(scr)
						scr._StashRunRemoveAll();
				}
			}
		};
		GUI.MessageBox.showYesNo("Are you sure you want to delete the entire list?", callback);
	}
	function _StashRunRemoveAll()
	{
		mStashList.clear();
		RefreshStashList();
		SaveStashContents();
	}
	function LoadStashContents()
	{
		mStashList.clear();
		try
		{
			local data = _cache.getCookie("PropStash");
			if(data)
				mStashList = unserialize(data);
		}
		catch(e)
		{
			mStashList = [];
		}
		if(mStashList == null)
			mStashList = [];
	}
	function SaveStashContents()
	{
		try
		{
			_cache.setCookie("PropStash", serialize(mStashList));
		}
		catch(e)
		{
			IGIS.info("Failed to save Prop Stash.");
		}
	}
	function RefreshStashList()
	{
		mColumnListPropStash.removeAllRows();
		if(mStashList.len() == 0)
			return;
		foreach(i, d in mStashList)
		{
			mColumnListPropStash.addRow([d]);
		}
	}
}

class Screens.PropSearch extends GUI.Frame
{
	static mClassName = "Screens.PropSearch";

	mInputAreaSearchFilter = null;
	mColumnListResults = null;
	mButtonSearch = null;
	mButtonSetAsset = null;

	mButtonSearchProp = null;
	mButtonSearchBldg = null;
	mButtonSearchCav = null;
	mButtonSearchDng = null;
	mButtonSearchCL = null;
	mButtonSearchPar = null;

	mLabelSearchResults = null;
	//mLabelSearchPosition = null;
	mCheckBoxSubSearch = null;
	static mSearchRowsVisible = 18;

	constructor()
	{
		GUI.Frame.constructor("Prop Search");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(_buildSearchPane());
		cmain.add(GUI.Label("Browse by category. Hover over button for more info."));
		cmain.add(_buildBroadSearchPane());
		cmain.add(_buildSubSearch());
		cmain.add(_buildSearchLabelPane());
		cmain.add(_buildResultsPane());
		cmain.add(_buildActionPane());
		setContentPane(cmain);
		setSize(300, 520);
	}
	function _buildSearchPane()
	{
		local container = GUI.Container(GUI.GridLayout(1,3));
		container.getLayoutManager().setColumns(65, 160, 40);
		container.getLayoutManager().setRows(20);

		mInputAreaSearchFilter = ::GUI.InputArea( "" );
		mInputAreaSearchFilter.addActionListener(this);

		mButtonSearch = _createButton("Search");

		container.add(GUI.Label("Search Filter:"));
		container.add(mInputAreaSearchFilter);
		container.add(mButtonSearch);
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

	function _buildBroadSearchPane()
	{
		local note = "<br>May requires ATS customization for proper textures and appearances.";
		mButtonSearchProp = _createButton("Prop", "All standard Prop assets.");
		mButtonSearchBldg = _createButton("Bldg", "All Bldg components (buildings, structures)." + note);
		mButtonSearchCav = _createButton("Cav", "All Cav dungeon components (cave sections). " + note);
		mButtonSearchDng = _createButton("Dng", "All Dng dungeon components (walled interior). " + note);
		mButtonSearchCL = _createButton("CL", "All prefab prop sets. These contain multiple props<br>which are placed as one singular entity.<br>Individual props will conform to the ground.");
		mButtonSearchPar = _createButton("Par", "All particle effects which can be placed as props.");

		local container = GUI.Container();
		container.add(mButtonSearchProp);
		container.add(mButtonSearchCL);
		container.add(mButtonSearchPar);
		container.add(mButtonSearchBldg);
		container.add(mButtonSearchCav);
		container.add(mButtonSearchDng);
		container.setPreferredSize(250, 28);
		return container;
	}
	function _buildSubSearch()
	{
		local container = GUI.Container();
		mCheckBoxSubSearch = ::GUI.CheckBox();
		mCheckBoxSubSearch.setSize(20, 20);
		mCheckBoxSubSearch.setFixedSize(20, 20);
		mCheckBoxSubSearch.setChecked(true);
		container.add(GUI.Label("Use Search Filter on broad category searches:"));
		container.add(mCheckBoxSubSearch);
		container.setPreferredSize(250, 20);
		return container;
	}
	function _buildSearchLabelPane()
	{
		//local container = GUI.Container(GUI.GridLayout(1,2));
		//container.getLayoutManager().setColumns(150, 150);
		//container.getLayoutManager().setRows(20);
		local container = GUI.Container();
		mLabelSearchResults = GUI.Label("Perform a search to browse the results.");
		//mLabelSearchPosition = GUI.Label("Showing 0 of 0");
		container.add(mLabelSearchResults);
		//container.add(mLabelSearchPosition);
		return container;
	}
	function _buildResultsPane()
	{
		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(250);
		container.getLayoutManager().setRows(334);

		mColumnListResults = GUI.ColumnList();
		//mColumnListResults.setPreferredSize(250, 360);
		//mColumnListResults.setSize(250, 360);
		mColumnListResults.setWindowSize(mSearchRowsVisible);
		mColumnListResults.addColumn("Search Results", 250);
		mColumnListResults.addActionListener(this);

		container.add(GUI.ScrollPanel(mColumnListResults), GUI.BorderLayout.CENTER);
		//container.add(mColumnListResults);
		return container;
	}
	function _buildActionPane()
	{
		local container = GUI.Container();
		local label = "If you have the build panel open (/togglebuilding)<br>" +
		              "then this will update the input box with the asset<br>" +
		              "name selected in the list above. You can then use<br>" +
		              "the build panel to insert the object into the world.<br>" +
		              "Alternatively, you may double click a list entry.";

		mButtonSetAsset = _createButton("Set as Current Asset", label);
		mButtonSetAsset.addActionListener(this);
		mButtonSetAsset.setReleaseMessage("onButtonPressed");

		container.add(mButtonSetAsset);
		return container;
	}

	function onInputComplete(button)
	{
		SearchAssets(mInputAreaSearchFilter.getText());
	}
	function onDoubleClick(list, evt)
	{
		if(list == mColumnListResults)
		{
			SetAsset();
		}
	}

	/*
	function onScrollUpdated(object)
	{
		if(object == mColumnListResults)
		{
			local base = mColumnListResults.mWindowBase;
			local max = base + mColumnListResults.mWindowLen;
			local newStr = "Showing " + base + " of " + max;
			mLabelSearchPosition.setText(newStr);
		}
	}
	*/
	function onButtonPressed(button)
	{
		if(button == mButtonSearch)
			SearchAssets(mInputAreaSearchFilter.getText());
		else if(button == mButtonSetAsset)
			SetAsset();
		else if(button == mButtonSearchProp)
			SearchAssets("Prop-");
		else if(button == mButtonSearchBldg)
			SearchAssets("Bldg-");
		else if(button == mButtonSearchCav)
			SearchAssets("Cav-");
		else if(button == mButtonSearchDng)
			SearchAssets("Dng-");
		else if(button == mButtonSearchCL)
			SearchAssets("CL-");
		else if(button == mButtonSearchPar)
			SearchAssets("Par-");
	}
	function SearchAssets(filter)
	{
		local results = [];
		results = GetAssetCompletions(filter);
		if(results.len() == 0)
			SetStatus("No matching objects were found.");

		results.sort(_assetSort);

		mColumnListResults.removeAllRows();
		local doSubSearch = mCheckBoxSubSearch.getChecked();
		local subSearch = mInputAreaSearchFilter.getText().tolower();
		local added = 0;
		foreach(d in results)
		{
			local asset = d.getAsset();
			if(doSubSearch == true)
				if(asset.tolower().find(subSearch, 0) == null)
					continue;

			mColumnListResults.addRow([asset]);
			added++;
		}
		SetStatus("Search found: " + added + " objects.");
	}
	function SetStatus(text)
	{
		mLabelSearchResults.setText(text);
	}
	static function _assetSort(a, b)
	{
		local astr = a.getAsset().tostring();
		local bstr = b.getAsset().tostring();
		if( astr < bstr )
			return -1;
		if( astr > bstr )
			return 1;
		return 0;

	}
	function SetAsset()
	{
		local rows = mColumnListResults.getSelectedRows();
		if(rows.len() == 0)
			return;
		local asset = mColumnListResults.mRowContents[rows[0]][0];
		local screen = Screens.get("BuildScreen", false);
		if(screen)
			screen.setInsertAsset(asset);
	}
}

function InputCommands::GT(args)
{
	Screens.toggle("GroveTools");
}

function InputCommands::TB(args)
{
	toggleBuilding(args);
}

function InputCommands::SB(args)
{
	sceneryBrowser(args);
}

function InputCommands::EATS(args)
{
	Screens.toggle("EasyATS");
}

function InputCommands::ISCRIPT(args)
{
	Screens.toggle("InstanceScript");
}

function InputCommands::PS(args)
{
	Screens.toggle("PropSearch");
}

function InputCommands::PG(args)
{
	Screens.toggle("PropGenerator");
}
