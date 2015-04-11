this.require("GUI/FullScreenComponent");
this.require("GUI/BigButton");
this.require("GUI/Spacer");
this.require("GUI/HTML");
class this.GUI.QuestionWindow extends this.GUI.FullScreenComponent
{
	constructor( ... )
	{
		::GUI.FullScreenComponent.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(0.5);
		this.getLayoutManager().setPackAlignment(0.5);
		this.mPanel = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mPanel.getLayoutManager().setExpand(true);
		this.mPanel.setInsets(5, 10, 5, 10);
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
		this.mInput = this.GUI.InputArea();
		this.mInput.setHeight(25);
		this.mPanel.add(this.mInput);
		this.mButtonHolder = this.GUI.Component(this.GUI.BoxLayout());
		this.mButtonHolder.setInsets(10, 0, 15, 0);
		this.mButtonHolder.setAppearance("Container");
		this.mButtonHolder.getLayoutManager().setAlignment(0.5);
		this.mButtonHolder.getLayoutManager().setPackAlignment(0.5);
		this.mPanel.add(this.mButtonHolder);
		local spacer = this.GUI.Spacer(10, 10);
		this.mButtonHolder.add(spacer);
		this.setAppearance("PanelTransparent");
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setOverlay("GUI/FullScreenComponentOverlay");
	}

	function setupButtons()
	{
		this.mOKButton = this.GUI.BigButton("OK");
		this.mCancelButton = this.GUI.BigButton("Cancel");

		if (this.mOKButton)
		{
			this.mOKButton.setReleaseMessage("onOK");
			this.mOKButton.addActionListener(this);
			this.mOKButton.setVisible(true);
			this.mButtonHolder.add(this.mOKButton);
		}

		if (this.mCancelButton)
		{
			this.mCancelButton.setReleaseMessage("onCancel");
			this.mCancelButton.addActionListener(this);
			this.mCancelButton.setVisible(true);
			this.mButtonHolder.add(this.mCancelButton);
		}
	}

	static function show( text, listener )
	{
		local cw = this.GUI.QuestionWindow();
		cw.setupButtons();
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
		this.mMessageBroadcaster.broadcastMessage("onCancelInput", this, this.mInput.getText());
		this.closeWindow();
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
		this.mMessageBroadcaster.broadcastMessage("onInput", this, this.mInput.getText());
	}

	function setText( pString )
	{
		if (pString != this.mHTML.mText)
		{
			this.mHTML.setText(pString);
		}
	}

	mPanel = null;
	mHTML = null;
	mButtonHolder = null;
	mConfirmationType = 0;
	mOKButton = null;
	mCancelButton = null;
	mInput = null;
	static mClassName = "QuestionWindow";
}

