this.require("UI/Screens");
this.require("UI/MiniMapScreen");
class this.Screens.MiniMapLocation extends this.GUI.Component
{
	static mClassName = "MiniMapLocation";
	mLocationLabel = null;
	mPopupMenu = null;
	mShardList = null;
	mLocation = "";
	mQuerying = false;
	mShard = "";
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.BorderLayout());
		this.setSize(179, 29);
		this.setPreferredSize(179, 29);
		this.setAppearance("MiniMapLocation");
		this.mLocationLabel = this.GUI.Label("");
		this.mLocationLabel.setSize(179, 20);
		this.mLocationLabel.setPreferredSize(179, 20);
		this.mLocationLabel.setTextAlignment(0.5, 0.30000001);
		this.add(this.mLocationLabel, this.GUI.BorderLayout.CENTER);
		this.setSticky("right", "top");
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();

		if (!this.mWidget)
		{
			this.mWidget.addListener(this);
		}

		local miniMapScreen = this.Screens.get("MiniMapScreen", false);

		if (miniMapScreen)
		{
			this.setPosition(-miniMapScreen.getWidth() - 10, 4);
		}

		this.setOverlay("GUI/MiniMap");
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function setLocationText( location )
	{
		this.mLocation = location;
		this.mLocationLabel.setText(this.mLocation + (this.mShard != "" ? "  (" + this.mShard + ")" : ""));
	}

	function onMenuItemPressed( menu, menuID )
	{
		::_Connection.sendQuery("shard.set", this, [
			menuID
		]);
	}

	function setLocationShard( shard )
	{
		if (this.mShard != shard)
		{
			this.mShard = shard;
			this.mLocationLabel.setText(this.mLocation + (this.mShard != "" ? "  (" + this.mShard + ")" : ""));

			if (this.mPopupMenu)
			{
				this.mPopupMenu.removeActionListener(this);
				this.mPopupMenu = null;
			}

			this.mQuerying = false;
		}
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "shard.list")
		{
			this.mQuerying = false;

			if (results.len() < 2)
			{
				return;
			}

			this.mPopupMenu = this.GUI.PopupMenu();
			this.mPopupMenu.addActionListener(this);

			foreach( r in results )
			{
				this.mPopupMenu.addMenuOption(r[0], r[0]);
			}

			this.showPopup();
		}
	}

	function onQueryError( qa, err )
	{
		this.mQuerying = false;
		::IGIS.error(err);
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function updatePosition( miniMapWidth )
	{
		this.setPosition(-miniMapWidth - 10, 4);
	}

	function showPopup()
	{
		if (this.mPopupMenu)
		{
			this.mPopupMenu.showMenu();
		}
	}

	function onMousePressed( evt )
	{
		if (this.mQuerying == true)
		{
			return;
		}

		if (this.mPopupMenu == null)
		{
			::_Connection.sendQuery("shard.list", this, []);
			this.mQuerying = true;
		}
		else
		{
			this.showPopup();
		}
	}

}

