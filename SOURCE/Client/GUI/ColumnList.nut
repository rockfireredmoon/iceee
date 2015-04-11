this.require("GUI/Spacer");
this.require("GUI/Component");
class this.GUI.ColumnListRow extends this.GUI.Spacer
{
	constructor( appearance, w, h, selectionInsets, ... )
	{
		this.GUI.Spacer.constructor(w, h);
		this.setInsets(0, 0, 0, 0);
		this.setAppearance(appearance);
		this.setSize(w, h);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mCells = [];
		this.setLayoutManager(this.GUI.BoxLayout());
		this.getLayoutManager().setGap(2);
		local rzo;
		local rsel;
		rzo = ::GUI.ZOffset(2);
		rzo.setPosition(0, 0);
		rzo.setSize(this.getSize());
		rzo.setLayoutExclude(true);
		rzo.setPassThru(true);
		rsel = ::GUI.Component();

		if (this.mAppearance)
		{
			rsel.setAppearance(this.mAppearance + "/Selection");
		}

		rsel.setPosition(selectionInsets[0], selectionInsets[1]);
		rsel.setSize(this.getWidth() - selectionInsets[0] * 2, this.getHeight() - selectionInsets[1]);
		rsel.setLayoutExclude(true);
		rsel.setPassThru(true);
		rzo.getDeepest().add(rsel);
		this.add(rzo);
		this.mSel = rzo;
		this.setSelectionVisible(false);

		if (vargc == 1)
		{
			foreach( width in vargv[0] )
			{
				local c = this.GUI.Spacer(width, this.getHeight());
				c.setInsets(0, 2, 0, 2);
				c.setSize(width, this.getHeight());

				if (this.mAppearance)
				{
					c.setAppearance(this.mAppearance + "/Cell");
				}

				c.setLayoutManager(this.GUI.GridLayout(1, 1));
				c.getLayoutManager().setGaps(0, 0);
				this.addCell(c);
			}
		}
	}

	function addCell( c )
	{
		this.mCells.append(c);
		this.add(c);
	}

	function getCell( i )
	{
		return this.mCells[i];
	}

