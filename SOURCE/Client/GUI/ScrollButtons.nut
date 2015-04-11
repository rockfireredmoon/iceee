this.require("GUI/GUI");
class this.GUI.ScrollButtons extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setAppearance("Container");
		this.setSize(25, 0);
		this.mPageUp = this.GUI.SmallButton("PageUp");
		this.mPageUp.setLayoutExclude(true);
		this.mPageUp.setSticky("left", "top");
		this.mPageUp.setPosition(0, 0);
		this.mPageUp.setPressMessage("onPageUp");
		this.mPageUp.addActionListener(this);
		this.add(this.mPageUp);
		this.mLineUp = this.GUI.SmallButton("LineUp");
		this.mLineUp.setLayoutExclude(true);
		this.mLineUp.setSticky("left", "top");
		this.mLineUp.setPosition(0, 25);
		this.mLineUp.setPressMessage("onLineUp");
		this.mLineUp.addActionListener(this);
		this.add(this.mLineUp);
		this.mLineDown = this.GUI.SmallButton("LineDown");
		this.mLineDown.setLayoutExclude(true);
		this.mLineDown.setSticky("left", "bottom");
		this.mLineDown.setPosition(0, -48);
		this.mLineDown.setPressMessage("onLineDown");
		this.mLineDown.addActionListener(this);
		this.add(this.mLineDown);
		this.mPageDown = this.GUI.SmallButton("PageDown");
		this.mPageDown.setLayoutExclude(true);
		this.mPageDown.setSticky("left", "bottom");
		this.mPageDown.setPosition(0, -23);
		this.mPageDown.setPressMessage("onPageDown");
		this.mPageDown.addActionListener(this);
		this.add(this.mPageDown);
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function getPreferredSize()
	{
		local sz = this.mPageUp.getPreferredSize();
		sz.height = sz.height * 4 + 2 * 3;
		return this._addInsets(sz);
	}

	function setIndex( pIndex )
	{
		if (this.mIndex != pIndex)
		{
			this.mIndex = pIndex;
			this._fireActionPerformed("onScrollUpdate");
		}
	}

	function getIndex()
	{
		return this.mIndex;
	}

	function onPageUp( evt )
	{
		local i = this.mIndex;
		i -= this.mPageSize;

		if (i < 0)
		{
			i = 0;
		}

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();
		}

		this.setIndex(i);
		this.mRepeatMessage = "onPageUp";
		this.onEnterFrame();
	}

	function onLineUp( evt, ... )
	{
		local i = this.mIndex;
		i -= 1;

		if (i < 0)
		{
			i = 0;
		}

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();
		}

		this.setIndex(i);

		if (vargc == 0 || vargv[0])
		{
			this.mRepeatMessage = "onLineUp";
			this.onEnterFrame();
		}
	}

	function onLineDown( evt, ... )
	{
		local i = this.mIndex;
		i += 1;

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();

			if ("getRows" in this.mAttachParent.getLayoutManager())
			{
				local rows = this.mAttachParent.getLayoutManager().getRows();

				if (rows && i >= rows.len() - 2)
				{
					return;
				}
			}
		}

		this.setIndex(i);

		if (vargc == 0 || vargv[0])
		{
			this.mRepeatMessage = "onLineDown";
			this.onEnterFrame();
		}
	}

	function onPageDown( evt )
	{
		local i = this.mIndex;
		i += this.mPageSize;

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();

			if ("getRows" in this.mAttachParent.getLayoutManager())
			{
				local rows = this.mAttachParent.getLayoutManager().getRows();

				if (rows && i >= rows.len() - 2)
				{
					return;
				}
			}
		}

		this.setIndex(i);
		this.mRepeatMessage = "onPageDown";
		this.onEnterFrame();
	}

	function onEnterFrame()
	{
		if (!this.mTimer)
		{
			::_enterFrameRelay.addListener(this);
			this.mTimer = ::Timer();
			this.mWaitTime = 500;
			return;
		}

		if (this.mTimer.getMilliseconds() >= this.mWaitTime && this.mRepeatMessage != "")
		{
			this.mWaitTime = 100;
			this.mTimer.reset();
			this[this.mRepeatMessage](this);
		}
	}

	function onExitButton( evt )
	{
		this.mTimer = null;
		this.mRepeatMessage = "";

		if (this.mPageUp)
		{
			this.mPageUp.mPressed = false;
		}

		if (this.mLineUp)
		{
			this.mLineUp.mPressed = false;
		}

		if (this.mLineDown)
		{
			this.mLineDown.mPressed = false;
		}

		if (this.mPageDown)
		{
			this.mPageDown.mPressed = false;
		}
	}

	function _recalc()
	{
		if (this.mAttachParent)
		{
			local container = this.mAttachParent.mParentComponent;
			local sz = this.mAttachParent.getSize();
			local pt = this.mAttachParent.getPosition();
			this.setSize(this.mWidth, sz.height - this.mIndent * 2);
			this.setPosition(pt.x + sz.width + this.mGap, pt.y + this.mIndent);
		}
	}

	function attach( pAttachParent )
	{
		this.mAttachParent = pAttachParent;
		this.addActionListener(this.mAttachParent);
		this.mAttachParent.setScroll(this);
		local container = this.mAttachParent.mParentComponent;

		if (container)
		{
			container.add(this);
		}
		else
		{
			this.setOverlay(this.mAttachParent.getOverlay());
		}

		this.invalidate();
	}

	function onActionPerformed( evt )
	{
		::_enterFrameRelay.removeListener(this);
		this.mTimer = null;
		this.mWaitTime = 0;
		this.mRepeatMessage = "";
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function validate()
	{
		this.GUI.Component.validate();
		this._recalc();
	}

	function setGap( pGap )
	{
		this.mGap = pGap;
		this.invalidate();
	}

	function setIndent( pIndent )
	{
		this.mIndent = pIndent;
		this.invalidate();
	}

	mPageUp = null;
	mLineUp = null;
	mLineDown = null;
	mPageDown = null;
	mAttachParent = null;
	mRepeatMessage = "";
	mWaitTime = 0;
	mTimer = null;
	mPageSize = 5;
	mIndex = 0;
	mGap = 0;
	mIndent = 0;
	static mClassName = "ScrollButtons";
}

