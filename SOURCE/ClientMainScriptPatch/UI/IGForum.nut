require("GUI/Link");

GUI.Link["mColor"] <- "FF0000";  //"447FFF";
GUI.Link.mHoverColor <- "FF0000"; //"AAC3FF";

function TryGetInteger(str)
{
	try
	{
		return str.tointeger();
	}
	catch(e)
	{
		return 0;
	}
	return 0;
}

function TryGetString(str)
{
	try
	{
		return str.tostring();
	}
	catch(e)
	{
	}
	return "";
}

class GUI.HTML2 extends GUI.HTML
{
	function onLinkClicked(button)
	{
		local target = button.getData().href;

		if(::Util.startsWith(target, "item:") == true)
		{
			local arr = ::Util.split(target, ":");
			if(arr.len() < 2)
				return;
			local ID = arr[1].tointeger();
			local lookID = 0;
			if(arr.len() >= 3)
				lookID = arr[2].tointeger();
			local scr = Screens.show("PreviewItem");
			scr.SetItem(ID, lookID);
		}
		else if(::Util.startsWith(target, "post:") == true)
		{
			local arr = ::Util.split(target, ":");
			if(arr.len() < 2)
				return;
			local postID = arr[1].tointeger();
			local scr = Screens.get("IGForum", true);
			scr.SetSelectedPost(postID);
		}
		else if(::Util.startsWith(target, "copy:") == true)
		{
			local str = target.slice(5, target.len());
			//str = ::Util.replace(str, "\\\"", "\"");
			str = ::Util.replace(str, "^Q", "\"");
			::System.setClipboard(str);
			IGIS.info("Copied tag to clipboard.");
		}
		else if(::Util.startsWith(target, "grove:") == true)
		{
			local str = target.slice(6, target.len());
			local data = ::Util.split(str, ":");
			local xloc = 0;
			local zloc = 0;
			local target = "";
			if(data.len() >= 1)
				target = data[0];
			if(data.len() >= 3)
			{
				local carr = ::Util.split(data[2], ",");
				if(carr.len() == 2)
				{
					xloc = TryGetInteger(carr[0]);
					zloc = TryGetInteger(carr[1]);
				}
			}
			if(target != "")
			{
				::_Connection.sendQuery("warpg", this, [ target, xloc, zloc ] );
			}
		}
		else
			System.openURL(target);
	}
}

	function GetTagLocation(text, startPos, tagBegin, tagEnd)
	{
		local spos = text.find(tagBegin, startPos);
		if(spos == null)
			return null;
		local epos = text.find(tagEnd, spos);
		if(epos == null)
			return { error = true, start = spos, end = (spos + tagBegin.len()), substr = ""};

		local part = text.slice(spos + tagBegin.len(), epos);

		epos += tagEnd.len();
		return {start = spos, end = epos, substr = part};
	}
	function GetTagLocation(text, startPos, rules)
	{
		local tagBegin = rules.searchOpen;
		local tagEnd = rules.searchClose;
		if(rules.ext == true)
			tagBegin = tagBegin.slice(0, tagBegin.len() - 1);

		local spos = text.find(tagBegin, startPos);
		if(spos == null)
			return null;

		local tagLen = tagBegin.len();
		local optParam = "";
		if(rules.ext == true)
		{
			local endBracket = text.find("]", spos);
			if(endBracket == null)
				return null;

			local paramStr = text.slice(spos, endBracket);
			local param = ::Util.split(paramStr, "=");
			if(param.len() >= 2)
			{
				for(local i = 1; i < param.len(); i++)
				{
					if(i > 1)
						optParam += "=";
					optParam += param[i];
				}
			}
			tagLen = endBracket - spos + 1;
		}

		local epos = text.find(tagEnd, spos);
		local fail = false;
		if(epos == null)
			fail = true;
		else if(spos + tagLen >= epos)
		{
			fail = true;
			tagLen = epos + tagEnd.len();
		}
		if(fail == true)
			return { error = true, start = spos, end = (spos + tagLen), substr = "", opt = ""};

		local part = text.slice(spos + tagLen, epos);

		epos += tagEnd.len();
		return {start = spos, end = epos, substr = part, opt = optParam};
	}
	function IsNum(str)
	{
		if(str.len() == 0)
			return false;

		foreach(i, d in str)
		{
			if(d < '0' || d > '9')
				return false;
		}
		return true;
	}
	function TrySlice(text, startPos, endPos)
	{
		local ret = "";
		try
		{
			ret = text.slice(startPos, endPos);
		}
		catch(e)
		{
		}
		return ret;
	}
	function ResolvePostTags(text)
	{
		local pos = 0;
		local result = 0;
		local tagTable =
		{
			  ["b"] = { searchOpen = "[b]", searchClose = "[/b]", repOpen = "<b>", repClose = "</b>", ext = false },
			  ["i"] = { searchOpen = "[i]", searchClose = "[/i]", repOpen = "<i>", repClose = "</i>", ext = false },
			["link"] = { searchOpen = "[link]", searchClose = "[/link]", repOpen = ".", repClose = ".", ext = false },
			["item"] = { searchOpen = "[item]", searchClose = "[/item]", repOpen = ".", repClose = ".", ext = false },
			["url"] = { searchOpen = "[url]", searchClose = "[/url]", repOpen = ".", repClose = ".", ext = true},
			["color"] = { searchOpen = "[color]", searchClose = "[/color]", repOpen = ".", repClose = ".", ext = true},
			["copy"] = { searchOpen = "[copy]", searchClose = "[/copy]", repOpen = ".", repClose = ".", ext = true},
			["grove"] = { searchOpen = "[grove]", searchClose = "[/grove]", repOpen = ".", repClose = ".", ext = false},
		}
		foreach(i, d in tagTable)
		{
			pos = 0;
			while(true)
			{
				//result = GetTagLocation(text, pos, d.searchOpen, d.searchClose);
				result = GetTagLocation(text, pos, d);
				if(result == null)
					break;
				pos = result.end;

				//local first = text.slice(0, result.start);
				//local second = text.slice(result.end, text.len());
				local first = TrySlice(text, 0, result.start);
				local second = TrySlice(text, result.end, text.len());
				local replace = result.substr;
				if(replace.len() > 0)
				{
					if(i == "link")
						replace = "<a href=\"" + result.substr + "\">" + result.substr + "</a>";
					else if(i == "url")
					{
						/*
						local data = ::Util.split(result.substr, "=");
						local short = "";
						local addr = "";
						if(data.len() >= 1)
							short = data[0];
						if(data.len() >= 2)
						{
							for(local i = 1; i < data.len(); i++)
							{
								if(i > 1)
									addr += "=";
								addr += data[i];
							}
						}
						replace = "<a href=\"" + addr + "\">" + short + "</a>";
						*/

						local addr = result.opt;
						local short = result.substr;
						replace = "<a href=\"" + addr + "\">" + short + "</a>";
					}
					else if(i == "item")
					{
						local data = [];
						if(result.substr)
							data = ::Util.split(result.substr, ":");
						local ID = 0;
						local Name = "[Item Preview]";
						local lookID = 0;
						if(data.len() >= 1)
						{
							if(IsNum(data[0]) == true)
								ID = data[0].tointeger();
						}

						local size = data.len();
						if(size >= 3)
						{
							local last = data[size - 1];
							if(IsNum(last) == true)
							{
								lookID = last.tointeger();
								data.pop();
							}
						}
						if(data.len() >= 2)
							data.remove(0);

						//Now that the first (ID) and Last (optional lookID) are removed
						//from the array, everything else is joined back into the name.
						if(data.len() > 0)
						{
							local joined = ::Util.join(data, ":");
							Name = "[" + joined + "]";
						}

						/*
						if(data.len() >= 2)
							Name = "[" + data[1] + "]";
						if(data.len() >= 3)
						{
							local t = data[2];
							if(IsNum(t) == true)
								lookID = data[2].tointeger();
							else
								IGIS.info("Not a number");
						}
						*/

						/*
						local last = data.len();
						if(last >= 3)
						{
							lookID = data[last - 1].tointeger();
							data.pop();
						}
						if(data.len() >= 2)
						{
							data.remove(0);
							local joined = ::Util.join(data);
							Name = "[" + joined + "]";
						}
						*/

						local linkstr = ID.tostring();
						if(lookID != 0)
							linkstr += ":" + lookID.tostring();
						replace = "<a href=\"item:" + linkstr + "\">" + Name + "</a>";
					}
					else if(i == "color")
					{
						replace = "<font color=\"" + result.opt + "\">" + result.substr + "</font>";
					}
					else if(i == "copy")
					{
						local shortname = result.opt;
						//local copytext = ::Util.replace(result.substr, "\"", "\\\"");
						local copytext = ::Util.replace(result.substr, "\"", "^Q");
						replace = "<a href=\"copy:" + copytext + "\">[Copy:" + shortname + "]</a>";
					}
					else if(i == "grove")
					{
						local name = "(no grove)";
						local arr = ::Util.split(result.substr, ":");
						if(arr.len() > 1)
						{
							name = TryGetString(arr[1]);
							if(name == "")
								name = arr[0];
						}
						else if(arr.len() == 1)
							name = arr[0];
						else if(arr.len() == 0)
							name = result.substr;
						replace = "<a href=\"grove:" + result.substr + "\">[Grove:" + name + "]</a>";
					}
					else
						replace = d.repOpen + result.substr + d.repClose;
				}
				text = first + replace + second;
			}
		}
		return text;
	}
	function ServerToHTML(text)
	{
		//The server uses a special encoding for newlines since it stores files as plain text,
		//and writing newlines will screw up the format.
		text = ::Util.convertHTMLtoText(text);
		text = ::Util.replace(text, "\n", "<br>");
		text = ::Util.replace(text, "^N", "<br>");
		text = ResolvePostTags(text);
		return text;
	}
	function ServerToBreak(text)
	{
		text = ::Util.replace(text, "^N", "\n");
		return text;
	}
	function ServerToFullBreak(text)
	{
		//Assume text uses bracket tags [b], [i], etc.
		//Expand to full newlines which are needed for proper clipboard copying.
		local lines = Util.split(text, "\n");
		text = "";
		foreach(i, d in lines)
		{
			if(i > 0)
				text += "\r\n";
			text += d;
		}
		text = ::Util.replace(text, "^N", "\r\n");
		//text = ResolvePostTags(text);
		return text;
	}
	function RemoveAngleBrackets(text)
	{
		return text;
		text = ::Util.replace(text, "<", "");
		text = ::Util.replace(text, ">", "");
		return text;
	}

