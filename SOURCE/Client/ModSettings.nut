require("InputCommands");
require("UI/UI");
require("UI/Screens");
require("UI/LoadScreen");
require("UI/SocialWindow");
require("UI/ChatManager");

require("UI/BugReport");

class Screens.ModSettings extends GUI.Frame
{
	static mClassName = "Screens.ModSettings";

	mMainWindowContainer = null;

	// CHAT BOX SIZE
	mChatBoxWidth = null;
	mChatBoxHeight = null;
	mChatBoxFontSize = null;
	mButtonApplyChat = null;
	mButtonDefaultChat = null;

	//CHAT BOX BACKGROUND COLOR
	mChatBoxColorA = null;
	mButtonApplyColor = null;
	mButtonClearColor = null;
	mColorPickerBackground = null;

	//CHAT BOX FOREGROUND COLOR OPTIONS
	mButtonChatChannelColor = null;

	mDefaultsLoaded = false;

	mCheckBoxUseSound = null;
	mInputAreaSoundFile = null;
	mButtonTestSound = null;
	mButtonStopSound = null;
	mButtonDefaultSound = null;
	mButtonApplySound = null;

	mTabPane = null;

	//Other Options
	mCheckboxSupercritDisabled = null;
	mButtonApplySettings = null;
	mButtonDungeonScale = null;

	constructor()
	{
		GUI.Frame.constructor("Mod Settings");

		local chatSection = GUI.Container(GUI.BoxLayoutV());
		chatSection.add(_buildChatSettings());
		chatSection.add(GUI.Spacer(10, 24));
		chatSection.add(GUI.Label("Sound effect to play on incoming chat messages:"));
		chatSection.add(_buildChatSoundSelection());
		chatSection.add(_buildChatSoundApplication());

		local otherSection = GUI.Container(GUI.BoxLayoutV());
		otherSection.add(_buildOtherOptionsRow1());

		mTabPane = GUI.TabbedPane();
		mTabPane.setSize(425, 300);
		mTabPane.setPreferredSize(425, 300);
		mTabPane.setTabPlacement("top");
		mTabPane.setTabFontColor( "E4E4E4" );
		mTabPane.addTab("Chat Panel", chatSection);
		mTabPane.addTab("Other Options", otherSection);
		mTabPane.addActionListener( this );

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.setInsets(5);
		cmain.getLayoutManager().setGap(10);
		cmain.add(mTabPane);

		setContentPane(cmain);
		setSize(425, 300);
		centerOnScreen();
	}

		/*
		GUI.Frame.constructor("Chat Configuration");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		//local cmain = GUI.Container(GUI.BoxLayout());

		//cmain.add(GUI.Spacer(25, 25));
		//cmain.add(_buildChatSize());
		//cmain.add(GUI.Spacer(40, 25));
		//cmain.add(_buildChatBackgroundColor());

		cmain.add(_buildPanelSettings());
		cmain.add(GUI.Spacer(10, 24));
		cmain.add(GUI.Label("Sound effect to play on incoming chat messages:"));
		cmain.add(_buildSoundSelection());
		cmain.add(_buildSoundApplication());

		setContentPane(cmain);
		setSize(400, 260);
		SetControlsFromPref();
		*/
	function _createButton(name)
	{
		local button = GUI.Button(name);
		button.addActionListener(this);
		button.setReleaseMessage("onButtonPressed");
		return button;
	}
	function _buildChatSettings()
	{
		local container = GUI.Container(GUI.BoxLayout());
		container.add(GUI.Spacer(25, 25));
		container.add(_buildChatSize(), GUI.BorderLayout.EAST);
		container.add(GUI.Spacer(25, 25));
		container.add(_buildChatBackgroundColor(), GUI.BorderLayout.WEST);
		return container;
	}