	function numCells()
	{
		return this.mCells.len();
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function onMousePressed( evt )
	{
		this.mMessageBroadcaster.broadcastMessage("onRowSelect", this, evt);
	}

	function setSelectionVisible( vis )
	{
		if (vis)
		{
			this.mSel.setVisible(true);
		}
		else
		{
			this.mSel.setVisible(false);
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	mMessageBroadcaster = null;
	mCells = null;
	mSel = null;
}

class this.GUI.ColumnList extends this.GUI.Component
{
	constructor()
	{
		this.mColumnDividers = [];
		this.mColumnNames = [];
		this.mColumnWidths = [];
		this.mRowContents = [];
		this.mSelectedRows = [];
		this.mSelectionInsets = [
			0,
			0
		];
		this.mMultipleSelectionCapable = false;
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.GUI.Component.constructor();
		this.mAppearance = "ColumnList";
		this.mHeadingBar = this.GUI.Component();
		this.mHeadingBar.setAppearance(this.mAppearance + "/HeadingBar");
		this.add(this.mHeadingBar);
		this.mDataPane = this.GUI.Component();
		this.mDataPane.setAppearance(this.mAppearance + "/DataPane");
		this.mDataPane.setLayoutManager(this.GUI.FlowLayout());
		this.mDataPane.getLayoutManager().setGaps(0, this.mRowGap);
		this.add(this.mDataPane);
		this.mHeadingDivider = this.GUI.Component();
		this.mHeadingDivider.setAppearance(this.mAppearance + "/HeadingDivider");
		this.mHeadingDivider.setLayoutExclude(true);
		this.add(this.mHeadingDivider);
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Component._removeNotify();
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();

		if (this.mOldSize == null || this.getWidth() != this.mOldSize.width || this.getHeight() != this.mOldSize.height)
		{
			this.mOldSize = this.getSize();
			this._recalc();
		}
	}

	function getPreferredSize()
	{
		local sz = {
			width = this.mTotalColumnWidths,
			height = this.mRowHeight
		};
		return this._addInsets(sz);
	}

	function setScroll( pScroll )
	{
		pScroll.addActionListener(this);
		this.mScroll = pScroll;
	}

	function onScrollUpdate( sbar )
	{
		local panespace = this.mDataPane.getHeight() - this.mDataPane.insets.top - this.mDataPane.insets.bottom - this.mRowGap;
		local rowsize = this.mRowHeight + this.mRowGap;
		local displayedcount = (panespace / rowsize).tointeger();
		local maxindex = this.Math.max(this.getRowCount() - displayedcount, 0);

		if (this.mDataPane == null || this.mRowContents == null)
		{
			return;
		}

		if (sbar.getIndex() < 0)
		{
			sbar.setIndex(0);
		}
		else if (sbar.getIndex() >= maxindex)
		{
			sbar.setIndex(maxindex);
		}

		if (this.mWindowBase == sbar.getIndex())
		{
			return;
		}

		this.mWindowBase = sbar.getIndex();
		this._displayAllRows();
		this.mDataPane.invalidate();
	}

	function setWindowSize( pSize )
	{
		if (this.mWindowLen == pSize)
		{
			return;
		}

		this.mWindowBase = 0;
		this.mWindowLen = this.Math.max(pSize, 1);
		this._recalc();
	}

	function getWindowSize()
	{
		return this.mWindowLen;
	}

	function _deselectRow( index )
	{
		if (index < 0 || index >= this.getRowCount())
		{
			this.print("tried to deselect nonexistant row" + index + "!");
			return;
		}

		for( local i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (this.mSelectedRows[i] == index)
			{
				if (index >= this.mWindowBase && index < this.mWindowBase + this.mWindowLen)
				{
					local row;
					row = this.mDataPane.components[index - this.mWindowBase];
					row.setSelectionVisible(false);
				}

				this.mSelectedRows.remove(i);
				this.mMessageBroadcaster.broadcastMessage("onRowSelectionChanged", this, index, false);
				return;
			}
		}

		return;
	}

	function _selectRow( index )
	{
		if (index < 0 || index >= this.getRowCount())
		{
			this.print("tried to select nonexistant row " + index + "!");
			return;
		}

		for( local i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (this.mSelectedRows[i] == index)
			{
				return;
			}
		}

		if (index >= this.mWindowBase && index < this.mWindowBase + this.mWindowLen)
		{
			local row;
			row = this.mDataPane.components[index - this.mWindowBase];
			row.setSelectionVisible(true);
		}

		this.mSelectedRows.append(index);
		this.mMessageBroadcaster.broadcastMessage("onRowSelectionChanged", this, index, true);
		return;
	}

	function _toggleRow( index )
	{
		if (index < 0 || index >= this.getRowCount())
		{
			throw this.Exception("Invalid index to GUI.ColumnList._toggleRow()");
		}

		for( local i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (this.mSelectedRows[i] == index)
			{
				this._deselectRow(index);
				return;
			}
		}

		this._selectRow(index);
		return;
	}

	function onRowSelect( row, evt )
	{
		local rowind;
		local additive = evt.isControlDown();
		local region = evt.isShiftDown();

		if (evt.clickCount == 2)
		{
			this.mMessageBroadcaster.broadcastMessage("onDoubleClick", this, evt);
		}

		for( rowind = 0; rowind < this.mDataPane.components.len(); rowind++ )
		{
			if (row == this.mDataPane.components[rowind])
			{
				rowind += this.mWindowBase;
				break;
			}
		}

		if (this.mMultipleSelectionCapable == false || !additive && !region)
		{
			local tarr = [];
			tarr.append(rowind);
			this.setSelectedRows(tarr);
			this.mLastClickedRow = rowind;
			return;
		}

		if (additive && !region)
		{
			this._toggleRow(rowind);
			this.mLastClickedRow = rowind;
			return;
		}

		if (!additive && region)
		{
			local lcr = this.mLastClickedRow;

			if (lcr >= this.getRowCount())
			{
				lcr = this.getRowCount() - 1;
			}

			if (lcr < 0)
			{
				lcr = 0;
			}

			local first = this.Math.min(rowind, lcr);
			local last = this.Math.max(rowind, lcr);
			local selme = [];

			for( local i = first; i <= last; i++ )
			{
				selme.append(i);
			}

			this.setSelectedRows(selme);
			return;
		}

		if (additive && region)
		{
			local lcr = this.mLastClickedRow;

			if (lcr >= this.getRowCount())
			{
				lcr = this.getRowCount() - 1;
			}

			if (lcr < 0)
			{
				lcr = 0;
			}

			local first = this.Math.min(rowind, lcr);
			local last = this.Math.max(rowind, lcr);

			for( local i = first; i <= last; i++ )
			{
				this._selectRow(i);
			}

			return;
		}
	}

	function _recalcAllRowHeight()
	{
		if (this.mForcedRowHeight != 0)
		{
			if (this.mRowHeight == this.mForcedRowHeight)
			{
				return;
			}

			this.mRowHeight = this.mForcedRowHeight;
			this._recalc();
			return;
		}

		local rh = 1;

		foreach( row in this.mDataPane.components )
		{
			rh = this.Math.max(rh, this._getRowHeight(row));
		}

		if (rh != this.mRowHeight)
		{
			this.mRowHeight = rh;
			this._recalc();
		}
	}

	function _recalcRowHeight( r )
	{
		local rh = this._getRowHeight(r);

		if (rh > this.mRowHeight)
		{
			this.mRowHeight = rh;
			this._recalc();
		}
	}

	function _getRowHeight( row )
	{
		local trh = 1;

		for( local i = 0; i < row.numCells(); i++ )
		{
			foreach( ele in row.getCell(i).components )
			{
				if (ele.getPreferredSize().height > trh)
				{
					trh = ele.getPreferredSize().height;
				}
			}
		}

		return trh;
	}

	function setForcedRowHeight( height )
	{
		if (height == this.mForcedRowHeight)
		{
			return;
		}

		if (height <= 0)
		{
			this.mForcedRowHeight = 0;
		}
		else
		{
			this.mForcedRowHeight = height;
		}

		this._recalcAllRowHeight();
	}

	function getForcedRowHeight()
	{
		return this.mForcedRowHeight;
	}

	function setMultipleSelectionCapable( msc )
	{
		if (msc)
		{
			this.mMultipleSelectionCapable = true;
		}
		else
		{
			this.mMultipleSelectionCapable = false;
		}
	}

	function isMultipleSelectionCapable()
	{
		return this.mMultipleSelectionCapable;
	}

	function isShowingHeaders()
	{
		return this.mShowingHeaders;
	}

	function setShowingHeaders( visible )
	{
		if (this.mShowingHeaders == visible)
		{
			return;
		}

		this.mShowingHeaders = visible;

		if (this.mHeadingBar)
		{
			this.mHeadingBar.setVisible(visible);
		}

		if (this.mHeadingDivider)
		{
			this.mHeadingDivider.setVisible(visible);
		}

		this._recalc();
	}

	function setAppearance( pAppearance )
	{
		this.GUI.Component.setAppearance(pAppearance);

		if (pAppearance)
		{
			this.mHeadingBar.setAppearance(this.mAppearance + "/HeadingBar");
			this.mDataPane.setAppearance(this.mAppearance + "/DataPane");
			this.mHeadingDivider.setAppearance(this.mAppearance + "/HeadingDivider");
		}
		else
		{
			this.mHeadingBar.setAppearance(this.mAppearance);
			this.mDataPane.setAppearance(this.mAppearance);
			this.mHeadingDivider.setAppearance(this.mAppearance);
		}

		this._recalc();
	}

	function addColumn( pname, pwidth )
	{
		if (pname != null && typeof pname == "string")
		{
			this.mColumnNames.append(pname);
		}
		else
		{
			this.mColumnNames.append("");
		}

		if (pwidth > 0)
		{
			this.mColumnWidths.append(pwidth);
		}
		else
		{
			this.print("Invalid column width " + pwidth + " for column " + pname + ", setting to 1");
			this.mColumnWidths.append(1);
		}

		local tw = 0;

		foreach( foo in this.mColumnWidths )
		{
			tw += foo;
		}

		if (tw > 0)
		{
			this.mTotalColumnWidths = tw;
		}
		else
		{
			this.mTotalColumnWidths = 1;
		}

		this._recalc();
	}

	function _recalc()
	{
		local headingHeight = this.mShowingHeaders ? this.mHeadingHeight : 0;
		local dividerHeight = this.mShowingHeaders ? this.mDividerHeight : 0;
		this.mHeadingBar.setSize(this.getWidth() - this.insets.left - this.insets.right, headingHeight);
		this.mHeadingBar.setPosition(this.insets.left, this.insets.top);
		this.mDataPane.setSize(this.getWidth() - this.insets.left - this.insets.right, this.getHeight() - headingHeight - this.insets.top - dividerHeight);
		this.mDataPane.setPosition(this.insets.left, headingHeight + dividerHeight);
		this.mHeadingDivider.setSize(this.getWidth(), this.mDividerHeight);
		this.mHeadingDivider.setPosition(0, headingHeight + this.insets.top);

		foreach( div in this.mColumnDividers )
		{
			this.remove(div);
		}

		this.mColumnDividers = [];
		this.mHeadingBar.removeAll();
		local xoffset = this.insets.left;

		for( local i = 0; i < this.mColumnNames.len(); i++ )
		{
			if (i > 0)
			{
				local div = this.GUI.Component();

				if (this.mAppearance)
				{
					div.setAppearance(this.mAppearance + "/ColumnDivider");
				}

				div.setPosition(xoffset, 0);
				div.setSize(this.mDividerWidth, this.getHeight());
				this.add(div);
				this.mColumnDividers.append(div);
				xoffset += this.mDividerWidth;
			}

			local c = this.GUI.Component();

			if (this.mAppearance)
			{
				c.setAppearance(this.mAppearance + "/HeadingBar/Cell");
			}

			c.setSize(this.columnPixelWidth(i), headingHeight);
			c.setPosition(xoffset, 0);
			c.setLayoutManager(this.GUI.FlowLayout());
			c.getLayoutManager().setAlignment("center");
			c.setLayoutExclude(true);
			xoffset += this.columnPixelWidth(i);
			this.mHeadingBar.add(c);
			c.add(this.GUI.Label(this.mColumnNames[i]));
		}

		this.mDataPane.removeAll();

		for( local i = 0; i < this.mWindowLen; i++ )
		{
			this.mDataPane.add(this._createBlankRow());
		}

		this._displayAllRows();
	}

	function _displayRow( index )
	{
		local dispind = index - this.mWindowBase;

		if (dispind < 0 || dispind >= this.mWindowLen)
		{
			return;
		}

		local r = this.mDataPane.components[dispind];

		for( local c = 0; c < this.mColumnNames.len(); c++ )
		{
			r.getCell(c).removeAll();

			if (index < this.getRowCount())
			{
				local rowlabel;

				if (typeof this.mRowContents[index][c] == "string")
				{
					rowlabel = this.GUI.Label(this.mRowContents[index][c]);
					rowlabel.setAutoFit(true);
				}
				else if (typeof this.mRowContents[index][c] == "instance")
				{
					rowlabel = this.mRowContents[index][c];
				}

				rowlabel.setSize(r.getCell(c).getSize());
				r.getCell(c).add(rowlabel);
			}
		}

		local i;

		for( i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (this.mSelectedRows[i] == index)
			{
				r.setSelectionVisible(true);
				break;
			}
		}

		if (i >= this.mSelectedRows.len())
		{
			r.setSelectionVisible(false);
		}

		this._recalcRowHeight(r);
	}

	function _uncorrectedColumnPixelWidth( index )
	{
		if (index >= this.mColumnWidths.len() || index < 0)
		{
			return 0;
		}

		local dw;

		if (this.mColumnWidths.len() > 1)
		{
			dw = (this.mColumnWidths.len() - 1) * this.mDividerWidth;
		}
		else
		{
			dw = 0;
		}

		return (1.0 * this.mColumnWidths[index] / this.mTotalColumnWidths * (this.getWidth() - this.insets.left - this.insets.right - dw)).tointeger();
	}

	function columnPixelWidth( index )
	{
		local tup = 0;

		for( local i = 0; i < this.mColumnWidths.len(); i++ )
		{
			tup += this._uncorrectedColumnPixelWidth(i);
		}

		local dw;

		if (this.mColumnWidths.len() > 1)
		{
			dw = (this.mColumnWidths.len() - 1) * this.mDividerWidth;
		}
		else
		{
			dw = 0;
		}

		local sparepixels = this.getWidth() - this.insets.left - this.insets.right - dw - tup;

		if (index >= sparepixels)
		{
			return this._uncorrectedColumnPixelWidth(index);
		}
		else
		{
			return this._uncorrectedColumnPixelWidth(index) + 1;
		}
	}

	function _createBlankRow()
	{
		local widths = [];

		for( local colnum = 0; colnum < this.mColumnNames.len(); colnum++ )
		{
			widths.append(this.columnPixelWidth(colnum));
		}

		local r = this.GUI.ColumnListRow(this.mRowAppearance, this.getWidth(), this.mRowHeight, this.mSelectionInsets, widths);
		r.addActionListener(this);
		return r;
	}

	function setSelectionInsets( insets )
	{
		this.mSelectionInsets = insets;
	}

	function getRowCount()
	{
		return this.mRowContents.len();
	}

	function getSelectedRows()
	{
		local ret = [];

		foreach( r in this.mSelectedRows )
		{
			ret.append(r);
		}

		return ret;
	}

	function setSelectedRows( rowarray )
	{
		if (typeof rowarray != "array" && rowarray != null)
		{
			throw this.Exception("Invalid arguments to GUI.ColumnList.setSelectedRows()");
		}

		if (rowarray == null)
		{
			rowarray = [];
		}

		local tempmsr = [];

		foreach( i in rowarray )
		{
			if (typeof i != "integer")
			{
				throw this.Exception("Invalid array passed to GUI.ColumnList.setSelectedRows()");
				return;
			}
			else if (i < 0 || i >= this.getRowCount())
			{
				this.print("tried to select nonexistant row " + i + "!");
			}
			else
			{
				tempmsr.append(i);
			}
		}

		if (this.mOneSelectMin && tempmsr.len() == 0)
		{
			return;
		}

		local deselme = [];

		foreach( r in this.mSelectedRows )
		{
			local keeper = false;

			foreach( s in tempmsr )
			{
				if (s == r)
				{
					keeper = true;
				}
			}

			if (!keeper)
			{
				deselme.append(r);
			}
		}

		foreach( r in deselme )
		{
			this._deselectRow(r);
		}

		foreach( r in tempmsr )
		{
			this._selectRow(r);
		}
	}

	function _displayAllRows()
	{
		for( local r = 0; r < this.mWindowLen; r++ )
		{
			this._displayRow(this.mWindowBase + r);
		}
	}

	function insertRow( position, contents )
	{
		if (contents.len() != this.mColumnNames.len())
		{
			throw this.Exception("Invalid arguments to GUI.ColumnList.addRow()");
		}

		if (position > this.getRowCount())
		{
			position = this.getRowCount();
		}

		if (position < 0)
		{
			position = 0;
		}

		for( local i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (this.mSelectedRows[i] >= position)
			{
				this.mSelectedRows[i] += 1;
			}
		}

		this.mRowContents.insert(position, contents);
		this._displayRow(position);
	}

	function removeRow( index )
	{
		this._deselectRow(index);

		if (index < 0 || index > this.getRowCount() - 1)
		{
			throw this.Exception("Tried to remove a nonexistant index with GUI.ColumnList.removeRow()");
		}

		local selectedRows = this.mSelectedRows;

		for( local i = 0; i < this.mSelectedRows.len(); i++ )
		{
			if (selectedRows[i] > index)
			{
				selectedRows[i] -= 1;
			}
		}

		this.mRowContents.remove(index);

		if (selectedRows.len() == 0 && this.mOneSelectMin)
		{
			if (this.mRowContents.len() > index)
			{
				this.setSelectedRows([
					index
				]);
			}
			else
			{
				this.setSelectedRows([
					this.mRowContents.len() - 1
				]);
			}
		}
		else
		{
			this.setSelectedRows(selectedRows);
		}

		this._displayAllRows();
		this._recalcAllRowHeight();
	}

	function removeAllRows()
	{
		local tempmsr = [];

		foreach( r in this.mSelectedRows )
		{
			tempmsr.append(r);
		}

		foreach( r in tempmsr )
		{
			this._deselectRow(r);
		}

		this.mSelectedRows = [];
		this.mRowContents = [];
		this.mDataPane.removeAll();
		this.mWindowBase = 0;
		this._recalc();
	}

	function getRow( index )
	{
		if (index < this.mRowContents.len())
		{
			return this.mRowContents[index];
		}

		return false;
	}

	function setRow( index, contents )
	{
		if (typeof contents != "array" || contents.len() != this.mColumnNames.len())
		{
			throw this.Exception("Invalid row contents given to ColumnList.setRow(): " + contents);
		}

		if (index >= this.mRowContents.len())
		{
			this.addRow(contents);
		}
		else
		{
			this.mRowContents[index] = contents;
		}
	}

	function addRow( contents )
	{
		this.insertRow(this.getRowCount(), contents);
	}

	function isRowSelected( index )
	{
		if (this.mSelectedRows == null)
		{
			return false;
		}

		foreach( sel in this.mSelectedRows )
		{
			if (sel == index)
			{
				return true;
			}
		}

		return false;
	}

	function setRowAppearance( appearance )
	{
		this.mRowAppearance = appearance;
	}

	function setOneSelectMin( bool )
	{
		this.mOneSelectMin = bool;

		if (this.mOneSelectMin)
		{
			if (this.mSelectedRows.len() == 0 && this.mRowContents.len() > 0)
			{
				this.setSelectedRows([
					0
				]);
			}
		}
	}

	function onMouseWheel( evt )
	{
		if (this.mScroll)
		{
			while (evt.units_v > 0)
			{
				this.mScroll.onLineUp(evt);
				evt.units_v--;
			}

			while (evt.units_v < 0)
			{
				this.mScroll.onLineDown(evt);
				evt.units_v++;
			}

			evt.consume();
			this.mScroll.onActionPerformed(evt);
		}
	}

	mHeadingBar = null;
	mDataPane = null;
	mHeadingDivider = null;
	mColumnDividers = null;
	mColumnNames = null;
	mColumnWidths = null;
	mTotalColumnWidths = 1;
	mMultipleSelectionCapable = false;
	mHeadingHeight = 24;
	mForcedRowHeight = 0;
	mRowHeight = 10;
	mRowContents = null;
	mSelectedRows = null;
	mMessageBroadcaster = null;
	mSelectionInsets = null;
	mDividerWidth = 2;
	mDividerHeight = 2;
	mRowGap = 1;
	mLastClickedRow = 0;
	mRowAppearance = "ColumnList/Row";
	mWindowLen = 15;
	mWindowBase = 0;
	mOldSize = null;
	mOneSelectMin = false;
	mShowingHeaders = true;
	mScroll = null;
	static mClassName = "ColumnList";
}

