this.require("GUI/FlowLayout");
class this.GUI.SingleLineFlowLayout extends this.GUI.FlowLayout
{
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
		local maxPassed = false;
		local rows = [];
		rows.append({});
		rows[rowIndex].height <- 0;
		rows[rowIndex].width <- 0;
		rows[rowIndex].rowComp <- [];
		rows[rowIndex].clip <- false;
		local c;
		local pt;
		local startIndex = container.mStartParsedIndex;
		local i = 0;

		while (i < container.components.len())
		{
			local c = container.components[i];

			if (c.getLayoutExclude())
			{
				i++;
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

			if (i < startIndex)
			{
				c.setVisible(false);
				rows[rowIndex].rowComp.append(c);
			}
			else if (rows[rowIndex].width + d.width + this.mHGap > maxw || maxPassed)
			{
				c.setVisible(false);
				rows[rowIndex].rowComp.append(c);
				maxPassed = true;
			}
			else if (count == 0 || rows[rowIndex].width + d.width + this.mHGap <= maxw)
			{
				if (rows[rowIndex].width > 0)
				{
					rows[rowIndex].width += this.mHGap;
				}

				pt = c.getPosition();

				if (!sim)
				{
					c.setPosition(rows[rowIndex].width + container.insets.left + this.mHGap, this.mVGap + container.insets.top);
					c.setVisible(true);
				}

				rows[rowIndex].height = this.Math.max(rows[rowIndex].height, d.height);
				rows[rowIndex].rowComp.append(c);
				rows[rowIndex].width += d.width;
			}

			i++;
		}

		local difference;

		if (this.mAlign.tolower() == "left")
		{
			difference = 0;
		}
		else if (this.mAlign.tolower() == "right")
		{
			difference = maxw - rows[0].width;
		}
		else if (this.mAlign.tolower() == "center")
		{
			difference = (maxw - rows[0].width) / 2;
		}

		if (!sim)
		{
			foreach( i, x in rows[0].rowComp )
			{
				x.setPosition(x.getPosition().x + difference, x.getPosition().y);
			}
		}

		this.mRows = rows;

		if (sim)
		{
			return {
				width = rows[0].width,
				height = rows[0].height
			};
		}
	}

	mAlign = "left";
	static mClassName = "SingleLineFlowLayout";
}

