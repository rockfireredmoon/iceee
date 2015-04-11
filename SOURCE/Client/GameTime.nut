this.require("EventScheduler");
this.LagType <- {
	GREEN = 0,
	YELLOW = 1,
	RED = 2
};
class this.GameTime 
{
	mServerGameTime = -1;
	mPredictedGameTime = -1;
	mGameTimeElapsed = -1;
	mLastUpdate = -1;
	mLagNotifyEvent = null;
	mCurLagType = this.LagType.GREEN;
	constructor()
	{
		this.mLastUpdate = this.System.currentTimeMillis();
		this.mServerGameTime = this.mLastUpdate;
		this._enterFrameRelay.addListener(this);
	}

	function onEnterFrame()
	{
		local curTime = this.System.currentTimeMillis();
		this.mPredictedGameTime += curTime - this.mLastUpdate;
		this.mLastUpdate = curTime;
	}

	function updateGameTime( serverDelta )
	{
		this.mServerGameTime += serverDelta;
		this.mPredictedGameTime = this.mServerGameTime;

		if (this.mLagNotifyEvent != null)
		{
			this._eventScheduler.cancel(this.mLagNotifyEvent);
		}

		this.mLagNotifyEvent = this._eventScheduler.repeatIn(6.0, 6.0, this, "updateLag");
		this.mCurLagType--;

		if (this.mCurLagType < this.LagType.GREEN)
		{
			this.mCurLagType = this.LagType.GREEN;
		}
	}

	function getGameTimeMiliseconds()
	{
		return this.mPredictedGameTime;
	}

	function getGameTimeSeconds()
	{
		return this.mPredictedGameTime / 1000;
	}

	function updateLag()
	{
		this.mCurLagType++;

		if (this.mCurLagType > this.LagType.RED)
		{
			this.mCurLagType = this.LagType.RED;
		}
	}

}

