this.require("UI/Screens");
this.require("UI/ActionContainer");
class this.Screens.BugReport extends this.GUI.Frame
{
	static mScreenName = "BugReport";
	mCategory = null;
	mDescription = null;
	mSummary = null;
	mSummaryContainer = null;
	CategoryType = {
		[0] = {
			name = "Character",
			prefix = "CHAR"
		},
		[1] = {
			name = "Combat",
			prefix = "COMBAT"
		},
		[2] = {
			name = "Chat / Communication",
			prefix = "CHAT"
		},
		[3] = {
			name = "Items/Gold",
			prefix = "ECON"
		},
		[4] = {
			name = "Lag/Performance",
			prefix = "PERF"
		},
		[5] = {
			name = "NPCs/Mobs/Creatures ",
			prefix = "MOB"
		},
		[6] = {
			name = "Quests",
			prefix = "QUEST"
		},
		[7] = {
			name = "Sound",
			prefix = "SND"
		},
		[8] = {
			name = "UI",
			prefix = "UI"
		},
		[9] = {
			name = "World (Collision/Props/Terrain)",
			prefix = "WORLD"
		},
		[10] = {
			name = "Suggestion/Request",
			prefix = "IDEA"
		},
		[11] = {
			name = "Other",
			prefix = "OTHER"
		}
	};
	constructor()
	{
		this.GUI.Frame.constructor("Bug Report");
		local content = this.GUI.Container(this.GUI.BoxLayoutV());
		local font = ::GUI.Font("Maiandra", 22);
		local info = ::GUI.HTML("<font size=\"22\">Please enter a detailed description of the problem you encountered. Please be as specific as possible." + " More detailed descriptions will help the development team solve the problem quickly.</font>");
		info.setResize(true);
		info.setMaximumSize(500, null);
		info.setInsets(0, 4, 0, 4);
		content.add(info);
		content.setInsets(4, 4, 4, 4);
		local container = this.GUI.Container(this.GUI.GridLayout(3, 2));
		container.getLayoutManager().setColumns(75, 400);
		container.getLayoutManager().setRows(25, 25, 200);
		container.getLayoutManager().setGaps(0, 4);
		container.setInsets(4, 8, 8, 4);
		local categoryLabel = ::GUI.Label("Category:");
		categoryLabel.setFont(font);
		container.add(categoryLabel);
		this.mCategory = this.GUI.DropDownList();

		foreach( key, categoryData in this.CategoryType )
		{
			this.mCategory.addChoice(categoryData.name);
		}

		this.mCategory.addSelectionChangeListener(this);
		container.add(this.mCategory);
		local summaryLabel = ::GUI.Label("Summary:");
		summaryLabel.setFont(font);
		this.mSummary = ::GUI.InputArea("");
		container.add(summaryLabel);
		container.add(this.mSummary);
		local descLabel = ::GUI.Label("Description:");
		descLabel.setFont(font);
		descLabel.setTextAlignment(0.0, 0.0);
		this.mDescription = ::GUI.InputArea("");
		this.mDescription.setMultiLine(true);
		container.add(descLabel);
		container.add(this.mDescription);
		content.add(container);
		local buttons = this.GUI.Container(this.GUI.BoxLayout());
		buttons.getLayoutManager().setPackAlignment(0.5);
		local submit = this.GUI.NarrowButton("Submit");
		submit.setReleaseMessage("onSubmitPressed");
		submit.addActionListener(this);
		buttons.add(submit);
		buttons.setInsets(0, 0, 4, 0);
		content.add(buttons);
		this.setContentPane(content);
		this.setInsets(4, 4, 4, 4);
		this.setSize(500, 415);
		this.center();
		this.mSummary.setText("");
		this.mDescription.setText("");
		this.setOverlay("GUI/BugReportOverlay");
		::_root.setKeysEnabled(true);
	}

