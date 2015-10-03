this.require("UI/TerrainPainting");
this.require("UI/Screens");

if (!("TerrainTextures" in this.getroottable()))
{
	this.TerrainTextures <- [
		"Mud_01.png",
		"Grass_01.png",
		"Grass_02.png",
		"Rock_01.png",
		"Sand_01.png"
	];
}

class this.Screens.TerrainSplatSelector extends this.GUI.Frame
{
	static mClassName = "Screens.TerrainSplatSelector";
	mCurrentPage = null;
	mSplatSelectors = null;
	mUpdatingSelectors = false;
	mPageX = null;
	mPageZ = null;
	constructor()
	{
		this.GUI.Frame.constructor("Terrain Texture Selection");
		local main = this.GUI.Container(this.GUI.BorderLayout());
		main.setInsets(5);
		local splats = [];

		if ("TerrainTextures" in this.getroottable())
		{
			foreach( t in ::TerrainTextures )
			{
				t = this.File.basename(t, ".png");
				splats.append(t);
			}
		}

		this.mSplatSelectors = [];
		this.mSplatSelectors.append(this._createSplatChoices(splats));
		this.mSplatSelectors.append(this._createSplatChoices(splats));
		this.mSplatSelectors.append(this._createSplatChoices(splats));
		this.mSplatSelectors.append(this._createSplatChoices(splats));
		this.mCurrentPage = this.GUI.Label("Current Textures for Page XX, ZZ");
		this.mCurrentPage.setTextAlignment(0.5, 0.5);
		this.mCurrentPage.setInsets(4);
		main.add(this.mCurrentPage, this.GUI.BorderLayout.NORTH);
		local p = this.GUI.Container(this.GUI.GridLayout(4, 2));

		foreach( i, s in this.mSplatSelectors )
		{
			p.add(this.GUI.Label("Texture " + i), 0);
			p.add(s);
		}

		main.add(p, this.GUI.BorderLayout.CENTER);
		this.mPageX = null;
		this.mPageZ = null;
		this.setContentPane(main);
		this.setSticky("right", "bottom");
		this.setPosition(this.getWidth() * -1, this.getHeight() * -1);
		this.setSize(this.getPreferredSize());
		this.setPosition(0, 0);
	}

	function _reshapeNotify()
	{
		this.mX = -this.mWidth;
		this.mY = -this.mHeight - this.Screens.get("BuildScreen", true).getHeight();
		this.GUI.Frame._reshapeNotify();
	}

	function getCurrentSplats()
	{
		local splats = [];

		foreach( s in this.mSplatSelectors )
		{
			splats.append(s.getCurrent());
		}

		return splats;
	}

	function _createSplatChoices( splats )
	{
		local c = this.GUI.DropDownList();

		foreach( t in splats )
		{
			c.addChoice(t);
		}

		c.addSelectionChangeListener(this);
		return c;
	}

	function _getPageMaterial()
	{
		return "Terrain/Splatting[" + this.mPageX + "," + this.mPageZ + "]";
	}

	function onSelectionChange( choiceList )
	{
		if (this.mUpdatingSelectors)
		{
			return;
		}

		local textures = {};

		foreach( i, s in this.mSplatSelectors )
		{
			textures["Splatting" + i] <- s.getCurrent() + ".png";
		}

		local info = this.Util.getTerrainPathInfo();
		local filename = info[1] + info[0] + "_x" + this.mPageX + "y" + this.mPageZ + ".cfg";

		try
		{
			::_scene.setTerrainPageTextures(this.mPageX, this.mPageZ, textures);
			::_scene.saveTerrainConfig(this.mPageX, this.mPageZ, filename);
		}
		catch( err )
		{
			this.log.error("Error writing terrain textures to " + filename + ": " + err);
		}

		local s = this.Screens.get("BuildScreen", true);
		s._updateSplatButtons(this.getCurrentSplats());
	}

