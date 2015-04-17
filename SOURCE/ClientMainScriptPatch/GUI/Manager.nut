this.require("GUI/GUI");
class this.GUI.Manager 
{
	mValidateQueue = [];
	mKeyboardFocusComponent = null;
	mCurrentDragEvent = null;
	mDragStartX = 0;
	mDragStartY = 0;
	mDragStartWidgetName = null;
	mDragHoverWidgetName = null;
	mTransientTops = null;
	mTooltipHoverComponent = null;
	mTooltipTimer = null;
	mTooltips = [];
	mMouseCaptureList = null;
	constructor()
	{
		this.mTransientTops = [];
		this.mMouseCaptureList = [];
		this._exitFrameRelay.addListener(this);
		this.Screen.setInputEventFilter(this);
	}

	function addMouseCapturer( capturer )
	{
		this.mMouseCaptureList.push(capturer);
	}

	function removeMouseCapturer( capturer )
	{
		for( local i = 0; i < this.mMouseCaptureList.len(); i++ )
		{
			if (this.mMouseCaptureList[i] == capturer)
			{
				this.mMouseCaptureList.remove(i);
				return;
			}
		}
	}

	function onExitFrame()
	{
		local c;

		while (this.mValidateQueue.len() > 0)
		{
			c = this.mValidateQueue.pop();

			if (c.isVisible())
			{
				c.validate();
			}
		}

		if (this.mTooltipTimer && this.mTooltipHoverComponent && this.mTooltipTimer.getMilliseconds() > 500)
		{
			if (this.mTooltipHoverComponent.getTooltip() != null && this.mTooltipHoverComponent.getTooltip() != "")
			{
				local tooltip = this.GUI.Tooltip(this.mTooltipHoverComponent.getTooltip());
				local childTooltips = [];
				childTooltips = this.mTooltipHoverComponent.getChildTooltips();
				local childTooltipComponents = [];
				this.mTooltips.append(tooltip);
				tooltip.validate();
				local totalTooltipWidth = tooltip.getWidth();
				local totalTooltipHeight = tooltip.getHeight();
				local largestTooltip = totalTooltipHeight;

				foreach( childTooltip in childTooltips )
				{
					local newTooltip = this.GUI.Tooltip(childTooltip);
					childTooltipComponents.append(newTooltip);
					this.mTooltips.append(newTooltip);
					newTooltip.validate();
					local tooltipHeight = newTooltip.getHeight();
					totalTooltipWidth += newTooltip.getWidth();
					totalTooltipHeight += tooltipHeight;

					if (tooltipHeight > largestTooltip)
					{
						largestTooltip = tooltipHeight;
					}
				}

				local cursorPos = this.Screen.getCursorPos();
				local fx = cursorPos.x;
				local fy = cursorPos.y - largestTooltip - 20;
				fx = fx >= 0 ? fx : 0;
				fy = fy >= 0 ? fy : 0;

				if (fx + totalTooltipWidth > ::Screen.getWidth())
				{
					fx = ::Screen.getWidth() - totalTooltipWidth;
				}

				if (fy + totalTooltipHeight > ::Screen.getHeight())
				{
					fy = ::Screen.getHeight() - totalTooltipHeight;
				}

				tooltip.setPosition(fx, fy);
				tooltip.setOverlay(this.GUI.TOOLTIP_OVERLAY);
				local nextTooltipX = fx + tooltip.getWidth();

				foreach( childTooltip in childTooltipComponents )
				{
					childTooltip.setPosition(nextTooltipX, fy);
					childTooltip.setOverlay(this.GUI.TOOLTIP_OVERLAY);
					nextTooltipX += childTooltip.getWidth();
				}

				this.mTooltipTimer = null;
			}
		}

		if (this.mCurrentDragEvent == null)
		{
			if (this.mTooltipHoverComponent)
			{
				::_cursor.setIcon(this.mTooltipHoverComponent.getCursor());
			}
			else if (::_tools.getActiveTool())
			{
				::_cursor.setIcon(::_tools.getActiveTool().getCursor());
			}
		}

		foreach( tooltip in this.mTooltips )
		{
			tooltip.onExitFrame();
		}

		for( local i = 0; i < this.mTooltips.len();  )
		{
			if (this.mTooltips[i].isFadeOutComplete())
			{
				this.mTooltips[i].destroy();
				this.mTooltips.remove(i);
			}
			else
			{
				i++;
			}
		}
	}

