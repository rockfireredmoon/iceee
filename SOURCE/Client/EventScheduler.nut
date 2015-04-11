class this.EventScheduler 
{
	mEventQueue = null;
	constructor()
	{
		this.mEventQueue = [];
	}

	function fireIn( delaySeconds, target, message, ... )
	{
		local event = {
			target = target,
			message = message,
			fireTime = ::_time + (delaySeconds * 1000).tointeger()
		};

		if (vargc > 0)
		{
			event.messageArg <- vargv[0];
		}

		this.schedule(event);
		return event;
	}

	function repeatIn( delaySeconds, period, target, message, ... )
	{
		if (period <= 0)
		{
			throw this.Exception("Invalid repeat period: " + period);
		}

		local event = {
			target = target,
			message = message,
			period = period,
			fireTime = ::_time + (delaySeconds * 1000).tointeger()
		};

		if (vargc > 0)
		{
			event.messageArg <- vargv[0];
		}

		this.schedule(event);
		return event;
	}

	function schedule( event )
	{
		this._verifyEventIsValid(event);

		if ("scheduler" in event)
		{
			throw this.Exception("Event is already scheduled to fire");
		}

		event.scheduler <- this;

		if (event.fireTime <= ::_time)
		{
			this._fireEvent(event);
			return;
		}

		if (this.mEventQueue.len() == 0)
		{
			::_nextFrameRelay.addListener(this);
		}

		this.mEventQueue.append(event);
		this.mEventQueue.sort(this._compareEvents);
	}

	function cancel( event )
	{
		if (event == null)
		{
			return;
		}

		this._verifyEventIsValid(event);

		if (!("scheduler" in event) || event.scheduler != this)
		{
			return;
		}

		local newQ = [];
		local cancelled = 0;

		foreach( e in this.mEventQueue )
		{
			if (e == event || e.message == event)
			{
				delete e.scheduler;
				cancelled++;
			}
			else
			{
				newQ.append(e);
			}
		}

		if (cancelled > 0)
		{
			this.mEventQueue = newQ;

			if (this.mEventQueue.len() > 0)
			{
				this.mEventQueue.sort(this._compareEvents);
			}
			else
			{
				::_nextFrameRelay.removeListener(this);
			}
		}
	}

	function cancelAll()
	{
		if (this.mEventQueue.len() > 0)
		{
			this.mEventQueue = [];
			::_nextFrameRelay.removeListener(this);
		}
	}

	function _verifyEventIsValid( event )
	{
		if (typeof event != "table" && typeof event != "instance" || !("fireTime" in event) || !("message" in event))
		{
			throw this.Exception("Invalid event: " + event);
		}
	}

	function _checkEvents()
	{
		while (this.mEventQueue.len() > 0)
		{
			local e = this.mEventQueue[0];

			if (e.fireTime > this._time)
			{
				break;
			}

			this.mEventQueue.remove(0);
			this._fireEvent(e);
		}
	}

	function onNextFrame()
	{
		this._checkEvents();
	}

	function _fireEvent( event )
	{
		local target = event.target;
		local message = event.message;

		if (message in target)
		{
			if ("messageArg" in event)
			{
				target[message](event.messageArg);
			}
			else
			{
				target[message]();
			}

			delete event.scheduler;

			if ("period" in event)
			{
				event.fireTime = this._time + (event.period * 1000).tointeger();
				this.schedule(event);
			}
		}
		else
		{
			this.log.error("Event handler (" + message + ") not found on " + target);
		}
	}

	function _compareEvents( a, b )
	{
		if (a.fireTime < b.fireTime)
		{
			return -1;
		}

		if (a.fireTime > b.fireTime)
		{
			return 1;
		}

		return 0;
	}

}

