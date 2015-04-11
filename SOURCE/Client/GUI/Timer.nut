this.require("GUI/GUI");
class this.GUI.Timer extends this.MessageBroadcaster
{
	constructor( message, ... )
	{
		this.MessageBroadcaster.constructor();
		this.mMessage = message;
		this.mRepeat = vargc > 1 ? vargv[1] : null;
		this.setDelay(vargc > 0 ? vargv[0] : null);
	}

	function setDelay( delay )
	{
		if (delay == null)
		{
			if (this.mFireTime != null)
			{
				this._root.removeListener(this);
				this.mFireTime = null;
			}

			return;
		}

		if (this.mFireTime == null)
		{
			this.mFireTime = this._time + delay;
			this._root.addListener(this);
		}
		else
		{
			this.mFireTime = this._time + delay;
		}
	}

	function onEnterFrame()
	{
		if (this._time >= this.mFireTime)
		{
			this.setDelay(this.mRepeat);
			this.broadcastMessage(this.mMessage);
		}
	}

	function getTimeUntilFire()
	{
		if (this.mFireTime == null)
		{
			return null;
		}

		return this.mFireTime - this._time;
	}

	function cancel()
	{
		this.setDelay(null);
	}

	mMessage = null;
	mFireTime = null;
	mRepeat = null;
}