	function requestKeyboardFocus( component )
	{
		if (component == this.mKeyboardFocusComponent)
		{
			return;
		}

		if (this.mKeyboardFocusComponent)
		{
			if ("onReleasedKeyboardFocus" in this.mKeyboardFocusComponent)
			{
				this.mKeyboardFocusComponent.onReleasedKeyboardFocus();
			}
		}

		if (component)
		{
			if ("onRequestedKeyboardFocus" in component)
			{
				component.onRequestedKeyboardFocus();
			}

			this.mKeyboardFocusComponent = component;
		}
		else
		{
			this._root.requestKeyboardFocus();
			this.mKeyboardFocusComponent = null;
		}
	}

	function releaseKeyboardFocus( ... )
	{
		local component = this.mKeyboardFocusComponent;

		if (vargc == 1)
		{
			component = vargv[0];
		}

		if (component == this.mKeyboardFocusComponent && component)
		{
			if (this.mKeyboardFocusComponent)
			{
				this.mKeyboardFocusComponent.onReleasedKeyboardFocus();
			}

			this._root.requestKeyboardFocus();
			this.mKeyboardFocusComponent = null;
		}
	}

	function isDragging()
	{
		return this.mCurrentDragEvent != null;
	}

	function addTransientToplevel( component )
	{
		foreach( c in this.mTransientTops )
		{
			if (c == component)
			{
				return;
			}
		}

		this.mTransientTops.append(component);
	}

	function removeTransientToplevel( component )
	{
		foreach( i, c in this.mTransientTops )
		{
			if (c == component)
			{
				this.mTransientTops.remove(i);
				return;
			}
		}
	}

	static function isSameWidget( w1, w2 )
	{
		return w1 && w2 && w1.getName() == w2.getName();
	}

	function _getWidget( name )
	{
		return this.Widget(name);
		  // [006]  OP_POPTRAP        1      0    0    0
		  // [007]  OP_JMP            0      2    0    0
		return null;
	}

	function preMouseMoved( evt )
	{
		local w = evt.getWidget();

		if (w != null)
		{
			local c = w.getUserData();

			if (c != null)
			{
				local c2 = c;
				local passThruFound = false;

				while (c2 != null)
				{
					if (c2.getToolTipPassThruOverride())
					{
						passThruFound = false;
						break;
					}

					if (c2.getPassThru())
					{
						passThruFound = true;
					}

					c2 = c2.getParent();
				}

				if (true == passThruFound)
				{
					c = null;
				}

				if (c && !passThruFound && !this.mCurrentDragEvent && !evt.isLButtonDown())
				{
					::_cursor.setState(this.GUI.Cursor.DEFAULT);
				}
			}

			if (c != null && (c.getTooltip() == null || c.getTooltip == ""))
			{
				for( local testc = c; testc.getParent() != null; testc = testc.getParent() )
				{
					if (testc.getChildrenInheritTooltip() == true)
					{
						c = testc;
						break;
					}
				}
			}

			if (c != this.mTooltipHoverComponent)
			{
				for( local i = 0; i < this.mTooltips.len(); i++ )
				{
					this.mTooltips[i].fadeOut();
				}

				this.mTooltipHoverComponent = c;
				this.mTooltipTimer = ::Timer();
				this.mTooltipTimer.reset();
			}
		}
		else
		{
			for( local i = 0; i < this.mTooltips.len(); i++ )
			{
				this.mTooltips[i].fadeOut();
			}

			this.mTooltipHoverComponent = null;
			this.mTooltipTimer = null;
		}

		if (this.mCurrentDragEvent)
		{
			local w = evt.getWidget();

			if (w && w.getName() == this.mDragHoverWidgetName)
			{
				this._fireDnDEvent(w, "onDragOver");
			}
			else
			{
				this._setDragHoverWidget(w);
			}
		}
		else if (evt.isLButtonDown() && this.mDragStartWidgetName)
		{
			local w = this._getWidget(this.mDragStartWidgetName);

			if (!w)
			{
				return;
			}

			if (this.abs(evt.x - this.mDragStartX) >= 3 || this.abs(evt.y - this.mDragStartY) >= 3)
			{
				this.mCurrentDragEvent = this.GUI.DnDEvent(evt);

				if (this._fireDnDEvent(w, "onDragRequested") && this.mCurrentDragEvent.getTransferable() != null)
				{
					this.log.debug("DRAG " + this.mCurrentDragEvent.getTransferable() + " with actions " + this.mCurrentDragEvent.mSupportedActions);
					this._setDragHoverWidget(w);
					::_cursor.attachAction(this.mCurrentDragEvent.getVisual(), this.mCurrentDragEvent.getVisualBG());
					::_cursor.setState(this.GUI.Cursor.DRAG);
				}
				else
				{
					this.mCurrentDragEvent = null;
				}

				this.mDragStartWidgetName = null;
			}
		}
	}

