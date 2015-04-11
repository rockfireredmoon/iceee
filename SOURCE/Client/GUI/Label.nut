this.require("GUI/Component");
class this.GUI.Label extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();

		if (vargc > 0 && typeof vargv[0] == "string")
		{
			this.mText = vargv[0];
		}
		else
		{
			this.mText = "";
		}

		if (this.mText == null)
		{
			this.mText = "";
		}

		this.mAppearance = "Label";
		this.mTextHAlign = 0.0;
		this.mTextVAlign = 0.5;
		this.mLayoutManager = null;
		this.setSize(this.getPreferredSize());
		this._updateDisplayedText();
	}

	function setText( text )
	{
		if (text == null)
		{
			text = "";
		}

		if (this.mText != text)
		{
			this.mText = text;
			this.invalidate();
		}
	}

	function getText()
	{
		return this.mText;
	}

	function setValue( value )
	{
		this.setText("" + value);
	}

	function getValue()
	{
		return this.getText();
	}

	function addActionListener( listener )
	{
	}

	function setFont( font )
	{
		this.GUI.Component.setFont(font);
		this.invalidate();
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.setText(this.mText);
	}

	function validate()
	{
		::GUI.Component.validate();

		if (this.mWidget != null)
		{
			this.mWidget.setParam("ignore_new_lines", this.mIgnoreNewLines.tostring());
		}
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();

		if (this.mWidget != null)
		{
			this._updateDisplayedText();
			this.mWidget.setText(this.mDisplayedText);
			local sz = this.getFont().getTextMetrics(this.mDisplayedText);
			local h = this.mTextHAlign;
			local v = this.mTextVAlign;
			local width = this.mWidth - this.insets.left - this.insets.right;
			local height = this.mHeight - this.insets.top - this.insets.bottom;
			local x = this.mX + width * h - h * sz.width;
			local y = this.mY + height * v - v * sz.height;
			this.mWidget.setPosition(x.tointeger() + this.insets.left, y.tointeger() + this.insets.top);
			this.mWidget.setSize(this.mAutoFit == true ? this.mWidth : sz.width, sz.height);
		}
	}

	function setAutoFit( af )
	{
		this.mAutoFit = af;
		this._updateDisplayedText();
	}

	function _updateDisplayedText()
	{
		if (this.mText == null)
		{
			this.mDisplayedText = "";
			return;
		}

		if (this.mAutoFit == false)
		{
			this.mDisplayedText = this.mText;
			return;
		}

		local oldtext = this.mDisplayedText;
		local sz;
		local maxsize = this.getSize();
		maxsize.height -= this.insets.top;
		maxsize.height -= this.insets.bottom;
		maxsize.width -= this.insets.left;
		maxsize.width -= this.insets.right;
		sz = this.getFont().getTextMetrics(this.mText != null ? this.mText : "");

		if (sz.width <= maxsize.width)
		{
			this.mDisplayedText = this.mText;
		}
		else
		{
			local len = this.mText.len();
			local step = 1;

			while (step * 2 < len)
			{
				step *= 2;
			}

			local displen = 0;

			while (step >= 1)
			{
				local newsize = this.getFont().getTextMetrics(this.mText.slice(0, displen + step) + "...");

				if (newsize.width <= maxsize.width)
				{
					displen += step;
				}

				step /= 2;
			}

			this.mDisplayedText = this.mText.slice(0, displen) + "...";

			if (this.mDisplayedText == "...")
			{
				this.mDisplayedText = this.mText.slice(0, 1) + "..";
			}
		}

		if (this.mDisplayedText != oldtext)
		{
			this.invalidate();
		}
	}

	function getPreferredSize()
	{
		return this._addInsets(this.getFont().getTextMetrics(this.mText));
	}

	function setTextAlignment( horizontal, vertical )
	{
		this.mTextHAlign = horizontal.tofloat();
		this.mTextVAlign = vertical.tofloat();
	}

	function getTextAlignment()
	{
		return {
			horizontal = this.mTextHAlign,
			vertical = this.mTextVAlign
		};
	}

	function setIgnoreNewLines( which )
	{
		this.mIgnoreNewLines = which;

		if (this.mWidget != null)
		{
			this.mWidget.setParam("ignore_new_lines", which.tostring());
		}
	}

	function getIgnoreNewLines()
	{
		return this.mIgnoreNewLines;
	}

	function _debugstring()
	{
		return this.GUI.Component._debugstring() + " text=\"" + this.mText + "\"";
	}

	mAutoFit = false;
	mDisplayedText = "";
	mText = "";
	mTextHAlign = 0.0;
	mTextVAlign = 0.5;
	mIgnoreNewLines = false;
	static mClassName = "Label";
}

