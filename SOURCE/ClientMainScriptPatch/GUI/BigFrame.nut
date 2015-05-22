require("GUI/Panel");

class GUI.BigTitleBar extends GUI.Panel
{
	mFrame = null;
	mTitle = null;
	mMouseOffset = {};
	mDragging = false;
	mLastPoint = null;
	mCloseButton = null;
	mButtonOffset = null;
	
	static mClassName = "BigTitleBar";
	
	// This is the normal height of the title bar as in art
	static BIG_FRAME_HEIGHT = 51;
	static SIDE_INSET_SIZE = 63;
	static BUTTON_COMPONENT_SIZE = 40;
	
	constructor( vFrame, vTitle, ... )
	{
		GUI.Panel.constructor(null);
		mFrame = vFrame;

		if (vargc > 1)
			mButtonOffset = vargv[1];

		setLayoutManager(GUI.BorderLayout());
		local component = GUI.Container(null);
		component.setSize(BUTTON_COMPONENT_SIZE, BUTTON_COMPONENT_SIZE);
		component.setPreferredSize(BUTTON_COMPONENT_SIZE, BUTTON_COMPONENT_SIZE);
		
		mCloseButton = GUI.CloseButton();
		mCloseButton.setPosition(5, 9.5);
		mCloseButton.setTooltip("Close");
		component.add(mCloseButton);
		mCloseButton.addActionListener({
			// Anonymous closure that will close this frame
			frame = vFrame,
			function onActionPerformed( button )
			{
				frame.onClosePressed();
			}

		});

		if (vargc > 0)
			mCloseButton.setVisible(vargv[0]);

		local width = mFrame.getWidth();
		
		setSize(width, BIG_FRAME_HEIGHT);
		setPreferredSize(width, BIG_FRAME_HEIGHT);
		
		setAppearance("BigFrameTitleBar");
		
		mTitle = GUI.Label(vTitle);
		mTitle.setFontColor(Colors.white);
		mTitle.setTextAlignment(0.5, 0.5);
		mTitle.setFont(GUI.Font("Maiandra", 28));
		mTitle.setInsets(5, SIDE_INSET_SIZE - BUTTON_COMPONENT_SIZE, 11, SIDE_INSET_SIZE);
		setPassThru(false);

		if (mButtonOffset == null) {
			add(mTitle, GUI.BorderLayout.CENTER);
			add(component, GUI.BorderLayout.EAST);
		}
		else {
			add(mTitle, GUI.BorderLayout.CENTER);
			add(GUI.Spacer(BUTTON_COMPONENT_SIZE, BUTTON_COMPONENT_SIZE), GUI.BorderLayout.EAST);
			add(component);
			component.setPosition(mButtonOffset.x, mButtonOffset.y);
			component.setLayoutExclude(true);
		}
	}

	function _addNotify() {
		GUI.Panel._addNotify();
		mWidget.addListener(this);
		mWidget.setChildProcessingEvents(true);
	}

	function _removeNotify() {
		mWidget.removeListener(this);
		GUI.Panel._removeNotify();
	}

	function setTitle( vTitle )	{
		mTitle.setText(vTitle);
	}

	function setTitleFont( font ) {
		mTitle.setFont(font);
	}

	function onMousePressed( evt )
	{
		if (mFrame.getMovable()) {
			mLastPoint = ::Screen.getCursorPos();
			mMouseOffset = {
				x = evt.x,
				y = evt.y
			};
			mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )	{
		if (mFrame.getMovable()) {
			mDragging = false;
			evt.consume();
		}
	}

	function onMouseMoved( evt ) {
		if (mFrame.getMovable()) {
			if (mDragging) {
				local newpos = ::Screen.getCursorPos();
				local deltax = newpos.x - mLastPoint.x;
				local deltay = newpos.y - mLastPoint.y;
				local pos = mFrame.getPosition();
				pos.x += deltax;
				pos.y += deltay;
				mLastPoint = newpos;
				mFrame.setPosition(pos);
				mFrame.fitToScreen();
				local newPos = mFrame.getPosition();

				if ("onFrameMoved" in mFrame)
					mFrame.onFrameMoved(newPos);
			}

			evt.consume();
		}
	}

}

class GUI.BigFrame extends GUI.Frame {
	mTitleBar = null;
	mContentPane = null;
	mMovable = true;
	static mClassName = "BigFrame";
	constructor( title, ... ) {
		GUI.Component.constructor(null);
		setLayoutManager(GUI.BorderLayout());
		setInsets(0);
		local closeButtonVisible = true;

		if (vargc > 0)
			closeButtonVisible = vargv[0];

		local closeButtonOffset;

		if (vargc > 1)
			closeButtonOffset = vargv[1];

		mMessageBroadcaster = MessageBroadcaster();
		mTitleBar = GUI.BigTitleBar(this, title, closeButtonVisible, closeButtonOffset);
		add(mTitleBar, GUI.BorderLayout.NORTH);
		setVisible(false);
		setOverlay(GUI.OVERLAY);
	}

}

