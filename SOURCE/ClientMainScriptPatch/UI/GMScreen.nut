this.require("UI/Screens");
local wasVisible = this.Screens.close("GMScreen");
class this.Screens.GMScreen extends this.GUI.Frame
{
	mPetitionScreenButton = null;
	mainScreen = null;
	mCreateItemId = null;
	mCreateItemButton = null;
	mListInventoryButton = null;
	mDeleteItemDefButton = null;
	mCopperAmount = null;
	mCreditsAmount = null;
	mModifyCopperButton = null;
	mModifyCreditsButton = null;
	mCopperReasonPopup = null;
	mCreditsReasonPopup = null;
	mRenameInputArea = null;
	mRenameButton = null;
	mRenameReasonPopup = null;
	mFreezeInputArea = null;
	mFreezeButton = null;
	mUnfreezeButton = null;
	mChatChannelInput = null;
	mJoinChatChannelButton = null;
	mLeaveChatChannelButton = null;
	mCreateItemPopup = null;
	mDeleteItemPopup = null;
	mAccountInfoInput = null;
	mAccountInfoButton = null;
	mInvisOnButton = null;
	mInvisOffButton = null;
	mSilenceInputArea = null;
	mSilenceOnButton = null;
	mSilenceOffButton = null;
	mGMSilenceReasonPopup = null;
	mQuestInputArea = null;
	constructor()
	{
		this.GUI.Frame.constructor("GM Screen");
		this.mainScreen = this._buildMainScreen();
		this.setSize(435, 435);
		this.setContentPane(this.mainScreen);
	}

