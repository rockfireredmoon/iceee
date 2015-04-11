this.require("GUI/AnchorPanel");
this.require("GUI/Panel");
class this.GUI.DropScrollButtons extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setAppearance("Container");
		this.mButtonContainer = this.GUI.Component();
		this.mButtonContainer.setLayoutManager(this.GUI.BorderLayout());
		this.add(this.mButtonContainer, this.GUI.BorderLayout.EAST);
		this.mButtonTop = this.GUI.Component();
		this.mButtonTop.setLayoutManager(this.GUI.BorderLayout());
		this.mButtonContainer.add(this.mButtonTop, this.GUI.BorderLayout.NORTH);
		this.mButtonBottom = this.GUI.Component();
		this.mButtonBottom.setLayoutManager(this.GUI.BorderLayout());
		this.mButtonContainer.add(this.mButtonBottom, this.GUI.BorderLayout.SOUTH);
		this.mPageUp = this.GUI.SmallButton("PageUp");
		this.mPageUp.setPressMessage("onPageUp");
		this.mPageUp.addActionListener(this);
		this.mPageUp.setAutoRepeat(true);
		this.mButtonTop.add(this.mPageUp, this.GUI.BorderLayout.NORTH);
		this.mLineUp = this.GUI.SmallButton("LineUp");
		this.mLineUp.setPressMessage("onLineUp");
		this.mLineUp.addActionListener(this);
		this.mLineUp.setAutoRepeat(true);
		this.mButtonTop.add(this.mLineUp, this.GUI.BorderLayout.SOUTH);
		this.mLineDown = this.GUI.SmallButton("LineDown");
		this.mLineDown.setPressMessage("onLineDown");
		this.mLineDown.addActionListener(this);
		this.mLineDown.setAutoRepeat(true);
		this.mButtonBottom.add(this.mLineDown, this.GUI.BorderLayout.NORTH);
		this.mPageDown = this.GUI.SmallButton("PageDown");
		this.mPageDown.setPressMessage("onPageDown");
		this.mPageDown.addActionListener(this);
		this.mPageDown.setAutoRepeat(true);
		this.mButtonBottom.add(this.mPageDown, this.GUI.BorderLayout.SOUTH);
		this.mMessageBroadcaster = this.MessageBroadcaster();
	}

	function onPageUp( evt )
	{
		this.mMessageBroadcaster.broadcastMessage("onPageUp", this);
	}

	function onLineUp( evt )
	{
		this.mMessageBroadcaster.broadcastMessage("onLineUp", this);
	}

	function onLineDown( evt )
	{
		this.mMessageBroadcaster.broadcastMessage("onLineDown", this);
	}

	function onPageDown( evt )
	{
		this.mMessageBroadcaster.broadcastMessage("onPageDown", this);
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	mPageUp = null;
	mLineUp = null;
	mLineDown = null;
	mPageDown = null;
	mButtonContainer = null;
	mButtonTop = null;
	mButtonBottom = null;
	static mClassName = "DropScrollButtons";
}

class this.GUI.DropDownListEntry extends this.GUI.Container
{
	constructor( owner, caption )
	{
		local layout = this.GUI.BorderLayout();
		this.GUI.Container.constructor(layout);
		this.setAppearance("Menu/Entry");
		this.setSelection(true);
		this.mSelection.setBlendColor(this.Color(0, 0, 0, 0));
		this.mChoice = caption;
		this.mLabel = this.GUI.Label();
		this.mLabel.setText(caption);
		this.mLabel.setResize(true);
		this.mLabel.setBlendColor(this.Color(1, 1, 1, 1));
		this.mLabel.setFontColor(this.Color(0.80000001, 0.80000001, 0.47799999, 1));
		this.add(this.mLabel);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.setFontColor(this.Color(1, 1, 1, 0));
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mMessageBroadcaster.addListener(owner);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
		this.mWidget.setChildProcessingEvents(false);
		this._enterFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		this._enterFrameRelay.removeListener(this);
		this.mWidget.removeListener(this);
		this.GUI.Component._removeNotify();
	}

