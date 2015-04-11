this.require("GUI/Label");
class this.GUI.HTMLOld extends this.GUI.Label
{
	constructor( ... )
	{
		this.GUI.Label.constructor();
		this.setAppearance("Container");
		this.setLayoutManager(::GUI.FlowLayout());
		this.getLayoutManager().setAlignment("left");
		this.getLayoutManager().setGaps(0, 0);
		this.setInsets(2);
		this.mOpts = {};
		this.mStack = [];
		this._buildOpts();

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
		this.mOpts = {};
		this.mLinkFlag = false;
		local fontFace = this.getFont().getFace();
		local fontHeight = this.getFont().getHeight();
		local fontBold = this.getFont().isBold();
		local fontItalic = this.getFont().isItalic();
		this.mOpts.fontColor <- this.getFontColor();

		foreach( x in this.mStack )
		{
			if (typeof x != "table")
			{
				continue;
			}

			foreach( i, a in x )
			{
				if (i.tolower() == "fontname")
				{
					fontFace = a;
				}
				else if (i.tolower() == "fontcolor")
				{
					this.mOpts.fontColor = this.Color(a);
				}
				else if (i.tolower() == "fontsize")
				{
					fontHeight = a;
				}
				else if (i.tolower() == "type" && a.tolower() == "href")
				{
					this.mLinkFlag = true;
				}
				else if (i.tolower() == "type" && a.tolower() == "b")
				{
					fontBold = true;
				}
				else if (i.tolower() == "type" && a.tolower() == "i")
				{
					fontItalic = true;
				}
				else if (i != "type")
				{
					this.mOpts[i] <- a;
				}
			}
		}

		this.mOpts.font <- ::GUI.Font(fontFace, fontHeight, fontBold, fontItalic);
		return this.mOpts;
	}

	function _stripTags( pString )
	{
		local text = "";
		local tokens;

		while (pString.len() > 0)
		{
			tokens = this.getToken(pString, this.GUI.RegExp.Tag);

			if (tokens)
			{
				local slash = this.getToken(tokens[0], this.GUI.RegExp.Slash);
				local name;

				if (slash)
				{
					name = this.getToken(slash[1], this.GUI.RegExp.WordNoSlash);
				}
				else
				{
					name = this.getToken(tokens[0], this.GUI.RegExp.WordNoSlash);
				}

				if (name)
				{
					if (name[0].tolower() == "br")
					{
						text += "\n";
					}
				}

				if (tokens[1].len() > 1)
				{
					pString = tokens[1].slice(1, tokens[1].len());
				}
				else
				{
					pString = "";
				}
			}
			else
			{
				tokens = this.getToken(pString, this.GUI.RegExp.NewLine);

				if (tokens)
				{
					pString = tokens[1];
				}
				else
				{
					tokens = this.getToken(pString, this.GUI.RegExp.Space);

					if (tokens)
					{
						text += tokens[0];
						pString = tokens[1];
					}
					else
					{
						tokens = this.getToken(pString, this.GUI.RegExp.Word);

						if (tokens)
						{
							text += tokens[0];
							pString = tokens[1];
						}
					}
				}
			}
		}

		return text;
	}

	function _removeTag( pType )
	{
		for( local x = this.mStack.len() - 1; x >= 0; x-- )
		{
			if (this.mStack[x].type == pType)
			{
				this.mStack.remove(x);
				this._buildOpts;
				local height = this.mOpts.font.height;
				local component = this.GUI.Spacer(0, height);
				component.setOpts(this.mOpts);
				this.add(component);
				return;
			}
		}
	}

	function _parseAttribute( pAttributeString )
	{
		local tokens = [];
		local attributes = [];

		while (pAttributeString.len() > 0)
		{
			local parts = {};
			local res = ::GUI.RegExp.Attributes.capture(pAttributeString);

			if (!res)
			{
				break;
			}

			parts.attribute <- pAttributeString.slice(res[1].begin, res[1].end);
			local start = res[1].end;
			parts.value <- "";

			if (res.len() == 3)
			{
				parts.value = pAttributeString.slice(res[2].begin, res[2].end);
				start = res[2].end;
			}

			pAttributeString = pAttributeString.slice(start, pAttributeString.len());

			if (!parts)
			{
				return null;
			}

			attributes.append(parts);
		}

		if (!attributes)
		{
			return null;
		}

		return attributes;
	}

