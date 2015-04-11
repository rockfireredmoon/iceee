this.require("UI/UI");
this.require("UI/Screens");
class this.UI.ZoneList extends this.GUI.ColumnList
{
	mOwner = null;
	mZoneDetails = null;
	mSelectOnFetch = "";
	constructor( owner, ... )
	{
		this.GUI.ColumnList.constructor();
		this.mOwner = owner;
		this.setOneSelectMin(true);
		this.addColumn("Name", 100);
		this.addColumn("ZoneID", 30);
		this.addActionListener(this);
		this.fetchList(vargc > 0 ? vargv[0] : "");
	}

	function fetchList( ... )
	{
		if (vargc > 0)
		{
			this.mSelectOnFetch = vargv[0];
		}

		this._Connection.sendQuery("zone.list", this, [
			0,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			11,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			21,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			31,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			41,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			51
		]);
	}

	function onQueryComplete( qa, rows )
	{
		if (qa.query == "zone.list")
		{
			if (this.mZoneDetails == null)
			{
				this.mZoneDetails = [];
			}

			this.mZoneDetails.extend(rows);
			local selectedrows = this.getSelectedRows();
			local selectedzone = this.mZoneDetails[0];

			if (selectedrows.len() > 0)
			{
				selectedzone = this.getRow(selectedrows[0]);
			}

			if (this.mSelectOnFetch != "")
			{
				selectedzone = -1;
			}

			this.removeAllRows();
			local rowindex = 0;

			foreach( row in this.mZoneDetails )
			{
				local rowobj = {
					zoneID = row[0],
					name = row[1],
					terrainConfig = row[2],
					environmentType = row[3],
					mapName = row.len() > 5 ? row[5] : "",
					groupName = row.len() > 6 ? row[6] : "",
					instanceMode = row.len() > 7 ? row[7] : "",
					beefFactor = row.len() > 8 ? row[8] : "0",
					category = row.len() > 9 ? row[9] : "",
					regions = row.len() > 10 ? row[10] : "",
					displayName = row.len() > 11 ? row[11] : ""
				};
				this.addRow([
					rowobj.name,
					rowobj.zoneID
				]);

				if (rowobj.zoneID == selectedzone || rowobj.name == this.mSelectOnFetch || rowobj.zoneID == this.mSelectOnFetch.tostring())
				{
					this.setSelectedRows([
						rowindex
					]);
					this.mSelectOnFetch = "";
				}

				rowindex++;
			}
		}
	}

	function onRowSelectionChanged( list, row, selected )
	{
		if (this.mOwner && selected)
		{
			local zonename = this.getRow(row)[0];
			local zoneid = this.getRow(row)[1];
			this.mOwner.onZoneSelected(list, zonename, zoneid);
		}
	}

	function getZoneDetails( zonename )
	{
		foreach( row in this.mZoneDetails )
		{
			local rowobj = {
				zoneID = row[0],
				name = row[1],
				terrainConfig = row[2],
				environmentType = row[3],
				mapName = row.len() > 5 ? row[5] : "",
				groupName = row.len() > 6 ? row[6] : "",
				instanceMode = row.len() > 7 ? row[7] : "",
				beefFactor = row.len() > 8 ? row[8] : "0",
				category = row.len() > 9 ? row[9] : "",
				regions = row.len() > 10 ? row[10] : "",
				displayName = row.len() > 11 ? row[11] : ""
			};

			if (rowobj.name == zonename)
			{
				return rowobj;
			}
		}

		return null;
	}

	function changeZoneName( zoneID, newname )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][1] = newname;
				break;
			}
		}

		for( i = 0; i < this.mRowContents.len(); i++ )
		{
			if (this.mRowContents[i][1].tointeger() == zoneID)
			{
				this.mRowContents[i][0] = newname;
				this._displayAllRows();
				this._recalcAllRowHeight();
				break;
			}
		}
	}

	function changeZoneTerrain( zoneID, newterrain )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][2] = newterrain;
				break;
			}
		}
	}

	function changeDisplayName( zoneID, displayName )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][11] = displayName;
				break;
			}
		}
	}

	function changeMapName( zoneID, mapName )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				if (this.mZoneDetails[i].len() > 5)
				{
					this.mZoneDetails[i][5] = mapName;
				}

				break;
			}
		}
	}

	function changeGroupName( zoneID, groupName )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				if (this.mZoneDetails[i].len() > 6)
				{
					this.mZoneDetails[i][6] = groupName;
				}

				break;
			}
		}
	}

	function changeRegions( zoneID, regions )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				if (this.mZoneDetails[i].len() > 6)
				{
					this.mZoneDetails[i][10] = regions;
				}

				break;
			}
		}
	}

	function changeZoneEnvironment( zoneID, newenvironment )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][3] = newenvironment;
				break;
			}
		}
	}

	function changeZoneInstanced( zoneID, which )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][7] = which;
				break;
			}
		}
	}

	function changeBeefFactor( zoneID, factor )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][8] = factor;
				break;
			}
		}
	}

	function changeCategory( zoneID, category )
	{
		local i;

		for( i = 0; i < this.mZoneDetails.len(); i++ )
		{
			if (this.mZoneDetails[i][0].tointeger() == zoneID)
			{
				this.mZoneDetails[i][9] = category;
				break;
			}
		}
	}

}

