require("UI/UI");
require("UI/ChatManager");
require("Preferences");
require("UI/Screens");
class Screens.ChatWindow extends GUI.Component
{
	mIdleBackgroundColor = Color(0.0, 0.0, 0.0, 0.0);
	mStoredIdleBackgroundColor = null;
	mIdleTabBackgroundColor = Color(0.0, 0.0, 0.0, 0.0);
	mActiveBackgroundColor = Color(0.0, 0.0, 0.0, 0.30000001);
	mInputArea = null;
	mInputComponent = null;
	mTypeChannelLabel = null;
	mTabPane = null;
	mTabContents = null;
	mRenamePopup = null;
	mLastClickedTab = -1;
	mFilterPopup = null;
	constructor()
	{
		GUI.Component.constructor(GUI.BorderLayout());
		mTabContents = {};
		_buildGUI();
		local tabList = mTabPane.getAllTabs();
		mTabPane.mTabsPane.getLayoutManager().setOffset(25, 6);

		foreach( tab in tabList )
		{
			tab.button.setBlendColor(mIdleTabBackgroundColor);
			tab.button.setFontColor(Color(0.0, 0.0, 0.0, 0.0));
		}

		Screen.setOverlayVisible("GUI/ChatWindowOverlay", true);
		setOverlay("GUI/ChatWindowOverlay");
		setPassThru(true);
		::IGIS.addListener(this);
		::_Connection.addListener(this);
		::_ChatWindow = this;
		::Pref.get("chatwindow.color");
		::Pref.get("chatwindow.chattabs");
		setCached(::Pref.get("video.UICache"));
	}

	function addChatTab( name, permanent )
	{
		local tab = mTabPane._findTab(name);

		if (tab != null)
		{
			log.debug("addChatTab could not create another tab named " + tab.name);
			return null;
		}

		mTabContents[name] <- ChatTab(permanent);
		mTabContents[name].setName(name);
		local windowSize = getSize();
		local mp = GUI.Panel();
		mTabContents[name].setMainWindow(mp);
		mp.setPreferredSize(windowSize.width, windowSize.height);
		mp.setLayoutManager(GUI.StackLayout());
		mp.setInsets(3, 3, 3, 3);
		mp.setAppearance("ChatWindow");
		mp.setBlendColor(mIdleBackgroundColor);
		local s = GUI.ScrollPanel(mp, GUI.BorderLayout.WEST);
		mTabContents[name].setScrollBar(s);
		s.setAppearance("Container");
		s.addActionListener(this);
		s.setIndent(10);
		mTabPane.addTab(name, s);
		local tabList = mTabPane.getAllTabs();
		local newTab = mTabPane.mTabs[tabList.len() - 1];
		newTab.button.setBlendColor(mActiveBackgroundColor);
		::_ChatManager.addChatListener(name, this);
		return newTab;
	}