	function _buildChatSoundSelection()
	{
		mInputAreaSoundFile = ::GUI.InputArea(::_ModPackage.mSettingChatSoundFile);
		mInputAreaSoundFile.setSize(250, 15);

		mButtonTestSound = _createButton("Test");
		mButtonStopSound = _createButton("Stop");
		mButtonDefaultSound = _createButton("Default");

		local container = GUI.Container();
		container.setSize(380, 18);
		container.add(mInputAreaSoundFile);
		container.add(mButtonTestSound);
		container.add(mButtonStopSound);
		container.add(mButtonDefaultSound);
		return container;
	}
	function _buildChatSoundApplication()
	{
		mCheckBoxUseSound = ::GUI.CheckBox();
		mCheckBoxUseSound.setSize(20, 20);
		mCheckBoxUseSound.setFixedSize(20, 20);
		mCheckBoxUseSound.setChecked(::_ModPackage.mSettingChatSoundEnabled);

		mButtonApplySound = _createButton("Apply Sound Changes");

		local container = GUI.Container();
		container.setSize(380, 18);
		container.add(GUI.Label("Use sound effect:"))
		container.add(mCheckBoxUseSound);
		container.add(GUI.Spacer(30, 18));
		container.add(mButtonApplySound);
		return container;
	}
	function _buildOtherOptionsRow1()
	{
		mCheckboxSupercritDisabled = ::GUI.CheckBox();
		mCheckboxSupercritDisabled.setChecked(::_ModPackage.mSettingSupercritDisabled);
		mCheckboxSupercritDisabled.addActionListener(this);

		mButtonDungeonScale = _createButton("Dungeon Scaling Options...");

		local row = GUI.Container();
		row.add(GUI.Label("Disable supercrit screen shake:"));
		row.add(mCheckboxSupercritDisabled);

		local container = GUI.Container(GUI.BoxLayoutV());
		container.add(row);
		container.add(mButtonDungeonScale);
		return container;
	}
	function onActionPerformed(component, ... )
	{
		if(component == mCheckboxSupercritDisabled)
		{
			local status = mCheckboxSupercritDisabled.getChecked();
			::_ModPackage.UpdateSupercritDisabled(status);
		}
	}
	function SetControlsFromPref()
	{
		if(mDefaultsLoaded == true)
			return;

		mDefaultsLoaded = true;

		local values = null;
		values = ::_ModPackage.GetPref("chatboxsize");
		if(values)
		{
			if(values.len() == 3)
			{
				mChatBoxWidth.setText(values[0].tostring());
				mChatBoxHeight.setText(values[1].tostring());
				mChatBoxFontSize.setText(values[2].tostring());
			}
		}

		local colors = ::_ChatWindow.mIdleBackgroundColor;
		if(colors)
		{
			local colorVal = Color(colors.r, colors.g, colors.b, 1.0);
			local a = colors.a * 100;
			a = ::Math.clamp(a.tointeger(), 0, 100);
			mColorPickerBackground.setColor(colorVal.toHexString());
			mChatBoxColorA.setText(a.tostring());
		}
	}
	function _buildChatSize()
	{
		mChatBoxWidth = GUI.InputArea();
		mChatBoxWidth.setAllowOnlyNumbers(true);
		mChatBoxWidth.setSize(70, 15);
		mChatBoxWidth.addActionListener(this);

		mChatBoxHeight = GUI.InputArea();
		mChatBoxHeight.setAllowOnlyNumbers(true);
		mChatBoxHeight.setSize(70, 15);
		mChatBoxHeight.addActionListener(this);

		mChatBoxFontSize = GUI.InputArea();
		mChatBoxFontSize.setAllowOnlyNumbers(true);
		mChatBoxFontSize.setSize(70, 15);
		mChatBoxFontSize.addActionListener(this);

		mButtonApplyChat = _createButton("Apply");
		mButtonDefaultChat = _createButton("Default");

		mChatBoxWidth.setText("400");
		mChatBoxHeight.setText("200");
		mChatBoxFontSize.setText("18");

		local grid = GUI.Container(GUI.GridLayout(4, 2));
		grid.add(GUI.Label("Width:"));
		grid.add(mChatBoxWidth);

		grid.add(GUI.Label("Height:"));
		grid.add(mChatBoxHeight);

		grid.add(GUI.Label("Font Size:"));
		grid.add(mChatBoxFontSize);

		grid.add(mButtonApplyChat);
		grid.add(mButtonDefaultChat);

		local container = GUI.Container(GUI.BoxLayoutV());
		container.add(GUI.Label("Chat Box Size"));
		container.add(grid);
		return container;
	}
	function _buildChatBackgroundColor()
	{
		mChatBoxColorA = GUI.InputArea();
		mChatBoxColorA.setAllowOnlyNumbers(true);
		mChatBoxColorA.setSize(70, 15);
		mChatBoxColorA.addActionListener(this);

		mButtonApplyColor = _createButton("Apply");
		mButtonClearColor = _createButton("Clear");

		mButtonChatChannelColor = _createButton("Text Colors...");

		mChatBoxColorA.setText("0");

		mColorPickerBackground = GUI.ColorPicker( "Background Color", "Background Color", ColorRestrictionType.DEVELOPMENT);
		mColorPickerBackground.setColor("FFFFFF");

		local grid = GUI.Container(GUI.GridLayout(4, 2));
		grid.getLayoutManager().setRows(22, 22, 22, 22);
		grid.getLayoutManager().setColumns(90, 90);

		grid.add(GUI.Label("Opacity (0-100%)"));
		grid.add(mChatBoxColorA);

		grid.add(GUI.Label("Color:"));
		grid.add(mColorPickerBackground);

		grid.add(mButtonApplyColor);
		grid.add(mButtonClearColor);

		grid.add(GUI.Spacer(0, 0));
		grid.add(mButtonChatChannelColor);

		local container = GUI.Container(GUI.BoxLayoutV());
		container.add(GUI.Label("Chat Box Color"));
		container.add(grid);
		return container;
	}