class this.UI.MarkerList extends this.GUI.ColumnList
{
	constructor( owner )
	{
		this.GUI.ColumnList.constructor();
		this.mOwner = owner;
		this.setOneSelectMin(true);
		this.addColumn("Name", 100);
		this.addActionListener(this);
	}

	function fetchList()
	{
		if (this.mTargetZone != null)
		{
			this._Connection.sendQuery("marker.list", this, [
				this.mTargetZone
			]);
		}
	}

	function onQueryComplete( qa, rows )
	{
		if (qa.query == "marker.list")
		{
			this.mMarkerDetails = rows;
			local selectedrows = this.getSelectedRows();
			local selectedmarker = "";

			if (selectedrows.len() > 0)
			{
				selectedmarker = this.getRow(selectedrows[0])[0];
			}

			this.removeAllRows();
			local rowindex = 0;

			foreach( row in rows )
			{
				local rowobj = {
					name = row[0],
					zoneID = row[1],
					position = row[2],
					heading = row[3]
				};
				this.addRow([
					rowobj.name
				]);

				if (rowobj.name == selectedmarker)
				{
					this.setSelectedRows([
						rowindex
					]);
				}

				rowindex++;
			}
		}
	}

	function onRowSelectionChanged( list, row, selected )
	{
		if (this.mOwner && selected)
		{
			local markername = this.getRow(row)[0];
			this.mOwner.onMarkerSelected(list, this.mTargetZone, markername);
		}
	}

	function onDoubleClick( list, evt )
	{
		this.mOwner.onMarkerDoubleClicked(evt);
	}

	function getMarkerDetails( markername )
	{
		foreach( row in this.mMarkerDetails )
		{
			local rowobj = {
				name = row[0],
				zoneID = row[1],
				position = row[2],
				heading = row[3]
			};

			if (rowobj.name == markername)
			{
				return rowobj;
			}
		}

		return null;
	}

	function setTargetZone( zone )
	{
		this.mTargetZone = zone;
		this.fetchList();
	}

	function changeMarkerName( markername, newname )
	{
		local i;

		for( i = 0; i < this.mMarkerDetails.len(); i++ )
		{
			if (this.mMarkerDetails[i][0] == markername)
			{
				this.mMarkerDetails[i][0] = newname;
				break;
			}
		}

		for( i = 0; i < this.mRowContents.len(); i++ )
		{
			if (this.mRowContents[i][0] == markername)
			{
				this.mRowContents[i][0] = newname;
				this._displayAllRows();
				this._recalcAllRowHeight();
				break;
			}
		}
	}

	function deleteMarker( markername )
	{
		local i;

		for( i = 0; i < this.mMarkerDetails.len(); i++ )
		{
			if (this.mMarkerDetails[i][0] == markername)
			{
				this.mMarkerDetails.remove(i);
				break;
			}
		}

		for( i = 0; i < this.mRowContents.len(); i++ )
		{
			if (this.mRowContents[i][0] == markername)
			{
				this.removeRow(i);
				break;
			}
		}
	}

	function changeMarkerPosition( markername, newpos )
	{
		local i;

		for( i = 0; i < this.mMarkerDetails.len(); i++ )
		{
			if (this.mMarkerDetails[i][0] == markername)
			{
				this.mMarkerDetails[i][2] = newpos;
				break;
			}
		}
	}

	function changeMarkerHeading( markername, newheading )
	{
		local i;

		for( i = 0; i < this.mMarkerDetails.len(); i++ )
		{
			if (this.mMarkerDetails[i][0] == markername)
			{
				this.mMarkerDetails[i][3] = newheading;
				break;
			}
		}
	}

	mOwner = null;
	mMarkerDetails = null;
	mTargetZone = null;
}

class this.UI.LootPackageList extends this.GUI.DropDownList
{
	owner = null;
	mLootPackages = null;
	constructor( incomingOwner )
	{
		this.GUI.DropDownList.constructor();
		this.owner = incomingOwner;
		this.fetchList();
		this.mLootPackages = [];
	}

