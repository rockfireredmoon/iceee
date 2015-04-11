this.require("Relay");
::States <- {};
class this.State 
{
	static mClassName = "UNKNOWN STATE";
	function onEnter()
	{
	}

	function onDestroy()
	{
	}

	function onReturn( state, arg )
	{
	}

	function onLeave()
	{
	}

	function onEnterFrame( time )
	{
	}

	function onExitFrame( time )
	{
	}

	function onKeyPressed( evt )
	{
	}

	function onKeyReleased( evt )
	{
	}

	function onScreenResize( width, height )
	{
	}

	function onScreenClose()
	{
	}

	function onScreenDefaultPositon()
	{
	}

	function onEvent( name, data )
	{
		local func;

		try
		{
			func = this["event_" + name];
		}
		catch( e )
		{
			return;
		}

		func(data);
	}

	function _tostring()
	{
		return this.mClassName;
	}

	function event_onUpdateLoadScreen( data )
	{
		::_contentLoader.updateLoadScreen();
	}

}

class this.StateManager 
{
	mStack = [];
	constructor()
	{
		this._screenResizeRelay.addListener(this);
		this._screenDefaultPositionRelay.addListener(this);
		this._screenCloseRelay.addListener(this);
		this._enterFrameRelay.addListener(this);
		this._exitFrameRelay.addListener(this);
	}

	function reset()
	{
		this.mStack.reverse();

		foreach( val in this.mStack )
		{
			val.onDestroy();
		}

		this.mStack = [];
	}

	function setState( state )
	{
		this.reset();
		this.pushState(state);
	}

	function peekCurrentState()
	{
		if (this.mStack.len() > 0)
		{
			return this.mStack[0];
		}
		else
		{
			return null;
		}
	}

	function pushState( state )
	{
		this.Assert.isInstanceOf(state, this.State);

		if (this.mStack.len() > 0)
		{
			this.mStack.top().onLeave();
		}

		this.mStack.push(state);
		state.onEnter();
	}

	function popState( ... )
	{
		local state = this.mStack.pop();
		state.onDestroy();

		if (this.mStack.len() > 0)
		{
			this.mStack.top().onReturn("mClassName" in state ? state.mClassName : "UNKNOWN", vargc > 0 ? vargv[0] : null);
		}
	}

	function onEnterFrame()
	{
		foreach( val in this.mStack )
		{
			val.onEnterFrame(0.0);
		}
	}

	function onExitFrame()
	{
		foreach( val in this.mStack )
		{
			val.onExitFrame(0.0);
		}
	}

	function onScreenResize()
	{
		foreach( val in clone this.mStack )
		{
			val.onScreenResize(::Screen.getWidth(), ::Screen.getHeight());
		}
	}

	function onScreenClose()
	{
		foreach( val in clone this.mStack )
		{
			val.onScreenClose();
		}
	}

	function onScreenDefaultPositon()
	{
		foreach( val in clone this.mStack )
		{
			val.onScreenDefaultPositon();
		}
	}

	function onEvent( name, ... )
	{
		local data = vargc > 0 ? vargv[0] : null;

		foreach( val in clone this.mStack )
		{
			val.onEvent(name, data);
		}
	}

	function top()
	{
		if (this.mStack.len() == 0)
		{
			return null;
		}

		return this.mStack[this.mStack.len() - 1];
	}

}

this.States.top <- function ()
{
	return ::_stateManager.top();
};
this.States.push <- function ( state )
{
	::_stateManager.pushState(state);
};
this.States.pop <- function ( ... )
{
	::_stateManager.popState(vargc > 0 ? vargv[0] : null);
};
this.States.set <- function ( state )
{
	::_stateManager.setState(state);
};
this.States.event <- function ( name, ... )
{
	::_stateManager.onEvent(name, vargc > 0 ? vargv[0] : null);
};
