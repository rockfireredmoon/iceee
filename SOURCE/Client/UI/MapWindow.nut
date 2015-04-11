this.require("GUI/CheckBoxLabelImage");
this.require("UI/MapDef");
this.require("UI/Screens");
class this.GUI.MapLegendWindow extends this.GUI.Component
{
	static nClassName = "MapLegendWindow";
	constructor()
	{
		this.GUI.InnerPanel.constructor(this.GUI.BoxLayoutV());
		this.setInsets(10, 0, 10, 0);
		this.setSize(170, 250);
		this.setPreferredSize(170, 250);
	}

}

class this.Screens.MapWindow extends this.GUI.MainScreenElement
{
	static nClassName = "MapWindow";
	static TITLE_HEIGHT = 50;
	static MINIMUN_TIME_SECONDS = 1000 * 15;
	static MINIMUM_DISTANCE_MOVED = 200;
	static MAX_DISTANCE = 1000;
	mMainWindowSize = {
		width = 800,
		height = 600
	};
	mMapContainer = null;
	mMapImage = null;
	mLegendContainer = null;
	mInvisibleMarkerLayer = null;
	mDistanceTimer = null;
	mLastDistanceMarkers = [];
	mPlayerMoved = false;
	mPlayerLastPosition = {
		x = 0,
		z = 0
	};
	mTargetZone = 1;
	mInitializeMarkerList = false;
	mPlayersMarker = {
		name = "You",
		zoneID = 1,
		position = {
			x = 60000,
			z = 120000
		},
		iconType = ::LegendItemTypes.YOU
	};
	mPartyMembers = null;
	mLocationLabel = null;
	static LOCAL_ICON_SIZE = 14.0;
	static MAP_CORNER_SIZE = 75;
	static CLOSE_SIZE = 26;
	static ZOOM_SIZE = 19;
	constructor()
	{
		local GAP = 10;
		this.mPartyMembers = {};
		this.mLastDistanceMarkers = [];
		this.GUI.MainScreenElement.constructor(this.GUI.BorderLayout());
		this.setSize(this.mMainWindowSize.width, this.mMainWindowSize.height);
		this.setPreferredSize(this.mMainWindowSize.width, this.mMainWindowSize.height);
		this.setAppearance("ThickSilverBorder");
		::_Connection.addListener(this);
		local titleComp = this.GUI.Container(this.GUI.BorderLayout());
		titleComp.setInsets(9, 15, 9, 15);
		titleComp.setSize(this.mMainWindowSize.width, this.TITLE_HEIGHT);
		titleComp.setPreferredSize(this.mMainWindowSize.width, this.TITLE_HEIGHT);
		titleComp.setAppearance("ThickSilverBorder");
		this.add(titleComp, this.GUI.BorderLayout.NORTH);
		local centerComp = this.GUI.Component(this.GUI.BoxLayout());
		centerComp.getLayoutManager().setAlignment(0.5);
		titleComp.add(centerComp, this.GUI.BorderLayout.CENTER);
		local legendButton = this.GUI.Button("Legend");
		legendButton.setFixedSize(50, 20);
		legendButton.addActionListener(this);
		legendButton.setReleaseMessage("_showLegendPress");
		centerComp.add(legendButton);
		this.mLocationLabel = this.GUI.Label("");
		this.mLocationLabel.setFont(this.GUI.Font("Maiandra", 32));
		titleComp.add(this.mLocationLabel, this.GUI.BorderLayout.EAST);
		this.mMapContainer = this.GUI.Container();
		this.mMapContainer.setInsets(0, 8, 8, 15);
		this.mMapContainer.setSize(this.mMainWindowSize.width, this.mMainWindowSize.height - this.TITLE_HEIGHT);
		this.mMapContainer.setPreferredSize(this.mMainWindowSize.width, this.mMainWindowSize.height - this.TITLE_HEIGHT);
		this.add(this.mMapContainer, this.GUI.BorderLayout.CENTER);
		local paperBG = this.GUI.Container(null);
		paperBG.setSize(this.mMainWindowSize.width - 9 * 2, this.mMainWindowSize.height - this.TITLE_HEIGHT - 9);
		paperBG.setPreferredSize(this.mMainWindowSize.width - 9 * 2, this.mMainWindowSize.height - this.TITLE_HEIGHT - 9);
		paperBG.setAppearance("PaperBackBorder");
		this.mMapContainer.add(paperBG);
		this.mMapImage = this.GUI.Map();
		this.mMapImage.setPosition(0, 0);
		this.mMapImage.addActionListener(this);
		paperBG.add(this.mMapImage);
		this.mInvisibleMarkerLayer = this._buildInvisibleMarkerLayer();
		this.mLegendContainer = this.GUI.MapLegendWindow();
		this.mLegendContainer.setPosition(5, 0);
		this.mLegendContainer.setVisible(false);
		this.mInvisibleMarkerLayer.add(this.mLegendContainer);
		this.mMapImage.add(this.mInvisibleMarkerLayer);
		local mapRightCorner = this.GUI.Container(null);
		mapRightCorner.setSize(this.MAP_CORNER_SIZE, this.MAP_CORNER_SIZE);
		mapRightCorner.setPreferredSize(this.MAP_CORNER_SIZE, this.MAP_CORNER_SIZE);
		mapRightCorner.setPosition(this.mMapImage.getSize().width - this.MAP_CORNER_SIZE + 10, -5);
		mapRightCorner.setAppearance("MapCorner");
		this.mInvisibleMarkerLayer.add(mapRightCorner);
		local closeButton = this.GUI.Button("");
		closeButton.setFixedSize(this.CLOSE_SIZE, this.CLOSE_SIZE);
		closeButton.addActionListener(this);
		closeButton.setReleaseMessage("_onClose");
		closeButton.setPosition(41, 8);
		closeButton.setAppearance("MapClose");
		mapRightCorner.add(closeButton);
		local plusButton = this.GUI.Button("");
		plusButton.setFixedSize(this.ZOOM_SIZE, this.ZOOM_SIZE);
		plusButton.addActionListener(this);
		plusButton.setReleaseMessage("_onZoomIn");
		plusButton.setPosition(16, 21);
		plusButton.setAppearance("MapPlus");
		mapRightCorner.add(plusButton);
		local minusButton = this.GUI.Button("");
		minusButton.setFixedSize(this.ZOOM_SIZE, this.ZOOM_SIZE);
		minusButton.addActionListener(this);
		minusButton.setReleaseMessage("_onZoomOut");
		minusButton.setPosition(35, 40);
		minusButton.setAppearance("MapMinus");
		mapRightCorner.add(minusButton);

		if (::_avatar)
		{
			this.mTargetZone = ::_sceneObjectManager.getCurrentZoneDefID();
			this._fetchList();
			this.mInitializeMarkerList = true;
		}

		::partyManager.addListener(this);
		::_exitFrameRelay.addListener(this);

		if (!this.Util.isDevMode())
		{
			this._removeDevLegendItems();
		}

		local seeInfo = this.MapDef;
		this._updateLegendItems(this.mMapImage.getZoomLevel());
	}

