this.require("UI/UI");
this.require("UI/Screens");
class this.GUI.SocialWindowListObject extends this.GUI.Component
{
	constructor( name, level, race, profession, online, status, shard, menu, rank )
	{
		this.GUI.Component.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setExpand(true);
		this.setInsets(0, 5, -1, 5);
		this.mPlayerName = name;
		this.mLevel = level;
		this.mProfession = profession;
		this.mOnline = online;
		this.mStatus = status;
		this.mMenu = menu;
		this.mRank = rank;
		this.mShard = shard;
		this._setRace(race);
		this.mNameLabel = this.GUI.HTML("");
		this.mNameLabel.setInsets(2, 0, -5, 0);
		this.add(this.mNameLabel);
		this.mInfoHTML = this.GUI.HTML("");
		this.mInfoHTML.setInsets(0, 2, -4, 0);
		this.add(this.mInfoHTML);
		this.mStatusLabel = this.GUI.Label("(No Status)");
		local font = ::GUI.Font("Maiandra", 16);
		this.mStatusLabel.setFont(font);
		this.mStatusLabel.setFontColor(this.Colors.white);
		this.mStatusLabel.setInsets(0, 0, 2, 0);
		this.mStatusLabel.setAutoFit(true);
		this.add(this.mStatusLabel);
		this._buildDisplayString();
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function _setRace( race )
	{
		if (race)
		{
			foreach( i, x in ::Races )
			{
				if (race == i || race == x)
				{
					this.mRace = x;
					return;
				}
			}
		}
		else
		{
			this.mRace = null;
		}
	}

	function _buildDisplayString()
	{
		local color = this.mOnline ? "ffffff" : "d2d2d2";
		this.mNameLabel.setText("<b><font color=\"" + color + "\" size=\"16\">" + this.mPlayerName + "</font>  " + (this.mOnline ? "<font size=\"16\" color=\"8aff87\">(Online)</font></b>" : "<font size=\"16\" color=\"d2d2d2\">(Offline)</font>"));
		local elementFont = this.mOnline ? "<font color=\"fff77f\">" : "<font color=\"d2d2d2\">";
		local info = "Level " + this.mLevel + "  " + this.mProfession;

		if (this.mRank)
		{
			info += "  " + elementFont + "Rank:</font> " + this.mRank;
		}

		if (this.mShard != "")
		{
			info += "  " + elementFont + "Shard:</font> " + this.mShard;
		}

		if (this.mStatus != null)
		{
			this.mStatusLabel.setText(this.mStatus == "" ? "(No Status)" : this.mStatus);
		}

		this.mInfoHTML.setText(info);
	}

	function hideSeperator( bool )
	{
		if (bool)
		{
			this.mSeperator.setVisible(false);
		}
		else
		{
			this.mSeperator.setVisible(true);
		}
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.RBUTTON)
		{
			this.mMenu.showMenu();
		}
	}

	mPlayerName = "";
	mLevel = 0;
	mRace = "";
	mOnline = false;
	mProfession = null;
	mStatus = "";
	mMenu = null;
	mRank = null;
	mShard = "";
	mNameLabel = null;
	mInfoHTML = null;
	mSeperator = null;
	mStatusLabel = null;
}