	function fetchList()
	{
		this._Connection.sendQuery("lootpackages.list", this, [
			0,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			100,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			200,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			300,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			400,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			500,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			600,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			700,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			800,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			900,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1000,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1100,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1200,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1300,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1400,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1500,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1600,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1700,
			100
		]);
		this._Connection.sendQuery("lootpackages.list", this, [
			1800
		]);
	}

	function findLootPackages()
	{
		this._Connection.sendQuery("zone.list", this, [
			0,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			11,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			21,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			31,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			41,
			10
		]);
		this._Connection.sendQuery("zone.list", this, [
			51
		]);
	}

	function onQueryComplete( qa, rows )
	{
		if (qa.query == "lootpackages.list")
		{
			this.removeAll();
			this.mLootPackages.extend(rows);
			this.addChoice("-none-");

			foreach( lootPackage in this.mLootPackages )
			{
				this.addChoice(lootPackage[0]);
			}
		}
		else if (qa.query == "zone.list")
		{
			foreach( zone in rows )
			{
				if (zone[0].tointeger() == this.owner.getSelectedZoneId())
				{
					local lootPackage = zone[4];

					if (lootPackage == "")
					{
						this.setCurrent("-none-");
					}
					else
					{
						this.setCurrent(lootPackage);
					}
				}
			}
		}
	}

	function setSelected( lootPackage )
	{
		this.setCurrent(lootPackage);
	}

	function selectLootPackage( lootPackage, zoneID )
	{
		this._Connection.sendQuery("zone.edit", this, [
			zoneID,
			"lootPackage",
			lootPackage
		]);
	}

}