	function _removeDevLegendItems()
	{
		local legendTypesToRemove = [
			"Default"
		];

		foreach( mapType in this.MapDef )
		{
			foreach( zoomLevel in mapType )
			{
				foreach( key, legendType in zoomLevel.channels )
				{
					foreach( channelType in legendTypesToRemove )
					{
						if (channelType == legendType)
						{
							zoomLevel.channels.remove(key);
							break;
						}
					}
				}
			}
		}
	}

	function _addNotify()
	{
		this.GUI.Container._addNotify();
		local questJournal = this.Screens.get("QuestJournal", false);

		if (questJournal)
		{
			questJournal.addActionListener(this);
		}
	}

	function _removeNotify()
	{
		local questJournal = this.Screens.get("QuestJournal", false);

		if (questJournal)
		{
			questJournal.removeActionListener(this);
		}

		this.GUI.Container._removeNotify();
	}

	function onEnvironmentUpdate( zoneId, zoneDefId, zonePageSize, mapName, envType )
	{
		if (mapName != "")
		{
			this.mMapImage.setMapName(mapName);
		}
	}

	function _buildInvisibleMarkerLayer()
	{
		local markerLayer = this.GUI.Container(null);
		markerLayer.setSize(this.mMapImage.getSize().width, this.mMapImage.getSize().height);
		markerLayer.setPreferredSize(this.mMapImage.getSize().width, this.mMapImage.getSize().height);
		markerLayer.setPosition(0, 0);
		return markerLayer;
	}

	function setSelectedLegendItems( value )
	{
		foreach( itemType, visible in value )
		{
			this.updateSelectedLegendItem(itemType, visible);
		}
	}

