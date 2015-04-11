this.require("GUI/LayoutManager");
class this.GUI.BoxLayout extends this.GUI.LayoutManager
{
	constructor( ... )
	{
		this.mHorizontal = true;

		if (vargc > 0 && vargv[0] == this.VERTICAL)
		{
			this.mHorizontal = false;
		}

		if (vargc > 1)
		{
			this.setExpand(vargv[1]);
		}
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
			if (!c.isLayoutManaged())
			{
				continue;
			}

			local d = c.getPreferredSize();

			if (total > 0)
			{
				total += this.mGap;
			}

			if (this.mHorizontal)
			{
				total += d.width;
				max = this.Math.max(d.height, max);
			}
			else
			{
				total += d.height;
				max = this.Math.max(d.width, max);
			}
		}

		if (this.mHorizontal)
		{
			sz.width = total;
			sz.height = max;
		}
		else
		{
			sz.width = max;
			sz.height = total;
		}

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
		local c;
		local visComponents = [];

		if (this.mHorizontal)
		{
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
				local expand = this.mExpand ? true : this.getConstraint(container, c);

				if (expand || d.height > height)
				{
					d.height = height;
				}

				if (x + d.width > width)
				{
					local min = c.getMinimumSize();

					if (x + min.width > width)
					{
						break;
					}

					d.width = width - x;
				}

				visComponents.append(c);
				c._setHidden(false);
				c.setSize(d);
				c.setPosition(x + container.insets.left, y + container.insets.top + (height - d.height) * this.mAlign);
				x += d.width;
				count += 1;
			}

			if (x < width)
			{
				local adjustment = (width - x) * this.mPackAlign;

				if (adjustment > 0)
				{
					foreach( c in visComponents )
					{
						c.setPosition(c.mX + adjustment + container.insets.left, c.mY + container.insets.top);
					}
				}
			}
		}
		else
		{
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
					y += this.mGap;
				}

				local d = c.getPreferredSize();
				local expand = this.mExpand ? true : this.getConstraint(container, c);

				if (expand || d.width > width)
				{
					d.width = width;
				}

				if (y + d.height > height)
				{
					local min = c.getMinimumSize();

					if (y + min.height > height)
					{
						break;
					}

					d.height = height - y;
				}

				visComponents.append(c);
				c._setHidden(false);
				c.setSize(d);
				c.setPosition(x + container.insets.left + (width - d.width) * this.mAlign, y + container.insets.top);
				y += d.height;
				count += 1;
			}

			if (y < height)
			{
				local adjustment = (height - y) * this.mPackAlign;

				if (adjustment > 0)
				{
					foreach( c in visComponents )
					{
						c.setPosition(c.mX + container.insets.left, c.mY + adjustment + container.insets.top);
					}
				}
			}
		}

		while (count < container.components.len())
		{
			container.components[count]._setHidden(true);
			count += 1;
		}
	}

	function setGap( gap )
	{
		this.mGap = gap;
	}

	function setExpand( value )
	{
		this.mExpand = value ? true : false;
	}

	function setAlignment( value )
	{
		this.mAlign = value;
	}

	function setPackAlignment( value )
	{
		this.mPackAlign = value;
	}

	mHorizontal = true;
	mGap = 2;
	mExpand = false;
	mAlign = 0.5;
	mPackAlign = 0.0;
	static HORIZONTAL = 0;
	static VERTICAL = 1;
	static mClassName = "BoxLayout";
}

class this.GUI.BoxLayoutV extends this.GUI.BoxLayout
{
	constructor( ... )
	{
		this.GUI.BoxLayout.constructor(this.GUI.BoxLayout.VERTICAL);

		if (vargc > 0 && vargv[0])
		{
			this.mExpand = true;
		}
	}

}

