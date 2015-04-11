class this.GUI.SimpleListEntry extends this.GUI.Container
{
	mLabel = null;
	mChoice = "";
	mOwner = null;
	static mClassName = "SimpleListEntry";
	constructor( owner, caption )
	{
		local layout = this.GUI.BoxLayoutV();
		this.GUI.Container.constructor(layout);
		this.setAppearance("Menu/Entry");
		this.setSelection(true);
		this.mChoice = caption;
		this.mOwner = owner;
		this.mLabel = this.GUI.Label();
		this.mLabel.setText(caption);
		this.mLabel.setResize(true);
		this.add(this.mLabel);
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

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			this.mOwner.setCurrent(this.mChoice);
		}

		evt.consume();
	}

	function getText()
	{
		return this.mChoice;
	}

}

class this.GUI.SimpleList extends this.GUI.Panel
{
	mMenuOffset = 0;
	mMenuMaxOffset = 0;
	mEntryContainer = null;
	mScrollButtons = null;
	mChoices = null;
	mEntries = null;
	mChangeMessage = null;
	mCurrent = null;
	mAlignment = 0;
	static mClassName = "SimpleList";
	constructor()
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		local layout = this.GUI.BoxLayoutV();
		layout.setAlignment(0);
		layout.setGap(0);
		layout.setExpand(true);
		this.setInsets(0, 0, 0, 0);
		this.mEntryContainer = this.GUI.Container(layout);
		this.add(this.mEntryContainer);
		this.mEntries = [];
		this.mChoices = [];
		this.setResize(true);
		this.mMessageBroadcaster = this.MessageBroadcaster();
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

	function buildMenuOptions()
	{
		this.mEntryContainer.removeAll();
		this.mEntries.clear();
		local OptionCount = this.mChoices.len();
		local MaxOptions = 16;
		local ShowButtons = OptionCount > MaxOptions;

		if (ShowButtons)
		{
			this.mMenuMaxOffset = OptionCount - MaxOptions;

			if (this.mScrollButtons == null)
			{
				this.mScrollButtons = this.GUI.DropScrollButtons();
				this.mScrollButtons.addActionListener(this);
				this.add(this.mScrollButtons, this.GUI.BorderLayout.EAST);
			}
		}

		local offset = 0;
		local count = 0;
		local firstEntryRendered = false;

		foreach( option in this.mChoices )
		{
			count++;

			if (count - 1 < this.mMenuOffset)
			{
				continue;
			}

			local menuitem = this.GUI.SimpleListEntry(this, option);
			menuitem.getLayoutManager().setAlignment(this.mAlignment);
			this.mEntryContainer.add(menuitem, true);
			this.mEntries.append(menuitem);

			if (count - this.mMenuOffset >= MaxOptions)
			{
				break;
			}
		}
	}

	function setAlignment( align )
	{
		this.mAlignment = align;

		foreach( x in this.mEntries )
		{
			x.getLayoutManager().setAlignment(align);
		}
	}

	function onLineUp( button )
	{
		this.mMenuOffset--;
		this.mMenuOffset = this.mMenuOffset > 0 ? this.mMenuOffset : 0;
		this.buildMenuOptions();
	}

	function onLineDown( button )
	{
		this.mMenuOffset++;
		this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		this.buildMenuOptions();
	}

	function onPageUp( button )
	{
		this.mMenuOffset -= 10;
		this.mMenuOffset = this.mMenuOffset > 0 ? this.mMenuOffset : 0;
		this.buildMenuOptions();
	}

	function onPageDown( button )
	{
		this.mMenuOffset += 10;
		this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		this.buildMenuOptions();
	}

	function getCurrent()
	{
		return this.mCurrent;
	}

	function setCurrent( cur, ... )
	{
		local found = false;

		foreach( i, x in this.mChoices )
		{
			if (x == cur)
			{
				this.mCurrent = cur;

				if (vargc == 0 || vargv[0])
				{
					this._fireActionPerformed(this.mChangeMessage);
				}

				break;
			}
		}

		foreach( x in this.mEntries )
		{
			x.setSelectionVisible(x.getText() == cur);
		}
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
		this.mCurrent = null;
		this.mChoices.resize(0);
		this.buildMenuOptions();
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

		this.buildMenuOptions();
	}

	function prependChoice( name )
	{
		if (typeof name != "string")
		{
			throw this.Exception("Invalid choice: " + name);
		}

		this.mChoices.insert(0, name);

		if (this.mChoices.len() == 1)
		{
			this.setCurrent(this.mChoices[0], false);
		}

		this.buildMenuOptions();
	}

	function getChoices()
	{
		return this.mChoices;
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

}