	function onButtonPressed(button)
	{
		if(button == mButtonApplyChat)
			ApplyChat();
		else if(button == mButtonDefaultChat)
			SetDefaultChat();
		else if(button == mButtonApplyColor)
			ApplyColor();
		else if(button == mButtonClearColor)
			ClearColor();
		else if(button == mButtonTestSound)
			TestSound();
		else if(button == mButtonStopSound)
			StopSound();
		else if(button == mButtonApplySound)
			ApplySound();
		else if(button == mButtonDefaultSound)
			DefaultSound();
		else if(button == mButtonChatChannelColor)
			Screens.toggle("ChatColor");
		else if(button == mButtonDungeonScale)
			Screens.toggle("DngScale");
	}
	function onInputComplete(button)
	{
		if(button == mChatBoxWidth)
			ApplyChat();
		else if(button == mChatBoxHeight)
			ApplyChat();
		else if(button == mChatBoxFontSize)
			ApplyChat();
		else if(button == mChatBoxColorA)
			ApplyColor();
	}
	function ApplyChat()
	{
		local width = Math.clamp(mChatBoxWidth.getValue(), 400, ::_ModPackage.ChatBoxGetMaxWidth());
		local height = Math.clamp(mChatBoxHeight.getValue(), 200, ::_ModPackage.ChatBoxGetMaxHeight());
		local size = Math.clamp(mChatBoxFontSize.getValue(), 12, 32);

		mChatBoxWidth.setText(width.tostring());
		mChatBoxHeight.setText(height.tostring());
		mChatBoxFontSize.setText(size.tostring());

		local values = [width, height, size];
		::_ModPackage.SetPref("chatboxsize", values);
		::_ModPackage.SetChatBoxSize(values);
		::Pref.set("chatwindow.windowSize", "Custom");
	}
	function SetDefaultChat()
	{
		mChatBoxWidth.setText("400");
		mChatBoxHeight.setText("200");
		mChatBoxFontSize.setText("18");

		local scr = Screens.get("ChatWindow", false);
		if(scr)
			scr.handleWindowResized("Small");
	}
	function ApplyColor()
	{
		local a = Math.clamp(mChatBoxColorA.getValue(), 0, 100);
		mChatBoxColorA.setText(a.tostring());

		local r, g, b;

		local hexColor = mColorPickerBackground.getCurrent();
		local colorVal = ::atoi(hexColor, 16);
		r = (colorVal & 0xFF0000) >> 16;
		g = (colorVal & 0x00FF00) >> 8;
		b = colorVal & 0x0000FF;
		r = Util.limitSignificantDigits(r.tofloat() / 255.0, 2);
		g = Util.limitSignificantDigits(g.tofloat() / 255.0, 2);
		b = Util.limitSignificantDigits(b.tofloat() / 255.0, 2);
		a = Util.limitSignificantDigits(a.tofloat() / 100.0, 2);

		local prefData = {r = r, g = g, b = b, a = a};
		::Pref.set("chatwindow.color", prefData, true, false);
		::_ChatWindow.unserializeColor(prefData);
	}
	function ClearColor()
	{
		mColorPickerBackground.setColor("000000");
		mChatBoxColorA.setText("0");
		ApplyColor();
	}

