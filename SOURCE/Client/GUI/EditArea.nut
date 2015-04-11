this.require("GUI/HTMLOld");
this.require("GUI/Label");
::_KeyNamesNumpad <- {
	[this.Key.VK_NUMPAD0] = "0",
	[this.Key.VK_NUMPAD1] = "1",
	[this.Key.VK_NUMPAD2] = "2",
	[this.Key.VK_NUMPAD3] = "3",
	[this.Key.VK_NUMPAD4] = "4",
	[this.Key.VK_NUMPAD5] = "5",
	[this.Key.VK_NUMPAD6] = "6",
	[this.Key.VK_NUMPAD7] = "7",
	[this.Key.VK_NUMPAD8] = "8",
	[this.Key.VK_NUMPAD9] = "9",
	[this.Key.VK_MULTIPLY] = "*",
	[this.Key.VK_ADD] = "+",
	[this.Key.VK_SUBTRACT] = "-",
	[this.Key.VK_DECIMAL] = ".",
	[this.Key.VK_DIVIDE] = "/"
};
class this.GUI.EditArea extends this.GUI.HTMLOld
{
	constructor( ... )
	{
		this.mComponentParsedIndexList = [];
		this.mSelectionComponents = [];
		this.mKeyFocus = false;
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			this.GUI.HTMLOld.constructor(vargv[0]);
		}
		else
		{
			this.GUI.HTMLOld.constructor();
		}