class this.Screens.ZoneTweakScreen extends this.GUI.Frame
{
	constructor()
	{
		this.GUI.Frame.constructor("Zone Tweak");
		this.setSize(450, 500);
		local contentcontainer = this.GUI.Container(this.GUI.BorderLayout());
		contentcontainer.setInsets(0);
		this.setContentPane(contentcontainer);
		local topcontainer = this.GUI.Container(this.GUI.BorderLayout());
		topcontainer.setInsets(5);
		topcontainer.setPreferredSize(340, 150);
		contentcontainer.add(topcontainer, this.GUI.BorderLayout.NORTH);
		local zonebuttonscontainer = this.GUI.Container(this.GUI.BoxLayoutV());
		zonebuttonscontainer.setInsets(3);
		topcontainer.add(zonebuttonscontainer, this.GUI.BorderLayout.EAST);
		local bottomcontainer = this.GUI.Container(this.GUI.BorderLayout());
		bottomcontainer.setInsets(5);
		contentcontainer.add(bottomcontainer, this.GUI.BorderLayout.CENTER);
		this.mZoneList = this.UI.ZoneList(this, this._sceneObjectManager.getCurrentZoneDefID());
		topcontainer.add(this.GUI.ScrollPanel(this.mZoneList));
		this.mRefreshButton = this._createButton("Refresh", "onRefresh");
		zonebuttonscontainer.add(this.mRefreshButton);
		this.mGotoZoneButton = this._createButton("Goto Zone", "onGotoZone");
		zonebuttonscontainer.add(this.mGotoZoneButton);
		this.mCreateZoneButton = this._createButton("Create Zone", "onCreateZone");
		zonebuttonscontainer.add(this.mCreateZoneButton);
		this.mCloneButton = this._createButton("Clone Zone", "onCloneZone");
		zonebuttonscontainer.add(this.mCloneButton);
		this.mLogButton = this._createButton("View Audit Log", "onViewAuditLog");
		zonebuttonscontainer.add(this.mLogButton);
		this.mTabs = this.GUI.TabbedPane();
		this.mPropertiesPage = this.GUI.Container(this.GUI.GridLayout(14, 2));
		this.mMarkersPage = this.GUI.Container(this.GUI.BorderLayout());
		this.mTabs.add(this.mMarkersPage, "Markers");
		this.mTabs.add(this.mPropertiesPage, "Properties");
		bottomcontainer.add(this.mTabs);
		this.mNameLabel = this.GUI.Label("Name");
		this.mNameTextbox = this.GUI.InputArea();
		this.mNameTextbox.addActionListener(this);
		this.mNameLabel.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mNameLabel);
		this.mPropertiesPage.add(this.mNameTextbox);
		this.mDisplayNameLabel = this.GUI.Label("Display Name");
		this.mDisplayNameTextbox = this.GUI.InputArea();
		this.mDisplayNameTextbox.addActionListener(this);
		this.mDisplayNameLabel.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mDisplayNameLabel);
		this.mPropertiesPage.add(this.mDisplayNameTextbox);
		this.mOwnerLabel = this.GUI.Label("Owner");
		this.mOwnerTextbox = this.GUI.InputArea();
		this.mOwnerTextbox.addActionListener(this);
		this.mOwnerTextbox.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mOwnerLabel);
		this.mPropertiesPage.add(this.mOwnerTextbox);
		this.mDescriptionLabel = this.GUI.Label("Description");
		this.mDescriptionTextbox = this.GUI.InputArea();
		this.mDescriptionTextbox.addActionListener(this);
		this.mDescriptionTextbox.setPreferredSize(175, 75);
		this.mPropertiesPage.add(this.mDescriptionLabel);
		this.mPropertiesPage.add(this.mDescriptionTextbox);
		this.mTerrainLabel = this.GUI.Label("Terrain");
		this.mTerrainTextbox = this.GUI.InputArea();
		this.mTerrainTextbox.addActionListener(this);
		this.mTerrainTextbox.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mTerrainLabel);
		this.mPropertiesPage.add(this.mTerrainTextbox);
		this.mEnvironmentLabel = this.GUI.Label("Environment Type");
		this.mEnvironmentDropList = this.GUI.DropDownList();
		this._populateEvironmentsList(this.mEnvironmentLabel);
		this.mEnvironmentDropList.addSelectionChangeListener(this);
		this.mEnvironmentDropList.setChangeMessage("onZoneEnvironmentChanged");
		this.mPropertiesPage.add(this.mEnvironmentLabel);
		this.mPropertiesPage.add(this.mEnvironmentDropList);
		this.mZoneLootPackageLabel = this.GUI.Label("Zone Loot Package");
		this.mZoneLootPackageList = this.UI.LootPackageList(this);
		this.mZoneLootPackageList.addSelectionChangeListener(this);
		this.mZoneLootPackageList.setChangeMessage("onZoneLootPackageChanged");
		this.mPropertiesPage.add(this.mZoneLootPackageLabel);
		this.mPropertiesPage.add(this.mZoneLootPackageList);
		this.mInstancedLabel = this.GUI.Label("Instanced");
		this.mInstancedList = this.GUI.DropDownList();
		this.mInstancedList.addSelectionChangeListener(this);
		this.mInstancedList.setChangeMessage("onZoneInstancedChanged");
		this.mInstancedList.addChoice("No");
		this.mInstancedList.addChoice("Yes");
		this.mPropertiesPage.add(this.mInstancedLabel);
		this.mPropertiesPage.add(this.mInstancedList);
		this.mBeefLabel = this.GUI.Label("Beef Factor");
		this.mBeefTextbox = this.GUI.InputArea();
		this.mBeefTextbox.addActionListener(this);
		this.mBeefTextbox.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mBeefLabel);
		this.mPropertiesPage.add(this.mBeefTextbox);
		this.mCategoryLabel = this.GUI.Label("Category");
		this.mCategoryTextbox = this.GUI.InputArea();
		this.mCategoryTextbox.setPreferredSize(120, 15);
		this.mCategoryTextbox.addActionListener(this);
		this.mPropertiesPage.add(this.mCategoryLabel);
		this.mPropertiesPage.add(this.mCategoryTextbox);
		this.mRegionsLabel = this.GUI.Label("Region Map");
		this.mRegionsTextbox = this.GUI.InputArea();
		this.mRegionsTextbox.setPreferredSize(120, 15);
		this.mRegionsTextbox.addActionListener(this);
		this.mPropertiesPage.add(this.mRegionsLabel);
		this.mPropertiesPage.add(this.mRegionsTextbox);
		this.mZoneLootPackageList.fetchList();
		this.mMapNameLabel = this.GUI.Label("Map Name");
		this.mMapNameTextbox = this.GUI.InputArea();
		this.mMapNameTextbox.addActionListener(this);
		this.mMapNameTextbox.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mMapNameLabel);
		this.mPropertiesPage.add(this.mMapNameTextbox);
		this.mGroupNameLabel = this.GUI.Label("Group Permission Name");
		this.mGroupNameTextbox = this.GUI.InputArea();
		this.mGroupNameTextbox.addActionListener(this);
		this.mGroupNameTextbox.setPreferredSize(120, 15);
		this.mPropertiesPage.add(this.mGroupNameLabel);
		this.mPropertiesPage.add(this.mGroupNameTextbox);
		this.mMarkerList = this.UI.MarkerList(this);
		this.mMarkersPage.add(this.GUI.ScrollPanel(this.mMarkerList), this.GUI.BorderLayout.CENTER);
		local rightmarkercontainer = this.GUI.Container(this.GUI.BorderLayout());
		rightmarkercontainer.setPreferredSize(225, 100);
		this.mMarkersPage.add(rightmarkercontainer, this.GUI.BorderLayout.EAST);
		local markerbuttoncontainer = this.GUI.Container(this.GUI.BoxLayoutV());
		markerbuttoncontainer.setInsets(3);
		rightmarkercontainer.add(markerbuttoncontainer, this.GUI.BorderLayout.NORTH);
		this.mCreateMarkerButton = this._createButton("Create Marker", "onCreateMarker");
		markerbuttoncontainer.add(this.mCreateMarkerButton);
		this.mDeleteMarkerButton = this._createButton("Delete Marker", "onDeleteMarker");
		markerbuttoncontainer.add(this.mDeleteMarkerButton);
		this.mSetMarkerButton = this._createButton("Set Marker from Avatar", "onSetMarkerFromAvatar");
		markerbuttoncontainer.add(this.mSetMarkerButton);
		this.mGotoMarkerButton = this._createButton("Goto Marker", "onGotoMarker");
		markerbuttoncontainer.add(this.mGotoMarkerButton);
		local markerpropertiescontainer = this.GUI.Container(this.GUI.GridLayout(3, 2));
		markerpropertiescontainer.setInsets(3);
		rightmarkercontainer.add(markerpropertiescontainer, this.GUI.BorderLayout.SOUTH);
		this.mMarkerNameLabel = this.GUI.Label("Name");
		this.mMarkerNameTextbox = this.GUI.InputArea();
		this.mMarkerNameTextbox.addActionListener(this);
		markerpropertiescontainer.add(this.mMarkerNameLabel);
		markerpropertiescontainer.add(this.mMarkerNameTextbox);
		this.mMarkerPositionLabel = this.GUI.Label("Position");
		this.mMarkerPositionTextbox = this.GUI.InputArea();
		this.mMarkerPositionTextbox.addActionListener(this);
		markerpropertiescontainer.add(this.mMarkerPositionLabel);
		markerpropertiescontainer.add(this.mMarkerPositionTextbox);
		this.mMarkerHeadingLabel = this.GUI.Label("Heading");
		this.mMarkerHeadingTextbox = this.GUI.InputArea();
		this.mMarkerHeadingTextbox.addActionListener(this);
		markerpropertiescontainer.add(this.mMarkerHeadingLabel);
		markerpropertiescontainer.add(this.mMarkerHeadingTextbox);
		this._sceneObjectManager.addListener(this);
	}

	function destroy()
	{
		this._sceneObjectManager.removeZoneUpdateListener(this);
	}

	function _createButton( label, msg )
	{
		local b = this.GUI.Button(label);
		b.setReleaseMessage(msg);
		b.addActionListener(this);
		return b;
	}

	function _populateEvironmentsList( list )
	{
		this.mEnvironmentDropList.removeAll();

		foreach( name, environment in ::Environments )
		{
			this.mEnvironmentDropList.addChoice(name);
		}
	}

	function onQueryComplete( qa, rows )
	{
		this.log.debug(this.DefaultQueryHandler.resultAsString(qa, rows));

		if (qa.query == "zone.edit")
		{
			if (qa.args[0] == "NEW")
			{
				this.mZoneList.fetchList(qa.args[2]);
			}

			if (qa.args.len() > 2 && qa.args[1] == "groupPermission")
			{
				local groupName = qa.args[2];
				this.mGroupNameTextbox.setText(groupName);
				this.mZoneList.changeGroupName(qa.args[0].tointeger(), groupName);
			}
		}
		else if (qa.query = "markers.edit")
		{
			this.mMarkerList.fetchList();
		}
	}

	function onQueryError( qa, reason )
	{
		if (qa.query == "zone.edit" && qa.args.len() > 2 && qa.args[1] == "groupPermission")
		{
			::_ChatWindow.addMessage("err/", reason, "General");
		}

		this.GUI.MessageBox.show("Server Error: " + reason);
	}

	function onInputComplete( inputbox )
	{
		if (inputbox == this.mNameTextbox)
		{
			this.onZoneNameChanged();
		}
		else if (inputbox == this.mDisplayNameTextbox)
		{
			this.onDisplayNameChanged();
		}
		else if (inputbox == this.mTerrainTextbox)
		{
			this.onZoneTerrainChanged();
		}
		else if (inputbox == this.mMarkerNameTextbox)
		{
			this.onMarkerNameChanged();
		}
		else if (inputbox == this.mMarkerPositionTextbox)
		{
			this.onMarkerPositionChanged();
		}
		else if (inputbox == this.mMapNameTextbox)
		{
			this.onMapNameChanged();
		}
		else if (inputbox == this.mBeefTextbox)
		{
			this.onBeefFactorChanged();
		}
		else if (inputbox == this.mCategoryTextbox)
		{
			this.onCategoryChanged();
		}
		else if (inputbox == this.mMarkerHeadingTextbox)
		{
			this.onMarkerHeadingChanged();
		}
		else if (inputbox == this.mRegionsTextbox)
		{
			this.onRegionsChanged();
		}
		else if (inputbox == this.mGroupNameTextbox)
		{
			this.onGroupPermissionNameChanged();
		}
	}

	function onZoneUpdate( newZoneID, newZoneDefID )
	{
		this.log.debug("Received Zone Update Event");

		if (this.mSelectedZoneID == newZoneDefID)
		{
			this.mCreateMarkerButton.setEnabled(true);
		}
		else
		{
			this.mCreateMarkerButton.setEnabled(false);
		}
	}

	function onRefresh( button )
	{
		this.mZoneList.fetchList();
	}

	function onGotoZone( button )
	{
		local selectedrows = this.mZoneList.getSelectedRows();

		if (selectedrows.len() > 0)
		{
			local selectedzone = this.mZoneList.getRow(selectedrows[0])[0];
			this._Connection.sendQuery("go", this, [
				selectedzone,
				0,
				0,
				0
			]);
		}
	}

	function onCreateZone( button )
	{
		this.GUI.MessageBox.showEx("Are you sure you wish to create a new zone?", [
			"Create",
			"Cancel"
		], this, "onCreateZoneConfirm");
	}

	function onCreateZoneConfirm( messageBox, alt )
	{
		if (alt == "Create")
		{
			local rand = this.Random(this.mTimer.getMilliseconds());
			local newname = "New_Zone_" + (rand.nextFloat() * 9000 + 1000).tointeger();
			this._Connection.sendQuery("zone.edit", this, [
				"NEW",
				"name",
				newname,
				"environmentType",
				"CloudyDay"
			]);
		}
	}

	function onCreateMarker( button )
	{
		this.mIgnoreEvents = true;
		local rand = this.Random(this.mTimer.getMilliseconds());
		local newname = "New_Marker_" + (rand.nextFloat() * 9000 + 1000).tointeger();
		this._Connection.sendQuery("marker.edit", this, [
			newname
		]);
		this.mMarkerList.addRow([
			newname
		]);
		local newindex = this.mMarkerList.getRowCount() - 1;
		this.mMarkerList.setSelectedRows([
			newindex
		]);
		this.mIgnoreEvents = false;
	}

	function onDeleteMarker( button )
	{
		this.GUI.MessageBox.showEx("Are you sure you wish to delete marker named " + this.mSelectedMarker + "?", [
			"Delete",
			"Cancel"
		], this, "onDeleteMarkerConfirmed");
	}

	function onDeleteMarkerConfirmed( messageBox, alt )
	{
		if (alt == "Delete")
		{
			this._Connection.sendQuery("marker.del", this, [
				this.mSelectedMarker
			]);
			this.mMarkerList.deleteMarker(this.mSelectedMarker);
		}
	}

	function onSetMarkerFromAvatar( button )
	{
		this.GUI.MessageBox.showEx("Are you sure you wish to set marker named " + this.mSelectedMarker + " to avatar\'s position?", [
			"Set",
			"Cancel"
		], this, "onSetMarkerConfirmed");
	}

	function onSetMarkerConfirmed( messageBox, alt )
	{
		if (alt == "Set")
		{
			this._Connection.sendQuery("marker.edit", this, [
				this.mSelectedMarker
			]);
		}
	}

	function onGotoMarker( button )
	{
		if (this.mSelectedMarker != "")
		{
			this._Connection.sendQuery("go", this, [
				this.mSelectedMarker
			]);
		}
	}

	function onZoneSelected( list, zonename, zoneid )
	{
		this.mIgnoreEvents = true;
		local zonedetails = this.mZoneList.getZoneDetails(zonename);
		this.mSelectedZoneName = zonename;
		this.mSelectedZoneID = zoneid.tointeger();
		this.mZoneLootPackageList.findLootPackages();

		if (zonedetails != null)
		{
			this.mNameTextbox.setText(zonedetails.name);
			this.mDisplayNameTextbox.setText(zonedetails.displayName);
			this.mTerrainTextbox.setText(zonedetails.terrainConfig);
			this.mEnvironmentDropList.setCurrent(zonedetails.environmentType);
			this.mInstancedList.setCurrent(zonedetails.instanceMode == "MULTIPLE" ? "Yes" : "No");
			this.mMapNameTextbox.setText(zonedetails.mapName);
			this.mGroupNameTextbox.setText(zonedetails.groupName);
			this.mBeefTextbox.setText(zonedetails.beefFactor);
			this.mCategoryTextbox.setText(zonedetails.category);
			this.mMarkerList.setTargetZone(zoneid);
			this.mRegionsTextbox.setText(zonedetails.regions);
		}
		else
		{
			this.mNameTextbox.setText("");
			this.mDisplayNameTextbox.setText("");
			this.mTerrainTextbox.setText("");
			this.mMapNameTextbox.setText("");
			this.mGroupNameTextbox.setText("");
			this.mCategoryTextbox.setText("");
			this.mBeefTextbox.setText("");
			this.mRegionsTextbox.setText("");
		}

		if (this.mSelectedZoneID == this._sceneObjectManager.getCurrentZoneDefID().tointeger())
		{
			this.mCreateMarkerButton.setEnabled(true);
		}
		else
		{
			this.mCreateMarkerButton.setEnabled(false);
		}

		this.mIgnoreEvents = false;
	}

	function onMarkerSelected( list, zonename, markername )
	{
		if (this.mIgnoreEvents == false)
		{
			local markerdetails = this.mMarkerList.getMarkerDetails(markername);
			this.mSelectedMarker = markername;

			if (markerdetails != null)
			{
				this.mMarkerNameTextbox.setText(markerdetails.name);
				this.mMarkerPositionTextbox.setText(markerdetails.position);
				this.mMarkerHeadingTextbox.setText(markerdetails.heading);
			}
			else
			{
				this.mMarkerNameTextbox.setText("");
				this.mMarkerPositionTextbox.setText("");
				this.mMarkerHeadingTextbox.setText("");
			}
		}
	}

	function onMarkerDoubleClicked( evt )
	{
		if (this.mSelectedMarker != "")
		{
			this._Connection.sendQuery("go", this, [
				this.mSelectedMarker
			]);
		}
	}

	function onZoneNameChanged()
	{
		local newname = this.mNameTextbox.getText();

		if (this.mSelectedZoneID.tointeger() >= 0 && newname != "")
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"name",
				newname
			]);
			this.mZoneList.changeZoneName(this.mSelectedZoneID, newname);
		}
	}

	function onZoneTerrainChanged()
	{
		local newterrain = this.mTerrainTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"terrain",
				newterrain
			]);
			this.mZoneList.changeZoneTerrain(this.mSelectedZoneID, newterrain);
		}
	}

	function onDisplayNameChanged()
	{
		local displayName = this.mDisplayNameTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"displayName",
				displayName
			]);
			this.mZoneList.changeDisplayName(this.mSelectedZoneID, displayName);
		}
	}

	function onMapNameChanged()
	{
		local mapName = this.mMapNameTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"mapName",
				mapName
			]);
			this.mZoneList.changeMapName(this.mSelectedZoneID, mapName);
		}
	}

	function onGroupPermissionNameChanged()
	{
		local groupName = this.mGroupNameTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"groupPermission",
				groupName
			]);
		}
	}

	function onZoneEnvironmentChanged( list )
	{
		if (this.mIgnoreEvents == false)
		{
			local newenvironment = this.mEnvironmentDropList.getCurrent();

			if (this.mSelectedZoneID >= 0)
			{
				this._Connection.sendQuery("zone.edit", this, [
					this.mSelectedZoneID,
					"environmentType",
					newenvironment
				]);
				this.mZoneList.changeZoneEnvironment(this.mSelectedZoneID, newenvironment);
			}
		}
	}

	function onZoneLootPackageChanged( list )
	{
		this.mZoneLootPackageList.selectLootPackage(list.getCurrent(), this.mSelectedZoneID);
	}

	function onZoneInstancedChanged( list )
	{
		local which = list.getCurrent() == "Yes" ? "MULTIPLE" : "SINGLE";
		::_Connection.sendQuery("zone.edit", this, [
			this.mSelectedZoneID,
			"instancing",
			which
		]);
		this.mZoneList.changeZoneInstanced(this.mSelectedZoneID, which);
	}

	function onBeefFactorChanged()
	{
		local beefFactor = this.mBeefTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"beefFactor",
				beefFactor
			]);
			this.mZoneList.changeBeefFactor(this.mSelectedZoneID, beefFactor);
		}
	}

	function onCategoryChanged()
	{
		local category = this.mCategoryTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"category",
				category
			]);
			this.mZoneList.changeCategory(this.mSelectedZoneID, category);
		}
	}

	function onRegionsChanged()
	{
		local regions = this.mRegionsTextbox.getText();

		if (this.mSelectedZoneID >= 0)
		{
			this._Connection.sendQuery("zone.edit", this, [
				this.mSelectedZoneID,
				"regions",
				regions
			]);
			this.mZoneList.changeRegions(this.mSelectedZoneID, regions);
		}
	}

	function onMarkerNameChanged()
	{
		local newname = this.mMarkerNameTextbox.getText();

		if (this.mSelectedMarker != "" && newname != "")
		{
			this._Connection.sendQuery("marker.edit", this, [
				this.mSelectedMarker,
				"n",
				newname
			]);
			this.mMarkerList.changeMarkerName(this.mSelectedMarker, newname);
			this.mSelectedMarker = newname;
		}
	}

	function onMarkerPositionChanged()
	{
		local newpos = this.mMarkerPositionTextbox.getText();

		if (this.mSelectedMarker != "" && newpos != "")
		{
			this._Connection.sendQuery("marker.edit", this, [
				this.mSelectedMarker,
				"p",
				newpos
			]);
			this.mMarkerList.changeMarkerPosition(this.mSelectedMarker, newpos);
		}
	}

	function onMarkerHeadingChanged()
	{
		local newheading = this.mMarkerHeadingTextbox.getText();

		if (this.mSelectedMarker != "" && newheading != "")
		{
			this._Connection.sendQuery("marker.edit", this, [
				this.mSelectedMarker,
				"h",
				newheading
			]);
			this.mMarkerList.changeMarkerHeading(this.mSelectedMarker, newheading);
		}
	}

	function onCloneZone( ... )
	{
		if (this.mSelectedZoneID >= 0)
		{
			this.GUI.MessageBox.showYesNo("Are you sure you want to clone the contents of zone \'" + this.mSelectedZoneName + "\' into the current zone?", this, "onCloneConfirmed");
		}
	}

	function onViewAuditLog( ... )
	{
		local zoneId = this.getSelectedZoneId();
		this.System.openURL("https://secure.sparkplaymedia.com/ee/auditlog/logview.php?type=ZoneDef&id=" + zoneId);
	}

	function onCloneConfirmed( button, text )
	{
		if (text == "Yes")
		{
			::_Connection.sendQuery("zone.edit", this, [
				"CLONE_CONTENTS",
				this.mSelectedZoneID
			]);
		}
	}

	function onMousePressed( evt )
	{
		evt.consume();
	}

	function getSelectedZone()
	{
		return this.mSelectedZoneName;
	}

	function getSelectedZoneId()
	{
		return this.mSelectedZoneID;
	}

	mZoneList = null;
	mRefreshButton = null;
	mGotoZoneButton = null;
	mCreateZoneButton = null;
	mTabs = null;
	mPropertiesPage = null;
	mMarkersPage = null;
	mNameLabel = null;
	mNameTextbox = null;
	mDisplayNameLabel = null;
	mDisplayNameTextbox = null;
	mOwnerLabel = null;
	mOwnerTextbox = null;
	mDescriptionLabel = null;
	mDescriptionTextbox = null;
	mTerrainLabel = null;
	mTerrainTextbox = null;
	mEnvironmentLabel = null;
	mEnvironmentDropList = null;
	mBeefLabel = null;
	mBeefTextbox = null;
	mCategoryLabel = null;
	mCategoryTextbox = null;
	mRegionsLabel = null;
	mRegionsTextbox = null;
	mMarkerList = null;
	mCreateMarkerButton = null;
	mDeleteMarkerButton = null;
	mGotoMarkerButton = null;
	mSetMarkerButton = null;
	mCloneButton = null;
	mLogButton = null;
	mInstancedLabel = null;
	mInstancedList = null;
	mMarkerNameLabel = null;
	mMarkerNameTextbox = null;
	mMarkerPositionLabel = null;
	mMarkerPositionTextbox = null;
	mMarkerHeadingLabel = null;
	mMarkerHeadingTextbox = null;
	mMapNameLabel = null;
	mMapNameTextbox = null;
	mGroupNameLabel = null;
	mGroupNameTextbox = null;
	mZoneLootPackageLabel = null;
	mZoneLootPackageList = null;
	mSelectedZoneName = "";
	mSelectedZoneID = -1;
	mSelectedMarker = "";
	mTimer = ::Timer();
	mIgnoreEvents = false;
}

