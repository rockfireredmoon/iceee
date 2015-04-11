this.require("GUI/LayoutManager");
class this.GUI.GridLayout extends this.GUI.LayoutManager
{
	mHGap = 2;
	mVGap = 2;
	mRows = 1;
	mCols = 1;
	mColSizes = [];
	mRowSizes = [];
	mColumnMajor = false;
	static CENTER = 0;
	static NORTH = 1;
	static SOUTH = 2;
	static EAST = 4;
	static WEST = 8;
	static NORTHEAST = 1 | 4;
	static NORTHWEST = 1 | 8;
	static SOUTHEAST = 2 | 4;
	static SOUTHWEST = 2 | 8;
	static EXPAND_W = 16;
	static EXPAND_H = 32;
	static FILL = 16 | 32;
	static TOP = 1 | 16;
	static BOTTOM = 2 | 16;
	static RIGHT = 4 | 32;
	static LEFT = 8 | 32;
	static mClassName = "GridLayout";
	constructor( rows, cols )
	{
		if (cols < 0 || rows < 0)
		{
			throw this.Exception("Invalid arguments to GUI.GridLayout()");
		}

		this.mCols = cols;
		this.mRows = rows;
		this.mColSizes = this.array(this.mCols, "*");
		this.mRowSizes = this.array(this.mRows, "*");
	}

	function setColumns( ... )
	{
		if (vargc != this.mCols)
		{
			throw this.Exception("Invalid argument count (want " + this.mCols + ")");
		}

		local i;

		for( i = 0; i < vargc; i++ )
		{
			this.setColumnSize(i, vargv[i]);
		}
	}

	function setColumnSize( col, size )
	{
		if (col < 0 || col > this.mCols)
		{
			throw this.Exception("Invalid column");
		}

		this.mColSizes[col] = size;
	}

	function setRows( ... )
	{
		if (vargc != this.mRows)
		{
			throw this.Exception("Invalid argument count (want " + this.mRows + ")");
		}

		local i;

		for( i = 0; i < vargc; i++ )
		{
			this.setRowSize(i, vargv[i]);
		}
	}

	function setRowSize( row, size )
	{
		if (row < 0 || row > this.mRows)
		{
			throw this.Exception("Invalid column");
		}

		this.mRowSizes[row] = size;
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
		local maxW = 0;
		local maxH = 0;
		local totalW = 0;
		local totalH = 0;
		n = 0;

		for( j = 0; j < this.mRows; j++ )
		{
			local rowH = this.mRowSizes[j];

			for( i = 0; i < this.mCols; n++ )
			{
				if (n >= container.components.len())
				{
				}
				else
				{
					local c = container.components[n];

					if (!c.isLayoutManaged())
					{
					}
					else
					{
						local d = c.getPreferredSize();
						local colW = this.mColSizes[i];

						if (rowH != null && rowH != "*")
						{
							maxH = this.Math.max(maxH, rowH);
						}
						else
						{
							maxH = this.Math.max(maxH, d.height);
						}

						if (colW != null && colW != "*")
						{
							maxW = this.Math.max(maxW, colW);
						}
						else
						{
							maxW = this.Math.max(maxW, d.width);
						}
					}
				}

				i++;
			}
		}

		for( i = 0; i < this.mCols; i++ )
		{
			local size = this.mColSizes[i];

			if (size != null && size != "*")
			{
				totalW += size;
			}
			else
			{
				totalW += maxW;
			}
		}

		for( i = 0; i < this.mRows; i++ )
		{
			local size = this.mRowSizes[i];

			if (size != null && size != "*")
			{
				totalH += size;
			}
			else
			{
				totalH += maxH;
			}
		}

		sz.width = totalW + this.mHGap * (this.mCols - 1) + container.insets.left + container.insets.right;
		sz.height = totalH + this.mVGap * (this.mRows - 1) + container.insets.top + container.insets.bottom;
		return sz;
	}

	function _calcSizes( presetSizes, available )
	{
		local i;
		local n = presetSizes.len();
		local free = 0;
		local perFree = 0;

		for( i = 0; i < n; i++ )
		{
			local v = presetSizes[i];

			if (v == null || v == "*")
			{
				free += 1;
			}
			else
			{
				available -= v;
			}
		}

		if (available < 0)
		{
			available = 0;
		}

		local perFree = free > 0 ? this.Math.max(available / free, 0) : 0;
		local finalSizes = this.array(presetSizes.len());

		for( i = 0; i < n; i++ )
		{
			local v = presetSizes[i];

			if (v == null || v == "*")
			{
				finalSizes[i] = perFree;
			}
			else
			{
				finalSizes[i] = v;
			}
		}

		return finalSizes;
	}

