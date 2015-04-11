this.require("GUI/Label");
class this.GUI.HTML extends this.GUI.Label
{
	mWrapWidth = 0;
	mWrapText = false;
	mFont = null;
	mTextChanged = false;
	mStack = [];
	mNoTagText = "";
	mTagOptsCache = {};
	mTagOptsDirty = true;
	mLinkClickedMessage = "onLinkClicked";
	mLinkStaticColor = this.Color("44AAFF");
	mLinkHoverColor = this.Color("AADDFF");
	mAllowColorChangeOnHover = true;
	static mClassName = "HTML";
	constructor( ... )
	{
		this.GUI.Label.constructor();
		this.setAppearance("Container");
		this.setLayoutManager(::GUI.FlowLayout());
		this.getLayoutManager().setAlignment("left");
		this.getLayoutManager().setGaps(0, 0);
		this.setInsets(0, 0, 0, 0);
		this.mStack = [];
		this.mTagOptsDirty = true;
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			this.setText(vargv[0]);
		}
	}

	function validate()
	{
		if (this.mTextChanged)
		{
			this.parseHTML(this.mText);
		}

		this.mTextChanged = false;
		this.GUI.Label.validate();
	}

	function _buildOpts()
	{
		if (this.mTagOptsDirty == false)
		{
			return this.mTagOptsCache;
		}

		this.mTagOptsCache = {};
		local fontFace = this.getFont().getFace();
		local fontHeight = this.getFont().getHeight();
		local fontBold = this.getFont().isBold();
		local fontItalic = this.getFont().isItalic();
		local fontColor = this.getFontColor();
		local isLink = false;
		local linkData = "";

		foreach( x in this.mStack )
		{
			if (typeof x != "table")
			{
				continue;
			}

			switch(x.type)
			{
			case "b":
				fontBold = true;
				break;

			case "i":
				fontItalic = true;
				break;

			case "a":
				isLink = true;
				linkData = {};
				fontColor = this.mLinkStaticColor;
				linkData.fontColor <- fontColor;

				foreach( i, a in x )
				{
					if (i.tolower() == "href")
					{
						linkData.href <- a;
					}

					if (i.tolower() == "info")
					{
						linkData.info <- a;
					}
				}

				break;

			case "font":
				foreach( i, a in x )
				{
					if (i.tolower() == "name")
					{
						fontFace = a;
					}
					else if (i.tolower() == "size")
					{
						fontHeight = a.tointeger();
					}
					else if (i.tolower() == "color")
					{
						fontColor = this.Color(a);
					}
				}

				break;
			}
		}

		this.mTagOptsCache.font <- ::GUI.Font(fontFace, fontHeight, fontBold, fontItalic);
		this.mTagOptsCache.link <- isLink;
		this.mTagOptsCache.linkdata <- linkData;
		this.mTagOptsCache.fontColor <- fontColor;
		this.mTagOptsDirty = false;
		return this.mTagOptsCache;
	}

	function _addTag( pType )
	{
		this.mTagOptsDirty = true;
		this.mStack.append({
			type = pType
		});
	}

	function _addAttribute( pName, pValue )
	{
		this.mTagOptsDirty = true;

		if (this.mStack.len() > 0)
		{
			this.mStack[this.mStack.len() - 1][pName] <- pValue;
		}
		else
		{
			this.log.debug("Attribute received outside of any tags.");
		}
	}

	function _removeTag( pType )
	{
		this.mTagOptsDirty = true;

		for( local x = this.mStack.len() - 1; x >= 0; x-- )
		{
			if (this.mStack[x].type == pType)
			{
				this.mStack.remove(x);
				return;
			}
		}
	}

	function getPreferredSize()
	{
		if (this.mPreferredSize != null)
		{
			return this.mPreferredSize;
		}

		this.validate();
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

	function getMinimumSize()
	{
		return this._addInsets(this.getFont().getTextMetrics("   "));
	}

	function parseHTML( pString )
	{
		this.mTagOptsDirty = true;
		this.removeAll();
		this.mNoTagText = "";
		::parseXML(pString, this);
	}

	function setWrapText( wrap, ... )
	{
		this.mWrapText = wrap;

		if (vargc > 1)
		{
			this.mFont = vargv[0];
			this.mWrapWidth = vargv[1];
		}
	}

	function onXmlElementStart( text )
	{
		if (text != "br")
		{
			this._addTag(text);
		}
		else
		{
			local opts = this._buildOpts();
			local height = opts.font.height;
			local component = this.GUI.Spacer(0, height);
			component.setOpts(opts);
			this.add(component, "break");
		}
	}

	function onXmlElementEnd( text )
	{
		if (text != "br")
		{
			this._removeTag(text);
		}
	}

	function onXmlText( text )
	{
		local opts = this._buildOpts();
		local component;

		if (this.mWrapText && this.mFont && this.mWrapWidth)
		{
			local face = this.mFont.getFullFace();
			local faceHeight = this.mFont.getHeight();

			if (this.mWrapWidth > 0)
			{
				local result = this.Screen.wordWrap(text, face, this.mWrapWidth, faceHeight);
				text = ::Util.replace(result.text, " \n", " ");
				text = ::Util.replace(text, "\n", " ");
			}
		}

		if (opts.link)
		{
			component = this.GUI.Link(text, this, "onLinkClicked");
			component.setChangeColorOnHover(this.mAllowColorChangeOnHover);
			component.setData(opts.linkdata);
			component.setStaticColor(this.mLinkStaticColor);
			component.setHoverColor(this.mLinkHoverColor);
			component.setOpts(opts);
			component.setVisible(true);
			this.add(component);
		}
		else
		{
			local parts = this._splitSpaces(text);

			foreach( p in parts )
			{
				component = this.GUI.Label(p);
				component.setOpts(opts);
				component.setVisible(true);
				this.add(component);
			}
		}

		this.mNoTagText += text;
	}

	function _splitSpaces( str )
	{
		local result = [];
		local start = 0;

		while (true)
		{
			local end = str.find(" ", start);

			if (end == null)
			{
				result.append(str.slice(start));
				break;
			}

			result.append(str.slice(start, end) + " ");
			start = end + 1;
		}

		return result;
	}

	function onXmlAttribute( key, value )
	{
		this._addAttribute(key, value);
	}

	function onXmlParseError( message, character )
	{
		this.log.debug("XML Event Received: !! Parse Error !!!!!!!! " + message + " (Character " + character + ")");
	}

	function onXmlParseWarning( message, character )
	{
		this.log.debug("XML Event Received: !! Parse Warning !!!!!! " + message + " (Character " + character + ")");
	}

	function setFont( font )
	{
		this.GUI.Panel.setFont(font);
		this.mTextChanged = true;
		this.invalidate();
	}

	function setFontColor( fontColor )
	{
		this.GUI.Panel.setFontColor(fontColor);
		this.mTextChanged = true;
		this.invalidate();
	}

	function setLinkStaticColor( staticColor )
	{
		this.mLinkStaticColor = staticColor;
	}

	function setLinkHoverColor( hoverColor )
	{
		this.mLinkHoverColor = hoverColor;
	}

	function setChangeColorOnHover( value )
	{
		this.mAllowColorChangeOnHover = value;
	}

	function setSize( ... )
	{
		local oldW = this.mWidth;
		local oldH = this.mHeight;
		local w;
		local h;

		if (vargc == 1 && (typeof vargv[0] == "table" || typeof vargv[0] == "instance"))
		{
			w = vargv[0].width;
			h = vargv[0].height;
		}
		else if (vargc == 2)
		{
			w = vargv[0];
			h = vargv[1];
		}
		else
		{
			throw this.Exception("Invalid arguments to Component.setSize()");
		}

		this.mWidth = w.tointeger();
		this.mHeight = h.tointeger();

		if (this.mIsRealized)
		{
			this._reshapeNotify();
		}

		this.invalidate();
	}

	function getSize()
	{
		return {
			width = this.mWidth,
			height = this.mHeight
		};
	}

	function getNoTagsText()
	{
		if (this.mTextChanged)
		{
			this.validate();
			return this.mNoTagText;
		}
		else
		{
			return this.mNoTagText;
		}
	}

	function setText( pString )
	{
		if (typeof pString != "string")
		{
			pString = "" + pString;
		}

		if (pString != this.mText)
		{
			this.mText = pString;
			this.mTextChanged = true;
			this.invalidate();
		}
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.setText(this.mText);
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function _reshapeNotify()
	{
		this.GUI.Component._reshapeNotify();
	}

	function setPosition( ... )
	{
		local x;
		local y;

		if (vargc == 0)
		{
			return;
		}

		if (vargc == 1 && typeof vargv[0] == "table")
		{
			x = vargv[0].x;
			y = vargv[0].y;
		}
		else if (vargc == 2)
		{
			x = vargv[0].tointeger();
			y = vargv[1].tointeger();
		}
		else
		{
			return;
		}

		::GUI.Component.setPosition(x, y);
	}

	function onMouseWheel( evt )
	{
		if (this.mScroll)
		{
			if (evt.units_v > 0)
			{
				this.mScroll.onLineUp(this, false);
			}
			else if (evt.units_v < 0)
			{
				this.mScroll.onLineDown(this, false);
			}
		}

		evt.consume();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function onLinkClicked( button )
	{
		local data = button.getData();
		data.clickedOnText <- button.getText();
		this.mMessageBroadcaster.broadcastMessage(this.mLinkClickedMessage, this, data);
	}

	function setLinkClickedMessage( message )
	{
		this.mLinkClickedMessage = message;
	}

}