	function setPage( pageX, pageZ )
	{
		this.mPageX = pageX;
		this.mPageZ = pageZ;
		this.mCurrentPage.setText("Current Textures for Page " + this.mPageX + ", " + this.mPageZ);

		try
		{
			local textures = ::_root.getMaterialTextures(this._getPageMaterial());
			this.mUpdatingSelectors = true;

			for( local i = 0; i < 4; i++ )
			{
				local t = this.File.basename(textures["Splatting" + i], ".png");
				this.mSplatSelectors[i].setCurrent(t);
			}

			this.mUpdatingSelectors = false;
		}
		catch( err )
		{
			this.log.error("Error fetching terrain textures: " + err);
		}
	}

}

class this.Screens.TerrainHeightSelector extends this.GUI.Frame
{
	static mClassName = "Screens.TerrainHeightSelector";
	mCurrentPage = null;
	mSplatSelectors = null;
	mUpdatingSelectors = false;
	mPageX = null;
	mPageZ = null;
	mEntry = null;
	constructor()
	{
		this.GUI.Frame.constructor("Page Height");
		local main = this.GUI.Container(this.GUI.GridLayout(1, 2));
		main.setInsets(3);
		local label = this.GUI.Label("Adjustment: ");
		main.setInsets(2);
		main.add(label);
		this.mEntry = this.GUI.InputArea();
		this.mEntry.setAllowOnlyNumbers(true);
		this.mEntry.addActionListener(this);
		main.add(this.mEntry);
		this.setContentPane(main);
		this.setSticky("right", "bottom");
		this.setPosition(this.getWidth() * -1, this.getHeight() * -1);
		this.setPosition(0, 0);
		this.setSize(200, 50);
	}

	function onInputComplete( button )
	{
		if (::_buildTool == null || ::_buildTool.mBuildNode == null)
		{
			return;
		}

		local pos = ::_buildTool.mBuildNode.getPosition();
		::_root.terrainHeightEdit(pos.x, pos.z, this.mEntry.getValue());
		::_scene.resetClutter();
		local tpos = this.Util.getTerrainPageIndex(pos);
		::_buildTool.markPageDirty("Height", tpos.x, tpos.z);
		::_buildTool.selectPaintTool().setStatus();
	}

	function _reshapeNotify()
	{
		this.mX = -this.mWidth;
		this.mY = -this.mHeight - this.Screens.get("BuildScreen", true).getHeight();
		this.GUI.Frame._reshapeNotify();
	}

}

class this.Screens.BuildScreen extends this.GUI.Panel
{
	mPropertyPanel = null;
	mStatusText = null;
	mStatusText2 = null;
	mInsertAsset = null;
	mInsertSpawn = null;
	mTemplateInsertAsset = null;
	mRealInsertAsset = null;
	mSelector = null;
	mPageContainer = null;
	mSplatButtons = null;
	mPages = null;
	mTerrainPageX = null;
	mTerrainPageZ = null;
	mGridLinesOn = false;
	mSnapToGrid = false;
	mSnapToFloor = true;
	mSnapToGridBtn = null;
	mSnapToFloorBtn = null;
	mEnvEntry = null;
	static mClassName = "Screens.BuildScreen";
	constructor()
	{
		this.GUI.Panel.constructor(this.GUI.BorderLayout());
		this.mPages = {};
		this.mPages.Scenery <- this._createSceneryPage();
		this.mPages.Template <- this._createTemplatePage();

	print("ICE: BUILD SCREEN: dev: " + this.Util.isDevMode()+ " hyb: " + this.Util.isHybridMode() + "\n"); 
		if (this.Util.isDevMode())
		{
			this.mPages["Terrain Texture"] <- this._createTerrainTexturePage();
			this.mPages["Terrain Height"] <- this._createTerrainHeightPage();
			this.mPages.Water <- this._createWaterPage();
			this.mPages.Environment <- this._createEnvironmentPage();
			this.mPages.Spawners <- this._createSpawnPage();
		}

		this.mSelector = this.GUI.DropDownList();
		this.mSelector.addChoice("Scenery");
		this.mSelector.addChoice("Template");

		if (this.Util.isDevMode())
		{
			this.mSelector.addChoice("Terrain Texture");
			this.mSelector.addChoice("Terrain Height");
			this.mSelector.addChoice("Water");
			this.mSelector.addChoice("Environment");
			this.mSelector.addChoice("Spawners");
		}

		this.mSelector.addSelectionChangeListener(this);
		this.mPageContainer = this.GUI.Container(this.GUI.BorderLayout());
		this.mPageContainer.setInsets(0, 0, 0, 3);
		this.mPageContainer.add(this.mPages.Scenery);
		local p = this.GUI.Container(this.GUI.BorderLayout());
		p.add(this.mSelector, this.GUI.BorderLayout.WEST);
		p.add(this.mPageContainer);
		this.add(p, this.GUI.BorderLayout.NORTH);
		p = this.GUI.Container(this.GUI.BorderLayout());
		this.mStatusText = this.GUI.HTML("Build mode");
		this.mStatusText2 = this.GUI.HTML("");
		p.add(this.mStatusText, this.GUI.BorderLayout.CENTER);
		p.add(this.mStatusText2, this.GUI.BorderLayout.EAST);
		this.add(p, this.GUI.BorderLayout.SOUTH);
		this.mPropertyPanel = this.GUI.PropertyPanel();
		local sz = this.getPreferredSize();
		sz.width = this.Math.max(sz.width, 450);
		this.setSize(sz);
		this.setSticky("right", "bottom");
		this.setPosition(this.getWidth() * -1, this.getHeight() * -1);
		this.setVisible(false);
		this.setOverlay(this.GUI.OVERLAY);
	}

