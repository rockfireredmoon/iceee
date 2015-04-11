this.require("GUI/Container");
class this.GUI.TabbedPane extends this.GUI.Container
{
	static mClassName = "TabbedPane";
	mTabs = null;
	mTabsPane = null;
	mTabPlacement = "top";
	mContentPane = null;
	mSelectedTab = null;
	mTabFontColor = null;
	mBroadcaster = null;
	mHasFrameBorder = true;
	constructor( ... )
	{
		this.GUI.Container.constructor(this.GUI.BorderLayout());
		this.mTabs = [];
		this.mContentPane = this.GUI.Panel(this.GUI.CardLayout());
		this.mContentPane.setInsets(5, 5, 5, 5);
		this.mContentPane.setAppearance("SilverBottomFrame");
		this.GUI.Container.add(this.mContentPane, this.GUI.BorderLayout.CENTER);
		this.mTabsPane = this.GUI.Container(this.GUI.TabLayout(this));
		this.mTabsPane.setInsets(0, 5, 0, 5);

		if (vargc > 0 && vargv[0])
		{
			this.mHasFrameBorder = false;
		}

		if (this.mHasFrameBorder)
		{
			this.mTabsPane.setAppearance("SilverBottomBorder");
		}

		this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.NORTH);
		this.mBroadcaster = this.MessageBroadcaster();
	}

	function addActionListener( listener )
	{
		this.mBroadcaster.addListener(listener);
	}

	function removeActionListener( listener )
	{
		this.mBroadcaster.removeListener(listener);
	}

	function setContentFrameAppearance( appearance )
	{
		this.mContentPane.setAppearance(appearance);
	}

	function setTabPlacement( placement )
	{
		local look = "Bottom";

		switch(placement.tolower())
		{
		case "north":
		case "top":
		case "default":
			this.mTabPlacement = "top";
			this.GUI.Container.remove(this.mTabsPane);
			this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.NORTH);
			this.mTabsPane.getLayoutManager().setPlacement(this.TabPlacement.NORTH);
			look = "Bottom";
			break;

		case "south":
		case "bottom":
			this.mTabPlacement = "bottom";
			this.GUI.Container.remove(this.mTabsPane);
			this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.SOUTH);
			this.mTabsPane.getLayoutManager().setPlacement(this.TabPlacement.SOUTH);
			look = "Top";
			break;

		case "east":
		case "right":
			this.mTabPlacement = "right";
			this.GUI.Container.remove(this.mTabsPane);
			this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.EAST);
			this.mTabsPane.getLayoutManager().setPlacement(this.TabPlacement.EAST);
			look = "Right";
			break;

		case "west":
		case "left":
			this.mTabPlacement = "left";
			this.GUI.Container.remove(this.mTabsPane);
			this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.WEST);
			this.mTabsPane.getLayoutManager().setPlacement(this.TabPlacement.WEST);
			this.mTabsPane.setInsets(5, 0, 5, 0);
			look = "Left";
			break;

		case "none":
			this.mTabPlacement = "none";
			this.GUI.Container.remove(this.mTabsPane);
			this.mTabsPane = null;
			break;

		default:
			throw this.Exception("Invalid tab placement: " + placement);
		}

		if (this.mHasFrameBorder)
		{
			this.mTabsPane.setAppearance("Silver" + look + "Border");
		}

		this.mContentPane.setAppearance("Silver" + look + "Frame");

		if (this.mTabsPane)
		{
			this.mTabsPane.getLayoutManager().setGap(0);
			this.mTabsPane.invalidate();
			this.mTabsPane.validate();
		}
	}

	function getTabCount()
	{
		return this.mTabs.len();
	}

	function add( component, ... )
	{
		if (vargc > 0 && typeof vargv[0] == "string")
		{
			this.addTab(vargv[0], component);
		}
		else
		{
			this.addTab(component.getName(), component);
		}
	}

	function removeAll()
	{
		this.mSelectedTab = null;
		this.mTabs = [];

		if (this.mTabsPane)
		{
			this.mTabsPane.removeAll();
		}

		this.mContentPane.removeAll();
	}

	function findArrayIndexOfTab( tab )
	{
		for( local i = 0; i < this.mTabs.len(); i++ )
		{
			if (tab == this.mTabs[i])
			{
				return i;
			}
		}

		return -1;
	}

	function getAllTabs()
	{
		return this.mTabs;
	}

	function remove( tab )
	{
		local index = this.findArrayIndexOfTab(tab);

		if (index < 0)
		{
			return;
		}

		local selectedTabIndex = this.findArrayIndexOfTab(this.mSelectedTab);
		this.mTabs.remove(index);

		if (this.mTabsPane)
		{
			this.mTabsPane.remove(tab.tabContainer);
		}

		if (selectedTabIndex == index)
		{
			if (index <= this.mTabs.len() && index != 0)
			{
				this.selectTab(index - 1);
			}
			else
			{
				this.selectTab(index);
			}
		}
	}

	function addTab( tabName, component )
	{
		return this.insertTab(tabName, component, this.mTabs.len());
	}

	function insertTab( tabName, component, index )
	{
		local tab = this._findTab(tabName);

		if (this.mTabsPane)
		{
			if (tab != null)
			{
				this.mContentPane.remove(tab.component);
				tab.component = component;
				this.mContentPane.add(component, tabName);
				return;
			}
		}

		if (index < 0)
		{
			index = 0;
		}

		if (index > this.mTabs.len())
		{
			index = this.mTabs.len();
		}

		local tabContainer = this.GUI.Container(this.GUI.BoxLayout());
		tabContainer.setInsets(-1, 0, 0, 5.5);
		local button = this.GUI.Button(tabName, this);

		if (this.mTabFontColor != null)
		{
			button.setFontColor(this.mTabFontColor);
		}

		button.setInsets(3, 12, 3, 12);
		button.setAppearance("TabBar/TabButton/Inactive/" + this.mTabPlacement);
		tabContainer.add(button);
		tab = {
			name = tabName,
			component = component,
			button = button,
			tabContainer = tabContainer,
			index = index,
			inactiveLook = "TabBar/TabButton/Inactive/" + this.mTabPlacement,
			activeLook = "TabBar/TabButton/Active/" + this.mTabPlacement
		};
		this.mTabs.insert(index, tab);

		if (this.mTabsPane)
		{
			this.mTabsPane.insert(index, tab.tabContainer);
		}

		this.mContentPane.add(component, tabName);

		if (this.mSelectedTab == null)
		{
			this.selectTab(index);
		}
	}

	function setTabBackground( appearance )
	{
		this.mTabAppearance.setAppearance(appearance);
	}

	function setTabFontColor( color )
	{
		this.mTabFontColor = color;
	}

	function setButtonAppearance( index, appearanceInactive, appearanceActive )
	{
		if (index < this.mTabs.len())
		{
			this.mTabs[index].inactiveLook = appearanceInactive;
			this.mTabs[index].activeLook = appearanceActive;

			if (this.mSelectedTab == this.mTabs[index])
			{
				this.mTabs[index].button.setAppearance(appearanceActive);
			}
			else
			{
				this.mTabs[index].button.setAppearance(appearanceInactive);
			}
		}
	}

	function setButtonContainerTooltip( index, tooltipText )
	{
		if (index < this.mTabs.len())
		{
			foreach( button in this.mTabs[index].tabContainer.components )
			{
				button.setTooltip(tooltipText);
			}
		}
	}

	function setButtonContainerAppearance( index, appearance )
	{
		if (index < this.mTabs.len())
		{
			this.mTabs[index].tabContainer.setAppearance(appearance);
		}
	}

	function setButtonContainerSize( index, width, height )
	{
		if (index < this.mTabs.len())
		{
			this.mTabs[index].tabContainer.setSize(width, height);
			this.mTabs[index].tabContainer.setPreferredSize(width, height);
		}
	}

	function setButtonContainerInset( index, top, right, bottom, left )
	{
		if (index < this.mTabs.len())
		{
			this.mTabs[index].tabContainer.setInsets(top, right, bottom, left);
		}
	}

	function setButtonSize( index, width, height )
	{
		if (index < this.mTabs.len())
		{
			this.mTabs[index].button.setFixedSize(width, height);
		}
	}

	function _findTab( tab )
	{
		if (typeof tab == "integer")
		{
			if (tab < 0 || tab >= this.mTabs.len())
			{
				return null;
			}

			return this.mTabs[tab];
		}

		if (typeof tab == "string")
		{
			local i;
			local t;

			foreach( i, t in this.mTabs )
			{
				if (t.name == tab)
				{
					return t;
				}
			}

			return null;
		}

		if (typeof tab == "instance" && (tab instanceof this.GUI.Component))
		{
			local i;
			local t;

			foreach( i, t in this.mTabs )
			{
				if (t.component == tab)
				{
					return t;
				}
			}

			return null;
		}

		throw this.Exception("Invalid tab name/index: " + tab);
	}

	function indexOfTab( tab )
	{
		local t = this._findTab(tab);
		return t == null ? -1 : t.index;
	}

	function getSelectedIndex()
	{
		return this.mSelectedTab == null ? -1 : this.mSelectedTab.index;
	}

	function getSelectedTab()
	{
		return this.mSelectedTab;
	}

	function getSelectedComponent()
	{
		return this.mSelectedTab == null ? null : this.mSelectedTab.component;
	}

	function selectTab( tab )
	{
		local t = this._findTab(tab);

		if (t == null)
		{
			this.log.warn("Cannot select tab (not found): " + tab);
			return;
		}

		if (t == this.mSelectedTab)
		{
			return;
		}

		if (this.mSelectedTab)
		{
			this.mSelectedTab.button.setAppearance(this.mSelectedTab.inactiveLook);
		}

		this.mSelectedTab = t;
		this.mContentPane.getLayoutManager().show(t.name, this.mContentPane);
		this.mSelectedTab.button.setAppearance(this.mSelectedTab.activeLook);
		this.mBroadcaster.broadcastMessage("onTabSelected", this, t);
		return t.index;
	}

	function onActionPerformed( button )
	{
		local i;
		local t;

		foreach( i, t in this.mTabs )
		{
			if (t.button == button)
			{
				local index = this.findArrayIndexOfTab(t);
				this.selectTab(index);
				return;
			}
		}
	}

	function onOpenMenu( button )
	{
		local i;
		local t;

		foreach( i, t in this.mTabs )
		{
			if (t.button == button)
			{
				this.mBroadcaster.broadcastMessage("onTabRightClicked", t);
				return;
			}
		}
	}

}