this.ClanRankValue <- {
	Leader = 4,
	Officer = 3,
	Member = 2,
	Initiate = 1
};
class this.Screens.SocialWindow extends this.GUI.Frame
{
	static mClassName = "Screens.SocialWindow";
	mFriendsBrowser = null;
	mIgnoredBrowser = null;
	mClanBrowser = null;
	mAddButton = null;
	mStatusButton = null;
	mRemoveButton = null;
	mAddIgnoreButton = null;
	mRemoveIgnoreButton = null;
	mClanAddButton = null;
	mClanRemoveButton = null;
	mClanMOTDButton = null;
	mClanDisbandButton = null;
	mClanPromoteButton = null;
	mClanDemoteButton = null;
	mClanMOTDLabel = null;
	mClanMOTD = null;
	mClanLeader = false;
	mFriendPopup = null;
	mFriends = null;
	mClan = null;
	mTabPane = null;
	mRequestEntry = null;
	mRequestComponent = null;
	mRequestLabel = null;
	mFriendsComponent = null;
	mIgnoredComponent = null;
	mClanComponent = null;
	mClanActions = null;
	mClanMemberActions = null;
	mClanName = null;
	mRequestName = null;
	mCurrentTab = null;
	mClanPassword = null;
	mStatus = "";
	mIsClanOfficer = false;
	mClanOfficerActions = null;
	MAX_CHARACTERS = 200;
	constructor()
	{
		this.GUI.Frame.constructor("Contacts");
		this.mFriends = [];
		this.mFriendsBrowser = this.GUI.ColumnList();
		this.mFriendsBrowser.addColumn("Name", 1.0);
		this.mFriendsBrowser.addActionListener(this);
		this.mFriendsBrowser.setShowingHeaders(false);
		this.mFriendsBrowser.setAppearance("DarkBorder");
		this.mFriendsBrowser.setRowAppearance("ColumnSelection");
		this.mFriendsBrowser.setSelectionInsets([
			3,
			3
		]);
		this.mRequestComponent = this.GUI.Container(this.GUI.BoxLayout());
		this.mRequestComponent.getLayoutManager().setExpand(true);
		this.mRequestComponent.setInsets(4, 0, 0, 0);
		this.mRequestLabel = this.GUI.Label();
		this.mRequestLabel.setSize(64, 20);
		this.mRequestEntry = this.GUI.InputArea();
		this.mRequestEntry.setSize(190, 20);
		this.mRequestEntry.setMaxCharacters(this.MAX_CHARACTERS);
		this.mRequestEntry.addActionListener(this);
		this.mRequestComponent.add(this.mRequestLabel);
		this.mRequestComponent.add(this.mRequestEntry);
		this.mAddButton = this.GUI.Button("Add Friend", this, "onAddFriend");
		this.mRemoveButton = this.GUI.Button("Remove Friend", this, "onRemoveFriend");
		this.mStatusButton = this.GUI.Button("Change Status", this, "onStatusChange");
		this.mAddIgnoreButton = this.GUI.Button("Add Name", this, "onAddIgnore");
		this.mRemoveIgnoreButton = this.GUI.Button("Remove Name", this, "onRemoveIgnore");
		this.mFriendsComponent = this.GUI.Container(this.GUI.BorderLayout());
		this.mFriendsComponent.add(this.GUI.ScrollPanel(this.mFriendsBrowser), this.GUI.BorderLayout.CENTER);
		local friendsActions = this.GUI.Container(this.GUI.BoxLayout());
		friendsActions.getLayoutManager().setPackAlignment(0.5);
		friendsActions.setInsets(3, 0, 3, 3);
		friendsActions.add(this.mAddButton);
		friendsActions.add(this.mRemoveButton);
		friendsActions.add(this.mStatusButton);
		local friendsPane = this.GUI.Container(this.GUI.BorderLayout());
		friendsPane.add(this.mFriendsComponent, this.GUI.BorderLayout.CENTER);
		friendsPane.add(friendsActions, this.GUI.BorderLayout.SOUTH);
		friendsPane.setInsets(3);
		this.mIgnoredBrowser = this.GUI.ColumnList();
		this.mIgnoredBrowser.addColumn("Name", 150);
		this.mIgnoredBrowser.setShowingHeaders(false);
		this.mIgnoredBrowser.setAppearance("DarkBorder");
		this.mIgnoredComponent = this.GUI.Component(this.GUI.BorderLayout());
		this.mIgnoredComponent.add(this.GUI.ScrollPanel(this.mIgnoredBrowser), this.GUI.BorderLayout.CENTER);
		local ignoredActions = this.GUI.Container(this.GUI.BoxLayout());
		ignoredActions.getLayoutManager().setPackAlignment(0.5);
		ignoredActions.setInsets(3, 0, 3, 3);
		ignoredActions.add(this.mAddIgnoreButton);
		ignoredActions.add(this.mRemoveIgnoreButton);
		local ignoredPane = this.GUI.Container(this.GUI.BorderLayout());
		ignoredPane.add(this.mIgnoredComponent, this.GUI.BorderLayout.CENTER);
		ignoredPane.add(ignoredActions, this.GUI.BorderLayout.SOUTH);
		ignoredPane.setInsets(3);
		this.mClanBrowser = this.GUI.ColumnList();
		this.mClanBrowser.addColumn("Name", 100);
		this.mClanBrowser.setShowingHeaders(false);
		this.mClanBrowser.setAppearance("DarkBorder");
		this.mClanBrowser.setRowAppearance("ColumnSelection");
		this.mClanBrowser.setSelectionInsets([
			3,
			3
		]);
		this.mClanMOTDLabel = this.GUI.HTML("");
		this.mClanMOTDLabel.setPreferredSize(100, 50);
		this.mClanMOTDLabel.setText("<i>Downloading clan information...</i>");
		this.mClanComponent = this.GUI.Component(this.GUI.BorderLayout());
		this.mClanComponent.add(this.mClanMOTDLabel, this.GUI.BorderLayout.CENTER);
		this.mClanAddButton = this.GUI.Button("Add", this, "onClanAddMember");
		this.mClanRemoveButton = this.GUI.Button("Remove", this, "onClanRemoveMember");
		this.mClanMOTDButton = this.GUI.Button("MotD", this, "onClanMOTDChange");
		this.mClanDisbandButton = this.GUI.Button("Disband", this, "onClanDisband");
		this.mClanPromoteButton = this.GUI.Button("Promote", this, "onClanPromote");
		this.mClanDemoteButton = this.GUI.Button("Demote", this, "onClanDemote");
		local leaveButton = this.GUI.Button("Leave Clan", this, "onLeaveClan");
		leaveButton.setSize(25, 100);
		this.mClanMemberActions = this.GUI.Container(this.GUI.BoxLayout());
		this.mClanMemberActions.getLayoutManager().setPackAlignment(0.5);
		this.mClanMemberActions.add(leaveButton);
		this.mClanMemberActions.setInsets(3, 0, 3, 3);
		this.mClanMemberActions.setVisible(false);
		this.mClanActions = this.GUI.Container(this.GUI.GridLayout(2, 3));
		this.mClanActions.setInsets(3, 0, 3, 3);
		this.mClanActions.add(this.mClanMOTDButton);
		this.mClanActions.add(this.mClanPromoteButton);
		this.mClanActions.add(this.mClanAddButton);
		this.mClanActions.add(this.mClanDisbandButton);
		this.mClanActions.add(this.mClanDemoteButton);
		this.mClanActions.add(this.mClanRemoveButton);
		this.mClanActions.setVisible(false);
		local clanAddButton = this.GUI.Button("Add", this, "onClanAddMember");
		local clanRemoveButton = this.GUI.Button("Remove", this, "onClanRemoveMember");
		local clanMOTDButton = this.GUI.Button("MotD", this, "onClanMOTDChange");
		local clanLeaveButton = this.GUI.Button("Leave Clan", this, "onLeaveClan");
		local clanPromoteButton = this.GUI.Button("Promote", this, "onClanPromote");
		local clanDemoteButton = this.GUI.Button("Demote", this, "onClanDemote");
		local clanLeaveButton = this.GUI.Button("Leave Clan", this, "onLeaveClan");
		this.mClanOfficerActions = this.GUI.Container(this.GUI.GridLayout(2, 3));
		this.mClanOfficerActions.setInsets(3, 0, 3, 3);
		this.mClanOfficerActions.add(clanMOTDButton);
		this.mClanOfficerActions.add(clanPromoteButton);
		this.mClanOfficerActions.add(clanAddButton);
		this.mClanOfficerActions.add(clanLeaveButton);
		this.mClanOfficerActions.add(clanDemoteButton);
		this.mClanOfficerActions.add(clanRemoveButton);
		this.mClanOfficerActions.setVisible(false);
		local clanPane = this.GUI.Container(this.GUI.BorderLayout());
		clanPane.add(this.mClanComponent, this.GUI.BorderLayout.CENTER);
		clanPane.add(this.mClanActions, this.GUI.BorderLayout.SOUTH);
		clanPane.add(this.mClanMemberActions, this.GUI.BorderLayout.SOUTH);
		clanPane.add(this.mClanOfficerActions, this.GUI.BorderLayout.SOUTH);
		clanPane.setInsets(3);
		this.mTabPane = this.GUI.TabbedPane();
		this.mTabPane.setTabPlacement("top");
		this.mTabPane.setTabFontColor("E4E4E4");
		this.mTabPane.addTab("Friends", friendsPane);
		this.mTabPane.addTab("Clan", clanPane);
		this.mTabPane.addTab("Ignored", ignoredPane);
		this.mTabPane.addActionListener(this);
		this.mTabPane.setInsets(3);
		this.mCurrentTab = "Friends";
		this.mFriendPopup = this.GUI.PopupMenu();
		this.mFriendPopup.addActionListener(this);
		this.mFriendPopup.addMenuOption("IM", "Send Instant Message");
		this.mFriendPopup.addMenuOption("SHARD", "Join friend\'s shard");
		this.setContentPane(this.mTabPane);
		this.setSize(300, 500);
		this.getFriendsList();
		this.fillIgnoreList();
		this.setCached(::Pref.get("video.UICache"));
		::_Connection.sendQuery("friends.getstatus", this, []);
		::_Connection.sendQuery("clan.info", this, []);
		::_Connection.sendQuery("clan.list", this, []);
		::_Connection.addListener(this);
		::_ChatManager.addListener(this);
	}