	function layoutContainer( container )
	{
		if (this.mRows == 0 || this.mCols == 0)
		{
			return;
		}

		local width = container.getWidth() - (container.insets.left + container.insets.right) - this.mHGap * (this.mCols - 1);
		local height = container.getHeight() - (container.insets.top + container.insets.bottom) - this.mVGap * (this.mRows - 1);
		local i;
		local j;
		local n;
		local x = 0;
		local y = 0;
		local colW = this._calcSizes(this.mColSizes, width);
		local rowH = this._calcSizes(this.mRowSizes, height);
		local major;
		local minor;
		local minorOffset = 0;

		if (this.mColumnMajor)
		{
			major = this.mCols;
			minor = this.mRows;
		}
		else
		{
			major = this.mRows;
			minor = this.mCols;
		}

		local y = container.insets.top;
		local x = container.insets.left;
		n = 0;

		for( j = 0; j < major; j++ )
		{
			minorOffset = 0;

			for( i = 0; i < minor - minorOffset; n++ )
			{
				local span = 1;
				local widthAdjustment = 0;
				local heightAdjustment = 0;

				if (n < container.components.len())
				{
					local c = container.components[n];

					if (c.isLayoutManaged())
					{
						c._setHidden(false);
						local sz = c.getPreferredSize();
						local constraints = this.getConstraint(container, c);
						local anchor = this.FILL;

						if (typeof constraints == "integer")
						{
							anchor = constraints;
						}
						else if (typeof anchor != "table")
						{
							if ("anchor" in constraints)
							{
								anchor = constraints.anchor;
							}

							if ("span" in constraints)
							{
								span = constraints.span;
							}
						}

						local _x;
						local _y;
						local _w;
						local _h;
						local cellW;
						local cellH;

						if (this.mColumnMajor)
						{
							cellW = colW[j];
							cellH = rowH[i];
							minorOffset += span - 1;
						}
						else
						{
							cellW = colW[i];
							cellH = rowH[j];
							minorOffset += span - 1;
						}

						if (this.mColumnMajor)
						{
							if (span > 1)
							{
								for( local k = 0; k < span - 1; k++ )
								{
									if (colW.len() > j + k)
									{
										heightAdjustment += rowH[j + k] + this.mVGap;
									}
								}
							}
						}
						else if (span > 1)
						{
							for( local k = 0; k < span - 1; k++ )
							{
								if (colW.len() > i + k)
								{
									widthAdjustment += colW[i + k] + this.mHGap;
								}
							}
						}

						if (anchor & this.EXPAND_W)
						{
							_w = cellW;
							_x = x;
						}
						else
						{
							_w = this.Math.min(sz.width, cellW);

							if (anchor & this.WEST)
							{
								_x = x;
							}
							else if (anchor & this.EAST)
							{
								_x = x + cellW - _w;
							}
							else
							{
								_x = x + (cellW - _w) / 2;
							}
						}

						if (anchor & this.EXPAND_H)
						{
							_h = cellH;
							_y = y;
						}
						else
						{
							_h = this.Math.min(sz.height, cellH);

							if (anchor & this.NORTH)
							{
								_y = y;
							}
							else if (anchor & this.SOUTH)
							{
								_y = y + cellH - _h;
							}
							else
							{
								_y = y + (cellH - _h) / 2;
							}
						}

						c.setPosition(_x, _y);
						c.setSize(_w + widthAdjustment, _h + heightAdjustment);
					}
				}

				if (this.mColumnMajor)
				{
					y += rowH[i] + this.mVGap;
					y += heightAdjustment;
				}
				else
				{
					x += colW[i] + this.mHGap;
					x += widthAdjustment;
				}

				i++;
			}

			if (this.mColumnMajor)
			{
				x += colW[j] + this.mHGap;
				y = container.insets.top;
			}
			else
			{
				y += rowH[j] + this.mVGap;
				x = container.insets.left;
			}
		}

		if (n < container.components.len())
		{
			local found = false;
			local str = "";

			while (n < container.components.len())
			{
				str += "(" + container.components[n] + ") ";

				if (container.components[n].isLayoutManaged())
				{
					container.components[n]._setHidden(true);
					found = true;
				}

				n++;
			}

			if (found)
			{
				this.log.warn("Hiding extra components due to small grid layout: " + str);
			}
		}
	}

	function setGaps( hgap, vgap )
	{
		this.mHGap = hgap;
		this.mVGap = vgap;
	}

	function setColumnMajor( value )
	{
		this.mColumnMajor = value;
	}

}