class Screens.IGForum extends GUI.Frame
{
	static mClassName = "Screens.IGForum";

	static ROOT_LABEL = "Main Index";
	static TYPE_CATEGORY = 0;  //Matches server values
	static TYPE_THREAD = 1;
	static POST_DISPLAY_COUNT = 5;

	//These POST variables should match the server.
	//They are used when sending edited posts to the server, so the server
	//knows what to do with the information.
	static POST_THREAD = 0;
	static POST_REPLY  = 1;
	static POST_EDIT   = 2;

	mInputAreaModerate = null;
	mDropDownModerate = null;
	mButtonModerate = null;

	mLabelIndex = null;
	mLabelThreadInfo = null;
	mLabelThreadAge = null;
	mLabelThreadBasicInfo = null;
	mButtonNewThread = null;
	mButtonNewPost = null;
	mColumnListIndex = null;
	mButtonExpand = null;
	mButtonRoot = null;
	mButtonFirstPage = null;
	mButtonPrevPage = null;
	mButtonNextPage = null;
	mButtonLastPage = null;
	mButtonPostHome = null;
	mButtonPostEnd = null;
	mScrollPanelPostArea = null;
	mHTML = null;
	mQueriedCategories = {};
	mColumnListPostArea = [];
	mPostData = {};
	mScrollbar = null;

	static ROOT_CATEGORY = 1;
	mCategoryViewID = 1;  //Default root category is 1.
	mCategoryViewTitle = "";
	mLastQueryType = 0;

	mPostViewCurrentThreadID = 0;
	mPostViewCurrentThreadTitle = "";
	mPostViewCurrentIndex = 0;   //Stores the navigation location in the post view.
	mPostViewMaximumIndex = 0;   //Stores the number of posts returned in the thread view.

	mLastOpenedCategory = null;   //Contains the object information for the category/thread that was opened in the navigation menu.

	mSelectSource = null;
	mSelectDestination = null;

	static ModChoices = [
		"Edit Post",
		"Delete Post",
		"Copy Posts to Clipboard",
		"New Category",
		"Rename Thread/Category",
		"Delete Thread/Category",
		"Lock Thread/Category",
		"Unlock Thread/Category"
		"Sticky Thread",
		"Select Source Thread",
		"Select Destination Category",
		"Move Source to Destination",
		];


	constructor()
	{
		mLastQueryType = TYPE_CATEGORY;

		GUI.Frame.constructor("In-Game Forum");
		local cmain = GUI.Container(GUI.BoxLayoutV());
		//cmain.add(_buildHeaderPane());
		//cmain.add(_buildNavigationPane());
		//cmain.add(_buildNavigationDisplay());

		cmain.add(_buildInteractPane());
		cmain.add(_buildContentPane());
		setContentPane(cmain);
		setSize(700, 500);

		ResetCategoryTrack();
		FetchCategoryList(ROOT_CATEGORY);
	}
	function _buildInteractPane()
	{
		local left = GUI.Container(GUI.BoxLayoutV());
		left.setPreferredSize(280, 80);
		left.setSize(280, 80);
		left.add(_buildLeft1());
		left.add(_buildLeft2());
		left.add(_buildLeft3());
		left.add(_buildLeft4());

		local right = GUI.Container(GUI.BoxLayoutV());
		right.setPreferredSize(380, 80);
		right.setSize(380, 80);
		right.add(_buildRight1());
		right.add(_buildRight2());
		right.add(_buildRight3());
		right.add(_buildRight4());

		local container = GUI.Container();
		container.add(left);
		container.add(right);
		return container;
	}
	function _buildLeft1()
	{
		mButtonNewThread = _createButton("New Thread");
		mButtonNewPost = _createButton("Reply to Thread");

		local container = GUI.Container();
		container.setPreferredSize(280, 24);
		container.add(mButtonNewThread);
		container.add(mButtonNewPost);
		return container;
	}
	function _buildLeft2()
	{
		mButtonExpand = _createButton("Open Category/Thread");
		mButtonRoot = _createButton("Back to Index");

		local container = GUI.Container();
		container.setPreferredSize(280, 24);
		container.add(mButtonExpand);
		container.add(mButtonRoot);
		return container;
	}

	function _buildLeft3()
	{
		mLabelThreadBasicInfo = GUI.Label("");
		local container = GUI.Container();
		container.setPreferredSize(380, 16);
		container.add(mLabelThreadBasicInfo);
		return container;
	}
	function _buildLeft4()
	{
		mLabelIndex = GUI.Label(ROOT_LABEL);
		local container = GUI.Container();
		container.setPreferredSize(280, 16);
		container.add(GUI.Label("Category:  "));
		container.add(mLabelIndex);
		return container;
	}
	function _buildRight1()
	{
		mDropDownModerate = GUI.DropDownList();
		foreach(i, d in ModChoices)
		{
			mDropDownModerate.addChoice(d);
		}
		mInputAreaModerate = GUI.InputArea("");
		mInputAreaModerate.setMaxCharacters(4);
		mInputAreaModerate.setAllowOnlyNumbers(true);
		mInputAreaModerate.setWidth(40);
		mInputAreaModerate.setText("1");

		mButtonModerate = _createButton("Run Action");

		local container = GUI.Container();
		container.setPreferredSize(380, 24);
		container.add(mDropDownModerate);
		container.add(GUI.Label("Post #"));
		container.add(mInputAreaModerate);
		container.add(mButtonModerate);
		return container;
	}

