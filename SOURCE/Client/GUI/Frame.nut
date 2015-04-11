this.require("GUI/Panel");
this.require("GUI/CloseButton");
this.require("GUI/InnerPanel");
class this.GUI.TitleBar extends this.GUI.Panel
{
	mFrame = null;
	mTitle = null;
	mCloseButton = null;
	mMouseOffset = {};
	mDragging = false;
	mLastPoint = null;
	constructor( vFrame, vTitle, ... )
	{
		this.GUI.Panel.constructor();
		this.setAppearance("FrameTopBar");
		this.mFrame = vFrame;
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setInsets(0);
		this.mCloseButton = this.GUI.CloseButton();
		this.mCloseButton.setTooltip("Close");
		this.mCloseButton.addActionListener({
			frame = vFrame,
			function onActionPerformed( button )
			{
				this.frame.onClosePressed();
			}

		});
		this.mCloseButton.setFixedSize(18, 18);

		if (vargc > 0)
		{
			this.mCloseButton.setVisible(vargv[0]);
		}

		this.mTitle = this.GUI.Label(vTitle);
		this.mTitle.setFontColor(this.Colors.white);
		this.mTitle.setTextAlignment(0.5, 0.5);
		this.setPassThru(false);
		this.mDebugName = "TitleBar";
		this.setInsets(2, 23, 5, 30);
		this.add(this.mTitle, this.GUI.BorderLayout.CENTER);
		this.add(this.mCloseButton, this.GUI.BorderLayout.EAST);
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
		if (this.mFrame && this.mFrame.getMovable())
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
		if (this.mFrame && this.mFrame.getMovable())
		{
			this.mDragging = false;
			evt.consume();
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mFrame && this.mFrame.getMovable())
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

class this.GUI.Frame extends this.GUI.Panel
{
	constructor( title, ... )
	{
		this.GUI.Panel.constructor();
		this.setAppearance(null);
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setInsets(0);
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			this.mTitleBar = this.GUI.TitleBar(this, title, vargv[0]);
		}
		else
		{
			this.mTitleBar = this.GUI.TitleBar(this, title);
		}

		this.add(this.mTitleBar, this.GUI.BorderLayout.NORTH);
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function setTitle( title )
	{
		this.mTitleBar.setTitle(title);
	}

	function setTitleFont( font )
	{
		this.mTitleBar.setTitleFont(font);
	}

	function setMovable( which )
	{
		this.mMovable = which;
	}

	function getMovable()
	{
		return this.mMovable;
	}

	function setContentPane( component )
	{
		if (this.mContentPane)
		{
			this.remove(this.mContentPane);
		}

		this.mContentPane = component;
		this.mContentPane.setAppearance("GoldBottomFrame");

		if (this.mContentPane != null)
		{
			this.add(this.mContentPane, this.GUI.BorderLayout.CENTER);
		}
	}

	function getContentPane()
	{
		if (this.mContentPane == null)
		{
			local contentPane = this.GUi.Container();
			contentPane.setAppearance("GoldBottomFrame");
			this.setContentPane(contentPane);
		}

		return this.mContentPane;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mTitleBar.mWidget.addListener(this);
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mTitleBar.mWidget != null)
		{
			this.mTitleBar.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setCloseMessage( msg )
	{
		this.mCloseMessage = msg;
	}

	function addActionListener( obj )
	{
		this.mMessageBroadcaster.addListener(obj);
	}

	function removeActionListener( obj )
	{
		this.mMessageBroadcaster.removeListener(obj);
	}

	function close()
	{
		this.setVisible(false);
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function onClosePressed()
	{
		this.mMessageBroadcaster.broadcastMessage(this.mCloseMessage);
		this.close();
	}

	function onFrameMoved( newPos )
	{
	}

	function center()
	{
		this.setPosition(::Screen.getWidth() / 2.0 - this.getWidth() / 2.0, ::Screen.getHeight() / 2.0 - this.getHeight() / 2.0);
	}

	mTitleBar = null;
	mContentPane = null;
	mMovable = true;
	mCloseMessage = "onFrameClosed";
	static mClassName = "Frame";
}

class this.GUI.ContainerFrame extends this.GUI.Container
{
	constructor( title )
	{
		this.GUI.Container.constructor();
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setInsets(0);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mTitleBar = this.GUI.TitleBar(this, title);
		this.add(this.mTitleBar, this.GUI.BorderLayout.NORTH);
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function setTitle( title )
	{
		this.mTitleBar.setTitle(title);
	}

	function setMovable( which )
	{
		this.mMovable = which;
	}

	function getMovable()
	{
		return this.mMovable;
	}

	function setContentPane( component )
	{
		if (this.mContentPane)
		{
			this.remove(this.mContentPane);
		}

		this.mContentPane = component;

		if (this.mContentPane != null)
		{
			this.add(this.mContentPane, this.GUI.BorderLayout.CENTER);
		}
	}

	function getContentPane()
	{
		if (this.mContentPane == null)
		{
			this.setContentPane(this.GUI.Container());
		}

		return this.mContentPane;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mTitleBar.mWidget.addListener(this);
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mTitleBar.mWidget != null)
		{
			this.mTitleBar.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setCloseMessage( msg )
	{
		this.mCloseMessage = msg;
	}

	function addActionListener( obj )
	{
		this.mMessageBroadcaster.addListener(obj);
	}

	function removeActionListener( obj )
	{
		this.mMessageBroadcaster.removeListener(obj);
	}

	function close()
	{
		this.setVisible(false);
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function onClosePressed()
	{
		this.mMessageBroadcaster.broadcastMessage(this.mCloseMessage);
		this.close();
	}

	mTitleBar = null;
	mContentPane = null;
	mMovable = true;
	mCloseMessage = "onFrameClosed";
	static mClassName = "ContainerFrame";
}