	function _tagB( pTag, ... )
	{
		local removeFlag = false;

		if (vargc > 0)
		{
			removeFlag = vargv[0];
		}

		if (removeFlag)
		{
			this._removeTag("b");
			this._buildOpts();
			return null;
		}

		this.mStack.append(pTag);
		this._buildOpts();
		local height = this.mOpts.font.height;
		local component = this.GUI.Spacer(0, height);
		component.setOpts(this.mOpts);
		this.add(component);
	}

	function _tagI( pTag, ... )
	{
		local removeFlag = false;

		if (vargc > 0)
		{
			removeFlag = vargv[0];
		}

		if (removeFlag)
		{
			this._removeTag("i");
			this._buildOpts();
			return null;
		}

		this.mStack.append(pTag);
		this._buildOpts();
		local height = this.mOpts.font.height;
		local component = this.GUI.Spacer(0, height);
		component.setOpts(this.mOpts);
		this.add(component);
	}

	function _tagBR()
	{
		local height = this.mOpts.font.height;
		local component = this.GUI.Spacer(0, height);
		this.add(component, "break");
		component.setOpts(this.mOpts);
		return null;
	}

	function _tagFont( pTag, pAttributeString, ... )
	{
		local tokens = [];
		local removeFlag = false;

		if (vargc > 0)
		{
			removeFlag = vargv[0];
		}

		if (removeFlag)
		{
			this._removeTag("font");
			this._buildOpts();
			return null;
		}
		else
		{
			tokens = this._parseAttribute(pAttributeString);

			if (tokens)
			{
				foreach( i, x in tokens )
				{
					switch(x.attribute.tolower())
					{
					case "color":
						pTag.fontColor <- x.value;
						break;

					case "name":
						pTag.fontName <- x.value;
						break;

					case "size":
						pTag.fontSize <- x.value.tointeger();
						break;

					default:
						break;
					}
				}
			}
		}

		if (pTag)
		{
			this.mStack.append(pTag);
			this._buildOpts();
		}

		local height = this.mOpts.font.height;
		local component = this.GUI.Spacer(0, height);
		component.setOpts(this.mOpts);
		this.add(component);
	}

	function _tagA( pTag, pAttributeString, ... )
	{
		local tokens = [];
		local removeFlag = false;
		local component;

		if (vargc > 0)
		{
			removeFlag = vargv[0];
		}

		if (removeFlag)
		{
			component = ::GUI.Link("]");

			if (this.mOpts)
			{
				component.setOpts(this.mOpts);
			}

			this.add(component);
			this._removeTag("a");
			this._buildOpts();
			return null;
		}

		pTag.fontColor <- "880088";
		tokens = this._parseAttribute(pAttributeString);

		if (tokens)
		{
			foreach( i, x in tokens )
			{
				tokens[0].attribute.tolower();
				  // [047]  OP_JMP            0      0    0    0
			}
		}

		if (pTag)
		{
			this.mStack.append(pTag);
			this._buildOpts();
		}

		component = ::GUI.Link("[");

		if (this.mOpts)
		{
			component.setOpts(this.mOpts);
		}

		this.add(component);
	}