	function onMouseEnter( evt )
	{
		if (this.mWidget != null && this.mSelection)
		{
			this.setSelectionVisible(true);
		}
	}

	function onMouseExit( evt )
	{
		if (this.mWidget != null && this.mSelection)
		{
			this.setSelectionVisible(false);
		}
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			local pos = this.getScreenPosition();
			this.mMessageBroadcaster.broadcastMessage("onSelectionPress", this.mChoice);
		}

		evt.consume();
	}

	function setBlendColor( col )
	{
		local fontcol = this.mLabel.getFontColor();
		fontcol.a = col.a;
		this.mLabel.setFontColor(fontcol);
		this.GUI.Container.setBlendColor(col);

		if (this.mSelection)
		{
			local a = col.a;
			this.mSelection.setBlendColor(this.Color(a, a, a, a));
		}
	}

	function getEnabled()
	{
		return this.mEnabled;
	}

	mLabel = null;
	mChoice = "";
	static mClassName = "DropDownListEntry";
}

class this.GUI.DropDownListRollout extends this.GUI.Panel
{
	constructor( owner )
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.mOwner = owner;
		local layout = this.GUI.BoxLayoutV();
		layout.setAlignment(0);
		layout.setExpand(true);
		this.mEntryContainer = this.GUI.Container(layout);
		this.add(this.mEntryContainer);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.setResize(true);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this._enterFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		this._enterFrameRelay.removeListener(this);
		this.GUI.Component._removeNotify();
	}

	function onOutsideClick( evt )
	{
		local pos = {
			x = evt.x,
			y = evt.y
		};

		if (this.mOwner.mDropButton.containsCursorPos(pos))
		{
			return;
		}

		this.mOwner._closeChoices(null);
	}

	function fadeIn()
	{
		this.mFadeOut = false;
		this.mFadeIn = true;
		this.mFadeOutComplete = false;
	}

	function fadeOut()
	{
		this.mFadeIn = false;
		this.mFadeOut = true;
		this.mFadeOutComplete = false;
	}

	function isFadeOutComplete()
	{
		return this.mFadeOutComplete;
	}

	function buildMenuOptions( fromcurrent )
	{
		this.mEntryContainer.removeAll();
		local MenuOptions = this.mOwner.getChoices();
		local OptionCount = MenuOptions.len();
		local MaxOptions = 16;
		local ShowButtons = OptionCount > MaxOptions;

		if (ShowButtons)
		{
			this.mMenuMaxOffset = OptionCount - MaxOptions;

			if (this.mScrollButtons == null)
			{
				this.mScrollButtons = this.GUI.DropScrollButtons();
				this.mScrollButtons.addActionListener(this);
				this.updateChildAlpha(this.mScrollButtons, this.mAlpha);
				this.add(this.mScrollButtons, this.GUI.BorderLayout.EAST);
			}
		}

		if (fromcurrent)
		{
			local current = this.mOwner.getCurrent();
			local index = 0;

			foreach( option in MenuOptions )
			{
				if (current == option)
				{
					this.mMenuOffset = index;
				}

				index++;
			}

			this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		}

		local offset = 0;
		local count = 0;
		local firstEntryRendered = false;

		foreach( option in MenuOptions )
		{
			count++;

			if (count - 1 < this.mMenuOffset)
			{
				continue;
			}

			local menuitem = this.GUI.DropDownListEntry(this.mOwner, option);
			this.updateChildAlpha(menuitem, this.mAlpha);
			this.mEntryContainer.add(menuitem, true);

			if (count - this.mMenuOffset >= MaxOptions)
			{
				break;
			}
		}
	}

	function onLineUp( button )
	{
		this.mMenuOffset--;
		this.mMenuOffset = this.mMenuOffset > 0 ? this.mMenuOffset : 0;
		this.buildMenuOptions(false);
	}

	function onLineDown( button )
	{
		this.mMenuOffset++;
		this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		this.buildMenuOptions(false);
	}

	function onPageUp( button )
	{
		this.mMenuOffset -= 10;
		this.mMenuOffset = this.mMenuOffset > 0 ? this.mMenuOffset : 0;
		this.buildMenuOptions(false);
	}

	function onPageDown( button )
	{
		this.mMenuOffset += 10;
		this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		this.buildMenuOptions(false);
	}

	function onEnterFrame()
	{
		local delta = ::_deltat / 1000.0;
		local alphachanged = false;

		if (this.mFadeIn)
		{
			this.mAlpha += delta / (this.mFadeTime / 1000.0);
			alphachanged = true;

			if (this.mAlpha > 0.99000001)
			{
				this.mAlpha = 1;
				this.mFadeIn = false;
			}
		}

		if (this.mFadeOut)
		{
			this.mAlpha -= delta / (this.mFadeTime / 1000.0);
			alphachanged = true;

			if (this.mAlpha < 0.0099999998)
			{
				this.mAlpha = 0;
				this.mFadeOut = false;
				this.mFadeOutComplete = true;
			}
		}

		if (alphachanged)
		{
			this.setBlendColor(this.Color(1, 1, 1, this.mAlpha));

			foreach( c in this.components )
			{
				this.updateChildAlpha(c, this.mAlpha);
			}
		}
	}

	function updateChildAlpha( child, alpha )
	{
		local col = child.getBlendColor();
		col.a = alpha;
		child.setBlendColor(col);

		foreach( c in child.components )
		{
			this.updateChildAlpha(c, alpha);
		}
	}

	mAlpha = 0;
	mFadeIn = true;
	mFadeOut = false;
	mFadeOutComplete = false;
	mFadeTime = 150;
	mOwner = null;
	mMenuOffset = 0;
	mMenuMaxOffset = 0;
	mEntryContainer = null;
	mScrollButtons = null;
	static mClassName = "DropDownListRollout";
}

