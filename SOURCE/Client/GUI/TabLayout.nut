this.require("GUI/LayoutManager");
this.TabPlacement <- {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
};
class this.GUI.TabLayout extends this.GUI.LayoutManager
{
	constructor( tab )
	{
		this.mTab = tab;
	}

	function preferredLayoutSize( container )
	{
		local sz = {
			width = 0,
			height = 0
		};
		local containerSize = this.mTab.getSize();
		local x = 0;
		local y = 0;
		local width = 0;
		local tabHeight = 0;
		local c;

		foreach( c in container.components )
		{
			if (!c.isLayoutManaged())
			{
				continue;
			}

			local d = c.getPreferredSize();

			if (tabHeight == 0)
			{
				tabHeight = d.height;
			}

			if (x > 0)
			{
				x += this.mGap;
			}

			x += d.width;

			if (x > containerSize.width)
			{
				y += tabHeight;
				x = 0;
			}

			if (x > width)
			{
				width = x;
			}
		}

		sz.width = width;
		sz.height = y + tabHeight;
		return container._addInsets(sz);
	}

	function layoutContainer( container )
	{
		local width = container.getWidth() - (container.insets.left + container.insets.right);
		local height = container.getHeight() - (container.insets.top + container.insets.bottom);
		local x = 0;
		local y = 0;
		local count = 0;
		local firstpass = true;
		local d;
		local c;

		foreach( c in container.components )
		{
			if (!c.isLayoutManaged())
			{
				count += 1;
				continue;
			}

			if (firstpass)
			{
				firstpass = false;
			}
			else
			{
				x += this.mGap;
			}

			local d = c.getPreferredSize();

			if (x + d.width > width)
			{
				if (this.TabPlacement.NORTH == this.mTabPlacement)
				{
					y += d.height;
				}
				else if (this.TabPlacement.SOUTH == this.mTabPlacement)
				{
					y -= d.height;
				}

				x = 0;
			}

			c._setHidden(false);
			c.setSize(d);

			if (this.TabPlacement.NORTH == this.mTabPlacement)
			{
				c.setPosition(x + container.insets.left + this.mXOffset, height - d.height - y + this.mYOffset);
			}
			else if (this.TabPlacement.SOUTH == this.mTabPlacement)
			{
				local tabLayers = height / d.height;
				c.setPosition(x + container.insets.left + this.mXOffset, height - d.height * tabLayers - y + this.mYOffset);
			}
			else if (this.TabPlacement.WEST == this.mTabPlacement)
			{
				c.setPosition(d.width + this.mXOffset, y + this.mYOffset);
				y += d.height;
			}

			x += d.width;
			count += 1;
		}
	}

	function setGap( gap )
	{
		this.mGap = gap;
	}

	function setPlacement( placement )
	{
		this.mTabPlacement = placement;
	}

	function setOffset( x, y )
	{
		this.mXOffset = x;
		this.mYOffset = y;
	}

	mTab = null;
	mGap = 0;
	mXOffset = 0;
	mYOffset = 0;
	mTabPlacement = this.TabPlacement.NORTH;
	static HORIZONTAL = 0;
	static VERTICAL = 1;
	static mClassName = "TabLayout";
}