	function setWindowZoomLevel( value )
	{
		this.mMapImage.setZoomLevel(value);

		if (!this.updateZoom())
		{
			this.mMapImage.setZoomLevel("World");
		}

		this.mLocationLabel.setText(this.mMapImage.getPresetMapType());
	}

	function updateMapBaseImage( mapImageName )
	{
		this.mMapImage.updateBaseImage(mapImageName);
	}

	function updateMapTitle()
	{
		this.mLocationLabel.setText(this.mMapImage.getPresetMapType());
	}

	function setMapType( value )
	{
		this.mMapImage.setMapType(value);
		this.mLocationLabel.setText(value);
	}

	function update()
	{
		if (::_avatar)
		{
			local position = ::_avatar.getNode().getPosition();
			this.mMapImage.setPlayerPosition(position);
		}

		return this.mMapImage.update();
	}

	function onShow()
	{
		this.updateZoom();
		::Audio.playSound("Sound-MapOpen.ogg");
	}

	function updateZoom()
	{
		if (this.update())
		{
			this.mMapImage.setPosition(0, 0);
			this.mInvisibleMarkerLayer.setSize(this.mMapImage.getSize().width, this.mMapImage.getSize().height);
			this.mInvisibleMarkerLayer.setPreferredSize(this.mMapImage.getSize().width, this.mMapImage.getSize().height);

			if (this.mTargetZone != this._sceneObjectManager.getCurrentZoneDefID())
			{
				this._removeAllMarkers();
				this.mTargetZone = this._sceneObjectManager.getCurrentZoneDefID();
				this._fetchList();
			}

			this._updateLegendItems(this.mMapImage.getZoomLevel());
			this._updateAllMarkersPosition();
			this._updateMarkerVisibility();
			this.updateInvisibleMarkers();
			local marker = this.getMarkerComponentGivenObj(this.mInvisibleMarkerLayer.components, this.mPlayersMarker);

			if (!marker)
			{
				this._addMarker(this.mPlayersMarker);
			}

			return true;
		}

		return false;
	}

	function _updateLegendItems( zoomLevel )
	{
		if (this.mMapImage.getMapName() == null)
		{
			return;
		}

		local channels = this.MapDef[this.mMapImage.getMapName()][zoomLevel].channels;
		local itemArray = [];
		this._resetAllLegendSelectionItems(false);

		foreach( itemType in channels )
		{
			itemArray.append(itemType);
			::LegendItems[itemType].isLegendShown = true;
		}

		this.addLegendItems(itemArray);
	}

	function _resetAllLegendSelectionItems( visible )
	{
		foreach( key, item in this.LegendItems )
		{
			if (("isLegendShown" in item) && !(key == this.LegendItemTypes.YOU))
			{
				item.isLegendShown = visible;
			}
		}
	}

	function addLegendItems( itemArray )
	{
		local iconSize = 20;
		local buttonSize = 40;
		this.removeAllLegendItems();
		local count = 1;

		foreach( itemType in itemArray )
		{
			local item = ::LegendItems[itemType];
			local legendCheckBox = this.GUI.CheckBoxLabelImage(item.prettyName, item.iconName, iconSize);
			legendCheckBox.setSize(150, iconSize);
			legendCheckBox.setPreferredSize(150, iconSize);
			legendCheckBox.setReleaseMessage("_onItemPressed", this);
			legendCheckBox.setCheckBoxData(itemType);
			legendCheckBox.setData(itemType);
			this.mLegendContainer.add(legendCheckBox);
			local checkBox = legendCheckBox.getCheckBox();
			checkBox.setChecked(::LegendItemSelected[itemType]);
			count = count + 1;
		}

		local closeButton = this.GUI.Button("Close");
		closeButton.addActionListener(this);
		closeButton.setReleaseMessage("_closeLegendPress");
		this.mLegendContainer.setSize(160, count * iconSize + 10);
		this.mLegendContainer.setPreferredSize(160, count * iconSize + 10);
		this._updateMarkerVisibility();
	}

	function removeAllLegendItems()
	{
		this.mLegendContainer.removeAll();
	}

	function _onClose( button )
	{
		this.setVisible(false);
		::Audio.playSound("Sound-MapClose.ogg");
	}

