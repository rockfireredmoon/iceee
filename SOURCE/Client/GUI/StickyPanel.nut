this.require("UI/MainScreenElement");
class this.GUI.StickyPanel extends this.GUI.MainScreenElement
{
	mXOffset = 0;
	mYOffset = 0;
	constructor( startX, startY, xOffset, yOffset, ... )
	{
		if (vargc > 0)
		{
			this.GUI.MainScreenElement.constructor(vargv[0]);
		}
		else
		{
			this.GUI.MainScreenElement.constructor();
		}

		this.allowDragging(false);
		this.setAppearance("Panel");
		this.mXOffset = xOffset;
		this.mYOffset = yOffset;
		this.onStickyComponentMoved(startX, startY);
	}

	function onStickyComponentMoved( newX, newY )
	{
		this.setPosition(this.mXOffset + newX, this.mYOffset + newY);
		this.fitToScreen();
	}

}