	function onButtonPressed( button )
	{
		if (button == this.mFreezeButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();

			if (selectedTarget != null)
			{
				local targetId = selectedTarget.getID();
				local time = this.mFreezeInputArea.getText();

				if (time == "")
				{
					time = 10;
				}

				::_Connection.sendQuery("statuseffect.set", this, [
					targetId,
					this.StatusEffects.GM_FROZEN,
					1,
					time
				]);
				this.mFreezeInputArea.setText("");
			}
		}
		else if (button == this.mUnfreezeButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();

			if (selectedTarget != null)
			{
				local targetId = selectedTarget.getID();
				::_Connection.sendQuery("statuseffect.set", this, [
					targetId,
					this.StatusEffects.GM_FROZEN,
					0
				]);
				this.mFreezeInputArea.setText("");
			}
		}
		else if (button == this.mDeleteItemDefButton)
		{
			local itemId = this.mCreateItemId.getText();

			if (itemId == "")
			{
				this.IGIS.error("No itemdef specified");
				return;
			}

			this.mDeleteItemPopup = this.GUI.PopupInputBox("Reason for deleting item:");
			this.mDeleteItemPopup.addActionListener(this);
			local inputBoxPosX = ::Screen.getWidth() / 2 - this.mDeleteItemPopup.getWidth() / 2;
			local inputBoxPosY = ::Screen.getHeight() / 2 - this.mDeleteItemPopup.getWidth() / 2;
			this.mDeleteItemPopup.setPosition(inputBoxPosX, inputBoxPosY);
			this.mDeleteItemPopup.showInputBox();
		}
		else if (button == this.mJoinChatChannelButton)
		{
			local channel = this.mChatChannelInput.getText();

			if (channel == "")
			{
				this.IGIS.error("No channel specified");
			}
			else
			{
				::_Connection.sendQuery("ps.join", this, [
					channel
				]);
			}

			this.mChatChannelInput.setText("");
		}
		else if (button == this.mLeaveChatChannelButton)
		{
			local channel = this.mChatChannelInput.getText();

			if (channel == "")
			{
				this.IGIS.error("No channel specified");
			}
			else
			{
				::_Connection.sendQuery("ps.leave", this, [
					channel
				]);
			}

			this.mChatChannelInput.setText("");
		}
		else if (button == this.mAccountInfoButton)
		{
			local personaName = this.mAccountInfoInput.getText();

			if (personaName == "")
			{
				local selectedTarget = ::_avatar.getTargetObject();

				if (selectedTarget)
				{
					personaName = selectedTarget.getName();
				}
			}

			if (personaName != "")
			{
				::_Connection.sendQuery("account.info", this, [
					personaName
				]);
			}
		}
		else if (button == this.mCreateItemButton)
		{
			local itemId = this.mCreateItemId.getText();

			if (itemId == "")
			{
				this.IGIS.error("No itemdef specified");
				return;
			}

			this.mCreateItemPopup = this.GUI.PopupInputBox("Reason for creating item:");
			this.mCreateItemPopup.addActionListener(this);
			local inputBoxPosX = ::Screen.getWidth() / 2 - this.mCreateItemPopup.getWidth() / 2;
			local inputBoxPosY = ::Screen.getHeight() / 2 - this.mCreateItemPopup.getWidth() / 2;
			this.mCreateItemPopup.setPosition(inputBoxPosX, inputBoxPosY);
			this.mCreateItemPopup.showInputBox();
		}
		else if (button == this.mModifyCopperButton)
		{
			local channel = this.mCopperAmount.getText();

			if (channel == "")
			{
				this.IGIS.error("No copper value set");
			}
			else
			{
				this.mCopperReasonPopup = this.GUI.PopupInputBox("Reason for modifying copper:");
				this.mCopperReasonPopup.addActionListener(this);
				local inputBoxPosX = ::Screen.getWidth() / 2 - this.mCopperReasonPopup.getWidth() / 2;
				local inputBoxPosY = ::Screen.getHeight() / 2 - this.mCopperReasonPopup.getWidth() / 2;
				this.mCopperReasonPopup.setPosition(inputBoxPosX, inputBoxPosY);
				this.mCopperReasonPopup.showInputBox();
			}
		}
		else if (button == this.mModifyCreditsButton)
		{
			local channel = this.mCreditsAmount.getText();

			if (channel == "")
			{
				this.IGIS.error("No copper value set");
			}
			else
			{
				this.mCreditsReasonPopup = this.GUI.PopupInputBox("Reason for modifying credits:");
				this.mCreditsReasonPopup.addActionListener(this);
				local inputBoxPosX = ::Screen.getWidth() / 2 - this.mCreditsReasonPopup.getWidth() / 2;
				local inputBoxPosY = ::Screen.getHeight() / 2 - this.mCreditsReasonPopup.getWidth() / 2;
				this.mCreditsReasonPopup.setPosition(inputBoxPosX, inputBoxPosY);
				this.mCreditsReasonPopup.showInputBox();
			}
		}
		else if (button == this.mPetitionScreenButton)
		{
			this.Screens.show("GMPetitionScreen");
		}
		else if (button == this.mSilenceOnButton)
		{
			this.mGMSilenceReasonPopup = this.GUI.PopupInputBox("Reason for applying penalty:");
			this.mGMSilenceReasonPopup.addActionListener(this);
			local inputBoxPosX = ::Screen.getWidth() / 2 - this.mGMSilenceReasonPopup.getWidth() / 2;
			local inputBoxPosY = ::Screen.getHeight() / 2 - this.mGMSilenceReasonPopup.getWidth() / 2;
			this.mGMSilenceReasonPopup.setPosition(inputBoxPosX, inputBoxPosY);
			this.mGMSilenceReasonPopup.showInputBox();
		}
		else if (button == this.mSilenceOffButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			::_Connection.sendQuery("statuseffect.set", this, [
				targetId,
				this.StatusEffects.GM_SILENCED,
				0
			]);
			this.mFreezeInputArea.setText("");
		}
		else if (button == this.mRenameButton)
		{
			local name = this.mRenameInputArea.getText();
			name = this.Util.split(name, " ");

			if (name.len() < 2)
			{
				this.IGIS.error("Must provide both a last and first name");
			}
			else
			{
				::_Connection.sendQuery("validate.name", this, name[0], name[1]);
			}
		}
		else if (button == this.mListInventoryButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			::_Connection.sendQuery("itemdef.contents", this, [
				targetId
			]);
		}

		if (button == this.mInvisOnButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			::_Connection.sendQuery("statuseffect.set", this, [
				targetId,
				this.StatusEffects.GM_INVISIBLE,
				1
			]);
		}
		else if (button == this.mInvisOffButton)
		{
			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			::_Connection.sendQuery("statuseffect.set", this, [
				targetId,
				this.StatusEffects.GM_INVISIBLE,
				0
			]);
		}
	}

