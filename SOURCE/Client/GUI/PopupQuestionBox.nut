this.require("GUI/InputBox");
class this.GUI.PopupQuestionBox extends this.GUI.Panel
{
	mEntryPrompt = "";
	mAcceptActionName = null;
	mCancelActionName = null;
	mAcceptButton = null;
	mCancelButton = null;
	mData = null;
	mBroadcaster = null;
	constructor( prompt )
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.mBroadcaster = this.MessageBroadcaster();
		this.mEntryPrompt = prompt;
		local text = "<font size=\"24\">" + this.mEntryPrompt + "</font>";
		local label = this.GUI.HTML("");
		label.setMaximumSize(550, null);
		label.setResize(true);
		label.setText(text);
		local buttonContainer = this.GUI.Container(this.GUI.BoxLayout());
		buttonContainer.getLayoutManager().setPackAlignment(0.5);
		buttonContainer.getLayoutManager().setGap(6);
		buttonContainer.setInsets(8, 8, 8, 4);
		this.mAcceptActionName = "onAccept";
		this.mCancelActionName = null;
		this.mAcceptButton = this.GUI.Button("Accept");
		this.mAcceptButton.addActionListener(this);
		this.mAcceptButton.setReleaseMessage("onAccept");
		this.mCancelButton = this.GUI.Button("Cancel");
		this.mCancelButton.addActionListener(this);
		this.mCancelButton.setReleaseMessage("onCancel");
		buttonContainer.add(this.mAcceptButton);
		buttonContainer.add(this.mCancelButton);
		this.add(label, this.GUI.BorderLayout.CENTER);
		this.add(buttonContainer, this.GUI.BorderLayout.SOUTH);
		local size = this.getPreferredSize();

		if (size.width < 375)
		{
			size.width = 375;
		}

		this.setSize(size);
		this.setInsets(8, 8, 8, 8);
		this.GUI._Manager.addTransientToplevel(this);
	}

	function addActionListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function setAcceptActionName( name )
	{
		this.mAcceptActionName = name;
	}

	function setCancelActionName( name )
	{
		this.mCancelActionName = name;
	}

	function getInputArea()
	{
		return this.mInputArea;
	}

	function getText()
	{
		return this.mInputArea.getText();
	}

	function hidePopup()
	{
		this.setOverlay(null);
	}

	function onAccept( button )
	{
		this.mBroadcaster.broadcastMessage(this.mAcceptActionName, this);
	}

	function setData( data )
	{
		this.mData = data;
	}

	function getData()
	{
		return this.mData;
	}

	function center()
	{
		local inputBoxPosX = ::Screen.getWidth() / 2 - this.getSize().width / 2;
		local inputBoxPosY = ::Screen.getHeight() / 2 - this.getSize().height / 2;
		this.setPosition(inputBoxPosX, inputBoxPosY);
	}

	function onCancel( button )
	{
		if (this.mCancelActionName != null)
		{
			this.mBroadcaster.broadcastMessage(this.mCancelActionName, this);
		}

		this.hidePopup();
	}

	function onOutsideClick( evt )
	{
		this.hidePopup();
	}

	function removeActionListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function setPosFromCursor()
	{
		local cursorPos = this.Screen.getCursorPos();
		local fx = cursorPos.x;
		local fy = cursorPos.y;
		fx = fx >= 0 ? fx : 0;
		fy = fy >= 0 ? fy : 0;

		if (fx + this.getWidth() > ::Screen.getWidth())
		{
			fx = ::Screen.getWidth() - this.getWidth();
		}

		if (fy + this.getHeight() > ::Screen.getHeight())
		{
			fy = ::Screen.getHeight() - this.getHeight();
		}

		this.setPosition(fx, fy);
	}

	function showInputBox()
	{
		this.setOverlay(this.GUI.POPUP_OVERLAY);
	}

}