	function setVisible( val )
	{
		this.GUI.Panel.setVisible(val);

		if (this.mPropertyPanel != null && val == false)
		{
			this.mPropertyPanel.setVisible(false);
		}

		if (this.mGridLinesOn == true && val == true)
		{
			this._scene.setTerrainGridVisible(true);
			this._scene.setTerrainLODEnabled(false);
		}
		else if (this.mGridLinesOn == true && val == false)
		{
			this._scene.setTerrainGridVisible(false);
			this._scene.setTerrainLODEnabled(true);
		}
	}

	function _createSceneryPage()
	{
		local p2 = this.GUI.Container(this.GUI.BorderLayout());
		local radio = this.GUI.RadioGroup();
		local panel = this.GUI.Container(this.GUI.BoxLayout());
		panel.insets.right = 3;
		panel.add(this._makeButton(radio, "Move", "onSceneryMovePressed"));
		panel.add(this._makeButton(radio, "Insert", "onSceneryInsertPressed"));
		this.mSnapToGridBtn = this._makeButton(null, "G", "onGridPressed");
		this.mSnapToFloorBtn = this._makeButton(null, "F", "onFloorPressed");
		this.mSnapToGridBtn.setToggledExplicit(this.mSnapToGrid);
		this.mSnapToFloorBtn.setToggledExplicit(this.mSnapToFloor);
		panel.add(this.mSnapToGridBtn);
		panel.add(this.mSnapToFloorBtn);
		p2.add(panel, this.GUI.BorderLayout.WEST);
		this.mInsertAsset = this.GUI.AssetRefInputBox();
		p2.add(this.mInsertAsset, this.GUI.BorderLayout.CENTER);
		return p2;
	}

	function _createSpawnPage()
	{
		local p2 = this.GUI.Container(this.GUI.BorderLayout());
		local radio = this.GUI.RadioGroup();
		local panel = this.GUI.Container(this.GUI.BoxLayout());
		panel.insets.right = 3;
		panel.add(this._makeButton(radio, "Insert", "onSpawnerInsertPressed"));
		p2.add(panel, this.GUI.BorderLayout.WEST);
		this.mInsertSpawn = this.GUI.SpawnerRefInputBox();
		p2.add(this.mInsertSpawn, this.GUI.BorderLayout.CENTER);
		return p2;
	}

	function onSpawnerInsertPressed( button )
	{
		if (::_buildTool.mBuildNode == null)
		{
			return;
		}

		local av_pos = ::_buildTool.mBuildNode.getPosition();
		local str_pos = av_pos.x.tostring() + " " + av_pos.y.tostring() + " " + av_pos.z.tostring();
		local name = this.mInsertSpawn.getText();
		local type = this.mInsertSpawn.getType();
		::_Connection.sendQuery("spawn.create", this, [
			type,
			name,
			str_pos
		]);
	}

