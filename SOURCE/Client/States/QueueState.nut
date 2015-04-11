this.require("UI/Screens");
class this.Screens.Queue extends this.GUI.FullScreenComponent
{
	static mClassName = "QueueState";
	mShardFullLabel = null;
	mQueuePositionText = "<font size=\"24\" color=\"dddddd\">Position in queue: </font>";
	mPlayersWaitingLabel = null;
	mChooseShardButton = null;
	mQueuePosition = -1;
	mSelectedCharacter = -1;
	mQuestion = null;
	mLoginQueueScreen = null;
	mPersona = null;
	mPanel = null;
	constructor()
	{
		::GUI.FullScreenComponent.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(0.5);
		this.getLayoutManager().setPackAlignment(0.5);
		this.setAppearance("PanelTransparent");
		this._buildScreen();
	}

	function _buildScreen()
	{
		this.setInsets(5, 10, 5, 10);
		this.mPanel = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mPanel.getLayoutManager().setExpand(false);
		this.mPanel.getLayoutManager().setAlignment(0.5);
		this.mPanel.getLayoutManager().setGap(7);
		this.mPanel.setInsets(5, 10, 5, 10);
		this.mPanel.setAppearance("Panel");
		this.add(this.mPanel);
		this.mShardFullLabel = this.GUI.HTML("");
		this.mShardFullLabel.setMaximumSize(400, null);
		this.mShardFullLabel.setResize(true);
		this.mPlayersWaitingLabel = this.GUI.HTML(this.mQueuePositionText);
		this.mChooseShardButton = this.GUI.NarrowButton("Cancel");
		this.mChooseShardButton.setReleaseMessage("cancel");
		this.mChooseShardButton.addActionListener(this);
		this.mPanel.add(this.mShardFullLabel);
		this.mPanel.add(this.mPlayersWaitingLabel);
		this.mPanel.add(this.mChooseShardButton);
		this.setOverlay("GUI/QueueScreen");
		this.setVisible(true);
	}

	function cancel( evt )
	{
		if (this.mQuestion == null)
		{
			this.mQuestion = ::GUI.MessageBox.showYesNo("Cancelling will disconnect you from the game. Are you sure want to disconnect?", this);
			this.mQuestion.setOverlay("GUI/QueueScreen");
			this.mPanel.setVisible(false);
		}
	}

	function onShow()
	{
		this.requestKeyboardFocus();

		if (this.mQuestion == null)
		{
			this.mPanel.setVisible(true);
		}
	}

	function onActionSelected( window, text )
	{
		this.mPanel.setVisible(true);
		this.mQuestion = null;

		if (text == "Yes")
		{
			::_Connection.close();
		}
	}

	function onClose()
	{
		if (this.mQuestion)
		{
			this.mQuestion.close();
			this.mQuestion = null;
		}
	}

	function setQueuePosition( queuePosition, mode )
	{
		local text = "";

		if (mode == this.QueueType.QUEUE_LOGIN)
		{
			text = "<font size=\"26\">You have been placed in the login queue.</font>";
		}
		else if (mode == this.QueueType.QUEUE_SHARD)
		{
			text = "<font size=\"26\">The shard is currently full. You have been placed in a queue until room is available.</font>";
		}
		else if (mode == this.QueueType.QUEUE_INSTANCE)
		{
			text = "<font size=\"26\">You have been placed in the instance queue.</font>";
		}
		else
		{
			text = "<font size=\"26\">You have been placed in a queue.</font>";
		}

		this.mShardFullLabel.setText(text);
		this.mQueuePosition = queuePosition;

		if (queuePosition > 0)
		{
			this.mPlayersWaitingLabel.setText("<font size=\"24\">Your current position:  </font><font size=\"24\" color=\"8aff87\">" + queuePosition + "</font>");
		}
	}

}