	function _buildRight2()
	{
		mButtonFirstPage = _createButton("First Page");
		mButtonPrevPage = _createButton("Previous");
		mButtonNextPage = _createButton("Next");
		mButtonLastPage = _createButton("Last Page");
		mButtonPostHome = _createButton("Home");
		mButtonPostEnd = _createButton("End");

		local container = GUI.Container();
		container.setPreferredSize(380, 24);
		container.add(mButtonFirstPage);
		container.add(mButtonPrevPage);
		container.add(mButtonNextPage);
		container.add(mButtonLastPage);
		container.add(mButtonPostHome);
		container.add(mButtonPostEnd);
		return container;
	}

	function _buildRight3()
	{
		mLabelThreadAge = GUI.Label("");
		local container = GUI.Container();
		container.setPreferredSize(380, 16);
		container.add(mLabelThreadAge);
		return container;
	}
	function _buildRight4()
	{
		mLabelThreadInfo = GUI.Label("No Thread Opened");

		local container = GUI.Container();
		container.setPreferredSize(380, 16);
		container.add(mLabelThreadInfo);
		return container;
	}

	function _buildHeaderPane()
	{
		local container = GUI.Container();
		container.setPreferredSize(680, 24);

		//mButtonNewCategory = _createButton("New Category");
		mButtonNewThread = _createButton("New Thread");
		mButtonNewPost = _createButton("New Post");

		mDropDownModerate = GUI.DropDownList();
		foreach(i, d in ModChoices)
		{
			mDropDownModerate.addChoice(d);
		}
		mInputAreaModerate = GUI.InputArea("");
		mInputAreaModerate.setMaxCharacters(4);
		mInputAreaModerate.setAllowOnlyNumbers(true);
		mInputAreaModerate.setWidth(40);

		mButtonModerate = _createButton("Run Action");

		//container.add(mButtonNewCategory);
		container.add(mButtonNewThread);
		container.add(mButtonNewPost);
		container.add(GUI.Spacer(50, 5));
		container.add(mDropDownModerate);
		container.add(GUI.Label("Post #"));
		container.add(mInputAreaModerate);
		container.add(mButtonModerate);

		return container;
	}

	function _buildNavigationPane()
	{
		local container = GUI.Container();
		mButtonExpand = _createButton("Open Category/Thread");
		mButtonRoot = _createButton("Back to Index");

		mButtonFirstPage = _createButton("First Page");
		mButtonPrevPage = _createButton("Previous");
		mButtonNextPage = _createButton("Next");
		mButtonLastPage = _createButton("Last Page");

		container.setPreferredSize(700, 24);

		container.add(mButtonExpand);
		container.add(mButtonRoot);
		container.add(GUI.Spacer(50, 5));
		container.add(mButtonFirstPage);
		container.add(mButtonPrevPage);
		container.add(mButtonNextPage);
		container.add(mButtonLastPage);
		return container;
	}

	function _buildNavigationDisplay()
	{
		mLabelIndex = GUI.Label(ROOT_LABEL);
		mLabelThreadInfo = GUI.Label("No Thread Opened");

		local container = GUI.Container();
		container.setPreferredSize(500, 16);
		container.add(GUI.Label("Location:  "));
		container.add(mLabelIndex);
		container.add(GUI.Spacer(50, 5));
		container.add(mLabelThreadInfo);
		return container;
	}
	function _buildContentPane()
	{
		local container = GUI.Container();
		mColumnListIndex = GUI.ColumnList();
		mColumnListIndex.setPreferredSize(258, 360);  //Was 350 wide
		mColumnListIndex.setWindowSize(100);
		mColumnListIndex.addColumn("Category / Thread Navigation", 184);
		mColumnListIndex.addColumn("Last", 45);  //was 48
		mColumnListIndex.addActionListener(this);

		local ctarea = GUI.ScrollPanel(mColumnListIndex);
		ctarea.setPreferredSize(258, 360);
		container.add(ctarea);

		mScrollPanelPostArea = GUI.ScrollPanel(_sub_buildPostPane());
		mScrollPanelPostArea.setPreferredSize(420, 360);  //Was 390 wide
		//mScrollPanelPostArea.setAppearance("PaperBackBorder");
		mScrollPanelPostArea.setAppearance("GoldBorder");

		//container.add(mScrollPanelPostArea);
		container.add(mScrollPanelPostArea);
		return container;
	}

	function _sub_buildPostPane()
	{
		mHTML = GUI.HTML2();
		mHTML.setMaximumSize(380, 360);  //was 360 wide
		mHTML.setResize( true );
		mHTML.setText("");
		mHTML.addActionListener(this);
		mHTML.setLinkClickedMessage("onLinkClicked2");
		mHTML.setInsets(8, 8, 8, 8);

		local width = mHTML.getSize().width - 28;
		local font = mHTML.getFont();
		mHTML.setWrapText(true, font, width);

		return mHTML;
	}
	function onDoubleClick(list, evt)
	{
		if(list == mColumnListIndex)
		{
			OpenSelectedCategory();
		}
	}
	function onRowSelectionChanged(list, index, selected)
	{
		if(list == mColumnListIndex)
			UpdateSelectedCategory();
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
		if(button == mButtonNewThread)
			ComposeNewThread();
		else if(button == mButtonNewPost)
			ComposeNewPost();
		else if(button == mButtonExpand)
			OpenSelectedCategory();
		else if(button == mButtonRoot)
			GoRoot();
		else if(button == mButtonPrevPage)
			BrowsePrevPage();
		else if(button == mButtonNextPage)
			BrowseNextPage();
		else if(button == mButtonFirstPage)
			BrowseFirstPage();
		else if(button == mButtonLastPage)
			BrowseLastPage(true);
		else if(button == mButtonModerate)
			PerformEditAction();
		else if(button == mButtonPostHome)
			BrowseHome();
		else if(button == mButtonPostEnd)
			BrowseEnd();
	}
	function SubmitOperation(objectType, parentID, renameID, objectName)
	{
		::_Connection.sendQuery("mod.igforum.editobject", this, [objectType, parentID, renameID, objectName] );
	}
	function GetQueriedCategory()
	{
		local rows = mColumnListIndex.getSelectedRows();
		if(rows.len() == 0)
			return null;
		local index = rows[0].tointeger();
		if(!(index in mQueriedCategories))
			return null;

		return mQueriedCategories[index];
	}
	function onQueryError(qa, error)
	{
		IGIS.error(error);
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.igforum.getcategory")
		{
			ProcessCategoryList(results);
			RefreshCategoryList();
		}
		else if(qa.query == "mod.igforum.opencategory")
		{
			UpdateDirectoryLabel(null);
			ProcessCategoryList(results);
			RefreshCategoryList();
		}
		else if(qa.query == "mod.igforum.sendpost")
		{
			GUI.MessageBox.show("Post successful.");
			local scr = Screens.get("IGForumPost", false);
			if(scr)
				scr.EraseData();

			FetchCategoryList(mCategoryViewID);

			if(qa.args[0].tointeger() == POST_EDIT)
			{
				//Refresh the current page
				QueryOpenThread(mPostViewCurrentThreadID, mPostViewCurrentIndex);
			}
			else
			{
				//Simulate a new post so that the last page can be properly calculated.
				mPostViewMaximumIndex++;
				BrowseLastPage(true);
			}
		}
		else if(qa.query == "mod.igforum.openthread")
		{
			ProcessThreadContents(results);
		}
		else if(qa.query == "mod.igforum.deletepost")
		{
			QueryOpenThread(mPostViewCurrentThreadID, mPostViewCurrentIndex);
		}
		else if(qa.query == "mod.igforum.setlockstatus")
		{
			FetchCategoryList(mCategoryViewID);
		}
		else if(qa.query == "mod.igforum.setstickystatus")
		{
			FetchCategoryList(mCategoryViewID);
		}
		else if(qa.query == "mod.igforum.deleteobject")
		{
			FetchCategoryList(mCategoryViewID);
		}
		else if(qa.query == "mod.igforum.editobject")
		{
			local scr = Screens.get("IGForumCategory", false);
			if(scr)
				scr.EraseData();

			FetchCategoryList(mCategoryViewID);
		}
	}
	function onQueryTimeout(qa)
	{
	}
	function SubmitPost(postType, placementID, postID, threadName, postBody)
	{
		::_Connection.sendQuery("mod.igforum.sendpost", this, [postType, placementID, postID, threadName, postBody] );
	}

