this.require("UI/MapDef");
this.require("UI/MapWindow");
class this.GUI.Map extends this.GUI.Container
{
	static MAP_WIDTH = 784.0;
	static MAP_HEIGHT = 541.0;
	BOX_SCALE = 1920;
	mMapType = "World";
	mZoomLevel = "World";
	mMapName = null;
	mPosition = null;
	mMessageBroadcaster = null;
	mBaseMapFromServer = null;
	constructor( ... )
	{
		this.GUI.Container.constructor(null);
		this.mMessageBroadcaster = this.MessageBroadcaster();
		this.setSize(this.MAP_WIDTH, this.MAP_HEIGHT);
		this.setPreferredSize(this.MAP_WIDTH, this.MAP_HEIGHT);
	}

	function update()
	{
		if (this.mPosition && this.mZoomLevel)
		{
			return this._updateMapImage();
		}

		return false;
	}

	function setPlayerPosition( position )
	{
		this.mPosition = position;
	}

	function setZoomLevel( zoomLevel )
	{
		this.mZoomLevel = zoomLevel;
	}

	function getZoomLevel()
	{
		return this.mZoomLevel;
	}

	function getPresetMapType()
	{
		return this.mMapType;
	}

	function getInvisibleMarkers()
	{
		local defaultMarkers = [];

		if (this.mZoomLevel in this.MapDef[this.mMapName])
		{
			local mapObj = this.MapDef[this.mMapName][this.mZoomLevel].maps[this.mMapType];

			if (mapObj && "extraMarkers" in mapObj)
			{
				defaultMarkers = mapObj.extraMarkers;
			}
		}

		return defaultMarkers;
	}

	function setMapType( mapType )
	{
		if ((this.mZoomLevel in this.MapDef[this.mMapName]) && mapType in this.MapDef[this.mMapName][this.mZoomLevel].maps)
		{
			this.mMapType = mapType;
		}
	}

	function updateBaseImage( mapImageName )
	{
		this.mBaseMapFromServer = mapImageName;
		this._updateMapImage();
	}

	function _updateMapImage()
	{
		if (this.mMapName == null || this.mPosition == null)
		{
			return;
		}

		if (vargc > 0)
		{
			this.forceZoomToImage = vargv[0];
		}

		local mapType = this.getMapType(this.mPosition, this.mZoomLevel);

		if (!mapType)
		{
			return false;
		}

		this.mMapType = mapType;

		if (!(this.mZoomLevel in this.MapDef[this.mMapName]))
		{
			return false;
		}

		local mapObj = this.MapDef[this.mMapName][this.mZoomLevel].maps[this.mMapType];

		if (mapObj)
		{
			this.setAppearance(mapObj.image);
			this.mMessageBroadcaster.broadcastMessage("updateMapTitle");
		}

		return true;
	}

	function getMapName()
	{
		return this.mMapName;
	}

	function onPackageComplete( name )
	{
		this._updateMapImage();
	}

	function onPackageError( name, error )
	{
		this.log.debug("Unable to load map archive: " + name + ", reason: " + error);
	}

	function setMapName( name )
	{
		this.mMapName = name;
		this._contentLoader.load(name, this.ContentLoader.PRIORITY_NORMAL, name, this);
	}

	function getMapType( position, ... )
	{
		if (!position || !this.mMapName)
		{
			return null;
		}

		local zoomLevel = this.mZoomLevel;

		if (vargc > 0)
		{
			zoomLevel = vargv[0];
		}

		local returnKey;

		if (this.mBaseMapFromServer != null && this.mBaseMapFromServer == "None")
		{
			return null;
		}

		if (this.mBaseMapFromServer != null && this.mBaseMapFromServer != "None")
		{
			returnKey = this.findMapTypeGivenImage(zoomLevel, this.mMapName, this.mBaseMapFromServer, false);

			if (returnKey == null)
			{
				this.findMapTypeGivenImage(zoomLevel, this.mMapName, this.mBaseMapFromServer, true);
			}
		}
		else
		{
			returnKey = this._getDefaultMapType(zoomLevel, position);
		}

		return returnKey;
	}

	function findMapTypeGivenImage( zoomLevel, selectedMapName, imageName, forceZoomToImage )
	{
		local resultKey;

		foreach( mapName, mapNameData in this.MapDef )
		{
			foreach( zoomType, zoomData in mapNameData )
			{
				foreach( key, map in zoomData.maps )
				{
					if (map.image == imageName)
					{
						if (zoomType == zoomLevel && mapName == selectedMapName)
						{
							return key;
						}
						else if (forceZoomToImage && mapName == selectedMapName)
						{
							this.mZoomLevel = zoomType;
							return key;
						}
						else if ("parentMapImage" in map)
						{
							resultKey = this.findMapTypeGivenImage(zoomLevel, selectedMapName, map.parentMapImage, false);

							if (resultKey)
							{
								return resultKey;
							}
						}
					}
				}
			}
		}

		return resultKey;
	}