	function onClanLeft()
	{
		this.mClanName = null;
		this.mClanMOTD = null;
		this.updateClanPanel();
	}

	function onLeaveClan( button )
	{
		local request = ::GUI.PopupQuestionBox("Are you sure you want to leave the Clan \'" + this.mClanName + "\'?");
		request.setAcceptActionName("onClanLeaveAccept");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanLeaveAccept( window )
	{
		window.destroy();
		::_Connection.sendQuery("clan.leave", this, []);
	}

	function onClanDisbanded()
	{
		this.mClanName = null;
		this.mClanMOTD = null;
		this.mClanLeader = false;
		this.updateClanPanel();
		this.mClanMOTDLabel.setText("<i><font size=\"14\">The clan has been disbanded.</font></i>");
	}

	function onInputComplete( entry )
	{
		local text = this.Util.trim(entry.getText());

		switch(this.mRequestName)
		{
		case "AddFriend":
			if (text != "")
			{
				::_Connection.sendQuery("friends.add", this, [
					text
				]);
			}

			break;

		case "Status":
			::_Connection.sendQuery("friends.status", this, [
				text
			]);
			this.mStatus = text;
			break;

		case "Ignore":
			if (text != "")
			{
				::_ChatManager.ignorePlayer(text);
			}

			break;
		}

		this.hideRequest();
	}

	function showRequest( name, text, ... )
	{
		this.mRequestLabel.setText(text);
		this.mRequestName = name;

		if (vargc > 0)
		{
			this.mRequestEntry.setText(vargv[0]);
		}
		else
		{
			this.mRequestEntry.setText("");
		}

		this.mRequestEntry.setCursorPosition(this.mRequestEntry.getText().len());

		if (this.mCurrentTab == "Friends")
		{
			this.mFriendsComponent.removeAll();
			this.mFriendsComponent.add(this.GUI.ScrollPanel(this.mFriendsBrowser), this.GUI.BorderLayout.CENTER);
			this.mFriendsComponent.add(this.mRequestComponent, this.GUI.BorderLayout.SOUTH);
		}
		else if (this.mCurrentTab == "Ignored")
		{
			this.mIgnoredComponent.removeAll();
			this.mIgnoredComponent.add(this.GUI.ScrollPanel(this.mIgnoredBrowser), this.GUI.BorderLayout.CENTER);
			this.mIgnoredComponent.add(this.mRequestComponent, this.GUI.BorderLayout.SOUTH);
		}

		this.GUI._Manager.requestKeyboardFocus(this.mRequestEntry);
	}

	function onTabSelected( pane, tab )
	{
		this.hideRequest();
		this.mCurrentTab = tab.name;
	}

	function hideRequest()
	{
		if (this.mCurrentTab == "Friends")
		{
			this.mFriendsComponent.removeAll();
			this.mFriendsComponent.add(this.GUI.ScrollPanel(this.mFriendsBrowser), this.GUI.BorderLayout.CENTER);
		}
		else if (this.mCurrentTab == "Ignored")
		{
			this.mIgnoredComponent.removeAll();
			this.mIgnoredComponent.add(this.GUI.ScrollPanel(this.mIgnoredBrowser), this.GUI.BorderLayout.CENTER);
		}
	}

	function onMenuItemPressed( menu, menuID )
	{
		local entry = this.getSelectedFriendEntry();

		if (entry == null)
		{
			return;
		}

		switch(menuID)
		{
		case "IM":
			::_ChatWindow.onStartChatInput();
			::_ChatWindow.addStringInput("/tell \"" + entry.name + "\" ");
			break;

		case "SHARD":
			if (entry.shard != "")
			{
				::_Connection.sendQuery("shard.set", this, [
					entry.shard
				]);
			}

			break;
		}
	}

	function findFriend( name )
	{
		foreach( f in this.mFriends )
		{
			if (f.name == name)
			{
				return f;
			}
		}

		return null;
	}

	function findClanMember( name )
	{
		foreach( f in this.mClan )
		{
			if (f.name == name)
			{
				return f;
			}
		}

		return null;
	}

	function onRowSelectionChanged( list, index, value )
	{
	}

	function onRefresh( button )
	{
		this.getFriendsList();
	}

	function onPlayerIgnored( manager, name )
	{
		this.fillIgnoreList();
	}

	function onPlayerUnignored( manager, name )
	{
		this.fillIgnoreList();
	}

	function getFriendsList()
	{
		::_Connection.sendQuery("friends.list", this, []);
		this.mAddButton.setEnabled(false);
		this.mRemoveButton.setEnabled(false);
	}

	function fillIgnoreList()
	{
		this.mIgnoredBrowser.removeAllRows();
		local ignoreMap = ::Pref.get("chat.ignoreList");

		foreach( k, v in ignoreMap )
		{
			this.mIgnoredBrowser.addRow([
				k
			]);
		}
	}

	function onStatusChange( button )
	{
		this.showRequest("Status", "Status Text:", this.mStatus);
	}

	function onAddIgnore( button )
	{
		this.showRequest("Ignore", "Name:");
	}

	function onCancelInput( window, text )
	{
		this.mStatusQuestion = null;
		this.mAddIgnoreQuestion = null;
		this.mAddFriendQuestion = null;
	}

	function onRemoveIgnore( button )
	{
		local name = this.getSelectedIgnored();

		if (name == null)
		{
			return;
		}

		::_ChatManager.unignorePlayer(name);
	}

	function onAddFriend( button )
	{
		this.showRequest("AddFriend", "Friend Name:");
	}

	function onInput( window, text )
	{
		if (this.mAddFriendQuestion)
		{
			local t = this.Util.trim(text);

			if (t != "")
			{
				::_Connection.sendQuery("friends.add", this, [
					t
				]);
			}

			this.mAddFriendQuestion = null;
		}
		else if (this.mAddIgnoreQuestion)
		{
			::_ChatManager.ignorePlayer(text);
			this.mAddIgnoreQuestion = null;
		}
		else if (this.mStatusQuestion)
		{
			::_Connection.sendQuery("friends.status", this, [
				text
			]);
			this.mStatusQuestion = null;
		}
	}

	function onRemoveFriend( button )
	{
		local friend = this.getSelectedFriend();

		if (friend == null)
		{
			return;
		}

		::_Connection.sendQuery("friends.remove", this, [
			friend
		]);
	}

	function onQueryError( qa, error )
	{
		this.IGIS.error(error);
	}

	function getSelectedFriend()
	{
		local entry = this.getSelectedFriendEntry();

		if (entry == null)
		{
			return null;
		}

		return entry.name;
	}

	function getSelectedFriendEntry()
	{
		local rows = this.mFriendsBrowser.getSelectedRows();

		if (rows.len() == 0)
		{
			return null;
		}

		local index = rows[0];

		if (index >= this.mFriends.len())
		{
			return null;
		}

		return this.mFriends[index];
	}

	function getSelectedClanMember()
	{
		local rows = this.mClanBrowser.getSelectedRows();

		if (rows.len() == 0)
		{
			return null;
		}

		local index = rows[0];

		if (index >= this.mClan.len())
		{
			return null;
		}

		return this.mClan[index].name;
	}

	function getSelectedIgnored()
	{
		local rows = this.mIgnoredBrowser.getSelectedRows();

		if (rows.len() == 0)
		{
			return null;
		}

		return this.mIgnoredBrowser.getRow(rows[0])[0];
	}

	function updateFriendBrowser()
	{
		local name = this.getSelectedFriend();
		this.mFriendsBrowser.removeAllRows();

		foreach( f in this.mFriends )
		{
			local obj = this.GUI.SocialWindowListObject(f.name, f.level, "Test", ::Professions[f.profession].name, f.online, f.status, f.shard, this.mFriendPopup, null);
			this.mFriendsBrowser.addRow([
				obj
			]);

			if (name == f.name)
			{
				this.mFriendsBrowser.setSelectedRows([
					this.mFriendsBrowser.getRowCount() - 1
				]);
			}
		}
	}

	function updateClanBrowser()
	{
		local name = this.getSelectedClanMember();
		this.mClanBrowser.removeAllRows();

		foreach( f in this.mClan )
		{
			local obj = this.GUI.SocialWindowListObject(f.name, f.level, "Test", ::Professions[f.profession].name, f.online, null, "", this.mFriendPopup, f.rank);
			this.mClanBrowser.addRow([
				obj
			]);

			if (name == f.name)
			{
				this.mClanBrowser.setSelectedRows([
					this.mClanBrowser.getRowCount() - 1
				]);
			}
		}
	}

	function sortClanMemberRank( m1, m2 )
	{
		if (m1 == m2)
		{
			return 0;
		}

		if (this.ClanRankValue[m1.rank] > this.ClanRankValue[m2.rank])
		{
			return -1;
		}

		if (this.ClanRankValue[m1.rank] < this.ClanRankValue[m2.rank])
		{
			return 1;
		}

		return 0;
	}

	function sortClan()
	{
		local online = [];
		local offline = [];
		local tmp = [];

		foreach( f in this.mClan )
		{
			if (f.online)
			{
				online.append(f);
			}
			else
			{
				offline.append(f);
			}
		}

		this.Util.bubbleSort(online, this.sortClanMemberRank);
		this.Util.bubbleSort(offline, this.sortClanMemberRank);
		local tmp = [];

		foreach( f in online )
		{
			tmp.append(f);
		}

		foreach( f in offline )
		{
			tmp.append(f);
		}

		this.mClan = tmp;
	}

	function sortFriends()
	{
		local online = [];
		local offline = [];
		local tmp = [];

		foreach( f in this.mFriends )
		{
			if (f.online)
			{
				online.append(f);
			}
			else
			{
				offline.append(f);
			}
		}

		local tmp = [];

		foreach( f in online )
		{
			tmp.append(f);
		}

		foreach( f in offline )
		{
			tmp.append(f);
		}

		this.mFriends = tmp;
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "friends.list")
		{
			this.mFriends = [];
			local x = 0;

			foreach( r in results )
			{
				local name = r[0];
				local level = r[1];
				local profession = r[2];
				local online = r[3];
				local status = r[4];
				local shard = r.len() > 5 ? r[5] : "";
				this.mFriends.append({
					name = name,
					profession = profession.tointeger(),
					level = level,
					online = online == "true",
					status = status,
					shard = shard
				});
				x++;
			}

			this.mAddButton.setEnabled(true);
			this.mRemoveButton.setEnabled(true);
			this.sortFriends();
			this.updateFriendBrowser();
		}
		else if (qa.query == "friends.remove")
		{
			this.getFriendsList();
		}
		else if (qa.query == "friends.getstatus")
		{
			this.mStatus = results[0][0];
		}
		else if (qa.query == "clan.info")
		{
			if (results.len() > 0)
			{
				this.mClanName = results[0][0];
				this.mClanMOTD = results[1][0];
				this.mClanLeader = results[2][0] == "true";
			}
			else
			{
				this.mClanName = null;
				this.mClanMOTD = null;
				this.mClanLeader = false;
			}

			this.mClanMOTDButton.setEnabled(true);
			this.updateClanPanel();
		}
		else if (qa.query == "clan.list")
		{
			this.mClan = [];
			local x = 0;

			foreach( r in results )
			{
				local name = r[0];
				local level = r[1];
				local profession = r[2];
				local online = r[3];
				local rank = r[4];
				this.mClan.append({
					name = name,
					profession = profession.tointeger(),
					level = level,
					online = online == "true",
					rank = rank
				});
				this._setClanOfficer(name, rank);
				x++;
			}

			this.sortClan();
			this.updateClanBrowser();
			this.updateClanPanel();
		}
	}