	//Fetch the contents of a category from the server.
	function FetchCategoryList(categoryID)
	{
		::_Connection.sendQuery("mod.igforum.getcategory", this, [categoryID] );
	}

	//Process the category list sent back from the server.  Enter them into the cache.
	function ProcessCategoryList(results)
	{
		mLastQueryType = TYPE_CATEGORY;
		mQueriedCategories.clear();
		if(results.len() == 0)
			return;
		foreach(i, r in results)
		{
			if(i == 0)
			{
				local ID = r[0].tointeger();
				local title = r[1];

				mCategoryViewID = ID;
				mCategoryViewTitle = title;
			}
			else
			{
				local type = r[0].tointeger();
				local id = r[1].tointeger();
				local locked = r[2].tointeger();
				local stickied = r[3].tointeger();
				local name = r[4];
				local entries = r[5].tointeger();
				local lastUpdate = r[6].tointeger();
				mQueriedCategories[i - 1] <- {type = type, id = id, name = name, locked = locked, stickied = stickied, entries = entries, lastupdate = lastUpdate};
			}
		}
	}

	//Refresh the category list from the cached data.
	function RefreshCategoryList()
	{
		mColumnListIndex.removeAllRows();
		local visName = "";
		foreach(i, d in mQueriedCategories)
		{
			if(d.type == TYPE_CATEGORY)
				visName = "< " + d.name + " >";
			else
				visName = d.name;

			/*
			if(d.type == TYPE_THREAD)
				visName += " (" + d.entries + "-" + GetAgeString(d.lastupdate, true) + ")";
			//visName += " (" + GetAgeString(d.lastupdate, true) + ")";
			else if(d.type == TYPE_CATEGORY)
			{
				if(d.lastupdate != 0)
					visName += " (" + d.entries + "-" + GetAgeString(d.lastupdate, true) + ")";
				else
					visName += " (" + d.entries + ")";
			}
			*/
			local last = d.entries + ", " + GetAgeString(d.lastupdate, 2);

			if(d.locked != 0)
				visName += " [L]";
			if(d.stickied != 0)
				visName = "[S] " + visName;

			mColumnListIndex.addRow([visName, last]);
		}
	}
	function ComposeNewCategory()
	{
		Screens.show("IGForumCategory");
	}
	function ComposeNewThread()
	{
		local scr = Screens.get("IGForumPost", true);
		if(scr)
		{
			scr.SetPostParams(POST_THREAD, mCategoryViewID, 0);
			scr.mInputAreaThreadName.setAllowTextEntryOnClick(true);
			scr.mInputAreaThreadName.setText("");
		}
		Screens.show("IGForumPost");
	}

	function ComposeNewPost()
	{
		if(mPostViewCurrentThreadID == 0)
		{
			GUI.MessageBox.show("You must open a thread before you can reply.");
			return;
		}

		local scr = Screens.get("IGForumPost", true);
		if(scr)
		{
			scr.SetPostParams(POST_REPLY, mPostViewCurrentThreadID, 0)
			scr.mInputAreaThreadName.setAllowTextEntryOnClick(false);
			local text = "RE: " + mPostViewCurrentThreadTitle;
			scr.mInputAreaThreadName.setText(text);
		}
		Screens.show("IGForumPost");
	}
	function OpenSelectedCategory()
	{
		local category = GetQueriedCategory();
		if(category == null)
		{
			IGIS.info("You must select a category to expand.");
			return;
		}
		mLastOpenedCategory = category;
		if(category.type == TYPE_CATEGORY)
		{
			::_Connection.sendQuery("mod.igforum.opencategory", this, [category.type, category.id] );
		}
		else
		{
			QueryOpenThread(category.id, 0);
		}
	}
	function GoRoot()
	{
		ResetCategoryTrack();
		FetchCategoryList(ROOT_CATEGORY);
	}
	function ResetCategoryTrack()
	{
		UpdateDirectoryLabel("Main Index");
	}
	function UpdateDirectoryLabel(text)
	{
		if(text == null)
		{
			text = mLastOpenedCategory.name;
			if(mLastOpenedCategory.locked != 0)
				text += " [LOCKED]"
		}
		mLabelIndex.setText(text);
	}
	function ProcessThreadContents(results)
	{
		mLastQueryType = TYPE_THREAD;
		mPostData.clear();
		foreach(i, d in results)
		{
			if(i == 0)
			{
				local threadID = d[0].tointeger();
				local threadTitle = d[1];
				local postStart = d[2].tointeger();
				local postCount = d[3].tointeger();
				local threadAgeMinutes = d[4].tointeger();
				UpdatePostNavigation(threadID, threadTitle, postStart, postCount, threadAgeMinutes);
			}
			else
			{
				local id = d[0].tointeger();
				local userName = d[1];
				local timeStamp = d[2];
				local postAgeMinutes = d[3].tointeger();
				local postBody = d[4];
				local editCount = d[5].tointeger();
				local editTime = d[6].tointeger();
				mPostData[i - 1] <- {id = id, user = userName, time = timeStamp, post = postBody, postnum = (mPostViewCurrentIndex + i), age = postAgeMinutes, editCount = editCount, editTime = editTime};
			}
		}
		RefreshPostDisplay();
	}
	function RefreshPostDisplay()
	{
		local text = "<font size=\"20\" color=\"FFFFFF\">";

		if(mPostData.len() > 0)
		{
			foreach(i, d in mPostData)
			{
				local postText = ServerToHTML(d.post);
				local postLink = "<a href=\"post:" + d.postnum + "\">#" + d.postnum + "</a>";
				text += d.user + " [Post" + postLink + "] (" + d.time + ") " + GetAgeString(d.age, false) + "<br>";
				if(d.editCount > 0)
				{
					local editLine =  "<font color=\"EDED8F\" size=\"16\">Edited: ";
					editLine += d.editCount + " time";
					if(d.editCount != 1)
						editLine += "s";
					editLine += ", Last update: " + GetAgeString(d.editTime, false);
					editLine += "<br><br></font>";
					text += editLine;
				}
				text += postText + "<br><br>";
				text += "<font color=\"EDED8F\">------------------------------</font><br>";
			}
		}
		text += "</font>";
		mHTML.setText(text);
		mScrollPanelPostArea.setIndex(0);
	}
	function GetAgeString(minutes, short)
	{
		local timeVal = 0;
		local timeStr = "";
		if(minutes < 60)
		{
			timeVal = minutes;
			timeStr = short ? "m" : " minute";
		}
		else if(minutes < 1440)
		{
			timeVal = minutes / 60;
			timeStr = short ? "h" : " hour";
		}
		else
		{
			timeVal = minutes / 1440;
			timeStr = short ? "d" : " day";
		}
		local retString = timeVal.tostring() + timeStr;
		if(timeVal != 1 && short == false)
			retString += "s";
		if(short.tointeger() != 2)
			retString += " ago";
		return retString;
	}
	function UpdatePostNavigation(threadID, threadTitle, postStart, postCount, threadAgeMinutes)
	{
		mPostViewCurrentThreadID = threadID;
		mPostViewCurrentThreadTitle = threadTitle;
		mPostViewCurrentIndex = postStart;
		mPostViewMaximumIndex = postCount;

		local threadText = threadTitle + " (ID#" + threadID + ") ";
		local postText = "with " + postCount + ((postCount != 1) ? " posts." : " post.");
		local postEnd = postStart + POST_DISPLAY_COUNT;
		if(postEnd > postCount)
			postEnd = postCount;
		local viewText = "   Viewing: " + (postStart + 1) + " to " + postEnd;
		mLabelThreadInfo.setText(threadText + postText + viewText);
		mLabelThreadAge.setText("Latest post: " + GetAgeString(threadAgeMinutes, false));
	}
	function BrowsePrevPage()
	{
		local newIndex = mPostViewCurrentIndex - POST_DISPLAY_COUNT;
		if(newIndex < 0)
			newIndex = 0;

		if(newIndex != mPostViewCurrentIndex)
			QueryOpenThread(mPostViewCurrentThreadID, newIndex);
	}
	function BrowseNextPage()
	{
		local newIndex = mPostViewCurrentIndex + POST_DISPLAY_COUNT;
		if(newIndex < mPostViewMaximumIndex)
			QueryOpenThread(mPostViewCurrentThreadID, newIndex);
	}
	function BrowseFirstPage()
	{
		if(mPostViewCurrentIndex != 0)
			QueryOpenThread(mPostViewCurrentThreadID, 0);
	}
	function BrowseLastPage(force)
	{
		local newIndex = mPostViewMaximumIndex - (mPostViewMaximumIndex % POST_DISPLAY_COUNT);
		if(newIndex != mPostViewCurrentIndex || force == true)
			QueryOpenThread(mPostViewCurrentThreadID, newIndex);
	}
	function BrowseHome()
	{
		mScrollPanelPostArea.setIndex(0);
		mScrollPanelPostArea.mAttachParent.invalidate();
		//mScrollPanelPostArea.onEnterFrame();
	}
	function BrowseEnd()
	{
		local rows = mScrollPanelPostArea.mAttachParent.getLayoutManager().getRows();
		local newRow = rows.len() - 2 - mScrollPanelPostArea.mPageSize;
		if(newRow < 0)
			newRow = 0;
		mScrollPanelPostArea.setIndex(newRow);
		mScrollPanelPostArea.mAttachParent.invalidate();
		//mScrollPanelPostArea.onEnterFrame();
	}
	function QueryOpenThread(threadID, postStart)
	{
		::_Connection.sendQuery("mod.igforum.openthread", this, [threadID, postStart, POST_DISPLAY_COUNT] );
	}
	function GetSelectedThread(prompt)
	{
		if(mPostViewCurrentThreadID == 0)
		{
			if(prompt == true)
				GUI.MessageBox.show("You must select a thread.");
		}
		return mPostViewCurrentThreadID;
	}
	function TruncateBreaks(text, limit, breakText)
	{
		local retstr = "";
		local count = 0;
		local arr = ::Util.split(text, breakText);
		foreach(i, d in arr)
		{
			retstr += (d + breakText);
			if(count++ >= limit)
				break;
		}
		return retstr
	}