	function _getDefaultMapType( zoomLevel, position )
	{
		if (zoomLevel in this.MapDef[this.mMapName])
		{
			local maps = this.MapDef[this.mMapName][zoomLevel].maps;
			local myPosition = {
				x = position.x.tointeger(),
				z = position.z.tointeger()
			};
			local found = false;
			local foundMapType;

			foreach( key, map in maps )
			{
				if (this.isPositionOnMap(myPosition, zoomLevel, key))
				{
					if (!foundMapType)
					{
						foundMapType = key;
					}
					else if (("priority" in maps[foundMapType]) && ("priority" in map) && maps[foundMapType].priority > map.priority)
					{
						foundMapType = key;
					}

					found = true;
				}
			}

			if (found)
			{
				return foundMapType;
			}
		}

		return null;
	}

	function scaleXZPosition( position )
	{
		if (("x" in position) && "z" in position)
		{
			if (this.mMapName && this.mZoomLevel && (this.mZoomLevel in this.MapDef[this.mMapName]) && this.mMapType in this.MapDef[this.mMapName][this.mZoomLevel].maps)
			{
				local mapObj = this.MapDef[this.mMapName][this.mZoomLevel].maps[this.mMapType];
				local scaleWidth = this.MAP_WIDTH / mapObj.numPagesAcross.tofloat();
				local scaleHeight = this.MAP_HEIGHT / mapObj.numPagesDown.tofloat();
				local scaledX = (position.x - mapObj.u0) / (this.BOX_SCALE.tofloat() / scaleWidth);
				local scaledZ = (position.z - mapObj.v0) / (this.BOX_SCALE.tofloat() / scaleHeight);
				local newPosition = {
					x = scaledX,
					z = scaledZ
				};
				return newPosition;
			}
		}

		return null;
	}

	function getMapToWorldPosition( posX, posZ )
	{
		if ((this.mZoomLevel in this.MapDef[this.mMapName]) && this.mMapType in this.MapDef[this.mMapName][this.mZoomLevel].maps)
		{
			local mapObj = this.MapDef[this.mMapName][this.mZoomLevel].maps[this.mMapType];
			local scaleWidth = this.MAP_WIDTH / mapObj.numPagesAcross.tofloat();
			local scaleHeight = this.MAP_HEIGHT / mapObj.numPagesDown.tofloat();
			local worldX = posX * (this.BOX_SCALE.tofloat() / scaleWidth) + mapObj.u0;
			local worldZ = posZ * (this.BOX_SCALE.tofloat() / scaleHeight) + mapObj.v0;
			local worldPos = {
				x = worldX,
				z = worldZ
			};
			return worldPos;
		}

		return null;
	}

	function isPositionOnMap( position, ... )
	{
		local zoomLevel = this.mZoomLevel;
		local mapType = this.mMapType;

		if (this.mMapName == null)
		{
			return false;
		}

		if (vargc > 1)
		{
			zoomLevel = vargv[0];
			mapType = vargv[1];
		}

		if (("x" in position) && ("z" in position) && zoomLevel in this.MapDef[this.mMapName])
		{
			if (mapType in this.MapDef[this.mMapName][zoomLevel].maps)
			{
				local mapObj = this.MapDef[this.mMapName][zoomLevel].maps[mapType];
				local x = position.x;
				local z = position.z;

				if (x >= mapObj.u0 && x <= mapObj.u1 && z >= mapObj.v0 && z <= mapObj.v1)
				{
					return true;
				}
			}
		}

		return false;
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		if (this.mWidget != null)
		{
			this.mWidget.removeListener(this);
		}

		this.GUI.Container._removeNotify();
	}

	function addActionListener( listener )
	{
		this.mMessageBroadcaster.addListener(listener);
	}

	function onMousePressed( evt )
	{
		if (evt.clickCount != 1)
		{
			return;
		}

		if (evt.button == this.MouseEvent.LBUTTON && !this.Key.isDown(this.Key.VK_CONTROL))
		{
			this.mMessageBroadcaster.broadcastMessage("closeLegendPanel");
		}

		if (evt.button == this.MouseEvent.LBUTTON && this.Key.isDown(this.Key.VK_CONTROL))
		{
			local worldPos = this.getMapToWorldPosition(evt.x, evt.y);

			if (worldPos)
			{
				this.mMessageBroadcaster.broadcastMessage("onMapCtrlClick", this, worldPos.x, worldPos.z);
			}

			evt.consume();
		}
	}

}

