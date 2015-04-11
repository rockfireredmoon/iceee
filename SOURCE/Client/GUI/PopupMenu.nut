this.require("GUI/Panel");
class this.GUI.PopupMenuArrow extends this.GUI.Component
{
	constructor()
	{
		this.GUI.Component.constructor();
		this.setAppearance("Menu/Arrow");
		this.setMinimumSize(4, 5);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.setResize(false);
	}

	static mClassName = "PopupMenuArrow";
}

class this.GUI.PopupMenuEntry extends this.GUI.Container
{
	constructor( owner, menuid, caption, haschildren, enabled )
	{
		local layout = this.GUI.BorderLayout();
		this.GUI.Container.constructor(layout);
		this.setAppearance("Menu/Entry");
		this.mMenuID = menuid;
		this.mEnabled = enabled;
		this.setSelection(true);
		this.mSelection.setBlendColor(this.Color(0, 0, 0, 0));
		this.mLabel = this.GUI.Label();
		this.mLabel.setText(caption + "  ");
		this.mLabel.setEnabled(enabled);
		this.mLabel.setResize(true);
		this.mLabel.setBlendColor(this.Color(1, 1, 1, 0));
		this.mLabel.setFont(::GUI.Font("Maiandra", 20));

		if (this.mEnabled)
		{
			this.mLabel.setFontColor(this.Color(0.80000001, 0.80000001, 0.47799999, 0));
		}
		else
		{
			this.mLabel.setFontColor(this.Color(0.40000001, 0.40000001, 0.234, 0));
		}

		this.add(this.mLabel);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.setFontColor(this.Color(1, 1, 1, 0));

		if (haschildren)
		{
			local arrowlayout = this.GUI.BoxLayout();
			arrowlayout.setAlignment(0.5);
			local ArrowContainer = this.GUI.Container(arrowlayout);
			this.add(ArrowContainer, this.GUI.BorderLayout.EAST);
			this.mArrow = this.GUI.PopupMenuArrow();

			if (this.mEnabled)
			{
				this.mArrow.setBlendColor(this.Color(1, 1, 1, 0));
			}
			else
			{
				this.mArrow.setBlendColor(this.Color(0.5, 0.5, 0.5, 0));
			}

			ArrowContainer.add(this.mArrow);
		}

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

		this.mTimer = ::Timer();
		this.mTimer.reset();
	}

	function onMouseExit( evt )
	{
		this.mTimer = null;

		if (this.mWidget != null && this.mSelection)
		{
			this.setSelectionVisible(false);
		}
	}

	function onMousePressed( evt )
	{
		if (this.mEnabled == false)
		{
			return;
		}

		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			local pos = this.getScreenPosition();
			this.mMessageBroadcaster.broadcastMessage("onMenuItemPressed", this.mMenuID, pos.x, pos.y, this.getWidth(), this.getHeight());
		}

		evt.consume();
	}

	function setBlendColor( col )
	{
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

	function onEnterFrame()
	{
		if (this.mTimer && this.mTimer.getMilliseconds() > 200)
		{
			local pos = this.getScreenPosition();
			this.mMessageBroadcaster.broadcastMessage("onMenuItemHovered", this.mMenuID, pos.x, pos.y, this.getWidth(), this.getHeight());
			this.mTimer = null;
		}
	}

	mLabel = null;
	mEnabled = true;
	mMenuID = "";
	mArrow = null;
	mTimer = null;
	static mClassName = "PopupMenuEntry";
}