	function GetSoundFile()
	{
		return ::Util.trim(mInputAreaSoundFile.getText());
	}
	function TestSound()
	{
		::_avatar.playSound(GetSoundFile());
	}
	function StopSound()
	{
		::_avatar.stopSounds();
	}
	function ApplySound()
	{
		local status = mCheckBoxUseSound.getChecked();
		local sound = GetSoundFile();
		::_ModPackage.UpdateChatSound(status, sound);
		IGIS.info("Sound settings applied.");
	}
	function DefaultSound()
	{
		mInputAreaSoundFile.setText("Sound-MapClose.ogg");
	}
}

function InputCommands::chatMod(args)
{
	Screens.toggle("ModSettings");
}

class ModPackage
{
	mModPref = {};
	mPrefLoaded = false;
	mFirstLoadScreen = false;

	//Individual preference values for various mod functions that are needed
	//for common runtime checks.
	mSettingChatSoundEnabled = false;   //If true, play a sound effect on incoming chat messages.
	mSettingChatSoundFile = "Sound-MapClose.ogg";         //The sound file to play.
	mSettingSupercritDisabled = false;   //If true, do not allow supercrits to shake the screen.

	constructor()
	{
		mModPref = {};
		mPrefLoaded = false;
		mFirstLoadScreen = false;
	}
	function LoadPref()
	{
		if(mPrefLoaded == true)
			return 0;

		mPrefLoaded = true;
		try
		{
			mModPref = unserialize(_cache.getCookie("ModPref"));
			if(mModPref == null)
				mModPref = {};
			ApplyPrefs();
		}
		catch(e)
		{
			mModPref = {};
		}
		return 1;
	}
	function SavePref()
	{
		_cache.setCookie("ModPref", serialize(mModPref));
	}
	function SetPref(key, value)
	{
		if(mModPref == null)
			return;

		mModPref[key] <- value;
		SavePref();
	}
	function GetPref(key)
	{
		if(key in mModPref)
			return mModPref[key];
		return null;
	}
	function FinishedLoading()
	{
		if(mFirstLoadScreen == true)
			return;
		mFirstLoadScreen = true;
		::_eventScheduler.fireIn(1.0, this, "ApplyPrefs");
		::_eventScheduler.fireIn(2.0, this, "AssignKeyBinding");
		::_eventScheduler.fireIn(2.0, this, "ApplyLatePrefs");
	}

	function ApplyPrefs()
	{
		local pref = ::Pref.get("chatwindow.windowSize");
		if(pref)
		{
			if(pref == "Custom")
				SetChatBoxSize(GetPref("chatboxsize"));
		}
		pref = ::_ModPackage.GetPref("ChatSoundEnabled");
		if(pref) mSettingChatSoundEnabled = ConvertPreference("bool", pref);

		pref = ::_ModPackage.GetPref("ChatSoundFile");
		if(pref) mSettingChatSoundFile = ConvertPreference("string", pref);

		pref = ::_ModPackage.GetPref("SupercritDisabled");
		if(pref) mSettingSupercritDisabled = ConvertPreference("bool", pref);
	}
	function ConvertPreference(expected, pref)
	{
		local type = typeof pref;
		if(expected == "string" && type != expected)
			return "";
		else if(expected == "bool" && type != expected)
			return false;
		return pref;
	}