	function onSubmitPressed( button )
	{
		local category = "CHAR";

		if (this.mCategory && this.mCategory.getCurrentIndex() != -1)
		{
			category = this.CategoryType[this.mCategory.getCurrentIndex()].prefix;
		}

		local summ = this.Util.trim(this.mSummary.getText());
		local desc = this.Util.trim(this.mDescription.getText());

		if (summ.len() < 5)
		{
			local mb = ::GUI.MessageBox.show("The summary is too short. Please add a more detailed summary.");
			mb.setOverlay("GUI/BugReportOverlay");
			return;
		}

		if (desc.len() < 15)
		{
			local mb = ::GUI.MessageBox.show("The description is too short. Please add a more detailed description.");
			mb.setOverlay("GUI/BugReportOverlay");
			return;
		}

		local req = this.XMLHttpRequest();
		req.onreadystatechange = function () : ( category, summ, desc )
		{
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
					local text = this.responseText;
					local indexStart = text.find("<value>") + 7;
					local indexEnd = text.find("</value>");
					local auth = text.slice(indexStart, indexEnd);
					this.CreateJiraTask(auth, category, summ, desc);
				}
				else
				{
					local mb = ::GUI.MessageBox.show("Cannot connect to the server. Please try again later.");
					mb.setOverlay("GUI/BugReportOverlay");
				}

				return;
			}
		};
		local txt = "<?xml version=\"1.0\"?>\n" + "<methodCall>\n" + "\t<methodName>jira1.login</methodName>\n" + "\t<params>\n" + "\t\t<param>\n" + "\t\t\t<value><string>bugger</string></value>\n" + "\t\t</param>\n" + "\t\t<param>\n" + "\t\t\t<value><string>bugger</string></value>\n" + "\t\t</param>\n" + "\t</params>\n" + "</methodCall>";
		req.setRequestHeader("Content-Type", "text/xml");
		req.open("POST", "https://jira.sparkplaymedia.com/rpc/xmlrpc");
		req.send(txt);
		this.setVisible(false);
	}

	function setVisible( value )
	{
		if (value && !this.isVisible())
		{
			this.mSummary.setText("");
			this.mDescription.setText("");
			this.mSummary.requestKeyboardFocus();
		}

		this.GUI.Frame.setVisible(value);
	}

}

function CreateJiraTask( auth, category, summ, desc )
{
	local req = this.XMLHttpRequest();
	req.onreadystatechange = function ()
	{
		if (this.readyState == 4)
		{
			if (this.status == 200)
			{
				local mb = ::GUI.MessageBox.show("Thank you! Your bug report has been submitted.");
				mb.setOverlay("GUI/BugReportOverlay");
			}
			else
			{
				local mb = ::GUI.MessageBox.show("Cannot connect to the server. Please try again later.");
				mb.setOverlay("GUI/BugReportOverlay");
			}
		}
	};
	local text = "Client Version: " + (this.Util.isDevMode() == false ? this.gVersion : "Dev") + "\n" + "Username: " + ::_username + "\n" + "Creature ID: " + (::_avatar != null ? ::_avatar.getID() : "(no creature ID)") + "\n" + "CreatureDef ID: " + (::_avatar != null ? ::_avatar.getCreatureDef().getID() : "(no creature def)") + "\n" + "Persona Name: " + (::_avatar != null ? ::_avatar.getName() : "(no name)") + "\n" + "Position: " + (::_avatar != null ? ::_avatar.getPosition().x + ", " + ::_avatar.getPosition().y + ", " + ::_avatar.getPosition().z : "(no position)") + "\n" + "Zone Def: " + ::_sceneObjectManager.getCurrentZoneDefID().tostring() + "\n" + "Zone ID: " + ::_sceneObjectManager.getCurrentZoneID().tostring() + "\n\n" + desc;
	local txt = "<?xml version=\"1.0\"?>\n" + "<methodCall>\n" + "\t<methodName>jira1.createIssue</methodName>\n" + "\t<params>\n" + "\t\t<param>\n" + "\t\t\t<value><string>" + auth + "</string></value>\n" + "\t\t</param>\n" + "\t\t<param>\n" + "\t\t\t<value>\n" + "\t\t\t\t<struct>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>project</name>\n" + "\t\t\t\t\t\t<value><string>BUG</string></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>type</name>\n" + "\t\t\t\t\t\t<value><int>1</int></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>summary</name>\n" + "\t\t\t\t\t\t<value><string> [" + category + "]BUG: " + summ + "</string></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>assignee</name>\n" + "\t\t\t\t\t\t<value><string>admin</string></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>priority</name>\n" + "\t\t\t\t\t\t<value><string>3</string></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t\t<member>\n" + "\t\t\t\t\t\t<name>description</name>\n" + "\t\t\t\t\t\t<value><string>" + text + "</string></value>\n" + "\t\t\t\t\t</member>\n" + "\t\t\t\t</struct>\n" + "\t\t\t</value>\n" + "\t\t</param>\n" + "\t</params>\n" + "</methodCall>";
	req.setRequestHeader("Content-Type", "text/xml");
	req.open("POST", "https://jira.sparkplaymedia.com/rpc/xmlrpc");
	req.send(txt);
}

