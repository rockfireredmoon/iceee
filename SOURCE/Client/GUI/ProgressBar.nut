this.require("GUI/Component");
class this.GUI.ProgressBar extends this.GUI.Component
{
	static PERCENTAGE = 1;
	static FRACTION = 2;
	mMask = null;
	mAnimateReverse = true;
	mIndicator = null;
	mRemainder = null;
	mLabel = null;
	mNotch = null;
	mType = 1;
	mLabelled = false;
	mMasked = true;
	mNotched = false;
	mAnimated = false;
	mAnimatedCurrent = 0.0;
	mProgress = 0.0;
	mCurrent = 0;
	mMax = 1;
	mNotchWidth = 10;
	static mClassName = "ProgressBar";
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.mAppearance = "ProgressBar";

		if (vargc > 0)
		{
			if (typeof vargv[0] == "string")
			{
				this.mAppearance = vargv[0];
			}
		}

		this.mIndicator = this.GUI.Component();
		this.mIndicator.setAppearance(this.mAppearance + "/Indicator");
		this.add(this.mIndicator);
		this.setPositionRelativeToMe(this.mIndicator, 0, 0);
		this.mIndicator.setVisible(true);
		this.mRemainder = this.GUI.Component();
		this.mRemainder.setAppearance(this.mAppearance + "/Remainder");
		this.mIndicator.add(this.mRemainder);
		this.setPositionRelativeToMe(this.mRemainder, 0, 0);
		this.mNotch = this.GUI.Component();
		this.mNotch.setAppearance(this.mAppearance + "/Notch");
		this.mNotchWidth = 1;
		this.mRemainder.add(this.mNotch);
		this.setPositionRelativeToMe(this.mNotch, 0, 0);
		this.setNotchVisible(false);
		this.mMask = this.GUI.Component();
		this.mMask.setAppearance(this.mAppearance + "/Mask");
		this.mMask.setWidth(this.getWidth());
		this.mMask.setHeight(this.getHeight());
		this.mNotch.add(this.mMask);
		this.setPositionRelativeToMe(this.mMask, 0, 0);
		this.setMaskVisible(true);
		this.mLabel = this.GUI.Label("");
		this.mMask.add(this.mLabel);
		this.setLabelVisible(false);
		this.setPercentage(0.0);
	}

	function setAppearance( pAppearance )
	{
		this.GUI.Component.setAppearance(pAppearance);
		this.mIndicator.setAppearance(this.mAppearance + "/Indicator");
		this.mRemainder.setAppearance(this.mAppearance + "/Remainder");
		this.mNotch.setAppearance(this.mAppearance + "/Notch");
		this.mMask.setAppearance(this.mAppearance + "/Mask");
	}

	function setPositionRelativeToMe( component, x, y )
	{
		local pos = {
			x = 0,
			y = 0
		};
		local p = component.mParentComponent;

		while (p && p.mParentComponent && p.mParentComponent != this)
		{
			local p2 = p.getPosition();
			pos.x += p2.x;
			pos.y += p2.y;
			p = p.mParentComponent;
		}

		component.setPosition(x - pos.x, y - pos.y);
	}

	function setAnimateReverse( which )
	{
		this.mAnimateReverse = which;
	}

	function setAnimated( val )
	{
		if (!this.mAnimated && val)
		{
			::_root.addListener(this);
		}
		else if (this.mAnimated && !val)
		{
			::_root.removeListener(this);
		}

		this.mAnimated = val;
	}

	function _removeNotify()
	{
		if (this.mAnimated)
		{
			::_root.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		delta = delta < 0.1 ? delta : 0.1;
		local pdelta = this.Math.abs(this.mAnimatedCurrent - this.mCurrent);

		if (this.mAnimatedCurrent < this.mCurrent)
		{
			this.mCurrent -= pdelta.tofloat() * 10 * delta.tofloat();
		}
		else
		{
			this.mCurrent += pdelta.tofloat() * 10 * delta.tofloat();
		}

		this._setProgress(1.0 * this.mCurrent / this.mMax);
	}

	function setMaskVisible( v )
	{
		if (v)
		{
			this.mMasked = true;
			this.mMask.setAppearance(this.mAppearance + "/Mask");
		}
		else
		{
			this.mMasked = false;
			this.mMask.setAppearance("Container");
		}
	}

	function setNotchVisible( v )
	{
		if (v)
		{
			this.mNotched = true;
			this.mNotch.setAppearance(this.mAppearance + "/Notch");
		}
		else
		{
			this.mNotched = false;
			this.mNotch.setAppearance("Container");
		}
	}

	function setLabelVisible( v )
	{
		if (v)
		{
			this.mLabelled = true;
			this._recalc();
			this.mLabel.setVisible(true);
		}
		else
		{
			this.mLabelled = false;
			this.mLabel.setVisible(false);
		}
	}

	function setNotchWidth( w )
	{
		this.mNotchWidth = w;
		this._recalc();
	}

	function getPreferredSize()
	{
		return {
			width = this.mWidth,
			height = this.mHeight
		};
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();
		this._recalc();
	}

	function _recalc()
	{
		if (this.mIndicator.mWidget != null)
		{
			this.mIndicator.setSize(this.floor(1.0 * this.getWidth() * this.mProgress), this.getHeight());
			this.mIndicator.mWidget.setParam("border_size", "0 " + this.floor(1.0 * this.getWidth() * this.mProgress) + " 0 0");
			this.mIndicator.mWidget.setParam("border_right_uv", "0.0000 0.0000 " + this.mProgress + " 1.0000");
		}

		if (this.mRemainder.mWidget != null)
		{
			this.setPositionRelativeToMe(this.mRemainder, this.floor(1.0 * this.getWidth() * this.mProgress), 0);
			this.mRemainder.setSize(this.floor(1.0 * this.getWidth() * (1 - this.mProgress)), this.getHeight());
			this.mRemainder.mWidget.setParam("border_size", "0 " + this.floor(1.0 * this.getWidth() * (1.0 - this.mProgress)) + " 0 0");
			this.mRemainder.mWidget.setParam("border_right_uv", this.mProgress + " 0.0000 1.000 1.0000");
		}

		this.setPositionRelativeToMe(this.mNotch, this.floor(1.0 * this.getWidth() * this.mProgress) - this.mNotch.getWidth() / 2, 0);
		this.mNotch.setSize(this.mNotchWidth, this.getHeight());
		this.setPositionRelativeToMe(this.mMask, 0, 0);
		this.mMask.setSize(this.getWidth(), this.getHeight());

		if (this.mLabelled)
		{
			switch(this.mType)
			{
			case this.PERCENTAGE:
				this.mLabel.setText(this.floor(this.mProgress * 100.0) + "%");
				break;

			case this.FRACTION:
				this.mLabel.setText(this.mCurrent + "/" + this.mMax);
				break;
			}

			local labelsize = this.mLabel.getPreferredSize();
			this.setPositionRelativeToMe(this.mLabel, (this.getWidth() - labelsize.width) / 2, (this.getHeight() - labelsize.height) / 2);
		}
	}

	function _setProgress( progress )
	{
		this.mProgress = progress;

		if (this.mProgress < 0.0)
		{
			this.mProgress = 0.0;
		}

		if (this.mProgress > 1.0)
		{
			this.mProgress = 1.0;
		}

		this._recalc();
	}

	function setLabelMode( t )
	{
		if (t == this.PERCENTAGE || t == this.FRACTION)
		{
			this.mType = t;
		}
		else
		{
			this.print("Tried to set to an invalid type " + t);
		}

		this._recalc();
	}

	function setMax( max )
	{
		if (max > 0)
		{
			this.mMax = max;
		}
		else
		{
			this.mMax = 1;
		}
	}

	function setCurrent( cur )
	{
		if (this.mAnimated && (cur >= this.mAnimatedCurrent || this.mAnimateReverse))
		{
			this.mAnimatedCurrent = cur;
			this.mAnimatedCurrent = this.mAnimatedCurrent > 0 ? this.mAnimatedCurrent : 0;
			this.mAnimatedCurrent = this.mAnimatedCurrent < this.mMax ? this.mAnimatedCurrent : this.mMax;
		}
		else
		{
			this.mCurrent = cur;
			this.mCurrent = this.mCurrent > 0 ? this.mCurrent : 0;
			this.mCurrent = this.mCurrent < this.mMax ? this.mCurrent : this.mMax;
			this.mAnimatedCurrent = this.mCurrent;
			this._setProgress(1.0 * this.mCurrent / this.mMax);
		}
	}

	function setPercentage( p )
	{
		this.setCurrent(this.mMax * p);
	}

	function getMax()
	{
		return this.mMax;
	}

	function getCurrent()
	{
		return this.mCurrent;
	}

	function getPercentage()
	{
		return this.mProgress;
	}

	function _debugstring()
	{
		return this.GUI.Component._debugstring() + "(" + this.mProgress + ")";
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this._recalc();
	}

}

