require("GUI/GUI");
class GUI.ScrollButtons extends GUI.Component
{
	constructor( ... )
	{
		GUI.Component.constructor();
		setAppearance("Container");
		setSize(25, 0);
		mPageUp = GUI.SmallButton("PageUp");
		mPageUp.setLayoutExclude(true);
		mPageUp.setSticky("left", "top");
		mPageUp.setPosition(0, 0);
		mPageUp.setPressMessage("onPageUp");
		mPageUp.addActionListener(this);
		add(mPageUp);
		mLineUp = GUI.SmallButton("LineUp");
		mLineUp.setLayoutExclude(true);
		mLineUp.setSticky("left", "top");
		mLineUp.setPosition(0, 25);
		mLineUp.setPressMessage("onLineUp");
		mLineUp.addActionListener(this);
		add(mLineUp);
		mLineDown = GUI.SmallButton("LineDown");
		mLineDown.setLayoutExclude(true);
		mLineDown.setSticky("left", "bottom");
		mLineDown.setPosition(0, -48);
		mLineDown.setPressMessage("onLineDown");
		mLineDown.addActionListener(this);
		add(mLineDown);
		mPageDown = GUI.SmallButton("PageDown");
		mPageDown.setLayoutExclude(true);
		mPageDown.setSticky("left", "bottom");
		mPageDown.setPosition(0, -23);
		mPageDown.setPressMessage("onPageDown");
		mPageDown.addActionListener(this);
		add(mPageDown);
		mMessageBroadcaster = MessageBroadcaster();
	}

	function getPreferredSize()
	{
		local sz = mPageUp.getPreferredSize();
		sz.height = sz.height * 4 + 2 * 3;
		return _addInsets(sz);
	}

	function setIndex( pIndex )
	{
		if (mIndex != pIndex)
		{
			mIndex = pIndex;
			_fireActionPerformed("onScrollUpdate");
		}
	}

	function getIndex()
	{
		return mIndex;
	}

	function onPageUp( evt )
	{
		local i = mIndex;
		i -= mPageSize;

		if (i < 0)
		{
			i = 0;
		}

		if (mAttachParent)
		{
			mAttachParent.invalidate();
		}

		setIndex(i);
		mRepeatMessage = "onPageUp";
		onEnterFrame();
	}

	function onLineUp( evt, ... )
	{
		local i = mIndex;
		i -= 1;

		if (i < 0)
		{
			i = 0;
		}

		if (mAttachParent)
		{
			mAttachParent.invalidate();
		}

		setIndex(i);

		if (vargc == 0 || vargv[0])
		{
			mRepeatMessage = "onLineUp";
			onEnterFrame();
		}
	}

	function onLineDown( evt, ... )
	{
		local i = mIndex;
		i += 1;

		if (mAttachParent)
		{
			mAttachParent.invalidate();

			if ("getRows" in mAttachParent.getLayoutManager())
			{
				local rows = mAttachParent.getLayoutManager().getRows();

				if (rows && i >= rows.len() - 2)
				{
					return;
				}
			}
		}

		setIndex(i);

		if (vargc == 0 || vargv[0])
		{
			mRepeatMessage = "onLineDown";
			onEnterFrame();
		}
	}

	function onPageDown( evt )
	{
		local i = mIndex;
		i += mPageSize;

		if (mAttachParent)
		{
			mAttachParent.invalidate();

			if ("getRows" in mAttachParent.getLayoutManager())
			{
				local rows = mAttachParent.getLayoutManager().getRows();

				if (rows && i >= rows.len() - 2)
				{
					return;
				}
			}
		}

		setIndex(i);
		mRepeatMessage = "onPageDown";
		onEnterFrame();
	}

	function onEnterFrame()
	{
		if (!mTimer)
		{
			::_enterFrameRelay.addListener(this);
			mTimer = ::Timer();
			mWaitTime = 500;
			return;
		}

		if (mTimer.getMilliseconds() >= mWaitTime && mRepeatMessage != "")
		{
			mWaitTime = 100;
			mTimer.reset();
			this[mRepeatMessage](this);
		}
	}

	function onExitButton( evt )
	{
		mTimer = null;
		mRepeatMessage = "";

		if (mPageUp)
		{
			mPageUp.mPressed = false;
		}

		if (mLineUp)
		{
			mLineUp.mPressed = false;
		}

		if (mLineDown)
		{
			mLineDown.mPressed = false;
		}

		if (mPageDown)
		{
			mPageDown.mPressed = false;
		}
	}

	function _recalc()
	{
		if (mAttachParent)
		{
			local container = mAttachParent.mParentComponent;
			local sz = mAttachParent.getSize();
			local pt = mAttachParent.getPosition();
			setSize(mWidth, sz.height - mIndent * 2);
			setPosition(pt.x + sz.width + mGap, pt.y + mIndent);
		}
	}

	function attach( pAttachParent )
	{
		mAttachParent = pAttachParent;
		addActionListener(mAttachParent);
		mAttachParent.setScroll(this);
		local container = mAttachParent.mParentComponent;

		if (container)
		{
			container.add(this);
		}
		else
		{
			setOverlay(mAttachParent.getOverlay());
		}

		invalidate();
	}

	function onActionPerformed( evt )
	{
		::_enterFrameRelay.removeListener(this);
		mTimer = null;
		mWaitTime = 0;
		mRepeatMessage = "";
	}

	function addActionListener( listener )
	{
		mMessageBroadcaster.addListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function validate()
	{
		GUI.Component.validate();
		_recalc();
	}

	function setGap( pGap )
	{
		mGap = pGap;
		invalidate();
	}

	function setIndent( pIndent )
	{
		mIndent = pIndent;
		invalidate();
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