	function _onZoomIn( button )
	{
		local previousZoomLevel = this.mMapImage.getZoomLevel();

		if ("zoomIn" in ::ZoomLevels[previousZoomLevel])
		{
			this.mMapImage.setZoomLevel(::ZoomLevels[previousZoomLevel].zoomIn);

			if (!this.updateZoom())
			{
				this.mMapImage.setZoomLevel(previousZoomLevel);
			}
			else
			{
				::Pref.set("map.ZoomLevel", ::ZoomLevels[previousZoomLevel].zoomIn);
				::Pref.set("map.MapType", this.mMapImage.getPresetMapType());
				this.mLocationLabel.setText(this.mMapImage.getPresetMapType());
			}
		}
	}

	function _onZoomOut( button )
	{
		local previousZoomLevel = this.mMapImage.getZoomLevel();

		if ("zoomOut" in ::ZoomLevels[previousZoomLevel])
		{
			this.mMapImage.setZoomLevel(::ZoomLevels[previousZoomLevel].zoomOut);

			if (!this.updateZoom())
			{
				this.mMapImage.setZoomLevel(previousZoomLevel);
			}
			else
			{
				::Pref.set("map.ZoomLevel", ::ZoomLevels[previousZoomLevel].zoomOut);
				::Pref.set("map.MapType", this.mMapImage.getPresetMapType());
				this.mLocationLabel.setText(this.mMapImage.getPresetMapType());
			}
		}
	}

	function closeLegendPanel()
	{
		this.mLegendContainer.setVisible(false);
	}

	function _showLegendPress( button )
	{
		if (this.mLegendContainer.isVisible())
		{
			this.mLegendContainer.setVisible(false);
		}
		else
		{
			this.mLegendContainer.setVisible(true);
		}
	}

	function _onItemPressed( button, val )
	{
		local itemType = button.getData();
		local legendSelectedTable = this.deepClone(::Pref.get("map.LegendItems"));
		legendSelectedTable[itemType] <- val;
		::Pref.set("map.LegendItems", legendSelectedTable);
		this.updateSelectedLegendItem(itemType, val);
	}

	function getButtonGivenItemType( itemType )
	{
		local legendItems = this.mLegendContainer.components;

		foreach( checkBoxImage in legendItems )
		{
			if (checkBoxImage.getData() == itemType)
			{
				return checkBoxImage.getCheckBox();
			}
		}

		return null;
	}

	function _addStickerToMiniMap( markerObj, sticker )
	{
		if (sticker != this.LegendItemTypes.YOU && sticker != this.LegendItemTypes.SHOP)
		{
			this._root.setMinimapMarkerPositionSticker(markerObj.name, this.Vector3(markerObj.position.x, 0, markerObj.position.z), sticker);
		}
	}

	function updateSelectedLegendItem( itemType, val )
	{
		local button = this.getButtonGivenItemType(itemType);

		if (button)
		{
			button.setChecked(val);
		}

		::LegendItemSelected[itemType] = val;

		if (itemType == ::LegendItemTypes.QUEST)
		{
			for( local i = 1; i < 5; i++ )
			{
				::LegendItemSelected[itemType + "Tracker" + i] = val;
			}
		}

		::Util.updateMiniMapStickers();
		local markers = this.mInvisibleMarkerLayer.components;

		foreach( item in markers )
		{
			if (item.getData())
			{
				local iconType = item.getData().iconType;

				if (item && item.getData() && (iconType == itemType || itemType == ::LegendItemTypes.QUEST && (iconType == ::LegendItemTypes.QUEST_TRACKER1 || iconType == ::LegendItemTypes.QUEST_TRACKER2 || iconType == ::LegendItemTypes.QUEST_TRACKER3 || iconType == ::LegendItemTypes.QUEST_TRACKER4)))
				{
					local markerObj = item.getData();

					if (this.mMapImage.isPositionOnMap(item.getData().position) && markerObj.zoneID.tointeger() == this.mTargetZone)
					{
						item.setVisible(val);
					}

					if (val && markerObj.zoneID.tointeger() == this.mTargetZone)
					{
						this._addStickerToMiniMap(markerObj, markerObj.iconType);
					}
					else
					{
						this._addStickerToMiniMap(markerObj, "");
					}
				}
			}
		}
	}