class this.GUI.DropDownList extends this.GUI.Panel
{
	static mClassName = "DropDownList";
	mCurrentLabel = null;
	mDropButton = null;
	mChoicePanel = null;
	mChoices = null;
	mChangeMessage = null;
	constructor( ... )
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.setInsets(1);
		this.mChoices = [];
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mDropButton = this.GUI.SmallButton("LineDown");
		this.mDropButton.setPressMessage("onDropDownButtonPress");
		this.mCurrentLabel = this.GUI.Label("");
		this.mCurrentLabel.setInsets(4);
		this.mCurrentLabel.setTextAlignment(0.0, 0.5);
		this.add(this.mCurrentLabel, this.GUI.BorderLayout.CENTER);
		local p = this.GUI.Container(this.GUI.GridLayout(1, 1));
		p.add(this.mDropButton, 0);
		this.add(p, this.GUI.BorderLayout.EAST);
		this.mChangeMessage = "onSelectionChange";
		this.mDropButton.addActionListener(this);

		if (vargc > 0)
		{
			if (typeof vargv[0] == "table" || typeof vargv[0] == "instance")
			{
				this.addActionListener(vargv[0]);
			}
		}
	}

	function _addNotify()
	{
		this.GUI.Panel._addNotify();
		this._enterFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		this._enterFrameRelay.removeListener(this);
		this.GUI.Panel._removeNotify();

		if (this.mChoicePanel)
		{
			this.mChoicePanel.setOverlay(null);
			this.mChoicePanel = null;
		}
	}

	function _closeChoices( ch )
	{
		if (ch != null)
		{
			this.setCurrent(ch);
		}

		if (this.mChoicePanel)
		{
			this.mChoicePanel.fadeOut();
		}

		this._fireActionPerformed("onRolloutClosed");
	}

	function _openChoices()
	{
		if (this.mChoicePanel)
		{
			return;
		}

		this.mChoicePanel = ::GUI.DropDownListRollout(this);
		this.mChoicePanel.buildMenuOptions(true);
		this.mChoicePanel.validate();
		local size = this.mChoicePanel.getSize();
		size.width = this.getWidth();
		this.mChoicePanel.setResize(false);
		this.mChoicePanel.setSize(size);
		local pos = this.getScreenPosition();
		pos.y += this.getHeight() - 1;
		pos.x = pos.x >= 0 ? pos.x : 0;
		pos.y = pos.y >= 0 ? pos.y : 0;

		if (pos.x + this.mChoicePanel.getWidth() > ::Screen.getWidth())
		{
			pos.x = ::Screen.getWidth() - this.mChoicePanel.getWidth();
		}

		if (pos.y + this.mChoicePanel.getHeight() > ::Screen.getHeight())
		{
			pos.y = ::Screen.getHeight() - this.mChoicePanel.getHeight();
		}

		this.mChoicePanel.setPosition(pos);
		this.GUI._Manager.addTransientToplevel(this.mChoicePanel);
		this.mChoicePanel.setOverlay(this.GUI.POPUP_OVERLAY);
	}

	function getPreferredSize()
	{
		local prefSize = this.GUI.Panel.getPreferredSize();
		local font = this.getFont();
		local maxLen = 0;
		local choice;

		foreach( choice in this.mChoices )
		{
			maxLen = this.Math.max(maxLen, font.getTextMetrics(choice).width);
		}

		prefSize.width = maxLen + this.insets.left + this.insets.right + this.mCurrentLabel.insets.left + this.mCurrentLabel.insets.right + this.mDropButton.getPreferredSize().width;
		return prefSize;
	}

	function getCurrent()
	{
		return this.mCurrentLabel.getText();
	}

	function setCurrent( cur, ... )
	{
		if (cur != this.mCurrentLabel.getText())
		{
			local found = false;

			foreach( x in this.mChoices )
			{
				if (x == cur)
				{
					found = true;
				}
			}

			if (found)
			{
				this.mCurrentLabel.setText(cur);

				if (vargc == 0 || vargv[0])
				{
					this._fireActionPerformed(this.mChangeMessage);
				}
			}
		}
	}

	function setValue( text )
	{
		this.setCurrent(text.tostring());
	}

	function getCurrentIndex()
	{
		local cur = this.getCurrent();

		for( local x = 0; x < this.mChoices.len(); x++ )
		{
			if (this.mChoices[x] == cur)
			{
				return x;
			}
		}

		return -1;
	}

	function removeAll()
	{
		this.mCurrentLabel.setText("");
		this.mChoices.resize(0);
	}

	function addChoice( name )
	{
		if (typeof name != "string")
		{
			throw this.Exception("Invalid choice: " + name);
		}

		this.mChoices.append(name);

		if (this.mChoices.len() == 1)
		{
			this.setCurrent(this.mChoices[0], false);
		}
	}

	function getChoices()
	{
		return this.mChoices;
	}

	function setChoices( choices, ... )
	{
		if (typeof choices != "array")
		{
			throw this.Exception("Illegal argument: should be an array");
		}

		local oldChoice = this.getCurrent();
		local haveOldChoice = false;
		this.removeAll();

		foreach( choice in choices )
		{
			if (choice == oldChoice)
			{
				haveOldChoice = true;
			}

			this.addChoice(choice);
		}

		if (haveOldChoice)
		{
			this.setCurrent(oldChoice, vargc == 0 || vargv[0]);
			return true;
		}

		if (choices.len() > 0)
		{
			this.setCurrent(choices[0], vargc == 0 || vargv[0]);
		}

		return false;
	}

	function onEnterFrame()
	{
		if (this.mChoicePanel && this.mChoicePanel.isFadeOutComplete())
		{
			this.mChoicePanel.setOverlay(null);
			this.mChoicePanel = null;
		}
	}

	function onSelectionPress( text )
	{
		this._closeChoices(text);
	}

	function onDropDownButtonPress( button )
	{
		if (this.mChoicePanel == null)
		{
			this._openChoices();
		}
		else
		{
			this._closeChoices(null);
		}
	}

	function addSelectionChangeListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeSelectionChangeListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage)
		{
			this.mMessageBroadcaster.broadcastMessage(pMessage, this);
		}
	}

	function setChangeMessage( pString )
	{
		this.mChangeMessage = pString;
	}

	function getChangeMessage()
	{
		return this.mChangeMessage;
	}

	function _debugstring()
	{
		return this.GUI.Panel._debugstring() + "";
	}

}