	function _createTemplatePage()
	{
		local p2 = this.GUI.Container(this.GUI.BorderLayout());
		local radio = this.GUI.RadioGroup();
		local panel = this.GUI.Container(this.GUI.BoxLayout());
		panel.insets.right = 3;
		panel.add(this._makeButton(radio, "Set Template", "onTemplateSetPressed"));
		panel.add(this._makeButton(radio, "Insert", "onTemplateInsertPressed"));
		p2.add(panel, this.GUI.BorderLayout.WEST);
		this.mTemplateInsertAsset = this.GUI.TemplateRefInputBox();
		p2.add(this.mTemplateInsertAsset, this.GUI.BorderLayout.CENTER);
		return p2;
	}

	function _createTerrainTexturePage()
	{
		local p = this.GUI.Container(this.GUI.BorderLayout());
		local sp = this.GUI.Container(this.GUI.GridLayout(1, 4));
		local radio = this.GUI.RadioGroup();
		this.mSplatButtons = [];

		for( local i = 0; i < 4; i++ )
		{
			local b = this._makeButton(radio, "" + i, "onSplatPressed");
			this.mSplatButtons.append(b);
			sp.add(b);
		}

		p.add(sp, this.GUI.BorderLayout.CENTER);
		p.add(this._makeButton(null, "...", "onSplatChangePressed"), this.GUI.BorderLayout.EAST);
		return p;
	}

	function _createTerrainHeightPage()
	{
		local p = this.GUI.Container(this.GUI.BoxLayout());
		local radio = this.GUI.RadioGroup();
		p.add(this._makeButton(radio, "Raise/Lower", "onTerrainHeightAction"));
		p.add(this._makeButton(radio, "Flatten", "onTerrainHeightAction"));
		p.add(this._makeButton(radio, "Smooth", "onTerrainHeightAction"));
		p.add(this._makeButton(radio, "Noise", "onTerrainHeightAction"));
		p.add(this._makeButton(radio, "Adjust Height", "onTerrainHeightAction"));
		return p;
	}

	mCopyButton = null;
	mPasteButton = null;
	mElevationEntry = null;
	mMaterialEntry = null;
	mCopyMaterial = null;
	mCopyElevation = null;
	function _createWaterPage()
	{
		local main = this.GUI.Container(this.GUI.BoxLayout());
		local label = this.GUI.Label("Material:");
		main.add(label);
		this.mMaterialEntry = this.GUI.InputArea();
		this.mMaterialEntry.addActionListener(this);
		this.mMaterialEntry.setWidth(100);
		main.add(this.mMaterialEntry);
		label = this.GUI.Label("Elevation:");
		main.add(label);
		this.mElevationEntry = this.GUI.NumericInputBox();
		this.mElevationEntry.addActionListener(this);
		this.mElevationEntry.setMinimumSize({
			width = 40,
			height = 20
		});
		main.add(this.mElevationEntry);
		this.mCopyButton = this.GUI.Button("Copy");
		this.mCopyButton.addActionListener(this);
		this.mCopyButton.setReleaseMessage("onWaterCopy");
		main.add(this.mCopyButton);
		this.mPasteButton = this.GUI.Button("Paste");
		this.mPasteButton.addActionListener(this);
		this.mPasteButton.setReleaseMessage("onWaterPaste");
		main.add(this.mPasteButton);
		return main;
	}

	function _createEnvironmentPage()
	{
		local main = this.GUI.Container(this.GUI.BoxLayout());
		local label = this.GUI.Label("Set Environment:");
		main.add(label);
		this.mEnvEntry = this.GUI.InputArea();
		this.mEnvEntry.addActionListener(this);
		this.mEnvEntry.setWidth(100);
		main.add(this.mEnvEntry);
		return main;
	}

	function onWaterCopy( button )
	{
		this.mCopyMaterial = this.mMaterialEntry.getText();
		this.mCopyElevation = this.mElevationEntry.getValue();
	}

	function onWaterPaste( button )
	{
		if (this.mCopyMaterial != null)
		{
			this.mMaterialEntry.setText(this.mCopyMaterial);
			this.onInputComplete(this.mMaterialEntry);
		}

		if (this.mCopyElevation != null)
		{
			this.mElevationEntry.setValue(this.mCopyElevation);
			this.onInputComplete(this.mElevationEntry);
		}
	}