	function ApplyLatePrefs()
	{
		local fullPref = _ModPackage.GetPref("body_customize");
		if(!fullPref)
			return;

		local cdefid = ::_avatar.getType();
		if(cdefid in fullPref)
		{
			if("tail" in fullPref[cdefid])
				::InputCommands.TailSize(fullPref[cdefid].tail);
			if("ear" in fullPref[cdefid])
				::InputCommands.EarSize(fullPref[cdefid].ear);
		}
	}
	function SetChatBoxSize(params)
	{
		if(!params)
			return;

		if(params.len() < 3)
			return;

		local width = Math.clamp(params[0], 400, ChatBoxGetMaxWidth());
		local height = Math.clamp(params[1], 200, ChatBoxGetMaxHeight());
		local size = Math.clamp(params[2], 12, 32);

   		::_ChatWindow.setSize(width, height);
		::_ChatWindow.setChatFontSize(size);

   		local x = 5;
		local y = - ::_ChatWindow.mHeight - 60;
		::_ChatWindow.setPosition( x, y );
		foreach( i, x in ::_ChatWindow.mTabContents )
		{
			::_ChatWindow._updateDisplayedMessages( i );
		}
	}
	function ChatBoxGetMaxWidth()
	{
		return ::Screen.getWidth() - 10;
	}
	function ChatBoxGetMaxHeight()
	{
		return ::Screen.getHeight() - 60;
	}
	function AssignKeyBinding()
	{
		::KeyBindingDef[c_KB( Key.VK_W )]          <- "";
		::KeyBindingDef[  KB( Key.VK_F4 )]         <- "";

		::KeyBindingDef[c_KB( Key.VK_F3 )]         <- "/toggleBuilding";
		::KeyBindingDef[c_KB( Key.VK_F4 )]         <- "/sceneryBrowser";
		::KeyBindingDef[c_KB( Key.VK_F5 )]         <- "/GT";
		::KeyBindingDef[c_KB( Key.VK_F6 )]         <- "/PS";
		::KeyBindingDef[c_KB( Key.VK_F7 )]         <- "/PG";
		::KeyBindingDef[c_KB( Key.VK_F8 )]         <- "/EATS";
		::KeyBindingDef[c_KB( Key.VK_F9 )]         <- "/IGF";
		::KeyBindingDef[c_KB( Key.VK_F10 )]         <- "/previewItem";
	}
	function UpdateChatSound(enabled, file)
	{
		mSettingChatSoundEnabled = enabled;
		mSettingChatSoundFile = file;

		SetPref("ChatSoundEnabled", mSettingChatSoundEnabled);
		SetPref("ChatSoundFile", mSettingChatSoundFile);
	}
	function UpdateSupercritDisabled(status)
	{
		mSettingSupercritDisabled = status;
		SetPref("SupercritDisabled", mSettingSupercritDisabled);
	}
}

_ModPackage <- ModPackage();

function LoadScreenManager :: onQueryCompleteHack(qa,results)
{
	if(qa.query == "client.loading" && qa.args.len() > 0)
	{
		if(qa.args[0] == false)
		{
			_ModPackage.FinishedLoading();
		}
	}
	onQueryCompleteOld(qa, results);
}
LoadScreenManager["onQueryCompleteOld"] <- LoadScreenManager["onQueryComplete"];
LoadScreenManager["onQueryComplete"] <- LoadScreenManager["onQueryCompleteHack"];

function Screens::SocialWindow::onMenuItemPressed(menu, menuID)
{
		local entry = getSelectedFriendEntry( );
		if ( entry == null )
			return;

		switch ( menuID )
		{
			case "IM":
				::_ChatWindow.onStartChatInput();
				::_ChatWindow.addStringInput( "/tell \"" + entry.name + "\" ");
				break;

			case "SHARD":
				if ( entry.shard != "" )
					::_Connection.sendQuery( "shard.set", this, [entry.shard, entry.name] );
				break;
		}
}