	function postMouseMoved( evt )
	{
	}

	function preMousePressed( evt )
	{
		if (this.mTransientTops.len() > 0)
		{
			local cursorPos = this.Screen.getCursorPos();
			local transients = this.mTransientTops;
			this.mTransientTops = [];
			local overTransient = false;

			foreach( c in transients )
			{
				if (c.isVisible() && c.containsCursorPos(cursorPos))
				{
					overTransient = true;
					break;
				}
			}

			foreach( c in transients )
			{
				if (c.isVisible())
				{
					if (overTransient)
					{
						this.mTransientTops.append(c);
					}
					else
					{
						c.onOutsideClick(evt);
					}
				}
			}
		}

		if (!this.mCurrentDragEvent && evt.getWidget())
		{
			this.mDragStartWidgetName = evt.getWidget().getName();
			this.mDragStartX = evt.x;
			this.mDragStartY = evt.y;
		}
	}

	function postMousePressed( evt )
	{
	}

	function preMouseReleased( evt )
	{
		this.mDragStartWidgetName = null;

		if (!this.mCurrentDragEvent)
		{
			return;
		}

		local actions = this.mCurrentDragEvent.mAcceptedActions;
		actions = actions & this.mCurrentDragEvent.mSupportedActions;
		local w;

		if (actions && this.mDragHoverWidgetName)
		{
			w = this._getWidget(this.mDragHoverWidgetName);
		}

		if (w)
		{
			this.log.debug("DROP " + this.mCurrentDragEvent.getTransferable() + " with action " + actions);
			this.mCurrentDragEvent.mAcceptedActions = actions;
			this._fireDnDEvent(w, "onDrop");

			if (this.mCurrentDragEvent.mDragSource && "onDragComplete" in this.mCurrentDragEvent.mDragSource)
			{
				this.mCurrentDragEvent.mDragSource.onDragComplete(this.mCurrentDragEvent, true);
			}
		}
		else
		{
			this.log.debug("No acceptable DROP is capable for " + this.mDragHoverWidgetName + ", skipping.");

			if (this.mCurrentDragEvent.mDragSource && "onDragComplete" in this.mCurrentDragEvent.mDragSource)
			{
				this.mCurrentDragEvent.mDragSource.onDragComplete(this.mCurrentDragEvent, false);
			}
		}

		::_cursor.attachAction(null, null);
		::_cursor.setState(this.GUI.Cursor.DEFAULT);
		this._setDragHoverWidget(null);
		this.mCurrentDragEvent = null;
		evt.consume();
	}

	function postMouseReleased( evt )
	{
		foreach( c in this.mMouseCaptureList )
		{
			c.onMouseReleased(evt);
		}
	}

	function _fireDnDEvent( widget, eventName )
	{
		if (!widget)
		{
			return false;
		}

		if (!this.mCurrentDragEvent)
		{
			throw this.Exception("Only valid during a drag operation");
		}

		this.mCurrentDragEvent.mConsumed = false;

		if (eventName != "onDrop")
		{
			this.mCurrentDragEvent.mAcceptedActions = 0;
		}

		while (widget)
		{
			this.mCurrentDragEvent.mWidget = widget;
			widget.broadcastMessage(eventName, this.mCurrentDragEvent);

			if (this.mCurrentDragEvent.isConsumed())
			{
				return true;
			}

			widget = widget.getParent();
		}

		return false;
	}