	function onInputComplete( entry )
	{
		if (this.mTerrainPageX == null || this.mTerrainPageZ == null)
		{
			return;
		}

		if (::_buildTool == null || ::_buildTool.mBuildNode == null)
		{
			return;
		}

		local pos = ::_buildTool.mBuildNode.getPosition();

		if (entry == this.mEnvEntry)
		{
			local info = this.Util.getTerrainPathInfo();
			local name = "x" + this.mTerrainPageX + "y" + this.mTerrainPageZ;
			local filename = info[1] + info[0] + "_" + name + ".nut";
			local env = entry.getText();
			local out = "TerrainPageDef[\"" + name + "\"] <- { Environment=\"" + env + "\" };";
			::System.writeToFile(filename, out);
			local terrain = ::_sceneObjectManager.getCurrentTerrainBase();

			if (!(terrain in ::TerrainEnvDef))
			{
				::TerrainEnvDef[terrain] <- {};
			}

			local tmp = ::TerrainEnvDef[terrain];
			tmp[name] <- {
				Environment = env
			};
			::_Environment.setForceNextUpdate(true);
			::_Environment.update();
			return;
		}

		if (entry == this.mElevationEntry)
		{
			::_scene.setTerrainWaterElevation(this.mTerrainPageX, this.mTerrainPageZ, this.mElevationEntry.getValue());
		}

		if (entry == this.mMaterialEntry)
		{
			::_scene.setTerrainWaterMaterial(this.mTerrainPageX, this.mTerrainPageZ, this.mMaterialEntry.getText());
		}

		local info = this.Util.getTerrainPathInfo();
		local filename = info[1] + info[0] + "_x" + this.mTerrainPageX + "y" + this.mTerrainPageZ + ".cfg";
		::_scene.saveTerrainConfig(this.mTerrainPageX, this.mTerrainPageZ, filename);
	}

	function _makeButton( radio, text, releaseMsg )
	{
		local b = this.GUI.Button(text);

		if (radio != null)
		{
			b.setRadioGroup(radio);
		}

		b.setPressMessage(releaseMsg);
		b.addActionListener(this);
		return b;
	}

	function onSceneryMovePressed( button )
	{
		this._buildTool.selectSceneryTool();
	}

	function onSceneryInsertPressed( button )
	{
		this.doBuildInsert();
	}

	function onGridPressed( button )
	{
		this.mSnapToGrid = !this.mSnapToGrid;
		this.mSnapToGridBtn.setToggledExplicit(this.mSnapToGrid);
		this._buildTool.getSceneryTool().snapToGrid(this.mSnapToGrid);
	}

	function onFloorPressed( button )
	{
		this.mSnapToFloor = !this.mSnapToFloor;
		this.mSnapToFloorBtn.setToggledExplicit(this.mSnapToFloor);
		this._buildTool.getSceneryTool().snapToFloor(this.mSnapToFloor);
	}

	function onTemplateSetPressed( button )
	{
		local sceneSelection = this._buildTool.getSelection();
		local text = this.mTemplateInsertAsset.getText();

		if (sceneSelection.objects().len() == 0 || text.len() == 0)
		{
			return;
		}

		local centerPosition = this.getCentroidOfAllSelectedObjects(sceneSelection.objects());
		local objectArray = [];

		foreach( so in sceneSelection.objects() )
		{
			local newpos = so.getPosition();
			newpos.x = newpos.x - centerPosition.x;
			newpos.y = newpos.y - centerPosition.y;
			newpos.z = newpos.z - centerPosition.z;
			local objPosition = {
				x = newpos.x,
				y = newpos.y,
				z = newpos.z
			};
			local objScale = {
				x = so.getScale().x,
				y = so.getScale().y,
				z = so.getScale().z
			};
			local objOrientation = {
				w = so.getOrientation().w,
				x = so.getOrientation().x,
				y = so.getOrientation().y,
				z = so.getOrientation().z
			};
			local objAsset = so.getTypeString();
			local newObj = {
				position = [
					objPosition.x,
					objPosition.y,
					objPosition.z
				],
				asset = objAsset
			};

			if (objScale.x != 1.0 || objScale.y != 1.0 || objScale.z != 1.0)
			{
				newObj.scale <- [
					objScale.x,
					objScale.y,
					objScale.z
				];
			}

			if (objOrientation.x != 0.0 || objOrientation.y != 0.0 || objOrientation.z != 0.0 || objOrientation.w != 1.0)
			{
				newObj.orientation <- [
					objOrientation.x,
					objOrientation.y,
					objOrientation.z,
					objOrientation.w
				];
			}

			objectArray.append(newObj);
		}

		local text = this.mTemplateInsertAsset.getText();
		this._buildTool.templateSaveQuery(text, objectArray);
	}