	function _updateMarkerVisibility()
	{
		local markers = this.mInvisibleMarkerLayer.components;

		foreach( item in markers )
		{
			if (item && (item instanceof this.GUI.Component) && item.getData() && item.getData().iconType)
			{
				local markerObj = item.getData();
				local itemType = item.getData().iconType;
				local myLegend = ::LegendItems[itemType];
				local isMarkerOnMap = this.mMapImage.isPositionOnMap(item.getData().position);
				local isLegendShown = ::LegendItems[itemType].isLegendShown;

				if (itemType == ::LegendItemTypes.QUEST_TRACKER1 || itemType == ::LegendItemTypes.QUEST_TRACKER2 || itemType == ::LegendItemTypes.QUEST_TRACKER3 || itemType == ::LegendItemTypes.QUEST_TRACKER4)
				{
					isLegendShown = true;
				}

				local visible = false;
				local stickerType = "";

				if (::LegendItemSelected[itemType] && isLegendShown && isMarkerOnMap && (markerObj.zoneID.tointeger() == this.mTargetZone || itemType == ::LegendItemTypes.YOU))
				{
					visible = true;
					stickerType = markerObj.iconType;
				}
				else if (::LegendItemSelected[itemType] && markerObj.zoneID.tointeger() == this.mTargetZone)
				{
					stickerType = markerObj.iconType;
				}

				item.setVisible(visible);
				this._addStickerToMiniMap(markerObj, stickerType);
			}
		}
	}

	function _addMarker( markerObj )
	{
		local isMarkerOnMap = this.mMapImage.isPositionOnMap(markerObj.position);

		if (markerObj.iconType == this.LegendItemTypes.YOU)
		{
			this.mPlayersMarker = markerObj;
		}

		local marker = this.GUI.MarkerComp(markerObj, this.mMapImage);
		marker.addActionListener(this);
		marker.setPressMessage("_gotoPosition");
		this.mInvisibleMarkerLayer.add(marker);
		local itemType = markerObj.iconType;
		local isLegendShown = ::LegendItems[markerObj.iconType].isLegendShown;

		if (itemType == ::LegendItemTypes.QUEST_TRACKER1 || itemType == ::LegendItemTypes.QUEST_TRACKER2 || itemType == ::LegendItemTypes.QUEST_TRACKER3 || itemType == ::LegendItemTypes.QUEST_TRACKER4)
		{
			isLegendShown = true;
		}

		if (::LegendItemSelected[markerObj.iconType] && isLegendShown && isMarkerOnMap && markerObj.zoneID.tointeger() == this.mTargetZone)
		{
			marker.setVisible(true);
		}
		else
		{
			marker.setVisible(false);
		}

		if (::LegendItemSelected[markerObj.iconType] && markerObj.zoneID.tointeger() == this.mTargetZone)
		{
			this._addStickerToMiniMap(markerObj, markerObj.iconType);
		}
		else
		{
			this._addStickerToMiniMap(markerObj, "");
		}

		return marker;
	}

	function clearInvisibleMarkers()
	{
		local markers = this.mInvisibleMarkerLayer.components;

		for( local i = markers.len() - 1; i >= 0; i = i - 1 )
		{
			if (markers[i] != null && (markers[i] instanceof this.GUI.InvisibleMarkerComp))
			{
				this.mInvisibleMarkerLayer.remove(markers[i]);
			}
		}
	}

	function updateInvisibleMarkers()
	{
		this.clearInvisibleMarkers();

		if (this.mMapImage)
		{
			local invisibleMarkers = this.mMapImage.getInvisibleMarkers();

			foreach( invisMarker in invisibleMarkers )
			{
				local marker = this.GUI.InvisibleMarkerComp(invisMarker);
				this.mInvisibleMarkerLayer.add(marker);
			}
		}
	}

	function _gotoPosition( markerComp )
	{
		local markerObj = markerComp.getData();
		local location = this.mMapImage.getMapName();
		location = this.Util.replace(location, "Maps-", "");
		::_Connection.sendQuery("go", this, [
			location,
			"" + markerObj.position.x,
			"-1.0",
			"" + markerObj.position.z
		]);
	}

	function onMapCtrlClick( mapImage, x, z )
	{
		local location = this.mMapImage.getMapName();
		location = this.Util.replace(location, "Maps-", "");
		::_Connection.sendQuery("go", this, [
			location,
			"" + x,
			"-1.0",
			"" + z
		]);
	}

	function _updateAllMarkersPosition()
	{
		local markers = this.mInvisibleMarkerLayer.components;

		foreach( marker in markers )
		{
			this._updateMarkerScaledPosition(marker);
		}
	}