	function GetPostByNumber(number)
	{
		foreach(i, d in mPostData)
		{
			if(d.postnum == number)
				return d;
		}
		return null;
	}
	function PromptMissingPost(ID)
	{
		GUI.MessageBox.show("Post #" + ID + " is not visible in the display panel. Change the number or navigate to the correct page.");
	}
	function PerformEditAction()
	{
		local postNum = mInputAreaModerate.getValue();
		local action = mDropDownModerate.getCurrent();
		if(action == "Delete Post")
		{
			local threadID = GetSelectedThread(true);
			if(threadID > 0)
			{
				local post = GetPostByNumber(postNum);
				if(post == null)
				{
					PromptMissingPost(postNum);
					return;
				}

				local postUser = post.user;
				local postText = post.post;
				if(postText.len() > 200)
				{
					postText = postText.slice(0, 200);
					postText += "[...]";
				}
				postText = ServerToHTML(postText);
				//If the post has too many line breaks, it won't be possible to see
				//the message box button. Truncate after a certain number of breaks.
				postText = TruncateBreaks(postText, 3, "<br>");

				local text = "Are you sure you want to delete this post:" +
					"<br>Author: " + postUser +
					"<br>Post: " + postText;

				local callback =
				{
					threadID = threadID,
					ID = post.id,
					handler = this,
					function onActionSelected(mb, alt)
					{
						if( alt == "Yes" )
							::_Connection.sendQuery("mod.igforum.deletepost", handler, [threadID, ID] );
					}
				};
				GUI.MessageBox.showYesNo(text, callback);
			}
		}
		else if(action == "Rename Thread/Category")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			BeginEditObject(cat.type, 0, cat.id, cat.name);
		}
		else if(action == "Delete Thread/Category")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			local stype = cat.type;
			local sid = cat.id;

			local text = "Are you sure you want to delete this ";
			if(stype == TYPE_CATEGORY)
				text += "category";
			else
				text += "thread";

			text += ":<br>" + cat.name;

			local callback =
			{
				type = stype,
				id = sid,
				handler = this,
				function onActionSelected(mb, alt)
				{
					if( alt == "Yes" )
						::_Connection.sendQuery("mod.igforum.deleteobject", handler, [type, id] );
				}
			};
			GUI.MessageBox.showYesNo(text, callback);
		}
		else if(action == "Copy Posts to Clipboard")
		{
			local text = "";
			foreach(i, d in mPostData)
			{
				local postText = ServerToFullBreak(d.post);
				text += d.user + " [Post#" + (i + 1) + "] (" + d.time + ")" + "\r\n" + postText + "\r\n\r\n";
			}
			if(text.len() > 0)
			{
				::System.setClipboard(text);
				IGIS.info("Copied visible posts to clipboard.");
			}
			else
				IGIS.error("Nothing copied.");
		}
		else if(action == "Lock Thread/Category")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			::_Connection.sendQuery("mod.igforum.setlockstatus", this, [cat.type, cat.id, 1] );
		}
		else if(action == "Unlock Thread/Category")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			::_Connection.sendQuery("mod.igforum.setlockstatus", this, [cat.type, cat.id, 0] );
		}
		else if(action == "Edit Post")
		{
			local threadID = GetSelectedThread(true);
			if(threadID <= 0)
				return;

			local post = GetPostByNumber(postNum);
			if(post == null)
			{
				PromptMissingPost(postNum);
				return;
			}

			local scr = Screens.get("IGForumPost", true);
			if(scr)
			{
				scr.SetPostParams(POST_EDIT, mPostViewCurrentThreadID, post.id)
				scr.mInputAreaThreadName.setAllowTextEntryOnClick(false);
				local threadTitle = "EDITING - RE: " + mPostViewCurrentThreadTitle;
				local postBody = ServerToBreak(post.post);
				scr.mInputAreaThreadName.setText(threadTitle);
				scr.mInputAreaPostBody.setText(postBody);
			}
			Screens.show("IGForumPost");
		}
		else if(action == "New Category")
			BeginEditObject(TYPE_CATEGORY, mCategoryViewID, 0, "");
		else if(action == "Sticky Thread")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			local status = (postNum == 0) ? 0 : 1;
			::_Connection.sendQuery("mod.igforum.setstickystatus", this, [cat.type, cat.id, status] );
		}
		else if(action == "Select Source Thread")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			mSelectSource = cat;
			IGIS.info("Source: " + cat.name);
		}
		else if(action == "Select Destination Category")
		{
			local cat = GetQueriedCategory();
			if(cat == null)
				return;
			mSelectDestination = cat;
			IGIS.info("Destination: " + cat.name);
		}
		else if(action == "Move Source to Destination")
		{
			if(mSelectSource == null)
			{
				IGIS.info("Source not selected.");
				return;
			}
			if(mSelectDestination == null)
			{
				IGIS.info("Destination not selected.");
				return;
			}
			local callback =
			{
				src = mSelectSource,
				dst = mSelectDestination,
				handler = this,
				function onActionSelected(mb, alt)
				{
					if( alt == "Yes" )
						::_Connection.sendQuery("mod.igforum.move", handler, [src.type, src.id, dst.type, dst.id] );
				}
			};
			local text = "Are you sure you want to move:<br>" + mSelectSource.name + "<br>To:<br>" + mSelectDestination.name;
			GUI.MessageBox.showYesNo(text, callback);
			mSelectSource = null;
			mSelectDestination = null;
		}
	}
	function BeginEditObject(type, parentID, renameID, existValue)
	{
		Screens.show("IGForumCategory");
		local scr = Screens.get("IGForumCategory", false);
		if(!scr)
			return;
		scr.SetData(type, parentID, renameID, existValue);
	}
	function SetSelectedPost(postID)
	{
		mInputAreaModerate.setText(postID.tostring());
	}
	function UpdateSelectedCategory()
	{
		local cat = GetQueriedCategory();
		if(cat == null)
			return;

		local text = "" + cat.entries;
		if(cat.type == TYPE_THREAD)
			text += " post";
		else if(cat.type == TYPE_CATEGORY)
			text += " thread";
		if(cat.entries != 1)
			text += "s";

		text += ".  Last updated: ";
		if(cat.type == TYPE_THREAD || cat.lastupdate != 0)
			text += GetAgeString(cat.lastupdate, false);
		else
			text += "unknown";

		mLabelThreadBasicInfo.setText(text);
	}
}