function Screens::BugReport::onSubmitPressed( button )
{
	local category = "CHAR";

	if(mCategory && (mCategory.getCurrentIndex() != -1) )
	{
		category = CategoryType[mCategory.getCurrentIndex()].prefix;
	}
	local summ = Util.trim( mSummary.getText( ) );
	local desc = Util.trim( mDescription.getText( ) );

	if ( summ.len() < 5 )
	{
		local mb = ::GUI.MessageBox.show( "The summary is too short. Please add a more detailed summary." );
		mb.setOverlay( "GUI/BugReportOverlay" );

		return;
	}

	if ( desc.len() < 15 )
	{
		local mb = ::GUI.MessageBox.show( "The description is too short. Please add a more detailed description." );
		mb.setOverlay( "GUI/BugReportOverlay" );

		return;
	}

	CreateJiraTask( "PLAIN_AUTH", category, summ, desc );
	setVisible( false );
}

function Screens::BugReport::CreateJiraTask( auth, category, summ, desc )
{
	local req = XMLHttpRequest();
	req.onreadystatechange = function ()
	{
		if( readyState == 4 )
		{
			if( status == 200 )
			{
				local mb = ::GUI.MessageBox.show( "Thank you! Your bug report has been submitted." );
				mb.setOverlay( "GUI/BugReportOverlay" );
			}
			else
			{
	 			local mb = ::GUI.MessageBox.show( "Cannot connect to the server. Please try again later." );
	 			mb.setOverlay( "GUI/BugReportOverlay" );
			}
  		}
 	};

	local text = "Client Version: " + ((Util.isDevMode() == false) ? gVersion : "Dev") + "\n" +
				 "Username: " + ::_username + "\n" +
				 "Creature ID: " + ((::_avatar != null) ? ::_avatar.getID() : "(no creature ID)") + "\n" +
				 "CreatureDef ID: " + ((::_avatar != null) ? ::_avatar.getCreatureDef().getID() : "(no creature def)") + "\n" +
				 "Persona Name: " + ((::_avatar != null) ? ::_avatar.getName() : "(no name)") + "\n" +
				 "Position: " + ((::_avatar != null) ? ::_avatar.getPosition().x + ", " + ::_avatar.getPosition().y + ", " + ::_avatar.getPosition().z : "(no position)") + "\n" +
				 "Zone Def: " + ::_sceneObjectManager.getCurrentZoneDefID().tostring() + "\n" +
				 "Zone ID: " + ::_sceneObjectManager.getCurrentZoneID().tostring() + "\n\n" + desc;

	local txt = "<?xml version=\"1.0\"?>\n" +
				"<methodCall>\n" +
				"	<methodName>jira1.createIssue</methodName>\n" +
				"	<params>\n" +
				"		<param>\n" +
				"			<value><string>" + auth + "</string></value>\n" +
				"		</param>\n" +
				"		<param>\n" +
				"			<value>\n" +
				"				<struct>\n" +
				"					<member>\n" +
				"						<name>project</name>\n" +
				"						<value><string>BUG</string></value>\n" +
				"					</member>\n" +
				"					<member>\n" +
				"						<name>type</name>\n" +
				"						<value><int>1</int></value>\n" +
				"					</member>\n" +
				"					<member>\n" +
				"						<name>summary</name>\n" +
				"						<value><string> [" + category + "]BUG: " + summ + "</string></value>\n" +
				"					</member>\n" +
				"					<member>\n" +
				"						<name>assignee</name>\n" +
				"						<value><string>admin</string></value>\n" +
				"					</member>\n" +
				"					<member>\n" +
				"						<name>priority</name>\n" +
				"						<value><string>3</string></value>\n" +
				"					</member>\n" +
				"					<member>\n" +
				"						<name>description</name>\n" +
				"						<value><string>" + text + "</string></value>\n" +
				"					</member>\n" +
				"				</struct>\n" +
				"			</value>\n" +
				"		</param>\n" +
				"	</params>\n" +
				"</methodCall>";

	req.setRequestHeader( "Content-Type", "text/xml" );

	req.open("POST", "http://localhost/bugreport");
	req.send( txt );
}