	function onInputComplete( inputArea )
	{
		if (inputArea == this.mCreateItemPopup)
		{
			if (this.mCreateItemPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for creating this item.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			local itemId = this.mCreateItemId.getText();

			if (itemId != "")
			{
				::_Connection.sendQuery("item.create", this, [
					itemId,
					targetId,
					this.mCreateItemPopup.getText()
				]);
			}

			this.mCreateItemId.setText("");
			this.mCreateItemPopup.hidePopup();
			this.mCreateItemPopup = null;
		}
		else if (inputArea == this.mDeleteItemPopup)
		{
			if (this.mDeleteItemPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for deleting this item.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			local itemId = this.mCreateItemId.getText();

			if (itemId != "")
			{
				::_Connection.sendQuery("itemdef.delete", this, [
					itemId,
					targetId,
					this.mDeleteItemPopup.getText()
				]);
			}

			this.mCreateItemId.setText("");
			this.mDeleteItemPopup.hidePopup();
			this.mDeleteItemPopup = null;
		}
		else if (inputArea == this.mCopperReasonPopup)
		{
			if (this.mCopperReasonPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for modifying copper.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local creatureName = ::_avatar.getName();

			if (selectedTarget != null)
			{
				creatureName = selectedTarget.getName();
			}

			local copperAmount = this.mCopperAmount.getText();

			if (copperAmount != "")
			{
				::_Connection.sendQuery("util.addFunds", this, [
					"COPPER",
					copperAmount,
					this.mCopperReasonPopup.getText(),
					creatureName
				]);
			}

			this.mCopperAmount.setText("");
			this.mCopperReasonPopup.hidePopup();
			this.mCopperReasonPopup = null;
		}
		else if (inputArea == this.mCreditsReasonPopup)
		{
			if (this.mCreditsReasonPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for modifying credits.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local creatureName = ::_avatar.getName();

			if (selectedTarget != null)
			{
				creatureName = selectedTarget.getName();
			}

			local creditsAmount = this.mCreditsAmount.getText();

			if (creditsAmount != "")
			{
				::_Connection.sendQuery("util.addFunds", this, [
					"CREDITS",
					creditsAmount,
					this.mCreditsReasonPopup.getText(),
					creatureName
				]);
			}

			this.mCreditsAmount.setText("");
			this.mCreditsReasonPopup.hidePopup();
			this.mCreditsReasonPopup = null;
		}
		else if (inputArea == this.mGMSilenceReasonPopup)
		{
			if (this.mGMSilenceReasonPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for silencing a character.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getID();
			}

			local time = this.mSilenceInputArea.getText();

			if (time == "")
			{
				time = 10;
			}

			::_Connection.sendQuery("statuseffect.set", this, [
				targetId,
				this.StatusEffects.GM_SILENCED,
				1,
				time,
				this.mGMSilenceReasonPopup.getText()
			]);
			this.mFreezeInputArea.setText("");
			this.mGMSilenceReasonPopup.hidePopup();
			this.mGMSilenceReasonPopup = null;
			this.mSilenceInputArea.setText("");
		}
		else if (inputArea == this.mRenameReasonPopup)
		{
			local name = this.mRenameInputArea.getText();

			if (this.mRenameReasonPopup.getText() == "")
			{
				this.IGIS.error("You must specify a reason for renaming a character.");
				return;
			}

			local selectedTarget = ::_avatar.getTargetObject();
			local targetId = ::_avatar.getCreatureDef().getID();

			if (selectedTarget != null)
			{
				targetId = selectedTarget.getCreatureDef().getID();
			}

			::_Connection.sendQuery("creature.def.edit", this, [
				"REASON",
				this.mRenameReasonPopup.getText(),
				targetId,
				"name",
				name
			]);
			::_Connection.sendQuery("creature.def.edit", this, [
				"REASON",
				this.mRenameReasonPopup.getText(),
				targetId,
				"DISPLAY_NAME",
				name
			]);
			this.mRenameInputArea.setText("");
			this.mRenameReasonPopup.hidePopup();
			this.mRenameReasonPopup = null;
		}
	}

	function setVisible( value )
	{
		if (value)
		{
			::_Connection.sendQuery("persona.gm", this);
		}
		else
		{
			this.GUI.Frame.setVisible(false);
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "itemdef.contents")
		{
			this.IGIS.info("--Inventory contents--");

			foreach( result in results )
			{
				this.IGIS.info("ItemDefId: " + result[0] + " Name: " + result[1]);
			}
		}
		else if (qa.query == "item.create")
		{
			this.IGIS.info("Item created successfully");
		}
		else if (qa.query == "itemdef.delete")
		{
			if (results[0][0] == "OK")
			{
				this.IGIS.info("Item deleted successfully");
			}
		}
		else if (qa.query == "validate.name")
		{
			this.mRenameReasonPopup = this.GUI.PopupInputBox("Reason for player rename:");
			this.mRenameReasonPopup.addActionListener(this);
			local inputBoxPosX = ::Screen.getWidth() / 2 - this.mRenameReasonPopup.getWidth() / 2;
			local inputBoxPosY = ::Screen.getHeight() / 2 - this.mRenameReasonPopup.getWidth() / 2;
			this.mRenameReasonPopup.setPosition(inputBoxPosX, inputBoxPosY);
			this.mRenameReasonPopup.showInputBox();
		}
		else if (qa.query == "ps.join" && results[0][0] == "OK")
		{
			this.IGIS.info("Joined channel " + qa.args[0] + " successfully");
		}
		else if (qa.query == "ps.leave" && results[0][0] == "OK")
		{
			this.IGIS.info("Left channel " + qa.args[0] + " successfully");
		}
		else if (qa.query == "account.info")
		{
			local accountEmail = results[0][0];
			local accountId = results[0][1];
			this.IGIS.info("Account email: " + accountEmail);
			this.IGIS.info("AccountId: " + accountId);
			local i = 0;

			foreach( result in results )
			{
				if (i != 0)
				{
					this.IGIS.info(result[0] + " - level: " + result[1] + " " + this.Professions[result[2].tointeger()].name + " " + result[3]);
				}

				i++;
			}
		}
		else if (qa.query == "persona.gm")
		{
			this.GUI.Frame.setVisible(true);
		}
		else if (qa.query == "quest.hack")
		{
			this.IGIS.info("Quest " + qa.args[1] + " has " + qa.args[0] + " successfully.");
		}
	}

	function onQueryError( qa, results )
	{
		this.IGIS.error(results);
	}

	function _buildAccountInfoSection()
	{
		local container = this.GUI.Container();
		this.mAccountInfoInput = this.GUI.InputArea();
		this.mAccountInfoInput.setSize(150, 15);
		this.mAccountInfoButton = this.GUI.Button("Account Info");
		this.mAccountInfoButton.addActionListener(this);
		this.mAccountInfoButton.setReleaseMessage("onButtonPressed");
		container.add(this.mAccountInfoInput);
		container.add(this.mAccountInfoButton);
		return container;
	}

	function _buildCreateItemSection()
	{
		local container = this.GUI.Container();
		this.mCreateItemId = this.GUI.InputArea();
		this.mCreateItemId.setSize(90, 15);
		this.mCreateItemButton = this.GUI.Button("Create Item");
		this.mCreateItemButton.addActionListener(this);
		this.mCreateItemButton.setReleaseMessage("onButtonPressed");
		this.mListInventoryButton = this.GUI.Button("List Inventory");
		this.mListInventoryButton.addActionListener(this);
		this.mListInventoryButton.setReleaseMessage("onButtonPressed");
		this.mDeleteItemDefButton = this.GUI.Button("Delete Item");
		this.mDeleteItemDefButton.addActionListener(this);
		this.mDeleteItemDefButton.setReleaseMessage("onButtonPressed");
		container.add(this.GUI.Label("Item"));
		container.add(this.mCreateItemId);
		container.add(this.mCreateItemButton);
		container.add(this.mListInventoryButton);
		container.add(this.mDeleteItemDefButton);
		return container;
	}

	function _buildChannelSection()
	{
		local container = this.GUI.Container();
		this.mChatChannelInput = this.GUI.InputArea();
		this.mChatChannelInput.setSize(100, 15);
		this.mJoinChatChannelButton = this.GUI.Button("Join Channel");
		this.mJoinChatChannelButton.addActionListener(this);
		this.mJoinChatChannelButton.setReleaseMessage("onButtonPressed");
		this.mLeaveChatChannelButton = this.GUI.Button("Leave Channel");
		this.mLeaveChatChannelButton.addActionListener(this);
		this.mLeaveChatChannelButton.setReleaseMessage("onButtonPressed");
		container.add(this.mChatChannelInput);
		container.add(this.mJoinChatChannelButton);
		container.add(this.mLeaveChatChannelButton);
		return container;
	}

	function _buildFreezeSection()
	{
		local container = this.GUI.Container();
		this.mFreezeInputArea = this.GUI.InputArea();
		this.mFreezeInputArea.setSize(75, 15);
		this.mFreezeButton = this.GUI.Button("GM Freeze");
		this.mFreezeButton.addActionListener(this);
		this.mFreezeButton.setReleaseMessage("onButtonPressed");
		this.mUnfreezeButton = this.GUI.Button("GM Unfreeze");
		this.mUnfreezeButton.addActionListener(this);
		this.mUnfreezeButton.setReleaseMessage("onButtonPressed");
		container.add(this.GUI.Label("Freeze an account(in minutes)"));
		container.add(this.mFreezeInputArea);
		container.add(this.mFreezeButton);
		container.add(this.mUnfreezeButton);
		return container;
	}

	function _buildInvisibleSection()
	{
		local container = this.GUI.Container();
		this.mInvisOnButton = this.GUI.Button("Turn Invisible");
		this.mInvisOnButton.addActionListener(this);
		this.mInvisOnButton.setReleaseMessage("onButtonPressed");
		this.mInvisOffButton = this.GUI.Button("Turn Visible");
		this.mInvisOffButton.addActionListener(this);
		this.mInvisOffButton.setReleaseMessage("onButtonPressed");
		container.add(this.mInvisOnButton);
		container.add(this.mInvisOffButton);
		return container;
	}

	function _buildMainScreen()
	{
		local container = this.GUI.Container(this.GUI.BoxLayoutV());
		this.mPetitionScreenButton = this.GUI.Button("Petitions");
		this.mPetitionScreenButton.addActionListener(this);
		this.mPetitionScreenButton.setReleaseMessage("onButtonPressed");
		container.add(this.mPetitionScreenButton);
		container.add(this.GUI.Spacer(1, 5));
		container.add(this._buildCreateItemSection());
		container.add(this._buildModifyFundsSection());
		container.add(this._buildModifyCreditsSection());
		container.add(this._buildRenameSection());
		container.add(this._buildFreezeSection());
		container.add(this._buildChannelSection());
		container.add(this._buildAccountInfoSection());
		container.add(this._buildInvisibleSection());
		container.add(this._buildSilenceSection());
		container.add(this._buildQuestSection());
		return container;
	}

	function _buildModifyFundsSection()
	{
		local container = this.GUI.Container();
		this.mCopperAmount = this.GUI.InputArea();
		this.mCopperAmount.setAllowOnlyNumbers(true);
		this.mCopperAmount.setSize(70, 15);
		this.mModifyCopperButton = this.GUI.Button("Modify Copper");
		this.mModifyCopperButton.addActionListener(this);
		this.mModifyCopperButton.setReleaseMessage("onButtonPressed");
		container.add(this.GUI.Label("Copper Amount"));
		container.add(this.mCopperAmount);
		container.add(this.mModifyCopperButton);
		return container;
	}
	function _buildModifyCreditsSection()
	{
		local container = this.GUI.Container();
		this.mCreditsAmount = this.GUI.InputArea();
		this.mCreditsAmount.setAllowOnlyNumbers(true);
		this.mCreditsAmount.setSize(70, 15);
		this.mModifyCreditsButton = this.GUI.Button("Modify Credits");
		this.mModifyCreditsButton.addActionListener(this);
		this.mModifyCreditsButton.setReleaseMessage("onButtonPressed");
		container.add(this.GUI.Label("Credits"));
		container.add(this.mCreditsAmount);
		container.add(this.mModifyCreditsButton);
		return container;
	}

	function _buildRenameSection()
	{
		local container = this.GUI.Container();
		this.mRenameInputArea = this.GUI.InputArea();
		this.mRenameInputArea.setSize(140, 15);
		this.mRenameButton = this.GUI.Button("Rename");
		this.mRenameButton.addActionListener(this);
		this.mRenameButton.setReleaseMessage("onButtonPressed");
		container.add(this.mRenameInputArea);
		container.add(this.mRenameButton);
		return container;
	}

	function _buildSilenceSection()
	{
		local container = this.GUI.Container();
		this.mSilenceInputArea = this.GUI.InputArea();
		this.mSilenceInputArea.setSize(75, 15);
		this.mSilenceOnButton = this.GUI.Button("GM Silence");
		this.mSilenceOnButton.addActionListener(this);
		this.mSilenceOnButton.setReleaseMessage("onButtonPressed");
		this.mSilenceOffButton = this.GUI.Button("GM Unsilence");
		this.mSilenceOffButton.addActionListener(this);
		this.mSilenceOffButton.setReleaseMessage("onButtonPressed");
		container.add(this.GUI.Label("Silence an account(in minutes)"));
		container.add(this.mSilenceInputArea);
		container.add(this.mSilenceOnButton);
		container.add(this.mSilenceOffButton);
		return container;
	}

	function _buildQuestSection()
	{
		local container = this.GUI.Container();
		this.mQuestInputArea = this.GUI.InputArea();
		this.mQuestInputArea.setSize(150, 15);
		local addButton = this.GUI.Button("Add");
		addButton.addActionListener(this);
		addButton.setData("add");
		addButton.setReleaseMessage("onQuestCommand");
		local removeButton = this.GUI.Button("Remove");
		removeButton.addActionListener(this);
		removeButton.setData("remove");
		removeButton.setReleaseMessage("onQuestCommand");
		local completeButton = this.GUI.Button("Complete");
		completeButton.addActionListener(this);
		completeButton.setData("complete");
		completeButton.setReleaseMessage("onQuestCommand");
		container.add(this.GUI.Label("Quest Id/Name"));
		container.add(this.mQuestInputArea);
		container.add(addButton);
		container.add(removeButton);
		container.add(completeButton);
		return container;
	}

	function onQuestCommand( button )
	{
		local selectedTarget = ::_avatar.getTargetObject();

		if (selectedTarget && button && button.getData())
		{
			local data = button.getData();
			local command = data + " " + this.mQuestInputArea.getText();
			::_Connection.sendQuery("quest.hack", this, [
				data,
				this.mQuestInputArea.getText(),
				"SELECT_TARGET"
			]);
		}
		else
		{
			this.IGIS.info("Please select a target to change their quest information.");
		}
	}

}


if (wasVisible)
{
	this.Screens.toggle("GMScreen");
}
