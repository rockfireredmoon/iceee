this.require("GUI/TabbedPane");
class this.GUI.EEFrame extends this.GUI.Container
{
	constructor( title )
	{
		this.GUI.Container.constructor(this.GUI.BorderLayout());
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.mWindowFrame = this.GUI.Panel(this.GUI.BorderLayout());
		this.mWindowFrame.setInsets(0);
		this.GUI.Container.add(this.mWindowFrame);
		this.mCloseButton = this.GUI.Button("X");
		this.mCloseButton.setInsets(1, 4, 1, 4);
		this.mCloseButton.addActionListener({
			frame = this,
			function onActionPerformed( button )
			{
				this.frame.onClosePressed();
			}

		});
		this.mTitle = this.GUI.Label(title);
		this.mTitleBar = this.GUI.Panel(this.GUI.BorderLayout());
		this.mTitleBar.mDebugName = "TitleBar";
		this.mTitleBar.setInsets(2, 2, 2, 7);
		this.mTitleBar.add(this.mTitle, this.GUI.BorderLayout.CENTER);
		this.mTitleBar.add(this.mCloseButton, this.GUI.BorderLayout.EAST);
		this.mWindowFrame.add(this.mTitleBar, this.GUI.BorderLayout.NORTH);
		this.mContentPane = this.GUI.Container(this.GUI.CardLayout());
		this.mContentPane.setInsets(5, 5, 5, 5);
		this.mWindowFrame.add(this.mContentPane, this.GUI.BorderLayout.CENTER);
		this.mTabsPane = this.GUI.Container(this.GUI.TabLayout(this));
		this.mTabsPane.setInsets(0, 5, 0, 5);
		this.GUI.Container.add(this.mTabsPane, this.GUI.BorderLayout.SOUTH);
		this.mTabs = [];
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function setTitle( title )
	{
		this.mTitle.setText(title);
	}

	function setIcon( material, width, height )
	{
		if (material != null)
		{
			this.mIcon = this.GUI.Component();
			this.mIcon.setAppearance("Icon");
			this.mIcon.setMaterial(material);
			this.mIcon.setSize(width, height);
			this.mIcon.setPosition(0, 25 - height);
			this.mIcon.setLayoutExclude(true);
			this.mTitleBar.add(this.mIcon);
			this.mTitleBar.setInsets(2, 2, 2, width + 5);
		}
		else if (this.mIcon != null)
		{
			this.mTitleBar.remove(this.mIcon);
			this.mTitleBar.setInsets(2, 2, 2, 7);
			this.mIcon.destroy();
			this.mIcon = null;
			this.mIconWidth = 0;
			this.mIconHeight = 0;
		}
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mTitleBar.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mTitleBar.mWidget != null)
		{
			this.mTitleBar.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setCloseMessage( msg )
	{
		this.mCloseMessage = msg;
	}

	function addActionListener( obj )
	{
		this.mMessageBroadcaster.addListener(obj);
	}

	function removeActionListener( obj )
	{
		this.mMessageBroadcaster.removeListener(obj);
	}

	function close()
	{
		this.setVisible(false);
	}

	function onClosePressed()
	{
		this.mMessageBroadcaster.broadcastMessage(this.mCloseMessage);
		this.close();
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

	function addOutsideTabs( component, ... )
	{
		if (vargc > 0 && typeof vargv[0] == "string")
		{
			this.mWindowFrame.add(component, vargv[0]);
		}
		else
		{
			this.mWindowFrame.add(component);
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

	function remove( component )
	{
		local index = this.indexOfComponent(component);

		if (index < 0)
		{
			return;
		}

		this.mTabs.remove(index);

		if (this.mTabsPane)
		{
			this.mTabsPane.remove(index);
		}

		if (this.getSelectedIndex() == index)
		{
			this.selectTab(index);
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

		local button = this.GUI.Button(tabName, this);
		button.setInsets(3, 12, 3, 12);
		button.setAppearance("TabBar/TabButton/Inactive/" + this.mTabPlacement);
		tab = {
			name = tabName,
			component = component,
			button = button,
			index = index
		};
		this.mTabs.insert(index, tab);

		if (this.mTabsPane)
		{
			this.mTabsPane.insert(index, tab.button);
		}

		this.mContentPane.add(component, tabName);

		if (this.mSelectedTab == null)
		{
			this.selectTab(index);
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
			this.mSelectedTab.button.setAppearance("TabBar/TabButton/Inactive/" + this.mTabPlacement);
		}

		this.mSelectedTab = t;
		this.mContentPane.getLayoutManager().show(t.name, this.mContentPane);
		this.mSelectedTab.button.setAppearance("TabBar/TabButton/Active/" + this.mTabPlacement);
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
				this.selectTab(t.index);
				return;
			}
		}
	}

	mIcon = null;
	mTabBar = null;
	mWindowFrame = null;
	mTitle = null;
	mTitleBar = null;
	mContentPane = null;
	mCloseButton = null;
	mTabs = null;
	mTabsPane = null;
	mTabPlacement = "bottom";
	mContentPane = null;
	mSelectedTab = null;
	mCloseMessage = "onFrameClosed";
	static mClassName = "EEFrame";
}

