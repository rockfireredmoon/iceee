require("InputCommands");

class Screens.EmoteBrowser extends GUI.Frame
{
	static mClassName = "Screens.EmoteBrowser";

	mDropDownListEmotes = null;
	mDropDownListTargEmotes = null;

	mButtonCopyAvatar = null;
	mButtonCopyTarget = null;
	mButtonRefreshTarget = null;

	mInputAreaSpeed = null;
	mCheckBoxLoop = null;
	mButtonDefault = null;

	mButtonAvatarEmote = null;
	mButtonAvatarStop = null;
	mButtonTargetEmote = null;
	mButtonTargetStop = null;
	mButtonScatter = null;

	//mButtonPause = null;

	constructor()
	{
		GUI.Frame.constructor("Emote Browser");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(GUI.Spacer(0, 5));
		cmain.add(_buildListPanels());
		//cmain.add(_buildEmoteList());
		//cmain.add(_buildTargEmoteList());
		cmain.add(_buildSpeedBar());
		cmain.add(_buildActionBar());
		setContentPane(cmain);
		setSize(390, 160);
	}
	function _buildListPanels()
	{
		mDropDownListEmotes = GUI.DropDownList();
		RefreshEmoteList(mDropDownListEmotes, ::_avatar);

		mDropDownListTargEmotes = GUI.DropDownList();
		//RefreshEmoteList(mDropDownListTargEmotes, ::_avatar);  //Populate default list.
		RefreshTargetEmotes();

		mButtonRefreshTarget = _createButton("Refresh");

		local label = "Pets of different creature types may have different animations.<br>" +
		"Select a pet and click Refresh to see the available emotes.<br>" +
		"If you have nothing selected, the server will attempt to find<br>" +
		"a pet that belongs to you, and search its emotes.  If it fails,<br>" +
		"it will default to your character's emotes.<br>";
		mButtonRefreshTarget.setTooltip(label);

		local grid = GUI.Container(GUI.GridLayout(2, 3));
		grid.setInsets(0);
		grid.getLayoutManager().setColumns(80, 230, 50);
		grid.getLayoutManager().setRows(24, 24);

		grid.add(GUI.Label("Avatar Emotes:"));
		grid.add(mDropDownListEmotes);
		grid.add(GUI.Spacer(1, 1));

		grid.add(GUI.Label("Target Emotes:"));
		grid.add(mDropDownListTargEmotes);
		grid.add(mButtonRefreshTarget);
		return grid;
	}
	function _buildEmoteList()
	{
		mDropDownListEmotes = GUI.DropDownList();
		RefreshEmoteList(mDropDownListEmotes, ::_avatar);

		local container = GUI.Container();
		container.setPreferredSize(430, 30);
		container.add(GUI.Label("Avatar Animations:"));
		container.add(GUI.Spacer(5, 0));
		container.add(mDropDownListEmotes);
		return container;
	}
	function _buildTargEmoteList()
	{
		mDropDownListTargEmotes = GUI.DropDownList();
		RefreshEmoteList(mDropDownListTargEmotes, ::_avatar);  //Populate default list.

		local container = GUI.Container();
		container.setPreferredSize(430, 30);
		container.add(GUI.Label("Target Animations:"));
		container.add(GUI.Spacer(5, 0));
		container.add(mDropDownListTargEmotes);
		return container;
	}
	function _buildSpeedBar()
	{
		mInputAreaSpeed = GUI.InputArea("1.0");
		mInputAreaSpeed.setSize(40, 18);
		mButtonDefault = _createButton("Default");
		mButtonDefault.setFixedSize(48, 18);

		mCheckBoxLoop = GUI.CheckBox();
		mCheckBoxLoop.setSize(20, 20);
		mCheckBoxLoop.setFixedSize(20, 20);
		mCheckBoxLoop.setChecked(false);

		local container = GUI.Container();
		container.setPreferredSize(300, 24);
		container.add(GUI.Label("Speed:"));
		container.add(mInputAreaSpeed);
		container.add(GUI.Spacer(15, 0));
		container.add(mCheckBoxLoop);
		container.add(GUI.Label("Loop"));
		container.add(GUI.Spacer(15, 0));
		container.add(mButtonDefault);
		return container;
	}
	function _buildActionBar()
	{
		mButtonAvatarEmote = _createButton("Emote");
		mButtonAvatarStop = _createButton("Stop");
		mButtonTargetEmote = _createButton("Emote");
		mButtonTargetStop = _createButton("Stop");
		mButtonCopyAvatar = _createButton("Copy");
		mButtonCopyAvatar.setTooltip("Copies the emote name in the 'Avatar' dropdown list to the clipboard.");
		mButtonCopyTarget = _createButton("Copy");
		mButtonCopyTarget.setTooltip("Copies the emote name in the 'Target' dropdown list to the clipboard.");
		mButtonScatter = _createButton("Scatter");
		mButtonScatter.setTooltip("Moves your pet to a random location around you.");


		local grid = GUI.Container(GUI.GridLayout(2, 7));
		grid.setInsets(0);
		grid.getLayoutManager().setColumns(45, 45, 35, 15, 35, 15, 40);
		grid.getLayoutManager().setRows(20, 20);

		grid.add(GUI.Label("Avatar:"));
		grid.add(mButtonAvatarEmote);
		grid.add(mButtonAvatarStop);
		grid.add(GUI.Spacer(15, 0));
		grid.add(mButtonCopyAvatar);
		grid.add(GUI.Spacer(15, 0));
		grid.add(GUI.Spacer(15, 0));

		grid.add(GUI.Label("Pet:"));
		grid.add(mButtonTargetEmote);
		grid.add(mButtonTargetStop);
		grid.add(GUI.Spacer(15, 0));
		grid.add(mButtonCopyTarget);
		grid.add(GUI.Spacer(15, 0));
		grid.add(mButtonScatter);
		return grid;

		/*
		mButtonEmote = _createButton("Emote!");
		mButtonStop = _createButton("Stop");
		mButtonPause = _createButton("Pause");

		local container = GUI.Container();
		container.setPreferredSize(380, 24);
		container.add(GUI.Spacer(15, 0));
		container.add(mButtonEmote);
		container.add(mButtonStop);
		container.add(mButtonPause);
		return container;
		*/
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
		if(button == mButtonCopyAvatar)
		{
			local emote = mDropDownListEmotes.getCurrent();
			::System.setClipboard(emote);
		}
		else if(button == mButtonCopyTarget)
		{
			local emote = mDropDownListTargEmotes.getCurrent();
			::System.setClipboard(emote);
		}
		else if(button == mButtonAvatarEmote)
			StartEmote(0);
		else if(button == mButtonTargetEmote)
			StartEmote(1);
		else if(button == mButtonAvatarStop)
			StopEmote(0);
		else if(button == mButtonTargetStop)
			StopEmote(1);
		else if(button == mButtonDefault)
		{
			mInputAreaSpeed.setText("1.0");
			mCheckBoxLoop.setChecked(false);
		}
		else if(button == mButtonRefreshTarget)
			RefreshTargetEmotes();
		else if(button == mButtonScatter)
			::_Connection.sendQuery("skscatter", this, [] );
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.getpet")
		{
			local id = results[0][0].tointeger();
			local creature = null;
			if(id != 0)
				creature = ::_sceneObjectManager.getCreatureByID(id);
			if(creature == null)
				creature = ::_avatar;
			RefreshEmoteList(mDropDownListTargEmotes, creature);
		}
	}
	function onQueryError(qa, error)
	{
		IGIS.error(error);
	}
	function RefreshEmoteList(listControl, creatureObject)
	{
		if(creatureObject == null)
			return;

		//Code slightly modified from the creaturetweak function "_updateAnimationList"
		listControl.removeAll();
		local entity = creatureObject.getEntity();
		if(entity)
		{
			foreach ( i in entity.getAnimationStates( ) )
			{
				local name;
				name = i;
				foreach ( p in [ "_b", "_h", "_t" ] )
				{
					// Format the animation state
					local pos = i.find( p );
					if ( pos && (pos == (i.len( )-2)) )
					{
						name = i.slice( 0, pos );
						break;
					}
				}

				// Is the choice already in the list?
				local found = false;
				foreach ( n in listControl.mChoices )
				{
					if ( n == name )
					{
						found = true;
						break;
					}
				}

				// Add entry to list
				if ( found == false )
					listControl.addChoice( name );
			}
		}
		if(listControl.mChoices.len() == 0)
			listControl.addChoice("-No Animations Found-");
	}
	function GetTargetID()
	{
		local targ = ::_avatar.getTargetObject();
		if(targ)
			return targ.getID();
		return 1;
	}
	function StartEmote(target)
	{
		//target is zero for avatar, one for target
		local emote = "";
		if(target == 0)
		{
			target = 0;
			emote = mDropDownListEmotes.getCurrent();
		}
		else
		{
			target = GetTargetID();
			emote = mDropDownListTargEmotes.getCurrent();
		}

		local speed = 1.0;
		local spdText = mInputAreaSpeed.getText();
		local loop = mCheckBoxLoop.getChecked() ? 1 : 0;
		try
		{
			speed = spdText.tofloat();
		}
		catch(e)
		{
			speed = 1.0;
		}
		::_Connection.sendQuery("mod.emote", this, [target, emote, speed, loop] );
	}
	function StopEmote(target)
	{
		//target is zero for avatar, one for target
		if(target == 1)
			target = GetTargetID();
		::_Connection.sendQuery("mod.emotecontrol", this, [target, 1] );
	}
	function RefreshTargetEmotes()
	{
		local obj = ::_avatar.getTargetObject();
		//If nothing is selected, see if we can fetch a pet from the server.
		//Build the list based off the query result.
		if(obj == null)
			::_Connection.sendQuery("mod.getpet", this, [] );
		else
			RefreshEmoteList(mDropDownListTargEmotes, obj);
	}
}

function InputCommands::emoteList(args)
{
	Screens.toggle("EmoteBrowser");
}

function InputCommands::pose(args)
{
	if(args.len() == 0)
		Screens.toggle("EmoteBrowser");
	else
		::_Connection.sendQuery("pose", NullQueryHandler(), args );
}