	function _parseTag( pTag )
	{
		local tag = {};
		local removeFlag = false;
		local tokens = this.getToken(pTag, this.GUI.RegExp.Slash);

		if (tokens)
		{
			removeFlag = true;
			pTag = tokens[1];
		}

		tokens = this.getToken(pTag, this.GUI.RegExp.WordNoSlash);

		if (tokens)
		{
			tag.type <- tokens[0];
			pTag = tokens[1];
		}

		switch(tag.type.tolower())
		{
		case "font":
			this._tagFont(tag, pTag, removeFlag);
			break;

		case "br":
			this._tagBR();
			break;

		case "b":
			this._tagB(tag, removeFlag);
			break;

		case "i":
			this._tagI(tag, removeFlag);
			break;

		case "a":
			this._tagA(tag, pTag, removeFlag);
			break;
		}
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

	function getMinimumSize()
	{
		return this._addInsets(this.getFont().getTextMetrics("   "));
	}

	function parseHTML( pString )
	{
		this.lstrip(pString);
		local tokens = [];
		local component;
		this.removeAll();
		this._buildOpts();

		while (pString.len() > 0)
		{
			tokens = this.getToken(pString, this.GUI.RegExp.Tag);

			if (!this.mShowTags && tokens)
			{
				this._parseTag(tokens[0]);

				if (tokens[1].len() > 1)
				{
					pString = tokens[1].slice(1, tokens[1].len());
				}
				else
				{
					pString = "";
				}
			}
			else
			{
				tokens = this.getToken(pString, this.GUI.RegExp.NewLine);

				if (tokens)
				{
					pString = tokens[1];
				}
				else
				{
					tokens = this.getToken(pString, this.GUI.RegExp.Space);

					if (tokens)
					{
						local height = this.mOpts.font.height;
						component = this.GUI.Spacer(3, height);
						component.setVisible(true);

						if (this.mOpts)
						{
							component.setOpts(this.mOpts);
						}

						this.add(component, "NoLineStart");
						pString = tokens[1];
					}
					else
					{
						tokens = this.getToken(pString, this.GUI.RegExp.Word);

						if (tokens)
						{
							if (this.mLinkFlag)
							{
								component = this.GUI.Link(tokens[0]);
							}
							else
							{
								component = this.GUI.Label(tokens[0]);
							}

							if (this.mOpts)
							{
								component.setOpts(this.mOpts);
							}

							pString = tokens[1];
							component.setVisible(true);
							this.add(component);
						}
						else
						{
							tokens = this.getToken(pString, this.GUI.RegExp.AnyWord);

							if (tokens)
							{
								if (this.mLinkFlag)
								{
									component = this.GUI.Link(tokens[0]);
								}
								else
								{
									component = this.GUI.Label(tokens[0]);
								}

								if (this.mOpts)
								{
									component.setOpts(this.mOpts);
								}

								pString = tokens[1];
								component.setVisible(true);
								this.add(component);
							}
						}
					}
				}
			}
		}
	}

	function getToken( pString, pRegExp )
	{
		local results = pRegExp.capture(pString);

		if (!results)
		{
			return null;
		}

		local tokens = [];
		local last = 0;

		foreach( i, x in results )
		{
			if (i != 0)
			{
				tokens.append(pString.slice(x.begin, x.end));
			}

			last = x.end;
		}

		tokens.append(pString.slice(last, pString.len()));
		return tokens;
	}

	function setShowTags( pBool )
	{
		this.mShowTags = pBool;
		this.mTextChanged = true;
		this.invalidate();
	}

	function getShowTags()
	{
		return this.mShowTags;
	}

	function setFont( font )
	{
		this.GUI.Panel.setFont(font);
		this._buildOpts();
		this.mTextChanged = true;
		this.invalidate();
	}

	function setFontColor( fontColor )
	{
		this.GUI.Panel.setFontColor(fontColor);
		this._buildOpts();
		this.mTextChanged = true;
		this.invalidate();
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
		return this._stripTags(this.mText);
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
			this.validate();
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
		this.mWidget.removeListener(this);
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
				this.mScroll.onLineUp(this);
			}
			else if (evt.units_v < 0)
			{
				this.mScroll.onLineDown(this);
			}
		}

		evt.consume();
	}

	mTextChanged = false;
	mBaseFont = null;
	mStack = [];
	mOpts = {};
	mShowTags = false;
	mLinkFlag = false;
	static mClassName = "HTML";
}

