this.require("UI/Screens");
this.MINIMAP_SMALL <- 0;
this.MINIMAP_LARGE <- 1;
this.MINIMAP_OFF <- 2;
class this.Screens.MiniMapScreen extends this.GUI.Component
{
	static mClassName = "MiniMapScreen";
	mMode = this.MINIMAP_SMALL;
	mMMZoom = 2500.0;
	mMMap = null;
	mToolTipComp = null;
	mToolTipPosition = null;
	mToolTipLastPosition = null;
	mTooltipTimer = null;
	mZoomInButton = null;
	mZoomOutButton = null;
	static TOOLTIP_SIZE = 10;
	static LOCATION_HEIGHT = 19;
	constructor()
	{
		this.GUI.Component.constructor(this.GUI.FlowLayout());
		local font = this.GUI.Font("Maiandra", 24, true);
		this.mToolTipPosition = {
			x = 0,
			y = 0
		};
		this.mToolTipLastPosition = {
			x = 0,
			y = 0
		};
		this.mMMap = this.GUI.MiniMap();
		this.mMMap.addActionListener(this);
		this.mMMap.setPreferredSize(175, 175);
		this.mMMap.setSize(175, 175);
		this.mMMap.setPosition(0, 0);
		this.add(this.mMMap);
		local zhack = this.GUI.ZOffset(1, null);
		zhack.setPosition(0, 0);
		zhack.setSize(175, 175);
		this.add(zhack);
		local bigMapContainer = this.GUI.Container(null);
		bigMapContainer.setAppearance("MiniMap/MapButtonHolder");
		bigMapContainer.setSize(35, 35);
		zhack.setPosition(0, 10);
		zhack.add(bigMapContainer);
		local bigMapButton = this.GUI.ImageButton();
		bigMapButton.setAppearance("MiniMap/MapButton");
		bigMapButton.setGlowImageName("MiniMap/MapButton/Glow");
		bigMapButton.setPressMessage("onMapToggle");
		bigMapButton.addActionListener(this);
		bigMapButton.setSize(35, 35);
		bigMapButton.setTooltip("World Map");
		bigMapContainer.add(bigMapButton);
		this.mZoomInButton = this.GUI.ImageButton();
		this.mZoomInButton.setAppearance("MiniMapPlus");
		this.mZoomInButton.setGlowEnabled(false);
		this.mZoomInButton.setPressMessage("onZoomIn");
		this.mZoomInButton.addActionListener(this);
		this.mZoomInButton.setPosition(133, 13);
		this.mZoomInButton.setSize(22, 22);
		this.mZoomInButton.setTooltip("Zoom In");
		zhack.add(this.mZoomInButton);
		this.mZoomOutButton = this.GUI.ImageButton();
		this.mZoomOutButton.setAppearance("MiniMapMinus");
		this.mZoomOutButton.setGlowEnabled(false);
		this.mZoomOutButton.setPressMessage("onZoomOut");
		this.mZoomOutButton.addActionListener(this);
		this.mZoomOutButton.setPosition(147, 13 + 20);
		this.mZoomOutButton.setSize(22, 22);
		this.mZoomOutButton.setTooltip("Zoom Out");
		zhack.add(this.mZoomOutButton);
		this.mToolTipComp = this.GUI.Container(null);
		this.mToolTipComp.setSize(this.TOOLTIP_SIZE, this.TOOLTIP_SIZE);
		this.mToolTipComp.setPreferredSize(this.TOOLTIP_SIZE, this.TOOLTIP_SIZE);
		this.mToolTipComp.setPosition(0, 0);
		zhack.add(this.mToolTipComp);
		local tbut = this.GUI.Button("size");
		tbut.setReleaseMessage("onSize");
		tbut.addActionListener(this);
		tbut = this.GUI.Button("close");
		tbut.setReleaseMessage("onClose");
		tbut.addActionListener(this);
		this.setSticky("right", "top");
		this.setPosition(-this.getWidth() - 4, 4 + this.LOCATION_HEIGHT);
		::Screen.setOverlayVisible("GUI/MiniMap", true);
		this.setOverlay("GUI/MiniMap");
		this.setMode(this.MINIMAP_SMALL);
	}

	function toggleMode()
	{
		if (this.mMode == this.MINIMAP_OFF)
		{
			this.mMode = this.MINIMAP_SMALL;
		}
		else
		{
			this.mMode = this.MINIMAP_OFF;
		}

		this.setMode(this.mMode);
	}

	function setMode( mode )
	{
		switch(mode)
		{
		case this.MINIMAP_SMALL:
			this.setPreferredSize(175, 175);
			this.setSize(175, 175);
			this.setVisible(true);
			break;

		case this.MINIMAP_LARGE:
			this.setPreferredSize(256, 256);
			this.setSize(256, 256);
			this.setVisible(true);
			break;

		case this.MINIMAP_OFF:
			this.setVisible(false);
			break;
		}

		this.log.debug(this.getPreferredSize().width + ", " + this.getPreferredSize().height);
		this.setPosition(-this.getWidth() - 4, 4 + this.LOCATION_HEIGHT);
		local miniMapLocation = this.Screens.get("MiniMapLocation", false);

		if (miniMapLocation)
		{
			miniMapLocation.updatePosition(this.getWidth());
		}

		this.mMode = mode;
	}

	function setVisible( visible )
	{
		local miniMapLocation = this.Screens.get("MiniMapLocation", true);
		miniMapLocation.setVisible(visible);

		if (visible)
		{
			this.Screens.show("MiniMapLocation");
		}

		this.GUI.Component.setVisible(visible);
	}

