this.require("GUI/Container");
class this.GUI.AssetRefInputBox extends this.GUI.Container
{
	static mClassName = "AssetRefInputBox";
	mInputBox = null;
	mChoices = null;
	mVarEditButton = null;
	mValidAsset = false;
	mShowShortName = true;
	constructor()
	{
		this.GUI.Container.constructor();
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mShowShortName = true;
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
		local asset = this.getAsset();

		if (typeof asset == "instance" && (asset instanceof this.AssetReference))
		{
			this.setFontColor(null);
			this.mValidAsset = true;
		}
		else if (typeof asset == "array")
		{
			this.setFontColor("FFFF00");
			this.mValidAsset = false;
		}
		else if (asset == null)
		{
			this.setFontColor("FF0000");
			this.mValidAsset = false;
		}
	}

	function onContentAssist( inputbox )
	{
		local assets = this.getAsset();

		if (this.IsInstanceOf(assets, this.AssetReference))
		{
			assets = [
				assets
			];
		}

		if (typeof assets != "array" || assets.len() == 0)
		{
			return;
		}

		this.mChoices.removeAll();
		local maxPerColumn = 20;
		local columns = (assets.len() - 1) / maxPerColumn + 1;
		local rows = this.Math.min(maxPerColumn, assets.len());
		local layoutMgr = this.GUI.GridLayout(rows, columns);
		layoutMgr.setColumnMajor(true);
		this.mChoices.setLayoutManager(layoutMgr);
		local ch;

		foreach( a in assets )
		{
			local ch = this.GUI.Button(this.mShowShortName == true ? a.getShortestAssetName() : "" + a.getAsset());
			ch.setAppearance("Container");
			ch.setSelection(true);
			ch.setPressMessage("_onCompletionPress");
			ch.addActionListener(this);
			this.mChoices.add(ch);
		}

		this.mChoices.validate();
		local pos = inputbox.getScreenPosition();
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
		else
		{
			this.mInputBox.setText(this.mShowShortName == true ? asset.getShortestAssetName() : "" + asset.getAsset());
		}
	}

	function setShowShortName( which )
	{
		this.mShowShortName = which;
	}

	function getAsset()
	{
		local str = this.getText();
		local a = this.AssetReference(str);

		if (a.isCataloged())
		{
			return a;
		}

		local vars = a.getVars();
		local list = this.GetAssetCompletions(str);

		if (list.len() == 1)
		{
			return list[0];
		}

		if (list.len() == 0)
		{
			return null;
		}

		return list;
	}

	function getValue()
	{
		local a = this.getAsset();

		if (this.IsInstanceOf(a, this.AssetReference))
		{
			return a;
		}

		return null;
	}

	function getText()
	{
		local asset = this.mInputBox.getText();

		if (asset == "")
		{
			return asset;
		}

		while (asset[asset.len() - 1] == 32)
		{
			asset = asset.slice(0, asset.len() - 1);
		}

		return asset;
	}

	function setValue( value )
	{
		if (!this.IsInstanceOf(value, this.AssetReference))
		{
			throw this.Exception("Invalid value: " + value);
		}

		this.setAsset(value);
	}

	function _onCompletionPress( button )
	{
		this.setAsset(button.getText());
		this.mChoices.setOverlay(null);
		this.GUI._Manager.releaseKeyboardFocus(this);
		this.mInputBox._fireActionPerformed("onInputComplete");
	}

	function destroy()
	{
		this.mChoices.destroy();
		this.mChoices = null;
		this.GUI.Container.destroy();
	}

}

