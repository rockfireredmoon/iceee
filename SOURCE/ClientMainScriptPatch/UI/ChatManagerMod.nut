require("InputCommands");
require( "UI/ChatManager");

class ChatColorManager
{
	mLoaded = false;
	mModified = false;
	mChatColors = {};

	function Load()
	{
		if(mLoaded == true)
			return;

		local colors = {};
		try
		{
			colors = unserialize(_cache.getCookie("ChatColors"));
		}
		catch(e)
		{
			colors = ChannelColors;
		}

		if(colors == null)
		{
			colors = {};
		}
		else if(typeof(colors) != "table")
		{
			colors = {};
		}

		mChatColors = colors;
		MergeColors();
		MergeDefaults();

		if(mModified == true)
			Save();

		mLoaded = true;
	}
	function Save()
	{
		try
		{
			_cache.setCookie("ChatColors", serialize(mChatColors));
		}
		catch(e)
		{
		}
	}
	function GetColor(channel)
	{
		Load();
		if(channel in mChatColors)
			return mChatColors[channel];

		local p = channel.find("/");
		if(p == null)
			return null;
		channel = channel.slice(0, p + 1);
		if(channel in mChatColors)
			return mChatColors[channel];

		return null;
	}
	function MergeColors()
	{
		foreach(i, d in ChannelColors)
		{
			if(!(i in mChatColors))
			{
				mChatColors[i] <- d;
				mModified = true;
			}
		}
	}
	function MergeDefaults()
	{
		local mergeTable = {
			["t/"] = Colors["purple2"],
			["yt/"] = Colors["purple2"],
			["sys/"] = Colors["yellow"],
			["err/"] = Colors["red"],
			["ch/"] = Colors["turquoise"],
			["gm/"] = Colors["coral"],
			["tc/"] = Colors["coral"],
			["rc/"] = Colors["Medium Grey"]
		}
		foreach(i, d in mergeTable)
		{
			if(!(i in mChatColors))
			{
				mChatColors[i] <- d;
				mModified = true;
			}
		}
	}
	function GetColorTable()
	{
		return mChatColors;
	}
	function SetDefaults()
	{
		mChatColors.clear();
		MergeColors();
		MergeDefaults();
	}
	function SetColor(channel, color)
	{
		if(channel == null || color == null)
			return;
		if(typeof channel != "string")
			return;
		if(typeof color != "string")
			return;
		if(channel.len() == 0 || color.len() == 0)
			return;
		channel = ::Util.trim(channel);
		color = ::Util.trim(color);
		mChatColors[channel] <- color;
	}
}

_ChatColorManager <- ChatColorManager();

	function UI::ChatManager::getColor( channel )
	{
		local color = _ChatColorManager.GetColor(channel);
		if(color != null)
			return color;

		//Default handler copied from the original getColor() function.
		if( channel in ChannelColors )
			return ChannelColors[channel];
		else if( Util.startsWith(channel, "t/") || Util.startsWith(channel, "yt/") )
			return Colors["purple2"];
		else if( Util.startsWith(channel, "sys/") )
			return Colors["yellow"];
		else if( Util.startsWith(channel, "err/") )
			return Colors["red"];
		else if( Util.startsWith(channel, "ch/") )
			return Colors["turquoise"];
		else if( Util.startsWith(channel, "gm/") )
			return Colors["coral"];
		else if( Util.startsWith(channel, "tc/") )
			return Colors["coral"];
		else if( Util.startsWith(channel, "rc/") )
			return Colors["Medium Grey"];
		else
			return ChannelColors.Default;
	}





class Screens.ChatColor extends GUI.Frame
{
	static mClassName = "Screens.ChatColor";

	mButtonRefresh = null;
	mButtonSetDefault = null;
	mButtonApplyColor = null;
	mButtonSaveChanges = null;
	mColumnListResults = null;
	mColorPicker = null;
	mInputAreaChannel = null;
	mButtonHelp = null;