	function onMapToggle( button )
	{
		this.Screens.toggle("MapWindow");
	}

	function onZoomIn( pButton )
	{
		local newScale = this.mMMap.mScale / 2;
		this.mMMap.setScale(newScale);
		::Pref.set("minimap.ZoomScale", newScale, true);

		if (newScale <= this.GUI.MiniMap.MIN_SCALE)
		{
			this.mZoomInButton.setEnabled(false);
		}

		this.mZoomOutButton.setEnabled(true);
	}

	function onZoomOut( pButton )
	{
		local newScale = this.mMMap.mScale * 2;
		this.mMMap.setScale(newScale);
		::Pref.set("minimap.ZoomScale", newScale, true);

		if (newScale >= this.GUI.MiniMap.MAX_SCALE)
		{
			this.mZoomOutButton.setEnabled(false);
		}

		this.mZoomInButton.setEnabled(true);
	}

	function setZoomScale( scaleSize )
	{
		this.mMMap.setScale(scaleSize);
		this.mZoomOutButton.setEnabled(true);
		this.mZoomInButton.setEnabled(true);

		if (this.newScale <= this.GUI.MiniMap.MIN_SCALE)
		{
			this.mZoomInButton.setEnabled(false);
		}

		if (this.newScale >= this.GUI.MiniMap.MAX_SCALE)
		{
			this.mZoomOutButton.setEnabled(false);
		}
	}

	function onClose( pButton )
	{
		this.setMode(this.MINIMAP_OFF);
	}

	function onSize( pButton )
	{
		this.setMode(this.mMode == this.MINIMAP_SMALL ? this.MINIMAP_LARGE : this.MINIMAP_SMALL);
	}

	function onBuildMode( button, state )
	{
	}

	function onMinimapClick( pMinimap, x, z )
	{
		if (this.Util.isDevMode())
		{
			this.print("*** clicked " + x + ", " + z);
			::_Connection.sendQuery("go", this, [
				"" + x,
				"-1.0",
				"" + z
			]);
			this.print("*** welcome to warp zone!");
		}
	}

	function onRowSelectionChanged( sender, index, bool )
	{
		local selection = sender.getRow(index);

		if (!selection)
		{
			return;
		}

		if (!this._avatar)
		{
			return;
		}

		if (!this.mInitialAvatarType)
		{
			local iniType = this._avatar.getType();

			if (!iniType)
			{
				this.reutrn;
			}
			else
			{
				this.mInitialAvatarType = iniType;
			}
		}

		local pType = selection[0];
		this.log.debug("Selection: " + selection[0]);

		if (pType == "-None-")
		{
			this._swapAvatar(pType);
		}
		else
		{
			this._loadCreatrureArchives(selection[0], true);
		}
	}

	function onRunAnimation( evt )
	{
		this.command("/emote " + this.mAnimationDropDown.getCurrent());
	}

	function command( pString )
	{
		::EvalCommand(pString);
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();

		if (!this.mWidget)
		{
			this.mWidget.addListener(this);
		}
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Component._removeNotify();
	}

	function onMouseMoved( evt )
	{
		this.mToolTipPosition = {
			x = evt.x,
			y = evt.y
		};
	}

	function onMouseEnter( evt )
	{
		this.mTooltipTimer = ::Timer();
		this._exitFrameRelay.addListener(this);
	}

	function onMouseExit( evt )
	{
		this.mTooltipTimer = null;
		this._exitFrameRelay.removeListener(this);
	}

	function onExitFrame()
	{
		if (this.mTooltipTimer && this.mTooltipTimer.getMilliseconds() > 490 && (this.mToolTipLastPosition.x != this.mToolTipPosition.x || this.mToolTipLastPosition.y != this.mToolTipPosition.y))
		{
			local VIEW_SIZE = 256.0;
			local MAP_SIZE = 175.0;
			local viewScale = VIEW_SIZE / MAP_SIZE;
			local x = this.mToolTipPosition.x.tofloat() * viewScale;
			local y = this.mToolTipPosition.y.tofloat() * viewScale;
			local result = this._root.getMarkerToolTip(x, y);

			if (result.name.find("Creature/") != null)
			{
				local creatureId = ::Util.replace(result.name, "Creature/", "");
				creatureId = creatureId.tointeger();

				if (::_sceneObjectManager.hasCreature(creatureId))
				{
					local creature = ::_sceneObjectManager.getCreatureByID(creatureId);
					this.mToolTipComp.setTooltip(creature.getName());
				}
			}
			else
			{
				this.mToolTipComp.setTooltip(result.name);
			}

			if (result.name != "")
			{
				this.mToolTipComp.setPosition(this.mToolTipPosition.x - this.TOOLTIP_SIZE / 2, this.mToolTipPosition.y - this.TOOLTIP_SIZE / 2 - 10);
			}

			this.mToolTipLastPosition = this.deepClone(this.mToolTipPosition);
			this.mTooltipTimer = null;
			this.mTooltipTimer = ::Timer();
			this.mTooltipTimer.reset();
		}
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.RBUTTON)
		{
			evt.consume();
		}

		if (evt.button == this.MouseEvent.LBUTTON)
		{
			local viewScale = 256.0 / 175.0;
			local x = evt.x.tofloat() * viewScale;
			local y = evt.y.tofloat() * viewScale;
			local clickpos = this._root.getMiniMapWorldCoordinate(x, y);
			this.onMinimapClick(this, clickpos.x, clickpos.z);
			evt.consume();
		}
	}

}

