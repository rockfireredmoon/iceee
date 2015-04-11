this.gVERTICAL_SLIDER <- 0;
this.gHORIZONTAL_SLIDER <- 1;
class this.GUI.Slider extends this.GUI.Container
{
	mCurrentNumber = 0;
	mMin = 0;
	mMax = 0;
	mStartingPos = 0;
	mEndingPos = 0;
	mSliderType = this.gVERTICAL_SLIDER;
	mDragging = false;
	mStartedDraggingPos = null;
	mStartDragCursorPos = null;
	mLastStartDragCursorPos = {
		x = 0,
		y = 0
	};
	mIntervalLengths = 0.0;
	mIntervals = [];
	mSliderNub = null;
	mSliderBackground = null;
	mMessageBroadcaster = null;
	mCursorOffset = null;
	static SLIDER_NUB_WIDTH = 26;
	static SLIDER_NUB_HEIGHT = 26;
	constructor( sliderType, minValue, maxValue, length )
	{
		this.GUI.Container.constructor(null);
		this.mSliderBackground = this.GUI.Component(null);
		this.mSliderNub = this.GUI.Component(null);
		this.mSliderNub.setAppearance("SliderNub");
		this.mSliderNub.setSize(this.SLIDER_NUB_WIDTH, this.SLIDER_NUB_HEIGHT);
		this.mSliderNub.setPreferredSize(this.SLIDER_NUB_WIDTH, this.SLIDER_NUB_HEIGHT);
		this.mSliderType = sliderType;
		this.add(this.mSliderBackground);
		this.add(this.mSliderNub);
		this.mMin = minValue;
		this.mMax = maxValue;
		local pos = this.getPosition();

		if (this.mSliderType == this.gVERTICAL_SLIDER)
		{
			local SLIDER_END_HEIGHT = 17;
			local backgroundWidth = this.SLIDER_NUB_WIDTH;
			local backgroundHeight = length + this.SLIDER_NUB_HEIGHT;
			this.mSliderBackground.setAppearance("VSliderBar");
			this.mSliderBackground.setPreferredSize(this.SLIDER_NUB_WIDTH, backgroundHeight + SLIDER_END_HEIGHT * 2);
			this.mSliderBackground.setSize(this.SLIDER_NUB_WIDTH, backgroundHeight + SLIDER_END_HEIGHT * 2);
			this.mStartingPos = SLIDER_END_HEIGHT;
			this.setPreferredSize(this.SLIDER_NUB_WIDTH, backgroundHeight + SLIDER_END_HEIGHT * 2);
			this.mSliderNub.setPosition(0, this.mStartingPos);
		}
		else if (this.mSliderType == this.gHORIZONTAL_SLIDER)
		{
			local SLIDER_END_HEIGHT = 10;
			local END_PIECE_WIDTH = 10;
			local backgroundWidth = length + this.SLIDER_NUB_WIDTH;
			local backgroundHeight = this.SLIDER_NUB_HEIGHT;
			this.mSliderBackground.setAppearance("HSliderBar");
			this.mSliderBackground.setPreferredSize(backgroundWidth + END_PIECE_WIDTH * 2, SLIDER_END_HEIGHT);
			this.mSliderBackground.setSize(backgroundWidth + END_PIECE_WIDTH * 2, SLIDER_END_HEIGHT);
			this.mSliderBackground.setPosition(0, this.SLIDER_NUB_HEIGHT / 2 - SLIDER_END_HEIGHT / 2);
			this.mStartingPos = END_PIECE_WIDTH;
			this.setPreferredSize(backgroundWidth + END_PIECE_WIDTH * 2, this.SLIDER_NUB_HEIGHT);
			this.mSliderNub.setPosition(this.mStartingPos, 0);
		}

		this.mEndingPos = this.mStartingPos + length;
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function setIntervals( interval )
	{
		this.mIntervals.clear();
		this.mIntervalLengths = interval.tofloat() / 100.0 * (this.mEndingPos - this.mStartingPos).tofloat();

		for( local i = this.mStartingPos; i <= this.mEndingPos; i += this.mIntervalLengths )
		{
			this.mIntervals.push(i);
		}

		local oldPos = this.mSliderNub.getPosition();

		if (this.mSliderType == this.gVERTICAL_SLIDER)
		{
			local newPos = this.findClosestInterval(oldPos.y);
			this.mSliderNub.setPosition(oldPos.x, newPos);
		}
		else
		{
			local newPos = this.findClosestInterval(oldPos.x);
			this.mSliderNub.setPosition(newPos, oldPos.y);
		}
	}

	function getValue()
	{
		return this.mCurrentNumber;
	}

	function setValue( value )
	{
		if (this.mMin < this.mMax)
		{
			this.mCurrentNumber = this.Math.clamp(value, this.mMin, this.mMax);
		}
		else
		{
			this.mCurrentNumber = this.Math.clamp(value, this.mMax, this.mMin);
		}

		this.UpdateSliderWithNewValue();
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mDragging = true;
			this.mStartedDraggingPos = this.mSliderNub.getPosition();
			this.mStartDragCursorPos = ::_cursor.getPosition();
			local nubPos = this.mSliderNub.getScreenPosition();
			this.mCursorOffset = {
				x = this.mStartDragCursorPos.x - nubPos.x,
				y = this.mStartDragCursorPos.y - nubPos.y
			};
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mDragging = false;
			this.mLastStartDragCursorPos = this.mStartDragCursorPos;
			evt.consume();
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this.mWidget.addListener(this);
	}

	function _forceDrop()
	{
		this.mDragging = false;
		this.mSliderNub.setPosition(this.mStartedDraggingPos.x, this.mStartedDraggingPos.y);
		this.UpdateCurrentValue();
	}

	function getTotalOffset( component )
	{
		local componentParent = component.getParent();
		local totalPosition = {
			x = 0,
			y = 0
		};

		if (componentParent)
		{
			local position = componentParent.getPosition();
			totalPosition.x += position.x;
			totalPosition.y += position.y;
			local parentOffset = this.getTotalOffset(componentParent);
			totalPosition.x += parentOffset.x;
			totalPosition.y += parentOffset.y;
			return totalPosition;
		}
		else
		{
			return totalPosition;
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mDragging == true)
		{
			if (this.mSliderType == this.gVERTICAL_SLIDER)
			{
				local currentPos = this.mSliderNub.getPosition();
				local endCursorPosition = ::_cursor.getPosition();
				local ny = endCursorPosition.y - this.getScreenPosition().y - this.mCursorOffset.y;
				local cursorOffset = {
					x = currentPos.x,
					y = ny
				};

				if (cursorOffset.x > currentPos.x + this.getWidth() + 60 || cursorOffset.x < currentPos.x - 60)
				{
					this._forceDrop();
					return;
				}

				if (this.mIntervalLengths > 0)
				{
					local closestInterval = this.findClosestInterval(cursorOffset.y);
					this.mSliderNub.setPosition(currentPos.x, closestInterval);
				}
				else
				{
					this.mSliderNub.setPosition(currentPos.x, cursorOffset.y);
				}
			}
			else
			{
				local currentPos = this.mSliderNub.getPosition();
				local cursorPosition = ::_cursor.getPosition();
				local offset = this.getTotalOffset(this.mSliderNub);
				local nx = cursorPosition.x - this.getScreenPosition().x - this.mCursorOffset.x;
				local cursorOffset = {
					x = nx,
					y = currentPos.y
				};

				if (cursorOffset.y > currentPos.y + this.getHeight() + 60 || cursorOffset.y < currentPos.y - 60)
				{
					this._forceDrop();
					return;
				}

				if (this.mIntervalLengths > 0)
				{
					local closestInterval = this.findClosestInterval(cursorOffset.x);
					this.mSliderNub.setPosition(closestInterval, currentPos.y);
				}
				else
				{
					this.mSliderNub.setPosition(cursorOffset.x, currentPos.y);
				}
			}

			local currentPos = this.mSliderNub.getPosition();
			local xPos = currentPos.x;
			local yPos = currentPos.y;

			if (this.mSliderType == this.gHORIZONTAL_SLIDER)
			{
				if (currentPos.x <= this.mStartingPos)
				{
					xPos = this.mStartingPos;
				}
				else if (currentPos.x >= this.mEndingPos)
				{
					xPos = this.mEndingPos;
				}
			}
			else if (currentPos.y <= this.mStartingPos)
			{
				yPos = this.mStartingPos;
			}
			else if (currentPos.y >= this.mEndingPos)
			{
				yPos = this.mEndingPos;
			}

			this.mSliderNub.setPosition(xPos, yPos);
			this.UpdateCurrentValue();
			evt.consume();
		}
	}

	function findClosestInterval( location )
	{
		local closestInterval = 0;

		for( local i = 0; i < this.mIntervals.len() - 1; i++ )
		{
			if (location + this.mIntervalLengths / 2.0 > this.mIntervals[i + 1])
			{
				closestInterval++;
			}
			else
			{
				return this.mIntervals[i];
			}
		}

		return this.mIntervals[this.mIntervals.len() - 1];
	}

	function UpdateCurrentValue()
	{
		local currentPos = this.mSliderNub.getPosition();
		local ratio = (this.mMax - this.mMin).tofloat();
		local componentToCompare = 0;

		if (this.mSliderType == this.gVERTICAL_SLIDER)
		{
			componentToCompare = currentPos.y;
		}
		else
		{
			componentToCompare = currentPos.x;
		}

		local amt = (this.mEndingPos - componentToCompare).tofloat() / (this.mEndingPos - this.mStartingPos).tofloat();
		amt = 1.0 - amt;
		this.mCurrentNumber = amt * ratio + this.mMin;
		this.log.debug(this.mCurrentNumber);
		this.mMessageBroadcaster.broadcastMessage("onSliderUpdated", this);
	}

	function UpdateSliderWithNewValue()
	{
		local difference = (this.mMax - this.mMin).tofloat();
		local currentNumber = this.mCurrentNumber - this.mMin;
		local sliderWidth = this.mEndingPos.tofloat() - this.mStartingPos.tofloat();

		if (this.mMin > this.mMax)
		{
			difference = (this.mMin - this.mMax).tofloat();
			currentNumber = this.mMin - this.mCurrentNumber;
		}

		local ratio = sliderWidth / difference.tofloat();
		local newPosition = currentNumber * ratio + this.mStartingPos;
		local curPos = this.mSliderNub.getPosition();

		if (this.mSliderType == this.gVERTICAL_SLIDER)
		{
			this.mSliderNub.setPosition(curPos.x, newPosition);
		}
		else
		{
			this.mSliderNub.setPosition(newPosition, curPos.y);
		}
	}

	function addChangeListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeChangeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

}