	function _updateMarkerScaledPosition( marker )
	{
		if (marker && marker.getData() && "position" in marker.getData())
		{
			local scaledPosition = this.mMapImage.scaleXZPosition(marker.getData().position);

			if (scaledPosition)
			{
				local xPos = scaledPosition.x - this.LOCAL_ICON_SIZE / 2.0;
				local yPos = scaledPosition.z - this.LOCAL_ICON_SIZE / 2.0;
				marker.setPosition(xPos, yPos);
			}
		}
	}

	function _removeAllMarkers()
	{
		local markers = this.mInvisibleMarkerLayer.components;

		for( local i = markers.len() - 1; i >= 0; i = i - 1 )
		{
			if (markers[i].getData())
			{
				local markerObj = markers[i].getData();
				this._addStickerToMiniMap(markerObj, "");
				this.mInvisibleMarkerLayer.remove(markers[i]);
			}
		}
	}

	function _getListOfMarkers( legendType )
	{
		local markers = this.mInvisibleMarkerLayer.components;
		local listOfMarkers = [];

		foreach( marker in markers )
		{
			if (marker.getData() && marker.getData().iconType == legendType)
			{
				local markerObj = marker.getData();
				listOfMarkers.append(marker);
			}
		}

		return listOfMarkers;
	}

	function getMarkerComponentGivenObj( markers, markerObj )
	{
		foreach( marker in markers )
		{
			if (marker && marker.getData() && marker.getData().name == markerObj.name && marker.getData().iconType == markerObj.iconType)
			{
				return marker;
			}
		}

		return null;
	}

	function refetchMarkers()
	{
		this._removeAllMarkers();
		this._fetchList();
	}

	function _fetchList()
	{
		if (this.mTargetZone != null)
		{
			if (this.Util.isDevMode())
			{
				this._Connection.sendQuery("marker.list", this, [
					this.mTargetZone
				]);
			}

			this._Connection.sendQuery("map.marker", this, [
				this.mTargetZone,
				::LegendItemTypes.SHOP,
				::LegendItemTypes.VAULT,
				::LegendItemTypes.QUEST_GIVER,
				::LegendItemTypes.HENGE,
				::LegendItemTypes.SANCTUARY
			]);
			this.updateQuestMarkers();
			this.updatePartyMarkers();
		}
	}

	function updateQuestMarkers()
	{
		local questMarkerTypes = ::_questManager.getSelectedQuestMarkerType();
		this._clearMarkers(::LegendItemTypes.QUEST_TRACKER1);
		this._clearMarkers(::LegendItemTypes.QUEST_TRACKER2);
		this._clearMarkers(::LegendItemTypes.QUEST_TRACKER3);
		this._clearMarkers(::LegendItemTypes.QUEST_TRACKER4);
		this._clearMarkers(::LegendItemTypes.QUEST);

		foreach( key, questType in questMarkerTypes )
		{
			local id = questType.questId;
			local questData = ::_questManager.getPlayerQuestDataById(id);

			foreach( objectiveObj in questData.getObjectives() )
			{
				local markerObjects = objectiveObj.getMarkerObjects();

				foreach( marker in markerObjects )
				{
					marker.iconType = questType.iconType;
					this._addMarker(marker);
				}
			}
		}

		this.updateQuestEnderMarker();
	}

	function updateQuestEnderMarker()
	{
		local questMarkerTypes = ::_questManager.getSelectedQuestMarkerType();
		this._clearMarkers(::LegendItemTypes.QUEST);

		foreach( key, questType in questMarkerTypes )
		{
			local id = questType.questId;
			local questData = ::_questManager.getPlayerQuestDataById(id);
			local questEnderMarker = questData.getQuestEnderMarker();

			if (::_avatar && questEnderMarker && questData.isObjectivesComplete() && this.Math.manhattanDistanceXZ(questEnderMarker.position, ::_avatar.getNode().getPosition()) <= this.MAX_DISTANCE)
			{
				questEnderMarker.iconType = questType.iconType;
				this._addMarker(questEnderMarker);
			}
		}
	}