// Class for composing and editing posts.
class Screens.IGForumPost extends GUI.Frame
{
	static mClassName = "Screens.IGForumPost";
	static MAXIMUM_POST_LENGTH = 4000;

	mInputAreaPostBody = null;
	mButtonPost = null;
	mButtonPostPreview = null;
	mButtonPostCopy = null;
	mInputAreaThreadName = null;

	mPostType = 0;    //Which type of edit this post is.  Used to communicate back to the forum manager and server for how to add this post.
	mPostPlacementID= 0;  //If replying, this is the Thread ID.
	mPostID = 0;      //If editing an existing post, this is the Post ID.


	/* FOR ITEM LINK */
	mItem = null;
	mItemID = 0;
	mInputAreaLink = null;
	mButtonCopy = null;
	mButtonInsert = null;
	mButtonHelp = null;

	mButtonHelp = null;

	constructor()
	{
		GUI.Frame.constructor("Edit Post");
		local cmain = GUI.Container(GUI.BoxLayoutV());

		cmain.add(_buildThreadBar());
		cmain.add(GUI.Label("Post limit is " + MAXIMUM_POST_LENGTH + " characters."));
		//cmain.add(GUI.Label("Accepted formatting tags (must be lowercase): [b]bold[/b]     [i]italic[/i]     [link]http://example.com[/link]"));
		cmain.add(_buildItemLink());
		cmain.add(_buildInputArea());
		cmain.add(_buildButtonRow());
		setContentPane(cmain);
		setSize(600, 430);
	}
	function _buildThreadBar()
	{
		local container = GUI.Container(GUI.GridLayout(1,2));
		container.getLayoutManager().setColumns(100, 400);

		mInputAreaThreadName = GUI.InputArea("");
		mInputAreaThreadName.setMaxCharacters(32);
		container.add(GUI.Label("Thread Title:"));
		container.add(mInputAreaThreadName);
		return container;
	}
	function _buildItemLink()
	{
		/*
		local iconCont = GUI.Container();
		iconCont.setSize(32, 32);
		iconCont.setPreferredSize(32, 32);
		iconCont.setPosition( 26, 40 );
		iconCont.setAppearance(null);
		*/

		mItem = GUI.ActionContainer("quickbar", 1, 1, 0, 0, this, false);
		mItem.setSize(32, 32);
		mItem.setPreferredSize(32, 32);
		mItem.setPosition( 26, 40 );
		mItem.setAppearance(null);

		mItem.setItemPanelVisible(false);
		mItem.setValidDropContainer(true);
		mItem.setAllowButtonDisownership(false);
		mItem.setSlotDraggable(false, 0);
		mItem.addListener(this);

		local size = mItem.getPreferredSize();
		mItem.setSize(size.width, size.height);
		mItem.setPosition( 18, 18 );
		mItem.addAcceptingFromProperties("inventory", AcceptFromProperties(this));

		mInputAreaLink = GUI.InputArea("");
		mInputAreaLink.setWidth(300);

		mButtonCopy = GUI.Button("Copy");
		mButtonCopy.addActionListener(this);
		mButtonCopy.setReleaseMessage("onButtonPressed");
		mButtonCopy.setTooltip("Copy the link tag to the clipboard.");

		mButtonInsert = GUI.Button("Insert");
		mButtonInsert.addActionListener(this);
		mButtonInsert.setReleaseMessage("onButtonPressed");
		mButtonInsert.setTooltip("Insert the Link tag into the post at the last known cursor position.");

		local container = GUI.Container();
		container.setPreferredSize(580, 38);
		container.add(GUI.Label("Item link:"));
		container.add(mButtonCopy);
		container.add(mButtonInsert);
		container.add(mInputAreaLink);
		container.add(mItem);
		container.add(GUI.Spacer(10, 0));
		return container;
	}
	function _buildInputArea()
	{
		local container = GUI.Container(GUI.GridLayout(1,1));
		container.getLayoutManager().setColumns(560);
		container.getLayoutManager().setRows(290);

		mInputAreaPostBody = ::GUI.InputArea( "" );
		mInputAreaPostBody.setMultiLine( true );
		mInputAreaPostBody.setPreferredSize(560, 290);
		mInputAreaPostBody.setMaxCharacters(MAXIMUM_POST_LENGTH);

		container.add(mInputAreaPostBody);
		return container;
	}
	function _buildButtonRow()
	{
		local container = GUI.Container();
		mButtonPost = _createButton("Submit Post");
		mButtonPostPreview = _createButton("Preview Post");
		mButtonPostCopy = _createButton("Copy Post to Clipboard");
		mButtonHelp = _createButton("Show Tag Help");

		container.add(mButtonPost);
		container.add(mButtonPostPreview);
		container.add(mButtonPostCopy);
		container.add(GUI.Spacer(20, 0));
		container.add(mButtonHelp);
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
		if(button == mButtonPost)
			SubmitPost();
		else if(button == mButtonPostPreview)
			PreviewPost();
		else if(button == mButtonPostCopy)
			CopyPost();
		else if(button == mButtonCopy)
			CopyItemLink();
		else if(button == mButtonInsert)
			InsertItemLink();
		else if(button == mButtonHelp)
			PopupHelp();
	}
	function SubmitPost()
	{
		local title = mInputAreaThreadName.getText();
		local text = mInputAreaPostBody.getText();
		if(title.len() == 0)
		{
			GUI.MessageBox.show("You must enter a title.");
			return;
		}
		if(text.len() == 0)
		{
			GUI.MessageBox.show("You must enter a post.");
			return;
		}
		else if(text.len() > MAXIMUM_POST_LENGTH)
		{
			GUI.MessageBox.show("The post is too long to submit: " + text.len() + " characters, maximum is " + MAXIMUM_POST_LENGTH + ".");
			return;
		}

		Screens.hide("IGForumPost");
		local scr = Screens.get("IGForum", false);
		if(scr)
			scr.SubmitPost(mPostType, mPostPlacementID, mPostID, title, text);
	}
	function SetPostParams(postType, placementID, postID)
	{
		mPostType = postType;
		mPostPlacementID = placementID;
		mPostID = postID;
	}
	function EraseData()
	{
		mInputAreaPostBody.setText("");
		mInputAreaThreadName.setText("");
		SetPostParams(0, 0, 0);
	}

	/* FOR ITEM LINK */
	function onValidDropSlot(newSlot, oldSlot)
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		local itemData = action.mItemData;

		local itemID = itemData.mItemDefId;
		//if(itemData.mItemLookDefId > 0)
			//itemID = itemData.mItemLookDefId;

		local itemDef = ::_ItemDataManager.getItemDef(itemID);  //was itemData.mItemDefId

		local appstr = itemDef.mID + ":" + itemDef.mDisplayName;
		if(itemData.mItemLookDefId > 0)
			appstr += ":" + itemData.mItemLookDefId;

