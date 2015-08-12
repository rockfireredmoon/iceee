this.require("UI/UI");
this.require("UI/ChatManager");
this.require("Preferences");
this.require("UI/Screens");
class this.Screens.ChatWindow extends this.GUI.Component
{
	mIdleBackgroundColor = this.Color(0.0, 0.0, 0.0, 0.0);
	mStoredIdleBackgroundColor = null;
	mIdleTabBackgroundColor = this.Color(0.0, 0.0, 0.0, 0.0);
	mActiveBackgroundColor = this.Color(0.0, 0.0, 0.0, 0.30000001);
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
		this.GUI.Component.constructor(this.GUI.BorderLayout());
		this.mTabContents = {};
		this._buildGUI();
		local tabList = this.mTabPane.getAllTabs();
		this.mTabPane.mTabsPane.getLayoutManager().setOffset(25, 6);

		foreach( tab in tabList )
		{
			tab.button.setBlendColor(this.mIdleTabBackgroundColor);
			tab.button.setFontColor(this.Color(0.0, 0.0, 0.0, 0.0));
		}

		this.Screen.setOverlayVisible("GUI/ChatWindowOverlay", true);
		this.setOverlay("GUI/ChatWindowOverlay");
		this.setPassThru(true);
		::IGIS.addListener(this);
		::_Connection.addListener(this);
		::_ChatWindow = this;
		::Pref.get("chatwindow.color");
		::Pref.get("chatwindow.chattabs");
		this.setCached(::Pref.get("video.UICache"));
	}

	function addChatTab( name, permanent )
	{
		local tab = this.mTabPane._findTab(name);

		if (tab != null)
		{
			this.log.debug("addChatTab could not create another tab named " + tab.name);
			return null;
		}

		this.mTabContents[name] <- this.ChatTab(permanent);
		this.mTabContents[name].setName(name);
		local windowSize = this.getSize();
		local mp = this.GUI.Panel();
		this.mTabContents[name].setMainWindow(mp);
		mp.setPreferredSize(windowSize.width, windowSize.height);
		mp.setLayoutManager(this.GUI.StackLayout());
		mp.setInsets(3, 3, 3, 3);
		mp.setAppearance("ChatWindow");
		mp.setBlendColor(this.mIdleBackgroundColor);
		local s = this.GUI.ScrollPanel(mp, this.GUI.BorderLayout.WEST);
		this.mTabContents[name].setScrollBar(s);
		s.setAppearance("Container");
		s.addActionListener(this);
		s.setIndent(10);
		this.mTabPane.addTab(name, s);
		local tabList = this.mTabPane.getAllTabs();
		local newTab = this.mTabPane.mTabs[tabList.len() - 1];
		newTab.button.setBlendColor(this.mActiveBackgroundColor);
		::_ChatManager.addChatListener(name, this);
		return newTab;
	}

	function addMessage( channel, message, tab, ... )
	{
		tab = this.mTabPane._findTab(tab);

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

				if (speakerName != ::_avatar.getName() && this.mTabPane._findTab(speakerName) == null)
				{
					this.addChatTab(speakerName, false);
					this.addMessage(channel, message, speakerName, speakerName);
				}
			}
		}

		if (!tell || speakerName != tab)
		{
			if (!(scope in this.mTabContents[tab].getFilterList()))
			{
				return;
			}
		}

		local oldLen = this.mTabContents[tab].getLog().len();
		local index = this.mTabContents[tab].getScrollBar().getIndex();
		this.mTabContents[tab].getLog().append({
			speakerName = speakerName,
			message = message,
			channel = channel
		});

		if (index == oldLen - 1)
		{
			index = this.mTabContents[tab].getLog().len() - 1;
		}

		if (this.mTabContents[tab].getLog().len() > 500)
		{
			this.mTabContents[tab].getLog().remove(0);

			if (index != this.mTabContents[tab].getLog().len())
			{
				index--;
			}
		}

		if (index <= 0)
		{
			index = 1;
		}

		this.mTabContents[tab].getScrollBar().setIndex(index);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "util.group.show")
		{
			local myResults = results[0];
			this.addMessage("sys/", "Groups: " + myResults[0], "General");
		}
		else if (qa.query == "util.group.set")
		{
			this.addMessage("sys/", "Group permissions set", "General");
		}
		else
		{
			throw this.Exception("Unknown query: " + qa.query);
		}
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "util.group.set")
		{
			this.addMessage("err/", error, "General");
		}
		else if (qa.query == "util.group.show")
		{
			this.addMessage("err/", error, "General");
		}
		else
		{
			this.log.error("Query Error in Chat Window " + error);
		}
	}

	function addStringInput( pText )
	{
		this.mInputArea.insertText(pText);
	}

	function destroy()
	{
		::_Connection.removeListener(this);
		::IGIS.removeListener(this);

		foreach( k, n in this.mTabContents )
		{
			::_ChatManager.removeChatListener(k);
		}

		this.Screen.setOverlayVisible("GUI/ChatWindowOverlay", false);
		::_ChatWindow = null;
		this.GUI.Component.destroy();
	}

	function getChatFont()
	{
		return this.mTabPane.mContentPane.getFont();
	}

	function getChatFontSize()
	{
		return this.getChatFont().getHeight();
	}

	mInResize = false;
	function handleWindowResized( sizeSelected )
	{
		if (this.mInResize == true)
		{
			return;
		}

		this.mInResize = true;
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
			this.setSize(chatSizeData.width, chatSizeData.height);
			local x = 5;
			local y = -this.mHeight - 60;
			this.setPosition(x, y);
			this.setChatFontSize(chatSizeData.fontSize);

			foreach( i, x in this.mTabContents )
			{
				this._updateDisplayedMessages(i);
			}

			local optionScreen = ::Screens.get("VideoOptionsScreen", true);

			if (optionScreen)
			{
				optionScreen.setChatWindowDropdownOption(sizeSelected);
			}

			::Pref.set("chatwindow.windowSize", sizeSelected);
		}

		this.mInResize = false;
	}

	function onCreateNewTab( menu )
	{
		for( local i = 1; i < 100; i++ )
		{
			if (this.mTabPane._findTab("New Tab " + i.tostring()) == null)
			{
				local chatTabName = "New Tab " + i.tostring();
				this.addChatTab(chatTabName, true);

				if (this.mTabContents[this.mLastClickedTab.name].isPermanent())
				{
					local filterList = this.mTabContents[this.mLastClickedTab.name].getFilterList();

					foreach( filter, value in filterList )
					{
						this.mTabContents[chatTabName].addFilter(filter);
					}
				}
				else
				{
					this.mTabContents[chatTabName].addFilter("Say");
				}

				return;
			}
		}

		this.serializeChatTabs();
	}

	function onChangeFilters( menu )
	{
		this.mFilterPopup = this.GUI.ChatFilterSelect();
		this.GUI.MessageBox.showEx(this.mFilterPopup, [
			"Accept",
			"Decline"
		], this, "onFilterSelected");
		local filterList = this.mTabContents[this.mLastClickedTab.name].getFilterList();

		foreach( channel, value in filterList )
		{
			this.mFilterPopup.setCategoryChecked(channel);
		}
	}

	function onChangeOpacity( menu )
	{
		local opacityChooser = this.GUI.OpacityChooser();
		opacityChooser.addActionListener(this);
		opacityChooser.setOpacityValue((this.mStoredIdleBackgroundColor.a.tofloat() * 100).tointeger());
		this.GUI.MessageBox.showEx(opacityChooser, [
			"Accept",
			"Decline"
		], this, "onOpacitySelected");
	}

	function onChangeWindowSizeLarge( menu )
	{
		this.handleWindowResized("Large");
	}

	function onChangeWindowSizeMedium( menu )
	{
		this.handleWindowResized("Medium");
	}

	function onChangeWindowSizeSmall( menu )
	{
		this.handleWindowResized("Small");
	}

	function onFilterSelected( messageBox, alt )
	{
		switch(alt)
		{
		case "Accept":
			this.mTabContents[this.mLastClickedTab.name].removeAllFilters();
			local filterList = this.mFilterPopup.getChatFilters();

			foreach( filter in filterList )
			{
				if (filter.isChecked())
				{
					this.mTabContents[this.mLastClickedTab.name].addFilter(filter.getChatChannelType());
				}
			}

			this.serializeChatTabs();
			break;

		default:
			if (alt == "Decline")
			{
			}
		}

		this.mFilterPopup = null;
	}

	function onLinkClicked( message, data )
	{
		if("clickedOnText" in data && this.Util.startsWith(data.clickedOnText, "http://") || this.Util.startsWith(data.clickedOnText, "https://")) {
			this.System.openURL(data.clickedOnText);
		} 
		else if ("info" in data)
		{
			switch(data.info)
			{
			case "speakerName":
				this.startChatInputOnChannel("t/" + "\"" + data.clickedOnText + "\"");
				break;

			case "chatChannel":
				local channel = this.Util.replace(data.clickedOnText, "[", "");
				channel = this.Util.replace(channel, "]", "");

				if (this.Util.startsWith(channel, "chan "))
				{
					local channelName = channel.slice(5, channel.len());
					this.startChatInputOnChannel("ch/" + channelName);
				}
				else
				{
					switch(channel)
					{
					case "Region":
						this.startChatInputOnChannel("rc/" + ::_Connection.getCurrentRegionChannel());
						break;
					case "Trade":
						this.startChatInputOnChannel("tc/" + ::_Connection.getCurrentRegionChannel());
						break;
					case "Earthsage":
						this.startChatInputOnChannel("gm/earthsages");
						break;
					case "Party":
						this.startChatInputOnChannel("party");
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
			this.mStoredIdleBackgroundColor = this.Color(this.mIdleBackgroundColor.r, this.mIdleBackgroundColor.g, this.mIdleBackgroundColor.b, this.mIdleBackgroundColor.a);
			this.serializeColor();
		}
		else if (alt == "Decline")
		{
			this.mIdleBackgroundColor = this.Color(this.mStoredIdleBackgroundColor.r, this.mStoredIdleBackgroundColor.g, this.mStoredIdleBackgroundColor.b, this.mStoredIdleBackgroundColor.a);

			foreach( i, x in this.mTabContents )
			{
				x.getMainWindow().setBlendColor(this.mIdleBackgroundColor);
			}
		}
	}

	function onOpacityUpdate( value )
	{
		this.mIdleBackgroundColor.a = value.tofloat() / 100.0;

		foreach( i, x in this.mTabContents )
		{
			x.getMainWindow().setBlendColor(this.mIdleBackgroundColor);
		}
	}

	function onMouseEnter( evt )
	{
		if (this.mIdleBackgroundColor.a <= 0.1)
		{
			foreach( i, x in this.mTabContents )
			{
				x.getMainWindow().setBlendColor(this.mActiveBackgroundColor);
			}
		}

		local tabList = this.mTabPane.getAllTabs();

		foreach( tab in tabList )
		{
			tab.button.setBlendColor(this.mActiveBackgroundColor);
			tab.button.setFontColor(this.Color(this.GUI.DefaultButtonFontColor));
		}
	}

	function onMouseExit( evt )
	{
		local chatSize = this.getSize();
		local chatPos = this.getScreenPosition();
		local mouseX = evt.x;
		local mouseY = evt.y;

		if (mouseX >= chatPos.x && mouseX <= chatPos.x + chatSize.width && mouseY >= chatPos.y && mouseY <= chatPos.y + chatSize.height)
		{
			return;
		}
		else
		{
			foreach( i, x in this.mTabContents )
			{
				x.getMainWindow().setBlendColor(this.mIdleBackgroundColor);
			}

			local tabList = this.mTabPane.getAllTabs();

			foreach( tab in tabList )
			{
				tab.button.setBlendColor(this.mIdleTabBackgroundColor);
				tab.button.setFontColor(this.Color(0.0, 0.0, 0.0, 0.0));
			}
		}
	}

	function onIGISTransitoryMessage( message, type )
	{
		if(type != this.IGIS.BROADCAST)
		{
			this.addMessage(type == this.IGIS.ERROR ? "Err" : "Info", message, "General");
		}
	}

	function onInputComplete( inputArea )
	{
		if (inputArea == this.mInputArea)
		{
			this.mInputArea.setAllowTextEntryOnClick(false);
			this.mInputComponent.setAppearance("Container");
			this.mInputComponent.setPassThru(true);
			this.mTypeChannelLabel.setText("");
		}
		else if (this.mRenamePopup == inputArea)
		{
			local newTabName = inputArea.getText();
			local tab = this.mTabPane._findTab(newTabName);

			if (tab != null)
			{
				this.IGIS.error("The tab name: " + newTabName + " already exists. ");
				return;
			}

			local activeIndex = this.mTabPane.getSelectedIndex();
			local oldTab = this.mTabContents[this.mLastClickedTab.name];
			::_ChatManager.removeChatListener(this.mLastClickedTab.name);
			this.mTabPane.remove(this.mLastClickedTab);
			delete this.mTabContents[this.mLastClickedTab.name];
			::_ChatManager.addChatListener(newTabName, this);
			this.mLastClickedTab.name = newTabName;
			this.mTabContents[this.mLastClickedTab.name] <- oldTab;
			this.mTabContents[this.mLastClickedTab.name].setName(newTabName);
			this.mTabPane.insertTab(this.mLastClickedTab.name, this.mLastClickedTab.component, this.mLastClickedTab.index);
			this.mTabPane.selectTab(activeIndex);
			this.mTabContents[this.mLastClickedTab.name].getMainWindow().setBlendColor(this.mIdleBackgroundColor);
			local tab = this.mTabPane.mTabs[this.mLastClickedTab.index];
			tab.button.setBlendColor(this.mIdleTabBackgroundColor);
			tab.button.setFontColor(this.Color(0.0, 0.0, 0.0, 0.0));
			inputArea.hidePopup();
			this.serializeChatTabs();
		}
	}

	function onScrollUpdate( sender )
	{
		local tab = this.mTabPane._findTab(sender).name;

		if (tab == null)
		{
			return;
		}

		local scroll = this.mTabContents[tab].getScrollBar();
		local index = scroll.getIndex();
		local log = this.mTabContents[tab].getLog();

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

		this._updateDisplayedMessages(tab);
	}

	function onRenameTab( menu )
	{
		this.mRenamePopup = this.GUI.PopupInputBox("Tab name:");
		this.mRenamePopup.addActionListener(this);
		local inputBoxSize = this.mRenamePopup.getSize();
		local inputBoxPosX = ::Screen.getWidth() / 2 - inputBoxSize.width / 2;
		local inputBoxPosY = ::Screen.getHeight() / 2 - inputBoxSize.height / 2;
		this.mRenamePopup.setPosition(inputBoxPosX, inputBoxPosY);
		this.mRenamePopup.showInputBox();
		this.mRenamePopup.getInputArea().setMaxCharacters(8);
	}

	function startChatInputOnChannel( channel, ... )
	{
		local ignoredCharacters = [];

		if (vargc > 0)
		{
			ignoredCharacters.append(vargv[0]);
		}

		this.LastCommand = channel;

		if (this.Util.startsWith(channel, "t/"))
		{
			local triggerChannel = channel.slice(2);
			triggerChannel = this.Util.replace(triggerChannel, "\"", "");
			this.EchoTrigger = "yt/" + triggerChannel;
		}
		else
		{
			this.EchoTrigger = "";
		}

		this.onStartChatInput();

		foreach( ignoredCharacter in ignoredCharacters )
		{
			this.mInputArea.addIgnoredCharacter(ignoredCharacter);
		}
	}

	function onStartChatInput()
	{
		this.mInputArea.setAllowTextEntryOnClick(true);
		this.mInputComponent.setAppearance("TextInputFields");
		this.mInputArea.onStartInput();
		this.mInputComponent.setPassThru(false);
		this.mTypeChannelLabel.setText(this._parseChannelStr(this.LastCommand));
		local LEFT_SPACE = 2;

		if (this.mTypeChannelLabel.getText().len() > 0)
		{
			local font = this.mTypeChannelLabel.getFont();
			local fontMetrics = font.getTextMetrics(this.mTypeChannelLabel.getText());
			this.mInputComponent.getLayoutManager().setColumns(fontMetrics.width - LEFT_SPACE, "*");
		}
		else
		{
			this.mInputComponent.getLayoutManager().setColumns(-LEFT_SPACE, "*");
		}
	}

	function onTabDelete( menu )
	{
		this.removeChatTab(this.mLastClickedTab);
		this.serializeChatTabs();
	}

	function onTabRightClicked( tab )
	{
		this.mLastClickedTab = tab;
		local menu = this.GUI.PopupMenu();
		menu.addActionListener(this);
		menu.addMenuOption("Create New Tab", "Create New Tab", "onCreateNewTab");

		if (this.mTabPane.findArrayIndexOfTab(this.mLastClickedTab) == 0)
		{
			menu.addMenuOption("Delete Tab", "Delete Tab", "onTabDelete", true, false);
		}
		else
		{
			menu.addMenuOption("Delete Tab", "Delete Tab", "onTabDelete");
		}

		if (this.mTabContents[this.mLastClickedTab.name].isPermanent())
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

		if (this.mTabContents[this.mLastClickedTab.name].isPermanent())
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
		this.mTabPane.mContentPane.setFont(font);

		if (!this.mTabContents || this.mTabContents.len() == 0)
		{
			return;
		}

		foreach( i, x in this.mTabContents )
		{
			if (!x.getActiveHTML() || x.getActiveHTML().len() == 0)
			{
				return;
			}

			foreach( j, y in x.getActiveHTML() )
			{
				y.setFont(font);
			}

			this._updateDisplayedMessages(i);
		}
	}

	function setChatFontSize( size )
	{
		local font = this.getChatFont();
		this.setChatFont(this.GUI.Font(font.getFace(), size));
	}

	function removeChatTab( tab )
	{
		::_ChatManager.removeChatListener(tab.name);
		this.mTabPane.remove(tab);
		this.mTabContents[tab.name].destroy();
		delete this.mTabContents[tab.name];
	}

	function serializeColor()
	{
		local data = {
			r = this.mIdleBackgroundColor.r,
			g = this.mIdleBackgroundColor.g,
			b = this.mIdleBackgroundColor.b,
			a = this.mIdleBackgroundColor.a
		};
		::Pref.set("chatwindow.color", data, true, false);
	}

	function serializeChatTabs()
	{
		local data = [];

		foreach( tab in this.mTabContents )
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
		this.mIdleBackgroundColor = this.Color(data.r, data.g, data.b, data.a);
		this.mStoredIdleBackgroundColor = this.Color(data.r, data.g, data.b, data.a);

		foreach( i, x in this.mTabContents )
		{
			x.getMainWindow().setBlendColor(this.mIdleBackgroundColor);
		}
	}

	function unserializeChatTabs( data )
	{
		foreach( tab in data )
		{
			this.addChatTab(tab.name, true);
			local chatTab = this.mTabContents[tab.name];

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
			foreach( i, tab in this.mTabContents )
			{
				local log = tab.getLog();

				foreach( logMessage in log )
				{
					local message = logMessage.message;

					if (!(this.Util.startsWith(message, "<b>") && this.Util.endsWith(message, "</b>")))
					{
						message = "<b>" + message + "</b>";
						logMessage.message = message;
					}
				}

				this._updateDisplayedMessages(i);
			}
		}
		else
		{
			foreach( i, tab in this.mTabContents )
			{
				local log = tab.getLog();

				foreach( logMessage in log )
				{
					local message = logMessage.message;

					if (this.Util.startsWith(message, "<b>") && this.Util.endsWith(message, "</b>"))
					{
					}

					message = message.slice(3);
					message = message.slice(0, message.len() - 4);
					logMessage.message = message;
				}

				this._updateDisplayedMessages(i);
			}
		}
	}

	function _buildGUI()
	{
		this.setSticky("left", "bottom");
		this.setSize(400, 200);
		this.setPreferredSize(400, 200);
		local x = 5;
		local y = -this.mHeight - 60;
		this.setPosition(x, y);
		this.mTabPane = this.GUI.TabbedPane(true);
		this.mTabPane.addActionListener(this);
		this.mTabPane.setTabPlacement("top");
		this.mTabPane.mContentPane.setAppearance("Container");

		if (this.mTabPane.mTabsPane)
		{
			this.mTabPane.mTabsPane.setInsets(0, 5, 0, 10);
		}

		this.setChatFont(this.GUI.Font("Maiandra", 18));
		this.add(this.mTabPane, this.GUI.BorderLayout.CENTER);
		this.mInputComponent = this.GUI.Component();
		this.mInputComponent.setLayoutManager(this.GUI.GridLayout(1, 2));
		this.mInputComponent.getLayoutManager().setColumns(20, "*");
		this.mInputComponent.getLayoutManager().setRows("*");
		this.mInputComponent.setInsets(2, 0, 0, 5);
		this.mInputComponent.setPassThru(true);
		this.add(this.mInputComponent, this.GUI.BorderLayout.SOUTH);
		this.mTypeChannelLabel = this.GUI.Label("");
		this.mTypeChannelLabel.setFont(this.GUI.Font("Maiandra", 20));
		this.mInputComponent.add(this.mTypeChannelLabel);
		this.mInputArea = this.GUI.InputArea();
		this.mInputArea.setAllowTextEntryOnClick(false);
		this.mInputArea.setAppearance("Container");
		this.mInputArea.setFont(this.GUI.Font("Maiandra", 20));
		this.mInputArea.addActionListener(::_ChatManager);
		this.mInputArea.addActionListener(this);
		this.mInputArea.setMaxCharacters(128);
		this.mInputComponent.add(this.mInputArea);
		this.setChatFont(this.GUI.Font("MaiandraShadow", this.getChatFontSize()));

		foreach( i, x in this.mTabContents )
		{
			x.getMainWindow().setAppearance("ChatWindow");
			x.getMainWindow().setBlendColor(this.Color(0.0, 0.0, 0.0, 0.0));
			this._updateDisplayedMessages(i);
		}
	}

	function _countMaxHTMLComponents( tab )
	{
		local first = this.GUI.HTML("");
		first._setHidden(true);
		local mainPanel = this.mTabContents[tab].getMainWindow();
		local sz = first.getMinimumSize();
		local height = sz.height + mainPanel.getLayoutManager().mGap;
		local total = 0;

		if (mainPanel.mIsRealized == false)
		{
			local mainPanelSize = mainPanel.getPreferredSize();
			local panelHeight = mainPanelSize.height - this.insets.top - this.insets.bottom;
			total = panelHeight.tointeger() / height.tointeger();
		}
		else
		{
			local panelHeight = mainPanel.getHeight() - this.insets.top - this.insets.bottom;
			total = panelHeight.tointeger() / height.tointeger();
		}

		return total;
	}

	function _parseChannelStr( channelStr )
	{
		local str = channelStr;
		local parseStr = "";

		if (this.Util.startsWith(str.tolower(), "*syschat"))
		{
			parseStr = "[SysChat]:";
		}
		else if (this.Util.startsWith(str.tolower(), "s"))
		{
			parseStr = "";
		}
		else if (this.Util.startsWith(str.tolower(), "t/"))
		{
			local resultsStr = ::Util.replace(str, "t/", "");

			if (this.Util.startsWith(resultsStr, "\""))
			{
				resultsStr = resultsStr.slice(1, resultsStr.len());
			}

			if (resultsStr.slice(resultsStr.len() - 1, resultsStr.len()) == "\"")
			{
				resultsStr = resultsStr.slice(0, resultsStr.len() - 1);
			}

			parseStr = "Tell " + resultsStr + ":";
		}
		else if (this.Util.startsWith(str.tolower(), "party"))
		{
			parseStr = "Party:";
		}
		else if (this.Util.startsWith(str.tolower(), "clan"))
		{
			parseStr = "Clan:";
		}
		else if (this.Util.startsWith(str.tolower(), "emote"))
		{
			parseStr = "Emote:";
		}
		else if (this.Util.startsWith(str.tolower(), "tc/"))
		{
			parseStr = "Trade:";
		}
		else if (this.Util.startsWith(str.tolower(), "rc/"))
		{
			parseStr = "Region:";
		}
		else if (this.Util.startsWith(str.tolower(), "gm/"))
		{
			parseStr = "Earthsage:";
		}
		else if (this.Util.startsWith(str.tolower(), "ch/"))
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
				local link = message.slice(sidx, eidx);
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
		local totalHTMLComponents = this._countMaxHTMLComponents(tab);
		local logLen = this.mTabContents[tab].getLog().len();
		local index = this.mTabContents[tab].getScrollBar().getIndex();
		local count = 0;
		local activeHTML = this.mTabContents[tab].getActiveHTML();

		while (logLen > 0 && index >= 0 && count <= totalHTMLComponents)
		{
			local message = this.mTabContents[tab].getLog()[index].message;
			local channel = this.mTabContents[tab].getLog()[index].channel;
			local color = this._ChatManager.getColor(channel);
			local wrapSize = this.getSize().width - 50;
			
			// Search for hyperlinks in the text and turn them into clickable links
			
			local linkReplace = function(link) {
				return "<a href=\"" + link + "\"><font color=\"7777ff\">" + link + "</font></a> ";
			}
			message = _searchAndReplace(message, "http://", " ", linkReplace);
			message = _searchAndReplace(message, "https://", " ", linkReplace);
			local colorReplace = function(color) {
				if(this.Util.startsWith(color, "#0")) 
					return "<font color=\"ff0000\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#1")) 
					return "<font color=\"ff7f00\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#2")) 
					return "<font color=\"ffff00\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#3")) 
					return "<font color=\"00ff00\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#4")) 
					return "<font color=\"0000ff\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#5")) 
					return "<font color=\"ff00ff\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#6")) 
					return "<font color=\"ff00ff\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#7")) 
					return "<font color=\"00ffff\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#8")) 
					return "<font color=\"ffffff\">" + color.slice(2) + "</font>";
				else if(this.Util.startsWith(color, "#9")) 
					return "<font color=\"8a5d27\">" + color.slice(2) + "</font>";
				return color;
			}
			message = _searchAndReplace(message, "#", "#", colorReplace);
			message = _searchAndReplace(message, "{", "}", function(text) {
				return "<i>" + text.slice(1) + "</i>";
			});
			message = _searchAndReplace(message, "\\", "\\", function(text) {
				return "<b>" + text.slice(1) + "</b>";
			});

			if (activeHTML.len() <= count)
			{
				local html = this.GUI.HTML(message);
				html.setLinkStaticColor(color);
				html.setChangeColorOnHover(false);
				html.addActionListener(this);
				html.setResize(true);
				html._setHidden(true);
				html.setFontColor(color);
				html.setWrapText(true, this.getChatFont(), wrapSize);
				this.mTabContents[tab].getMainWindow().add(html);
				activeHTML.append(html);
			}
			else
			{
				activeHTML[count].setText(message);
				activeHTML[count].setSize(0, 0);
				activeHTML[count].setWrapText(true, this.getChatFont(), wrapSize);
				activeHTML[count].setFontColor(color);
				activeHTML[count].setLinkStaticColor(color);
			}

			count++;
			index--;
		}

		while (count < activeHTML.len())
		{
			this.mTabContents[tab].getMainWindow().remove(activeHTML[count]);
			activeHTML.remove(count);
		}
	}

}

