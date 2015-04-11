this.require("GUI/Container");
this.require("GUI/Label");
::_KeyNames <- {
	[this.Key.VK_A] = "a",
	[this.Key.VK_B] = "b",
	[this.Key.VK_C] = "c",
	[this.Key.VK_D] = "d",
	[this.Key.VK_E] = "e",
	[this.Key.VK_F] = "f",
	[this.Key.VK_G] = "g",
	[this.Key.VK_H] = "h",
	[this.Key.VK_I] = "i",
	[this.Key.VK_J] = "j",
	[this.Key.VK_K] = "k",
	[this.Key.VK_L] = "l",
	[this.Key.VK_M] = "m",
	[this.Key.VK_N] = "n",
	[this.Key.VK_O] = "o",
	[this.Key.VK_P] = "p",
	[this.Key.VK_Q] = "q",
	[this.Key.VK_R] = "r",
	[this.Key.VK_S] = "s",
	[this.Key.VK_T] = "t",
	[this.Key.VK_U] = "u",
	[this.Key.VK_V] = "v",
	[this.Key.VK_W] = "w",
	[this.Key.VK_X] = "x",
	[this.Key.VK_Y] = "y",
	[this.Key.VK_Z] = "z",
	[this.Key.VK_1] = "1",
	[this.Key.VK_2] = "2",
	[this.Key.VK_3] = "3",
	[this.Key.VK_4] = "4",
	[this.Key.VK_5] = "5",
	[this.Key.VK_6] = "6",
	[this.Key.VK_7] = "7",
	[this.Key.VK_8] = "8",
	[this.Key.VK_9] = "9",
	[this.Key.VK_0] = "0",
	[this.Key.VK_ENTER] = "\n",
	[186] = ";",
	[187] = "=",
	[188] = ",",
	[189] = "-",
	[190] = ".",
	[191] = "/",
	[192] = "`",
	[219] = "[",
	[220] = "\\",
	[221] = "]",
	[222] = "\'",
	[this.Key.VK_SPACE] = " ",
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
::_KeyNames_Caps <- {
	[this.Key.VK_A] = "A",
	[this.Key.VK_B] = "B",
	[this.Key.VK_C] = "C",
	[this.Key.VK_D] = "D",
	[this.Key.VK_E] = "E",
	[this.Key.VK_F] = "F",
	[this.Key.VK_G] = "G",
	[this.Key.VK_H] = "H",
	[this.Key.VK_I] = "I",
	[this.Key.VK_J] = "J",
	[this.Key.VK_K] = "K",
	[this.Key.VK_L] = "L",
	[this.Key.VK_M] = "M",
	[this.Key.VK_N] = "N",
	[this.Key.VK_O] = "O",
	[this.Key.VK_P] = "P",
	[this.Key.VK_Q] = "Q",
	[this.Key.VK_R] = "R",
	[this.Key.VK_S] = "S",
	[this.Key.VK_T] = "T",
	[this.Key.VK_U] = "U",
	[this.Key.VK_V] = "V",
	[this.Key.VK_W] = "W",
	[this.Key.VK_X] = "X",
	[this.Key.VK_Y] = "Y",
	[this.Key.VK_Z] = "Z",
	[this.Key.VK_1] = "1",
	[this.Key.VK_2] = "2",
	[this.Key.VK_3] = "3",
	[this.Key.VK_4] = "4",
	[this.Key.VK_5] = "5",
	[this.Key.VK_6] = "6",
	[this.Key.VK_7] = "7",
	[this.Key.VK_8] = "8",
	[this.Key.VK_9] = "9",
	[this.Key.VK_0] = "0",
	[this.Key.VK_ENTER] = "\n",
	[186] = ";",
	[187] = "=",
	[188] = ",",
	[189] = "-",
	[190] = ".",
	[191] = "/",
	[192] = "`",
	[219] = "[",
	[220] = "\\",
	[221] = "]",
	[222] = "\'",
	[this.Key.VK_SPACE] = " "
};
::_KeyNames_Caps_Shift <- {
	[this.Key.VK_A] = "a",
	[this.Key.VK_B] = "b",
	[this.Key.VK_C] = "c",
	[this.Key.VK_D] = "d",
	[this.Key.VK_E] = "e",
	[this.Key.VK_F] = "f",
	[this.Key.VK_G] = "g",
	[this.Key.VK_H] = "h",
	[this.Key.VK_I] = "i",
	[this.Key.VK_J] = "j",
	[this.Key.VK_K] = "k",
	[this.Key.VK_L] = "l",
	[this.Key.VK_M] = "m",
	[this.Key.VK_N] = "n",
	[this.Key.VK_O] = "o",
	[this.Key.VK_P] = "p",
	[this.Key.VK_Q] = "q",
	[this.Key.VK_R] = "r",
	[this.Key.VK_S] = "s",
	[this.Key.VK_T] = "t",
	[this.Key.VK_U] = "u",
	[this.Key.VK_V] = "v",
	[this.Key.VK_W] = "w",
	[this.Key.VK_X] = "x",
	[this.Key.VK_Y] = "y",
	[this.Key.VK_Z] = "z",
	[this.Key.VK_1] = "!",
	[this.Key.VK_2] = "@",
	[this.Key.VK_3] = "#",
	[this.Key.VK_4] = "$",
	[this.Key.VK_5] = "%",
	[this.Key.VK_6] = "^",
	[this.Key.VK_7] = "&",
	[this.Key.VK_8] = "*",
	[this.Key.VK_9] = "(",
	[this.Key.VK_0] = ")",
	[this.Key.VK_ENTER] = "\n",
	[186] = ":",
	[187] = "+",
	[188] = "<",
	[189] = "_",
	[190] = ">",
	[191] = "?",
	[192] = "~",
	[219] = "{",
	[220] = "|",
	[221] = "}",
	[222] = "\"",
	[this.Key.VK_SPACE] = " "
};
::_KeyNames_Shift <- {
	[this.Key.VK_A] = "A",
	[this.Key.VK_B] = "B",
	[this.Key.VK_C] = "C",
	[this.Key.VK_D] = "D",
	[this.Key.VK_E] = "E",
	[this.Key.VK_F] = "F",
	[this.Key.VK_G] = "G",
	[this.Key.VK_H] = "H",
	[this.Key.VK_I] = "I",
	[this.Key.VK_J] = "J",
	[this.Key.VK_K] = "K",
	[this.Key.VK_L] = "L",
	[this.Key.VK_M] = "M",
	[this.Key.VK_N] = "N",
	[this.Key.VK_O] = "O",
	[this.Key.VK_P] = "P",
	[this.Key.VK_Q] = "Q",
	[this.Key.VK_R] = "R",
	[this.Key.VK_S] = "S",
	[this.Key.VK_T] = "T",
	[this.Key.VK_U] = "U",
	[this.Key.VK_V] = "V",
	[this.Key.VK_W] = "W",
	[this.Key.VK_X] = "X",
	[this.Key.VK_Y] = "Y",
	[this.Key.VK_Z] = "Z",
	[this.Key.VK_1] = "!",
	[this.Key.VK_2] = "@",
	[this.Key.VK_3] = "#",
	[this.Key.VK_4] = "$",
	[this.Key.VK_5] = "%",
	[this.Key.VK_6] = "^",
	[this.Key.VK_7] = "&",
	[this.Key.VK_8] = "*",
	[this.Key.VK_9] = "(",
	[this.Key.VK_0] = ")",
	[this.Key.VK_ENTER] = "\n",
	[186] = ":",
	[187] = "+",
	[188] = "<",
	[189] = "_",
	[190] = ">",
	[191] = "?",
	[192] = "~",
	[219] = "{",
	[220] = "|",
	[221] = "}",
	[222] = "\"",
	[this.Key.VK_SPACE] = " "
};
class this.GUI.InputArea extends this.GUI.Container
{
	mCursor = null;
	mTabOrderTarget = null;
	mPassword = false;
	mStoredIndex = 0;
	mStoredInputs = null;
	mStartIndex = 0;
	mKeyFocus = false;
	mMousePressed = false;
	mCursorStart = 0;
	mCursorEnd = 0;
	mMultiLine = false;
	mScrollTimer = 0.0;
	mLocked = false;
	mMaxCharacters = -1;
	mNumbersOnly = false;
	mLettersOnly = false;
	mLabelXOffset = 0;
	mCenterText = false;
	mLineOffset = 0;
	mMaxLinesSeen = 1;
	mMouseMove = false;
	mRows = [];
	mText = "";
	mAutoCapitalize = false;
	mAllowSpaces = true;
	mMousePressedMessage = null;
	static mClassName = "InputArea";
	mAllowInputInClick = true;
	mIgnoreInitialCharacters = null;
	constructor( ... )
	{
		this.mIgnoreInitialCharacters = {};
		this.GUI.Container.constructor(null);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mStoredInputs = [];

		if (vargc > 0)
		{
			this.setText(vargv[0]);
		}

		this.setInsets(2, 3, 2, 5);
		this.setAppearance("TextInputFields");
		local height = this.getFont().height;
		this.mMaxLinesSeen = (this.mHeight - this.insets.top) / height;
		this._enterFrameRelay.addListener(this);
	}

	function addIgnoredCharacter( character )
	{
		this.mIgnoreInitialCharacters[character] <- true;
	}

	function resetIgnoredCharacters()
	{
		this.mIgnoreInitialCharacters = {};
	}

	function destroy()
	{
		this._enterFrameRelay.removeListener(this);
		this.GUI.Container.destroy();
	}

	function getPreferredSize()
	{
		local sz = this.getSize();

		if (this.mLayoutManager)
		{
			sz = this.mLayoutManager.preferredLayoutSize(this);
		}

		local minSz = this.getMinimumSize();

		if (minSz.width > sz.width)
		{
			sz.width = minSz.width;
		}

		if (minSz.height > sz.height)
		{
			sz.height = minSz.height;
		}

		return sz;
	}

	function pickRow( x, y )
	{
		if (this.mRows.len() > 0 && y < this.insets.top)
		{
			if (this.mLineOffset < this.mRows.len())
			{
				return this.mRows[this.mLineOffset];
			}
			else
			{
				return this.mRows[0];
			}
		}

		foreach( index, n in this.mRows )
		{
			local ny = n.y + this.insets.top;

			if (y >= ny && y <= ny + n.height)
			{
				local indexInto = this.mLineOffset + index;

				if (indexInto < this.mRows.len())
				{
					return this.mRows[indexInto];
				}
			}
		}

		if (this.mRows.len() > 0)
		{
			return this.mRows.top();
		}

		return null;
	}

	function pickRowFromCursor( loc )
	{
		local index = this.getRowIndexFromCursor(loc);
		return index != null ? this.mRows[index] : null;
	}

	function getRowIndexFromCursor( loc )
	{
		for( local x = 0; x < this.mRows.len(); x++ )
		{
			local row = this.mRows[x];

			if (loc >= row.start && loc <= row.end)
			{
				if (loc == row.end && x < this.mRows.len() - 1 && this.mRows[x + 1].text[0] != 10)
				{
					return x + 1;
				}

				return x;
			}
		}

		return null;
	}

	function getSelectionRange()
	{
		if (this.mCursorStart < this.mCursorEnd)
		{
			return [
				this.mCursorStart,
				this.mCursorEnd
			];
		}

		return [
			this.mCursorEnd,
			this.mCursorStart
		];
	}

	function onEnterFrame()
	{
		if (this.mMultiLine == false && this.mKeyFocus && this.mMousePressed)
		{
			this.mScrollTimer += this._deltat;

			if (this.mScrollTimer > 15)
			{
				local d = this.getCursorInRange();

				if (d != 0)
				{
					this.mStartIndex += d;

					if (this.mStartIndex < 0)
					{
						this.mStartIndex = 0;
					}

					this.buildRows();
					this.updateCursorPosition(false);
				}

				this.mScrollTimer = 0;
			}
		}
	}

	function addRow( text, start, end, y )
	{
		local label = this.GUI.Label(text);
		local font = label.getFont();
		local xOffset = 0.0;

		if (this.mCenterText == true)
		{
			local size = this.getPreferredSize();
			local length = label.getPreferredSize();
			xOffset = (size.width - this.insets.right) / 2.0 - length.width / 2.0 + this.insets.left / 2.0;
			this.mLabelXOffset = xOffset;
		}
		else
		{
			xOffset = this.insets.left;
		}

		label.setPosition(xOffset, y + this.insets.top);
		label.setFontColor(this.getFontColor().toHexString());
		label.setHeight(font.getHeight());
		label.setTextAlignment(0.0, 0.0);
		label.setIgnoreNewLines(true);
		this.add(label);
		local height = label.getFont().getHeight();
		local selection;

		if (this.mCursorStart != this.mCursorEnd)
		{
			local range = this.getSelectionRange();

			if (this.mMultiLine == false || range[0] <= end && range[1] >= start)
			{
				local sstart = range[0] < start ? start : range[0];
				local send = range[1] > end ? end : range[1];
				sstart -= start;
				send -= start;
				local sbstart = this.getFont().getTextMetrics(text.slice(0, sstart)).width + this.insets.left;
				local sbwidth = this.getFont().getTextMetrics(text.slice(sstart, send)).width;
				local sbheight = this.getFont().getHeight();

				if (this.mCenterText == true)
				{
					sbstart += this.mLabelXOffset - this.insets.left;
				}

				selection = this.GUI.Component();
				selection.setAppearance("EditArea/Selection");
				selection.setSize(sbwidth, sbheight);
				selection.setPosition(sbstart, y + this.insets.top);
				selection.setLayoutExclude(true);
				selection.setVisible(true);
				this.add(selection);
			}
		}

		local rowObj = {
			text = text,
			start = start,
			end = end,
			label = label,
			height = height,
			y = y,
			selection = selection
		};
		this.mRows.append(rowObj);
		return rowObj;
	}

	function setMousePressMessage( message )
	{
		this.mMousePressedMessage = message;
	}

	function convertPasswordText( text )
	{
		if (this.mPassword == true)
		{
			local tmp = "";

			for( local i = 0; i < text.len(); i++ )
			{
				tmp += "*";
			}

			return tmp;
		}

		return text;
	}

	function getVisibleText( text )
	{
		local size = this.getSize();
		local lines = [];

		foreach( l in text )
		{
			local txt = "";

			for( local i = this.mStartIndex; i < l.len(); i++ )
			{
				local line = l.slice(this.mStartIndex, i + 1) + this.insets.left;
				local c = l.slice(i, i + 1);
				local w = this.getFont().getTextMetrics(line).width;

				if (w > size.width)
				{
					break;
				}

				txt += c;
			}

			lines.append(txt);
		}

		return lines;
	}

	function setAutoCapitalize( value )
	{
		this.mAutoCapitalize = value;
	}

	function setAllowSpaces( value )
	{
		this.mAllowSpaces = value;
	}

	function buildRows()
	{
		local tmp = "";
		local height = this.getFont().height;
		local start = 0;
		local end = 0;
		local y = 0;
		local font = this.getFont();
		local lines = this.mMultiLine == true ? this.Screen.wordWrapList(this.convertPasswordText(this.mText), font.getFullFace(), this.getWidth() - this.insets.left - this.insets.right, font.getHeight()).text : [
			this.convertPasswordText(this.mText)
		];
		local text = this.mMultiLine == true ? lines : this.getVisibleText(lines);
		this.removeAll();

		foreach( r in this.mRows )
		{
			if (r.selection)
			{
				r.selection.destroy();
			}

			r.label.destroy();
		}

		this.mRows = [];
		this.mMaxLinesSeen = (this.mHeight - this.insets.top) / height;
		local startTextIndex = this.mLineOffset;
		local endTextIndex = startTextIndex + this.mMaxLinesSeen - 1;

		foreach( index, l in text )
		{
			end += l.len();
			local rowObj = this.addRow(l, this.mStartIndex + start, this.mStartIndex + end, y);

			if (startTextIndex > 0)
			{
				local oldPosition = rowObj.label.getPosition();
				local newYPosition = y - height * startTextIndex;
				rowObj.label.setPosition(oldPosition.x, newYPosition);

				if (rowObj.selection)
				{
					rowObj.selection.setPosition(rowObj.selection.getPosition().x, newYPosition);
				}
			}

			if (index < startTextIndex || index > endTextIndex)
			{
				rowObj.label.setVisible(false);

				if (rowObj.selection)
				{
					rowObj.selection.setVisible(false);
				}
			}

			y += height;
			start = end;
		}

		if (this.mCursor)
		{
			this.add(this.mCursor);
		}
	}

	function getMinimumSize()
	{
		return this._addInsets(this.getFont().getTextMetrics("   "));
	}

	function setPassword( pBool )
	{
		this.mPassword = pBool;
		this.buildRows();
	}

	function getPassword( pBool )
	{
		return this.mPassword;
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.setChildProcessingEvents(false);
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Container._removeNotify();
	}

	function _reshapeNotify()
	{
		this.GUI.Container._reshapeNotify();
		this.buildRows();
	}

	function onStartInput( ... )
	{
		this.GUI._Manager.requestKeyboardFocus(this);
		this.invalidate();
	}

	function moveCursor( amount, ... )
	{
		this.setCursorPosition(this.getCursorPosition() + amount, vargc > 0 ? ::Key.isDown(this.Key.VK_SHIFT) : false);
	}

	function moveCursorY( val, ... )
	{
		if (this.mCursor)
		{
			local rowIndex = this.getRowIndexFromCursor(this.mCursorEnd);

			if (rowIndex == null)
			{
				return;
			}

			local desiredRow = rowIndex + val;

			if (desiredRow >= 0 && desiredRow < this.mRows.len())
			{
				local offset = this.mCursorEnd - this.mRows[rowIndex].start;
				local desiredRowObj = this.mRows[desiredRow];
				local desiredOffset = desiredRowObj.start + (desiredRowObj.text[0] == 10 ? offset + 1 : offset);

				if (desiredRowObj.end < desiredOffset)
				{
					desiredOffset = desiredRowObj.end;
				}

				if (this.mMultiLine)
				{
					if (desiredRow - this.mLineOffset >= this.mMaxLinesSeen)
					{
						this.mLineOffset = this.mLineOffset + 1;
						this.buildRows();
					}
					else if (desiredRow < this.mLineOffset)
					{
						this.mLineOffset = this.mLineOffset - 1;
						this.buildRows();
					}
				}

				this.setCursorPosition(desiredOffset, vargc > 0 ? ::Key.isDown(this.Key.VK_SHIFT) : false);
			}
		}
	}

	function setCursorPosition( pos, ... )
	{
		local extend = vargc > 0 ? vargv[0] : false;

		if (pos < 0)
		{
			pos = 0;
		}

		if (pos > this.mText.len())
		{
			pos = this.mText.len();
		}

		if (extend == false)
		{
			this.clearSelection();
			this.mCursorStart = pos;
			this.mCursorEnd = pos;
		}
		else
		{
			this.mCursorEnd = pos;
			this.buildRows();
		}

		this.updateCursorPosition();
	}

	function getCursorPosition()
	{
		return this.mCursorEnd;
	}

	function getCursorVisible()
	{
		if (this.mCursor == null)
		{
			return true;
		}

		local cursorLoc = this.getPointFromCursorLocation(this.mCursorEnd);

		if (cursorLoc.x + this.mCursor.getWidth() >= this.getWidth())
		{
			return false;
		}

		return true;
	}

	function getCursorInRange()
	{
		if (this.mCursor == null)
		{
			return 0;
		}

		local paddingLeft = 5;
		local rangeStart = this.getFont().getTextMetrics(this.mText.slice(0, this.mStartIndex)).width + this.insets.left + this.mCursor.getWidth();
		local rangeEnd = rangeStart + this.getWidth() - this.insets.right - this.mCursor.getWidth() - paddingLeft;
		local cursorPos = this.getFont().getTextMetrics(this.mText.slice(0, this.mCursorEnd + 1)).width + this.insets.left;

		if (cursorPos < rangeStart)
		{
			return -1;
		}

		if (cursorPos + this.mCursor.getWidth() >= rangeEnd)
		{
			return 1;
		}

		return 0;
	}

	function updateCursorPosition( ... )
	{
		if (this.mCursor)
		{
			local adjusted = false;
			local d = 0;

			if (this.mMultiLine == false && (vargc == 0 || vargv[0] == true))
			{
				d = this.getCursorInRange();

				while (d != 0)
				{
					this.mStartIndex += d;
					adjusted = true;

					if (this.mStartIndex < 0)
					{
						this.mStartIndex = 0;
						break;
					}
				}
			}

			if (adjusted)
			{
				this.buildRows();
			}

			local cursorLoc = this.getPointFromCursorLocation(this.mCursorEnd);
			local offset = this._calcCursorOffset();

			if (cursorLoc)
			{
				local pos = {
					x = cursorLoc.x + offset.x,
					y = cursorLoc.y + offset.y
				};
				this.mCursor.setPosition(pos);
				local height = this.mRows[0].height;
				this.mCursor.setSize(1, height);
			}
			else
			{
				this.mCursor.setPosition(offset);
				this.mCursor.setSize(1, this.getFont().height);
			}
		}
	}

	function _calcCursorOffset()
	{
		if (this.mCursor)
		{
			local font = this.mCursor.getFont();
			local size = this.mCursor.getSize();
			local x = -size.width;
			local y = 0;
			return {
				x = x + this.insets.left,
				y = y + this.insets.top
			};
		}

		return null;
	}

	function clearSelection()
	{
		foreach( m in this.mRows )
		{
			if (m.selection)
			{
				m.selection.destroy();
				m.selection = null;
			}
		}
	}

	function setCursorVisible( which )
	{
		this.clearSelection();

		if (this.mCursor)
		{
			this.remove(this.mCursor);
			this.mCursor.destroy();
			this.mCursor = null;
		}

		if (which == true)
		{
			this.mCursor = this.GUI.Component(null);
			this.mCursor.setAppearance("EditArea/Caret");
			this.mCursor.setLayoutExclude(true);
			this.mCursor.setVisible(true);
			this.add(this.mCursor);
			this.updateCursorPosition();
		}
	}

	function getCursorLocationFromPoint( x, y, ... )
	{
		local row = this.pickRow(x, y);

		if (row)
		{
			local label = row.label;
			local font = label.getFont();
			local pos = label.getPosition().x;
			local text = row.text;

			for( local i = 0; i < text.len(); i++ )
			{
				local c = text.slice(0, i + 1);
				local newpos = font.getTextMetrics(c).width + label.getPosition().x;
				local start = pos;
				local end = newpos;

				if (x >= start && x < end)
				{
					if (vargc == 0 || vargv[0] == true)
					{
						if (x > start + (end - start) / 2)
						{
							return row.start + i + 1;
						}
					}

					return row.start + i;
				}

				pos = newpos;
			}

			if (x <= this.insets.left)
			{
				if (this.mMultiLine == false)
				{
					return row.start > 0 ? row.start - 1 : row.start;
				}

				return row.start;
			}

			return row.end;
		}

		return this.mText.len();
	}

	function getRowWidth( row )
	{
		local font = row.label.getFont();
		return font.getTextMetrics(row.text).width;
	}

	function getPointFromCursorLocation( loc )
	{
		local row = this.pickRowFromCursor(loc);

		if (row != null)
		{
			local font = row.label.getFont();
			local text = row.text;
			local xpos = 0;
			local c = text.slice(0, loc - row.start);
			xpos = font.getTextMetrics(c).width;
			local offset = 0.0;

			if (this.mCenterText)
			{
				offset = this.mLabelXOffset - this.insets.left;
			}

			local yOffset = 0;

			if (this.mMultiLine && this.mLineOffset > 0)
			{
				yOffset = this.mLineOffset * row.height;
			}

			local yAmount = row.y - yOffset;

			if (this.mMultiLine)
			{
				if (yAmount >= this.mHeight - this.insets.top - row.height)
				{
					this.mLineOffset = this.mLineOffset + 2;
					yAmount = yAmount - row.height * 2;

					if (yAmount > this.mHeight - this.insets.top - row.height)
					{
						local extraLines = (yAmount - this.mHeight) / row.height + 1;
						local extra = (yAmount - this.mHeight) % row.height;

						if (extra > 0)
						{
							extraLines = extraLines + 1;
						}

						this.mLineOffset = this.mLineOffset + extraLines;
						yAmount = yAmount - row.height * extraLines;
					}

					this.buildRows();
				}
				else if (yAmount <= 0 && this.mLineOffset > 0)
				{
					if (yAmount < 0 || this.mMouseMove)
					{
						this.mLineOffset = this.mLineOffset - 1;
						yAmount = yAmount + row.height;
						this.buildRows();
					}
					else
					{
						yAmount = 0;
					}
				}
			}

			return {
				x = xpos + offset,
				y = yAmount
			};
		}

		return null;
	}

	function _wordStart()
	{
		if (this.mPassword || this.mText.len() == 0)
		{
			return 0;
		}
		else
		{
			local i = this.mCursorEnd - 1;

			while (i >= 0)
			{
				if (this.mText[i] == 32 || this.mText[i] == 9 || this.mText[i] == 13 || this.mText[i] == 10)
				{
					break;
				}

				--i;
			}

			++i;

			if (i > this.mCursorEnd)
			{
				i = this.mCursorEnd;
			}

			return i;
		}
	}

	function _wordEnd()
	{
		local len = this.mText.len();

		if (this.mPassword || len == 0)
		{
			return 0;
		}
		else
		{
			local i = this.mCursorEnd;

			while (i < len)
			{
				if (this.mText[i] == 32 || this.mText[i] == 9 || this.mText[i] == 13 || this.mText[i] == 10)
				{
					break;
				}

				++i;
			}

			return i;
		}
	}

	function setAllowTextEntryOnClick( value )
	{
		this.mAllowInputInClick = value;
	}

	function onMousePressed( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			if (this.mAllowInputInClick)
			{
				this.GUI._Manager.requestKeyboardFocus(this);
			}

			if (evt.clickCount == 1 && this.mAllowInputInClick)
			{
				this.setCursorVisible(true);
				local cursorLoc = this.getCursorLocationFromPoint(evt.x, evt.y);
				this.setCursorPosition(cursorLoc, ::Key.isDown(this.Key.VK_SHIFT));
				this.mMousePressed = true;
			}
			else if (evt.clickCount == 2)
			{
				this.clearSelection();
				this.mCursorStart = this._wordStart();
				this.mCursorEnd = this._wordEnd();
				this.buildRows();
				this.updateCursorPosition();
			}
			else if (evt.clickCount == 3)
			{
				this.clearSelection();

				if (this.mPassword)
				{
					this.mCursorStart = 0;
					this.mCursorEnd = 0;
				}
				else
				{
					this.mCursorStart = 0;
					this.mCursorEnd = this.mText.len();
				}

				this.buildRows();
				this.updateCursorPosition();
			}

			if (this.mMousePressedMessage != null)
			{
				this._fireActionPerformed(this.mMousePressedMessage);
			}

			evt.consume();
		}
	}

	function onMouseReleased( evt )
	{
		if (evt.button == this.MouseEvent.LBUTTON)
		{
			evt.consume();
			this.mMousePressed = false;
		}
	}

	function onMouseMoved( evt )
	{
		if (this.mMousePressed)
		{
			this.mMouseMove = true;
			this.mCursorEnd = this.getCursorLocationFromPoint(evt.x, evt.y);
			this.updateCursorPosition(false);
			this.buildRows();
			evt.consume();
			this.mMouseMove = false;
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

	function setTabOrderTarget( target )
	{
		this.mTabOrderTarget = target;
	}

	function getTabOrderTarget()
	{
		return this.mTabOrderTarget;
	}

	function setText( text )
	{
		if (text == null)
		{
			return;
		}

		if (typeof text != "string")
		{
			text = text.tostring();
		}

		this.mText = text;

		if (!this.mAllowSpaces)
		{
			this.mText = this.Util.replace(this.mText, " ", "");
		}

		if (this.mAutoCapitalize)
		{
			local fixedText = "";
			local textLen = this.mText.len();

			if (textLen > 0)
			{
				local firstLetter = this.mText.slice(0, 1);
				fixedText = firstLetter.toupper();

				if (textLen > 1)
				{
					local secondLetter = this.mText.slice(1);
					fixedText += secondLetter.tolower();
				}

				this.mText = fixedText;
			}
		}

		this.mCursorStart = this.Math.min(this.mCursorStart, this.mText.len());
		this.mCursorEnd = this.Math.min(this.mCursorEnd, this.mText.len());
		this.buildRows();
		this.updateCursorPosition();
	}

	function getText()
	{
		return this.mText;
	}

	function getValue()
	{
		return this.mText.tointeger();
		  // [006]  OP_POPTRAP        1      0    0    0
		  // [007]  OP_JMP            0      2    0    0
		return 0;
	}

	function _recallUp()
	{
		if (this.mStoredIndex < 9 && this.mStoredIndex + 1 < this.mStoredInputs.len())
		{
			this.mStoredIndex++;
			this.setText(this.mStoredInputs[this.mStoredIndex]);
			this.setCursorPosition(this.mText.len());
		}
	}

	function _recallDown()
	{
		if (this.mStoredIndex > 0 && this.mStoredIndex - 1 < this.mStoredInputs.len())
		{
			this.mStoredIndex--;
			this.setText(this.mStoredInputs[this.mStoredIndex]);
			this.setCursorPosition(this.mText.len());
		}
	}

	function _saveInput()
	{
		this.mStoredIndex = -1;

		if (this.mText != "")
		{
			if (this.mStoredInputs.len() > 0 && this.mStoredInputs[0] == this.mText)
			{
				return;
			}

			this.mStoredInputs.insert(0, this.mText);

			while (this.mStoredInputs.len() > 10)
			{
				this.mStoredInputs.remove(10);
			}
		}
	}

	function insertText( text )
	{
		local newText;

		if (this.mCursorStart != this.mCursorEnd)
		{
			this.deleteSelection();
		}

		local tstart = this.mText.slice(0, this.mCursorStart);
		local tend = this.mText.slice(this.mCursorEnd);
		local newText = tstart + text + tend;
		this.setText(newText);
		this.moveCursor(text.len());
	}

	function deleteSelection()
	{
		if (this.mText.len() == 0)
		{
			return;
		}

		if (this.mCursorStart != this.mCursorEnd)
		{
			local range = this.getSelectionRange();
			local tstart = this.mText.slice(0, range[0]);
			local tend = this.mText.slice(range[1]);
			this.mCursorStart = range[0];
			this.mCursorEnd = range[0];
			this.setText(tstart + tend);
		}
		else if (this.mCursorEnd != this.mText.len())
		{
			this.mCursorEnd++;
			this.backspace();
		}
	}

	function backspace()
	{
		if (this.mText.len() == 0 || this.mCursorEnd == 0)
		{
			return;
		}

		if (this.mCursorStart == this.mCursorEnd)
		{
			local tstart = this.mText.slice(0, this.mCursorEnd - 1);
			local tend = this.mText.slice(this.mCursorEnd);
			this.moveCursor(-1);
			this.setText(tstart + tend);
		}
		else
		{
			this.deleteSelection();
		}
	}

	function onRequestedKeyboardFocus()
	{
		if (this.mWidget != null && !this.mKeyFocus)
		{
			this.mWidget.requestKeyboardFocus();
			this.setCursorVisible(true);
		}

		this.mKeyFocus = true;
	}

	function onReleasedKeyboardFocus()
	{
		if (this.mWidget != null && this.mKeyFocus)
		{
			this.setCursorVisible(false);
			this.mCursorStart = this.mCursorEnd;
		}

		this.mKeyFocus = false;
	}

	function getCurrentRow()
	{
		return this.pickRowFromCursor(this.mCursorStart);
	}

	function goToHome()
	{
		if (this.mMultiLine == true)
		{
			local rowIndex = this.getRowIndexFromCursor(this.mCursorEnd);

			if (rowIndex == null)
			{
				return;
			}

			local row = this.mRows[rowIndex];
			this.setCursorPosition(row.start + (row.text[0] == 10 ? 1 : 0), ::Key.isDown(this.Key.VK_SHIFT));
		}
		else
		{
			this.setCursorPosition(0, ::Key.isDown(this.Key.VK_SHIFT));
		}
	}

	function goToEnd()
	{
		if (this.mMultiLine == true)
		{
			local rowIndex = this.getRowIndexFromCursor(this.mCursorEnd);

			if (rowIndex == null)
			{
				return;
			}

			local row = this.mRows[rowIndex];

			if (rowIndex < this.mRows.len() - 1 && this.mRows[rowIndex + 1].text[0] != 10)
			{
				this.setCursorPosition(row.end - 1, ::Key.isDown(this.Key.VK_SHIFT));
			}
			else
			{
				this.setCursorPosition(row.end, ::Key.isDown(this.Key.VK_SHIFT));
			}
		}
		else
		{
			this.setCursorPosition(this.mText.len(), ::Key.isDown(this.Key.VK_SHIFT));
		}
	}

	function getSelectionText()
	{
		local range = this.getSelectionRange();
		return this.mText.slice(range[0], range[1]);
	}

	function copy()
	{
		this.System.setClipboard(this.getSelectionText());
	}

	function paste()
	{
		local text = this.System.getClipboard();
		local count = this.getText().len() + text.len();

		if (this.mMaxCharacters >= 0 && count > this.mMaxCharacters)
		{
			text = text.slice(0, text.len() - (count - this.mMaxCharacters));
		}

		this.insertText(text);
	}

	function getKeyTable( event )
	{
		if (this.Key.capsOn() && event.isShiftDown())
		{
			return this._KeyNames_Caps_Shift;
		}
		else if (event.isShiftDown())
		{
			return this._KeyNames_Shift;
		}
		else if (this.Key.capsOn())
		{
			return this._KeyNames_Caps;
		}

		return this._KeyNames;
	}

	function setMultiLine( which )
	{
		this.mMultiLine = which;
		this.buildRows();
	}

	function setCenterText( value )
	{
		this.mCenterText = value;
	}

	function setMaxCharacters( maxCharacters )
	{
		this.mMaxCharacters = maxCharacters;
	}

	function isKeyNumeric( key )
	{
		local numeric = key == this.Key.VK_1 || key == this.Key.VK_2 || key == this.Key.VK_3 || key == this.Key.VK_4 || key == this.Key.VK_5 || key == this.Key.VK_6 || key == this.Key.VK_7 || key == this.Key.VK_8 || key == this.Key.VK_9 || key == this.Key.VK_0 || key == this.Key.VK_NUMPAD0 || key == this.Key.VK_NUMPAD1 || key == this.Key.VK_NUMPAD2 || key == this.Key.VK_NUMPAD3 || key == this.Key.VK_NUMPAD4 || key == this.Key.VK_NUMPAD5 || key == this.Key.VK_NUMPAD6 || key == this.Key.VK_NUMPAD7 || key == this.Key.VK_NUMPAD8 || key == this.Key.VK_NUMPAD9 || key == 189;
		return numeric;
	}

	function setAllowOnlyNumbers( value )
	{
		this.mNumbersOnly = value;

		if (this.mNumbersOnly)
		{
			this.mLettersOnly = false;
		}
	}

	function setAllowOnlyLetters( value )
	{
		this.mLettersOnly = value;

		if (this.mLettersOnly)
		{
			this.mNumbersOnly = false;
		}
	}

	function onRealKeyReleased( evt )
	{
		if (this.mLocked == false)
		{
			local numberOnly = ::regexp("-|\\d+");
			local found = numberOnly.search(evt.key);
			local letterOnly = ::regexp("[a-zA-Z]+");
			local letterOnlyFound = letterOnly.search(evt.key);

			if (!(evt.key in this.mIgnoreInitialCharacters))
			{
				if (this.mText.len() >= this.mMaxCharacters && this.mMaxCharacters != -1)
				{
				}
				else if (this.mNumbersOnly && found)
				{
					this.insertText(evt.key);
				}
				else if (this.mLettersOnly && letterOnlyFound)
				{
					this.insertText(evt.key);
				}
				else if (!this.mNumbersOnly && !this.mLettersOnly)
				{
					if (!(::Key.isDown(this.Key.VK_CONTROL) && evt.key == " "))
					{
						this.insertText(evt.key);
					}
				}
			}

			this.resetIgnoredCharacters();
		}

		evt.consume();
		this.onChanged();
	}

	function onKeyPressed( evt )
	{
		switch(evt.keyCode)
		{
		case ::Key.VK_BACK:
			if (this.mLocked == false)
			{
				this.backspace();
			}

			evt.consume();
			break;

		case ::Key.VK_ESCAPE:
			if (this.mMultiLine == false)
			{
				::GUI._Manager.requestKeyboardFocus(null);
				evt.consume();
			}

			break;

		case ::Key.VK_LEFT:
			this.moveCursor(-1, true);
			evt.consume();
			break;

		case ::Key.VK_RIGHT:
			this.moveCursor(1, true);
			evt.consume();
			break;

		case ::Key.VK_HOME:
			this.goToHome();
			evt.consume();
			break;

		case ::Key.VK_DELETE:
			if (this.mLocked == false)
			{
				this.deleteSelection();
			}

			evt.consume();
			break;

		case ::Key.VK_END:
			this.goToEnd();
			evt.consume();
			break;

		case ::Key.VK_DOWN:
			if (this.mMultiLine == false)
			{
				this._recallDown();
			}
			else
			{
				this.moveCursorY(1, true);
			}

			evt.consume();
			break;

		case ::Key.VK_UP:
			if (this.mMultiLine == false)
			{
				this._recallUp();
			}
			else
			{
				this.moveCursorY(-1, true);
			}

			evt.consume();
			break;

		case ::Key.VK_ENTER:
			if (this.mLocked == false && this.mMultiLine == false)
			{
				this._saveInput();
				this.GUI._Manager.releaseKeyboardFocus(this);
				this._fireActionPerformed("onInputComplete");
				evt.consume();
				break;
			}
			else if (this.mLocked == false && this.mMultiLine == true)
			{
				this.insertText("\n");
				evt.consume();
				break;
			}

		case 9:
			if (this.mLocked == false && this.mMultiLine == false && this.mTabOrderTarget)
			{
				this.GUI._Manager.requestKeyboardFocus(this.mTabOrderTarget);

				if ("setCursorVisible" in this.mTabOrderTarget)
				{
					this.mTabOrderTarget.setCursorVisible(true);
				}

				evt.consume();
				break;
			}

		case ::Key.VK_C:
			if (::Key.isDown(this.Key.VK_CONTROL))
			{
				this.copy();
				evt.consume();
				break;
			}

		case ::Key.VK_V:
			if (this.mLocked == false && ::Key.isDown(this.Key.VK_CONTROL))
			{
				this.paste();
				evt.consume();
				break;
			}

		default:
			evt.consume();
			this.onChanged();
			break;
		}
	}

	function onChanged()
	{
		this._fireActionPerformed("onTextChanged");
	}

	function setLocked( which )
	{
		this.mLocked = which;
	}

}