	function addMessage( channel, message, tab, ... )
	{
		tab = mTabPane._findTab(tab);

		if (!tab)
		{
			return;
		}

		local tell = false;
		local speakerName = "";
		tab = tab.name;
		local scope = ::_ChatManager.getScope(channel);

		if (vargc > 0)
		{
			speakerName = vargv[0];

			if (scope == "Tell")
			{
				tell = true;

				if (speakerName != ::_avatar.getName() && mTabPane._findTab(speakerName) == null)
				{
					addChatTab(speakerName, false);
					addMessage(channel, message, speakerName, speakerName);
				}
			}
		}

		if (!tell || speakerName != tab)
		{
			if (!(scope in mTabContents[tab].getFilterList()))
			{
				return;
			}
		}

		local oldLen = mTabContents[tab].getLog().len();
		local index = mTabContents[tab].getScrollBar().getIndex();
		mTabContents[tab].getLog().append({
			speakerName = speakerName,
			message = message,
			channel = channel
		});

		if (index == oldLen - 1)
		{
			index = mTabContents[tab].getLog().len() - 1;
		}

		if (mTabContents[tab].getLog().len() > 500)
		{
			mTabContents[tab].getLog().remove(0);

			if (index != mTabContents[tab].getLog().len())
			{
				index--;
			}
		}

		if (index <= 0)
		{
			index = 1;
		}

		mTabContents[tab].getScrollBar().setIndex(index);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "util.group.show")
		{
			local myResults = results[0];
			addMessage("sys/", "Groups: " + myResults[0], "General");
		}
		else if (qa.query == "util.group.set")
		{
			addMessage("sys/", "Group permissions set", "General");
		}
		else
		{
			throw Exception("Unknown query: " + qa.query);
		}
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "util.group.set")
		{
			addMessage("err/", error, "General");
		}
		else if (qa.query == "util.group.show")
		{
			addMessage("err/", error, "General");
		}
		else
		{
			log.error("Query Error in Chat Window " + error);
		}
	}

	function addStringInput( pText )
	{
		mInputArea.insertText(pText);
	}

	function destroy()
	{
		::_Connection.removeListener(this);
		::IGIS.removeListener(this);

		foreach( k, n in mTabContents )
		{
			::_ChatManager.removeChatListener(k);
		}

		Screen.setOverlayVisible("GUI/ChatWindowOverlay", false);
		::_ChatWindow = null;
		GUI.Component.destroy();
	}

	function getChatFont()
	{
		return mTabPane.mContentPane.getFont();
	}

	function getChatFontSize()
	{
		return getChatFont().getHeight();
	}

	mInResize = false;
	function handleWindowResized( sizeSelected )
	{
		if (mInResize == true)
		{
			return;
		}

		mInResize = true;
		local ChatWindowSizes = {
			Small = {
				owner = "ChatWindowSize.Small",
				caption = "Small",
				callback = "onChangeWindowSizeSmall",
				width = 400,
				height = 200,
				fontSize = 18
			},
			Medium = {
				owner = "ChatWindowSize.Medium",
				caption = "Medium",
				callback = "onChangeWindowSizeMedium",
				width = 500,
				height = 300,
				fontSize = 22
			},
			Large = {
				owner = "ChatWindowSize.Large",
				caption = "Large",
				callback = "onChangeWindowSizeLarge",
				width = 600,
				height = 400,
				fontSize = 24
			}
		};

		if (sizeSelected in ChatWindowSizes)
		{
			local chatSizeData = ChatWindowSizes[sizeSelected];
			setSize(chatSizeData.width, chatSizeData.height);
			local x = 5;
			local y = -mHeight - 60;
			setPosition(x, y);
			setChatFontSize(chatSizeData.fontSize);

			foreach( i, x in mTabContents )
			{
				_updateDisplayedMessages(i);
			}

			local optionScreen = ::Screens.get("VideoOptionsScreen", true);

			if (optionScreen)
			{
				optionScreen.setChatWindowDropdownOption(sizeSelected);
			}

			::Pref.set("chatwindow.windowSize", sizeSelected);
		}

		mInResize = false;
	}

	function onCreateNewTab( menu )
	{
		for( local i = 1; i < 100; i++ )
		{
			if (mTabPane._findTab("New Tab " + i.tostring()) == null)
			{
				local chatTabName = "New Tab " + i.tostring();
				addChatTab(chatTabName, true);

				if (mTabContents[mLastClickedTab.name].isPermanent())
				{
					local filterList = mTabContents[mLastClickedTab.name].getFilterList();

					foreach( filter, value in filterList )
					{
						mTabContents[chatTabName].addFilter(filter);
					}
				}
				else
				{
					mTabContents[chatTabName].addFilter("Say");
				}

				return;
			}
		}

		serializeChatTabs();
	}

	function onChangeFilters( menu )
	{
		mFilterPopup = GUI.ChatFilterSelect();
		GUI.MessageBox.showEx(mFilterPopup, [
			"Accept",
			"Decline"
		], this, "onFilterSelected");
		local filterList = mTabContents[mLastClickedTab.name].getFilterList();

		foreach( channel, value in filterList )
		{
			mFilterPopup.setCategoryChecked(channel);
		}
	}

	function onChangeOpacity( menu )
	{
		local opacityChooser = GUI.OpacityChooser();
		opacityChooser.addActionListener(this);
		opacityChooser.setOpacityValue((mStoredIdleBackgroundColor.a.tofloat() * 100).tointeger());
		GUI.MessageBox.showEx(opacityChooser, [
			"Accept",
			"Decline"
		], this, "onOpacitySelected");
	}

	function onChangeWindowSizeLarge( menu )
	{
		handleWindowResized("Large");
	}

	function onChangeWindowSizeMedium( menu )
	{
		handleWindowResized("Medium");
	}

	function onChangeWindowSizeSmall( menu )
	{
		handleWindowResized("Small");
	}

	function onFilterSelected( messageBox, alt )
	{
		switch(alt)
		{
		case "Accept":
			mTabContents[mLastClickedTab.name].removeAllFilters();
			local filterList = mFilterPopup.getChatFilters();

			foreach( filter in filterList )
			{
				if (filter.isChecked())
				{
					mTabContents[mLastClickedTab.name].addFilter(filter.getChatChannelType());
				}
			}

			serializeChatTabs();
			break;

		default:
			if (alt == "Decline")
			{
			}
		}

		mFilterPopup = null;
	}

	function onLinkClicked( message, data )
	{
		if("href" in data && Util.startsWith(data.href, "http://") || Util.startsWith(data.href, "https://")) {
			System.openURL(data.href);
		}
		else if("href" in data && Util.startsWith(data.href, "forum://")) {
			local forumId = data.href.slice(8);
			local igf = Screens.show("IGForum");
			if(igf) {
				igf.QueryOpenThread(forumId.tointeger(), 0);
			}
		}  
		else if ("info" in data)
		{
			switch(data.info)
			{
			case "speakerName":
				startChatInputOnChannel("t/" + "\"" + data.clickedOnText + "\"");
				break;

			case "chatChannel":
				local channel = Util.replace(data.clickedOnText, "[", "");
				channel = Util.replace(channel, "]", "");

				if (Util.startsWith(channel, "chan "))
				{
					local channelName = channel.slice(5, channel.len());
					startChatInputOnChannel("ch/" + channelName);
				}
				else
				{
					switch(channel)
					{
					case "Region":
						startChatInputOnChannel("rc/" + ::_Connection.getCurrentRegionChannel());
						break;
					case "Trade":
						startChatInputOnChannel("tc/" + ::_Connection.getCurrentRegionChannel());
						break;
					case "Earthsage":
						startChatInputOnChannel("gm/earthsages");
						break;
					case "Party":
						startChatInputOnChannel("party");
						break;
					default:
						break;
					}
				}
			}
		}
	}

	function onOpacitySelected( messageBox, alt )
	{
		if (alt == "Accept")
		{
			mStoredIdleBackgroundColor = Color(mIdleBackgroundColor.r, mIdleBackgroundColor.g, mIdleBackgroundColor.b, mIdleBackgroundColor.a);
			serializeColor();
		}
		else if (alt == "Decline")
		{
			mIdleBackgroundColor = Color(mStoredIdleBackgroundColor.r, mStoredIdleBackgroundColor.g, mStoredIdleBackgroundColor.b, mStoredIdleBackgroundColor.a);

			foreach( i, x in mTabContents )
			{
				x.getMainWindow().setBlendColor(mIdleBackgroundColor);
			}
		}
	}

	function onOpacityUpdate( value )
	{
		mIdleBackgroundColor.a = value.tofloat() / 100.0;

		foreach( i, x in mTabContents )
		{
			x.getMainWindow().setBlendColor(mIdleBackgroundColor);
		}
	}

	function onMouseEnter( evt )
	{
		if (mIdleBackgroundColor.a <= 0.1)
		{
			foreach( i, x in mTabContents )
			{
				x.getMainWindow().setBlendColor(mActiveBackgroundColor);
			}
		}

		local tabList = mTabPane.getAllTabs();

		foreach( tab in tabList )
		{
			tab.button.setBlendColor(mActiveBackgroundColor);
			tab.button.setFontColor(Color(GUI.DefaultButtonFontColor));
		}
	}

	function onMouseExit( evt )
	{
		local chatSize = getSize();
		local chatPos = getScreenPosition();
		local mouseX = evt.x;
		local mouseY = evt.y;

		if (mouseX >= chatPos.x && mouseX <= chatPos.x + chatSize.width && mouseY >= chatPos.y && mouseY <= chatPos.y + chatSize.height)
		{
			return;
		}
		else
		{
			foreach( i, x in mTabContents )
			{
				x.getMainWindow().setBlendColor(mIdleBackgroundColor);
			}

			local tabList = mTabPane.getAllTabs();

			foreach( tab in tabList )
			{
				tab.button.setBlendColor(mIdleTabBackgroundColor);
				tab.button.setFontColor(Color(0.0, 0.0, 0.0, 0.0));
			}
		}
	}

	function onIGISTransitoryMessage( message, type )
	{
		if(type != IGIS.BROADCAST)
		{
			addMessage(type == IGIS.ERROR ? "Err" : "Info", message, "General");
		}
	}

	function onInputComplete( inputArea )
	{
		if (inputArea == mInputArea)
		{
			mInputArea.setAllowTextEntryOnClick(false);
			mInputComponent.setAppearance("Container");
			mInputComponent.setPassThru(true);
			mTypeChannelLabel.setText("");
		}
		else if (mRenamePopup == inputArea)
		{
			local newTabName = inputArea.getText();
			local tab = mTabPane._findTab(newTabName);

			if (tab != null)
			{
				IGIS.error("The tab name: " + newTabName + " already exists. ");
				return;
			}

			local activeIndex = mTabPane.getSelectedIndex();
			local oldTab = mTabContents[mLastClickedTab.name];
			::_ChatManager.removeChatListener(mLastClickedTab.name);
			mTabPane.remove(mLastClickedTab);
			delete mTabContents[mLastClickedTab.name];
			::_ChatManager.addChatListener(newTabName, this);
			mLastClickedTab.name = newTabName;
			mTabContents[mLastClickedTab.name] <- oldTab;
			mTabContents[mLastClickedTab.name].setName(newTabName);
			mTabPane.insertTab(mLastClickedTab.name, mLastClickedTab.component, mLastClickedTab.index);
			mTabPane.selectTab(activeIndex);
			mTabContents[mLastClickedTab.name].getMainWindow().setBlendColor(mIdleBackgroundColor);
			local tab = mTabPane.mTabs[mLastClickedTab.index];
			tab.button.setBlendColor(mIdleTabBackgroundColor);
			tab.button.setFontColor(Color(0.0, 0.0, 0.0, 0.0));
			inputArea.hidePopup();
			serializeChatTabs();
		}
	}

	function onScrollUpdate( sender )
	{
		local tab = mTabPane._findTab(sender).name;

		if (tab == null)
		{
			return;
		}

		local scroll = mTabContents[tab].getScrollBar();
		local index = scroll.getIndex();
		local log = mTabContents[tab].getLog();

		if (index > log.len() - 1)
		{
			scroll.setIndex(log.len() - 1);
			return;
		}
		else if (index < 0)
		{
			scroll.setIndex(0);
			return;
		}

		_updateDisplayedMessages(tab);
	}

	function onRenameTab( menu )
	{
		mRenamePopup = GUI.PopupInputBox("Tab name:");
		mRenamePopup.addActionListener(this);
		local inputBoxSize = mRenamePopup.getSize();
		local inputBoxPosX = ::Screen.getWidth() / 2 - inputBoxSize.width / 2;
		local inputBoxPosY = ::Screen.getHeight() / 2 - inputBoxSize.height / 2;
		mRenamePopup.setPosition(inputBoxPosX, inputBoxPosY);
		mRenamePopup.showInputBox();
		mRenamePopup.getInputArea().setMaxCharacters(8);
	}

	function startChatInputOnChannel( channel, ... )
	{
		local ignoredCharacters = [];

		if (vargc > 0)
		{
			ignoredCharacters.append(vargv[0]);
		}

		LastCommand = channel;

		if (Util.startsWith(channel, "t/"))
		{
			local triggerChannel = channel.slice(2);
			triggerChannel = Util.replace(triggerChannel, "\"", "");
			EchoTrigger = "yt/" + triggerChannel;
		}
		else
		{
			EchoTrigger = "";
		}

		onStartChatInput();

		foreach( ignoredCharacter in ignoredCharacters )
		{
			mInputArea.addIgnoredCharacter(ignoredCharacter);
		}
	}

	function onStartChatInput()
	{
		mInputArea.setAllowTextEntryOnClick(true);
		mInputComponent.setAppearance("TextInputFields");
		mInputArea.onStartInput();
		mInputComponent.setPassThru(false);
		mTypeChannelLabel.setText(_parseChannelStr(LastCommand));
		local LEFT_SPACE = 2;

		if (mTypeChannelLabel.getText().len() > 0)
		{
			local font = mTypeChannelLabel.getFont();
			local fontMetrics = font.getTextMetrics(mTypeChannelLabel.getText());
			mInputComponent.getLayoutManager().setColumns(fontMetrics.width - LEFT_SPACE, "*");
		}
		else
		{
			mInputComponent.getLayoutManager().setColumns(-LEFT_SPACE, "*");
		}
	}

	function onTabDelete( menu )
	{
		removeChatTab(mLastClickedTab);
		serializeChatTabs();
	}

	function onTabRightClicked( tab )
	{
		mLastClickedTab = tab;
		local menu = GUI.PopupMenu();
		menu.addActionListener(this);
		menu.addMenuOption("Create New Tab", "Create New Tab", "onCreateNewTab");

		if (mTabPane.findArrayIndexOfTab(mLastClickedTab) == 0)
		{
			menu.addMenuOption("Delete Tab", "Delete Tab", "onTabDelete", true, false);
		}
		else
		{
			menu.addMenuOption("Delete Tab", "Delete Tab", "onTabDelete");
		}

		if (mTabContents[mLastClickedTab.name].isPermanent())
		{
			menu.addMenuOption("Rename Tab", "Rename Tab", "onRenameTab");
		}
		else
		{
			menu.addMenuOption("Rename Tab", "Rename Tab", "onRenameTab", true, false);
		}

		menu.addMenuOption("Change Idle Opacity", "Change Idle Opacity", "onChangeOpacity");
		menu.addMenuOption("ChatWindowSize", "Resize Chat Window");
		menu.addMenuOption("ChatWindowSize.Small", "Small", "onChangeWindowSizeSmall");
		menu.addMenuOption("ChatWindowSize.Meduim", "Medium", "onChangeWindowSizeMedium");
		menu.addMenuOption("ChatWindowSize.Large", "Large", "onChangeWindowSizeLarge");

		if (mTabContents[mLastClickedTab.name].isPermanent())
		{
			menu.addMenuOption("Filters", "Filters", "onChangeFilters");
		}
		else
		{
			menu.addMenuOption("Filters", "Filters", "onChangeFilters", true, false);
		}

		menu.showMenu();
	}

	function setChatFont( font )
	{
		mTabPane.mContentPane.setFont(font);

		if (!mTabContents || mTabContents.len() == 0)
		{
			return;
		}

		foreach( i, x in mTabContents )
		{
			if (!x.getActiveHTML() || x.getActiveHTML().len() == 0)
			{
				return;
			}

			foreach( j, y in x.getActiveHTML() )
			{
				y.setFont(font);
			}

			_updateDisplayedMessages(i);
		}
	}

	function setChatFontSize( size )
	{
		local font = getChatFont();
		setChatFont(GUI.Font(font.getFace(), size));
	}

	function removeChatTab( tab )
	{
		::_ChatManager.removeChatListener(tab.name);
		mTabPane.remove(tab);
		mTabContents[tab.name].destroy();
		delete mTabContents[tab.name];
	}

	function serializeColor()
	{
		local data = {
			r = mIdleBackgroundColor.r,
			g = mIdleBackgroundColor.g,
			b = mIdleBackgroundColor.b,
			a = mIdleBackgroundColor.a
		};
		::Pref.set("chatwindow.color", data, true, false);
	}

	function serializeChatTabs()
	{
		local data = [];

		foreach( tab in mTabContents )
		{
			if (tab.isPermanent())
			{
				data.append(tab.serialize());
			}
		}

		::Pref.set("chatwindow.chattabs", data, true, false);
	}

	function unserializeColor( data )
	{
		mIdleBackgroundColor = Color(data.r, data.g, data.b, data.a);
		mStoredIdleBackgroundColor = Color(data.r, data.g, data.b, data.a);

		foreach( i, x in mTabContents )
		{
			x.getMainWindow().setBlendColor(mIdleBackgroundColor);
		}
	}

	function unserializeChatTabs( data )
	{
		foreach( tab in data )
		{
			addChatTab(tab.name, true);
			local chatTab = mTabContents[tab.name];

			foreach( filter, value in tab.filters )
			{
				chatTab.addFilter(filter);
			}
		}
	}

	function updateBoldness( value )
	{
		if (value)
		{
			foreach( i, tab in mTabContents )
			{
				local log = tab.getLog();

				foreach( logMessage in log )
				{
					local message = logMessage.message;

					if (!(Util.startsWith(message, "<b>") && Util.endsWith(message, "</b>")))
					{
						message = "<b>" + message + "</b>";
						logMessage.message = message;
					}
				}

				_updateDisplayedMessages(i);
			}
		}
		else
		{
			foreach( i, tab in mTabContents )
			{
				local log = tab.getLog();

				foreach( logMessage in log )
				{
					local message = logMessage.message;

					if (Util.startsWith(message, "<b>") && Util.endsWith(message, "</b>"))
					{
					}

					message = message.slice(3);
					message = message.slice(0, message.len() - 4);
					logMessage.message = message;
				}

				_updateDisplayedMessages(i);
			}
		}
	}

	function _buildGUI()
	{
		setSticky("left", "bottom");
		setSize(400, 200);
		setPreferredSize(400, 200);
		local x = 5;
		local y = -mHeight - 60;
		setPosition(x, y);
		mTabPane = GUI.TabbedPane(true);
		mTabPane.addActionListener(this);
		mTabPane.setTabPlacement("top");
		mTabPane.mContentPane.setAppearance("Container");

		if (mTabPane.mTabsPane)
		{
			mTabPane.mTabsPane.setInsets(0, 5, 0, 10);
		}

		setChatFont(GUI.Font("Maiandra", 18));
		add(mTabPane, GUI.BorderLayout.CENTER);
		mInputComponent = GUI.Component();
		mInputComponent.setLayoutManager(GUI.GridLayout(1, 2));
		mInputComponent.getLayoutManager().setColumns(20, "*");
		mInputComponent.getLayoutManager().setRows("*");
		mInputComponent.setInsets(2, 0, 0, 5);
		mInputComponent.setPassThru(true);
		add(mInputComponent, GUI.BorderLayout.SOUTH);
		mTypeChannelLabel = GUI.Label("");
		mTypeChannelLabel.setFont(GUI.Font("Maiandra", 20));
		mInputComponent.add(mTypeChannelLabel);
		mInputArea = GUI.InputArea();
		mInputArea.setAllowTextEntryOnClick(false);
		mInputArea.setAppearance("Container");
		mInputArea.setFont(GUI.Font("Maiandra", 20));
		mInputArea.addActionListener(::_ChatManager);
		mInputArea.addActionListener(this);
		mInputArea.setMaxCharacters(128);
		mInputComponent.add(mInputArea);
		setChatFont(GUI.Font("MaiandraShadow", getChatFontSize()));

		foreach( i, x in mTabContents )
		{
			x.getMainWindow().setAppearance("ChatWindow");
			x.getMainWindow().setBlendColor(Color(0.0, 0.0, 0.0, 0.0));
			_updateDisplayedMessages(i);
		}
	}

	function _countMaxHTMLComponents( tab )
	{
		local first = GUI.HTML("");
		first._setHidden(true);
		local mainPanel = mTabContents[tab].getMainWindow();
		local sz = first.getMinimumSize();
		local height = sz.height + mainPanel.getLayoutManager().mGap;
		local total = 0;

		if (mainPanel.mIsRealized == false)
		{
			local mainPanelSize = mainPanel.getPreferredSize();
			local panelHeight = mainPanelSize.height - insets.top - insets.bottom;
			total = panelHeight.tointeger() / height.tointeger();
		}
		else
		{
			local panelHeight = mainPanel.getHeight() - insets.top - insets.bottom;
			total = panelHeight.tointeger() / height.tointeger();
		}

		return total;
	}

	function _parseChannelStr( channelStr )
	{
		local str = channelStr;
		local parseStr = "";

		if (Util.startsWith(str.tolower(), "*syschat"))
		{
			parseStr = "[SysChat]:";
		}
		else if (Util.startsWith(str.tolower(), "s"))
		{
			parseStr = "";
		}
		else if (Util.startsWith(str.tolower(), "t/"))
		{
			local resultsStr = ::Util.replace(str, "t/", "");

			if (Util.startsWith(resultsStr, "\""))
			{
				resultsStr = resultsStr.slice(1, resultsStr.len());
			}

			if (resultsStr.slice(resultsStr.len() - 1, resultsStr.len()) == "\"")
			{
				resultsStr = resultsStr.slice(0, resultsStr.len() - 1);
			}

			parseStr = "Tell " + resultsStr + ":";
		}
		else if (Util.startsWith(str.tolower(), "party"))
		{
			parseStr = "Party:";
		}
		else if (Util.startsWith(str.tolower(), "clan"))
		{
			parseStr = "Clan:";
		}
		else if (Util.startsWith(str.tolower(), "emote"))
		{
			parseStr = "Emote:";
		}
		else if (Util.startsWith(str.tolower(), "tc/"))
		{
			parseStr = "Trade:";
		}
		else if (Util.startsWith(str.tolower(), "rc/"))
		{
			parseStr = "Region:";
		}
		else if (Util.startsWith(str.tolower(), "gm/"))
		{
			parseStr = "Earthsage:";
		}
		else if (Util.startsWith(str.tolower(), "ch/"))
		{
			parseStr = ::Util.replace(str, "ch/", "");
		}

		return parseStr;
	}
	
	function _searchAndReplace(message, searchString, endString, callbackFunction) {
		local sidx = 0;
		while(sidx < message.len()) {
			local fidx = message.find(searchString, sidx);
			if(fidx == null)
				break;
			else {
				sidx = fidx;
				local eidx = message.find(endString, sidx + 1);
				if(eidx == null)
					eidx = message.len();
					
				// Have link start and end position now so extract it and create link HTML
				local link = message.slice(sidx, eidx == message.len() ? eidx : (eidx + 1));
				local newlink = callbackFunction(link);
				
				// Reconstruct the message
				if(sidx > 0) {
					if(eidx == message.len())
						message = message.slice(0, sidx) + newlink;
					else
						message = message.slice(0, sidx) + newlink + message.slice(eidx + 1);
				}
				else {
					if(eidx == message.len())
						message = newlink;
					else 
						message = newlink + message.slice(eidx + 1);
				}
					
				// Next link
				sidx = sidx + newlink.len() + 1;
			}
		}
		return message;
	}
	
	function _updateDisplayedMessages( tab )
	{
		local totalHTMLComponents = _countMaxHTMLComponents(tab);
		local logLen = mTabContents[tab].getLog().len();
		local index = mTabContents[tab].getScrollBar().getIndex();
		local count = 0;
		local activeHTML = mTabContents[tab].getActiveHTML();

		while (logLen > 0 && index >= 0 && count <= totalHTMLComponents)
		{
			local message = mTabContents[tab].getLog()[index].message;
			
			local channel = mTabContents[tab].getLog()[index].channel;
			local color = _ChatManager.getColor(channel);
			local wrapSize = getSize().width - 50;
			
			local bolded = Util.startsWith(message, "<b>");
			if(bolded) {
				message = message.slice(3, message.len());
				if(Util.endsWith(message, "</b>")) 
					message = message.slice(0, message.len() - 4);
			}
			
			message = Util.replace(message, "Http://", "http://");
			message = Util.replace(message, "Https://", "https://");
			message = Util.replace(message, "Forum://", "forum://");
			
			// Search for hyperlinks in the text and turn them into clickable links
			
			local linkReplace = function(link) {
				local ed = Util.endsWith(link, ".");
				if(ed) {
					link = link.slice(0, link.len() - 1); 
				}
				local es = Util.endsWith(link, " ");
				if(es) {
					link = link.slice(0, link.len() - 1);
				}
				local linkText = link;				
				if(linkText.len() > 30) {
					linkText = linkText.slice(0, 26) + "...";
				}
				link = "<a href=\"" + link + "\"><font color=\"7777ff\">" + Util.trim(linkText) + "</font></a>";
				if(ed) {
					link += ".";
				} 
				else if(es) {
					link += " ";
				}
				return link;
			}
			message = _searchAndReplace(message, "http://", " ", linkReplace);
			message = _searchAndReplace(message, "https://", " ", linkReplace);
			message = _searchAndReplace(message, "forum://", " ", linkReplace);
			local colorReplace = function(color) {
				
				if(Util.startsWith(color, "#0")) 
					return "<font color=\"ff0000\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#1")) 
					return "<font color=\"ff7f00\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#2")) 
					return "<font color=\"ffff00\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#3")) 
					return "<font color=\"00ff00\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#4")) 
					return "<font color=\"0000ff\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#5")) 
					return "<font color=\"ff00ff\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#6")) 
					return "<font color=\"ff00ff\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#7")) 
					return "<font color=\"00ffff\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#8")) 
					return "<font color=\"ffffff\">" + color.slice(2, color.len() - 1) + "</font>";
				else if(Util.startsWith(color, "#9")) 
					return "<font color=\"8a5d27\">" + color.slice(2, color.len() - 1) + "</font>";
				return color;
			}
			message = _searchAndReplace(message, "#", "#", colorReplace);
			message = _searchAndReplace(message, "{", "}", function(text) {
				return "<i>" + text.slice(1, text.len() - 1) + "</i>";
			});
			message = _searchAndReplace(message, "\\", "\\", function(text) {
				return "<b>" + text.slice(1, text.len() - 1) + "</b>";
			});
			
			if(bolded) 
				message = "<b>" + message + "</b>";
			
			if (activeHTML.len() <= count)
			{
				local html = GUI.HTML(message);
				html.setLinkStaticColor(color);
				html.setChangeColorOnHover(false);
				html.addActionListener(this);
				html.setResize(true);
				html._setHidden(true);
				html.setFontColor(color);
				html.setWrapText(true, getChatFont(), wrapSize);
				mTabContents[tab].getMainWindow().add(html);
				activeHTML.append(html);
			}
			else
			{
				activeHTML[count].setText(message);
				activeHTML[count].setSize(0, 0);
				activeHTML[count].setWrapText(true, getChatFont(), wrapSize);
				activeHTML[count].setFontColor(color);
				activeHTML[count].setLinkStaticColor(color);
			}

			count++;
			index--;
		}

		while (count < activeHTML.len())
		{
			mTabContents[tab].getMainWindow().remove(activeHTML[count]);
			activeHTML.remove(count);
		}
	}

}