class this.ChatTab 
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
		this.mLog = [];
		this.mActiveHTML = [];
		this.mPermanentTab = permanent;

		if (permanent)
		{
			this.mFilters = {};
		}
	}

	function destroy()
	{
		this.mScrollBar.destroy();
		this.mMainWindow.destroy();
	}

	function addFilter( channelCategory )
	{
		if (this.mPermanentTab)
		{
			this.mFilters[channelCategory] <- true;
		}
	}

	function getActiveHTML()
	{
		return this.mActiveHTML;
	}

	function getLog()
	{
		return this.mLog;
	}

	function getName()
	{
		return this.mName;
	}

	function getMainWindow()
	{
		return this.mMainWindow;
	}

	function getScrollBar()
	{
		return this.mScrollBar;
	}

	function getFilterList()
	{
		return this.mFilters;
	}

	function isPermanent()
	{
		return this.mPermanentTab;
	}

	function removeAllFilters()
	{
		if (this.mPermanentTab)
		{
			this.mFilters = {};
		}
	}

	function setActiveHTML( html )
	{
		this.mActiveHTML = html;
	}

	function setFilterList( filterList )
	{
		this.mFilters = filterList;
	}

	function setLog( log )
	{
		this.mLog = log;
	}

	function setMainWindow( mainWindow )
	{
		this.mMainWindow = mainWindow;
	}

	function setName( name )
	{
		this.mName = name;
	}

	function setScrollBar( scrollBar )
	{
		this.mScrollBar = scrollBar;
	}

	function serialize()
	{
		if (this.mPermanentTab)
		{
			return {
				name = this.mName,
				filters = this.mFilters
			};
		}

		return null;
	}

}

::_ChatWindow <- null;