	function _setClanOfficer( name, rank )
	{
		if (::_avatar)
		{
			local avatarName = ::_avatar.getName();

			if (avatarName == name)
			{
				if (rank == "Officer")
				{
					this.mIsClanOfficer = true;
					this.mClanLeader = false;
				}
				else
				{
					this.mIsClanOfficer = false;
				}

				if (rank == "Leader")
				{
					this.mClanLeader = true;
				}
			}
		}
	}

	function onClanMemberLeft( name )
	{
		local x;

		for( x = 0; x < this.mClan.len(); x++ )
		{
			if (this.mClan[x].name == name)
			{
				this.mClan.remove(x);
				break;
			}
		}

		this.updateClanBrowser();
	}

	function onClanMemberJoined( name, level, profession )
	{
		this.mClan.append({
			name = name,
			profession = profession,
			level = level,
			rank = "Initiate",
			online = true
		});
		this.sortClan();
		this.updateClanBrowser();
	}

	function onClanRankChanged( name, rank )
	{
		local member = this.findClanMember(name);

		if (member == null)
		{
			return;
		}

		member.rank = rank;
		this._setClanOfficer(name, member.rank);
		this.sortClan();
		this.updateClanBrowser();
		this.updateClanPanel();
	}

	function onClanJoined( name )
	{
		this.mClanName = null;
		this.mClanMOTD = null;
		this.mClanLeader = false;
		this.updateClanPanel();
		::_Connection.sendQuery("clan.list", this, []);
		::_Connection.sendQuery("clan.info", this, []);
	}