		this.setInsets(2, 3, 2, 5);
		this.setAppearance("TextInputFields");
	}

	function parseHTML( source )
	{
		this.GUI.HTMLOld.parseHTML(source + " ");
	}

	function validate()
	{
		if (this.mIsValid)
		{
			return;
		}

		this.GUI.HTMLOld.validate();

		if (this.components.len() > 0)
		{
			this._removeSelection(true);
			this._populateComponentParsedIndexList();
			this._showSelection();
		}

		if (this.mCaretParsedIndex > this.mMaxParsedIndex)
		{
			this.mCaretParsedIndex = this.mMaxParsedIndex;
		}
		else if (this.mCaretParsedIndex < 0)
		{
			this.mCaretParsedIndex = 0;
		}

		if (this.mCaret)
		{
			this.add(this.mCaret);
			this._positionCaret();

			while (!this.mScrollUpdate && !this._caretVisible())
			{
				this.validate();
			}
		}

		this.mScrollUpdate = false;
		this.mIsValid = true;
	}

	function invalidate()
	{
		this.GUI.HTMLOld.invalidate();
	}

	function _addNotify()
	{
		this.GUI.HTMLOld._addNotify();
	}

	function _removeNotify()
	{
		this.GUI.HTMLOld._removeNotify();
	}

	function _calcCaretOffset()
	{
		if (this.mCaret)
		{
			local size = this.mCaret.getSize();
			local x = -size.width;
			local y = 0;
			return {
				x = x,
				y = y
			};
		}

		this.print("GUI.EditArea Error: Caret offset calculated while caret not set");
		return null;
	}

	function _positionCaret()
	{
		if (!this.mCaret)
		{
			return;
		}

		local offset = {};

		if (this.mCaretParsedIndex == 0 && this.components.len() < 2)
		{
			offset = this._calcCaretOffset();
			this.mCaret.setPosition(this.insets.left + offset.x, this.insets.top);
			local size = this.getSize();
			this.mCaret.setHeight(size.height);
			this.mCaret.setWidth(1);
		}
		else
		{
			if (this.mCaretParsedIndex > this.mMaxParsedIndex)
			{
				throw this.Exception("The caret index is greater than the max allowed.");
			}

			local ci = this.getComponentIndexByParsedIndex(this.mCaretParsedIndex);
			local c = this.components[ci];
			local rows = this._getRows();
			local ri = this.getRowIndexByParsedIndex(this.mCaretParsedIndex);
			local p = c.getPosition();
			this.mCaret.setVisible(c.isVisible());
			offset = this._calcCaretOffset();

			if (c.mClassName != "Spacer")
			{
				local sliceIndex = this.mCaretParsedIndex - this.mComponentParsedIndexList[ci].start;

				if (sliceIndex != 0)
				{
					local partial = c.getText().slice(0, sliceIndex);
					local szSlice = c.getFont().getTextMetrics(partial);
					p.x += szSlice.width;
				}
			}

			this.mCaret.setPosition(p.x + offset.x, p.y);
			local height = rows[ri.row].height;
			this.mCaret.setHeight(height);
			this.mCaret.setWidth(1);
		}
	}

	function _populateComponentParsedIndexList()
	{
		local parsedIndexArray = [];
		local i = 0;

		while (i < this.components.len())
		{
			local c = this.components[i];

			if (c.getLayoutExclude())
			{
				i++;
				continue;
			}

			if (c.mClassName == "Label" || c.mClassName == "Link")
			{
				if (i == 0)
				{
					parsedIndexArray.insert(i, {
						start = 0,
						end = c.getText().len() - 1
					});
				}
				else
				{
					local startIndex = parsedIndexArray[i - 1].end + 1;
					parsedIndexArray.insert(i, {
						start = startIndex,
						end = startIndex + c.getText().len() - 1
					});
				}
			}
			else if (c.mClassName == "Spacer")
			{
				if (i == 0)
				{
					parsedIndexArray.insert(i, {
						start = 0,
						end = 0
					});
				}
				else
				{
					local startIndex = parsedIndexArray[i - 1].end + 1;
					parsedIndexArray.insert(i, {
						start = startIndex,
						end = startIndex
					});
				}
			}

			i++;
		}

		this.mComponentParsedIndexList = parsedIndexArray;

		if (this.mComponentParsedIndexList.len() > 0)
		{
			local lastIndex = this.mComponentParsedIndexList.len() - 1;
			this.mMaxParsedIndex = this.mComponentParsedIndexList[lastIndex].end;
		}
		else
		{
			this.mMaxParsedIndex = 0;
		}
	}

	function _getRows()
	{
		return this.getLayoutManager().getRows();
	}

	function _showSelection()
	{
		if (this.mSelectionRange == null)
		{
			return;
		}

		if (this.mComponentParsedIndexList.len() == 0)
		{
			return;
		}

		for( local i = 0; i < this.mComponentParsedIndexList.len(); i++ )
		{
			local c = this.components[i];
			local cpi = this.mComponentParsedIndexList[i];

			if (c.isVisible())
			{
				if (!(this.mSelectionRange.start > cpi.end) && !(this.mSelectionRange.end < cpi.start))
				{
					local sz = c.getSize();
					local pt = c.getPosition();

					if (c.mClassName != "Spacer")
					{
						local start = this.mSelectionRange.start - cpi.start;
						local end = this.mSelectionRange.end - cpi.start + 1;

						if (start < 0)
						{
							start = 0;
						}

						if (end > c.getText().len())
						{
							end = c.getText().len();
						}

						local font = c.getFont();
						local text = c.getText().slice(0, start);
						pt.x += font.getTextMetrics(text).width;
						text = c.getText().slice(start, end);
						sz.width = font.getTextMetrics(text).width;
					}

					local sc = this.GUI.Component();
					sc.setSize(sz.width, sz.height);
					sc.setAppearance("EditArea/Selection");
					sc.setVisible(true);
					sc.setPosition(pt);
					sc.setLayoutExclude(true);
					this.add(sc);
					this.mSelectionComponents.append(sc);
				}
			}
		}
	}

	function _updateSelection( pi, pIsActive )
	{
		if (pIsActive)
		{
			if (pi >= this.mSelectionAnchor)
			{
				this.setSelectionRange(this.mSelectionAnchor, pi - 1);
				this.invalidate();
			}

			if (pi < this.mSelectionAnchor)
			{
				this.setSelectionRange(pi, this.mSelectionAnchor - 1);
				this.invalidate();
			}
		}
		else
		{
			this.mSelectionAnchor = pi;
			this.setSelectionRange(null);
			this.invalidate();
		}
	}

	function _removeSelection( ... )
	{
		local deleteMainComponents = true;

		if (vargc > 0)
		{
			deleteMainComponents = vargv[0];
		}

		if (this.mSelectionComponents.len() == 0)
		{
			return false;
		}

		while (this.mSelectionComponents.len() > 0)
		{
			if (deleteMainComponents)
			{
				this.remove(this.mSelectionComponents[0]);
			}

			this.mSelectionComponents.remove(0);
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function onScrollUpdate( evt )
	{
		this.print("Scroll Event Called");
		this.mScrollUpdate = true;
		this.invalidate();
	}

	function showCaret( pBool )
	{
		if (pBool && this.mCaret == null)
		{
			this.mCaret = this.GUI.Component(null);
			this.mCaret.setAppearance("EditArea/Caret");
			this.mCaret.setLayoutExclude(true);
			this.mCaret.setVisible(true);
			this.add(this.mCaret);
			this.invalidate();
		}
		else if (!pBool && this.mCaret)
		{
			this.remove(this.mCaret);
			this.mCaret = null;
			this.invalidate();
		}
	}

	function setSelectionRange( ... )
	{
		local srOld = this.getSelectionRange();
		local start;
		local end;

		if (vargc == 0)
		{
			return;
		}

		if (vargc == 1 && typeof vargv[0] == "table")
		{
			start = vargv[0].start;
			end = vargv[0].end;
			this.mSelectionRange = {
				start = start,
				end = end
			};

			if (srOld == null || this.mSelectionRange.start != srOld.start || this.mSelectionRange.end != srOld.end)
			{
				this.invalidate();
			}
		}
		else if (vargc == 2)
		{
			start = vargv[0].tointeger();
			end = vargv[1].tointeger();

			if (start < 0)
			{
				start = 0;
			}

			this.mSelectionRange = {
				start = start,
				end = end
			};

			if (srOld == null || this.mSelectionRange.start != srOld.start || this.mSelectionRange.end != srOld.end)
			{
				this.invalidate();
			}
		}
		else if (this.mSelectionRange != null)
		{
			this.mSelectionRange = null;
			this.invalidate();
		}
	}

	function getSelectionRange()
	{
		if (this.mSelectionRange == null)
		{
			return this.mSelectionRange;
		}

		return {
			start = this.mSelectionRange.start,
			end = this.mSelectionRange.end
		};
	}

	function setCaretIndex( pi )
	{
		local mpi = this.mMaxParsedIndex;
		local cpi = this.mCaretParsedIndex;

		if (pi > mpi)
		{
			cpi = mpi;
		}
		else if (pi < 0)
		{
			cpi = 0;
		}
		else
		{
			cpi = pi;
		}

		this.mCaretParsedIndex = cpi;
		this.mCaretUpdate = true;
		this.invalidate();
	}

	function getCaretIndex()
	{
		return this.mCaretParsedIndex;
	}

	function getSourceIndexByParsedIndex( pi )
	{
		local source = this.mText;
		local si = 0;
		local piCounter = 0;
		local tokens;

		while (pi > piCounter)
		{
			tokens = this.getToken(source, this.GUI.RegExp.Tag);

			if (!this.mShowTags && tokens)
			{
				si += tokens[0].len() + 2;
				piCounter += 1;

				if (source.len() > 1)
				{
					source = tokens[1].slice(1);
				}
				else
				{
					source = "";
				}
			}
			else
			{
				tokens = this.getToken(source, this.GUI.RegExp.NewLine);

				if (tokens)
				{
					si += tokens[0].len();
					source = tokens[0];
				}
				else
				{
					tokens = this.getToken(source, this.GUI.RegExp.Space);

					if (tokens)
					{
						si += tokens[0].len();
						piCounter += tokens[0].len();
						source = tokens[1];
					}
					else
					{
						tokens = this.getToken(source, this.GUI.RegExp.Letter);

						if (tokens)
						{
							si += tokens[0].len();
							piCounter += tokens[0].len();
							source = tokens[1];
						}
						else
						{
							tokens = this.getToken(source, this.GUI.RegExp.AnyLetter);

							if (tokens)
							{
								si += tokens[0].len();
								piCounter += tokens[0].len();
								source = tokens[1];
							}
						}
					}
				}
			}
		}

		return si;
	}

	function getComponentIndexByParsedIndex( pi )
	{
		local i = 0;
		local cpil = this.mComponentParsedIndexList;

		while (i < cpil.len())
		{
			if (pi >= cpil[i].start && pi <= cpil[i].end)
			{
				return i;
			}

			i++;
		}

		return null;
	}

	function getRowIndexByParsedIndex( pi )
	{
		local rows = this._getRows();

		if (this.mComponentParsedIndexList.len() == 0 || rows == null)
		{
			return {
				row = 0,
				index = 0
			};
		}

		if (pi > this.mMaxParsedIndex)
		{
			pi = this.mMaxParsedIndex;
		}

		local ci = this.getComponentIndexByParsedIndex(pi);
		local c = this.components[ci];
		local ri = this.getLayoutManager().getRowIndexByComponent(c);
		local r = rows[ri];
		local cStart = r.rowComp[0];
		local ciStart = this.getComponentIndexByComponent(cStart);
		local piStart = this.mComponentParsedIndexList[ciStart].start;
		local rpi = pi - piStart;
		return {
			row = ri,
			index = rpi
		};
	}

	function getParsedIndexByRowIndex( ri )
	{
		local rows = this._getRows();
		local row = rows[ri.row];
		local c = row.rowComp[0];
		local ci = this.getComponentIndexByComponent(c);
		local piStart = this.mComponentParsedIndexList[ci].start;
		return piStart + ri.index;
	}

	function getComponentIndexByComponent( component )
	{
		if (this.components.len() == 0)
		{
			return null;
		}

		foreach( i, c in this.components )
		{
			if (c == component)
			{
				return i;
			}
		}

		return null;
	}

	function getLocalPositionByParsedIndex( pi )
	{
		if (pi > this.mMaxParsedIndex)
		{
			pi = this.mMaxParsedIndex;
		}

		local ci = this.getComponentIndexByParsedIndex(pi);
		local c = this.components[ci];
		local rows = this._getRows();
		local piStart = this.mComponentParsedIndexList[ci].start;
		local ri = this.getRowIndexByParsedIndex(pi).row;
		local x = this.insets.top;
		local y = 0;

		for( local i = 0; i <= ri; i++ )
		{
			y += rows[i].height;
		}

		y -= rows[ri].height / 2;
		x = c.getPosition().x;
		local li = 0;

		if (c.mClassName != "Spacer")
		{
			if (c.getText().len() != 0)
			{
				li = pi - piStart + 1;
				local partial = c.getText().slice(0, li);
				local font = c.getFont();
				local sz = font.getTextMetrics(partial);
				x += sz.width;

				if (partial.len() > 0)
				{
					partial = partial.slice(partial.len() - 1);
					sz = font.getTextMetrics(partial);
					x -= sz.width / 2;
				}
			}
		}
		else
		{
			x += c.getSize().width / 2;
		}

		return {
			x = x,
			y = y
		};
	}

	function getParsedIndexByLocalPosition( p )
	{
		local rows = this._getRows();

		if (rows.len() == 0)
		{
			return null;
		}

		local height = this.insets.top;
		local ri = 0;

		if (this.mScroll)
		{
			ri = this.mScroll.getIndex();
		}

		while (ri < rows.len())
		{
			height += rows[ri].height;

			if (height > p.y)
			{
				break;
			}

			if (ri == rows.len() - 1)
			{
				break;
			}

			ri++;
		}

		local rci = 0;
		local rc = rows[ri].rowComp;
		local width = this.insets.left;

		while (rci < rc.len())
		{
			if (rc[rci].isVisible())
			{
				width += rc[rci].getWidth();
			}

			if (width > p.x)
			{
				break;
			}

			if (rci == rc.len() - 1 && rows.len() - 1 != ri)
			{
				break;
			}

			rci++;
		}

		if (rows.len() - 1 == ri && rci == rc.len())
		{
			return this.mMaxParsedIndex;
		}

		local c = rc[rci];
		local ci = this.getComponentIndexByComponent(c);
		local ciStart = this.mComponentParsedIndexList[ci].start;

		if (c.mClassName == "Spacer")
		{
			return ciStart;
		}

		local cw = p.x - (width - c.getWidth());
		local li = 0;
		local pi;
		local text = c.getText();
		local partial = "";
		local font = c.getFont();

		while (li < text.len())
		{
			partial = text.slice(0, li);
			local pcw = font.getTextMetrics(partial).width;

			if (pcw > cw)
			{
				break;
			}

			if (li == text.len())
			{
				break;
			}

			li++;
		}

		return ciStart + li - 1;
	}

	function getParsedIndexByScreenPosition( p )
	{
		local sp = this.getScreenPosition();
		p.x = p.x - sp.x;
		p.y = p.y - sp.y;
		return this.getParsedIndexByLocalPosition(p);
	}

	function getLastRowIndexByRowIndex( ri )
	{
		local rows = this._getRows();
		local rc = rows[ri.row].rowComp;
		local c = rc[rc.len() - 1];
		local ci = this.getComponentIndexByComponent(c);
		local piEnd = this.mComponentParsedIndexList[ci].end;
		return this.getRowIndexByParsedIndex(piEnd);
	}

	function endCaretRow( ... )
	{
		local selection = false;

		if (vargc > 0)
		{
			selection = vargv[0];
		}

		local pi = this.getCaretIndex();
		local ri = this.getRowIndexByParsedIndex(pi);
		local rows = this._getRows();
		local index = ri.index;
		local riEnd = this.getLastRowIndexByRowIndex(ri);
		local piNew = this.getParsedIndexByRowIndex({
			row = riEnd.row,
			index = riEnd.index
		});
		this.setCaretIndex(piNew);
		this._updateSelection(piNew, selection);
		this.invalidate();
	}

	function startCaretRow( ... )
	{
		local selection = false;

		if (vargc > 0)
		{
			selection = vargv[0];
		}

		local pi = this.getCaretIndex();
		local ri = this.getRowIndexByParsedIndex(pi);
		local piNew = this.getParsedIndexByRowIndex({
			row = ri.row,
			index = 0
		});
		this.setCaretIndex(piNew);
		this._updateSelection(piNew, selection);
		this.invalidate();
	}

	function downCaretRow( ... )
	{
		local selection = false;

		if (vargc > 0)
		{
			selection = vargv[0];
		}

		local pi = this.getCaretIndex();
		local ri = this.getRowIndexByParsedIndex(pi);
		local rows = this._getRows();
		local index = ri.index;

		if (rows.len() > ri.row + 1)
		{
			ri.row += 1;
			local riEnd = this.getLastRowIndexByRowIndex(ri);

			if (riEnd.index < index)
			{
				index = riEnd.index;
			}

			local piNew = this.getParsedIndexByRowIndex({
				row = ri.row,
				index = index
			});
			this.setCaretIndex(piNew);
			this._updateSelection(piNew, selection);
		}

		this.invalidate();
	}

	function upCaretRow( ... )
	{
		local selection = false;

		if (vargc > 0)
		{
			selection = vargv[0];
		}

		local pi = this.getCaretIndex();
		local ri = this.getRowIndexByParsedIndex(pi);
		local rows = this._getRows();
		local index = ri.index;

		if (0 <= ri.row - 1)
		{
			ri.row -= 1;
			local riEnd = this.getLastRowIndexByRowIndex(ri);

			if (riEnd.index < index)
			{
				index = riEnd.index;
			}

			local piNew = this.getParsedIndexByRowIndex({
				row = ri.row,
				index = index
			});
			this.setCaretIndex(piNew);
			this._updateSelection(piNew, selection);
		}

		this.invalidate();
	}

	function onStartInput( ... )
	{
		this.GUI._Manager.requestKeyboardFocus(this);
		this.showCaret(true);
		this.invalidate();
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.GUI._Manager.requestKeyboardFocus(this);
			local p = {
				x = evt.x,
				y = evt.y
			};
			local pi = this.getParsedIndexByLocalPosition(p);
			this.setCaretIndex(pi);
			this._updateSelection(pi, this.Key.isDown(this.Key.VK_SHIFT));
			this.mPressedLeft = true;
			this.invalidate();
		}

		evt.consume();
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mPressedLeft = false;
		}

		evt.consume();
	}

	function onMouseMoved( evt )
	{
		if (this.mPressedLeft)
		{
			local p = {
				x = evt.x,
				y = evt.y
			};
			local pi = this.getParsedIndexByLocalPosition(p);
			this.setCaretIndex(pi);
			this._updateSelection(pi, true);
		}

		evt.consume();
	}

	function setText( text )
	{
		if (text != this.mText)
		{
			this.GUI.HTMLOld.setText(text);
			this.mIsValid = false;
			this.validate();
			this.setCaretIndex(this.mMaxParsedIndex);
			this._fireActionPerformed("onTextChanged");
		}
	}

	function addText( text )
	{
		if (this.mSelectionRange)
		{
			this.deleteSelection();
		}

		local pi = this.getCaretIndex();
		local si = this.getSourceIndexByParsedIndex(pi);
		local oldMax = this.mMaxParsedIndex;
		local startText = this.mText.slice(0, si);
		local endText = this.mText.slice(si);
		local finalText = startText + text + endText;
		this.GUI.HTMLOld.setText(finalText);
		this.mIsValid = false;
		this.validate();
		local newMax = this.mMaxParsedIndex;
		this.setCaretIndex(pi + (newMax - oldMax));
		this.mSelectionAnchor = this.mCaretParsedIndex;
		this.invalidate();
		this._fireActionPerformed("onTextChanged");
	}

	function getSelectionText()
	{
		return this.mText.slice(this.mSelectionRange.start, this.mSelectionRange.end + 1);
	}

	function replaceSelection( newText )
	{
		local srStart = this.mSelectionRange.start;
		local srEnd = this.mSelectionRange.end;
		local sis = this.getSourceIndexByParsedIndex(srStart);
		local sie = this.getSourceIndexByParsedIndex(srEnd + 1);
		local oldMax = this.mMaxParsedIndex;
		local startText = this.mText.slice(0, sis);
		local endText = "";

		if (sie <= oldMax)
		{
			endText = this.mText.slice(sie);
		}

		local finalText = startText + newText + endText;
		this.GUI.HTMLOld.setText(finalText);
		this.mIsValid = false;
		this.validate();

		if (newText.len() == 0)
		{
			this.setSelectionRange(null);
		}
		else
		{
			srEnd = this.mSelectionRange.start + newText.len();
			this.setSelectionRange(srStart, srEnd);
		}

		local newMax = this.mMaxParsedIndex;
		this.setCaretIndex(srEnd + 1 + (newMax - oldMax));
		this.mSelectionAnchor = this.mCaretParsedIndex;
		this._fireActionPerformed("onTextChanged");
	}

	function deleteSelection()
	{
		this.replaceSelection("");
		this._fireActionPerformed("onTextChanged");
	}

	function deleteCaretIndex()
	{
		if (this.mSelectionRange)
		{
			this.deleteSelection();
			return;
		}

		local pi = this.getCaretIndex();

		if (pi >= 0)
		{
			if (pi == this.mMaxParsedIndex)
			{
				return;
			}

			local si = this.getSourceIndexByParsedIndex(pi + 1);
			local sib = this.getSourceIndexByParsedIndex(pi);
			local oldMax = this.mMaxParsedIndex;
			local startText = this.mText.slice(0, sib);
			local endText = "";

			if (si <= oldMax)
			{
				endText = this.mText.slice(si);
			}

			local finalText = startText + endText;
			this.GUI.HTMLOld.setText(finalText);
			this.mIsValid = false;
			this.validate();
			local newMax = this.mMaxParsedIndex;
			this.setCaretIndex(pi + 1 + (newMax - oldMax));
			this.mSelectionAnchor = this.mCaretParsedIndex;
			this.invalidate();
		}
	}

	function backspace()
	{
		if (this.mSelectionRange)
		{
			this.deleteSelection();
			return;
		}

		local pi = this.getCaretIndex();

		if (pi > 0)
		{
			local si = this.getSourceIndexByParsedIndex(pi);
			local sib = this.getSourceIndexByParsedIndex(pi - 1);
			local oldMax = this.mMaxParsedIndex;
			local startText = this.mText.slice(0, sib);
			local endText = this.mText.slice(si);
			local finalText = startText + endText;
			this.GUI.HTMLOld.setText(finalText);
			this.mIsValid = false;
			this.validate();
			local newMax = this.mMaxParsedIndex;
			this.setCaretIndex(pi + (newMax - oldMax));
			this.mSelectionAnchor = this.mCaretParsedIndex;
			this.invalidate();
			this._fireActionPerformed("onTextChanged");
		}
	}

	function _caretVisible()
	{
		if (!this.mCaret)
		{
			return true;
		}

		if (!this.mScroll)
		{
			return true;
		}

		this.print("GUI.EditArea._caretVisible Called");
		local rows = this._getRows();
		local ri = this.getRowIndexByParsedIndex(this.mCaretParsedIndex);
		this.print(ri.row + " is the current caret row.");

		if (rows[ri.row].clip)
		{
			local scrollRow = this.mScroll.getIndex();

			if (scrollRow < ri.row)
			{
				this.mScroll.setIndex(scrollRow + 1);
				return false;
			}
			else if (scrollRow > 0)
			{
				this.mScroll.setIndex(scrollRow - 1);
				return false;
			}
		}

		return true;
	}

	function setTabOrderTarget( target )
	{
		this.mTabOrderTarget = target;
	}

	function getTabOrderTarget()
	{
		return this.mTabOrderTarget;
	}

	function onRequestedKeyboardFocus()
	{
		if (this.mWidget != null && !this.mKeyFocus)
		{
			this.mWidget.requestKeyboardFocus();
			this.showCaret(true);
		}

		this.mKeyFocus = true;
	}

	function onReleasedKeyboardFocus()
	{
		if (this.mWidget != null && this.mKeyFocus)
		{
			this.showCaret(false);
			this.setCaretIndex(this.mMaxParsedIndex);
			this._updateSelection(this.getCaretIndex(), false);
		}

		this.mKeyFocus = false;
	}

	function onRealKeyReleased( evt )
	{
		this._fireActionPerformed("onKeyPressed");

		if (!(::Key.isDown(this.Key.VK_CONTROL) && evt.key == " "))
		{
			this.addText(evt.key);
		}

		evt.consume();
	}

	function onKeyPressed( evt )
	{
		this._fireActionPerformed("onKeyPressed");

		switch(evt.keyCode)
		{
		case ::Key.VK_DOWN:
			this.downCaretRow(this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case ::Key.VK_UP:
			this.upCaretRow(this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case ::Key.VK_LEFT:
			this.setCaretIndex(this.getCaretIndex() - 1);
			this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case ::Key.VK_RIGHT:
			this.setCaretIndex(this.getCaretIndex() + 1);
			this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case ::Key.VK_SHIFT:
			evt.consume();
			break;

		case ::Key.VK_CONTROL:
			evt.consume();
			break;

		case ::Key.VK_ALT:
			evt.consume();
			break;

		case 9:
			if (this.mTabOrderTarget)
			{
				this.GUI._Manager.requestKeyboardFocus(this.mTabOrderTarget);
				this.mTabOrderTarget._caretVisible();
			}

			evt.consume();
			break;

		case 20:
			evt.consume();
			break;

		case 27:
			this.GUI._Manager.releaseKeyboardFocus(this);
			this._fireActionPerformed("onInputCancelled");
			evt.consume();
			break;

		case 33:
			evt.consume();
			break;

		case 34:
			evt.consume();
			break;

		case 35:
			this.endCaretRow(this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case 36:
			this.startCaretRow(this.Key.isDown(this.Key.VK_SHIFT));
			evt.consume();
			break;

		case 45:
			if (this.Key.isDown(this.Key.VK_SHIFT))
			{
				this.addText(this.System.getClipboard());
				evt.consume();
			}

			break;

		case 46:
			if (this.Key.isDown(this.Key.VK_SHIFT))
			{
				this.System.setClipboard(this.getSelectionText());
			}

			this.deleteCaretIndex();
			evt.consume();
			break;

		case ::Key.VK_ENTER:
			if (!this.mShowTags)
			{
				this.addText("<BR>");
			}
			else
			{
				this.addText("\n");
			}

			evt.consume();
			break;

		case ::Key.VK_BACK:
			this.backspace();
			evt.consume();
			break;

		case this.Key.VK_SPACE:
			if (this.mContentAssistEnabled && this.Key.isDown(this.Key.VK_CONTROL))
			{
				this._fireActionPerformed("onContentAssist");
			}

			evt.consume();
			break;

		case 112:
			evt.consume();
			break;

		case 113:
			evt.consume();
			break;

		case 114:
			evt.consume();
			break;

		case 115:
			evt.consume();
			break;

		case 116:
			evt.consume();
			break;

		case 117:
			evt.consume();
			break;

		case 118:
			evt.consume();
			break;

		case 119:
			evt.consume();
			break;

		case 120:
			evt.consume();
			break;

		case 121:
			evt.consume();
			break;

		case 122:
			evt.consume();
			break;

		case 123:
			evt.consume();
			break;

		default:
			if (evt.isControlDown())
			{
				if (evt.keyCode == this.Key.VK_X)
				{
					this.System.setClipboard(this.getSelectionText());
					this.replaceSelection("");
				}
				else if (evt.keyCode == this.Key.VK_C)
				{
					this.System.setClipboard(this.getSelectionText());
				}
				else if (evt.keyCode == this.Key.VK_V)
				{
					this.addText(this.System.getClipboard());
				}
			}

			evt.consume();
			break;
		}
	}

	function setContentAssistEnabled( value )
	{
		this.mContentAssistEnabled = value;
	}

	function isContentAssistEnabled()
	{
		return this.mContentAssistEnabled;
	}

	mLoopCount = 0;
	mComponentParsedIndexList = null;
	mMaxParsedIndex = 0;
	mDeleteInProgress = false;
	mSelectionAnchor = null;
	mSelectionRange = null;
	mSelectionComponents = null;
	mSelectionChanged = false;
	mScrollUpdate = false;
	mHTMLUpdate = false;
	mCaretUpdate = false;
	mCaret = null;
	mCaretParsedIndex = 0;
	mKeyFocus = false;
	mContentAssistEnabled = false;
	mTabOrderTarget = null;
	static mClassName = "EditArea";
}

