class this.GUI.MainScreenElement extends this.GUI.Container
{
	mDragging = false;
	mMouseOffset = {};
	mConstructed = false;
	mAllowDrag = true;
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Container.constructor(vargv[0]);
		}
		else
		{
			this.GUI.Container.constructor();
		}
	}

	function fitToScreen()
	{
		local pos = this.getPosition();
		pos.x = pos.x > 0 ? pos.x : 0;
		pos.y = pos.y > 0 ? pos.y : 0;
		pos.x = pos.x < ::Screen.getWidth() - this.getWidth() ? pos.x : ::Screen.getWidth() - this.getWidth();
		pos.y = pos.y < ::Screen.getHeight() - this.getHeight() ? pos.y : ::Screen.getHeight() - this.getHeight();
		this.setPosition(pos);
	}

	function onMouseMoved( evt )
	{
		if (this.mAllowDrag)
		{
			if (this.mDragging)
			{
				local newpos = ::Screen.getCursorPos();
				local deltax = newpos.x - this.mMouseOffset.x;
				local deltay = newpos.y - this.mMouseOffset.y;
				local pos = this.getPosition();
				pos.x += deltax;
				pos.y += deltay;
				this.mMouseOffset = newpos;
				this.setPosition(pos);
				this.fitToScreen();
			}
		}

		evt.consume();
	}

	function onMousePressed( evt )
	{
		if (this.mAllowDrag)
		{
			this.mMouseOffset = ::Screen.getCursorPos();
			this.mDragging = true;
			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (this.mAllowDrag)
		{
			this.mDragging = false;
			evt.consume();
		}
	}

	function allowDragging( value )
	{
		this.mAllowDrag = value;
	}

}

