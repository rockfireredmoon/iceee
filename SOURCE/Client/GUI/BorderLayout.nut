this.require("GUI/LayoutManager");
class this.GUI.BorderLayout extends this.GUI.LayoutManager
{
	function preferredLayoutSize( container )
	{
		local bc = this._getBorderTable(container);
		local sz = {
			width = 0,
			height = 0
		};
		local ps;
		local c;
		local topW = 0;
		local midW = 0;
		local eastW = 0;
		local westW = 0;
		local botW = 0;

		if ("north" in bc)
		{
			ps = bc.north.getPreferredSize();
			topW = ps.width;
			sz.height += ps.height;
		}

		if ("south" in bc)
		{
			ps = bc.south.getPreferredSize();
			botW = ps.width;
			sz.height += ps.height;
		}

		local sideH = 0;

		if ("east" in bc)
		{
			ps = bc.east.getPreferredSize();
			eastW = ps.width;
			sideH = this.Math.max(sideH, ps.height);
		}

		if ("west" in bc)
		{
			ps = bc.west.getPreferredSize();
			westW = ps.width;
			sideH = this.Math.max(sideH, ps.height);
		}

		if ("center" in bc)
		{
			ps = bc.center.getPreferredSize();
			sideH = this.Math.max(sideH, ps.height);
			sz.width = this.Math.max(sz.width, ps.width);
		}

		sz.height += sideH;
		sz.width += eastW + westW;
		sz.width = this.Math.max(sz.width, topW);
		sz.width = this.Math.max(sz.width, botW);
		return container._addInsets(sz);
	}

	function layoutContainer( container )
	{
		local bc = this._getBorderTable(container);
		local fullWidth = container.getWidth();
		local fullHeight = container.getHeight();
		local width = fullWidth - (container.insets.left + container.insets.right);
		local height = fullHeight - (container.insets.top + container.insets.bottom);
		local ps;
		local c;
		local keepVis = [];
		local north = container.insets.top;
		local south = container.insets.bottom;
		local east = container.insets.right;
		local west = container.insets.left;

		if ("north" in bc)
		{
			c = bc.north;
			keepVis.append(c);
			ps = c.getPreferredSize();
			c.setPosition(container.insets.left, container.insets.top);
			c.setSize(width, ps.height);
			north += ps.height;
		}

		if ("south" in bc)
		{
			c = bc.south;
			keepVis.append(c);
			ps = c.getPreferredSize();
			c.setPosition(container.insets.left, fullHeight - ps.height - container.insets.bottom);
			c.setSize(width, ps.height);
			south += ps.height;
		}

		if ("west" in bc)
		{
			c = bc.west;
			keepVis.append(c);
			ps = c.getPreferredSize();
			c.setPosition(west, north);
			c.setSize(ps.width, fullHeight - north - south);
			west += ps.width;
		}

		if ("east" in bc)
		{
			c = bc.east;
			keepVis.append(c);
			ps = c.getPreferredSize();
			c.setPosition(fullWidth - ps.width - container.insets.right, north);
			c.setSize(ps.width, fullHeight - north - south);
			east += ps.width;
		}

		if ("center" in bc)
		{
			c = bc.center;
			keepVis.append(c);
			local w = fullWidth - (east + west);
			local h = fullHeight - (north + south);

			if (w < 0)
			{
				w = 0;
			}

			if (h < 0)
			{
				h = 0;
			}

			c.setPosition(west, north);
			c.setSize(w, h);
		}

		local c;

		foreach( c in container.components )
		{
			if (!c.getLayoutExclude())
			{
				local vc;
				local found = false;

				foreach( vc in keepVis )
				{
					if (vc == c)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					c.setVisible(false);
				}
			}
		}
	}

	function _getBorderTable( container )
	{
		local results = {};
		local c;

		foreach( c in container.components )
		{
			if (!c.isVisible() || c.getLayoutExclude())
			{
				continue;
			}

			local constraint = this.getConstraint(container, c);

			if (constraint == null && !("center" in results))
			{
				results.center <- c;
				continue;
			}

			if (!constraint || typeof constraint != "string")
			{
				continue;
			}

			constraint = constraint.tolower();

			if (constraint == "north" || constraint == "south" || constraint == "east" || constraint == "west" || constraint == "center")
			{
				if (!(constraint in results))
				{
					results[constraint] <- c;
				}
				else
				{
					this.print("[WARNING] multiple \"" + constraint + "\" components in " + container);
					results[constraint] = c;
				}
			}
		}

		return results;
	}

	static NORTH = "north";
	static EAST = "east";
	static SOUTH = "south";
	static WEST = "west";
	static CENTER = "center";
	static mClassName = "BorderLayout";
}