	constructor()
	{
		GUI.Frame.constructor("Chat Color Editor");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		setContentPane(cmain);
		setSize(290, 415);

		local labelCont = GUI.Container(GUI.BoxLayoutV());
		labelCont.add(GUI.Label("Double-click a channel to edit."));
		labelCont.add(GUI.Label("Use the color box, then click Apply Color."));

		mButtonHelp = GUI.NarrowButton("Help");
		mButtonHelp.addActionListener(this);
		mButtonHelp.setReleaseMessage("onButtonPressed");
		mButtonHelp.setFixedSize(48, 32);

		local topBar = GUI.Container();
		topBar.add(labelCont);
		topBar.add(mButtonHelp);
		topBar.setPreferredSize(285, 36);

		cmain.add(topBar);

		cmain.add(_buildResultList());
		cmain.add(_buildEditRow());
		cmain.add(GUI.Spacer(0, 5));
		cmain.add(_buildButtonRow());

		Refresh();
	}
	function _buildEditRow()
	{
		mButtonApplyColor = _createButton("Apply Color");

		mInputAreaChannel = GUI.InputArea("");
		mInputAreaChannel.setSize(100, 15);

		mColorPicker = GUI.ColorPicker( "Channel Color", "Channel Color", ColorRestrictionType.DEVELOPMENT);
		mColorPicker.setColor("FFFFFF");

		local container = GUI.Container();
		container.add(GUI.Label("Channel:"));
		container.add(mInputAreaChannel);
		container.add(mColorPicker);
		container.add(mButtonApplyColor);
		container.setPreferredSize(280, 24);
		return container;
	}
	function _buildButtonRow()
	{
		mButtonRefresh = _createButton("Refresh List");
		mButtonSetDefault = _createButton("Restore Defaults");
		mButtonSaveChanges = _createButton("Save Changes");

		local container = GUI.Container();
		container.add(mButtonSetDefault);
		container.add(mButtonRefresh);
		container.add(mButtonSaveChanges);
		container.setPreferredSize(280, 24);
		return container;
	}
	function _buildResultList()
	{
		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(260);
		container.getLayoutManager().setRows(280);

		mColumnListResults = GUI.ColumnList();
		mColumnListResults.addColumn("Channel", 70);
		mColumnListResults.addColumn("Color", 70);
		mColumnListResults.addActionListener(this);

		container.add(GUI.ScrollPanel(mColumnListResults));
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
	function SortChannelArray(a, b)
	{
		if(a.channel > b.channel) return 1;
		else if(a.channel < b.channel) return -1;
		return 0;
	}

	function Refresh()
	{
		local sortArr = [];
		local colors = _ChatColorManager.GetColorTable();
		foreach(i, d in colors)
		{
			sortArr.append( {channel=i, color=d} );
		}
		sortArr.sort(SortChannelArray);

		mColumnListResults.removeAllRows();
		foreach(i, d in sortArr)
		{
			mColumnListResults.addRow([d.channel, d.color]);
		}
/*
		local colors = _ChatColorManager.GetColorTable();
		mColumnListResults.removeAllRows();
		foreach(i, d in colors)
		{
			mColumnListResults.addRow([i, d]);
		}
*/
	}
	function onButtonPressed(button)
	{
		if(button == mButtonRefresh)
			Refresh();
		else if(button == mButtonApplyColor)
		{
			local text = mInputAreaChannel.getText();
			if(!text)
				return;
			if(text.len() == 0)
				return;
			_ChatColorManager.SetColor(text, mColorPicker.getCurrent());
			Refresh();
		}
		else if(button == mButtonSetDefault)
		{
			AskConfirmSetDefaults();
		}
		else if(button == mButtonSaveChanges)
		{
			_ChatColorManager.Save();
		}
		else if(button == mButtonHelp)
		{
			_URLManager.LaunchURL("Chat Channel Colors");
		}
	}
	function onDoubleClick(list, evt)
	{
		if(list == mColumnListResults)
		{
			local channel = GetSelectedChannel();
			if(channel)
			{
				local colors = _ChatColorManager.GetColorTable();
				if(channel in colors)
				{
					mInputAreaChannel.setText(channel);
					mColorPicker.setColor(colors[channel]);
				}
			}
		}
	}
	function GetSelectedChannel()
	{
		local rows = mColumnListResults.getSelectedRows();
		if(rows == null)
			return null;
		if(rows.len() == 0)
			return null;

		return mColumnListResults.mRowContents[rows[0]][0];
	}
	function AskConfirmSetDefaults()
	{
		/*
		local callback =
		{
			manager = this
			function onActionSelected(mb, alt)
			{
				if( alt == "Yes" )
				{
					_ChatColorManager.SetDefaults();
					manager.Refresh();
				}
			}
		};

		GUI.MessageBox.showYesNo("Are you sure you want to restore the color table to its default settings?", callback);
		*/

		local request = ::GUI.PopupQuestionBox("Are you sure you want to restore the color table to its default settings?");
		request.setAcceptActionName("onAcceptSetDefaults" );
		request.addActionListener( this );
		request.showInputBox();
		request.center();
	}

	function onAcceptSetDefaults( window )
	{
		window.destroy();
		_ChatColorManager.SetDefaults();
		_ChatColorManager.Save();
		Refresh();
	}
}

function InputCommands::ChatColor(args)
{
	Screens.toggle("ChatColor");
}

