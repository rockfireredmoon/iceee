this.require("GUI/Container");
class this.GUI.AnimatedBar extends this.GUI.Container
{
	mCounter = 0.0;
	mDestWidth = 0.0;
	constructor()
	{
		this.GUI.Container.constructor();
	}

	function _addNotify()
	{
		::GUI.Container._addNotify();
	}

	function _removeNotify()
	{
		::_enterFrameRelay.removeListener(this);
		::GUI.Container._removeNotify();
	}

	function setDestWidth( v, anim )
	{
		this.mDestWidth = v;

		if (!anim)
		{
			this.setWidth(this.mDestWidth);
			this.mCounter = v;
		}
		else
		{
			::_enterFrameRelay.addListener(this);
		}
	}

	function onEnterFrame()
	{
		if (this.mCounter == this.mDestWidth)
		{
			::_enterFrameRelay.removeListener(this);
			return;
		}

		this.mCounter = this.Math.GravitateValue(this.mCounter, this.mDestWidth, this._deltat / 1000.0, 6.0);

		if (this.Math.abs(this.mDestWidth - this.mCounter) < 0.1)
		{
			this.mCounter = this.mDestWidth;
		}

		this.setWidth(this.mCounter);
	}

}