	function onTemplateInsertPressed( button )
	{
		local text = this.mTemplateInsertAsset.getText();
		this._buildTool.templateGetQuery(text);
	}

	function getCentroidOfAllSelectedObjects( objects )
	{
		local xTotal = 0;
		local yTotal = 0;
		local zTotal = 0;

		foreach( so in objects )
		{
			local position = so.getPosition();
			xTotal = xTotal + position.x;
			yTotal = yTotal + position.y;
			zTotal = zTotal + position.z;
		}

		local numObjects = objects.len();
		local centerPosition = this.Vector3(xTotal / numObjects, yTotal / numObjects, zTotal / numObjects);
		local increment = this.gBuildTranslateSnap;
		centerPosition.x = (centerPosition.x / increment).tointeger() * increment;
		centerPosition.z = (centerPosition.z / increment).tointeger() * increment;
		return centerPosition;
	}

	function triggerBuildTemplateList()
	{
		this.mTemplateInsertAsset.getTemplateList();
	}

	function onSplatPressed( button )
	{
		local t = this._buildTool.selectPaintTool();
		local tx = button.getText() + ".png";
		t.setPaintHandler(this.TerrainPaintHandler.Splat(tx));
	}

	function onSplatChangePressed( button )
	{
		local s = this.Screens.toggle("TerrainSplatSelector");

		if (s.isVisible())
		{
			s.setPage(this.mTerrainPageX, this.mTerrainPageZ);
		}
	}

	function onTerrainHeightAction( button )
	{
		local t = this._buildTool.selectPaintTool();

		switch(button.getText())
		{
		case "Raise/Lower":
			t.setPaintHandler(this.TerrainPaintHandler.RaiseAndLower());
			break;

		case "Flatten":
			t.setPaintHandler(this.TerrainPaintHandler.Flatten());
			break;

		case "Smooth":
			t.setPaintHandler(this.TerrainPaintHandler.Smooth());
			break;

		case "Noise":
			t.setPaintHandler(this.TerrainPaintHandler.Noise());
			break;

		case "Adjust Height":
			this.Screens.show("TerrainHeightSelector");
			break;
		}
	}

	function resetButtons( layout )
	{
		foreach( b in this.mSplatButtons )
		{
			b.setToggled(false);
		}

		foreach( b in layout.components )
		{
			try
			{
				b.setToggled(false);
			}
			catch( error )
			{
			}
		}
	}

	function onSelectionChange( list )
	{
		local pageName = this.mSelector.getCurrent();
		local page = this.mPages[pageName];
		::_enterFrameRelay.removeListener(this);
		::_buildTool.selectPaintTool().setPaintHandler(null);
		this.resetButtons(page);
		this.mPageContainer.removeAll();
		this.mPageContainer.add(page, this.GUI.BorderLayout.CENTER);

		if ("_buildTool" in this.getroottable())
		{
			if (pageName == "Scenery" || pageName == "Template")
			{
				this._buildTool.selectSceneryTool();

				if (this.mGridLinesOn == true)
				{
					this._scene.setTerrainGridVisible(false);
					this._scene.setTerrainLODEnabled(true);
					this.mGridLinesOn = false;
				}
			}
			else if (pageName == "Spawners")
			{
				this._buildTool.selectSceneryTool(true);

				if (this.mGridLinesOn == true)
				{
					this._scene.setTerrainGridVisible(false);
					this._scene.setTerrainLODEnabled(true);
					this.mGridLinesOn = false;
				}
			}
			else
			{
				this._buildTool.selectPaintTool();
				::_enterFrameRelay.addListener(this);

				if (pageName == "Environment")
				{
					this.updateEnvironmentFromSceneNode(::_avatar.getNode());
				}
				else if (pageName == "Terrain Height" && this.mGridLinesOn == false)
				{
					this._scene.setTerrainGridVisible(true);
					this._scene.setTerrainLODEnabled(false);
					this.mGridLinesOn = true;
				}
				else if (this.mGridLinesOn == true)
				{
					this._scene.setTerrainGridVisible(false);
					this._scene.setTerrainLODEnabled(false);
					this.mGridLinesOn = false;
				}
			}
		}
	}

