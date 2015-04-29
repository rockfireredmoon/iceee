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
			print("READY STATE: " + this.readyState);
			print("REPSONSE: " + this.responseText);
			print("STATUS: " + this.status);
			print("READY STATE: " + this.readyState);
			if (this.readyState == 4)
			{
				if (this.status == 201)
				{
					local text = this.responseText;
					local mb = ::GUI.MessageBox.show("Thank you! Your bug report has been submitted.");
					mb.setOverlay("GUI/BugReportOverlay");
					this.setVisible(false);
				}
				else
				{
					local mb = ::GUI.MessageBox.show("Cannot contact the server. Please try again. " + this.responseText + " /" + this.status);
					mb.setOverlay("GUI/BugReportOverlay");
				}

				return;
			}
		};
		
		summ = _prepare(summ);
		desc = _prepare("Category: " + category + "\n\n" + desc);
		
		//  <token>:x-oauth-basic 
		//Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
		// 5d296bbcc111e6bdbb3a15beaeb53170fc560748
		
		local txt = "{\n" + 
			" \"title\": \"" + summ + "\",\n" + 
			" \"body\": \" + desc "\",\n" + 
			" \"labels\": [ \"bug\" ]\n" +
			"}\n";
		local TOKEN = "5d296bbcc111e6bdbb3a15beaeb53170fc560748";
		local userpw = TOKEN + ":x-oauth-basic";
		req.setRequestHeader("Content-Type", "application/json");
		req.setRequestHeader("Authorization", "Basic " + this.base64_encode(userpw));
		req.open("POST", "http://api.github.com/repos/rockfireredmoon/iceee/issues");
		print("REQ: " + txt);
		req.send(txt);
	}

	function _prepare(text) {
		text = this.Util.replace(text, "\r\n", "\\n");
		text = this.Util.replace(text, "\n", "\\n");
		text = this.Util.replace(text, "\"", "\\\"");
		return text;
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