		//mInputAreaLink.setText("[item]" + itemDef.mID + ":" + itemDef.mDisplayName + "[/item]");
		mInputAreaLink.setText("[item]" + appstr + "[/item]");
		return true;
	}
	function CopyItemLink()
	{
		local text = mInputAreaLink.getText();
		if(text.len() > 0)
			::System.setClipboard(mInputAreaLink.getText());
	}
	function InsertItemLink()
	{
		local text = mInputAreaLink.getText() + "\n";
		if(text.len() > 0)
			mInputAreaPostBody.insertText(text);
	}
	/* END ITEM LINK */

	function PopupHelp()
	{
		local text = "<font size=\"24\">";
		text += "Accepted formatting tags (must be lowercase):<br>";
		text += "   [b]bold[/b]<br>";;
		text += "   [i]italic[/i]<br>";
		text += "   [link]http://example.com[/link]<br>";
		text += "   [url=http://example.com]Example Site[/url]<br>";
		text += "   [color=00ff00]Hexadecimal RGB coloring[/color]<br>";
		text += "   [grove]warpname:Description:x,z[/grove]<br>";
		text += "       Warps to a grove (restrictions apply).  The warpname must be a<br>";
		text += "       fully typed grove name.  Description is optional, and appears in the<br>";
		text += "       link text.  Coordinates are also optional.<br>";
		text += "       Example: <font color=\"00FFFF\">[grove]blahblah1:My Grove!:2000,1000[/grove]</font><br>";
		text += "   [copy=Shorthand Text]blah blah blah ...[/copy]<br>";
		text += "       A link that copies text to the clipboard when clicked.<br><br>";
		text += "Item Linking:<br>";
		text += "Drag an item into the slot.  A special link tag will be generated in the input box.  ";
		text += "The Copy button will place the tag onto the clipboard so you can paste it wherever ";
		text += "you need it in the post.  Or you can click Insert to place the tag at the last known ";
		text += "cursor position in the post window.";
		text += "</font>";
		GUI.MessageBox.show(text);
		return ;
	}
	function PreviewPost()
	{
		local scr = Screens.show("PostPreview");
		if(scr)
		{
			local title = mInputAreaThreadName.getText();
			local post = ServerToHTML(mInputAreaPostBody.getText());
			//local post = ServerToHTML(RemoveAngleBrackets(mInputAreaPostBody.getText()));
			scr.SetPreview(title, post);
		}
	}
	function CopyPost()
	{
		local text = ServerToFullBreak(mInputAreaPostBody.getText());
		::System.setClipboard(text);
	}
}

class Screens.IGForumCategory extends GUI.Frame
{
	static mClassName = "Screens.IGForumCategory";

	mInputAreaTitle = null;
	mButtonConfirm = null;
	mButtonCancel = null;
	mLabelHelper = null;

	mEditType = 0;
	mEditParentID = 0;
	mEditRenameID = 0;

	constructor()
	{
		GUI.Frame.constructor("Create Object");
		local cmain = GUI.Container(GUI.BoxLayoutV());

		cmain.add(_buildInputArea());
		cmain.add(_buildButtonRow());
		setContentPane(cmain);
		setSize(400, 160);
	}
	function _buildInputArea()
	{
		local container = GUI.Container(GUI.GridLayout(2,1));
		container.getLayoutManager().setColumns(350);

		mLabelHelper = GUI.Label("Enter a name for the new category:");
		mInputAreaTitle = ::GUI.InputArea("");
		mInputAreaTitle.setMaxCharacters(32);
		container.add(mLabelHelper);
		container.add(mInputAreaTitle);
		return container;
	}
	function _buildButtonRow()
	{
		local container = GUI.Container();
		mButtonConfirm = Screens.IGForum._createButton("Post");
		mButtonCancel = Screens.IGForum._createButton("Cancel");
		container.add(mButtonConfirm);
		container.add(mButtonCancel);
		return container;
	}
	function onButtonPressed(button)
	{
		if(button == mButtonCancel)
			Screens.hide("IGForumCategory");
		else if(button == mButtonConfirm)
			SubmitOperation();
	}
	function SubmitOperation()
	{
		local text = "" + mInputAreaTitle.getText();
		if(text.len() == 0)
		{
			GUI.MessageBox.show("You must enter a name.");
			return;
		}
		local scr = Screens.get("IGForum", false);
		if(scr)
			scr.SubmitOperation(mEditType, mEditParentID, mEditRenameID, mInputAreaTitle.getText());
	}

	function SetLabel(text)
	{
		mLabelHelper.setText(text);
	}

	function SetData(type, parentID, renameID, existValue)
	{
		if(type == Screens.IGForum.TYPE_CATEGORY)
			SetLabel("Enter a name for the category:");
		else
			SetLabel("Enter a name for the thread:");
		mEditType = type;
		mEditParentID = parentID;
		mEditRenameID = renameID;
		mInputAreaTitle.setText(existValue);
	}
	function EraseData()
	{
		SetData(Screens.IGForum.TYPE_CATEGORY, 0, 0, "");
	}
}

class Screens.PreviewItem extends GUI.Frame
{
	mItemDef = null;

	mIcon = null;
	mButtonPreview = null;
	mButtonPreviewOriginal = null;
	mButtonRestore = null;
	mButtonForumToggle = null;
	mButtonNudify = null;
	mItemDrag = null;

	mSetItemID = 0;
	mSetLookID = 0;

	constructor()
	{
		GUI.Frame.constructor("Item Preview");
		setSize( 200, 340 );  //was 260

		mIcon = GUI.Image();
		mIcon.setAppearance("Icon");
		mIcon.setSize(49, 50);
		mIcon.setPreferredSize(49, 50);

		mButtonPreview = _createButton("Preview");
		mButtonPreview.setFixedSize(120, 18);

		mButtonPreviewOriginal = _createButton("Preview Original");
		mButtonPreviewOriginal.setFixedSize(120, 18);
		mButtonPreviewOriginal.setTooltip("If the item has been refashioned, this allows you to see the original look.");

		mButtonNudify = _createButton("Nudify!");
		mButtonNudify.setFixedSize(120, 18);
		mButtonNudify.setTooltip("Preview your character nude. This can help to preview items that may be<br>partially covered by other sets of clothing.");

		mButtonRestore = _createButton("Restore Appearance");
		mButtonRestore.setFixedSize(120, 18);
		mButtonRestore.setTooltip("Returns your character to its normal body and equipment appearance.");

		mButtonForumToggle = _createButton("Toggle Forum Pane");
		mButtonForumToggle.setFixedSize(120, 18);


		mItemDrag = GUI.ActionContainer("itempreview", 1, 1, 0, 0, this, false);
		mItemDrag.setItemPanelVisible(false);
		mItemDrag.setValidDropContainer(true);
		mItemDrag.setAllowButtonDisownership(false);
		mItemDrag.setSlotDraggable(false, 0);
		mItemDrag.addListener(this);
		local size = mItemDrag.getPreferredSize();
		mItemDrag.setSize(size.width, size.height);
		//mItemDrag.setPosition( 18, 18 );
		mItemDrag.addAcceptingFromProperties("inventory", AcceptFromProperties(this));
		mItemDrag.addAcceptingFromProperties("auctionhouse", AcceptFromProperties(this));
		mItemDrag.addAcceptingFromProperties("vault", AcceptFromProperties(this));
		mItemDrag.addAcceptingFromProperties("deliveryBox", AcceptFromProperties(this));
		mItemDrag.addAcceptingFromProperties("item_shop", AcceptFromProperties(this));



		local container = GUI.Container(GUI.BoxLayoutV());
		//container.add(GUI.Spacer(0, 10));
		container.add(GUI.Label("Click a forum Item Link."));
		container.add(GUI.Label("Or drag an item here."));
		container.add(mItemDrag);
		container.add(GUI.Spacer(0, 10));
		container.add(GUI.Label("This item will be previewed:"));
		container.add(mIcon);
		container.add(GUI.Spacer(0, 5));

		local labelText = "Preview allows you to see what the item looks like when equipped.  It is visible on your screen only.";
		local labels = GUI.HTML();
		local font = labels.getFont();
		labels.setMaximumSize(180, 56);
		labels.setWrapText(true, font, 180);
		labels.setResize(true);
		labels.setText(labelText);

		container.add(labels);
		container.add(mButtonPreview);
		container.add(mButtonPreviewOriginal);
		container.add(mButtonNudify);
		container.add(mButtonRestore);
		container.add(mButtonForumToggle);

		setContentPane(container);
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
		if(button == mButtonPreview)
			RunPreview(false);
		else if(button == mButtonPreviewOriginal)
			RunPreview(true);
		else if(button == mButtonNudify)
			SetNudePreview();
		else if(button == mButtonRestore)
			Restore();
		else if(button == mButtonForumToggle)
			ToggleForumPane();
	}
	function onValidDropSlot(newSlot, oldSlot)
	{
		local button = oldSlot.getActionButton();
		local action = button.getAction();
		
		
		if(action instanceof ItemAction) { 
			local itemData = action.mItemData;
	
			local itemID = itemData.mItemDefId;
			local lookID = 0;
			if(itemData.mItemLookDefId > 0)
				lookID = itemData.mItemLookDefId;
	
			local itemDef = ::_ItemDataManager.getItemDef(itemID);
			SetItem(itemID, lookID);
			return true;
		}
		else if(action instanceof ItemProtoAction) {
			SetItem(action.mItemDefId, action.mLookId);
			return true;
		}
		
		return false;
	}
	function SetItem(itemID, lookID)
	{
		mSetItemID = itemID;
		mSetLookID = lookID;
		SetButtonStatus();

		//Callback when the itemdef has been fetched
		local callback =
		{
			handler = this,
			function doWork(itemDef)
			{
				handler.SetData(itemDef);
			}
		};
		local itemdef = ::_ItemDataManager.getItemDef(itemID, callback);
	}
	function SetData(itemDef)
	{
		mItemDef = itemDef;
		mIcon.setTooltip(itemDef.getTooltip(null));
		local iconArr = ::Util.split(itemDef.getIcon(), "|");
		if(iconArr.len() == 0)
		{
			mIcon.setImageName("");
		}
		if(iconArr.len() >= 1)
		{
			mIcon.setImageName(iconArr[0]);
		}
	}
	function SetButtonStatus()
	{
		mButtonPreviewOriginal.setEnabled((mSetLookID != 0));
	}
	function ToggleForumPane()
	{
		Screens.toggle("IGForum");
		Screens.toggle("PreviewItem");  //Turn off
		Screens.toggle("PreviewItem");  //Turn back on (as topmost window above IGForum)
	}

