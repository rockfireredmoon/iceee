this.require("GUI/LayoutManager");
class this.GUI.FlowLayout extends this.GUI.LayoutManager
{
	function preferredLayoutSize( container )
	{
		local sz = this.layoutContainer(container, true);
		sz.width += this.mHGap * 2 + container.insets.left + container.insets.right;
		sz.height += this.mVGap * 2 + container.insets.top + container.insets.bottom;
		return sz;
	}

	function _trimRows( row, container )
	{
		local lastIndex = row.rowComp.len() - 1;

		if (lastIndex >= 0)
		{
			local c = this.r.rowComp[lastIndex];
			local constraint = this.getConstraint(container, c);

			if (constraint == "NoLineStart")
			{
				c.setVisible(false);
				this.r.width -= c.getWidth();
			}
		}
	}

	function layoutContainer( container, ... )
	{
		local sim = false;

		if (vargc > 0)
		{
			sim = vargv[0];
		}

		local y = 0;
		local maxw;
		local maxh;

		if (sim)
		{
			local maxsz = container.getMaxSize();

			if (maxsz.width == null)
			{
				maxsz.width = 60000;
			}

			if (maxsz.height == null)
			{
				maxsz.height = 60000;
			}

			maxw = maxsz.width - (container.insets.left + container.insets.right + this.mHGap * 2);
			maxh = maxsz.height - (container.insets.top + container.insets.bottom + this.mVGap * 2);
		}
		else
		{
			maxw = container.getWidth() - (container.insets.left + container.insets.right + this.mHGap * 2);
			maxh = container.getHeight() - (container.insets.top + container.insets.bottom + this.mVGap * 2);
		}

		local rowIndex = 0;
		local count = 0;
		local rows = [];
		rows.append({});
		rows[rowIndex].height <- 0;
		rows[rowIndex].width <- 0;
		rows[rowIndex].rowComp <- [];
		local c;
		local pt;

		foreach( c in container.components )
		{
			if (c.getLayoutExclude())
			{
				continue;
			}

			local constraint = this.getConstraint(container, c);
			local d = c.getPreferredSize();

			if (!sim)
			{
				c.setWidth(d.width);
				c.setHeight(d.height);
			}

			count = rows[rowIndex].rowComp.len();

			if (count == 0 || rows[rowIndex].width + d.width <= maxw)
			{
				if (rows[rowIndex].width > 0)
				{
					rows[rowIndex].width += this.mHGap;
				}

				pt = c.getPosition();

				if (!sim)
				{
					c.setPosition(rows[rowIndex].width, 0);
				}

				rows[rowIndex].height = this.Math.max(rows[rowIndex].height, d.height);
				rows[rowIndex].rowComp.append(c);

				if (constraint == "break")
				{
					rows[rowIndex].width -= this.mHGap;
					rowIndex++;
					rows.append({});
					rows[rowIndex].height <- 0;
					rows[rowIndex].width <- 0;
					rows[rowIndex].rowComp <- [];
				}
				else if (constraint == "NoLineStart" && rows[rowIndex].width == this.mHGap && container.components[container.components.len() - 1] != c)
				{
					c.setVisible(false);
				}
				else
				{
					rows[rowIndex].width += d.width;
				}
			}
			else
			{
				rows[rowIndex].width -= this.mHGap;
				rowIndex++;
				rows.append({});
				rows[rowIndex].height <- 0;
				rows[rowIndex].width <- 0;
				rows[rowIndex].rowComp <- [];

				if (rows[rowIndex].width > 0)
				{
					rows[rowIndex].width += this.mHGap;
				}

				pt = c.getPosition();

				if (!sim)
				{
					c.setPosition(rows[rowIndex].width, 0);
				}

				rows[rowIndex].height = this.Math.max(rows[rowIndex].height, d.height);

				if (constraint != "NoLineStart" || container.components[container.components.len() - 1] == c)
				{
					rows[rowIndex].width += d.width;
				}
				else
				{
					c.setVisible(false);
				}

				rows[rowIndex].rowComp.append(c);
			}
		}

		this.mRows = rows;
		return this._positionRows(rows, maxw, maxh, container, sim);
	}

	function getRows()
	{
		if (!this.mRows || this.mRows.len() == 0)
		{
			return null;
		}

		return this.mRows;
	}

	function setAlignment( align )
	{
		local a = align.tolower();

		if (a != "left" && a != "center" && a != "right")
		{
			throw this.Exception("Invalid alignment: " + align);
		}

		this.mAlign = align;
	}

	function setGaps( hgap, vgap )
	{
		this.mHGap = hgap;
		this.mVGap = vgap;
	}

	function getRowIndexByComponent( pComponent )
	{
		foreach( i, r in this.mRows )
		{
			foreach( c in r.rowComp )
			{
				if (c == pComponent)
				{
					return i;
				}
			}
		}
	}

	function _checkFill( rows, index, maxh )
	{
		local filled = false;
		local indexCheck = index - 1;

		while (index > 0 && filled == false)
		{
			local y = 0;

			for( local i = indexCheck; i < rows.len(); i++ )
			{
				y += rows[i].height;
				y += this.mVGap;
			}

			y -= this.mVGap;

			if (y >= maxh)
			{
				filled = true;
			}

			if (!filled)
			{
				index--;
				indexCheck--;
			}
		}

		return index;
	}

	function _positionRows( pRows, maxw, maxh, container, sim )
	{
		local y = 0;
		local yAdd = container.insets.top;
		local simW = 0;
		local simH = 0;
		local clip = true;
		local count = 0;
		local index = 0;
		local scroll = container.getScroll();

		if (scroll)
		{
			index = scroll.getIndex();
		}

		if (scroll)
		{
			scroll.setIndex(index);
		}

		foreach( i, r in pRows )
		{
			local hspace = maxw - r.width;
			local x = container.insets.left;

			switch(this.mAlign)
			{
			case "left":
				x += 0;
				break;

			case "center":
				x += hspace / 2;
				break;

			case "right":
				x += hspace;
				break;
			}

			if (count >= index)
			{
				clip = false;
				pRows[i].clip <- false;
			}

			if (y + r.height > maxh)
			{
				clip = true;
			}

			if (sim || !clip)
			{
				simW = ::Math.max(simW, r.width);
			}

			if (!sim)
			{
				foreach( c in r.rowComp )
				{
					if (!clip || count == index)
					{
						local pt = c.getPosition();
						c.setPosition(pt.x + x, y + yAdd + (r.height - c.getHeight()));
						c.setVisible(true);
						pRows[i].clip <- false;
					}
					else
					{
						c.setVisible(false);
						pRows[i].clip <- true;
					}
				}
			}

			if (!clip || count == index)
			{
				y += r.height + this.mVGap;
			}

			count++;
		}

		if (sim)
		{
			simH = y;
			return {
				width = simW,
				height = simH
			};
		}
	}

	mAlign = "center";
	mHGap = 5;
	mVGap = 5;
	mRows = null;
	mSingleRow = false;
	static mClassName = "FlowLayout";
}