class this.GUI.MenuPanel extends this.GUI.Panel
{
	constructor( owner, menuid )
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.mMenuID = menuid;
		this.mOwner = owner;
		local layout = this.GUI.BoxLayoutV();
		layout.setAlignment(0);
		layout.setExpand(true);
		this.mEntryContainer = this.GUI.Container(layout);
		this.add(this.mEntryContainer);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.setResize(false);
	}

	function onOutsideClick( evt )
	{
		this.mOwner.closeAllMenus();
	}

	function getMenuID()
	{
		return this.mMenuID;
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

	function buildMenuOptions()
	{
		this.mEntryContainer.removeAll();
		local MenuOptions = this.mOwner.getMenuOptions(this.mMenuID);
		local OptionCount = MenuOptions.len();

		foreach( option in MenuOptions )
		{
			if (option.caption == "-")
			{
				OptionCount--;
			}
		}

		local MaxOptions = (::Screen.getHeight() - 35) / 18;
		local ShowButtons = OptionCount > MaxOptions;

		if (ShowButtons)
		{
			MaxOptions -= 3;
			this.mMenuMaxOffset = OptionCount - MaxOptions - 1;
		}

		if (ShowButtons && this.mUpButton == null)
		{
			this.mUpButton = this.GUI.Button("");
			this.mUpButton.setPressMessage("onScrollUp");
			this.mUpButton.setSize(10, 5);
			this.mUpButton.addActionListener(this);
			this.mUpButton.setAutoRepeat(true);
			this.mUpButton.setAutoRepeatDelay(100);
			this.add(this.mUpButton, this.GUI.BorderLayout.NORTH);
			this.mDownButton = this.GUI.Button("");
			this.mDownButton.setPressMessage("onScrollDown");
			this.mDownButton.setSize(10, 5);
			this.mDownButton.addActionListener(this);
			this.mDownButton.setAutoRepeat(true);
			this.mDownButton.setAutoRepeatDelay(100);
			this.add(this.mDownButton, this.GUI.BorderLayout.SOUTH);
		}

		local offset = 0;
		local count = 0;
		local firstEntryRendered = false;

		foreach( option in MenuOptions )
		{
			if (option.caption != "-")
			{
				count++;
			}

			if (count - 1 < this.mMenuOffset)
			{
				continue;
			}

			if (option.visible)
			{
				local menuitem;

				if (option.caption == "-")
				{
					if (firstEntryRendered == false)
					{
						continue;
					}

					menuitem = this.GUI.PopupMenuDivider(this);
				}
				else
				{
					menuitem = this.GUI.PopupMenuEntry(this.mOwner, option.menuid, option.caption, this.mOwner.isSubMenu(option.menuid), option.enabled);
					firstEntryRendered = true;
				}

				this.updateChildAlpha(menuitem, this.mAlpha);
				this.mEntryContainer.add(menuitem, true);
			}

			if (count - this.mMenuOffset > MaxOptions)
			{
				break;
			}
		}
	}

	function onScrollUp( button )
	{
		local sticky = false;

		if (this.getHeight() + this.getPosition().y >= ::Screen.getHeight() - 1)
		{
			sticky = true;
		}

		this.mMenuOffset--;
		this.mMenuOffset = this.mMenuOffset > 0 ? this.mMenuOffset : 0;
		this.buildMenuOptions();

		if (sticky)
		{
			this.setPosition(this.getPosition().x, ::Screen.getHeight() - this.getHeight());
		}
	}

	function onScrollDown( button )
	{
		local sticky = false;

		if (this.getHeight() + this.getPosition().y >= ::Screen.getHeight() - 1)
		{
			sticky = true;
		}

		this.mMenuOffset++;
		this.mMenuOffset = this.mMenuOffset < this.mMenuMaxOffset ? this.mMenuOffset : this.mMenuMaxOffset;
		this.buildMenuOptions();

		if (sticky)
		{
			this.setPosition(this.getPosition().x, ::Screen.getHeight() - this.getHeight());
		}
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

	function getPreferredSize()
	{
		local size = this.GUI.Panel.getPreferredSize();
		size.width = size.width > 200 ? size.width : 200;
		return size;
	}

	function getMinimumSize()
	{
		local size = this.GUI.Panel.getPreferredSize();
		size.width = size.width > 200 ? size.width : 200;
		return size;
	}

	function updateChildAlpha( child, alpha )
	{
		local col = child.getBlendColor();
		col.a = alpha;
		child.setBlendColor(col);
		col = child.getFontColor();
		col.a = alpha;
		child.setFontColor(col);

		foreach( c in child.components )
		{
			this.updateChildAlpha(c, alpha);
		}
	}

	mAlpha = 0;
	mFadeIn = true;
	mFadeOut = false;
	mFadeOutComplete = false;
	mFadeTime = 100;
	mOwner = null;
	mMenuID = "";
	mMenuOffset = 0;
	mMenuMaxOffset = 0;
	mEntryContainer = null;
	mUpButton = null;
	mDownButton = null;
	static mClassName = "MenuPanel";
}

class this.GUI.PopupMenuDivider extends this.GUI.Component
{
	constructor( menuid )
	{
		this.GUI.Component.constructor();
		this.setAppearance("Menu/Divider");
		this.setMinimumSize(80, 1);
		this.setBlendColor(this.Color(1, 1, 1, 0));
		this.mMenuID = menuid;
	}

	function getMenuID()
	{
		return this.mMenuID;
	}

	mMenuID = "";
	static mClassName = "PopupMenuDivider";
}

class this.GUI.PopupMenu 
{
	constructor( ... )
	{
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mMenuOptions = [];
		this.mMenuPanels = [];
		this._enterFrameRelay.addListener(this);
		this.mMenuOptions = [];
		this.mMenuPanels = [];
	}

	function destroy()
	{
		this._enterFrameRelay.removeListener(this);

		foreach( m in this.mMenuPanels )
		{
			m.destroy();
		}
	}

	function getMenuRoot( menuid )
	{
		local parts = this.split(menuid, ".");
		local root = "";

		for( local i = 0; i < parts.len() - 1; i++ )
		{
			root += (root != "" ? "." : "") + parts[i];
		}

		return root;
	}

	function addMenuOption( newID, newCaption, ... )
	{
		local newCallback = "";
		local newVisible = true;
		local newEnabled = true;

		if (vargc > 0)
		{
			newCallback = vargv[0];
		}

		if (vargc > 1)
		{
			newVisible = vargv[1];
		}

		if (vargc > 2)
		{
			newEnabled = vargv[2];
		}

		local newRoot = this.getMenuRoot(newID);
		this.mMenuOptions.append({
			menuid = newID,
			root = newRoot,
			caption = newCaption,
			callback = newCallback,
			visible = newVisible,
			enabled = newEnabled,
			component = null
		});
	}

	function setMenuOption( id, newCaption, ... )
	{
		local newCallback = "";
		local newVisible = true;
		local newEnabled = true;

		for( local i = 0; i < this.mMenuOptions.len(); i++ )
		{
			if (this.mMenuOptions[i].menuid == id)
			{
				this.mMenuOptions[i].caption = newCaption;

				if (vargc > 0)
				{
					this.mMenuOptions[i].callback = vargv[0];
				}

				if (vargc > 1)
				{
					this.mMenuOptions[i].visible = vargv[1];
				}

				if (vargc > 2)
				{
					this.mMenuOptions[i].enabled = vargv[2];
				}

				break;
			}
		}
	}

	function getMenuOption( id )
	{
		for( local i = 0; i < this.mMenuOptions.len(); i++ )
		{
			if (this.mMenuOptions[i].menuid == id)
			{
				return this.mMenuOptions[i];
			}
		}
	}

	function removeMenuChildren( id )
	{
		local options = this.getMenuOptions(id);

		foreach( option in options )
		{
			this.removeMenuOption(option.menuid);
		}
	}

	function removeMenuOption( id )
	{
		this.removeMenuChildren(id);

		for( local i = 0; i < this.mMenuOptions.len(); i++ )
		{
			if (this.mMenuOptions[i].menuid == id)
			{
				this.mMenuOptions.remove(i);
				break;
			}
		}
	}

	function removeAllMenuOptions()
	{
		this.mMenuOptions = [];
	}

	function buildMenuPanel( rootid )
	{
		local panel = this.GUI.MenuPanel(this, rootid);
		panel.setResize(true);
		panel.buildMenuOptions();
		this.GUI._Manager.addTransientToplevel(panel);
		this.mMenuPanels.append(panel);
		panel.validate();
		return panel;
	}

	function getMenuOptions( menuid )
	{
		local MenuOptions = [];

		foreach( option in this.mMenuOptions )
		{
			if (option.root == menuid)
			{
				MenuOptions.append(option);
			}
		}

		return MenuOptions;
	}

	function getAllMenuOptions()
	{
		return this.mMenuOptions;
	}

	function createJoinedMenu( menuTop, menuBottom, divider )
	{
		local menuTopOptions = menuTop.getAllMenuOptions();
		local menuBottomOptions = menuBottom.getAllMenuOptions();
		local joinedMenu = this.GUI.PopupMenu();

		foreach( option in menuTopOptions )
		{
			joinedMenu.addMenuOption(option.menuid, option.caption);
		}

		if (divider == true)
		{
			joinedMenu.addMenuOption("-", "-");
		}

		foreach( option in menuBottomOptions )
		{
			joinedMenu.addMenuOption(option.menuid, option.caption);
		}

		return joinedMenu;
	}

	function showMenu()
	{
		local cursorPos = this.Screen.getCursorPos();
		local panel = this.buildMenuPanel("");
		local fx = cursorPos.x;
		local fy = cursorPos.y;
		fx = fx >= 0 ? fx : 0;
		fy = fy >= 0 ? fy : 0;

		if (fx + panel.getWidth() > ::Screen.getWidth())
		{
			fx = ::Screen.getWidth() - panel.getWidth();
		}

		if (fy + panel.getHeight() > ::Screen.getHeight())
		{
			fy = ::Screen.getHeight() - panel.getHeight();
		}

		panel.setPosition(fx, fy);
		panel.setOverlay(this.GUI.POPUP_OVERLAY);
	}

	function isSubMenu( id )
	{
		local submenu = false;

		foreach( option in this.mMenuOptions )
		{
			if (option.root == id)
			{
				submenu = true;
			}
		}

		return submenu;
	}

	function onMenuItemPressed( menuid, x, y, width, height )
	{
		local callback = this.getMenuOption(menuid).callback;
		local submenu = this.isSubMenu(menuid);

		if (callback != "")
		{
			this.mMessageBroadcaster.broadcastMessage(callback, this);
		}
		else if (submenu)
		{
			this.mMessageBroadcaster.broadcastMessage("onSubMenuPressed", this, menuid);
		}
		else
		{
			this.mMessageBroadcaster.broadcastMessage("onMenuItemPressed", this, menuid);
		}

		if (submenu)
		{
			this.closeOtherSubMenus(menuid);

			if (this.isMenuOpen(menuid) == false)
			{
				local panel = this.buildMenuPanel(menuid);
				local fx = x + width + 3;
				local fy = y - 5;

				if (fx + panel.getWidth() > ::Screen.getWidth())
				{
					fx = x - panel.getWidth() - 3;
				}

				if (fy + panel.getHeight() > ::Screen.getHeight())
				{
					fy = ::Screen.getHeight() - panel.getHeight();
				}

				fx = fx >= 0 ? fx : 0;
				fy = fy >= 0 ? fy : 0;
				panel.setPosition(fx, fy);
				panel.setOverlay(this.GUI.POPUP_OVERLAY);
			}
		}
		else
		{
			this.closeAllMenus();
		}
	}

	function onMenuItemHovered( menuid, x, y, width, height )
	{
		local callback = this.getMenuOption(menuid).callback;
		local submenu = this.isSubMenu(menuid);

		if (callback != "" && submenu)
		{
			this.mMessageBroadcaster.broadcastMessage(callback, this);
		}
		else if (submenu)
		{
			this.mMessageBroadcaster.broadcastMessage("onSubMenuPressed", this, menuid);
		}

		if (submenu)
		{
			this.closeOtherSubMenus(menuid);

			if (this.isMenuOpen(menuid) == false)
			{
				local panel = this.buildMenuPanel(menuid);
				local fx = x + width + 3;
				local fy = y - 5;

				if (fx + panel.getWidth() > ::Screen.getWidth())
				{
					fx = x - panel.getWidth() - 3;
				}

				if (fy + panel.getHeight() > ::Screen.getHeight())
				{
					fy = ::Screen.getHeight() - panel.getHeight();
				}

				fx = fx >= 0 ? fx : 0;
				fy = fy >= 0 ? fy : 0;
				panel.setPosition(fx, fy);
				panel.setOverlay(this.GUI.POPUP_OVERLAY);
			}
		}
	}

	function isMenuOpen( menuid )
	{
		foreach( m in this.mMenuPanels )
		{
			if (m.getMenuID() == menuid)
			{
				return true;
			}
		}

		return false;
	}

	function isParentMenu( parentid, childid )
	{
		local isparent = false;
		local parentparts = this.split(parentid, ".");
		local childparts = this.split(childid, ".");

		if (parentparts.len() > childparts.len())
		{
			return false;
		}

		for( local i = 0; i < parentparts.len(); i++ )
		{
			if (childparts[i] != parentparts[i])
			{
				return false;
			}
		}

		return true;
	}

	function closeOtherSubMenus( menuid )
	{
		for( local i = 0; i < this.mMenuPanels.len();  )
		{
			local thisid = this.mMenuPanels[i].getMenuID();

			if (this.isParentMenu(thisid, menuid) == false)
			{
				this.mMenuPanels[i].fadeOut();
				i++;
			}
			else
			{
				i++;
			}
		}
	}

	function closeAllMenus()
	{
		foreach( el in this.mMenuPanels )
		{
			el.fadeOut();
		}
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mMessageBroadcaster.removeListener(listener);
	}

	function onEnterFrame()
	{
		local newPanels = [];

		foreach( p in this.mMenuPanels )
		{
			p.onEnterFrame();

			if (!p.isFadeOutComplete())
			{
				newPanels.append(p);
			}
			else
			{
				p.destroy();
			}
		}

		this.mMenuPanels = newPanels;
	}

	function setData( data )
	{
		this.mData = data;
	}

	function getData()
	{
		return this.mData;
	}

	mMessageBroadcaster = null;
	mMenuOptions = [];
	mMenuPanels = [];
	mData = null;
	static mClassName = "PopupMenu";
}