	function updatePartyMarkers()
	{
		this._clearMarkers(::LegendItemTypes.PARTY);
		this.mPartyMembers = {};
		local partyMembers = ::partyManager.getPartyMembers();

		foreach( member in partyMembers )
		{
			local creature = ::_sceneObjectManager.getCreatureByName(member.name);

			if (creature)
			{
				local memberMarker = {
					name = creature.getName(),
					zoneID = ::_sceneObjectManager.getCurrentZoneDefID().tointeger(),
					position = {
						x = creature.getPosition().x,
						z = creature.getPosition().z
					},
					iconType = ::LegendItemTypes.PARTY
				};
				this.mPartyMembers[member.id] <- memberMarker;
				this._addMarker(memberMarker);
			}
		}
	}

	function _clearMarkers( legendType )
	{
		local markers = this.mInvisibleMarkerLayer.components;

		for( local i = markers.len() - 1; i > 0; i = i - 1 )
		{
			if (markers[i].getData() && markers[i].getData().iconType == legendType)
			{
				local markerData = markers[i].getData();
				this._addStickerToMiniMap(markerData, "");
				this.mInvisibleMarkerLayer.remove(markers[i]);
			}
		}
	}

	function onExitFrame()
	{
		if (::_avatar == null)
		{
			return;
		}

		local updateDistanceMarker = true;

		if (!this.mInitializeMarkerList)
		{
			local position = ::_avatar.getNode().getPosition();
			this.mTargetZone = ::_sceneObjectManager.getCurrentZoneDefID();
			this.mMapImage.setPlayerPosition(position);
			this._fetchList();
			this.mInitializeMarkerList = true;
			updateDistanceMarker = false;
		}

		if (this.mTargetZone != this._sceneObjectManager.getCurrentZoneDefID())
		{
			this._removeAllMarkers();
			this.mTargetZone = this._sceneObjectManager.getCurrentZoneDefID();
			this._fetchList();
			updateDistanceMarker = false;
		}

		local playerPosition = this.updatePlayersMarker();

		if (playerPosition != null)
		{
			local totalDistance = this.sqrt((this.mPlayerLastPosition.x - playerPosition.x) * (this.mPlayerLastPosition.x - playerPosition.x) + (this.mPlayerLastPosition.z - playerPosition.z) * (this.mPlayerLastPosition.z - playerPosition.z));

			if (totalDistance >= this.MINIMUM_DISTANCE_MOVED)
			{
				this.mPlayerLastPosition = playerPosition;
				this.mPlayerMoved = true;
			}
		}

		if (!this.mDistanceTimer)
		{
			this.mDistanceTimer = ::Timer();
		}

		if (updateDistanceMarker && this.mPlayerMoved && this.mDistanceTimer.getMilliseconds() > this.MINIMUN_TIME_SECONDS)
		{
			this._updateDistanceMarkers();
			this.mDistanceTimer.reset();
			this.mPlayerMoved = false;
		}
	}

	function updatePlayersMarker()
	{
		local apos = ::_avatar.getNode().getPosition();

		if (this.mPlayersMarker && (this.mPlayersMarker.position.x != apos.x || this.mPlayersMarker.position.z != apos.z))
		{
			this.mPlayersMarker.position.x = apos.x;
			this.mPlayersMarker.position.z = apos.z;
			local marker = this.getMarkerComponentGivenObj(this.mInvisibleMarkerLayer.components, this.mPlayersMarker);

			if (marker)
			{
				marker.getData().position = this.mPlayersMarker.position;
				marker.setTooltipText(this.mPlayersMarker.name, "x:" + this.mPlayersMarker.position.x + " z: " + this.mPlayersMarker.position.z);
				this._updateMarkerScaledPosition(marker);
			}
			else
			{
				this._addMarker(this.mPlayersMarker);
			}

			local playerPosition = {
				x = apos.x,
				z = apos.z
			};
			return playerPosition;
		}

		return null;
	}

	function updatePartyPosition( creature, name )
	{
		if (!creature.getCreatureDef())
		{
			return;
		}

		local creatureDefId = creature.getCreatureDef().getID();

		if (creatureDefId in this.mPartyMembers)
		{
			local partyMarker = this.mPartyMembers[creatureDefId];
			partyMarker.position.x = creature.getPosition().x;
			partyMarker.position.z = creature.getPosition().z;
			local marker = this.getMarkerComponentGivenObj(this.mInvisibleMarkerLayer.components, partyMarker);

			if (marker)
			{
				marker.getData().name = name;
				marker.getData().position = partyMarker.position;
				marker.setTooltipText(partyMarker.name, "x:" + partyMarker.position.x + " z: " + partyMarker.position.z);
				this._updateMarkerScaledPosition(marker);
				this._root.setMinimapMarkerPositionSticker(partyMarker.name, this.Vector3(partyMarker.position.x, 0, partyMarker.position.z), ::LegendItemTypes.PARTY);
			}
			else
			{
				partyMarker.name = name;
				this._addMarker(partyMarker);
			}
		}
		else
		{
			local memberMarker = {
				name = name,
				zoneID = ::_sceneObjectManager.getCurrentZoneDefID().tointeger(),
				position = {
					x = creature.getPosition().x,
					z = creature.getPosition().z
				},
				iconType = ::LegendItemTypes.PARTY
			};
			this.mPartyMembers[creatureDefId] <- memberMarker;
			this._addMarker(memberMarker);
		}
	}

