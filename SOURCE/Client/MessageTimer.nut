this.MessageTimer <- {};
class this.MessageTimer.Manager extends this.MessageBroadcaster
{
	constructor()
	{
		::MessageBroadcaster.constructor();
		this.mMessageTimers = [];
		::_enterFrameRelay.addListener(this);
	}

	function createTimeOut( pDuration, pMessage )
	{
		local timeOut = this.MessageTimer.TimeOut(pDuration, pMessage);
		this.mMessageTimers.append(timeOut);
		this.mMessageTimers.sort(this.sortByNextMessage);
		return timeOut;
	}

	function createHeartBeat( pDuration, pMessage, pHeartBeat, pHeartBeatMessage )
	{
		local heartBeat = this.MessageTimer.HeartBeat(pDuration, pMessage, pHeartBeat, pHeartBeatMessage);
		this.mMessageTimers.append(heartBeat);
		this.mMessageTimers.sort(this.sortByNextMessage);
		return heartBeat;
	}

	function sortByNextMessage( A, B )
	{
		if (A.mNextMessageTime > B.mNextMessageTime)
		{
			return 1;
		}
		else if (A.mNextMessageTime < B.mNextMessageTime)
		{
			return -1;
		}

		return 0;
	}

	function onSort()
	{
		this.mMessageTimers.sort(this.sortByNextMessage);
	}

	function removeTimer( pTimer )
	{
		if (this.mMessageTimers.len() == 0)
		{
			return;
		}

		foreach( i, x in this.mMessageTimers )
		{
			if (x == pTimer)
			{
				this.mMessageTimers.remove(i);
				return null;
			}
		}
	}

	function onEnterFrame()
	{
		if (!this.mMessageTimers || this.mMessageTimers.len() == 0)
		{
			return;
		}

		this.onSort();

		foreach( i, x in this.mMessageTimers )
		{
			if (x.checkTime())
			{
				this.mMessageTimers.remove(i);
			}
			else
			{
				break;
			}
		}
	}

	mMessageTimers = [];
}

class this.MessageTimer.TimeOut extends this.MessageBroadcaster
{
	constructor( pDuration, pMessage )
	{
		::MessageBroadcaster.constructor();
		this.mCompletionTime = ::_time + pDuration * 1000;
		this.mNextMessageTime = this.mCompletionTime;
		this.mMessage = pMessage;
	}

	function checkTime()
	{
		if (this.mCompletionTime <= this._time)
		{
			this.broadcastMessage(this.mMessage);
			return true;
		}

		return false;
	}

	mNextMessageTime = 0.0;
	mCompletionTime = 0.0;
	mMessage = "";
}

class this.MessageTimer.HeartBeat extends this.MessageTimer.TimeOut
{
	constructor( pDuration, pMessage, pHeartBeat, pHeartBeatMessage )
	{
		::MessageTimer.TimeOut.constructor(pDuration, pMessage);

		if (pDuration == 0.0)
		{
			this.mCompletionTime = 0.0;
		}

		this.mNextMessageTime = ::_time + pHeartBeat * 1000;
		this.mHeartBeat = pHeartBeat * 1000;
		this.mHeartBeatMessage = pHeartBeatMessage;
	}

	function checkTime()
	{
		if (this.mNextMessageTime <= this._time)
		{
			this.mNextMessageTime = this.mNextMessageTime + this.mHeartBeat;
			this.broadcastMessage(this.mHeartBeatMessage);
		}

		if (this.mCompletionTime <= this._time && this.mCompletionTime != 0.0)
		{
			this.broadcastMessage(this.mMessage);
			return true;
		}

		return false;
	}

	mHeartBeatMessage = "";
	mHeartBeat = 0.0;
}

