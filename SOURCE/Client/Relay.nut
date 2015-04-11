class this.Relay extends this.MessageBroadcaster
{
	constructor()
	{
		::MessageBroadcaster.constructor();
	}

}

class this.RenderSystemEventRelay extends this.Relay
{
	constructor()
	{
		::Relay.constructor();
		::_root.addListener(this);
	}

	function onDeviceLost()
	{
		this.broadcastMessage("onDeviceLost");
	}

	function onDeviceRestored()
	{
		this.broadcastMessage("onDeviceRestored");
	}

}

this._renderSystemEventRelay <- ::RenderSystemEventRelay();
class this.EnterFrameRelay extends this.Relay
{
	constructor()
	{
		::Relay.constructor();
		::_root.addListener(this);
	}

	function onEnterFrame()
	{
		this.broadcastMessage("onEnterFrame");
	}

}

this._enterFrameRelay <- ::EnterFrameRelay();
class this.ScreenResizeRelay extends this.Relay
{
	mOldScreenWidth = 0;
	mOldScreenHeight = 0;
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
		this.mOldScreenWidth = this.Screen.getWidth();
		this.mOldScreenHeight = this.Screen.getHeight();
	}

	function getOldWidth()
	{
		return this.mOldScreenWidth;
	}

	function getOldHeight()
	{
		return this.mOldScreenHeight;
	}

	function onScreenResize()
	{
		this.broadcastMessage("onScreenResize");
		this.mOldScreenWidth = this.Screen.getWidth();
		this.mOldScreenHeight = this.Screen.getHeight();
	}

}

this._screenResizeRelay <- ::ScreenResizeRelay();
class this.ScreenCloseRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
	}

	function onScreenClose()
	{
		this.Screens.saveAllScreenPosition();
		this.broadcastMessage("onScreenClose");
	}

}

this._screenCloseRelay <- ::ScreenCloseRelay();
class this.ScreenDefaultPositionRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
	}

	function onScreenDefaultPositon()
	{
		this.Screens.clearScreenSavePositions();
		this.broadcastMessage("onScreenDefaultPositon");
	}

}

this._screenDefaultPositionRelay <- ::ScreenDefaultPositionRelay();
class this.CSMReadyRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
	}

	function onCSMReady( node )
	{
		this.broadcastMessage("onCSMReady", node);
	}

	function onCSMDestroyed( node )
	{
		this.broadcastMessage("onCSMDestroyed", node);
	}

}

this._csmReadyRelay <- ::CSMReadyRelay();
class this.ExitFrameRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
	}

	function onExitFrame()
	{
		this.broadcastMessage("onExitFrame");
	}

}

this._exitFrameRelay <- ::ExitFrameRelay();
class this.ExitGameStateRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
	}

	function gameExited()
	{
		this.broadcastMessage("onExitGame");
	}

}

this._exitGameStateRelay <- ::ExitGameStateRelay();
class this.NextFrameRelay extends this.Relay
{
	constructor()
	{
		this.Relay.constructor();
		::_root.addListener(this);
	}

	function onNextFrame()
	{
		this.broadcastMessage("onNextFrame");
	}

}

this._nextFrameRelay <- ::NextFrameRelay();