	function _updateDistanceMarkers()
	{
		local DistanceLegendTypes = [
			::LegendItemTypes.SHOP,
			::LegendItemTypes.QUEST_GIVER,
			::LegendItemTypes.SANCTUARY
		];

		foreach( legendType in DistanceLegendTypes )
		{
			this.mLastDistanceMarkers.extend(this._getListOfMarkers(legendType));
		}

		if (this.mTargetZone != null)
		{
			this._Connection.sendQuery("map.marker", this, [
				this.mTargetZone,
				::LegendItemTypes.SHOP,
				::LegendItemTypes.QUEST_GIVER,
				::LegendItemTypes.SANCTUARY
			]);
		}

		this.updateQuestEnderMarker();
	}

	function _handleRemoveOldDistanceMarkers( tempMarkers )
	{
		foreach( oldMarker in this.mLastDistanceMarkers )
		{
			local marker = this.getMarkerComponentGivenObj(tempMarkers, oldMarker.getData());

			if (marker == null)
			{
				local markerObj = oldMarker.getData();
				this._addStickerToMiniMap(markerObj, "");
				this.mInvisibleMarkerLayer.remove(oldMarker);
			}
		}

		this.mLastDistanceMarkers.clear();
	}

	function onQueryComplete( qa, rows )
	{
		switch(qa.query)
		{
		case "marker.list":
			local rowindex = 0;

			foreach( row in rows )
			{
				local foundPos = row[0].tostring().find("Henge/");

				if (foundPos == 0)
				{
					continue;
				}

				local xyz = ::Util.replace(row[2], "(", "");
				xyz = ::Util.replace(xyz, ")", "");
				xyz = this.split(xyz, ",");
				xyz[0] = xyz[0].tointeger();
				xyz[1] = xyz[1].tointeger();
				xyz[2] = xyz[2].tointeger();
				local rowobj = {
					name = row[0].tostring(),
					zoneID = row[1].tointeger(),
					position = {
						x = xyz[0],
						z = xyz[2]
					},
					iconType = ::LegendItemTypes.DEFAULT
				};
				this._addMarker(rowobj);
				rowindex++;
			}

			break;

		case "map.marker":
			local rowindex = 0;
			local tempMarkers = [];

			foreach( row in rows )
			{
				local xyz = ::Util.replace(row[1], "(", "");
				xyz = ::Util.replace(xyz, ")", "");
				xyz = this.split(xyz, " ");
				xyz[0] = xyz[0].tointeger();
				xyz[1] = xyz[1].tointeger();
				xyz[2] = xyz[2].tointeger();
				local displayName = row[0].tostring();

				if (row[2].tostring() == ::LegendItemTypes.QUEST_GIVER)
				{
					displayName = "Quest Giver (" + displayName + ")";
				}

				local rowobj = {
					name = displayName,
					zoneID = qa.args[0].tointeger(),
					position = {
						x = xyz[0],
						z = xyz[2]
					},
					iconType = row[2].tostring()
				};
				local marker = this.getMarkerComponentGivenObj(this.mInvisibleMarkerLayer.components, rowobj);

				if (marker && marker.getData())
				{
					marker.getData().position = rowobj.position;
					marker.getData().zoneID = rowobj.zoneID.tointeger();
					marker.setTooltipText(rowobj.name, "x:" + rowobj.position.x + " z: " + rowobj.position.z);
					this._updateMarkerScaledPosition(marker);
				}
				else
				{
					this._addMarker(rowobj);
				}

				tempMarkers.append(marker);
				rowindex++;
			}

			this._handleRemoveOldDistanceMarkers(tempMarkers);
			break;

		case "go":
			break;
		}
	}

}