	function _setDragHoverWidget( widget )
	{
		if (!this.mCurrentDragEvent)
		{
			throw this.Exception("Only valid during a drag operation");
		}

		if (this.mDragHoverWidgetName)
		{
			this._fireDnDEvent(this._getWidget(this.mDragHoverWidgetName), "onDragExit");
		}

		if (widget)
		{
			this.mDragHoverWidgetName = widget.getName();
			this._fireDnDEvent(widget, "onDragEnter");
			this._fireDnDEvent(widget, "onDragOver");
		}
		else
		{
			this.mDragHoverWidgetName = null;
		}

		return true;
	}

	function queueValidation( component )
	{
		this.mValidateQueue.append(component);
	}

}

this.GUI._Manager <- this.GUI.Manager();
::_setOverlayVisible <- this.Screen.setOverlayVisible;
::_overlayVisibilityMap <- {};
::_overlayForceInvisibleMap <- {};
this.Screen.setOverlayVisible <- function ( name, which )
{
	::_overlayVisibilityMap[name] <- which;
	local force = ::Screen.isOverlayForceInvisible(name);
	::_setOverlayVisible(name, force == true ? false : which);
};
this.Screen.isOverlayForceInvisible <- function ( name )
{
	if (name in ::_overlayForceInvisibleMap)
	{
		return ::_overlayForceInvisibleMap[name];
	}

	return false;
};
this.Screen.isOverlayVisible <- function ( name )
{
	if (name in ::_overlayVisibilityMap)
	{
		return ::_overlayVisibilityMap[name];
	}

	return false;
};
this.Screen.toggleOverlayForceInvisible <- function ( name )
{
	local force = ::Screen.isOverlayForceInvisible(name);
	::Screen.setOverlayForceInvisible(name, !force);
};
this.Screen.setOverlayForceInvisible <- function ( name, which )
{
	::_overlayForceInvisibleMap[name] <- which;
	local force = ::Screen.isOverlayForceInvisible(name);
	::_setOverlayVisible(name, force == true ? false : this.Screen.isOverlayVisible(name));
};
this.Screen.resetForceInvisible <- function ()
{
	::_overlayForceInvisibleMap <- {};
	::_UIVisible <- true;

	foreach( k, v in ::_overlayVisibilityMap )
	{
		::Screen.setOverlayVisible(k, v);
	}
};
this.Screen.setOverlayVisible(this.GUI.OVERLAY, true);
this.Screen.setOverlayVisible(this.GUI.ROLLOUT_OVERLAY, true);
this.Screen.setOverlayVisible(this.GUI.CURSOR_OVERLAY, true);
this.Screen.setOverlayVisible(this.GUI.CONFIRMATION_OVERLAY, true);
this.Screen.setOverlayVisible(this.GUI.POPUP_OVERLAY, true);
this.Screen.setOverlayVisible("GUI/ChatOverlay", true);
this.Screen.setOverlayVisible("GUI/QueueScreen", true);
this.Screen.setOverlayVisible("GUI/Overlay", true);
this.Screen.setOverlayVisible("GUI/Overlay2", true);
this.Screen.setOverlayVisible("GUI/FullScreenComponentOverlay", true);
this.Screen.setOverlayVisible("GUI/IGISOverlay", true);
this.Screen.setOverlayVisible("GUI/TooltipOverlay", true);
this.Screen.setOverlayVisible("GUI/SelectionBox", true);
this.Screen.setOverlayVisible("GUI/PartyOverlay", true);
this.Screen.setOverlayVisible("GUI/ChatBubbleOverlay", true);
this.Screen.setOverlayVisible("GUI/BugReportOverlay", true);
this.Screen.setOverlayVisible("GUI/MainUIOverlay", true);
this.Screen.setOverlayVisible("GUI/TargetOverlay", true);
this.Screen.setOverlayVisible("GUI/MiniMap", true);
this.Screen.setOverlayVisible("GUI/QuestTracker", true);
this.Screen.setOverlayVisible("GUI/SpecialPurchaseOffer", true);
