this.require("GUI/EditArea");
this.require("GUI/SingleLineFlowLayout");
class this.GUI.InputBox extends this.GUI.EditArea
{
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.EditArea.constructor(vargv[0]);
		}
		else
		{
			this.GUI.EditArea.constructor();
		}

		this.setAppearance("TextInputFields");
		this.setLayoutManager(this.GUI.SingleLineFlowLayout());
		this.getLayoutManager().setGaps(0, 0);
		this.mStoredInputs = [];
	}

	function parseHTML( pString )
	{
		this.mHTMLUpdate = true;
		pString += " ";
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
						if (this.mPassword && pString.len() != 1)
						{
							component = this.GUI.Label("*");
						}
						else
						{
							local height = this.mOpts.font.height;
							component = this.GUI.Spacer(3, height);
						}

						component.setVisible(true);

						if (this.mOpts)
						{
							component.setOpts(this.mOpts);
						}

						this.add(component);
						pString = tokens[1];
					}
					else
					{
						tokens = this.getToken(pString, this.GUI.RegExp.Letter);

						if (tokens)
						{
							if (this.mPassword)
							{
								component = this.GUI.Label("*");
							}
							else if (this.mLinkFlag)
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
							tokens = this.getToken(pString, this.GUI.RegExp.AnyLetter);

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

	function setPassword( pBool )
	{
		this.mPassword = pBool;
		this.invalidate();
	}

	function getPassword( pBool )
	{
		return this.mPassword;
	}

	function _caretVisible()
	{
		if (!this.mCaret)
		{
			return true;
		}

		if (this.components.len() > 1)
		{
			local ci = this.getComponentIndexByParsedIndex(this.mCaretParsedIndex);
			local c = this.components[ci];

			if (!c.isVisible() && !c.getLayoutExclude())
			{
				if (this.mStartParsedIndex < this.mCaretParsedIndex)
				{
					this.mStartParsedIndex += 1;
					return false;
				}
				else if (this.mStartParsedIndex != this.mCaretParsedIndex)
				{
					this.mStartParsedIndex -= 1;
					return false;
				}
			}
		}

		return true;
	}

	function _recallUp()
	{
		if (this.mStoredIndex < 9 && this.mStoredIndex + 1 < this.mStoredInputs.len())
		{
			this.mStoredIndex++;
			this.setText(this.mStoredInputs[this.mStoredIndex]);
		}
	}

	function _recallDown()
	{
		if (this.mStoredIndex > 0 && this.mStoredIndex - 1 < this.mStoredInputs.len())
		{
			this.mStoredIndex--;
			this.setText(this.mStoredInputs[this.mStoredIndex]);
		}
	}

	function _saveInput()
	{
		this.mStoredIndex = -1;
		this.mStoredInputs.insert(0, this.mText);
		this.print(" Input saved: " + this.mText);

		while (this.mStoredInputs.len() > 10)
		{
			this.mStoredInputs.remove(10);
		}
	}

	function onKeyPressed( evt )
	{
		switch(evt.keyCode)
		{
		case ::Key.VK_DOWN:
			this._recallDown();
			evt.consume();
			break;

		case ::Key.VK_UP:
			this._recallUp();
			evt.consume();
			break;

		case ::Key.VK_ENTER:
			this._saveInput();
			this.GUI._Manager.releaseKeyboardFocus(this);
			this._fireActionPerformed("onInputComplete");
			evt.consume();
			break;

		default:
			this.GUI.EditArea.onKeyPressed(evt);
		}
	}

	mStartParsedIndex = 0;
	mPassword = false;
	mStoredIndex = 0;
	mStoredInputs = null;
	static mClassName = "InputBox";
}

