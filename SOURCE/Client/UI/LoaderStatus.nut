this.require("UI/UI");
this.require("UI/Screens");
class this.Screens.LoadStatusScreen extends this.GUI.Frame
{
	mTerrainSquares = null;
	mObjectSquares = null;
	mUpdateTimer = 0;
	mUpdateFrequency = 250;
	constructor()
	{
		this.GUI.Frame.constructor("Loading Status");
		this.setSize(390, 220);
		local allcontainer = this.GUI.Container(this.GUI.GridLayout(2, 2));
		allcontainer.getLayoutManager().setRows(25, "*");
		allcontainer.setInsets(5);
		this.add(allcontainer);
		allcontainer.add(this.GUI.HTML("<font size=\"22\" color=\"bbbbff\">Terrain Loading Status</font>"));
		allcontainer.add(this.GUI.HTML("<font size=\"22\" color=\"bbbbff\">Objects Loading Status</font>"));
		local leftcontainer = this.GUI.Panel(this.GUI.GridLayout(9, 9));
		leftcontainer.setSize(176, 176);
		leftcontainer.setInsets(5);
		allcontainer.add(leftcontainer);
		local rightcontainer = this.GUI.Panel(this.GUI.GridLayout(9, 9));
		rightcontainer.setSize(176, 176);
		rightcontainer.setInsets(5);
		allcontainer.add(rightcontainer);
		this.mTerrainSquares = {};
		this.mObjectSquares = {};

		for( local x = 0; x < 9; x++ )
		{
			for( local y = 0; y < 9; y++ )
			{
				local index = x + " " + y;
				this.mTerrainSquares[index] <- this.GUI.ColorSplotch(this.Color(0.30000001, 0.30000001, 0.30000001, 1), false);
				this.mObjectSquares[index] <- this.GUI.ColorSplotch(this.Color(0.30000001, 0.30000001, 0.30000001, 1), false);
				this.mTerrainSquares[index].setAppearance("ColorSquare");
				this.mObjectSquares[index].setAppearance("ColorSquare");
				this.mTerrainSquares[index].setSize(6, 6);
				this.mObjectSquares[index].setSize(6, 6);
				leftcontainer.add(this.mTerrainSquares[index]);
				rightcontainer.add(this.mObjectSquares[index]);
			}
		}

		this.mUpdateTimer = ::Timer();
		this.setOverlay("GUI/EditBorderOverlay");
	}

	function _addNotify()
	{
		this.GUI.Frame._addNotify();
		this._enterFrameRelay.addListener(this);
	}

	function _removeNotify()
	{
		this._enterFrameRelay.removeListener(this);
		this.GUI.Frame._removeNotify();
	}

	function onEnterFrame()
	{
		if (this.mUpdateTimer.getMilliseconds() > this.mUpdateFrequency)
		{
			if (::_avatar == null || ::_avatar.getNode() == null || ::_avatar.mLastServerUpdate == null)
			{
				return;
			}

			local apos = ::_avatar.getNode().getPosition();

			for( local x = 0; x < 9; x++ )
			{
				for( local y = 0; y < 9; y++ )
				{
					local index = y + " " + x;
					local terrainindex = this.Util.getTerrainPageIndex(apos);
					local sceneindex = this._sceneObjectManager.getSceneryPageIndex(apos);
					local extents = this._root.getTerrainExtents();

					if (terrainindex.x - 4 + x > extents.x || terrainindex.z - 4 + y > extents.z || terrainindex.x - 4 + x < 0 || terrainindex.z - 4 + y < 0)
					{
						this.mTerrainSquares[index].setColor(this.Color(0.1, 0.1, 0.1, 1));
					}
					else
					{
						switch(::_sceneObjectManager.getTerrainPageState(terrainindex.x - 4 + x, terrainindex.z - 4 + y))
						{
						case "Loaded":
							this.mTerrainSquares[index].setColor(this.Color(0.2, 0.80000001, 0.2, 1));
							break;

						case "Error":
							this.mTerrainSquares[index].setColor(this.Color(0.80000001, 0.2, 0.2, 1));
							break;

						default:
							this.mTerrainSquares[index].setColor(this.Color(0.30000001, 0.30000001, 0.30000001, 1));
							break;
						}
					}

					local page = ::_sceneObjectManager.getSceneryPage(::_sceneObjectManager.mCurrentZoneDefId, sceneindex.x - 4 + x, sceneindex.z - 4 + y);

					if (page)
					{
						switch(page.getState())
						{
						case this.PageState.PENDINGREQUEST:
							this.mObjectSquares[index].setColor(this.Color(0.40000001, 0.40000001, 0.2, 1));
							break;

						case this.PageState.REQUESTED:
							this.mObjectSquares[index].setColor(this.Color(0.69999999, 0.69999999, 0.2, 1));
							break;

						case this.PageState.LOADING:
							this.mObjectSquares[index].setColor(this.Color(0.2, 0.2, 0.80000001, 1));
							break;

						case this.PageState.READY:
							this.mObjectSquares[index].setColor(this.Color(0.2, 0.80000001, 0.2, 1));
							break;

						case this.PageState.ERRORED:
							this.mObjectSquares[index].setColor(this.Color(0.80000001, 0.2, 0.2, 1));
							break;

						default:
							this.mObjectSquares[index].setColor(this.Color(0.30000001, 0.30000001, 0.30000001, 1));
							break;
						}
					}
				}
			}

			this.mUpdateTimer.reset();
		}
	}

}