	function isClanLeader()
	{
		return this.mClanLeader;
	}

	function getClanName()
	{
		return this.mClanName;
	}

	function onClanAddMember( button )
	{
		local request = ::GUI.PopupInputBox("Enter name of new Clan member:");
		request.setActionName("onClanAddNameInput");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanAddNameInput( window )
	{
		window.destroy();
		::_Connection.sendQuery("clan.invite", this, [
			window.getText()
		]);
	}

	function onClanRemoveMember( button )
	{
		local member = this.getSelectedClanMember();

		if (member == null)
		{
			return;
		}

		local request = ::GUI.PopupQuestionBox("Are you sure you want to remove <b>" + member + "</b> from the clan?");
		request.setAcceptActionName("onClanRemoveMemberAccept");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanRemoveMemberAccept( window )
	{
		window.destroy();
		local member = this.getSelectedClanMember();

		if (member == null)
		{
			return;
		}

		::_Connection.sendQuery("clan.remove", this, [
			member
		]);
	}

	function getPromotedRank( currentRank )
	{
		if (currentRank == "Initiate")
		{
			return "Member";
		}

		if (currentRank == "Member")
		{
			return "Officer";
		}

		return null;
	}

	function getDemotedRank( currentRank )
	{
		if (currentRank == "Member")
		{
			return "Initiate";
		}

		if (currentRank == "Officer")
		{
			return "Member";
		}

		return null;
	}

	function onClanPromote( button )
	{
		local member = this.getSelectedClanMember();

		if (member == null)
		{
			return;
		}

		foreach( m in this.mClan )
		{
			if (m.name == member)
			{
				local rank = this.getPromotedRank(m.rank);

				if (rank)
				{
					::_Connection.sendQuery("clan.rank", this, [
						member,
						rank
					]);
				}
			}
		}
	}

	function onClanDemote( button )
	{
		local member = this.getSelectedClanMember();

		if (member == null)
		{
			return;
		}

		foreach( m in this.mClan )
		{
			if (m.name == member)
			{
				local rank = this.getDemotedRank(m.rank);

				if (rank)
				{
					::_Connection.sendQuery("clan.rank", this, [
						member,
						rank
					]);
				}
			}
		}
	}

	function onClanMOTDChange( button )
	{
		local request = ::GUI.PopupInputBox("Enter a new message:");
		request.setActionName("onClanMOTDInput");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanMOTDInput( request )
	{
		local text = request.getText();
		::_Connection.sendQuery("clan.motd", this, [
			text
		]);
		request.destroy();
	}

	function onClanDisband( button )
	{
		local request = ::GUI.PopupQuestionBox("<font color=\"ffdd66\"><b>Warning:</b></font> This will disband your Clan permanently.<br/>Do you wish to proceed?");
		request.setAcceptActionName("onClanDisbandAccept");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanDisbandAccept( window )
	{
		window.destroy();

		if (::_Connection.getProtocolVersionId() < 28)
		{
			local request = ::GUI.PopupInputBox("Enter Clan leader password:");
			request.setActionName("onClanDisbandPasswordInput");
			request.addActionListener(this);
			request.setPassword(true);
			request.showInputBox();
			request.center();
		}
		else
		{
			this.onClanDisbandPasswordInput(null);
		}
	}

	function onClanDisbandPasswordInput( window )
	{
		if (::_Connection.getProtocolVersionId() < 28)
		{
			this.mClanPassword = window.getText();
			window.destroy();
		}

		local request = ::GUI.PopupQuestionBox("Are you sure you wish to disband the Clan?");
		request.setAcceptActionName("onClanDisbandSend");
		request.addActionListener(this);
		request.showInputBox();
		request.center();
	}

	function onClanDisbandSend( window )
	{
		window.destroy();

		if (::_Connection.getProtocolVersionId() < 28)
		{
			::_Connection.sendQuery("clan.disband", this, [
				this.mClanPassword
			]);
			this.mClanPassword = null;
		}
		else
		{
			::_Connection.sendQuery("clan.disband", this);
		}
	}

	function onClanMOTDChanged( text )
	{
		this.mClanMOTD = text;
		this.updateClanMOTD();
	}

	function updateClanPanel()
	{
		this.mClanComponent.removeAll();

		if (this.mClanName == null)
		{
			this.mClanMOTDLabel.setText("<i>You are not in a clan.</i>");
			this.mClanComponent.add(this.mClanMOTDLabel, this.GUI.BorderLayout.CENTER);
			this.mClanMemberActions.setVisible(false);
			this.mClanActions.setVisible(false);
			this.mClanOfficerActions.setVisible(false);
		}
		else
		{
			this.mClanComponent.add(this.mClanMOTDLabel, this.GUI.BorderLayout.NORTH);
			this.mClanComponent.add(this.GUI.ScrollPanel(this.mClanBrowser), this.GUI.BorderLayout.CENTER);

			if (this.mClanLeader)
			{
				this.mClanMemberActions.setVisible(false);
				this.mClanActions.setVisible(true);
				this.mClanOfficerActions.setVisible(false);
			}
			else if (this.mIsClanOfficer)
			{
				this.mClanMemberActions.setVisible(false);
				this.mClanActions.setVisible(false);
				this.mClanOfficerActions.setVisible(true);
			}
			else
			{
				this.mClanMemberActions.setVisible(true);
				this.mClanActions.setVisible(false);
				this.mClanOfficerActions.setVisible(false);
			}

			this.updateClanMOTD();
		}
	}

	function updateClanMOTD()
	{
		local txt;

		if (this.mClanName == null || this.mClanMOTD == null)
		{
			txt = "<i>Downloading clan information...</i>";
		}
		else
		{
			txt = "<b><font size=\"23\" color=\"fff77f\">" + this.mClanName + "</font></b><br><i><font size=\"16\">" + (this.mClanMOTD != "" ? this.mClanMOTD : "Clan message not set.") + "</font></i>";
		}

		this.mClanMOTDLabel.setText(txt);
	}

	function onQueryTimeout( qa )
	{
		if (qa.query == "friends.list")
		{
			this.getFriendsList();
		}
	}

	function onPlayerStatusChanged( name, text )
	{
		local friend = this.findFriend(name);

		if (friend == null)
		{
			return;
		}

		friend.status = text;
		this.updateFriendBrowser();
	}

	function onClanPlayerLoggedIn( name )
	{
		local member = this.findClanMember(name);

		if (member == null)
		{
			return;
		}

		member.online = true;
		this.sortClan();
		this.updateClanBrowser();
	}

	function onClanPlayerLoggedOut( name )
	{
		local member = this.findClanMember(name);

		if (member == null)
		{
			return;
		}

		member.online = false;
		this.sortClan();
		this.updateClanBrowser();
	}

	function onPlayerShardChanged( name, shard )
	{
		local friend = this.findFriend(name);

		if (friend == null)
		{
			return;
		}

		friend.shard = shard;
		this.updateFriendBrowser();
	}

	function onPlayerLoggedIn( name )
	{
		local friend = this.findFriend(name);

		if (friend == null)
		{
			return;
		}

		friend.online = true;
		this.sortFriends();
		this.updateFriendBrowser();
	}

	function onPlayerLoggedOut( name )
	{
		local friend = this.findFriend(name);

		if (friend == null)
		{
			return;
		}

		friend.online = false;
		this.sortFriends();
		this.updateFriendBrowser();
	}

	function onFriendAdded( name )
	{
		this.getFriendsList();
	}

	function setVisible( value )
	{
		this.GUI.Frame.setVisible(value);

		if (value)
		{
			::Audio.playSound("Sound-Friendslist.ogg");
		}
	}

}

