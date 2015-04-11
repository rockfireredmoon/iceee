this.require("GUI/InputBox");
class this.GUI.NumericInputBox extends this.GUI.InputBox
{
	constructor( ... )
	{
		if (vargc > 0 && this.type(vargv[0]).tolower() == "integer")
		{
			this.GUI.InputBox.constructor(vargv[0]);
		}
		else
		{
			this.GUI.InputBox.constructor();
		}

		this.setLayoutManager(this.GUI.SingleLineFlowLayout());
		this.getLayoutManager().setGaps(0, 0);
		this.getLayoutManager().setAlignment("right");
		this.mComponentParsedIndexList = [];
		this.mSelectionComponents = [];
		this.mKeyFocus = false;
		this.setText(this.mNumber.tostring());
		this.mMinNumber = 0;
	}

	function setText( pString )
	{
		try
		{
			local number = this.Math.max(this.mMinNumber, pString.tofloat());

			if (this.mMaxNumber != null)
			{
				number = this.Math.min(this.mMaxNumber, number);
			}

			this.GUI.InputBox.setText(number.tostring());
		}
		catch( err )
		{
		}
	}

	function getValue()
	{
		return this.mText == "" ? 0.0 : this.mText.tofloat();
		  // [013]  OP_POPTRAP        1      0    0    0
		  // [014]  OP_JMP            0      2    0    0
		return 0.0;
	}

	function setMaxNumber( pNumber )
	{
		this.mMaxNumber = pNumber;
	}

	function getMaxNumber()
	{
		return this.mMaxNumber;
	}

	function setMinNumber( pNumber )
	{
		this.mMinNumber = pNumber;
	}

	function getMinNumber()
	{
		return this.mMinNumber;
	}

	function onMouseWheel( evt )
	{
		if (evt.units_v > 0)
		{
			this.onNumberUp();
		}
		else if (evt.units_v < 0)
		{
			this.onNumberDown();
		}

		evt.consume();
	}

	function onNumberUp( ... )
	{
		local number = this.getValue() + 1;
		this.setText(number.tostring());
		this.setCaretIndex(0);
		this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
		this._fireActionPerformed("onInputComplete");
	}

	function onNumberDown( ... )
	{
		local number = this.getValue() - 1;
		this.setText(number.tostring());
		this.setCaretIndex(0);
		this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
		this._fireActionPerformed("onInputComplete");
	}

	function setMinimumSize( v )
	{
		this.mMinimumSize = v;
	}

	function getMinimumSize()
	{
		if (this.mMinimumSize != null)
		{
			return this.mMinimumSize;
		}

		if (this.mMaxNumber == null)
		{
			return {
				width = 0,
				height = 0
			};
		}

		local text = this.mMaxNumber.tostring();
		return this._addInsets(this.getFont().getTextMetrics(text + "0"));
	}

	function onKeyPressed( evt )
	{
		switch(evt.keyCode)
		{
		case ::Key.VK_DOWN:
			this.onNumberDown();
			evt.consume();
			break;

		case ::Key.VK_UP:
			this.onNumberUp();
			evt.consume();
			break;

		case ::Key.VK_LEFT:
			this.setCaretIndex(this.getCaretIndex() - 1);
			this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
			this._caretVisible();
			evt.consume();
			break;

		case ::Key.VK_RIGHT:
			this.setCaretIndex(this.getCaretIndex() + 1);
			this._updateSelection(this.getCaretIndex(), this.Key.isDown(this.Key.VK_SHIFT));
			this._caretVisible();
			evt.consume();
			break;

		case 46:
			if (this.Key.isDown(this.Key.VK_SHIFT))
			{
				this.System.setClipboard(this.getSelectionText());
			}

			this.deleteCaretIndex();
			evt.consume();
			break;

		case ::Key.VK_PERIOD:
			this.addText(".");
			evt.consume();
			break;

		case 189:
		case ::Key.VK_SUBTRACT:
			this.addText("-");
			evt.consume();
			break;

		case ::Key.VK_BACK:
			this.backspace();
			evt.consume();
			break;

		case this.Key.VK_1:
			this.addText("1");
			evt.consume();
			break;

		case this.Key.VK_2:
			this.addText("2");
			evt.consume();
			break;

		case this.Key.VK_3:
			this.addText("3");
			evt.consume();
			break;

		case this.Key.VK_4:
			this.addText("4");
			evt.consume();
			break;

		case this.Key.VK_5:
			this.addText("5");
			evt.consume();
			break;

		case this.Key.VK_6:
			this.addText("6");
			evt.consume();
			break;

		case this.Key.VK_7:
			this.addText("7");
			evt.consume();
			break;

		case this.Key.VK_8:
			this.addText("8");
			evt.consume();
			break;

		case this.Key.VK_9:
			this.addText("9");
			evt.consume();
			break;

		case this.Key.VK_0:
			this.addText("0");
			evt.consume();
			break;

		case ::Key.VK_ENTER:
			this.GUI._Manager.releaseKeyboardFocus(this);
			this._fireActionPerformed("onInputComplete");
			evt.consume();
			break;
		}
	}

	mNumber = 0;
	mMinNumber = 0;
	mMaxNumber = null;
	mMinimumSize = null;
	static mClassName = "NumericInputBox";
}