	function setInsertAsset( asset )
	{
		this.mInsertAsset.setAsset(asset);
		this.setStatusText("");
	}

	function updateEnvironmentFromSceneNode( node )
	{
		local pos = node.getWorldPosition();
		local tpos = this.Util.getTerrainPageIndex(pos);

		if (tpos)
		{
			local terrain = ::_sceneObjectManager.getCurrentTerrainBase();
			local name = "x" + tpos.x + "y" + tpos.z;

			if (terrain in ::TerrainEnvDef)
			{
				local pageMap = ::TerrainEnvDef[terrain];

				if (name in pageMap)
				{
					this.mEnvEntry.setText(pageMap[name].Environment);
				}
				else
				{
					this.mEnvEntry.setText("");
				}
			}
			else
			{
				this.mEnvEntry.setText("");
			}
		}
	}

	function getInsertAsset()
	{
		return this.mInsertAsset.getValue();
	}

	function doBuildInsert()
	{
		local asset = this.getInsertAsset();

		if (typeof asset == "array")
		{
			this.setStatusText("Ambiguous asset (" + asset.len() + " matches)");
		}
		else if (asset == null)
		{
			this.setStatusText("Invalid asset (not found in catalog)");
		}
		else
		{
			this._buildTool.insertScenery(asset);
		}
	}

	function _reshapeNotify()
	{
		this.mX = -this.mWidth;
		this.mY = -this.mHeight;
		this.GUI.Component._reshapeNotify();
	}

	function destroy()
	{
		this.mPropertyPanel.destroy();
		this.mPropertyPanel = null;
		this.GUI.Panel.destroy();
	}

	function updateSelectedObject( so )
	{
		this.mPropertyPanel.setBean(so);
		this.mPropertyPanel.setVisible(so != null);
	}

	function getSelectedObject()
	{
		return this.mPropertyPanel.getBean();
	}

	function setStatusText( text, ... )
	{
		if (text != null)
		{
			this.mStatusText.setText(text);
		}

		if (vargc > 0)
		{
			this.mStatusText2.setText(vargv[0]);
		}
		else
		{
			this.mStatusText2.setText("");
		}
	}

	function getPropertyPanel()
	{
		return this.mPropertyPanel;
	}

	function _updateSplatButtons( splats )
	{
		foreach( i, tx in splats )
		{
			local b = this.mSplatButtons[i];
			b.setText(this.File.basename(tx, ".png"));
		}
	}

	function onEnterFrame()
	{
		local node;

		if (("_buildTool" in this.getroottable()) && ::_buildTool != null)
		{
			node = ::_buildTool.mBuildNode;
		}
		else
		{
			node = this._scene.getCamera("Default").getParentSceneNode();
		}

		if (node == null)
		{
			return;
		}

		local pos = node.getWorldPosition();
		local tpos = this.Util.getTerrainPageIndex(pos);

		if (!tpos)
		{
			return;
		}

		local px = tpos.x;
		local pz = tpos.z;

		if (px == this.mTerrainPageX && pz == this.mTerrainPageZ)
		{
			return;
		}

		this.mTerrainPageX = px;
		this.mTerrainPageZ = pz;
		local tss = this.Screens.get("TerrainSplatSelector", true);
		tss.setPage(this.mTerrainPageX, this.mTerrainPageZ);

		if (px != null && pz != null)
		{
			this.mElevationEntry.setValue(::_scene.getTerrainWaterElevation(px, pz));
			this.mMaterialEntry.setText(::_scene.getTerrainWaterMaterial(px, pz));
		}

		this._updateSplatButtons(tss.getCurrentSplats());
	}

}

