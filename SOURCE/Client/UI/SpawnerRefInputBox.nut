this.require("GUI/Container");
class this.GUI.SpawnerRefInputBox extends this.GUI.Container
{
	static mClassName = "SpawnerRefInputBox";
	mInputBox = null;
	mChoices = null;
	mVarEditButton = null;
	mValidAsset = false;
	mTemplateList = null;
	mLastInputBox = null;
	mType = null;
	constructor()
	{
		this.GUI.Container.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mInputBox = this.GUI.InputBox();
		this.mInputBox.setContentAssistEnabled(true);
		this.mInputBox.addActionListener(this);
		this.mTemplateList = [];
		this.mVarEditButton = this.GUI.Button("...");
		this.mVarEditButton.setReleaseMessage("onVarEdit");
		this.mVarEditButton.addActionListener(this);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.add(this.mInputBox, this.GUI.BorderLayout.CENTER);
		this.add(this.mVarEditButton, this.GUI.BorderLayout.EAST);
		this.mChoices = this.GUI.AnchorPanel(this);
		this.mChoices.setOverlay(null);
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function onInputComplete( inputbox )
	{
		this._fireActionPerformed("onInputComplete");
	}

	function _fireActionPerformed( message )
	{
		if (message == "onInputComplete" && !this.mValidAsset)
		{
			return;
		}

		this.mMessageBroadcaster.broadcastMessage(message, this);
	}

	function onVarEdit( button )
	{
	}

	function onTextChanged( inputbox )
	{
		local isValid = this.validTemplate(::Util.trim(inputbox.getText()));
		local templates = this.getTemplates();
		this.mType = null;

		if (isValid)
		{
			this.mInputBox.setFontColor("FFFFFF");
			this.mValidAsset = true;
		}
		else if (typeof templates == "array")
		{
			this.mInputBox.setFontColor("FFFF00");
			this.mValidAsset = false;
		}
		else if (templates == null)
		{
			this.mInputBox.setFontColor("FF0000");
			this.mValidAsset = false;
		}
	}

	function validTemplate( searchStr )
	{
		foreach( template in this.mTemplateList )
		{
			if (searchStr.len() > 0 && template[0] == searchStr)
			{
				return true;
			}
		}

		return false;
	}

	function onContentAssist( inputbox )
	{
		this.mLastInputBox = inputbox;
		this.getTemplateList();
	}

	function getTemplates()
	{
		return this.mTemplateList;
	}

	function createChoices()
	{
		if (!this.mLastInputBox)
		{
			return;
		}

		local templates = this.getTemplates();

		if (this.IsInstanceOf(templates, this.AssetReference))
		{
			templates = [
				templates
			];
		}

		if (typeof templates != "array" || templates.len() == 0)
		{
			return;
		}

		this.mChoices.removeAll();
		local maxPerColumn = 20;
		local columns = (templates.len() - 1) / maxPerColumn + 1;
		local rows = this.Math.min(maxPerColumn, templates.len());
		local layoutMgr = this.GUI.GridLayout(rows, columns);
		layoutMgr.setColumnMajor(true);
		this.mChoices.setLayoutManager(layoutMgr);
		local ch;

		foreach( a in templates )
		{
			local ch = this.GUI.Button(a[0] + (a[1] == "PACKAGE" ? " (Package)" : " (Creature)"));
			ch.setData(a);
			ch.setAppearance("Container");
			ch.setSelection(true);
			ch.setPressMessage("_onCompletionPress");
			ch.addActionListener(this);
			this.mChoices.add(ch);
		}

		this.mChoices.validate();
		local pos = this.mLastInputBox.getScreenPosition();
		this.mChoices.setPosition(pos);
		this.mChoices.setSize(this.mChoices.getPreferredSize());
		this.mChoices.keepOnScreen();
		this.mChoices.setOverlay(this.GUI.POPUP_OVERLAY);
		this.mChoices.setVisible(true);
		this.GUI._Manager.addTransientToplevel(this.mChoices);
	}

	function isValidAsset()
	{
		return this.mValidAsset;
	}

	function setAsset( asset )
	{
		if (typeof asset == "string")
		{
			this.mInputBox.setText(asset);
		}
	}

	function getTemplateList()
	{
		local text = this.Util.trim(this.mInputBox.getText());

		if (text == "")
		{
			return;
		}

		if (text.len() < 2)
		{
			this.mTemplateList.removeAllRows();
			this.mTemplateList = [];
			this.mTemplateList.append([
				"You must provide a search string at least 2 characters long",
				""
			]);
			this.mList.removeAllRows();
			return;
		}

		::_Connection.sendQuery("spawn.list", this, [
			"",
			text
		]);
		this.mTemplateList = [];
	}

	function getType()
	{
		if (this.mType != null)
		{
			return this.mType;
		}

		local text = this.getText();

		foreach( t in this.mTemplateList )
		{
			if (t[0] == text)
			{
				return t[1];
			}
		}

		return null;
	}

	function getText()
	{
		return this.mInputBox.getText();
	}

	function _onCompletionPress( button )
	{
		this.setAsset(button.getData()[0]);
		this.mType = button.getData()[1];
		this.mChoices.setOverlay(null);
		this.GUI._Manager.releaseKeyboardFocus(this);
		this.mInputBox._fireActionPerformed("onInputComplete");
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "spawn.list")
		{
			foreach( r in results )
			{
				this.mTemplateList.append([
					r[1],
					r[2] == "C" ? "CREATURE" : "PACKAGE"
				]);
			}

			this.createChoices();
			this.onTextChanged(this.mInputBox);
		}
	}

	function destroy()
	{
		this.mChoices.destroy();
		this.mChoices = null;
		this.GUI.Container.destroy();
	}

}

