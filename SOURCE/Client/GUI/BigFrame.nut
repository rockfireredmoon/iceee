this.require("GUI/Panel");
class this.GUI.BigTitleBar extends this.GUI.Panel
{
	mFrame = null;
	mTitle = null;
	mMouseOffset = {};
	mDragging = false;
	mLastPoint = null;
	mCloseButton = null;
	mButtonOffset = null;
	static mClassName = "BigTitleBar";
	static BIG_FRAME_HEIGHT = 51;
	static SIDE_INSET_SIZE = 63;
	static BUTTON_COMPONENT_SIZE = 40;
	constructor( vFrame, vTitle, ... )
	{
		this.GUI.Panel.constructor(null);
		this.mFrame = vFrame;

		if (vargc > 1)
		{
			this.mButtonOffset = vargv[1];
		}

		this.setLayoutManager(this.GUI.BorderLayout());
		local component = this.GUI.Container(null);
		component.setSize(this.BUTTON_COMPONENT_SIZE, this.BUTTON_COMPONENT_SIZE);
		component.setPreferredSize(this.BUTTON_COMPONENT_SIZE, this.BUTTON_COMPONENT_SIZE);
		this.mCloseButton = this.GUI.CloseButton();
		this.mCloseButton.setPosition(5, 9.5);
		this.mCloseButton.setTooltip("Close");
		component.add(this.mCloseButton);
		this.mCloseButton.addActionListener({
			frame = vFrame,
			function onActionPerformed( button )
			{
				this.frame.onClosePressed();
			}

		});

		if (vargc > 0)
		{
			this.mCloseButton.setVisible(vargv[0]);
		}

		local width = this.mFrame.getWidth();
		this.setSize(width, this.BIG_FRAME_HEIGHT);
		this.setPreferredSize(width, this.BIG_FRAME_HEIGHT);
		this.setAppearance("BigFrameTitleBar");
		this.mTitle = this.GUI.Label(vTitle);
		this.mTitle.setFontColor(this.Colors.white);
		this.mTitle.setTextAlignment(0.5, 0.5);
		this.mTitle.setFont(this.GUI.Font("Maiandra", 28));
		this.mTitle.setInsets(5, this.SIDE_INSET_SIZE - this.BUTTON_COMPONENT_SIZE, 11, this.SIDE_INSET_SIZE);
		this.setPassThru(false);

		if (this.mButtonOffset == null)
		{
			this.add(this.mTitle, this.GUI.BorderLayout.CENTER);
			this.add(component, this.GUI.BorderLayout.EAST);
		}
		else
		{
			this.add(this.mTitle, this.GUI.BorderLayout.CENTER);
			this.add(this.GUI.Spacer(this.BUTTON_COMPONENT_SIZE, this.BUTTON_COMPONENT_SIZE), this.GUI.BorderLayout.EAST);
			this.add(component);
			component.setPosition(this.mButtonOffset.x, this.mButtonOffset.y);
			component.setLayoutExclude(true);
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
		this.mWidget.setChildProcessingEvents(true);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Panel._removeNotify();
	}

	function setTitle( vTitle )
	{
		this.mTitle.setText(vTitle);
	}

	function setTitleFont( font )
	{
		this.mTitle.setFont(font);
	}

	function onMousePressed( evt )
	{
		if (this.mFrame.getMovable())
		{
			this.mLastPoint = ::Screen.getCursorPos();
			this.mMouseOffset = {
				x = evt.x,
				y = evt.y
			};
			this.mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mFrame.getMovable())
		{
			this.mDragging = false;
			evt.consume();
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mFrame.getMovable())
		{
			if (this.mDragging)
			{
				local newpos = ::Screen.getCursorPos();
				local deltax = newpos.x - this.mLastPoint.x;
				local deltay = newpos.y - this.mLastPoint.y;
				local pos = this.mFrame.getPosition();
				pos.x += deltax;
				pos.y += deltay;
				this.mLastPoint = newpos;
				this.mFrame.setPosition(pos);
				this.mFrame.fitToScreen();
				local newPos = this.mFrame.getPosition();

				if ("onFrameMoved" in this.mFrame)
				{
					this.mFrame.onFrameMoved(newPos);
				}
			}

			evt.consume();
		}
	}

}

class this.GUI.BigFrame extends this.GUI.Frame
{
	mTitleBar = null;
	mContentPane = null;
	mMovable = true;
	static mClassName = "BigFrame";
	constructor( title, ... )
	{
		this.GUI.Component.constructor(null);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setInsets(0);
		local closeButtonVisible = true;

		if (vargc > 0)
		{
			closeButtonVisible = vargv[0];
		}

		local closeButtonOffset;

		if (vargc > 1)
		{
			closeButtonOffset = vargv[1];
		}

		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mTitleBar = this.GUI.BigTitleBar(this, title, closeButtonVisible, closeButtonOffset);
		this.add(this.mTitleBar, this.GUI.BorderLayout.NORTH);
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

}