class ChatTab 
{
	mScrollBar = null;
	mMainWindow = null;
	mLog = null;
	mActiveHTML = null;
	mName = "";
	mFilters = null;
	mPermanentTab = true;
	constructor( permanent )
	{
		mLog = [];
		mActiveHTML = [];
		mPermanentTab = permanent;

		if (permanent)
		{
			mFilters = {};
		}
	}

	function destroy()
	{
		mScrollBar.destroy();
		mMainWindow.destroy();
	}

	function addFilter( channelCategory )
	{
		if (mPermanentTab)
		{
			mFilters[channelCategory] <- true;
		}
	}

	function getActiveHTML()
	{
		return mActiveHTML;
	}

	function getLog()
	{
		return mLog;
	}

	function getName()
	{
		return mName;
	}

	function getMainWindow()
	{
		return mMainWindow;
	}

	function getScrollBar()
	{
		return mScrollBar;
	}

	function getFilterList()
	{
		return mFilters;
	}

	function isPermanent()
	{
		return mPermanentTab;
	}

	function removeAllFilters()
	{
		if (mPermanentTab)
		{
			mFilters = {};
		}
	}

	function setActiveHTML( html )
	{
		mActiveHTML = html;
	}

	function setFilterList( filterList )
	{
		mFilters = filterList;
	}

	function setLog( log )
	{
		mLog = log;
	}

	function setMainWindow( mainWindow )
	{
		mMainWindow = mainWindow;
	}

	function setName( name )
	{
		mName = name;
	}

	function setScrollBar( scrollBar )
	{
		mScrollBar = scrollBar;
	}

	function serialize()
	{
		if (mPermanentTab)
		{
			return {
				name = mName,
				filters = mFilters
			};
		}

		return null;
	}

}

::_ChatWindow <- null;
