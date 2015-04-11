this.require("GUI/Container");
class this.GUI.TemplateRefInputBox extends this.GUI.Container
{
	static mClassName = "TemplateRefInputBox";
	mInputBox = null;
	mChoices = null;
	mVarEditButton = null;
	mValidAsset = false;
	mTemplateList = null;
	mLastInputBox = null;
	constructor()
	{
		this.GUI.Container.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mInputBox = this.GUI.InputBox();
		this.mInputBox.setContentAssistEnabled(true);
		this.mInputBox.addActionListener(this);
		this.mVarEditButton = this.GUI.Button("...");
		this.mVarEditButton.setReleaseMessage("onVarEdit");
		this.mVarEditButton.addActionListener(this);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.add(this.mInputBox, this.GUI.BorderLayout.CENTER);
		this.add(this.mVarEditButton, this.GUI.BorderLayout.EAST);
		this.mChoices = this.GUI.AnchorPanel(this);
		this.mChoices.setOverlay(null);
		this.getTemplateList();
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
			if (searchStr.len() > 0 && template == searchStr)
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
		local results = [];
		local searchStr = this.mInputBox.getText();
		searchStr = this.Util.trim(searchStr);

		foreach( template in this.mTemplateList )
		{
			if (searchStr.len() == 0 || template.tolower().find(searchStr) != null)
			{
				results.append(template);
			}
		}

		if (results.len() == 0)
		{
			results = null;
		}

		return results;
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
			local ch = this.GUI.Button(a);
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
		this.print(" mWidth: " + this.mWidth + ", mHeight: " + this.mHeight);

		if (typeof asset == "string")
		{
			this.mInputBox.setText(asset);
		}
	}

	function getTemplateList()
	{
		this._Connection.sendQuery("build.template.list", this);
	}

	function getText()
	{
		return this.mInputBox.getText();
	}

	function _onCompletionPress( button )
	{
		this.setAsset(button.getText());
		this.mChoices.setOverlay(null);
		this.GUI._Manager.releaseKeyboardFocus(this);
		this.mInputBox._fireActionPerformed("onInputComplete");
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "build.template.list")
		{
			this.mTemplateList = results[0];
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

