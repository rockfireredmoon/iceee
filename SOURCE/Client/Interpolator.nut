this.require("Relay");
this.Tween <- {};
class this.Tween.Base 
{
	function getValue( time, duration, start, delta )
	{
		return start;
	}

}

class this.Tween.Linear 
{
	function getValue( time, duration, start, delta )
	{
		return this.t.tofloat() / duration * delta + start;
	}

}

this.Evaluator <- {};
class this.Evaluator.Base 
{
	function isFinished()
	{
		return true;
	}

	function get()
	{
		return null;
	}

}

class this.Evaluator.Tween extends this.Evaluator.Base
{
	mStart = null;
	mDelta = null;
	mStartT = null;
	mDuration = null;
	mTween = null;
	constructor( start, delta, duration, tween )
	{
		this.mStart = start;
		this.mDelta = delta;
		this.mStartT = this.System.currentTimeMillis();
		this.mDuration = duration;
	}

	function isFinished()
	{
		return this.mDuration < 0;
	}

	function get()
	{
		if (this.mDuration < 0)
		{
			return this.mStart + this.mDelta;
		}

		local t = (this.System.currentTimeMillis() - this.mStartT) / 1000.0;

		if (t >= this.mDuration)
		{
			this.mDuration = -1;
			t = this.mDuration;
		}

		return this.mTween.getValue(t, this.mDuration, this.mStart, this.mDelta);
	}

}

class this.Evaluator.Geometric extends this.Evaluator.Base
{
	mEnd = null;
	mDelta = null;
	mRate = 4.0;
	constructor( start, delta, ... )
	{
		this.mEnd = start + delta;
		this.mDelta = delta;

		if (vargc > 0)
		{
			this.mRate = vargv[0].tofloat();
		}
	}

	function isFinished()
	{
		return this.fabs(this.mDelta) <= 0.001;
	}

	function get()
	{
		this.mDelta -= this.mDelta / this.mRate;
		return this.mEnd - this.mDelta;
	}

}

class this.Interpolator 
{
	mObject = null;
	mMethod = null;
	mEval = null;
	constructor( object, method, evaluator )
	{
		this.mObject = object;
		this.mMethod = method;

		if (!(this.mMethod in this.mObject))
		{
			throw this.Exception("Unknown method in object: " + method);
		}

		this.mEval = evaluator;
		this._enterFrameRelay.addListener(this);
		this.log.debug("Beginning Interpolation of " + this.mMethod + " on " + this.mObject + " using " + this.mEval);
	}

	function onEnterFrame()
	{
		local value = this.mEval.get();

		if (this.mEval.isFinished())
		{
			this.cancel();
		}

		this.mObject[this.mMethod](value);
	}

	function cancel()
	{
		this.log.debug("Ending Interpolation of " + this.mMethod + " on " + this.mObject);
		this._enterFrameRelay.removeListener(this);
	}

}

function Interpolate( object, method, start, end, ... )
{
	local interp = vargc > 0 ? vargv[0] : "geometric";
	local delta = end - start;
	local eval;

	if (interp == "geometric")
	{
		eval = this.Evaluator.Geometric(start, delta);
	}
	else if (interp == "linear")
	{
		eval = this.Evaluator.Tween(start, delta, vargv[1], this.Tween.Linear);
	}

	return this.Interpolator(object, method, eval);
}

