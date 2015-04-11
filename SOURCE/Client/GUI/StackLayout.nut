this.require("GUI/LayoutManager");
class this.GUI.StackLayout extends this.GUI.LayoutManager
{
	constructor()
	{
	}

	function preferredLayoutSize( container )
	{
		local sz = {
			width = 0,
			height = 0
		};
		local i;
		local j;
		local n;
		local max = 0;
		local total = 0;
		local c;

		foreach( c in container.components )
		{
			if (c.getLayoutExclude())
			{
				continue;
			}

			local d = c.getPreferredSize();

			if (total > 0)
			{
				total += this.mGap;
			}

			total += d.height;
			max = this.Math.max(d.width, max);
		}

		sz.width = max;
		sz.height = total;
		return container._addInsets(sz);
	}

	function layoutContainer( container )
	{
		local width = container.getWidth() - (container.insets.left + container.insets.right);
		local height = container.getHeight() - (container.insets.top + container.insets.bottom);
		local x = container.insets.left;
		local y = container.insets.top;
		local count = 0;
		local firstpass = true;
		local c;

		for( local i = 0; i < container.components.len(); i++ )
		{
			local c = container.components[i];

			if (!c.isLayoutManaged())
			{
				count -= 1;
			}
			else
			{
				if (firstpass)
				{
					firstpass = false;
				}
				else
				{
					y += this.mGap;
				}

				c.setMaximumSize(width, null);
				c.setResize(true);
				local d = c.getPreferredSize();
				d.width = width;

				if (y + d.height > height)
				{
					break;
				}

				c._setHidden(false);
				c.setSize(d);
				y += d.height;
				c.setPosition(x, height - y + container.insets.top + container.insets.bottom);
				count += 1;
			}
		}

		while (count < container.components.len() - 1)
		{
			container.components[count]._setHidden(true);
			count += 1;
		}
	}

	function setGap( gap )
	{
		this.mGap = gap;
	}

	mGap = 0;
	static mClassName = "StackLayout";
}