	static EquipSlotMapping =
	{
	[ItemEquipType.NONE             ] = ItemEquipSlot.NONE,
	[ItemEquipType.WEAPON_1H        ] = ItemEquipSlot.WEAPON_MAIN_HAND,
	[ItemEquipType.WEAPON_1H_UNIQUE ] = ItemEquipSlot.WEAPON_MAIN_HAND,
	[ItemEquipType.WEAPON_1H_MAIN   ] = ItemEquipSlot.WEAPON_MAIN_HAND,
	[ItemEquipType.WEAPON_1H_OFF    ] = ItemEquipSlot.WEAPON_OFF_HAND,
	[ItemEquipType.WEAPON_2H        ] = ItemEquipSlot.WEAPON_MAIN_HAND,
	[ItemEquipType.WEAPON_RANGED    ] = ItemEquipSlot.WEAPON_RANGED,
	[ItemEquipType.ARMOR_SHIELD     ] = ItemEquipSlot.WEAPON_OFF_HAND,
	[ItemEquipType.ARMOR_HEAD       ] = ItemEquipSlot.ARMOR_HEAD,
	[ItemEquipType.ARMOR_NECK       ] = ItemEquipSlot.ARMOR_NECK,
	[ItemEquipType.ARMOR_SHOULDER   ] = ItemEquipSlot.ARMOR_SHOULDER,
	[ItemEquipType.ARMOR_CHEST      ] = ItemEquipSlot.ARMOR_CHEST,
	[ItemEquipType.ARMOR_ARMS       ] = ItemEquipSlot.ARMOR_ARMS,
	[ItemEquipType.ARMOR_HANDS      ] = ItemEquipSlot.ARMOR_HANDS,
	[ItemEquipType.ARMOR_WAIST      ] = ItemEquipSlot.ARMOR_WAIST,
	[ItemEquipType.ARMOR_LEGS       ] = ItemEquipSlot.ARMOR_LEGS,
	[ItemEquipType.ARMOR_FEET       ] = ItemEquipSlot.ARMOR_FEET,
	[ItemEquipType.ARMOR_RING       ] = ItemEquipSlot.ARMOR_RING_L,
	[ItemEquipType.ARMOR_RING_UNIQUE] = ItemEquipSlot.ARMOR_RING_L,
	[ItemEquipType.ARMOR_AMULET     ] = ItemEquipSlot.ARMOR_AMULET,
	};

	function GetEquipSlotFromEquipType(type)
	{
		if(type in EquipSlotMapping)
			return EquipSlotMapping[type];
		return ItemEquipSlot.NONE;
	}

	function RunPreview(bOriginal)
	{
		local previewID = mSetItemID;
		if(mSetLookID != 0 && bOriginal == false)
			previewID = mSetLookID;

		if(mItemDef == null)
			return;

		local newSlot = GetEquipSlotFromEquipType(mItemDef.mEquipType);
		if(newSlot == ItemEquipSlot.NONE)
		{
			GUI.MessageBox.show("You cannot preview that item.");
			return;
		}

		local stat = unserialize(::_avatar.getStat(Stat.EQ_APPEARANCE, true));
		if(!stat)
			stat = {};
		stat[newSlot] <- previewID;  //was mItemDef.mID;
		local replace = serialize(stat);
		::_avatar.setStat(Stat.EQ_APPEARANCE, replace);

		::_avatar.getAssembler().setEquipmentAppearance(replace);
//		::_avatar.reassemble();
	}
	function SetNudePreview()
	{
		local replace = "{}";
		::_avatar.setStat(Stat.EQ_APPEARANCE, "");
		::_avatar.getAssembler().setEquipmentAppearance(replace);
//		::_avatar.reassemble();
	}
	function Restore()
	{
		::_Connection.sendQuery("mod.restoreappearance", this, [] );
	}
	function onQueryComplete(qa, results)
	{
		if(qa.query == "mod.restoreappearance")
		{
			local app = results[0][0];
			local eqapp = results[1][0];
			::_avatar.setStat(Stat.APPEARANCE, app);
			::_avatar.setStat(Stat.EQ_APPEARANCE, eqapp);
			::_avatar.getAssembler().mConfig = app;  //instead of calling configure(app) just set the config directly, the EQ function below will reassemble anyway.
			::_avatar.getAssembler().setEquipmentAppearance(eqapp);
//			::_avatar.reassemble();
		}
	}
}

class Screens.PostPreview extends GUI.Frame
{
	static mClassName = "Screens.PostPreview";

	mLabelTitle = null;
	mHTMLBody = null;
	mButtonClose = null;
	mScrollPanel = null;

	constructor()
	{
		GUI.Frame.constructor("Post Preview");

		mLabelTitle = GUI.Label("");

		mHTMLBody = GUI.HTML2();
		mHTMLBody.setMaximumSize(330, 320); //Need to be smaller so it doesn't run off the edge.
		mHTMLBody.setResize( true );
		mHTMLBody.setText("");
		mHTMLBody.setInsets(8, 8, 8, 8);
		local width = mHTMLBody.getSize().width - 28;
		local font = mHTMLBody.getFont();
		mHTMLBody.setWrapText(true, font, width);

		mButtonClose = GUI.Button("Close");
		mButtonClose.addActionListener(this);
		mButtonClose.setReleaseMessage("onButtonPressed");

		mScrollPanel = GUI.ScrollPanel(mHTMLBody);
		mScrollPanel.setPreferredSize(380, 320);
		mScrollPanel.setAppearance("GoldBorder");

		local cmain = GUI.Container(GUI.BoxLayoutV());
		cmain.add(mLabelTitle);
		cmain.add(mScrollPanel);
		cmain.add(mButtonClose);
		setContentPane(cmain);
		setSize(400, 400);
	}
	function onButtonPressed(button)
	{
		if(button == mButtonClose)
			close();
	}
	function SetPreview(title, postBody)
	{
		mLabelTitle.setText(title);
		local adjustedPost = "<font size=\"20\" color=\"FFFFFF\">" + postBody + "</font>";
		mHTMLBody.setText(adjustedPost);
		mScrollPanel.setIndex(0);
	}
}

function InputCommands::IGF(args)
{
	Screens.toggle("IGForum");
}

function InputCommands::previewItem(args)
{
	Screens.toggle("PreviewItem");
}
