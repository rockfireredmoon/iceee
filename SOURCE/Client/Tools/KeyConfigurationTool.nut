class this.KeyConfigurationTool extends this.AbstractCameraTool
{
	mBroadcaster = null;
	constructor()
	{
		this.AbstractCameraTool.constructor(null, false);
		this.mBroadcaster = this.MessageBroadcaster();
	}

	function addListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function onKeyPressed( evt )
	{
		this.mBroadcaster.broadcastMessage("onKeyPressed", evt);
		evt.consume();
	}

	function removeListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

}

