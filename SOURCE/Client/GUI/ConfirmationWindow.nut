this.require("GUI/FullScreenComponent");
this.require("GUI/BigButton");
this.require("GUI/Spacer");
this.require("GUI/HTML");
class this.GUI.ConfirmationWindow extends this.GUI.FullScreenComponent
{
	constructor( ... )
	{
		::GUI.FullScreenComponent.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(0.5);
		this.getLayoutManager().setPackAlignment(0.5);
		this.mPanel = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mPanel.getLayoutManager().setExpand(true);
		this.mPanel.setAppearance("Panel");
		this.add(this.mPanel);
		this.mHTML = this.GUI.HTML("");
		this.mHTML.setInsets(30, 30, 5, 30);
		this.mHTML.setResize(true);
		this.mHTML.setMaximumSize(500, null);
		this.mHTML.getLayoutManager().setAlignment("center");
		this.mHTML.setFont(this.GUI.Font("Maiandra", 32));
		this.mHTML.setText("No text set!");
		this.mPanel.add(this.mHTML);
		this.mImagePanel = this.GUI.Component(this.GUI.BoxLayout());
		this.mImagePanel.getLayoutManager().setAlignment(0.5);
		this.mImagePanel.getLayoutManager().setPackAlignment(0.5);
		this.mImagePanel.setAppearance("Container");
		this.mPanel.add(this.mImagePanel);
		this.mButtonHolder = this.GUI.Component(this.GUI.BoxLayout());
		this.mButtonHolder.setInsets(5, 10, 10, 10);
		this.mButtonHolder.setAppearance("Container");
		this.mButtonHolder.getLayoutManager().setAlignment(0.5);
		this.mButtonHolder.getLayoutManager().setPackAlignment(0.5);
		this.mPanel.add(this.mButtonHolder);
		this.setAppearance("PanelTransparent");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setOverlay("GUI/FullScreenComponentOverlay");
	}

	function setImage( image )
	{
		if (this.mImage)
		{
			this.mImagePanel.remove(this.mImage);
		}

		if (image)
		{
			this.mImagePanel.add(image);
		}

		this.mImage = image;
	}

	function setConfirmationType( pType )
	{
		this.mConfirmationType = pType;
		local buttonFound = false;

		if (this.mOKButton)
		{
			this.mButtonHolder.remove(this.mOKButton);
			this.mOKButton = null;
			buttonFound = true;
		}

		if (this.mCancelButton)
		{
			this.mButtonHolder.remove(this.mCancelButton);
			this.mCancelButton = null;
			buttonFound = true;
		}

		switch(pType)
		{
		case 0:
			this.mOKButton = this.GUI.BigButton("OK");
			this.mCancelButton = this.GUI.BigButton("Cancel");
			break;

		case 1:
			this.mOKButton = this.GUI.BigButton("OK");
			break;

		case 2:
			this.mCancelButton = this.GUI.BigButton("Cancel");
			break;

		case 3:
			this.mOKButton = this.GUI.BigButton("Yes");
			this.mCancelButton = this.GUI.BigButton("No");
			break;
		}

		if (this.mOKButton)
		{
			this.mOKButton.setReleaseMessage("onOK");
			this.mOKButton.addActionListener(this);
			this.mOKButton.setVisible(true);
			this.mButtonHolder.add(this.mOKButton);
			this.requestKeyboardFocus();
		}

		if (this.mCancelButton)
		{
			this.mCancelButton.setReleaseMessage("onCancel");
			this.mCancelButton.addActionListener(this);
			this.mCancelButton.setVisible(true);
			this.mButtonHolder.add(this.mCancelButton);
		}
	}

	function showOk( text, ... )
	{
		local cw = this.GUI.ConfirmationWindow();
		cw.setConfirmationType(this.GUI.ConfirmationWindow.OK);
		cw.setText(text);

		if (vargc > 0)
		{
			cw.addActionListener(vargv[0]);
		}

		return cw;
	}

	function showOkCancel( text, listener )
	{
		local cw = this.GUI.ConfirmationWindow();
		cw.setConfirmationType(this.GUI.ConfirmationWindow.OK_CANCEL);
		cw.setText(text);
		cw.addActionListener(listener);
		return cw;
	}

	function onOK( evt )
	{
		this._fireActionPerformed(true);
	}

	function onCancel( evt )
	{
		this._fireActionPerformed(false);
	}

	function closeWindow()
	{
		this.setOverlay(null);
		return null;
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _fireActionPerformed( pBool )
	{
		this.closeWindow();
		this.mMessageBroadcaster.broadcastMessage(this.mEventName, this, pBool);
	}

	function setText( pString )
	{
		if (pString != this.mHTML.mText)
		{
			this.mHTML.setText(pString);
			this.setConfirmationType(this.mConfirmationType);
		}
	}

	function setEventName( name )
	{
		this.mEventName = name;
	}

	function getHTML()
	{
		return this.mHTML;
	}

	static OK_CANCEL = 0;
	static OK = 1;
	static CANCEL = 2;
	static YES_NO = 3;
	static NONE = 4;
	mEventName = "onConfirmation";
	mPanel = null;
	mHTML = null;
	mButtonHolder = null;
	mConfirmationType = 0;
	mOKButton = null;
	mCancelButton = null;
	mImagePanel = null;
	mImage = null;
	static mClassName = "ConfirmationWindow";
}

