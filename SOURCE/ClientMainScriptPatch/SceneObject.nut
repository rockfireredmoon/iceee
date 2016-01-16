::_avatar <- null;
::_loadNode <- null;

PageState <- {
	FETCHREQUEST = 0,
	FETCHED = 1,
	PENDINGREQUEST = 2,
	REQUESTED = 3,
	REQUESTEDLINKS = 4,
	LOADING = 5,
	READY = 6,
	ERRORED = 7
};

FloorAlignMode <- {
	NONE = 0,
	WHILE_ASCENDING_DESCENDING = 1,
	ALWAYS = 2
};

ForcedQuestMarkerYPosition <- {
	["Prop-Notice_Board"] = 20
};

AlwaysVisible <- {
	["Horde-Anubian_Catapult"] = true,
	["Horde-Invisible_Biped"] = true
};

class SceneryLinkListHandler {
	mPage = null;
	mObjects = null;
	constructor( page, objects ) {
		mPage = page;
		mObjects = objects;
	}

	function onQueryComplete( qa, results )	{
		this.log.debug("Received server links for page " + mPage.getX() + ", " + mPage.getZ());

		// Add the links to the visualizer
		
		foreach( r in results )
			_scene.addLink("Scenery/" + r[0], "Scenery/" + r[1], Color(1.0, 0.0, 1.0, 1.0));

		// Change state to LOADING. This will make the page
		// wait until all scenery has been assembled.
		
		mPage.setState(this.PageState.LOADING);
	}

	function onQueryError( qa, error ) {
		// Just set the page to the loading state
		mPage.setState(PageState.LOADING);
	}

	function onQueryTimeout( qa ) {
		// Ask for the links from the server again
		::_Connection.sendQuery("scenery.link.list", SceneryLinkListHandler(mPage, mObjects), mObjects);
	}

}

class SceneryListHandler 
{
	mZoneID = 0;
	mX = 0;
	mZ = 0;
	constructor( zoneID, x, z )	{
		mZoneID = zoneID;
		mX = x;
		mZ = z;
	}

	function onQueryComplete( qa, results )	{
		log.debug("Received server objects for page " + mX + ", " + mZ);
		local page = ::_sceneObjectManager.getSceneryPage(mZoneID, mX, mZ);

		if (page != null) {
			// Make sure the page is still in the requested state.
			if (page.getState() != PageState.REQUESTED)
				throw Exception("Scenery query logic error!");

			// Add the scenery to the page
			
			foreach( i in results )
				page.addScenery(i[0].tointeger());

			
			// Store the total so the loadscreen can be accurate.
			
			page.mTotalScenery = results.len();
			
						
			// Change state to LOADING. This will make the page
			// wait until all scenery has been assembled.
			
			page.setState(PageState.LOADING);
		}
	}

	function onQueryError( qa, error )	{
		log.debug("" + qa.query + " failed: " + error);
		
		// Set the page to error state
		
		local page = ::_sceneObjectManager.getSceneryPage(mZoneID, mX, mZ);
		if (page != null)
			page.setState(PageState.PENDINGREQUEST);
	}

	function onQueryTimeout( qa ) {
		log.debug("" + qa.query + " timed out. Requesting scenery again...");
		
		// Set the page to pending state so it will be requested again
		
		local page = ::_sceneObjectManager.getSceneryPage(mZoneID, mX, mZ);
		if (page != null)
			page.setState(PageState.PENDINGREQUEST);
	}

}

class SceneryPreloadHandler {
	static nextPackageName = 0;
	mPriority = 0;
	constructor( priority )	{
		mPriority = priority;
	}

	function onQueryComplete( qa, results )	{
		local assets = [];
		::_sceneObjectManager.mRetrievingZoneQuery = false;

		foreach( s in results )
			::_contentLoader.load(s[0], mPriority, "SceneryPreload_" + nextPackageName++, null);
	}

	function onQueryError( qa, error ) {
	}

}

class this.PageEntry 
{
	mState = this.PageState.PENDINGREQUEST;
	mCreatures = [];
	mScenery = [];
	mTotalScenery = 0;
	mZoneID = null;
	mX = 0;
	mZ = 0;
	constructor( zoneID, pageX, pageZ )
	{
		this.mState = this.PageState.PENDINGREQUEST;
		this.mZoneID = zoneID;
		this.mCreatures = [];
		this.mScenery = [];
		this.mX = pageX;
		this.mZ = pageZ;
	}

	function updatePendingScenery()
	{
		local nl = [];

		foreach( e in this.mScenery )
		{
			if (::_sceneObjectManager.hasScenery(e))
			{
				local s = ::_sceneObjectManager.getSceneryByID(e);

				if (!s.isAssembled() && s.getAssemblerError() == null)
				{
					nl.append(e);
				}
			}
			else
			{
				nl.append(e);
			}
		}

		this.mScenery = nl;
		return nl;
	}

	function removeScenery( objectID )
	{
		local l = [];

		foreach( s in this.mScenery )
		{
			if (s != objectID)
			{
				l.append(s);
			}
		}

		this.mScenery = l;
	}

	function setState( value )
	{
		this.mState = value;
	}

	function getState()
	{
		return this.mState;
	}

	function addScenery( id )
	{
		this.mScenery.append(id);
	}

	function getScenery()
	{
		return this.mScenery;
	}

	function setZoneID( id )
	{
		this.mZoneID = id;
	}

	function getZoneID()
	{
		return this.mZoneID;
	}

	function getX()
	{
		return this.mX;
	}

	function getZ()
	{
		return this.mZ;
	}

	function getPendingSceneryCount()
	{
		return this.mScenery.len();
	}

	function _tostring()
	{
		return "SceneryPage[" + this.mX + "," + this.mZ + "](State: " + this.mState + ", Pending: " + this.Util.join(this.mScenery, ", ") + ")";
	}

}

class this.SceneObjectUpdater extends this.MessageBroadcaster
{
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this._nextFrameRelay.addListener(this);
	}

	function onNextFrame()
	{
		this.broadcastMessage("onEnterFrame");
	}

	function onEnterFrame()
	{
		this.broadcastMessage("onEnterFrame");
	}

	function _pump()
	{
		this.broadcastMessage("onEnterFrame");
	}

}

class this.SceneObjectManager extends this.MessageBroadcaster
{
	mLoadedTerrain = {};
	mRandom = null;
	mPages = [];
	mCreatures = {};
	mScenery = {};
	mCurrentPage = null;
	mUnassembledIndex = {};
	mCurrentZoneId = 0;
	mCurrentZoneDefId = 0;
	mCurrentZonePageSize = 2000.0;
	mLoadedTerrainTiles = null;
	mAssemblyQueueQuota = 100;
	mAssemblyQueue = [];
	mCreatureQueue = [];
	mAssemblingCreature = null;
	mDestroyQueue = [];
	mZoneQueriesQueue = [];
	mRetrievingZoneQuery = false;
	mCurrentTerrain = null;
	mCurrentTerrainBase = null;
	mCurrentLocationName = "Unknown";
	mUpdater = null;
	mBuildingCSMs = {};
	NEARBY_LOD = 1;
	mPendingAssemblyCount = 0;
	mTransforming = [];
	static rxSceneObjectNode = this.regexp("^([^/]+)/(\\d+)$");
	constructor()
	{
		::MessageBroadcaster.constructor();
		this.mRandom = this.Random();
		this.mPages = [];
		this.mBuildingCSMs = {};
		this.mLoadedTerrain = {};
		this.mCreatures = {};
		this.mScenery = {};
		this.mZoneQueriesQueue = [];
		this.mRetrievingZoneQuery = false;
		this.mUnassembledIndex = {};
		this._enterFrameRelay.addListener(this);
		this._exitFrameRelay.addListener(this);
		this._csmReadyRelay.addListener(this);
		this._nextFrameRelay.addListener(this);
		this._scene.addListener(this);
		this._eventScheduler.repeatIn(0.5, 0.5, this, "_detectGarbageCreatures");
		this._eventScheduler.repeatIn(15.0, 15.0, this, "_flushUnusedAssemblers");
		this.mUpdater = this.SceneObjectUpdater();
		this.reset();
	}

	function _flushUnusedAssemblers()
	{
		::Assembler.flush(false);
	}

	function addUpdateListener( so )
	{
		this.mUpdater.addListener(so);
	}

	function removeUpdateListener( so )
	{
		this.mUpdater.removeListener(so);
	}

	function getCreatures()
	{
		return this.mCreatures;
	}

	function getScenery()
	{
		return this.mScenery;
	}

	function setCurrentTerrain( terrain )
	{
		if (this.mCurrentTerrain != terrain)
		{
			if (typeof terrain == "string" && terrain.len() > 0)
			{
				local a = this.AssetReference(terrain);
				this.mCurrentTerrain = terrain;
				this.mCurrentTerrainBase = this.File.basename(a.getAsset(), ".cfg");
				::_scene.setWorldGeometry(null);
				::_scene.setClutterEnabled(false);
				::_scene.setPageLoadingPattern(this.mCurrentTerrainBase);
				this._contentLoader.load(this.mCurrentTerrainBase, this.ContentLoader.PRIORITY_REQUIRED, "Terrain", {
					som = this,
					t = terrain,
					base = this.mCurrentTerrainBase,
					function onPackageComplete( name )
					{
						if (this.t == this.som.mCurrentTerrain)
						{
							::_scene.setWorldGeometry(this.base + ".cfg");
							::_scene.setClutterConfig("TerrainClutter.cfg");
							::_scene.setClutterEnabled(true);

							if (!(this.som.mCurrentTerrainBase in ::TerrainEnvDef))
							{
								::TerrainEnvDef[this.som.mCurrentTerrainBase] <- ::TerrainPageDef;
							}

							::TerrainPageDef = {};
						}
					}

					function onPackageError( name, reason )
					{
						this.som.mCurrentTerrain = null;
						this.som.mCurrentTerrainBase = null;
						::TerrainPageDef = {};
					}

				});
			}
			else
			{
				this.mCurrentTerrain = null;
				this.mCurrentTerrainBase = null;
				::_scene.setClutterEnabled(false);
				::_scene.setWorldGeometry(null);
			}
		}
	}

	function getCurrentTerrainBase()
	{
		return this.mCurrentTerrainBase;
	}

	function getSceneryPage( zoneID, x, z )
	{
		if (zoneID != this.mCurrentZoneId)
		{
			return null;
		}

		foreach( p in this.mPages )
		{
			if (p.getZoneID() == zoneID && p.getX() == x && p.getZ() == z)
			{
				return p;
			}
		}

		return null;
	}

	function getTerrainReady( x, z )
	{
		local terrain = this.getCurrentTerrainBase();

		if (terrain == null)
		{
			return true;
		}

		if (::_scene.isTerrainReady(x, z, this.mCurrentZonePageSize))
		{
			return true;
		}

		return false;
	}

	function getPageEntry( x, z )
	{
		foreach( page in this.mPages )
		{
			if (page.mX == x && page.mZ == z)
			{
				return page;
			}
		}

		return null;
	}

	function getPages()
	{
		return this.mPages;
	}

	function getTerrainPageState( x, z )
	{
		local extents = this._root.getTerrainExtents();

		if (x > extents.x || z > extents.z || x < 0 || z < 0)
		{
			return false;
		}

		if (x + "_" + z in this.mLoadedTerrain)
		{
			return this.mLoadedTerrain[x + "_" + z].mode;
		}
		else
		{
			return false;
		}
	}

	function isSceneryPageReady( x, z )
	{
		local pageReady = true;
		pageReady = this._singleCheckPageReady(x, z);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x - 1, z + 1);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x, z + 1);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x + 1, z + 1);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x - 1, z);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x + 1, z);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x - 1, z - 1);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x, z - 1);

		if (!pageReady)
		{
			return false;
		}

		pageReady = this._singleCheckPageReady(x + 1, z - 1);

		if (!pageReady)
		{
			return false;
		}

		return true;
	}

	function _singleCheckPageReady( x, z )
	{
		foreach( p in this.mPages )
		{
			if (p.getX() == x && p.getZ() == z)
			{
				if (p.getState() == this.PageState.READY || p.getState() == this.PageState.ERRORED)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}

		return true;
	}

	function getSceneryPageState( zoneID, x, z )
	{
		if (zoneID == 0)
		{
			return false;
		}

		foreach( p in this.mPages )
		{
			if (p.getZoneID() == zoneID && p.getX() == x && p.getZ() == z)
			{
				return p.getState();
			}
		}

		return false;
	}

	function isPageRequested( zoneID, x, z )
	{
		foreach( p in this.mPages )
		{
			if (p.getZoneID() == zoneID && p.getX() == x && p.getZ() == z)
			{
				return true;
			}
		}

		return false;
	}

	function getUnassembledPageCount()
	{
		local count = 0;

		foreach( p in this.mPages )
		{
			if (p.getState() != this.PageState.READY)
			{
				count++;
			}
		}

		return count;
	}

	function setLocationName( location )
	{
		this.mCurrentLocationName = location;
	}

	function getLocationName()
	{
		return this.mCurrentLocationName;
	}

	function updatePendingPages()
	{
		foreach( p in this.mPages )
		{
			if (p.getState() == this.PageState.LOADING)
			{
				local list = p.updatePendingScenery();

				if (list.len() == 0)
				{
					this._root.updateMiniMapBackground();
					p.setState(this.PageState.READY);
				}
				else
				{
				}
			}
		}
	}

	function getCreatureReady( so )
	{
		if (this.mCurrentZoneDefId == 0)
		{
			return true;
		}

		local cpage = so.getSceneryPageCoords();
		return this.isSceneryPageReady(cpage.x, cpage.z);
	}

	function reset()
	{
		foreach( val in clone this.mCreatures )
		{
			val.destroy();
		}

		this.mCreatures = {};

		foreach( val in clone this.mScenery )
		{
			val.destroy();
		}

		this.mScenery = {};
		this.mCurrentZoneId = 0;
		this.mCurrentZoneDefId = 0;
		this.mCurrentPage = null;
		this.mCreatureQueue = [];
		this.mAssemblyQueue = [];
		this.mAssemblingCreature = null;
		this.mPages = [];
		this.mLoadedTerrain = {};
		this.mUnassembledIndex = {};
		this.mBuildingCSMs = {};
		this.mPendingAssemblyCount = 0;
		this.mCurrentPage = null;
		this.setCurrentTerrain(null);
		::_creatureDefManager.reset();
		::Assembler.flush(true);
	}

	function onCSMReady( name )
	{
		if (name in this.mBuildingCSMs)
		{
			delete this.mBuildingCSMs[name];
		}
	}

	function onCSMDestroyed( name )
	{
		this._cancelCSMBuild(name);
	}

	function _notifyCSMBuilding( name )
	{
		this.mBuildingCSMs[name] <- true;
	}

	function _isCSMBuilding( name )
	{
		return name in this.mBuildingCSMs;
	}

	function _cancelCSMBuild( name )
	{
		if (name in this.mBuildingCSMs)
		{
			delete this.mBuildingCSMs[name];
		}
	}

	function _nextSceneryListQuery()
	{
		if (this.mZoneQueriesQueue.len() > 0 && !this.mRetrievingZoneQuery)
		{
			local query = this.mZoneQueriesQueue[0];
			this.mZoneQueriesQueue.remove(0);
			this.mRetrievingZoneQuery = true;
			::_Connection.sendQuery("scenery.list", query.callback, [
				query.zoneID,
				query.x,
				query.z,
				query.mode
			]);
		}
	}

	function getPageAssets( x, z, ... )
	{
		if (x < 0 || z < 0)
		{
			return;
		}

		local fetchcallback;

		if (vargc > 0)
		{
			fetchcallback = vargv[0];
		}

		local assets = [];

		if (fetchcallback)
		{
			this.mZoneQueriesQueue.append({
				zoneID = this.mCurrentZoneDefId,
				x = x,
				z = z,
				mode = "a",
				callback = fetchcallback
			});
			return;
		}
		else
		{
			foreach( s in this.mScenery )
			{
				local tpos = s.getTerrainPageCoords();

				if (tpos.x == x && tpos.z == z)
				{
					if (this.Util.indexOf(assets, s.getType()) == null)
					{
						assets.push(s.getType());
					}
				}
			}
		}

		return assets;
	}

	function findSceneryObjects( Radius )
	{
		if (::_buildTool == null || ::_buildTool.mBuildNode == null)
		{
			this.log.error("The Scenery Object Browser tool can only be used in Build Mode");
			return null;
		}

		return this.getAssetsAroundRadius(this._buildTool.mBuildNode.getWorldPosition(), Radius);
	}

	function getAssetsAroundRadius( AvatarPosition, Radius )
	{
		local assets = [];

		foreach( object in this.mScenery )
		{
			local Node = object.getNode();

			if (Node == null)
			{
				continue;
			}

			local SceneryNodePosition = Node.getPosition();
			local Distance = AvatarPosition - SceneryNodePosition;

			if (Distance.length() <= Radius.tofloat())
			{
				assets.push(object);
			}
		}

		return assets;
	}

	function prefetchPages( node, dist, priority )
	{
		if (::_avatar == null || ::_avatar.getNode() == null)
		{
			return;
		}

		local apos = ::_avatar.getNode().getPosition();
		local tpos = {
			x = (apos.x / this.mCurrentZonePageSize).tointeger(),
			z = (apos.z / this.mCurrentZonePageSize).tointeger()
		};

		foreach( d in dist )
		{
			if (d > 0)
			{
				for( local x = -d; x <= d; x++ )
				{
					for( local y = -d; y <= d; y++ )
					{
						if (this.abs(x) == d || this.abs(y) == d)
						{
							this.getPageAssets(tpos.x + x, tpos.z + y, this.SceneryPreloadHandler(priority));
						}
					}
				}
			}
			else if (dist == 0)
			{
				this.getPageAssets(tpos.x, tpos.z, this.SceneryPreloadHandler(priority));
			}
		}
	}

	function getPendingRequiredAssemblyCount()
	{
		local offsets = [
			[
				1,
				1
			],
			[
				-1,
				-1
			],
			[
				1,
				0
			],
			[
				0,
				1
			],
			[
				-1,
				0
			],
			[
				0,
				-1
			],
			[
				-1,
				1
			],
			[
				1,
				-1
			],
			[
				0,
				0
			]
		];
		local pending = 0;

		if (::_avatar == null || ::_avatar.getNode() == null || this.mCurrentPage == null)
		{
			return null;
		}

		if (!this._avatar.isAssembled())
		{
			pending += 1;
		}

		local pending = 0;

		foreach( o in offsets )
		{
			local x = this.mCurrentPage.x + o[0];
			local z = this.mCurrentPage.z + o[1];
			local state = this.PageState.FETCHREQUEST;
			local page = this.getSceneryPage(this.mCurrentZoneId, x, z);

			if (page)
			{
				state = page.getState();
			}

			switch(state)
			{
			case this.PageState.ERRORED:
			case this.PageState.FETCHED:
			case this.PageState.LOADING:
				local n = page.getPendingSceneryCount();
				pending += n;
				break;

			case this.PageState.READY:
				break;

			case this.PageState.FETCHREQUEST:
				return null;
				break;

			default:
				pending += 150;
			}
		}

		return pending;
	}

	function prefetchAssets( node )
	{
		if (node == null)
		{
			return;
		}

		local page = this.Util.getTerrainPageName(node.getPosition());

		if (page == null)
		{
			return;
		}

		local dist = this.getSceneryLoadPageRange();
		local required_dists = [
			0,
			1
		];
		local low_dists = [];

		for( local a = 2; a <= dist; a++ )
		{
			low_dists.append(a);
		}

		local fetch_dists = [];
		fetch_dists = [
			dist + 1,
			dist + 2
		];
		this.log.debug("Prefetching assets for " + page + "...");
	}

	static function _lookup( table, id, kind )
	{
		local safeId;

		if (!id)
		{
			return null;
		}

		try {
			safeId = id.tointeger();
		} catch(e) {			
			this.log.warn("lookup of " + kind + "id: " + id + " is not an integer");
			return null;
		}

		if (safeId == 0)
		{
			this.log.warn("lookup of invalid " + kind + "id: " + id);
			return null;
		}

		if (safeId in table)
		{
			return table[safeId];
		}

		return table[safeId] <- SceneObject(safeId, kind);
	}

	function getCreatureByName( name )
	{
		foreach( id, creature in this.mCreatures )
		{
			if (creature.getName() == name)
			{
				return creature;
			}
		}

		return null;
	}

	function getCreatureByID( id )
	{
		return this._lookup(this.mCreatures, id, "Creature");
	}

	function getSceneryByID( id )
	{
		return this._lookup(this.mScenery, id, "Scenery");
	}

	function peekCreatureByID( id )
	{
		if (id in this.mCreatures)
		{
			return this.mCreatures[id];
		}

		return null;
	}

	function peekSceneryByID( id )
	{
		if (id in this.mScenery)
		{
			return this.mScenery[id];
		}

		return null;
	}

	function hasObject( type, id )
	{
		if (type == "Creature")
		{
			return id in this.mCreatures ? this.mCreatures[id] : null;
		}

		if (type == "Scenery")
		{
			return id in this.mScenery ? this.mScenery[id] : null;
		}

		if (type == "Assembling")
		{
			return id in this.mCreatures ? this.mCreatures[id] : null;
		}

		throw this.Exception("Invalid scene object type: " + type);
	}

	function hasCreature( id )
	{
		return this.hasObject("Creature", id);
	}

	function hasScenery( id )
	{
		return this.hasObject("Scenery", id);
	}

	function _remove( so )
	{
		local id = so.getID();

		if (so in this.mUnassembledIndex)
		{
			delete this.mUnassembledIndex[so];
		}

		local index;
		index = this.Util.indexOf(this.mAssemblyQueue, so);

		if (index != null)
		{
			this.mAssemblyQueue.remove(index);
		}

		index = this.Util.indexOf(this.mCreatureQueue, so);

		if (index != null)
		{
			this.mCreatureQueue.remove(index);
		}

		if (so == this.mAssemblingCreature)
		{
			this.mAssemblingCreature = null;
		}

		index = this.Util.indexOf(this.mDestroyQueue, so);

		if (index != null)
		{
			this.mDestroyQueue.remove(index);
		}

		if (so.mObjectClass == "Creature" && id in this.mCreatures)
		{
			delete this.mCreatures[id];

			foreach( i, s in this.mCreatures )
			{
				s.onOtherDestroy(so);
			}
		}
		else if (so.mObjectClass == "Scenery" && id in this.mScenery)
		{
			delete this.mScenery[id];
		}

		if (so.isScenery())
		{
			foreach( p in this.mPages )
			{
				p.removeScenery(id);
			}
		}
	}

	function pickSceneObject( x, y, ... )
	{
		local flags = this.QueryFlags.ANY;

		if (vargc > 0)
		{
			flags = vargv[0];
		}

		local canSelectLocked = false;

		if (vargc > 1)
		{
			canSelectLocked = vargv[1];
		}

		local ray = this.Screen.getViewportRay(x, y);
		local hits = this._scene.rayQuery(ray.origin, ray.dir, flags, true, false);
		local so;
		local h;

		foreach( h in hits )
		{
			so = this.findSceneObject(h.object);

			if (so && (!so.isLocked() || canSelectLocked))
			{
				return so;
			}
		}

		return null;
	}

	function pickCreature( x, y, ... )
	{
		local excludeAvatar = false;

		if (vargc > 0)
		{
			excludeAvatar = vargv[0];
		}

		local rate = 16;
		local subrate = rate / 2;
		local subrate2 = rate / 4;
		local offsets = [
			[
				0,
				0
			],
			[
				-subrate2,
				-subrate2
			],
			[
				subrate2
				subrate2
			],
			[
				-subrate2,
				subrate2
			],
			[
				subrate2,
				-subrate2
			],
			[
				-subrate2,
				0
			],
			[
				subrate2,
				0
			],
			[
				0,
				subrate2
			],
			[
				0,
				-subrate2
			],
			[
				-subrate,
				-subrate
			],
			[
				subrate
				subrate
			],
			[
				-subrate,
				subrate
			],
			[
				subrate,
				-subrate
			],
			[
				-subrate,
				0
			],
			[
				subrate,
				0
			],
			[
				0,
				subrate
			],
			[
				0,
				-subrate
			]
		];

		foreach( o in offsets )
		{
			local ray = this.Screen.getViewportRay(x + o[0], y + o[1]);
			local hits = this._scene.rayQuery(ray.origin, ray.dir, this.QueryFlags.ANY, false, false, 0, null, false);

			foreach( h in hits )
			{
				local so = this.findSceneObject(h.object);

				if (so && so.isCreature())
				{
					if (!excludeAvatar || so != ::_avatar)
					{
						return so;
					}
				}
			}
		}

		return null;
	}

	
	/**
		Attempt to find the scene object associated with this movable
		object. This will traverse up the object's heirarchy as needed
		in order to find an encompassing scene object. It stops on the
		first one found (if it's part of a deep hierarchy).
		
		@return The SceneObject that contains this MovableObject or
			<tt>null</tt> if none could be found.
	*/
	function findSceneObject( movableObject )
	{
		if (!movableObject)
			return null;

	 	// Note we use getParentSceneNode since we may be
	 	// starting with a bone attachment and we want to
	 	// break out of that heirarchy and into the entity.
		local so = null;
		
		local node = movableObject.getParentSceneNode();

		while (node != null && so == null) {
			local name = node.getName();
			local loc = name.find("/");

			if (loc) {
	 			// Found one that matches, let's just make sure
				local type = name.slice(0, loc);
				local id = name.slice(loc + 1, name.len());

				try	{
					so = hasObject(type, id.tointeger());
					if (so)	
						return so;
				}
				catch( err ) { 
					// Invalid object type, try again.
				}
			}

			node = node.getParent();
		}

		return so;
	}

	function getAssemblyQueueSize()	{
		return mAssemblyQueue.len();
	}

	function _calcDebugStatus( table )
	{
		local aCount = 0;
		local uCount = 0;
		local aItems = {};
		local uItems = {};
		local lods = [
			0,
			0,
			0,
			0,
			0,
			0
		];

		foreach( so in table )
		{
			local ass = so.mAssembler ? so.mAssembler.getAssemblerDesc() : "null";
			local items;
			local lod = so.getLOD();

			if (lod == -1)
			{
				lod = 5;
			}

			lods[lod] += 1;

			if (so.isAssembled())
			{
				aCount++;
				items = aItems;
			}
			else
			{
				if (lod >= 4)
				{
					continue;
				}

				uCount++;
				items = uItems;
			}

			if (ass in items)
			{
				items[ass] += 1;
			}
			else
			{
				items[ass] <- 1;
			}
		}

		return {
			aitems = aItems,
			acount = aCount,
			uitems = uItems,
			ucount = uCount,
			lods = lods
		};
	}

	function getSceneryAssemblyStatus()
	{
		return this._calcDebugStatus(this.mScenery);
	}

	function getDebugStatusText( ... )
	{
		local str;
		local s_status = this._calcDebugStatus(this.mScenery);
		local c_status = this._calcDebugStatus(this.mCreatures);
		str = "  --- Stats ---\n";
		str += "    Scenery " + s_status.acount + " of " + this.mScenery.len() + " LOD=[" + this.Util.join(s_status.lods, ", ") + "]\n" + "    Creatures " + c_status.acount + " of " + this.mCreatures.len() + " LOD=[" + this.Util.join(c_status.lods, ", ") + "]\n";

		if (vargc == 0 && !this.varcv[0])
		{
			return str;
		}

		if (s_status.uitems.len() > 0)
		{
			str += "  --- Unassembled Scenery ---\n";
			local count = 0;

			foreach( k, v in s_status.uitems )
			{
				count++;

				if (count == 5)
				{
					if (vargv[0] == "brief")
					{
						str += "    ...";
						break;
					}

					str += "\n";
					count = 0;
				}

				if (vargv[0] == "brief")
				{
					str += "    " + k + " x " + v + "\n";
				}
				else
				{
					str += "    " + k + " x " + v;
				}
			}

			str += "\n";
		}

		if (c_status.uitems.len() > 0)
		{
			str += "  --- Unassembled Creatures ---\n";
			local count = 0;

			foreach( k, v in c_status.uitems )
			{
				count++;

				if (count == 5)
				{
					if (vargv[0] == "brief")
					{
						str += "...";
						break;
					}

					str += "\n";
					count = 0;
				}

				if (vargv[0] == "brief")
				{
					str += "    " + k + " x " + v + "\n";
				}
				else
				{
					str += "    " + k + " x " + v;
				}
			}

			str += "\n";
		}

		return str;
	}

	mUnassembledScenery = [];
	function getPendingAssemblyCount()
	{
		return this.mPendingAssemblyCount;
	}

	function getTotalAssemblyCount()
	{
		local total = 0;

		foreach( p in this.mPages )
		{
			total += p.mTotalScenery;
		}

		return total;
	}

	function auditPendingAssemblyCount()
	{
		local count = 0;
		this.mUnassembledScenery = [];

		foreach( s in this.mScenery )
		{
			if (!s.mAssembled && s.mLODLevel <= 1)
			{
				local assembler = s.mAssembler;

				if (assembler && assembler.getArchiveError() == null)
				{
					this.mUnassembledScenery.append(s);
					count++;
				}
			}
		}

		if (this.mPendingAssemblyCount != count)
		{
			this.log.warn("SceneObjectManager.PendingAssemblyCount was incorrect! (" + this.mPendingAssemblyCount + " should be " + count + ")");
		}

		this.mPendingAssemblyCount = count;
		return this.mPendingAssemblyCount;
	}

	function setAvatar( pID )
	{
		if (this._avatar)
		{
			this._avatar.setController("Creature");
		}

		::_tools.setActiveTool(::_playTool);
		::gToolMode = "play";
		::_avatar <- this.getCreatureByID(pID);
		::_scene.setSoundListener(::_avatar.getNode());
		::_avatar.onSetAvatar();
		this.broadcastMessage("onAvatarSet");
		::_avatar.setController("Avatar2");

		if (this.gAvatar.headLight)
		{
			::_avatar.setHeadLight(true);
		}
	}

	function queueAssembly( so, ... )
	{
		if (so.mAssemblyQueueTime != null)
		{
			return;
		}

		if (so == ::_avatar)
		{
			return;
		}

		if (so.getAssemblerError() != null)
		{
			return;
		}

		if (so.isCreature())
		{
			this.mCreatureQueue.append(so);
		}
		else
		{
			this.mAssemblyQueue.append(so);
		}

		so.mAssemblyQueueTime = this.System.currentTimeMillis();

		if (vargc > 0)
		{
			so.mAssemblyQueueTime += vargv[0];
		}
	}

	function _queueDestroy( so, destroy )
	{
		if (destroy)
		{
			this.mDestroyQueue.append(so);
		}
		else
		{
			local index = this.Util.indexOf(this.mDestroyQueue, so);

			if (index != null)
			{
				this.mDestroyQueue.remove(index);
			}
		}
	}

	function _getNextCreatureToAssemble()
	{
		local a = ::_avatar;

		if (this.mAssemblingCreature != null)
		{
			local assemblingAssembler = this.mAssemblingCreature.getAssembler();

			if (assemblingAssembler)
			{
				if (!this.mAssemblingCreature.isAssembled() && assemblingAssembler.getReady() && this.mAssemblingCreature.getAssemblerError() == null)
				{
					return this.mAssemblingCreature;
				}
			}
		}

		local t = this.System.currentTimeMillis();
		local apos = a ? a.getPosition() : null;
		local readyList = [];

		foreach( c in this.mCreatureQueue )
		{
			local assembler = c.getAssembler();
			local hasNode = c.getNode() != null;
			local timeGood = c.getAssemblyQueueTime() <= t;
			local assemblerReady = assembler != null && assembler.getReady() != false;
			local aposNull = apos == null;
			local alwaysVisible = c.isAlwaysVisible();
			local cpos = c.getPosition();
			local distGood = false;

			if (apos && hasNode && cpos)
			{
				local avatarToCreatureDistance = apos - cpos;
				avatarToCreatureDistance = avatarToCreatureDistance.length();
				distGood = avatarToCreatureDistance <= this.gCreatureVisibleRange;
			}

			if (assembler != null && hasNode && timeGood && assemblerReady && (aposNull || alwaysVisible || distGood))
			{
				readyList.append(c);
			}
		}

		if (a)
		{
			if (!a.isAssembled() && a.mAssemblyQueueTime <= t)
			{
				this.mAssemblingCreature = a;
				this.mAssemblingCreature.mAssemblyQueueTime = null;
				local idx = this.Util.indexOf(this.mCreatureQueue, this.mAssemblingCreature);

				if (idx >= 0)
				{
					this.mCreatureQueue.remove();
				}

				return a;
			}

			local avatarPos = ::_avatar.getPosition();
			local f = function ( c1, c2 ) : ( avatarPos )
			{
				if (c1 == c2)
				{
					return 0;
				}

				local pos1 = c1.getPosition();
				local pos2 = c2.getPosition();

				if (pos1 == null || pos2 == null || avatarPos == null)
				{
					return 0;
				}

				local dist1 = pos1.squaredDistance(avatarPos);
				local dist2 = pos2.squaredDistance(avatarPos);

				if (dist1 < dist2)
				{
					return -1;
				}

				if (dist1 > dist2)
				{
					return 1;
				}

				return 0;
			};
			this.Util.bubbleSort(readyList, f);
		}

		if (readyList.len() == 0)
		{
			this.mAssemblingCreature = null;
			return null;
		}

		this.mAssemblingCreature = readyList[0];
		this.mAssemblingCreature.mAssemblyQueueTime = null;
		this.mCreatureQueue.remove(this.Util.indexOf(this.mCreatureQueue, this.mAssemblingCreature));
		return this.mAssemblingCreature;
	}

	function assembleCreature( ignoreCreature )
	{
		if (this.mAssemblyQueueQuota > 0)
		{
			local t0 = this.System.currentTimeMillis();
			local creature = this._getNextCreatureToAssemble();

			if (!creature || creature == ignoreCreature)
			{
				return null;
			}

			local assembler = creature.getAssembler();

			if (assembler == null)
			{
				this.mAssemblingCreature = null;
			}
			else if (assembler.assemble(creature) == true || creature.getAssemblerError() != null)
			{
				if (creature.getAssemblerError() != null)
				{
					this.log.warn("******************************************************");
					this.log.warn("***>> assembler error " + creature.getAssemblerError());
					this.log.warn("******************************************************");
				}

				this.mAssemblingCreature = null;
			}

			local assembleTime = this.System.currentTimeMillis() - t0;

			if (assembleTime >= 10)
			{
				this.log.warn("*** A creature took a long time to assemble: " + creature.getName() + " (" + assembleTime + " ms)");
			}

			this.mAssemblyQueueQuota -= assembleTime > 0 ? assembleTime : 1;
			return creature;
		}

		return null;
	}

	function assembleCreatures()
	{
		local lastCreatureAssembled = null;

		// Assemble creatures until we run out of creatures or run out of quota:
		while( mAssemblyQueueQuota > 0 )
		{
			local newLastCreatureAssembled = assembleCreature(lastCreatureAssembled);
						
			if(!newLastCreatureAssembled || lastCreatureAssembled==newLastCreatureAssembled)
				return;
			
			lastCreatureAssembled = newLastCreatureAssembled;
		}	
	}

	function updateAssemblyQueue()
	{
		local state = ::_stateManager.peekCurrentState();
		local currentState = "mClassName" in state ? state.mClassName : "";

		if (this.LoadScreen.isVisible())
		{
			this.mAssemblyQueueQuota = 75;
		}
		else if (currentState == "CharacterSelectionState")
		{
			this.mAssemblyQueueQuota = 10;
		}
		else if (this.gToolMode == "build")
		{
			this.mAssemblyQueueQuota = 10;
		}
		else
		{
			this.mAssemblyQueueQuota = this.Math.clamp(this.mAssemblyQueueQuota + 1, -25, 1);
		}

		if (this.mAssemblyQueueQuota < 5)
		{
			this.mAssemblyQueueQuota = 5;
		}

		local retries = [];
		local startQuota = this.mAssemblyQueueQuota;
		local startQueueLen = this.mAssemblyQueue.len();
		this.assembleCreatures();

		while (this.mAssemblyQueueQuota > 0 && this.mAssemblyQueue.len() > 0)
		{
			local t0 = this.System.currentTimeMillis();
			local so = this.mAssemblyQueue[0];
			this.mAssemblyQueue.remove(0);

			if (so != ::_avatar)
			{
				if (!so.getNode())
				{
					so.mAssemblyQueueTime = null;
				}
				else if (so.mAssemblyQueueTime > t0)
				{
					retries.append(so);
				}
				else
				{
					so.mAssemblyQueueTime = null;
					local res = so.reassemble(true);

					if (typeof res == "integer")
					{
						so.mAssemblyQueueTime = t0 + res;
						retries.append(so);
					}
					else if (!res && so.getAssemblerError() == null)
					{
						local delay = 250 + this.mRandom.nextInt(150);
						so.mAssemblyQueueTime = t0 + delay;
						retries.append(so);
					}

					local t = this.System.currentTimeMillis();
					local assembleTime = t - t0;
					this.mAssemblyQueueQuota -= assembleTime > 0 ? assembleTime : 1;

					if (assembleTime >= 10)
					{
						this.log.warn("An object took a long time to assemble: " + so.getName() + " (" + assembleTime + " ms) - " + so);
					}
				}
			}
		}

		foreach( so in retries )
		{
			if (typeof so.mAssemblyQueueTime != "integer")
			{
				throw this.Exception("Whoops, internal scheduling error");
			}

			this.mAssemblyQueue.append(so);
		}
	}

	function onEnterFrame()
	{
		this.pushNextSceneryRequest();
		this.updateAssemblyQueue();
		this.updatePendingPages();
		this.loadNearbyObjects();
		this._nextSceneryListQuery();
		
		foreach(transform in mTransforming) {
			transform.updateTransformationSequences();
		}
	}

	function unloadSceneryInZone( zoneX, zoneZ )
	{
		local removeList = [];

		foreach( k, v in this.mScenery )
		{
			local coords = v.getSceneryPageCoords();

			if (coords.x == zoneX && coords.z == zoneZ)
			{
				::_sceneObjectManager._queueDestroy(v, true);
			}
		}
	}

	function getSceneryLoadPageRange()
	{
		local pagedistance = (this._CameraObject.getFarClipDistance() / this.mCurrentZonePageSize).tointeger();
		pagedistance = pagedistance > 1 ? pagedistance : 1;
		pagedistance = pagedistance < 4 ? pagedistance : 4;
		return pagedistance;
	}

	function getSceneryUnloadPageRange()
	{
		return this.getSceneryLoadPageRange() + 1;
	}

	function getSceneryPageIndex( pos )
	{
		if (pos.x < 0 || pos.z < 0)
		{
			local breakhere;
		}

		return {
			x = this.floor(pos.x / this.mCurrentZonePageSize).tointeger(),
			z = this.floor(pos.z / this.mCurrentZonePageSize).tointeger()
		};
	}

	function unloadFarPages()
	{
		local maxdist = this.getSceneryUnloadPageRange();
		local pages = [];

		foreach( p in this.mPages )
		{
			local zoneX = p.getX();
			local zoneZ = p.getZ();
			local xdist = this.abs(zoneX - this.mCurrentPage.x);
			local zdist = this.abs(zoneZ - this.mCurrentPage.z);

			if (xdist > maxdist || zdist > maxdist)
			{
				this.log.debug("Unloading page " + zoneX + ", " + zoneZ + "...");
				this.unloadSceneryInZone(zoneX, zoneZ);
			}
			else
			{
				pages.append(p);
			}
		}

		this.mPages = pages;
	}

	function getSceneryInRange( scenery )
	{
		if (this.mCurrentPage == null)
		{
			return false;
		}

		local coords = scenery.getSceneryPageCoords();

		if (!coords || !this.mCurrentPage)
		{
			return false;
		}

		local maxdist = this.getSceneryUnloadPageRange();
		local xdist = this.abs(coords.x - this.mCurrentPage.x);
		local zdist = this.abs(coords.z - this.mCurrentPage.z);

		if (xdist > maxdist || zdist > maxdist)
		{
			return false;
		}

		return true;
	}

	function loadNearbyObjects()
	{
		local node;

		if (::_loadNode == null)
		{
			if (::_avatar == null)
			{
				return;
			}

			node = ::_avatar.getNode();
		}
		else
		{
			node = ::_loadNode;
		}

		local coord = this.getSceneryPageIndex(node.getPosition());
		local pagedistance = this.getSceneryLoadPageRange();
		local a = ::_avatar;

		if (::_avatar != null && ::_avatar.mLastServerUpdate != null && (this.mCurrentPage == null || coord.x != this.mCurrentPage.x || coord.z != this.mCurrentPage.z))
		{
			this.mCurrentPage = coord;
			this.unloadFarPages();

			for( local x = coord.x - pagedistance; x <= coord.x + pagedistance; x++ )
			{
				for( local z = coord.z - pagedistance; z <= coord.z + pagedistance; z++ )
				{
					this.queryScenery(x, z);
				}
			}

			local buildScreen = this.Screens.get("BuildScreen", false);

			if (buildScreen)
			{
				if (::_buildTool)
				{
					local buildNode = ::_buildTool.getBuildNode();

					if (buildNode)
					{
						buildScreen.updateEnvironmentFromSceneNode(buildNode);
					}
				}
			}

			this.prefetchAssets(node);
		}
	}

	function _detectGarbageCreatures()
	{
		if (::_avatar == null)
		{
			return;
		}

		local apos = ::_avatar.getPosition();

		foreach( c in this.mCreatures )
		{
			if (c == ::_avatar)
			{
				continue;
			}

			if (::partyManager.isCreaturePartyMember(c))
			{
				continue;
			}

			if (c != ::_avatar && c.getZoneID() != this.mCurrentZoneId)
			{
				c.destroy();
				continue;
			}

			local serverDistance = this.Math.manhattanDistanceXZ(apos, c.getPosition());

			if (serverDistance > this.gServerListenDistance)
			{
				c.destroy();
			}
		}
	}

	function onNextFrame()
	{
		local t0 = this.System.currentTimeMillis();

		for( local timeQuota = 5; this.mDestroyQueue.len() > 0;  )
		{
			local t = this.System.currentTimeMillis();

			if (t - t0 > timeQuota)
			{
				break;
			}

			local so = this.mDestroyQueue[0];
			this.mDestroyQueue.remove(0);
			so.destroy();
		}
	}

	function onExitFrame()
	{
	}

	function onZoneUpdate( newZoneId, zoneDefId, zonePageSize, mask )
	{
		if (mask & this.ZoneUpdateFlags.ENTERING_ZONE)
		{
			::_loadScreenManager.setLoadScreenVisible(true, true);

			if (::_avatar)
			{
				::_avatar.setTargetObject(null);
				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.NONE, false);
				local anim_handler = ::_avatar.getAnimationHandler();

				if (anim_handler)
				{
					anim_handler.reset();
				}
			}

			if (zonePageSize == 0)
			{
				this.gPreSimWaiting = true;
				return false;
			}
		}

		this.gPreSimWaiting = false;

		if (this.mCurrentZoneId == newZoneId)
		{
			return false;
		}

		this.mCurrentZoneId = newZoneId;
		this.mCurrentZoneDefId = zoneDefId;
		this.mCurrentZonePageSize = zonePageSize;

		if (mask & this.ZoneUpdateFlags.PARTITION_TRANSFER)
		{
			foreach( p in this.mPages )
			{
				p.setZoneID(this.mCurrentZoneId);
			}

			return;
		}
		else
		{
			this.mCurrentPage = null;
			this.mPages = [];
		}

		this.log.debug("Zone set to " + newZoneId + ", destroying everything in " + this.mCurrentZoneId);
		this.mCreatureQueue = [];
		this.mAssemblingCreature = null;

		if ("setTerrainBufferDistance" in this.Scene)
		{
			this._scene.setTerrainBufferDistance(zonePageSize + 350);
		}

		local list = [];
		local id;
		local so;

		foreach( id, so in this.mScenery )
		{
			list.append(so);
		}

		foreach( id, so in this.mCreatures )
		{
			if (so != this._avatar)
			{
				list.append(so);
			}
		}

		::_avatar.reassemble();

		foreach( so in list )
		{
			so.destroy();
		}

		this.Assert.isEqual(this.mScenery.len(), 0);
		this.Assert.isEqual(this.mCreatures.len(), ::_avatar == null ? 0 : 1);
		::_tutorialManager.onZoneUpdate(zoneDefId);
		this.broadcastMessage("onZoneUpdate", newZoneId, zoneDefId);
		return true;
	}

	function _updateLodBucket( so, oldLod, newLod )
	{
		if (so.mAssembled || so.getAssemblerError() != null || so.getObjectClass() != "Scenery")
		{
			return;
		}

		if (oldLod <= this.NEARBY_LOD && newLod > this.NEARBY_LOD)
		{
			this.mPendingAssemblyCount--;
		}
		else if (oldLod > this.NEARBY_LOD && newLod <= this.NEARBY_LOD)
		{
			this.mPendingAssemblyCount++;
		}
	}

	function _updateAssemblyStatus( so, status )
	{
		this.log.debug("Assembly of " + so + " <- " + status);

		if (so.getObjectClass() == "Scenery")
		{
			if (status)
			{
				if (so.getLOD() <= this.NEARBY_LOD)
				{
					this.mPendingAssemblyCount--;
				}
			}
			else if (so.getLOD() <= this.NEARBY_LOD)
			{
				this.mPendingAssemblyCount++;
			}
		}
	}

	function _terrainPageName( pageX, pageZ )
	{
		local terrain = this.getCurrentTerrainBase();
		return terrain + "_x" + pageX + "y" + pageZ;
	}

	function pushNextSceneryRequest()
	{
		local page;

		foreach( p in this.mPages )
		{
			if (p.getState() == this.PageState.REQUESTED)
			{
				return;
			}

			if (p.getState() == this.PageState.PENDINGREQUEST)
			{
				page = p;
			}
		}

		if (page == null)
		{
			return;
		}

		local pageX = page.getX();
		local pageZ = page.getZ();
		this.log.debug("Requesting objects for page " + pageX + ", " + pageZ + " from server...");
		::_Connection.sendQuery("scenery.list", this.SceneryListHandler(this.mCurrentZoneId, pageX, pageZ), [
			this.mCurrentZoneDefId,
			pageX,
			pageZ
		]);
		page.setState(this.PageState.REQUESTED);
	}

	function queryScenery( pageX, pageZ )
	{
		if (this.isPageRequested(this.mCurrentZoneId, pageX, pageZ))
		{
			return;
		}

		this.mPages.append(this.PageEntry(this.mCurrentZoneId, pageX, pageZ));
	}

	function onTerrainPageLoaded( pageX, pageZ, bounds )
	{
		this.mLoadedTerrain[pageX + "_" + pageZ] <- {
			x = pageX,
			z = pageZ,
			mode = "Loaded"
		};
		this._root.updateMiniMapBackground();
	}

	function onTerrainPageError( pageX, pageZ )
	{
		local name = pageX + "_" + pageZ;

		if (name in this.mLoadedTerrain)
		{
			return;
		}

		this.mLoadedTerrain[name] <- {
			x = pageX,
			z = pageZ,
			mode = "Error"
		};
	}

	function onTerrainPageUnloaded( pageX, pageZ, bounds )
	{
		local name = pageX + "_" + pageZ;

		if (name in this.mLoadedTerrain)
		{
			delete this.mLoadedTerrain[name];
		}
	}

	function onTerrainTileLoaded( pageX, pageZ, tileX, tileZ, bounds )
	{
	}

	function onTerrainTileUnloaded( pageX, pageZ, tileX, tileZ, bounds )
	{
	}

	function getCurrentZoneID()
	{
		return this.mCurrentZoneId;
	}

	function getCurrentZoneDefID()
	{
		return this.mCurrentZoneDefId;
	}

}

class this.SceneObject extends this.MessageBroadcaster
{
	mID = -1;
	mAlwaysVisible = null;
	mObjectClass = null;
	mZoneID = null;
	mSceneryLayer = "";
	mCreatureDef = null;
	mType = "";
	mVars = null;
	mDef = null;
	mOpacity = -1.0;
	mFlags = 0;
	mTimeSinceLastUpdate = 0;
	mNode = null;
	mFadeEnabled = true;
	mAssemblingNode = null;
	mDeathAppearanceChangeEvent = null;
	mAnimationState = null;
	mAssembler = null;
	mPickedAssembler = null;
	mVelocityUpdatePending = false;
	mVelocityUpdateSchedule = null;
	mAssemblyData = null;
	mAssembling = false;
	mAssembled = false;
	mAssemblyQueueTime = null;
	mAnimationHandler = null;
	mController = null;
	mEffectsHandler = null;
	mMorphEffectsHandler = null;
	mSlopeSlideInertia = null;
	mDead = false;
	mNeedToRunDeathAnim = false;
	mHasLoot = false;
	mInCombat = false;
	mAttemptingCombat = false;
	mLastServerUpdate = null;
	mCurrentlyJumping = false;
	mSpeed = 0.0;
	mHeading = 0.0;
	mRotation = 0.0;
	mVerticalSpeed = 0.0;
	mObjectBelowAvatar = false;
	mDownwardAcceleration = 250.0;
	mDistanceToFloor = 0.0;
	mShadowVisible = false;
	mDestroying = false;
	mResetTabTargetting = false;
	mShadowDecal = null;
	mShadowProjector = null;
	mNameInRange = false;
	mNamePlatePosition = null;
	mNamePlateScale = null;
	UPDATE_TIMEOUT = 30000;
	mTimeoutEnabled = true;
	mNormalSize = 1.0;
	mTimer = null;
	mCastingEndTime = 0.0;
	mCastingWarmupTime = 0.0;
	mUsingAbilityID = 0;
	mSelectionProjector = null;
	mSelectionNode = null;
	mStats = null;
	mIsSunVisible = true;
	mShowName = null;
	mNameBoard = null;
	mShowHeadLight = false;
	mHeadLight = null;
	mStatusEffects = null;
	mStatusModifiers = null;
	mUniqueBuffs = null;
	mUniqueStatusEffects = null;
	mUniqueStatusModifiers = null;
	mCarryingFlag = false;
	mFlag = null;
	mStartNormal = null;
	mEndNormal = null;
	mInterpolateFramesLeft = -1;
	FRAMES_TO_INTERPOLATE = 15;
	FORCE_UPDATE = 100;
	mInteractParticle = null;
	mInteractParticleNode = null;
	UP_DOWN_SLOPE_ANGLE = 0.34999999;
	mFloorAlignMode = this.FloorAlignMode.NONE;
	mCurrentlyOriented = false;
	mLastNormal = this.Vector3();
	mTargetObject = null;
	mAttachments = null;
	mAttachmentOverride = null;
	mWeapons = null;
	mPreviousWeaponSet = this.VisibleWeaponSet.INVALID;
	mVisibleWeaponSet = this.VisibleWeaponSet.INVALID;
	mMeleeAutoAttackActive = false;
	mRangedAutoAttackActive = false;
	mLODLevel = -1;
	mPercentToNextLOD = 1.0;
	mProperties = {};
	mGarbagificationTime = null;
	mGeometryPage = null;
	mSoundEmitters = null;
	LOCKED = 1;
	PRIMARY = 2;
	mTypeString = "";
	mCurrentFadeLevel = 0.0;
	mDesiredFadeLevel = 1.0;
	mFadeTarget = 1.0;
	mCorking = false;
	mCorkedStatusEffects = null;
	mCorkedStatusModifiers = null;
	mCorkedFloaties = null;
	mCorkedChatMessage = null;
	mCorkTimeout = 0;
	mGone = false;
	mAbilityEffect = null;
	mSwimming = false;
	mWaterElevation = 0.0;
	mQuestIndicator = null;
	mWidth = null;
	mHeight = null;
	mLastYPos = 0;
	mClickBox = null;
	mBaseStats = null;
	mFloorAlignOrientation = null;
	mForceShowEquipment = false;
	mIsScenery = false;
	mPreviousAsset = null;
	mPreviousScale = null;
	mParticleAttachments = {};
	mCurrentTransformationSequence = null;
	mCurrentTransformationFrame = null;
	
	constructor( pID, objectClass )
	{
		::MessageBroadcaster.constructor();
		this.mIsScenery = objectClass == "Scenery";
		this.mObjectClass = objectClass;
		this.mProperties = {};
		this.mStatusModifiers = [];
		this.mStatusEffects = {};
		this.mUniqueBuffs = [];
		this.mUniqueStatusEffects = [];
		this.mUniqueStatusModifiers = [];
		this.mAttachments = {};
		this.mAttachmentOverride = {};
		this.mWeapons = {};
		this.mCorkedFloaties = [];
		this.mCorkedChatMessage = [];

		if (pID == null && objectClass == "Dummy")
		{
			this.mID = ::_DummyCount;
			::_DummyCount++;
		}
		else
		{
			this.mID = pID;
		}

		this.mDef = {};
		this.mNode = ::_scene.createSceneNode(this.mObjectClass + "/" + this.mID);
		::_scene.getRootSceneNode().addChild(this.mNode);

		if (this.mObjectClass == "Creature")
		{
			if (typeof this.mID == "integer")
			{
				this._Connection.sendInspectCreature(this.mID);
			}

			this.setShowingShadow(true);
			this.prepareAssemblingNode();
			this.mTimer = ::Timer();
			this.mBaseStats = {};
			this.mBaseStats[this.Stat.STRENGTH] <- 0;
			this.mBaseStats[this.Stat.DEXTERITY] <- 0;
			this.mBaseStats[this.Stat.CONSTITUTION] <- 0;
			this.mBaseStats[this.Stat.PSYCHE] <- 0;
			this.mBaseStats[this.Stat.SPIRIT] <- 0;
		}

		this.mMorphEffectsHandler = this.EffectsHandler(this);
		this.mEffectsHandler = this.EffectsHandler(this);

		if (objectClass == "Dummy")
		{
			this._setAssembled(true);
		}

		::_scene.updateLinks();
	}

	function setZoneID( zoneID )
	{
		this.mZoneID = zoneID;
	}

	function resetTimeSinceLastUpdate()
	{
		this.mTimeSinceLastUpdate = 0;
	}

	function getZoneID()
	{
		return this.mZoneID;
	}

	function getZoneDefId()
	{
		if (this.mZoneID != null && this.mZoneID != "")
		{
			local splitZoneString = this.Util.split(this.mZoneID, "-");

			if (splitZoneString.len() == 3)
			{
				return splitZoneString[1].tointeger();
			}
		}

		return -1;
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "creature.isusable")
		{
			local result = results[0][0];

			if (result == "Q")
			{
				this.addInteractParticle();
			}
			else if (result == "D")
			{
				this.removeInteractParticle();
			}
			else
			{
				this.removeInteractParticle();
			}

			::_useableCreatureManager.addCreatureDef(qa.args[0], result);

			if (::_playTool == null)
			{
				return;
			}

			local cursorpos = this.Screen.getCursorPos();
			local so = this._sceneObjectManager.pickSceneObject(cursorpos.x, cursorpos.y, this.QueryFlags.ANY);

			if (so != null && so.getCreatureDef() != null && so.getCreatureDef().getID() == qa.args[0].tointeger())
			{
				::_playTool.updateMouseCursor(so);
			}
			else
			{
				::_playTool.updateMouseCursor(null);
			}
		}
	}

	function setFadeEnabled( which )
	{
		this.mFadeEnabled = which;
	}

	function onQueryError( qa, error )
	{
		if (qa.query == "scenery.edit")
		{
			::_ChatWindow.addMessage("err/", error, "General");
		}
		else if (qa.query == "creature.use")
		{
			::IGIS.error(error);
		}
	}

	function onSetAvatar()
	{
		this.mVelocityUpdateSchedule = ::_eventScheduler.fireIn(0.80000001, this, "sendPendingVelocityUpdate");
	}

	function queryUsable()
	{
		if (this.hasStatusEffect(this.StatusEffects.IS_USABLE))
		{
			::_Connection.sendQuery("creature.isusable", this, this.getID());
		}
	}

	function setProperties( values )
	{
		this.mProperties = values;
	}

	function getAssemblyQueueTime()
	{
		return this.mAssemblyQueueTime;
	}

	function showErrorDetails()
	{
		local error = this.getAssemblerError();
		this.GUI.MessageBox.show(error ? error.tostring() : "No Error");
	}

	function addProperty( name, value )
	{
		this.mProperties[name] = value;
		local args = [];
		args.append(this.mID);
		args.append(name);
		args.append("" + value);
		this._Connection.sendQuery("scenery.edit", {}, args);
	}

	function getProperties()
	{
		return this.mProperties;
	}

	function isPropCreature()
	{
		if (this.isCreature() && this.mAssembler && ("mSceneryAssembler" in this.mAssembler) && this.mAssembler.getSceneryAssembler())
		{
			return true;
		}

		return false;
	}

	function hasStatusEffect( id )
	{
		return id in this.mStatusEffects;
	}

	function setForceShowWeapon( visible )
	{
		this.mForceShowEquipment = visible;
	}

	function isForceShowWeapon()
	{
		return this.mForceShowEquipment;
	}

	function setStatusModifiers( mods, effects )
	{
		local reztest_WasDead = this.StatusEffects.DEAD in this.mStatusEffects;
		local reztest_IsDead = this.StatusEffects.DEAD in effects;
		local meleeAutoAttackOn = this.StatusEffects.AUTO_ATTACK in this.mStatusEffects;
		local rangedAutoAttackOn = this.StatusEffects.AUTO_ATTACK_RANGED in this.mStatusEffects;

		if (this.mCorking)
		{
			if (::_avatar == this && this.mDead != reztest_IsDead)
			{
				if (reztest_WasDead)
				{
					this.log.debug("REZ: corked: dead status changed: Rezed!");
				}
				else
				{
					this.log.debug("REZ: corked: dead status changed: Died!");
				}
			}

			this.mCorkedStatusEffects = effects;
			this.mCorkedStatusModifiers = mods;
			return;
		}

		if (::_avatar == this && this.mDead != reztest_IsDead)
		{
			if (reztest_WasDead)
			{
				this.log.debug("REZ: dead status changed: Rezed!");
			}
			else
			{
				this.log.debug("REZ: dead status changed: Died!");
			}
		}

		local previousEffects = this.mStatusEffects;
		local previousModifiers = this.mStatusModifiers;
		this.mStatusEffects = effects;
		this.mStatusModifiers = mods;

		if (this == ::_avatar)
		{
			if (meleeAutoAttackOn)
			{
				if (!(this.StatusEffects.AUTO_ATTACK in this.mStatusEffects))
				{
					this.stopAutoAttack(false);
				}
			}

			if (rangedAutoAttackOn)
			{
				if (!(this.StatusEffects.AUTO_ATTACK_RANGED in this.mStatusEffects))
				{
					this.stopAutoAttack(true);
				}
			}
		}

		local buffDebuffScreen;

		if (::_avatar == this)
		{
			buffDebuffScreen = this.Screens.get("PlayerBuffDebuff", false);
		}
		else
		{
			local partyMemberComponent = ::partyManager.getMemberComponent(this.mID);

			if (partyMemberComponent)
			{
				buffDebuffScreen = partyMemberComponent;
			}
		}

		local targetBuffDebuffScreen;

		if (::_avatar != null && ::_avatar.getTargetObject() == this)
		{
			targetBuffDebuffScreen = this.Screens.get("TargetWindow", false);
		}

		this.mUniqueBuffs = [];
		this.mUniqueStatusModifiers = [];

		foreach( mod in this.mStatusModifiers )
		{
			local modExists = false;

			foreach( existingBuff in this.mUniqueBuffs )
			{
				if (mod.getAbilityID() == existingBuff.getAbilityID())
				{
					existingBuff.addStatusModifier(mod);
					modExists = true;
					break;
				}
			}

			if (!modExists)
			{
				local abilityInfo = ::_AbilityManager.getAbilityById(mod.getAbilityID());
				local newBuff = this.BuffDebuff(this, mod, abilityInfo.mForegroundImage + "|" + abilityInfo.mBackgroundImage);
				this.mUniqueBuffs.append(newBuff);
			}
		}

		this.mUniqueStatusEffects = [];

		foreach( k, v in this.mStatusEffects )
		{
			if (v)
			{
				local newStatusEffect = this.StatusEffect(k);
				this.mUniqueStatusEffects.append(newStatusEffect);
			}
		}

		if (this.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
		{
			if (!(this.StatusEffects.UNATTACKABLE in previousEffects))
			{
				this.updateNameBoardColor();
			}
		}
		else if (this.StatusEffects.UNATTACKABLE in previousEffects)
		{
			this.updateNameBoardColor();
		}

		if (this.hasStatusEffect(this.StatusEffects.INVINCIBLE))
		{
			if (!(this.StatusEffects.INVINCIBLE in previousEffects))
			{
				this.updateNameBoardColor();
			}
		}
		else if (this.StatusEffects.INVINCIBLE in previousEffects)
		{
			this.updateNameBoardColor();
		}

		if (this.hasStatusEffect(this.StatusEffects.DAZE) && !(this.StatusEffects.DAZE in previousEffects))
		{
			this.setAbilityEffect(this.cue("Daze"));
		}
		else if (!this.hasStatusEffect(this.StatusEffects.DAZE) && this.StatusEffects.DAZE in previousEffects)
		{
			this.interruptAbility(-1);
		}

		if (this.hasStatusEffect(this.StatusEffects.STUN) && !(this.StatusEffects.STUN in previousEffects))
		{
			this.setAbilityEffect(this.cue("Stun"));
		}
		else if (!this.hasStatusEffect(this.StatusEffects.STUN) && this.StatusEffects.STUN in previousEffects)
		{
			this.interruptAbility(-1);
		}

		this.mUniqueStatusModifiers = this.mUniqueBuffs;

		foreach( statusEffect in this.mUniqueStatusEffects )
		{
			this.mUniqueStatusModifiers.append(statusEffect);
		}

		if (buffDebuffScreen)
		{
			buffDebuffScreen.updateMods(this.mUniqueStatusModifiers);
		}

		if (targetBuffDebuffScreen)
		{
			targetBuffDebuffScreen.updateMods(this);
		}

		foreach( effectToAdd in this.mUniqueStatusEffects )
		{
			if (::_avatar == this)
			{
				::_tutorialManager.onStatusEffectSet(effectToAdd.getEffectID());
			}
		}

		if (!this.mDead)
		{
			if (this.hasStatusEffect(this.StatusEffects.DEAD))
			{
				this.mDead = true;
				this.Util.updateMiniMapStickers();
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", false);

				if (selfTargetWindow)
				{
					selfTargetWindow.fillOut();
				}

				local portalRequestScreen = this.Screens.get("PortalRequest", false);

				if (portalRequestScreen)
				{
					portalRequestScreen.setVisible(false);
				}

				if (::_avatar && ::_avatar.getTargetObject() == this)
				{
					local targetWindow = this.Screens.get("TargetWindow", false);

					if (targetWindow)
					{
						targetWindow.fillOut(this);
					}
				}

				this.onDeath();

				if (this == ::_avatar)
				{
					::Screens.show("RezScreen");
					::_avatar.setTargetObject(null);
					this.Screens.get("RezScreen", false).start();
				}
			}

			if (this.mAssembled)
			{
				this.checkAndSetCTFFlag();
			}

			this.queryUsable();
		}
		else if (!this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			this.mDead = false;
			this.onRes();
		}

		if (this == ::_avatar)
		{
			this.updateInvisibilityScreen();
		}

		local inCombat = this.hasStatusEffect(this.StatusEffects.IN_COMBAT);
		local autoAttack = this.hasStatusEffect(this.StatusEffects.AUTO_ATTACK);
		local autoAttackRanged = this.hasStatusEffect(this.StatusEffects.AUTO_ATTACK_RANGED);
		local combatStand = this.hasStatusEffect(this.StatusEffects.IN_COMBAT_STAND);

		if (this == ::_avatar)
		{
			if (this.mInCombat && !inCombat)
			{
				local selfTargetWindow = this.Screens.get("SelfTargetWindow", true);

				if (selfTargetWindow)
				{
					selfTargetWindow.startCombatEndBlinkdown();
				}
			}
			else if (!this.mInCombat && inCombat)
			{
				local portalRequestScreen = this.Screens.get("PortalRequest", false);

				if (portalRequestScreen)
				{
					portalRequestScreen.setVisible(false);
				}

				local selfTargetWindow = this.Screens.get("SelfTargetWindow", true);

				if (selfTargetWindow)
				{
					selfTargetWindow.endCombatBlinkdown();
				}
			}
		}

		this.mInCombat = inCombat;

		if (this.mAnimationHandler)
		{
			if (combatStand)
			{
				this.mAnimationHandler.setCombat(true);
				this.mAnimationHandler.setIdleState("$IDLE_COMBAT$");
			}
			else
			{
				this.mAnimationHandler.setCombat(false);
				this.mAnimationHandler.setIdleState("Idle");
			}
		}

		inCombat = inCombat || autoAttack || autoAttackRanged;

		if (inCombat)
		{
			this.mAttemptingCombat = true;

			if (::_avatar == this)
			{
				if (autoAttackRanged && this.hasWeapon(this.ItemEquipSlot.WEAPON_RANGED))
				{
					this.setVisibleWeapon(this.VisibleWeaponSet.RANGED, false);
				}
				else if (this.hasWeapon(this.ItemEquipSlot.WEAPON_MAIN_HAND))
				{
					if (!::_avatar.hasWeaponSet(this.VisibleWeaponSet.RANGED))
					{
						this.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
					}
				}
			}
		}
	}

	function getUniqueBuffs()
	{
		return this.mUniqueBuffs;
	}

	function getUniqueEffects()
	{
		return this.mUniqueStatusEffects;
	}

	function getUniqueStatusModifiers()
	{
		return this.mUniqueStatusModifiers;
	}

	function updateInvisibilityScreen()
	{
		local invisScreen = this.Screens.get("InvisibilityScreen", true);

		if (invisScreen)
		{
			if ((this.hasStatusEffect(this.StatusEffects.INVISIBLE) || this.hasStatusEffect(this.StatusEffects.WALK_IN_SHADOWS)) && !this.hasStatusEffect(this.StatusEffects.GM_INVISIBLE))
			{
				if (!invisScreen.isVisible())
				{
					invisScreen.reset();
					invisScreen.setVisible(true);
				}
			}
			else
			{
				invisScreen.setVisible(false);
			}
		}
	}

	function getStatusModifiers()
	{
		return this.mStatusModifiers;
	}

	function getObjectClass()
	{
		return this.mObjectClass;
	}

	function isScenery()
	{
		return this.mIsScenery;
	}

	function isCreature()
	{
		return !this.mIsScenery;
	}

	function isPlayer()
	{
		return this.mCreatureDef && this.mCreatureDef.isPlayer();
	}

	function _tostring()
	{
		if (this == this._avatar)
		{
			return this.getNodeName() + " (Avatar)";
		}

		local str = this.getNodeName();

		if (this.mAssembler)
		{
			str += " (" + this.mAssembler.getAssemblerDesc() + ")";
		}

		return str;
	}

	function getAssembler()
	{
		return this.mAssembler;
	}

	function getAssemblerError()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getArchiveError();
		}

		return null;
	}

	function isAssembled()
	{
		return this.mAssembled;
	}

	function forceUpdate()
	{
		this.mFrameCounter = this.FORCE_UPDATE;
		this.mForceUpdate = true;
	}

	function prepareAssemblingNode()
	{
		if (this.mAssemblingNode != null)
		{
			return;
		}

		this.mAssemblingNode = this._scene.createSceneNode("Assembling/" + this.mID);
		local assemblingNode = this.getAssemblingNode();

		if (assemblingNode)
		{
			local boxNode = this._scene.createEntity(this.mAssemblingNode.getName() + "/InvisBox", "Manipulator-ClickBox.mesh");
			boxNode.setVisibilityFlags(this.VisibilityFlags.ANY);
			boxNode.setQueryFlags(this.QueryFlags.ANY | this.QueryFlags.MANIPULATOR);
			boxNode.setOpacity(0.0);
			local loadEntity = this._scene.createEntity(this.mAssemblingNode.getName() + "/Creature_Load", "Manipulator-Creature_Load.mesh");
			loadEntity.setAnimationUnitCount(1);
			loadEntity.getAnimationUnit(0).setIdleState("Idle");
			loadEntity.getAnimationUnit(0).setEnabled(true);
			assemblingNode.setAutoTracking(true, this._camera.getParentSceneNode());
			assemblingNode.setFixedYawAxis(true);
			loadEntity.setVisibilityFlags(this.VisibilityFlags.ANY);
			this.log.debug("attachObject(boxNode)");
			assemblingNode.attachObject(boxNode);
			this.log.debug("attachObject(loadEntity)");
			assemblingNode.attachObject(loadEntity);
			::_scene.getRootSceneNode().addChild(assemblingNode);
		}
	}

	function _setAssembled( value )
	{
		if (value)
		{
			this.markAsGarbage(false);
			this.mAssembling = false;
			this.forceUpdate();

			if (this.hasStatusEffect(this.StatusEffects.CARRYING_RED_FLAG))
			{
				local flagLoadedCallback = {
					sceneObject = this,
					function onPackageComplete( pkg )
					{
						this.sceneObject.checkAndSetCTFFlag();
					}

				};
				::_contentLoader.load("Armor-Base1A-Helmet", this.ContentLoader.PRIORITY_NORMAL, "Red Flag", flagLoadedCallback);
			}
			else if (this.hasStatusEffect(this.StatusEffects.CARRYING_BLUE_FLAG))
			{
				local flagLoadedCallback = {
					sceneObject = this,
					function onPackageComplete( pkg )
					{
						this.sceneObject.checkAndSetCTFFlag();
					}

				};
				::_contentLoader.load("Armor-Base1A-Helmet", this.ContentLoader.PRIORITY_NORMAL, "Blue Flag", flagLoadedCallback);
			}
		}
		else
		{
			if (this.isScenery() && this.mAssembler)
			{
				this._sceneObjectManager._cancelCSMBuild(this.getName() + "/" + this.mAssembler.getName());
			}

			this.mCurrentFadeLevel = 0.0;
		}

		if (value && this.mAssemblingNode)
		{
			local wasShowingSelection = this.isShowingSelection();

			if (wasShowingSelection)
			{
				this.mSelectionNode.detachObject(this.mSelectionProjector);
				this.mAssemblingNode.removeChild(this.mSelectionNode);
				this.mSelectionProjector.destroy();
				this.mSelectionNode.destroy();
				this.mSelectionProjector = null;
				this.mSelectionNode = null;
			}

			this.mAssemblingNode.destroy();
			this.mAssemblingNode = null;

			if (wasShowingSelection)
			{
				this.setShowingSelection(wasShowingSelection);
			}
		}

		this.mAssembled = value;
		this._sceneObjectManager._updateAssemblyStatus(this, this.mAssembled);

		if (this.mAssembled && this.isCreature())
		{
			if (this.mCreatureDef && (this.getMeta("quest_giver") || this.getMeta("quest_ender") || this.hasStatusEffect(this.StatusEffects.IS_USABLE)) && this.mQuestIndicator == null)
			{
				if (this.mQuestIndicator == null)
				{
					this.mQuestIndicator = this.QuestIndicator(this, this.mID, this.mNode, this.getNamePlatePosition());
				}

				this.mQuestIndicator.setCreatureId(this.mID);
				this.mQuestIndicator.requestQuestIndicator();
			}

			local currWeapon = this.getStat(this.Stat.VIS_WEAPON);

			if (!currWeapon)
			{
				currWeapon = this.mVisibleWeaponSet;
			}

			this.setVisibleWeapon(this.VisibleWeaponSet.INVALID, false, false);

			if (currWeapon != this.VisibleWeaponSet.INVALID)
			{
				this.setVisibleWeapon(currWeapon, false, false);
			}
			else
			{
				this.setVisibleWeapon(this.VisibleWeaponSet.NONE, false, false);
			}

			local pt = this.Util.safePointOnFloor(this.getPosition());
			this.setDistanceToFloor(this.getPosition().y - pt.y);

			if (this.mDistanceToFloor < 0.0)
			{
				this.mDistanceToFloor = 0.0;
			}
		}

		this._positionName();

		if (value)
		{
			if (this.mEffectsHandler != null)
			{
				this.mEffectsHandler.onAssembled();
			}

			if (this.isDead() == true)
			{
			}
			else
			{
				local controller = this.getController();

				if (controller)
				{
					if (("mServerPosition" in controller) && controller.mServerPosition != null)
					{
						controller.mServerPosition.y = this.getPosition().y;
					}

					if (("mLastServerPos" in controller) && controller.mLastServerPos != null)
					{
						controller.mLastServerPos.y = this.getPosition().y;
					}
				}

				this.mLastServerUpdate = null;
				this.fireUpdate();
			}

			this.updateInteractParticle();
			this.queryUsable();
		}
		else
		{
			if (this.mQuestIndicator)
			{
				this.mQuestIndicator.destroy();
				this.mQuestIndicator = null;
			}

			::_useableCreatureManager.removeFromCache(this.getID());
			this.removeInteractParticle();
		}
	}

	function _getDefaultNamePlatePosition()
	{
		local position = this.getBoundingBox().getMaximum();
		position.y *= this.getScale().y;
		position.y += 3.0;
		local name = !this.mAssembler ? null : this.mAssembler.getPropName();

		if (name && name in this.ForcedQuestMarkerYPosition)
		{
			position.y = this.ForcedQuestMarkerYPosition[name];
		}

		return position;
	}

	function getNamePlatePosition()
	{
		this._calculateNamePlatePosition();
		return this.mNamePlatePosition;
	}

	function _calculateNamePlatePosition()
	{
		this.mNamePlatePosition = null;
		local entity = this.getEntity();

		if (entity)
		{
			local namePointAttachment = this.mAssembler.getAttachPointDef("nameplate");

			if (namePointAttachment)
			{
				local boneNameplate = namePointAttachment.bone;

				if (entity.hasBone(boneNameplate))
				{
					this.mNamePlatePosition = entity.getBoneDerivedPosition(boneNameplate);

					if ("position" in namePointAttachment)
					{
						this.mNamePlatePosition += namePointAttachment.position;
					}

					if ("scale" in namePointAttachment)
					{
						this.mNamePlateScale = namePointAttachment.scale.y;
					}
				}
			}
			else if (entity.hasBone("Bone-Head"))
			{
				this.mNamePlatePosition = entity.getBoneDerivedPosition("Bone-Head");
			}

			if (this.mNamePlatePosition)
			{
				this.mNamePlatePosition.y *= this.getScale().y;
				this.mNamePlatePosition.y += 3.0;
			}
		}

		if (!this.mNamePlatePosition)
		{
			this.mNamePlatePosition = this._getDefaultNamePlatePosition();
		}
	}

	function removeInteractParticle()
	{
		if (this.mInteractParticle)
		{
			this.mInteractParticle.destroy();
			this.mInteractParticle = null;
			this.mInteractParticleNode.destroy();
			this.mInteractParticleNode = null;
		}
	}

	function updateInteractParticle()
	{
		local MIN_SCALE = 1.0;
		local MAX_SCALE = 2.5;

		if (this.mInteractParticleNode != null)
		{
			local boundingRadius = this.getBoundingRadius();
			local size = boundingRadius / 3.0;

			if (size < MIN_SCALE)
			{
				size = MIN_SCALE;
			}
			else if (size > MAX_SCALE)
			{
				size = MAX_SCALE;
			}

			this.mInteractParticleNode.setScale(this.Vector3(size, size, size));
			this.mInteractParticle.setVisible(this.mAssembled);
		}
	}

	function addInteractParticle()
	{
		if (this.mInteractParticle == null && this.mNode != null)
		{
			local uniqueName = this.mNode.getName() + "/Interact_Particle";
			this.mInteractParticle = ::_scene.createParticleSystem(uniqueName, "Par-Ground_Sparkle");
			this.mInteractParticle.setVisibilityFlags(this.VisibilityFlags.ANY);
			this.mInteractParticleNode = this.mNode.createChildSceneNode();
			this.mInteractParticleNode.attachObject(this.mInteractParticle);
			this.updateInteractParticle();
		}
	}

	function getQuestIndicator()
	{
		return this.mQuestIndicator;
	}

	function disassemble()
	{
		if (this.mAssembler && this.mAssembled)
		{
			if (this.mAbilityEffect)
			{
				this.mAbilityEffect.destroy();
				this.mAbilityEffect = null;
			}

			this.broadcastMessage("onDisassemble");
			this.mAssembler.disassemble(this);
			this.Assert.isEqual(this.mAssembled, false);
		}
	}

	function reassemble( ... )
	{
		local assembleNow = vargc > 0 ? vargv[0] : false;

		if (!this.mAssembler && !this.mPickedAssembler)
		{
			this.setAssembler(null);
		}

		if (this.mAssembler)
		{
			if (assembleNow)
			{
				return this.mAssembler.reassemble(this);
			}
			else
			{
				if (this == null)
				{
					this.log.debug("REASSEMBLE - NULL THIS!");
				}

				this.mAssembler.disassemble(this);
				this._sceneObjectManager.queueAssembly(this);
			}
		}

		return false;
	}

	function shouldRender()
	{
		if (!this.mNode)
		{
			return false;
		}

		if (this == ::_avatar)
		{
			return true;
		}

		return this.isAlwaysVisible() || this.mLODLevel < 5;
	}

	function isPageReady()
	{
		local cpage = this.getTerrainPageCoords();
		return !this._sceneObjectManager.isPagePending(this._sceneObjectManager.mCurrentZoneDefId, cpage.x, cpage.z);
	}

	function setAbilityEffect( eff )
	{
		this.mAbilityEffect = eff;
	}

	function interruptAbility( abId )
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilityCancel");
		}

		this.mAbilityEffect = null;
	}

	function setbackAbility()
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilitySetback");
		}
	}

	function warmupComplete()
	{
		if (this.mAbilityEffect)
		{
			this.mAbilityEffect.dispatch("onAbilityWarmupComplete");
		}
	}

	function handleSpecialCaseUsage( so )
	{
		local callback = {
			creature = so,
			function onActionSelected( mb, alt )
			{
				if (alt == "Ok")
				{
					if (creature.hasStatusEffect(this.StatusEffects.USABLE_BY_COMBATANT) || !::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT))
					{
						if (::_useableCreatureManager.isUseable(this.creature.getID()))
						{
							::_Connection.sendQuery("creature.use", this, this.creature.getID());
						}
					}
				}
			}

		};

		if (!::_useableCreatureManager.isUseable(so.getID()))
		{
			return false;
		}

		switch(so.getCreatureDef().getID())
		{
		case 2100:
			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.Util.getRangeOffset(this._avatar, so) + this.MAX_USE_DISTANCE)
			{
				local popupBox = this.GUI.MessageBox.showOkCancel("You are about to leave Corsica. Once you enter the Southend Passage you will not be able to return to Corsica." + " Make sure you have finished any remaining quests before you continue!", callback);
				return true;
			}

			break;

		case 2198:
			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.Util.getRangeOffset(this._avatar, so) + this.MAX_USE_DISTANCE)
			{
				local popupBox = this.GUI.MessageBox.showOkCancel("You are about to leave the new player area. Once you enter the portal you will be teleported to Anglorum." + " You will not be able to return to this area again. Make sure you have finished any remaining quests before you continue!", callback);
				return true;
			}

			break;
		}

		return false;
	}

	function isUsableDistance( targetSo )
	{
		if (!targetSo)
		{
			return false;
		}

		local sqDistance = this.getPosition().squaredDistance(targetSo.getPosition());
		local usableDistance = this.Util.getRangeOffset(this, targetSo);
		usableDistance *= usableDistance;
		usableDistance += this.MAX_USE_DISTANCE_SQ;
		return sqDistance <= usableDistance;
	}

	function detachParticleSystem (tag)
	{
		if(tag in this.mParticleAttachments)
		{
			local particles = this.mParticleAttachments[tag];
			particles[0].destroy();
			particles[1].destroy();
			delete this.mParticleAttachments[tag];
		}
	}
	
	function attachParticleSystem(name, tag, size)
	{
			// TODO make more unique
			local uniqueName = this.mNode.getName() + "/" + name;
			local particle = ::_scene.createParticleSystem(uniqueName, name);
			particle.setVisibilityFlags(this.VisibilityFlags.ANY);
			local particleNode = this.mNode.createChildSceneNode();
			particleNode.attachObject(particle);
			particleNode.setScale(this.Vector3(size, size, size));
			particle.setVisible(this.mAssembled);
			this.mParticleAttachments[tag] <- [particle, particleNode];
			return uniqueName;
	}

	function useCreature( so )
	{
		local pvpable = so.hasStatusEffect(this.StatusEffects.PVPABLE);
		local creatureUsed = true;
		::_avatar.setResetTabTarget(true);

		if (!(so == ::_avatar))
		{
			::_avatar.setTargetObject(so);
		}

		local useageType = "";

		if (this.handleSpecialCaseUsage(so))
		{
			useageType = "Special";
		}
		else if (!so.isDead() && this.Key.isDown(this.Key.VK_SHIFT))
		{
			this.sendConMessage(::_avatar.getStat(this.Stat.LEVEL), so.getStat(this.Stat.LEVEL));
			useageType = "Consider";
		}
		else if (so.isDead() && so.hasLoot())
		{
			if (this.isDead())
			{
				this.IGIS.error("You cannot loot while dead.");
				return;
			}

			if (this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition()) <= this.MAX_USE_DISTANCE)
			{
				useageType = "Loot";
				local lootScreen = this.Screens.get("LootScreen", true);

				if (lootScreen)
				{
					if (lootScreen.checkLootingPermissions(so))
					{
						if (this.Key.isDown(this.Key.VK_SHIFT))
						{
							lootScreen.setAutoLoot(true);
						}
						else
						{
							lootScreen.setAutoLoot(false);
						}

						lootScreen.setTitle(so.getName());
						lootScreen.populateLoot(so.getID());
						lootScreen.setVisible(false, true);
					}
				}
			}
			else
			{
				::_avatar.getController().startFollowing(so, true);
			}
		}
		else if (so.getMeta("persona") && !pvpable)
		{
			::_playTool.setupCreatureMenu(so);
			::_playTool.mMenu.showMenu();
			::_playTool.mRotating = false;
			useageType = "PlayerMenu";
		}
		else if (!so.isDead() && !so.hasStatusEffect(this.StatusEffects.UNATTACKABLE))
		{
			local forceAAUpdate = false;
			local distance = this.Math.DetermineDistanceBetweenTwoPoints(this._avatar.getPosition(), so.getPosition());
			local meleeAA = this._AbilityManager.getAbilityByName("melee");
			local range = meleeAA.getRange();
			local mouseMoveEnabled = true;
			local rangeOffset = this.Util.getRangeOffset(this._avatar, so);

			if (distance < range + rangeOffset)
			{
				if (::_avatar.isRangedAutoAttackActive())
				{
					::_avatar.stopAutoAttack(true);
					forceAAUpdate = true;
				}

				::_avatar.setVisibleWeapon(this.VisibleWeaponSet.MELEE, false);
				::_avatar.startAutoAttack(false, forceAAUpdate);
			}
			else
			{
				::_avatar.getController().startFollowing(so, true);
				creatureUsed = false;
			}

			useageType = "Attack";
		}
		else if (this.isUsableDistance(so))
		{
			if (this.isDead())
			{
				this.IGIS.error("You cannot talk to creatures while dead.");
				return;
			}

			if (so.getMeta("copper_shopkeeper"))
			{
				local shopScreen = this.Screens.get("ItemShop", true);
				shopScreen.setMerchantId(so.getID());
				shopScreen.setShopScreenCategory(this.CurrencyCategory.COPPER);
				this.Screens.show("ItemShop", true);
				useageType = "CopperShop";
			}
			else if (so.getMeta("credit_shopkeeper"))
			{
				local shopScreen = this.Screens.get("ItemShop", true);
				shopScreen.setMerchantId(so.getID());
				shopScreen.setShopScreenCategory(this.CurrencyCategory.CREDITS);
				this.Screens.show("ItemShop", true);
				useageType = "CreditShop";
			}
			else if (so.getMeta("essence_vendor"))
			{
				local essenceScreen = this.Screens.get("EssenceShop", true);
				essenceScreen.setMerchantId(so.getID());
				essenceScreen.setVisible(true);
				useageType = "EssenceShop";
			}
			else if (so.getMeta("vault"))
			{
				local vaultScreen = this.Screens.get("Vault", true);

				if (vaultScreen)
				{
					vaultScreen.setVaultId(so.getID());
					vaultScreen.setVisible(true);
				}

				useageType = "Vault";
			}
			else if (so.getMeta("clan_registrar"))
			{
				local socialWindow = ::Screens.get("SocialWindow", true);

				if (socialWindow && socialWindow.isClanLeader())
				{
					::_playTool.beginClanTransfer();
				}
				else
				{
					local callback = {
						playTool = ::_playTool,
						function onActionSelected( mb, alt )
						{
							if (alt == "Yes")
							{
								::_playTool.beginClanCreation();
							}
						}

					};
					local copperAmt = 0;

					if (::_avatar)
					{
						copperAmt = ::_avatar.getStat(this.Stat.COPPER);
					}

					local amountNeeded = this.gCopperPerGold * 10;

					if (copperAmt < amountNeeded)
					{
						this.GUI.MessageBox.showYesNo("You do not have enough gold to create a clan.  Are you sure you want to continue?", callback);
					}
					else
					{
						::_playTool.beginClanCreation();
					}
				}

				useageType = "Clan";
			}
			else if (so.getMeta("crafter"))
			{
				local craftScreen = this.Screens.get("CraftingWindow", true);
				craftScreen.clearCraftingWindow();
				craftScreen.setCrafterId(so.getID());
				this.Screens.show("CraftingWindow", true);
				useageType = "Crafter";
			}
			else if (so.getMeta("credit_shop") != null)
			{
				local creditShop = this.Screens.get("CreditShop", true);

				if (creditShop)
				{
					local metaData = so.getMeta("credit_shop");
					creditShop.setVisible(true);
					creditShop.selectPanel(metaData);
				}

				useageType = "CreditPurchaseShop";
			}
			else if (so.hasStatusEffect(this.StatusEffects.TRANSFORMER))
			{
				local mscreen = this.Screens.get("MorphItemScreen", true);
				mscreen.reset();
				mscreen.setMorpherId(so.getID());
				this.Screens.show("MorphItemScreen");
				useageType = "Transformer";
			}
			else if (so.getQuestIndicator() && so.getQuestIndicator().hasValidQuest())
			{
				::_questManager.requestQuestOffer(so.getQuestIndicator().getCreatureId());
				useageType = "RequestQuest";
			}
			else if (so.getQuestIndicator() && so.getQuestIndicator().hasCompletedNotTurnInQuest())
			{
				::_questManager.requestCompleteNotTurnInQuest(so.getQuestIndicator().getCreatureId());
				useageType = "RequestComplete";
			}
			else if (::_useableCreatureManager.isUseable(so.getID()))
			{
				if (so.hasStatusEffect(this.StatusEffects.USABLE_BY_COMBATANT) || !::_avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT))
				{
					::_Connection.sendQuery("creature.use", this, so.getID());
					useageType = "UseCreature";
				}
				else
				{
					this.IGIS.info("You cannot interact with this when you\'re in combat.");
				}
			}
		}
		else if (!so.isDead())
		{
			if (::_avatar.getController().canInteractWith(so))
			{
				::_avatar.getController().startFollowing(so, true);
			}
			else
			{
				if (this.Pref.get("gameplay.mousemovement") == true)
				{
					local distance = this.Math.manhattanDistanceXZ(this._avatar.getPosition(), so.getPosition());

					if (distance < 500)
					{
						::_avatar.getController().startFollowing(so, true);
					}
				}

				creatureUsed = false;
			}
		}
		else
		{
			creatureUsed = false;
		}

		if (creatureUsed)
		{
			::_tutorialManager.onCreatureUsed(so.getID(), useageType);
		}

		return creatureUsed;
	}

	function gone()
	{
		if (this.mAssembled == false || this.mOpacity <= 0.0)
		{
			this.mGone = true;
			this.destroy();
			return;
		}

		this.mDesiredFadeLevel = 0.0;
		this.mFadeTarget = 0.0;
		this.mGone = true;
	}

	function destroy()
	{
		this.mDestroying = true;
		this._sceneObjectManager.removeUpdateListener(this);

		if (this.mVelocityUpdateSchedule)
		{
			::_eventScheduler.cancel(this.mVelocityUpdateSchedule);
			this.mVelocityUpdateSchedule = null;
		}

		if (::_buildTool)
		{
			this._buildTool.getSelection().remove(this);
		}

		this.setShowingShadow(false);
		this.setShowName(false);
		local name = this.getName();

		if (::_avatar && ::_avatar.getTargetObject() == this)
		{
			this.setResetTabTarget(true);
			this.setTargetObject(null);
		}

		if (::_Environment.isMarker(this))
		{
			::_Environment.removeMarker(this);
		}

		if (this.mEffectsHandler)
		{
			this.mEffectsHandler = this.mEffectsHandler.destroy();
		}

		if (this.mController)
		{
			this.mController = this.mController.destroy();
		}

		if (this.mHeadLight)
		{
			this.setHeadLight(false);
		}

		if (this.mAttachments.len() > 0)
		{
			local i;
			local slot;

			foreach( i, slot in this.Util.tableKeys(this.mAttachments) )
			{
				local io = this.mAttachments[slot];
				this.removeAttachment(io);
				io.destroy();
			}
		}

		this.mAttachments = {};

		foreach( att in this.mWeapons )
		{
			att.destroy();
		}

		foreach( att in this.mAttachmentOverride )
		{
			att.destroy();
		}

		this.mWeapons = {};
		this.mAttachmentOverride = {};
		this.disassemble();

		if (this.mAssembler)
		{
			this.mAssembler.removeManagedInstance(this);
			this.mAssembler = null;
		}

		this._sceneObjectManager._remove(this);
		this.mFlags = 0;
		this.stopSounds();

		if (::_avatar == this)
		{
			::_scene.setSoundListener(null);
			::_avatar = null;
		}

		if (this.mAssemblingNode)
		{
			this.mAssemblingNode.destroy();
			this.mAssemblingNode = null;
		}

		if (this.mQuestIndicator)
		{
			this.mQuestIndicator.destroy();
			this.mQuestIndicator = null;
		}

		if (this.mNode)
		{
			this.mNode.destroy();
			this.mNode = null;
		}
	}

	function onOtherDestroy( so )
	{
		if (so == this.mTargetObject)
		{
			this.setResetTabTarget(true);
			this.setTargetObject(null);
		}
	}

	function getVarsTypeAsAsset()
	{
		local a = this.AssetReference(this.mType);

		if (this.mVars && this.mVars.len() > 0)
		{
			a.setVars(this.mVars);
		}

		return a;
	}

	function setVarsTypeFromAsset( assetRef )
	{
		local archive = this.GetAssetArchive(assetRef.getAsset());

		if (archive == null)
		{
			throw this.Exception("Cannot determine archive for: " + assetRef.getAsset() + " (forget to rebuild the catalog?)");
		}

		this.setType(assetRef.getAsset(), assetRef.getVars());

		if (this.isAssembled())
		{
			this.reassemble();
		}
		else
		{
			this._eventScheduler.fireIn(1.0, this, "reassemble");
		}
	}

	function getType()
	{
		return this.mType;
	}

	function setTypeFromString( pType )
	{
		local tmp = this.AssetReference(pType);
		return this.setType(tmp.getAsset(), tmp.getVars());
	}

	function getTypeString()
	{
		return this.mTypeString;
	}

	function setType( pType, ... )
	{
		if (pType == null)
		{
			throw this.Exception("Invalid type: " + pType);
		}

		local oldVarStr = this.mVars == null ? null : this.System.encodeVars(this.mVars);
		local newVarStr;

		if (vargc > 0)
		{
			local vars = vargv[0];

			if (typeof vars == "string")
			{
				vars = this.System.decodeVars(vars);
			}

			this.mVars = vars;
			newVarStr = this.mVars == null ? null : this.System.encodeVars(this.mVars);
		}
		else
		{
			newVarStr = oldVarStr;
		}

		if (pType == this.mType && newVarStr == oldVarStr)
		{
			return false;
		}

		this.mTypeString = pType + (newVarStr != null ? "?" + newVarStr : "");
		this.mType = pType;

		if (typeof this.mID == "integer" && this.isCreature())
		{
			this.mCreatureDef = ::_creatureDefManager.getCreatureDef(this.mType);
		}
		else
		{
			this.mCreatureDef = null;
		}

		if (typeof this.mID == "integer" && ::_Environment.isMarker(this))
		{
			::_Environment.addMarker(this);
		}

		if (this.mPickedAssembler == null)
		{
			this.setAssembler(null);
		}

		if (this.isCreature())
		{
			this.setController(::_avatar == this ? "Avatar2" : "Creature");
		}
		else
		{
			this.setController(null);
		}

		return true;
	}

	function getDef()
	{
		return this.mDef;
	}

	function isInteractive()
	{
		return this.hasStatusEffect(this.StatusEffects.IS_USABLE);
	}

	function setAssembler( assembler )
	{
		this.mPickedAssembler = assembler;
		local wasAssembled = this.mAssembled;

		if (this.mAssembler)
		{
			this.disassemble();
			this.mAssembler.removeManagedInstance(this);
			this.mAssembler = null;
		}

		this.mAnimationHandler = null;
		this.mAnimationState = null;
		this.mDef = {};

		if (assembler == null)
		{
			this.mAssembler = this.GetAssembler(this.mObjectClass, this.mType);
		}
		else
		{
			this.mAssembler = assembler;
		}

		if (this.mAssembler)
		{
			this.mAssembler.addManagedInstance(this);
		}

		if (wasAssembled)
		{
			this.reassemble();
		}

		if (this.isCreature())
		{
			this._sceneObjectManager.addUpdateListener(this);
		}

		this.broadcastMessage("onAssembled");
	}

	function getAttachPointDef( pointName )
	{
		if (!this.mAssembler)
		{
			return null;
		}

		return this.mAssembler.getAttachPointDef(pointName);
	}

	function getCreatureDef()
	{
		return this.mCreatureDef;
	}

	function getDefaultAssembler()
	{
		return this.GetAssembler(this.mObjectClass, this.mType);
	}

	function _weaponSlot( slot )
	{
		switch(slot)
		{
		case this.ItemEquipSlot.WEAPON_MAIN_HAND:
		case this.ItemEquipSlot.WEAPON_OFF_HAND:
		case this.ItemEquipSlot.WEAPON_RANGED:
			return true;
		}

		return false;
	}

	function addAttachment( io )
	{
		this.mAttachments[io.getID()] <- io;
		io.setAttachedTo(this);
		this.broadcastMessage("onAttachmentAdded", io);

		if (this.mAnimationHandler && io.mAssemblyData && ("right_hand" == io.mAttachmentPointName || "left_hand" == io.mAttachmentPointName))
		{
			this.updateGrip();
		}
	}

	function addAttachmentOverride( io, slot )
	{
		local attachmentPoint = "right_hand";

		if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			attachmentPoint = "left_hand";
		}

		foreach( attachment in this.mAttachments )
		{
			if (attachment.getAttachmentPointName() == attachmentPoint)
			{
				this.removeAttachment(attachment);

				if (!(slot in this.mWeapons) && !(slot in this.mAttachmentOverride))
				{
					this.mWeapons[slot] <- attachment;
				}
			}
		}

		this.mAttachmentOverride[slot] <- io;
		this.addAttachment(io);
	}

	function removeAttachmentOverride( slot )
	{
		if (!(slot in this.mAttachmentOverride))
		{
			return;
		}

		local attachmentPoint = "right_hand";

		if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
		{
			attachmentPoint = "left_hand";
		}

		if (this.mAttachmentOverride[slot].getID() in this.mAttachments)
		{
			delete this.mAttachments[this.mAttachmentOverride[slot].getID()];
		}

		this.mAttachmentOverride[slot].destroy();
		delete this.mAttachmentOverride[slot];

		if (slot in this.mWeapons)
		{
			local new_weapon = this.Item.Attachable(null, this.mWeapons[slot].mMeshName, attachmentPoint, this.mWeapons[slot].mColors, this.mWeapons[slot].mEffectName);
			this.addAttachment(new_weapon);
			new_weapon.assemble();
		}
	}

	function removeAttachment( io )
	{
		if (io.getID() in this.mAttachments)
		{
			this.broadcastMessage("onAttachmentRemoved", io);
			delete this.mAttachments[io.getID()];
			io.destroy();

			if (!this.mDestroying && this.mAnimationHandler && ("right_hand" == io.mAttachmentPointName || "left_hand" == io.mAttachmentPointName))
			{
				this.updateGrip();
			}
		}
	}

	function hideWeapons()
	{
		foreach( att in this.mAttachments )
		{
			foreach( w in this.mWeapons )
			{
				if (att.getID() == w.getID())
				{
					this.removeAttachment(att);
				}
			}
		}

		this.updateSheathedWeapons();
	}

	function inWeaponSet( set, slot )
	{
		switch(set)
		{
		case this.VisibleWeaponSet.MELEE:
			return slot == this.ItemEquipSlot.WEAPON_MAIN_HAND || slot == this.ItemEquipSlot.WEAPON_OFF_HAND;

		case this.VisibleWeaponSet.RANGED:
			return slot == this.ItemEquipSlot.WEAPON_RANGED;
		}

		return false;
	}

	function showWeapons()
	{
		foreach( s, w in this.mWeapons )
		{
			if (this.inWeaponSet(this.mVisibleWeaponSet, s))
			{
				local att_point = w.mAttachmentPointName;

				if (w.getWeaponType() == "Bow")
				{
					att_point = "left_hand";
				}

				local new_weapon = this.Item.Attachable(null, w.mMeshName, att_point, w.mColors, w.mEffectName);
				this.addAttachment(new_weapon);
				new_weapon.assemble();
				local oldWeaponAttachemnt = this.mWeapons[s];

				if (oldWeaponAttachemnt)
				{
					this.removeAttachment(oldWeaponAttachemnt);
				}

				this.mWeapons[s] <- new_weapon;
			}
		}

		if (this.mVisibleWeaponSet == this.VisibleWeaponSet.MELEE)
		{
			this.removedSheathedWeapons();
		}

		this.updateGrip();
	}

	function updateSheathedWeapons()
	{
		if (this.mForceShowEquipment || this.mAssembler == null || !("mRequestedItems" in this.mAssembler))
		{
			return;
		}

		local unserializedAppearance = this.mAssembler.mRequestedItems;

		if (unserializedAppearance == null)
		{
			return;
		}

		local visibleWeapons = this.getStat(this.Stat.VIS_WEAPON);

		if (visibleWeapons != null && visibleWeapons)
		{
			return;
		}

		if (this.mWeapons.len() == 0)
		{
			return;
		}

		this.removedSheathedWeapons();

		if ((this.ItemEquipSlot.WEAPON_MAIN_HAND in this.mWeapons) && this.ItemEquipSlot.WEAPON_MAIN_HAND in unserializedAppearance)
		{
			local mainHand = unserializedAppearance[this.ItemEquipSlot.WEAPON_MAIN_HAND];
			local mainHandCallback = {
				so = this,
				function doWork( itemDef )
				{
					local unserializedAppearance = this.so.mAssembler.mRequestedItems;

					if (unserializedAppearance && unserializedAppearance[this.ItemEquipSlot.WEAPON_MAIN_HAND] == itemDef.mID)
					{
						this.so.sheathWeapon(itemDef, this.ItemEquipSlot.WEAPON_MAIN_HAND);
					}
				}

			};
			::_ItemDataManager.getItemDef(mainHand, mainHandCallback);
		}

		if ((this.ItemEquipSlot.WEAPON_OFF_HAND in this.mWeapons) && this.ItemEquipSlot.WEAPON_OFF_HAND in unserializedAppearance)
		{
			local offHand = unserializedAppearance[this.ItemEquipSlot.WEAPON_OFF_HAND];
			local offHandCallback = {
				so = this,
				function doWork( itemDef )
				{
					local unserializedAppearance = this.so.mAssembler.mRequestedItems;

					if (unserializedAppearance && unserializedAppearance[this.ItemEquipSlot.WEAPON_OFF_HAND] == itemDef.mID)
					{
						this.so.sheathWeapon(itemDef, this.ItemEquipSlot.WEAPON_OFF_HAND);
					}
				}

			};
			::_ItemDataManager.getItemDef(offHand, offHandCallback);
		}
	}

	function sheathWeapon( def, slot )
	{
		local attachmentPoint;
		local rightHandedWeapon = false;

		if (slot == this.ItemEquipSlot.WEAPON_MAIN_HAND)
		{
			rightHandedWeapon = true;
		}

		local weaponType = def.getWeaponType();
		local armorType = def.getArmorType();

		if (weaponType != this.WeaponType.NONE)
		{
			attachmentPoint = this.getShealthedAttachmentPointForWeaponType(weaponType, rightHandedWeapon);
		}
		else if (armorType == this.ArmorType.SHIELD)
		{
			attachmentPoint = "back_6";
		}

		local new_weapon = this.Item.Attachable(null, this.mWeapons[slot].mMeshName, attachmentPoint, this.mWeapons[slot].mColors, this.mWeapons[slot].mEffectName);
		this.addAttachment(new_weapon);
		new_weapon.assemble();
	}

	function getShealthedAttachmentPointForWeaponType( weaponType, rightHandedWeapon )
	{
		local attachmentPoint = "";

		if (rightHandedWeapon)
		{
			attachmentPoint = "left";
		}
		else
		{
			attachmentPoint = "right";
		}

		if (weaponType != this.WeaponType.NONE)
		{
			switch(weaponType)
			{
			case this.WeaponType.SMALL:
				return attachmentPoint += "_hip";
				break;

			case this.WeaponType.ONE_HAND:
				return attachmentPoint += "_hip";
				break;

			case this.WeaponType.TWO_HAND:
				attachmentPoint = "back_sheathe";
				return attachmentPoint;
				break;

			case this.WeaponType.POLE:
				attachmentPoint = "back_sheathe";
				return attachmentPoint;
				break;
			}
		}
	}

	function removedSheathedWeapons()
	{
		foreach( att in this.mAttachments )
		{
			if (!att.mAttachmentPointName)
			{
				continue;
			}

			if (att.mAttachmentPointName == "right_hip" || att.mAttachmentPointName == "left_hip" || att.mAttachmentPointName == "back_sheathe" || att.mAttachmentPointName == "back_6")
			{
				this.removeAttachment(att);
			}
		}
	}

	function setVisibleWeapon( set, animated, ... )
	{
		if (("switchingWeapons" in this.mAnimationHandler) && this.mAnimationHandler.switchingWeapons())
		{
			return false;
		}

		if (this.mVisibleWeaponSet == set)
		{
			return false;
		}

		this.mPreviousWeaponSet = this.mVisibleWeaponSet;
		this.mVisibleWeaponSet = set;
		local notify = true;
		local wait = false;
		local waiter;

		switch(vargc)
		{
		case 1:
			notify = vargv[0];
			break;

		case 2:
			wait = vargv[0];
			waiter = vargv[1];
			break;

		case 3:
			notify = vargv[0];
			wait = vargv[1];
			waiter = vargv[2];
			break;
		}

		if (animated && this.mAnimationHandler)
		{
			this.mAnimationHandler.switchWeapons(this.mPreviousWeaponSet, this.mVisibleWeaponSet, waiter);
		}
		else
		{
			this.hideWeapons();
			this.showWeapons();
		}

		if (notify && ::_Connection.isPlaying())
		{
			::_Connection.sendQuery("visWeapon", this, [
				set
			]);
		}

		return true;
	}

	function onVisibleWeaponUpdate( visible )
	{
		if (visible)
		{
			return;
		}

		if (this.mAssembler == null || !("mRequestedItems" in this.mAssembler) || this.mAssembler.mRequestedItems == null)
		{
			return;
		}

		this.updateSheathedWeapons();
	}

	function onEQAppearanceUpdate( value )
	{
		local visibleWeapon = this.getStat(this.Stat.VIS_WEAPON);

		if (visibleWeapon == null || visibleWeapon)
		{
			return;
		}

		this.updateSheathedWeapons();
	}

	function getVisibleWeapon()
	{
		return this.mVisibleWeaponSet;
	}

	function setWeapon( slot, att )
	{
		if (!this._weaponSlot(slot))
		{
			return;
		}

		this.mWeapons[slot] <- att;
	}

	function removeWeapons()
	{
		this.mWeapons = {};
	}

	function hasWeapon( slot )
	{
		return slot in this.mWeapons;
	}

	function hasWeaponSet( set )
	{
		foreach( s, w in this.mWeapons )
		{
			if (this.inWeaponSet(set, s))
			{
				return true;
			}
		}

		return false;
	}

	function updateGrip()
	{
		if ("setGrip" in this.mAnimationHandler)
		{
			if (this.hasItemInHand())
			{
				this.mAnimationHandler.setGrip(true);
			}
			else
			{
				this.mAnimationHandler.setGrip(false);
			}
		}
	}

	function hideHandAttachments()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				if (attachment.getEntity())
				{
					attachment.getEntity().setVisible(false);
				}

				local particleSystem = attachment.getParticleSystem();

				if (particleSystem)
				{
					particleSystem.setVisible(false);
				}
			}
		}
	}

	function unHideHandAttachments()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity)
				{
					entity.setVisible(true);
				}

				local particleSystem = attachment.getParticleSystem();

				if (particleSystem)
				{
					particleSystem.setVisible(true);
				}
			}
		}
	}

	function hasItemInHand()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return true;
				}
			}
		}

		return false;
	}

	function getHandAttachments( ... )
	{
		local visibleOnly = vargc > 0 ? vargv[0] : true;
		local results = [];

		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName || "left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && (!visibleOnly || entity.isVisible()))
				{
					results.append(attachment);
				}
			}
		}

		return results;
	}

	function getLeftHandItem()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("left_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return attachment;
				}
			}
		}

		return null;
	}

	function getRightHandItem()
	{
		foreach( attachment in this.mAttachments )
		{
			if ("right_hand" == attachment.mAttachmentPointName)
			{
				local entity = attachment.getEntity();

				if (entity && entity.isVisible())
				{
					return attachment;
				}
			}
		}

		return null;
	}

	function removeAllAttachments()
	{
		local da = [];

		foreach( io in this.mAttachments )
		{
			this.broadcastMessage("onAttachmentRemoved", io);
			io.destroy();
		}

		this.mAttachments.clear();
	}

	function removeNoneItemAttachments()
	{
		local da = [];

		foreach( key, io in this.mAttachments )
		{
			if (io.getObjectClass() == "Attachment")
			{
				da.append(io.getID());
				io.disassemble();
				io.destroy();
			}
		}

		foreach( x in da )
		{
			delete this.mAttachments[x];
		}
	}

	function getAttachments()
	{
		return this.mAttachments;
	}

	function setSceneryName( name )
	{
		this.mProperties.NAME = name;
	}

	function getSceneryName()
	{
		if ("NAME" in this.mProperties)
		{
			return this.mProperties.NAME;
		}

		return "Unnamed (" + this.getName() + ")";
	}

	function getName()
	{
		local name = this.getStat(this.Stat.DISPLAY_NAME);
		return name == null ? this.getNodeName() : name;
	}

	function onDeath()
	{
		if (this.isCreature())
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onDeath();
			}
			else
			{
				this.mNeedToRunDeathAnim = true;
			}
		}
	}

	function onRes()
	{
		if (this.isCreature())
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onRes();
			}
		}
	}

	function getNodeName()
	{
		return this.mNode.getName();
	}

	function getNode()
	{
		return this.mNode;
	}

	function getAssemblingNode()
	{
		return this.mAssemblingNode;
	}

	function getID()
	{
		return this.mID;
	}

	function getLOD()
	{
		return this.mLODLevel;
	}

	function markAsGarbage( ... )
	{
		if (vargc > 0 && !vargv[0])
		{
			if (this.mGarbagificationTime != null)
			{
				this.mGarbagificationTime = null;
				this._sceneObjectManager._queueDestroy(this, false);
			}
		}
		else if (this.mGarbagificationTime == null)
		{
			this.mGarbagificationTime = this.System.currentTimeMillis();
			this._sceneObjectManager._queueDestroy(this, true);
		}
	}

	function setAnimationHandler( pAnimationHandler )
	{
		if (this.mAnimationHandler == pAnimationHandler)
		{
			return;
		}

		if (this.mAnimationHandler)
		{
			this.mAnimationState = this.mAnimationHandler.getAnimationState();
			this.mAnimationHandler.destroy();
		}

		this.mAnimationHandler = pAnimationHandler;

		if (this.mAnimationHandler && this.mAnimationState)
		{
			this.mAnimationHandler.setAnimationState(this.mAnimationState);
			this.mAnimationState = null;
		}

		if (this.mNeedToRunDeathAnim)
		{
			if (this.mAnimationHandler)
			{
				this.mAnimationHandler.onDeath();
			}

			this.mNeedToRunDeathAnim = false;
		}

		if (this.mAnimationHandler && this.mSpeed > 0.0)
		{
			this.mAnimationHandler.onMove(this.mSpeed);
		}
	}

	function getAnimationHandler()
	{
		return this.mAnimationHandler;
	}

	function setController( pControllerName )
	{
		local data;

		if (this.mController)
		{
			data = this.mController.getData();
			this.mController.destroy();
		}

		if (pControllerName in this.Controller)
		{
			this.mController = this.Controller[pControllerName](this);
		}
		else
		{
			this.mController = null;
		}

		if (data && this.mController)
		{
			this.mController.setData(data);
		}
	}

	function getController()
	{
		return this.mController;
	}

	function setEffectsHandler( effectsHandler )
	{
		this.mEffectsHandler = effectsHandler;
	}

	function getEffectsHandler()
	{
		return this.mEffectsHandler;
	}

	function serverVelosityUpdate( ... )
	{
		if (this == ::_avatar)
		{
			local force = vargc > 0 ? vargv[0] : false;

			if (force)
			{
				this._Connection.sendVelocityUpdate(this.getPosition(), this.mHeading, this.mRotation, this.mSpeed);
			}
			else
			{
				this.mVelocityUpdatePending = true;
			}
		}
	}

	function sendPendingVelocityUpdate()
	{
		if (this.mVelocityUpdatePending == true)
		{
			this._Connection.sendVelocityUpdate(this.getPosition(), this.mHeading, this.mRotation, this.mSpeed);
			this.mVelocityUpdatePending = false;
		}

		if (::_avatar == this)
		{
			this.mVelocityUpdateSchedule = ::_eventScheduler.fireIn(0.25, this, "sendPendingVelocityUpdate");
		}
		else
		{
			this.mVelocityUpdateSchedule = null;
		}
	}

	function setVerticalSpeed( speed )
	{
		this.mVerticalSpeed = speed;
	}

	function collideAndMoveSweep( pos, dir, mask )
	{
		local stepHeight = 1.9;
		local box = this.Vector3(2.5, 7.0, 2.5);
		return this.Util.collideAndSlide(box, stepHeight, pos, dir, mask);
	}

	function _setAssembling( which )
	{
		this.mAssembling = which;
	}

	function getAssembling()
	{
		return this.mAssembling;
	}

	function _notifyUpdateReceived()
	{
	}

	function isCharacterOrientableOnTerrain( heading, normal )
	{
		if (this.mFloorAlignMode == this.FloorAlignMode.ALWAYS || (heading.dot(normal) > this.UP_DOWN_SLOPE_ANGLE || heading.dot(normal) < -this.UP_DOWN_SLOPE_ANGLE) && this.mFloorAlignMode == this.FloorAlignMode.WHILE_ASCENDING_DESCENDING)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	function _scaleNameBoard()
	{
		if (this.mNameBoard == null)
		{
			return;
		}

		local distance = this._getDistanceFromCamera();
		local h = this.Math.lerp(this.gNameNearHeight, this.gNameFarHeight, distance / this.gNameFarClip);

		if (this.mNamePlateScale)
		{
			this.mNameBoard.setLineHeight(h * this.mNamePlateScale);
		}
		else
		{
			this.mNameBoard.setLineHeight(h);
		}
	}

	function isAlwaysVisible()
	{
		if (this.mAlwaysVisible != null)
		{
			return this.mAlwaysVisible;
		}

		local assembler = this.getAssembler();

		if (assembler)
		{
			local config = assembler.getConfig();

			if (config != null)
			{
				local tmp = config.find("[\"c\"]=\"");

				if (tmp != null)
				{
					local end = config.find("\"", tmp + 7);
					local type = config.slice(tmp + 7, end);

					if (type && type in ::AlwaysVisible)
					{
						this.mAlwaysVisible = ::AlwaysVisible[type];
						return this.mAlwaysVisible;
					}
				}
			}
		}

		return false;
	}
	
	/*
	 * Takes a table that defines a sequence of transformations that should be
	 * applied to this object over a period of time.
	 *
	 * @param sequence table of transformations
	 */
	function setTransformationSequence(sequence)
	{
		if(mCurrentTransformationSequence != null)
			stopTransformationSequence();

		// Store the state at the state of the transformation			
		mCurrentTransformationSequence = sequence;
		mCurrentTransformationSequence.startPosition <- Vector3(getPosition().x, getPosition().y, getPosition().z);;
		mCurrentTransformationSequence.startScale <- Vector3(getScale().x, getScale().y, getScale().z);
		local q = getOrientation();
		mCurrentTransformationSequence.startRotation <- Quaternion(q.w, q.x, q.y, q.z);
		mCurrentTransformationSequence.startOpacity <- getOpacity();
		
		::_sceneObjectManager.mTransforming.append(this);
		
		return nextTransformationFrame();
	}
	
	/*
	 * Moves to next frame in transformation sequence
	 */
	function nextTransformationFrame() {
		/* Reset / remove any state from the current frame (if any) so
		 * if the frame is repeated it will process from the start
		 */
		if(mCurrentTransformationFrame != null) {
			foreach(transform in mCurrentTransformationFrame.transforms) {
				local tType = "type" in transform ? transform.type : "unknown"; 
				if(tType == "scale" || tType == "rotate" || tType == "translate" || tType == "opacity") {
					if("start" in transform)
						delete transform.start
				}
				else if(tType == "playAudio") {
					if("played" in transform) 
						delete transform.played;
				}
				else if(tType == "stopAudio") {
					if("stopped" in transform) 
						delete transform.stopped;
				}
			}
		}  
	
		
		if(!("index" in mCurrentTransformationSequence)) {		
			// Starting sequence
			mCurrentTransformationSequence.index <- 0;
		}
		else
			mCurrentTransformationSequence.index++;
			
		print("ICE! Index is now " + mCurrentTransformationSequence.index); 
			
		if(!("frames" in mCurrentTransformationSequence)) {
			print("ICE! No frames in sequence");
			return false;
		}
		
		local frames = mCurrentTransformationSequence.frames;
		if(mCurrentTransformationSequence.index >= frames.len()) {
			print("ICE! At end of sequence at " + mCurrentTransformationSequence.index);
			mCurrentTransformationFrame = null;
			return false;
		}
		
		mCurrentTransformationFrame = frames[mCurrentTransformationSequence.index];
		
		if(!("transforms" in mCurrentTransformationFrame)) {
			print("ICE! No transforms in frame " + mCurrentTransformationSequence.index);
			return nextTransformationFrame();
		}
		
		mCurrentTransformationFrame.startScale <- Vector3(getScale().x, getScale().y, getScale().z);	
		mCurrentTransformationFrame.started <- System.currentTimeMillis();
		mCurrentTransformationFrame.durationMs <- ( ( ("duration" in mCurrentTransformationFrame) ? mCurrentTransformationFrame.duration.tofloat() : 1.0 ) * 1000).tointeger();  
		mCurrentTransformationFrame.end <- mCurrentTransformationFrame.started + mCurrentTransformationFrame.durationMs;
			
		print("ICE! Frame " + mCurrentTransformationSequence.index + " will run from " + mCurrentTransformationFrame.started + " to " + mCurrentTransformationFrame.end);
		
		print("ICE! Scenery: " + mIsScenery + " Should render: " + shouldRender() + " mForceUpdate = " + mForceUpdate);
			
		// Setup the interpolators for all of the transforms			
		foreach(transform in mCurrentTransformationFrame.transforms) {
			if("interpolation" in transform)
				transform.interpolator <- Interpolator.interpolate(transform.interpolation);
			else 
				transform.interpolator <- Interpolator.INTERPOLATION_LINEAR;
		}
			
			
		return true;	
	}
	
	/*
	 * Stops a transformation sequence, return the object to the state it was in before
	 * the transformations were started.
	 */
	function stopTransformationSequence() 
	{
		if(mCurrentTransformationSequence != null) {
						
			if(!mGone) {
				this.mNode.setScale(mCurrentTransformationSequence.startScale);
				this.mNode.setPosition(mCurrentTransformationSequence.startPosition);
				this.mNode.setOrientation(mCurrentTransformationSequence.startRotation);
				setOpacity(mCurrentTransformationSequence.startOpacity);
			}
			
			mCurrentTransformationSequence = null;
			mCurrentTransformationFrame = null;
			
			local index = Util.indexOf(::_sceneObjectManager.mTransforming, this);
			if (index != null)
				::_sceneObjectManager.mTransforming.remove(index);
		}
	}
	
	/*
	 * Update any current active transformation sequences.
	 */
	function updateTransformationSequences()
	{
		if (mGone || mCurrentTransformationFrame == null)
			return;
			
		// Work how out how along the transformation we are
		local now = System.currentTimeMillis();
		local progress = now - mCurrentTransformationFrame.started;
		local pc = mCurrentTransformationFrame.durationMs == 0 ? 1.0 : progress.tofloat() / mCurrentTransformationFrame.durationMs.tofloat();
		
		if(pc > 1)
			pc = 1.0;
			
		/* Apply each of the transformations (using the state of the object before the
		 * frame started as a base) and interpolate using the current progress
		 */
		local tType = null;  
		foreach(transform in mCurrentTransformationFrame.transforms) {
			tType = "type" in transform ? transform.type : "unknown"; 
			if(tType == "opacity") {
				// Scale				
				if(!("start" in transform))
					transform.start <- getOpacity();
				local amtV = interpolatedTransformation(transform, pc);
				Util.setNodeOpacity(mNode, amtV.x);				
			}
			else if(tType == "scale") {
				// Scale				
				if(!("start" in transform))
					transform.start <- Vector3(getScale().x, getScale().y, getScale().z);
				local amtV = interpolatedTransformation(transform, pc);
				mNode.setScale(amtV);
			}
			else if(tType == "rotate") {
				// Rotate
				if(!("start" in transform)) {
					local q = mNode.getOrientation();
					transform.start <- Quaternion(q.w, q.x, q.y, q.z);
				}
				
				local amtV = interpolatedTransformation(transform, pc);
				
				// Need amounts in radians
				amtV.x = Math.deg2rad(amtV.x);
				amtV.y = Math.deg2rad(amtV.y);
				amtV.z = Math.deg2rad(amtV.z);
				
				local qx = this.Quaternion(amtV.x, Vector3(1.0, 0.0, 0.0));
				local qy = this.Quaternion(amtV.y, Vector3(0.0, 1.0, 0.0));
				local qz = this.Quaternion(amtV.z, Vector3(0.0, 0.0, 1.0));
				local quat = qz * qy * qx;
				
				mNode.setOrientation(quat);
			}
			else if(tType == "translate") {
				// Translate
				if(!("start" in transform))
					transform.start <- Vector3(getPosition().x, getPosition().y, getPosition().z);
				mNode.setPosition(interpolatedTransformation(transform, pc) + transform.start);
			}
			else if(tType == "goto") {
				// Jump to a different frame
				if(!("to" in transform)) {
					transform.to <- 0;
				}
				
				mCurrentTransformationSequence.index = transform.to.tointeger() - 1;
				nextTransformationFrame();
				return;
				
			}
			else if(tType == "playAudio") {
				// Play an audio asset
				if("played" in transform) 
					continue;
					
				if(!("asset" in transform)) {
					print("ICE! No audio asset in transform");
					continue; 
				}
				
				if(!("delay" in transform) || progress >= ( transform.delay.tofloat() * 1000)) {
					Audio.playMusic(transform.asset, ( "channel" in transform ) ? transform.channel : Audio.DEFAULT_CHANNEL);
					transform.played <- true;
				}
			}
			else if(tType == "stopAudio") {
				// Play an audio asset
				if("stopped" in transform) 
					continue;
					
				if(!("delay" in transform) || progress >= ( transform.delay.tofloat() * 1000)) {
					if("fadeSpeed" in transform) 
						Audio.stopMusic(( "channel" in transform ) ? transform.channel : Audio.DEFAULT_CHANNEL, transform.fadeSpeed);
					else 
						Audio.stopMusic(( "channel" in transform ) ? transform.channel : Audio.DEFAULT_CHANNEL);
					transform.stopped <- true;
				}
			}
		}
		
		if(pc >= 1) {
			if(!nextTransformationFrame()) {
				// Stop the updates
				local index = Util.indexOf(::_sceneObjectManager.mTransforming, this);
				if (index != null)
					::_sceneObjectManager.mTransforming.remove(index);
			}
		}
	}
	
	function interpolatedTransformation(transform, pc) {
	
	
		local toV = arrayToVector3(transform.to);
		local fromV = ("from" in transform) ? arrayToVector3(transform.from) : ( "start" in transform ? transform.start : Vector3(0.0,0.0,0.0) );
		/*return Vector3(fromV.x + ( ( toV.x - fromV.x ) * pc ),
			fromV.y + ( ( toV.y - fromV.y ) * pc ),
			fromV.z + ( ( toV.z - fromV.z ) * pc ));*/
			
		//function apply(start, end, a) {
		
		return Vector3(transform.interpolator.apply(fromV.x, toV.x, pc ),
						transform.interpolator.apply(fromV.y, toV.y, pc ),
						transform.interpolator.apply(fromV.z, toV.z, pc ));
	}
	
	function arrayToVector3(arr) {
		if(typeof arr != "array")
			return Vector3(arr.tofloat(), arr.tofloat(), arr.tofloat());
		else
			return Vector3(arr[0].tofloat(),arr[1].tofloat(),arr[2].tofloat());
	}

	function updateFade()
	{
		if (this.mGone)
		{
			return;
		}

		if (this.isAlwaysVisible())
		{
			this.mCurrentFadeLevel = 1.0;
			this.mFadeTarget = 1.0;
			this.setOpacity(1.0);
			return;
		}

		if (this.mFadeEnabled == false)
		{
			this.mCurrentFadeLevel = this.mDesiredFadeLevel;
			this.mFadeTarget = this.mDesiredFadeLevel;
			this.setOpacity(this.mDesiredFadeLevel);
			return;
		}

		local distance = this._getDistanceFromCamera();
		local fadeStart = this.gCreatureVisibleRange - this.gLodBlockSize;
		local fadeEnd = this.gCreatureVisibleRange;
		local fadeSpan = fadeEnd - fadeStart;
		local fadeLevel = 0.0;

		if (distance >= fadeEnd)
		{
			fadeLevel = 0.0;
		}
		else if (distance >= fadeStart)
		{
			fadeLevel = 1.0 - (distance - fadeStart) / fadeSpan;
			this.Math.clamp(fadeLevel, 0.0, 1.0);
		}
		else
		{
			fadeLevel = 1.0;
		}

		if (this.isInvisible())
		{
			fadeLevel *= 0.30000001;
		}

		this.mFadeTarget = fadeLevel;
	}

	function isInvisible()
	{
		return this.hasStatusEffect(this.StatusEffects.INVISIBLE) || this.hasStatusEffect(this.StatusEffects.WALK_IN_SHADOWS) || this.hasStatusEffect(this.StatusEffects.GM_INVISIBLE);
	}

	function _getDistanceFromCamera()
	{
		local pos = this.getPosition();
		local cpos = this._camera.getParentSceneNode().getPosition();
		return pos.distance(cpos);
	}

	function _interpolateFade()
	{
		local delta = this._deltat / 1000.0 * 2.0;
		local opacity = this.mOpacity;

		if (this.mFadeTarget < opacity)
		{
			opacity = this.Math.max(this.mFadeTarget, opacity - delta);
		}
		else if (this.mFadeTarget > opacity)
		{
			opacity = this.Math.min(this.mFadeTarget, opacity + delta);
		}

		this.setOpacity(opacity);
	}

	function _updateLOD()
	{
		local distance = this._getDistanceFromCamera();
		this.mNameInRange = distance <= this.gNameFarClip;

		if (!this.mForceUpdate)
		{
			this.mLODCounter++;

			if (this.mLODCounter < this.LOD_FRAME_COHERENCE)
			{
				return;
			}
		}

		this.mLODCounter = 0;
		local newLod = (distance / this.gLodBlockSize).tointeger();

		if (newLod > 5)
		{
			newLod = 5;
		}

		local lodStart = newLod * this.gLodBlockSize;
		local lodEnd = (newLod + 1) * this.gLodBlockSize;
		local mPercentToNextLOD = (lodEnd - distance) / this.gLodBlockSize;

		if (newLod != this.mLODLevel)
		{
			local oldLod = this.mLODLevel;
			this.mLODLevel = newLod;
			this._sceneObjectManager._updateLodBucket(this, oldLod, newLod);

			if (newLod <= 3)
			{
				if (!this.mAssembled && this.mType != "")
				{
					this.reassemble();
				}
			}
			else if (newLod >= 5 && !this.isAlwaysVisible())
			{
				if (this.mAssembled)
				{
					this.disassemble();
				}
			}
		}
	}

	LOD_FRAME_COHERENCE = 32;
	FRAME_COHERENCE = 32;
	ORIENT_COHERENCE = 15;
	mFirstUpdate = false;
	mOrientCounter = 0;
	mFrameCounter = 0;
	mForceUpdate = true;
	mLODCounter = 0;
	mVisible = false;
	function _updateVisibility()
	{
		if (this == ::_avatar)
		{
			this.mVisible = true;
			return;
		}

		local pos = this.getPosition();

		if (pos != null)
		{
			local forward = ::_camera.getParentNode()._getDerivedOrientation().rotate(this.Vector3(0.0, 0.0, -1.0));
			forward.normalize();
			local dir = pos - ::_camera.getParentNode().getWorldPosition();
			dir.normalize();
			local vis = forward.dot(dir) > 0.60000002;

			if (vis != this.mVisible)
			{
				this.forceUpdate();
			}

			this.mVisible = vis;
		}
	}

	function _basicUpdate()
	{
		local speed = this.gDefaultCreatureSpeed * (this.mSpeed / 100.0);

		if (speed > 0.0099999998)
		{
			local dir = ::Math.ConvertRadToVector(this.mHeading);
			dir = dir * speed * ::_deltat / 1000.0;
			local newPos = this.getPosition() + dir;

			if (this.mVisible && this.mLODLevel < 4)
			{
				local floor = this.Util.getFloorHeightAt(newPos, 20.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true, this.getNode());

				if (floor)
				{
					newPos.y = floor.pos.y;
				}
			}

			this.setPosition(newPos);
		}
	}

	function _updateFloorAlignment()
	{
		if (this.mFloorAlignMode != this.FloorAlignMode.WHILE_ASCENDING_DESCENDING)
		{
			return;
		}

		local pos = this.getPosition();
		local terrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());

		if (terrain == null)
		{
			return;
		}

		local heading = ::Math.ConvertRadToVector(this.mHeading);

		if (this.mInterpolateFramesLeft <= 0 && !this.isCharacterOrientableOnTerrain(heading, terrain.normal))
		{
			this.mStartNormal = this.mLastNormal;
			this.mEndNormal = terrain.normal;
			this.mLastNormal = this.mEndNormal;
			this.mInterpolateFramesLeft = this.FRAMES_TO_INTERPOLATE;
			this.mCurrentlyOriented = false;
		}
	}

	function _interpolateFloorAlignment()
	{
		if (this.mInterpolateFramesLeft >= 0)
		{
			local yAxis = this.Math.slerpVectors(this.mStartNormal, this.mEndNormal, 1.0 - this.mInterpolateFramesLeft.tofloat() / this.FRAMES_TO_INTERPOLATE.tofloat());
			local xAxis = this.Vector3(1.0, 0.0, 0.0);
			local zAxis = xAxis.cross(yAxis);
			zAxis.normalize();
			xAxis = yAxis.cross(zAxis);
			xAxis.normalize();
			local coordSpace = this.Quaternion(xAxis, yAxis, zAxis);
			local entity = this.getEntity();

			if (entity)
			{
				this.mFloorAlignOrientation = coordSpace;
				entity.getParentNode().setOrientation(coordSpace);
				entity.getParentNode().rotate(this.Vector3(0, 1, 0), this.mRotation);
			}

			this.mInterpolateFramesLeft--;
		}
	}

	function setTimeoutEnabled( which )
	{
		this.mTimeoutEnabled = which;
	}

	function _checkTimeout()
	{
		if (this != ::_avatar && this.mTimeoutEnabled && !this.mGone)
		{
			this.mTimeSinceLastUpdate += this._deltat;

			if (this.mTimeSinceLastUpdate > this.UPDATE_TIMEOUT)
			{
				this.log.debug("Removing creature " + this.getName() + " because it has expired!");
				this.gone();
			}
		}
	}

	function onEnterFrame()
	{
		this._checkTimeout();
		
		/*if(mCurrentTransformationFrame != null) {
			print("ICE! UPDATE Asm: " + this.mAssembled + " ambling: " + this.mAssemblingNode + " Scenery: " + mIsScenery + " Should render: " + shouldRender() + " mForceUpdate = " + mForceUpdate + " gone: " + mGone);
		}*/

		if (!this.mAssembled && !this.mAssemblingNode)
		{
			return;
		}

		this._updateVisibility();
		this._updateLOD();
		this._positionName();

		if ((this.mIsScenery || !this.shouldRender()) && !this.mForceUpdate)
		{
			return;
		}

		this._interpolateFade();
		this._interpolateFloorAlignment();

		if (this.mVisible)
		{
			this._scaleNameBoard();
		}

		if (this.mCorking && this.mCorkTimeout > 0)
		{
			this.mCorkTimeout -= this._deltat;

			if (this.mCorkTimeout <= 0)
			{
				this.uncork();
			}
		}

		if (this.mController)
		{
			if (::_avatar == this)
			{
				this.mController.setEnabled(!this.hasStatusEffect(this.StatusEffects.ROOT) && !this.hasStatusEffect(this.StatusEffects.DEAD));
			}

			this.mController.onEnterFrame();
		}

		if (this.mAnimationHandler)
		{
			this.mAnimationHandler.onEnterFrame();
		}
		
		//this.updateTransformationSequences();

		if (!this.isPlayer() && !this.mForceUpdate && this.mDistanceToFloor < 0.25)
		{
			this.mFrameCounter++;

			if (this.mFrameCounter < this.FRAME_COHERENCE)
			{
				this._basicUpdate();
				return;
			}

			this.mFrameCounter = 0;
		}

		this._updateFloorAlignment();
		this._checkSounds();
		this.updateFade();

		if (this.mGone && this.mOpacity <= 0.0)
		{
			this.destroy();
			return;
		}

		local pos = this.getPosition();
		local oldPos = this.Vector3(pos.x, pos.y, pos.z);

		if (!this.mForceUpdate && this.mSpeed < 0.001 && this.mDistanceToFloor < 0.001 && this.mSlopeSlideInertia == null)
		{
			return;
		}

		local dir = ::Math.ConvertRadToVector(this.mHeading);
		local baseSpeed = this.gDefaultCreatureSpeed;
		local speed = baseSpeed * (this.mSpeed / 100.0);
		dir = dir * speed * ::_deltat / 1000.0;
		local sizeY = this.BASE_AVATAR_HEIGHT * this.getScale().y;

		if (sizeY <= 0.0)
		{
			sizeY = 1.0;
		}

		local checkSwimming = !this.mSwimming;

		if (this.mSwimming)
		{
			local sterrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());

			if (sterrain && (this.mWaterElevation - sterrain.pos.y) / sizeY < 0.30000001)
			{
				checkSwimming = false;
				this.mSwimming = false;
				this.mController.onStopSwimming();
			}
			else
			{
				local avatarPosition = this.getPosition();
				local startingPoint = this.Vector3(avatarPosition.x, this.mWaterElevation, avatarPosition.z);
				local box = this.Vector3(2.0, 4.0, 2.0);
				local groundTestDir = this.Vector3(0.0, -1.5, 0.0);
				local groundTest = this._scene.sweepBox(box, startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);

				if (sterrain == null || groundTest.distance < 1)
				{
					checkSwimming = false;
					this.mSwimming = false;
					this.mController.onStopSwimming();
				}
				else
				{
					pos.y = this.mWaterElevation;
					pos = this.collideAndMoveSweep(pos, dir, this.QueryFlags.BLOCKING).pos;
					this.setPosition(pos);

					if (this.mAnimationHandler)
					{
						this.mAnimationHandler.forceAnimUpdate();
					}

					return;
				}
			}
		}

		pos = this.collideAndMoveSweep(pos, dir, this.QueryFlags.BLOCKING).pos;
		local deltaY = this.mVerticalSpeed * ::_deltat / 1000.0;

		if (this.mDistanceToFloor > 0.001)
		{
			local res = this.collideAndMoveSweep(pos, this.Vector3(0.0, deltaY, 0.0), this.QueryFlags.BLOCKING);
			pos = res.pos;

			if (this.mVerticalSpeed > 0.0 && res.hit == true)
			{
				this.mVerticalSpeed = 0.0;
			}

			this.mVerticalSpeed -= this.mDownwardAcceleration * ::_deltat / 1000.0;
		}

		local terrain = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.KEEP_ON_FLOOR, true, this.getNode());
		this._updateFloorAlignment();
		local StepValue = 6.0;
		local avatarPosition = this.getPosition();
		local startingPoint = this.Vector3(avatarPosition.x, avatarPosition.y + StepValue, avatarPosition.z);
		local box = this.Vector3(2.0, 4.0, 2.0);
		local movementTestDir = this.Vector3(0.0, deltaY, 0.0);
		local MovementTest = this._scene.sweepBox(box, startingPoint, startingPoint + movementTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
		local finalMovementPos = startingPoint + movementTestDir * MovementTest.distance;
		local groundTestDir = this.Vector3(0.0, -5000.0, 0.0);
		local groundTest = this._scene.sweepBox(box, startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
		local finalGroundPos = startingPoint + groundTestDir * groundTest.distance;
		local wasVerticalSpeed = this.mVerticalSpeed;
		finalGroundPos.y -= StepValue;
		finalMovementPos.y -= StepValue;

		if (groundTest.distance < 1.0)
		{
			if (this.mDistanceToFloor > 0.001)
			{
				local groundTest2 = this._scene.sweepBox(this.Vector3(3.0, 4.0, 3.0), startingPoint, startingPoint + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);
				local normalAngle = groundTest2.normal.dot(this.Vector3(0, 1, 0));

				if (normalAngle < 0.2)
				{
					finalGroundPos += groundTest.normal * 3.5;
				}
				else
				{
					finalGroundPos += groundTest.normal * 1;
				}
			}
			else
			{
			}
		}

		local floor = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true, this.getNode());

		if (terrain == null)
		{
			terrain = floor;
		}
		
		local hitGround = false;

		if (floor != null && terrain != null)
		{
			if (!this.mCurrentlyJumping && this.abs(pos.y - finalGroundPos.y) < 6.5 && (this.abs(dir.x) > 0.5 || this.abs(dir.z) > 0.5))
			{
				this.setDistanceToFloor(0.0, floor.normal);
				this.mVerticalSpeed = 0.0;
				//print("ICE! Hit ground 1\n");
				//hitGround = true;
				pos.y = finalGroundPos.y;
			}

			if (groundTest.distance < 1 && finalGroundPos.y > terrain.pos.y)
			{
				if (this.isPropCreature() && groundTest.distance <= 0)
				{
				}
				else if (this.mObjectBelowAvatar == false && this.mVerticalSpeed <= 0.1 && this.mVerticalSpeed >= -0.1)
				{
					this.setDistanceToFloor(0.0, floor.normal);
					this.mVerticalSpeed = 0.0;
					this.mCurrentlyJumping = false;
					pos.y = finalGroundPos.y;
					//print("ICE! Hit ground 2\n");
					//hitGround = true;
				}
				else if (finalMovementPos.y - finalGroundPos.y <= 1.0)
				{
					pos.y = finalGroundPos.y;
					this.mVerticalSpeed = 0.0;
					this.setDistanceToFloor(0.0, terrain.normal);
					this.mCurrentlyJumping = false;
					print("ICE! Hit ground 3\n");
					hitGround = true;
				}
				else
				{
					this.setDistanceToFloor(finalMovementPos.y - finalGroundPos.y - 1, floor.normal);
				}

				this.mObjectBelowAvatar = true;
			}
			else
			{
				if (::_avatar == this)
				{
				}

				this.mObjectBelowAvatar = false;

				if (pos.y <= terrain.pos.y)
				{
					this.mVerticalSpeed = 0.0;
					pos = terrain.pos;
					this.setDistanceToFloor(0.0, terrain.normal);
					this.mCurrentlyJumping = false;
					//print("ICE! Hit ground 4\n");
					//hitGround = true;
				}
				else
				{
					this.setDistanceToFloor(-terrain.t, terrain.normal);
				}
			}

			if (pos.y < floor.pos.y)
			{
				pos.y = floor.pos.y;
				this.mObjectBelowAvatar = false;
			}

			local slideableTerrainHeight = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR, true, this.getNode());
			
			local sentUpdate = false;

			if (::_avatar == this)
			{
				if (slideableTerrainHeight != null && this.abs(floor.pos.y - slideableTerrainHeight.pos.y) < 0.050000001 && slideableTerrainHeight.normal.dot(this.Vector3(0, 1, 0)) < this.gMaxSlope && this.mDistanceToFloor <= 0.001)
				{
					local norm = slideableTerrainHeight.normal;
					norm.y = 0;
					norm.normalize();

					if (this.mSlopeSlideInertia != null)
					{
						this.mSlopeSlideInertia.x += norm.x * baseSpeed * 4 * ::_deltat / 1000.0;
						this.mSlopeSlideInertia.z += norm.z * baseSpeed * 4 * ::_deltat / 1000.0;
					}
					else
					{
						this.mSlopeSlideInertia = {
							x = norm.x * baseSpeed * 4 * ::_deltat / 1000.0,
							z = norm.z * baseSpeed * 4 * ::_deltat / 1000.0
						};
					}
				}
				else if (this.mSlopeSlideInertia != null)
				{
					this.mSlopeSlideInertia.x *= 0.60000002;
					this.mSlopeSlideInertia.z *= 0.60000002;

					if (this.fabs(this.mSlopeSlideInertia.x) < 0.050000001 && this.fabs(this.mSlopeSlideInertia.z) < 0.050000001)
					{
						this.mSlopeSlideInertia = null;
						this.serverVelosityUpdate();
						sentUpdate = true;
					}
				}
			}

			if (this.mSlopeSlideInertia != null && this.mDistanceToFloor <= 0.001)
			{
				if (::_avatar == this)
				{
				}

				this.serverVelosityUpdate();
				sentUpdate = true;
				local slidePosition = this.Vector3(pos.x, pos.y, pos.z);
				slidePosition.x += this.mSlopeSlideInertia.x * ::_deltat / 1000.0;
				slidePosition.z += this.mSlopeSlideInertia.z * ::_deltat / 1000.0;
				slidePosition.y += StepValue;
				local startingPos = this.Vector3(pos.x, pos.y, pos.z);
				local collision = this._scene.sweepBox(box, pos, slidePosition, this.QueryFlags.BLOCKING, false);

				if (collision.distance < 1.0)
				{
					pos = oldPos;
					pos.y += 5.0;
					local slideVector = slidePosition - pos;
					slidePosition = this.collideAndMoveSweep(pos, slideVector, this.QueryFlags.BLOCKING | this.QueryFlags.FLOOR).pos;
				}

				pos.x = slidePosition.x;
				pos.z = slidePosition.z;
				pos.y = slidePosition.y;
				local terrainHeight = this.Util.getFloorHeightAt(pos, 10.0, this.QueryFlags.FLOOR, false, this.getNode());
				local height = this._scene.sweepBox(box, pos, pos + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);

				if (height.distance < 1.0)
				{
					local finalGroundPos = pos + groundTestDir * height.distance;
					pos.y = finalGroundPos.y;
				}

				pos.y -= StepValue;

				if (terrainHeight != null)
				{
					if (pos.y < terrainHeight || height.distance >= 1.0)
					{
						pos.y = terrainHeight;
					}
				}

				checkSwimming = true;
			}
			
			if (::_avatar == this && !sentUpdate && hitGround) {
				print("ICE! Hit ground!!!!\n");
			}
		}

		if (checkSwimming)
		{
			local swimming = false;
			this.mWaterElevation = this.Util.getWaterHeightAt(pos);

			if (this.mWaterElevation > pos.y)
			{
				if (sizeY > 0.0)
				{
					local uwp = (this.mWaterElevation - pos.y) / sizeY;

					if (uwp > 0.5)
					{
						this.mSwimming = true;
						this.mController.onStartSwimming(this.mWaterElevation);
						pos.y = this.mWaterElevation - sizeY * 0.60000002;
					}
				}
			}
		}

		local collision = ::_scene.sweepSphere(0.5, avatarPosition, pos);

		if (this.mSlopeSlideInertia != null && collision.distance < 1.0)
		{
			collision.normal.y = 0;
			pos = avatarPosition + collision.normal;
		}

		if (::_avatar == this)
		{
		}

		this.setPosition(pos);
		::_LightingManager.updateVisibility(this);
		this.mForceUpdate = false;
	}

	function fireUpdate()
	{
		if (this.mController)
		{
			this.mController.onUpdate();
		}

		this.broadcastMessage("onUpdate");
	}

	function addFloatie( text, ... )
	{
		local floatieType = this.IGIS.FLOATIE_DEFAULT;

		if (vargc > 0)
		{
			floatieType = vargv[0];
		}

		if (this.mCorking)
		{
			this.mCorkedFloaties.append({
				message = text,
				type = floatieType
			});
			return;
		}

		::IGIS.floatie(text, floatieType, this);
	}

	function addCombatMessage( msgChannel, combatMessage )
	{
		if (this.mCorking)
		{
			this.mCorkedChatMessage.append({
				message = combatMessage,
				channel = msgChannel
			});
			return;
		}

		::_ChatManager.addMessage(msgChannel, combatMessage);
	}

	function onJump()
	{
		if (this.mController)
		{
			this.mController.onJump();
			this.setJumping(true);
		}
	}

	function setJumping( value )
	{
		this.mCurrentlyJumping = true;
	}

	function startAutoAttack( ranged, ... )
	{
		local force = false;

		if (vargc > 0)
		{
			force = vargv[0];
		}

		local target = this.getTargetObject();

		if (target == null)
		{
			this.IGIS.error("Nothing to attack.");
			return;
		}

		if (target.isDead())
		{
			this.IGIS.error("Your target is dead.");
			return;
		}

		if (target.hasStatusEffect(this.StatusEffects.UNATTACKABLE) || target.hasStatusEffect(this.StatusEffects.INVINCIBLE))
		{
			this.IGIS.error("You cannot attack that target.");
			return;
		}

		if (target.isPlayer() && !target.hasStatusEffect(this.StatusEffects.PVPABLE))
		{
			this.IGIS.error("You cannot attack that target.");
			return;
		}

		if (force || !this._avatar.hasStatusEffect(this.StatusEffects.IN_COMBAT_STAND))
		{
			local ab;
			local quickbar1 = ::_quickBarManager.getQuickBar(0);
			local abilityActiveAnimation = this.GUI.Container();
			abilityActiveAnimation.setSize(32, 32);

			if (ranged)
			{
				this.mRangedAutoAttackActive = true;
				ab = this._AbilityManager.getAbilityByName("ranged_melee");
				local rangedAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(1);

				if (rangedAbilityButton)
				{
					rangedAbilityButton.addExtraComponent(abilityActiveAnimation);
					abilityActiveAnimation.setMaterial("AbilityActive");
					quickbar1.getActionContainer().updateContainer();
				}
			}
			else
			{
				this.mMeleeAutoAttackActive = true;
				ab = this._AbilityManager.getAbilityByName("melee");
				local meleeAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(0);

				if (meleeAbilityButton)
				{
					meleeAbilityButton.addExtraComponent(abilityActiveAnimation);
					abilityActiveAnimation.setMaterial("AbilityActive");
					quickbar1.getActionContainer().updateContainer();
				}
			}

			if (ab)
			{
				ab.sendActivationRequest();
			}
		}
	}

	function stopAutoAttack( ranged )
	{
		local quickbar1 = ::_quickBarManager.getQuickBar(0);

		if (ranged)
		{
			local rangedAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(1);

			if (rangedAbilityButton)
			{
				rangedAbilityButton.removeExtraComponent();
				this.mRangedAutoAttackActive = false;
			}
		}
		else
		{
			local meleeAbilityButton = quickbar1.getActionContainer().getActionButtonFromIndex(0);

			if (meleeAbilityButton)
			{
				meleeAbilityButton.removeExtraComponent();
				this.mMeleeAutoAttackActive = false;
			}
		}
	}

	function onServerPosition( pX, pY, pZ )
	{
		if (!this.mController)
		{
			local pos = this.Util.safePointOnFloor(this.Vector3(pX, pY, pZ), this.getNode());
			this.setPosition(pos);
		}
		else
		{
			this.mController.onServerPosition(pX, pY, pZ);
		}
	}

	function onServerVelocity( pServerHeading, pServerRotation, pServerSpeed )
	{
		if (!this.mLastServerUpdate)
		{
			local rotation = pServerRotation;
			this.mRotation = pServerRotation;
			this.mHeading = pServerHeading;
			this.setOrientation(rotation);
		}

		if (this.mController)
		{
			this.mController.onServerVelocity(pServerHeading, pServerRotation, pServerSpeed);
		}
	}

	function getDistanceToFloor()
	{
		return this.mDistanceToFloor;
	}

	function setDistanceToFloor( value, ... )
	{
		if (this.mDistanceToFloor == value)
		{
			return;
		}

		this.mDistanceToFloor = value;

		if (this.mController)
		{
			this.mController.setFalling(this.mDistanceToFloor > 0.0099999998);
		}
	}

	function getPosition()
	{
		return this.mNode != null ? this.mNode.getPosition() : null;
	}

	function getTerrainPageName()
	{
		return this.Util.getTerrainPageName(this.mNode.getPosition());
	}

	function getTerrainPageCoords()
	{
		return this.Util.getTerrainPageIndex(this.mNode.getPosition());
	}

	function getSceneryPageCoords()
	{
		return {
			x = (this.mNode.getPosition().x / ::_sceneObjectManager.mCurrentZonePageSize).tointeger(),
			z = (this.mNode.getPosition().z / ::_sceneObjectManager.mCurrentZonePageSize).tointeger()
		};
	}

	function setPosition( pos )
	{
		if (pos == null)
		{
			return;
		}

		local tx = pos.x;
		local ty = pos.y;
		local tz = pos.z;

		if (this.mNode == null)
		{
			return;
		}

		local oldPos = this.mNode.getPosition();

		if (!this.Util.fuzzyCmpVector3(pos, oldPos))
		{
			this.mNode.setPosition(pos);

			if (this.mAssemblingNode)
			{
				pos.y += 8.0;
				this.mAssemblingNode.setPosition(pos);
			}

			if (this.mAssembler)
			{
				this.mAssembler.notifyTransformed(this);
			}

			this._LightingManager.queueVisibilityUpdate(this);
		}
	}

	function setOrientation( value )
	{
		if (typeof value == "float" || typeof value == "integer")
		{
			value = this.Quaternion(value, this.Vector3().UNIT_Y);
		}

		this.Assert.isInstanceOf(value, this.Quaternion);
		local rot = this.mNode.getOrientation();

		if (rot && this.Util.fuzzyCmpQuaternion(rot, value))
		{
			return;
		}

		this.mNode.setOrientation(value);

		if (this.mAssembler)
		{
			this.mAssembler.notifyTransformed(this);
		}
	}

	function getOrientation()
	{
		return this.mNode.getOrientation();
	}

	function setShowingShadow( value )
	{
		if ((value ? true : false) == this.isShowingShadow())
		{
			return;
		}

		this.mShadowVisible = value;

		if (value)
		{
		}
		else if (this.mShadowDecal)
		{
			this.mShadowDecal.destroy();
			this.mShadowDecal = null;
		}
		else if (this.mShadowProjector)
		{
			this.mShadowProjector.getParentSceneNode().destroy();
			this.mShadowProjector = null;
		}
	}

	function isShowingShadow()
	{
		return this.mShadowVisible;
	}

	function setShowingSelection( value )
	{
		if ((value ? true : false) == this.isShowingSelection())
		{
			return;
		}

		if (value)
		{
			local radius = this.getBoundingRadius();

			if (radius <= 1.5)
			{
				radius = 1.5;
			}

			this.mSelectionProjector = this._scene.createTextureProjector(this.getNodeName() + "/SelectionDecal", "SelectionRing.png");
			this.mSelectionProjector.setNearClipDistance(0.1);
			this.mSelectionProjector.setFarClipDistance(160);
			this.mSelectionProjector.setOrthoWindow(radius, radius);
			this.mSelectionProjector.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
			this.mSelectionProjector.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
			this.mSelectionProjector.setAlphaBlended(true);
			local parentNode = this.mAssemblingNode ? this.mAssemblingNode : this.mNode;
			this.mSelectionNode = parentNode.createChildSceneNode();
			this.mSelectionNode.setPosition(this.Vector3(0, 150, 0));
			this.mSelectionNode.attachObject(this.mSelectionProjector);
			this.mSelectionNode.lookAt(parentNode.getWorldPosition());
		}
		else if (this.mSelectionProjector)
		{
			local parentNode = this.mAssemblingNode ? this.mAssemblingNode : this.mNode;
			this.mSelectionNode.detachObject(this.mSelectionProjector);
			parentNode.removeChild(this.mSelectionNode);
			this.mSelectionProjector.destroy();
			this.mSelectionNode.destroy();
			this.mSelectionProjector = null;
			this.mSelectionNode = null;
		}
	}

	function isShowingSelection()
	{
		return this.mSelectionProjector != null;
	}

	function setScale( value )
	{
		if (value == null)
		{
			value = this.mNormalSize;
		}

		if (typeof value == "float" || typeof value == "integer")
		{
			value = this.Vector3(value, value, value);
		}

		local s = this.mNode.getScale();

		if (s && this.Util.fuzzyCmpVector3(s, value))
		{
			return;
		}

		this.mNode.setScale(value);

		if (this.mAssembler)
		{
			this.mAssembler.notifyTransformed(this);
		}

		this.updateInteractParticle();
	}

	function getScale()
	{
		return this.mNode.getScale();
	}

	function getNormalSize()
	{
		return this.mNormalSize;
	}

	function setNormalSize( size )
	{
		this.mNormalSize = size;
	}

	function getVars()
	{
		return this.mVars;
	}

	function setVars( vars )
	{
		this.setType(this.mType, vars);
	}

	function getFlags()
	{
		return this.mFlags;
	}

	function setFlags( flags )
	{
		local wasPrimary = this.isPrimary();
		this.mFlags = flags;
	}

	function isLocked()
	{
		return (this.mFlags & this.LOCKED) != 0;
	}

	function isDead()
	{
		return this.mDead;
	}

	function isGMFrozen()
	{
		return this.hasStatusEffect(this.StatusEffects.GM_FROZEN);
	}

	function hasLoot()
	{
		return this.mHasLoot;
	}

	function setLocked( value )
	{
		if (value)
		{
			this.setFlags(this.mFlags | this.LOCKED);
		}
		else
		{
			this.setFlags(this.mFlags & ~this.LOCKED);
		}
	}

	function isPrimary()
	{
		return this.mFlags & this.PRIMARY;
	}

	function setPrimary( value )
	{
		if (value)
		{
			this.setFlags(this.mFlags | this.PRIMARY);
		}
		else
		{
			this.setFlags(this.mFlags & ~this.PRIMARY);
		}
	}

	function setSceneryLayer( name )
	{
		this.mSceneryLayer = name;
	}

	function getSceneryLayer()
	{
		return this.mSceneryLayer;
	}

	function setOpacity( value )
	{
		if (value > 0.99900001)
		{
			value = 1.0;
		}

		if (this.mOpacity == value)
		{
			return;
		}

		this.mOpacity = value;

		if (this.mNode == null)
		{
			return;
		}

		local body = this.getAssembler().getBaseEntity(this);

		if (body)
		{
			body.setOpacity(value);
			local entities = body.getAttachedObjects();

			foreach( i, x in entities )
			{
				if (x != null && (x instanceof this.Entity))
				{
					x.setOpacity(value);
				}
			}
		}
	}

	function onAttachmentPointChanged( entity )
	{
		if (entity != null && (entity instanceof this.Entity))
		{
			entity.setOpacity(this.mOpacity);
		}
	}

	function getOpacity()
	{
		local assembler = this.getAssembler();

		if (assembler)
		{
			local body = assembler.getBaseEntity(this);

			if (body)
			{
				return body.getOpacity();
			}
		}

		return 1.0;
	}

	function getEntity()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getBaseEntity(this);
		}

		return null;
	}

	function getBoundingRadius()
	{
		if (this.mAssembler)
		{
			return this.mAssembler.getBoundingRadius(this);
		}

		return 0.0;
	}

	function getBoundingBox()
	{
		local e = this.getEntity();

		if (e)
		{
			return e.getBoundingBox();
		}

		return this.AxisAlignedBox();
	}

	function onCreatureDefUpdate( statId, value )
	{
		this.broadcastMessage("onStatUpdated", this, statId, value);
	}

	function setStat( statId, value )
	{
		if (this.mStats == null)
		{
			this.mStats = {};
		}

		local oldValue = statId in this.mStats ? this.mStats[statId] : null;

		if (this.Util.tableSetOrRemove(this.mStats, statId, value))
		{
			this.onStatUpdated(statId, value, oldValue);
			this.broadcastMessage("onStatUpdated", this, statId, value);
		}
	}

	function onStatUpdated( statId, value, oldValue )
	{
		if (::_avatar == this)
		{
			::_tutorialManager.onStatUpdated(statId, value, oldValue);
		}

		if (statId == this.Stat.APPEARANCE_OVERRIDE || statId == this.Stat.LOOT_SEEABLE_PLAYER_IDS)
		{
			local appearance = this.getStat(this.Stat.APPEARANCE_OVERRIDE);
			local lootString = this.getStat(this.Stat.LOOT_SEEABLE_PLAYER_IDS);

			if (appearance == "")
			{
				this.setAssembler(null);
				return;
			}

			if (this != ::_avatar && appearance && lootString)
			{
				this.handleLootbagChange(lootString, appearance);
			}
		}
		else if (statId == this.Stat.REZ_PENDING)
		{
			if (value > 0 && this.mDead)
			{
				local rez_screen = this.Screens.get("RezScreen", false);
			}
		}
		else if (statId == this.Stat.LEVEL)
		{
			if (::_avatar == this)
			{
				::QuestIndicator.updateCreatureIndicators();
			}

			if (oldValue != null && value != oldValue)
			{
				if (::_avatar == this)
				{
					::IGIS.info("You have reached Level " + value + "!");
				}

				this.cue("LevelDing");
			}
		}
		else if (statId == this.Stat.CURRENT_ABILITY_POINTS)
		{
			if (oldValue != null && value != oldValue)
			{
				if (::_avatar == this)
				{
					local difference = value - oldValue;

					if (difference > 0)
					{
						::IGIS.info("You gain " + difference + " ability points.");
					}
				}
			}
		}
		else if (statId == this.Stat.VIS_WEAPON)
		{
			if (::_avatar != this)
			{
				this.setVisibleWeapon(value, true, false);
			}
		}
		else if (statId == this.Stat.MOD_MOVEMENT)
		{
			if (this == ::_avatar)
			{
				::_avatar.getController().setAvatarSpeed(100 + value);
			}
		}
		else if (statId == this.Stat.BASE_STATS)
		{
			if (value != "")
			{
				local splitStats = this.Util.split(value, ",");
				this.mBaseStats[this.Stat.STRENGTH] = splitStats[0].tointeger();
				this.mBaseStats[this.Stat.DEXTERITY] = splitStats[1].tointeger();
				this.mBaseStats[this.Stat.CONSTITUTION] = splitStats[2].tointeger();
				this.mBaseStats[this.Stat.PSYCHE] = splitStats[3].tointeger();
				this.mBaseStats[this.Stat.SPIRIT] = splitStats[4].tointeger();
			}
		}
		else if (statId == this.Stat.SELECTIVE_EQ_OVERRIDE)
		{
			if (this.mDead)
			{
				return;
			}

			local eqOverride = this.unserialize(value);

			if (oldValue && oldValue != "" && oldValue != value)
			{
				local oldEQOverride = this.unserialize(oldValue);

				foreach( eqSlot, itemDef in oldEQOverride )
				{
					if (!(eqSlot in eqOverride))
					{
						this.removeAttachmentOverride(eqSlot);
					}
				}
			}

			if (value != "")
			{
				foreach( eqSlot, itemDefId in eqOverride )
				{
					local callback = {
						so = this,
						slot = eqSlot,
						function doWork( itemDef )
						{
							if (this.so == null)
							{
								return;
							}

							local type;
							local colors;
							local appearance = itemDef.getAppearance();

							if (appearance && appearance != "")
							{
								if (typeof appearance == "string")
								{
									appearance = this.unserialize(appearance);
								}

								appearance = appearance[0];

								if ("a" in appearance)
								{
									appearance = appearance.a;

									if ("type" in appearance)
									{
										type = appearance.type;
									}

									if ("colors" in appearance)
									{
										colors = appearance.colors;
									}

									local placeAttacmentCallback = {
										sceneObject = this.so,
										equipmentSlot = this.slot,
										def = itemDef,
										attType = type,
										attColors = colors,
										function onPackageComplete( pkg )
										{
											this.sceneObject.placeAttachmentOverride(this.attType, this.attColors, this.def, this.equipmentSlot);
										}

									};

									if (!(type in ::AttachableDef))
									{
										this.Util.waitForAssets(type, placeAttacmentCallback);
										return;
									}

									this.so.placeAttachmentOverride(type, colors, itemDef, this.slot);
								}
							}
						}

					};
					::_ItemDataManager.getItemDef(itemDefId, callback);
				}
			}
		}
		else if (statId == this.Stat.AGGRO_PLAYERS)
		{
			this.updateNameBoardColor();
		}
		else if (statId == this.Stat.SUB_NAME)
		{
			if (this.mNameBoard)
			{
				if (value != "")
				{
					this.mNameBoard.setText(this.getName() + "\n<" + value + ">");
				}
				else
				{
					this.mNameBoard.setText(this.getName());
				}
			}
		}
	}

	function updateNameBoardColor()
	{
		local aggroPlayers = this.getStat(this.Stat.AGGRO_PLAYERS);

		if (aggroPlayers != null && this.mNameBoard)
		{
			if (aggroPlayers == 1)
			{
				this.mNameBoard.setColorTop(this.Color(1.0, 0.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(1.0, 0.0, 0.0, 1.0));
			}
			else if (this.hasStatusEffect(this.StatusEffects.UNATTACKABLE) || this.hasStatusEffect(this.StatusEffects.INVINCIBLE))
			{
				this.mNameBoard.setColorTop(this.Color(0.0, 1.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(0.0, 1.0, 0.0, 1.0));
			}
			else
			{
				this.mNameBoard.setColorTop(this.Color(1.0, 1.0, 0.0, 1.0));
				this.mNameBoard.setColorBottom(this.Color(1.0, 1.0, 0.0, 1.0));
			}
		}
	}

	function placeAttachmentOverride( type, colors, itemDef, slot )
	{
		if (type != null && colors != null)
		{
			local selectiveOverride = this.getStat(this.Stat.SELECTIVE_EQ_OVERRIDE);
			local stringToFind = slot + "]=" + itemDef.getID();

			if (selectiveOverride.find(stringToFind))
			{
				local attachmentPoint = "right_hand";

				if (slot == this.ItemEquipSlot.WEAPON_OFF_HAND)
				{
					attachmentPoint = "left_hand";
				}

				local new_weapon = this.Item.Attachable(null, type, attachmentPoint, colors, null);
				this.addAttachmentOverride(new_weapon, slot);
				new_weapon.assemble();
			}
		}
	}

	function getBaseStatValue( stat )
	{
		if (stat in this.mBaseStats)
		{
			return this.mBaseStats[stat];
		}
		else
		{
			return 0;
		}
	}

	function handleLootbagChange( lootableSeeablePlayerIds, newAppearances )
	{
		local avatarID = ::_avatar.getID();

		if (this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			local appearances = this.Util.split(newAppearances, "|");
			local def = ::_avatar.mCreatureDef;
			local newAppearance = newAppearances;

			if (def)
			{
				local lootableIds = this.Util.split(lootableSeeablePlayerIds, ",");
				local visible = false;

				foreach( i in lootableIds )
				{
					if (i != "" && i.tointeger() == def.mID)
					{
						visible = true;
						break;
					}
				}

				if (visible)
				{
					newAppearance = appearances[0];
					this.mHasLoot = true;
					::_tutorialManager.lootDropped();
				}
				else if (appearances.len() > 1)
				{
					newAppearance = appearances[1];
					this.mHasLoot = false;
				}

				this.morphFromStat(newAppearance);
			}
		}
	}

	function morphFromStat( stat )
	{
		try
		{
			local data = ::unserialize(stat);
			local assembler = this.getAssembler();

			if (data != null && assembler.getObjectType() == data.a)
			{
				  // [016]  OP_POPTRAP        1      0    0    0
				return;
			}

			if (this.mDeathAppearanceChangeEvent)
			{
				::_eventScheduler.cancel(this.mDeathAppearanceChangeEvent);
				this.mDeathAppearanceChangeEvent = null;
			}

			this.log.debug("Morphing to: " + stat);

			if (data == null)
			{
				this.setAssembler(null);
				  // [039]  OP_POPTRAP        1      0    0    0
				return;
			}

			local delay = "delay" in data ? data.delay : 0;
			delay = delay.tointeger();

			if (this.isAssembled())
			{
				if (delay > 0)
				{
					this.mDeathAppearanceChangeEvent = ::_eventScheduler.fireIn(delay.tofloat() / 1000.0, this, "performMorph", data);
					this.log.debug("Scheduled morph: " + stat);
					  // [076]  OP_POPTRAP        1      0    0    0
					return;
				}
			}

			if (this.mDeathAppearanceChangeEvent == null)
			{
				this.performMorph(data);
			}
		}
		catch( err )
		{
			this.log.debug(err);
		}
	}

	function performMorph( data )
	{
		local type = "type" in data ? data.type : "CreatureDef";

		if (type == "CreatureDef")
		{
			type = "Creature";
		}

		if (this.isAssembled() && "effect" in data)
		{
			if (this.mMorphEffectsHandler)
			{
				this.mMorphEffectsHandler.addEffectNarrative(data.effect);
			}
		}

		local assembler = this.GetAssembler(type, data.a);
		this.setAssembler(assembler);
		local size = "size" in data ? data.size : null;
		this.setScale(size);
		this.mDeathAppearanceChangeEvent = null;
	}

	function getStat( statId, ... )
	{
		local value;

		if (this.mStats == null)
		{
			value = null;
		}
		else
		{
			value = this.Util.tableSafeGet(this.mStats, statId);
		}

		if ((value == null || value == "{}") && this.mCreatureDef && (vargc == 0 || vargv[0]))
		{
			value = this.mCreatureDef.getStat(statId);
		}

		return value;
	}

	function getMeta( key )
	{
		return this.mCreatureDef ? this.mCreatureDef.getMeta(key) : null;
	}

	function setShowName( value )
	{
		this.mShowName = value;
	}

	function setHeadLight( bool )
	{
		if (bool && !this.mHeadLight)
		{
			local lightNode = this.mNode.createChildSceneNode();
			this.mShowHeadLight = bool;

			if (this._scene.hasLight("avatarHeadLight"))
			{
				this._scene.getLight("avatarHeadLight").destroy();
				this.log.error("avatarHeadLight being set but it already exists!");
			}

			this.mHeadLight = this._scene.createLight("avatarHeadLight");
			this.mHeadLight.setLightType(this.Light.POINT);
			this.mHeadLight.setAttenuation(1000.0, 0.2, 0.0, 0.00050000002);
			this.mHeadLight.setDiffuseColor(this.Color("ffffff"));
			lightNode.setPosition(0.0, 25.0, 0.0);
			lightNode.attachObject(this.mHeadLight);
		}
		else if (!bool && this.mHeadLight)
		{
			this.mShowHeadLight = bool;
			this.mHeadLight.destroy();
		}
	}

	function setSpeed( value )
	{
		this.mSpeed = value;
	}

	function getSpeed()
	{
		return this.mSpeed;
	}

	function addTimeCastingTime( amtToAdd )
	{
		if (this.isCreature())
		{
			this.mCastingEndTime += amtToAdd;
		}
	}

	function cancelCasting()
	{
		this.mCastingEndTime = 0;
	}

	function getCastingTimeRemaining()
	{
		if (this.isCreature() && this.mTimer)
		{
			return this.mCastingEndTime - this.mTimer.getMilliseconds();
		}

		return -1;
	}

	function getCurrentCastingTime()
	{
		if (this.isCreature() && this.mTimer)
		{
			return this.mTimer.getMilliseconds();
		}

		return -1;
	}

	function getCastingEndTime()
	{
		if (this.isCreature())
		{
			return this.mCastingEndTime;
		}

		return -1;
	}

	function getUsingAbilityID()
	{
		if (this.isCreature())
		{
			return this.mUsingAbilityID;
		}

		return null;
	}

	function isCasting()
	{
		if (this.isCreature())
		{
			return this.mTimer.getMilliseconds() < this.mCastingEndTime;
		}

		return false;
	}

	function isRangedAutoAttackActive()
	{
		return this.mRangedAutoAttackActive;
	}

	function isMeleeAutoAttackActive()
	{
		return this.mMeleeAutoAttackActive;
	}

	function startCasting( id )
	{
		if (this.isCreature() && this.mTimer)
		{
			local ab = this._AbilityManager.getAbilityById(id);
			this.mCastingWarmupTime = ab.getWarmupDuration();
			local totalCastMod = 0.0;
			local modCastingSpeed = this.getStat(this.Stat.MOD_CASTING_SPEED);
			local magicAttackSpeed = this.getStat(this.Stat.MAGIC_ATTACK_SPEED);

			if (modCastingSpeed)
			{
				totalCastMod += modCastingSpeed;
			}

			if (modCastingSpeed)
			{
				totalCastMod += magicAttackSpeed * 0.001;
			}

			this.mCastingWarmupTime -= this.mCastingWarmupTime * totalCastMod;
			this.mCastingEndTime = this.mCastingWarmupTime + this.mTimer.getMilliseconds();
			this.mUsingAbilityID = id;
		}
	}

	function setHeading( value )
	{
		this.mHeading = value;

		if (this.mController)
		{
			this.mController.onHeadingChanged();
		}
	}

	function updateHeading( value )
	{
		this.mHeading = value;
	}

	function getHeading()
	{
		return this.mHeading;
	}

	function setRotation( value )
	{
		this.setOrientation(value);
		this.mRotation = value;
	}

	function getRotation()
	{
		return this.mRotation;
	}

	function getVerticalSpeed()
	{
		return this.mVerticalSpeed;
	}

	function getTargetObject()
	{
		return this.mTargetObject;
	}

	function getStats()
	{
		return this.mStats;
	}

	function getResetTabTarget()
	{
		return this.mResetTabTargetting;
	}

	function setResetTabTarget( value )
	{
		this.mResetTabTargetting = value;
	}

	function setTargetObject( so )
	{
		if (so)
		{
			::_tutorialManager.onTargetSelected(so);
		}

		if (this == ::_avatar && this.hasStatusEffect(this.StatusEffects.DEAD))
		{
			so = null;
		}

		if (this.mTargetObject && this.mTargetObject.hasStatusEffect(this.StatusEffects.HENGE))
		{
			local projectorNode = this._scene.getSceneNode(this.mTargetObject.getNodeName() + "/FeedbackNode");

			if (projectorNode)
			{
				this._scene.getRootSceneNode().removeChild(projectorNode);
				projectorNode.destroy();
			}
		}

		if (this.mTargetObject)
		{
			this.mTargetObject.setShowingSelection(false);
		}

		if (so)
		{
			local targetWindow = ::Screens.show("TargetWindow");
			targetWindow.fillOut(so);
			targetWindow.updateMods(so);

			if (so.hasStatusEffect(this.StatusEffects.HENGE))
			{
				local op = this._scene.createTextureProjector(so.getNodeName() + "/FeedbackOuterProjector", "Area_Circle_Green.png");
				op.setNearClipDistance(0.1);
				op.setFarClipDistance(500);
				op.setAlphaBlended(true);
				op.setOrthoWindow(300, 300);
				op.setProjectionQueryMask(this.QueryFlags.FLOOR | this.QueryFlags.VISUAL_FLOOR);
				op.setVisibilityFlags(this.VisibilityFlags.CREATURE | this.VisibilityFlags.ANY);
				local opn = this._scene.getRootSceneNode().createChildSceneNode(so.getNodeName() + "/FeedbackNode");
				opn.attachObject(op);
				local pos = so.getNode().getWorldPosition();
				pos.y += 200;
				opn.setPosition(pos);
				opn.lookAt(pos + this.Vector3(0, -1, 0));
			}
		}
		else if (::_avatar && ::_avatar.getTargetObject() == so || null == so)
		{
			::Screens.hide("TargetWindow");
		}

		local lastTargetObject = this.mTargetObject;
		this.mTargetObject = so;

		if (lastTargetObject && lastTargetObject != this.mTargetObject)
		{
			lastTargetObject._positionName();
		}

		if (this.mTargetObject)
		{
			this.mTargetObject.setShowingSelection(true);
		}

		::_avatar.broadcastMessage("onTargetObjectChanged", this, this.mTargetObject);
	}

	function isNameShown()
	{
		if (this.mShowName == false)
		{
			return false;
		}

		if (this.mAssembler)
		{
			return this.mAssembler.getShowNameType();
		}

		return false;
	}

	function cork()
	{
		if (::_avatar == this)
		{
			this.log.debug("REZ: Corking");

			if (this.mCorkedStatusEffects)
			{
				this.log.debug("REZ: CorkedStatusEffects alive!!");
			}
		}

		this.mCorking = true;
		this.mCorkTimeout = this.gCorkTimeout;
	}

	function uncork()
	{
		if (::_avatar == this)
		{
			this.log.debug("REZ: Uncorking");
		}

		if (!this.mCorking)
		{
			this.log.debug("Uncorking malfunction");
			this.mCorkTimeout = 0;
			this.mCorkedStatusEffects = null;
			this.mCorkedStatusModifiers = null;
			this.mCorkedFloaties = [];
			this.mCorkedChatMessage = [];
			return;
		}

		local reztest_IsDead = this.StatusEffects.DEAD in this.mCorkedStatusEffects;

		if (::_avatar == this && this.mDead != reztest_IsDead)
		{
			if (this.mDead)
			{
				this.log.debug("REZ: uncorked: dead status changed: Rezed!");
			}
			else
			{
				this.log.debug("REZ: uncorked: dead status changed: Died!");
			}
		}

		this.mCorking = false;
		this.mCorkTimeout = 0;
		local CorkedStatusModifiers = this.mCorkedStatusModifiers;
		local CorkedStatusEffects = this.mCorkedStatusEffects;
		local CorkedFloaties = this.mCorkedFloaties;
		local CorkedCombatMessages = this.mCorkedChatMessage;
		this.mCorkedStatusModifiers = null;
		this.mCorkedStatusEffects = null;
		this.mCorkedFloaties = [];
		this.mCorkedChatMessage = [];

		if (CorkedStatusEffects && CorkedStatusModifiers)
		{
			local wasDead = this.mDead;
			this.setStatusModifiers(CorkedStatusModifiers, CorkedStatusEffects);

			if (!wasDead && this.mDead)
			{
				local appearanceOverride = this.getStat(this.Stat.APPEARANCE_OVERRIDE);
				local lootSeeablePlayerString = this.getStat(this.Stat.LOOT_SEEABLE_PLAYER_IDS);

				if (appearanceOverride && appearanceOverride != "" && lootSeeablePlayerString)
				{
					this.handleLootbagChange(lootSeeablePlayerString, appearanceOverride);
				}
			}
		}

		foreach( cf in CorkedFloaties )
		{
			this.addFloatie(cf.message, cf.type);
		}

		foreach( combatMessage in CorkedCombatMessages )
		{
			this.addCombatMessage(combatMessage.channel, combatMessage.message);
		}
	}

	function isInRange()
	{
		return this.mNameInRange;
	}

	function _positionName()
	{
		if (this.isAssembled() && this.isNameShown() && ::_avatar != this && this.mVisible && this.mNameInRange)
		{
			local namePlateOffset = 0;
			local showNameboard = true;

			if (this.getStat(this.Stat.HIDE_NAMEBOARD) == 1)
			{
				showNameboard = false;

				if (this.mNameBoard)
				{
					this.mNameBoard.destroy();
					this.mNameBoard = null;
				}
			}

			if (this.mNameBoard == null && showNameboard)
			{
				local scale = this.mNamePlateScale == null ? 1.0 : this.mNamePlateScale;
				this.mNameBoard = this._scene.createTextBoard(this.getNodeName() + "/NameBoard", "MaiandraOutline_16", 2.0 * scale, this.getName());
				this.mNode.attachObject(this.mNameBoard);
				this.mNameBoard.setYOffset(this.getNamePlatePosition().y);
				this.mNameBoard.setVisibilityFlags(this.VisibilityFlags.ANY | this.VisibilityFlags.FEEDBACK);
				this.updateNameBoardColor();
				local subName = this.getStat(this.Stat.SUB_NAME);

				if (subName != null && subName != "")
				{
					this.mNameBoard.setText(this.getName() + "\n<" + subName + ">");
				}
				else
				{
					this.mNameBoard.setText(this.getName());
				}
			}
			else if (!showNameboard)
			{
				if (this.mNameBoard)
				{
					this.mNameBoard.destroy();
					this.mNameBoard = null;
				}
			}

			if (this.mNameBoard)
			{
				this.mNameBoard.setVisibilityFlags(::_UIVisible == true ? this.VisibilityFlags.ANY | this.VisibilityFlags.FEEDBACK : 0);
			}
		}
		else if (this.mNameBoard)
		{
			this.mNameBoard.destroy();
			this.mNameBoard = null;
		}
	}

	function cue( visual, ... )
	{
		if (!this.mAssembled)
		{
			return;
		}

		if (this.gLogEffects)
		{
			this.log.debug("Visual Cue for " + this + ": " + visual);
		}

		local result;

		if (this.mEffectsHandler)
		{
			if (vargc > 2)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0], vargv[1], vargv[2]);
			}
			else if (vargc > 1)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0], vargv[1]);
			}
			else if (vargc > 0)
			{
				result = this.mEffectsHandler.addEffectNarrative(visual, vargv[0]);
			}
			else
			{
				result = this.mEffectsHandler.addEffectNarrative(visual);
			}
		}

		return result;
	}

	function playSound( sound, ... )
	{
		if (this.mSoundEmitters == null)
		{
			this.mSoundEmitters = [];

			if (this.mObjectClass == "Scenery")
			{
				this._sceneObjectManager.addUpdateListener(this);
			}
		}

		local emitter = this._audioManager.createSoundEmitter(sound);
		emitter.setAmbient(true);
		this.mSoundEmitters.append(emitter);
		this.mNode.attachObject(emitter);
		emitter.play();
		return emitter;
	}

	function stopSounds()
	{
		if (this.mSoundEmitters == null)
		{
			return;
		}

		foreach( e in this.mSoundEmitters )
		{
			e.stop();
			e.destroy();
		}

		this.mSoundEmitters = null;

		if (this.mObjectClass == "Scenery")
		{
			this._sceneObjectManager.removeUpdateListener(this);
		}
	}

	function getFloorAlignMode()
	{
		return this.mFloorAlignMode;
	}

	function setFloorAlignMode( alignMode )
	{
		this.mFloorAlignMode = alignMode;
	}

	function checkAndSetCTFFlag()
	{
		local carryingRedFlag = this.hasStatusEffect(this.StatusEffects.CARRYING_RED_FLAG);
		local carryingBlueFlag = this.hasStatusEffect(this.StatusEffects.CARRYING_BLUE_FLAG);

		if (carryingRedFlag || carryingBlueFlag)
		{
			this.mCarryingFlag = true;

			if (carryingRedFlag)
			{
				this.mFlag = this.Item.Attachable(null, "Armor-Base1A-Helmet", "back_sheathe");
				this.addAttachment(this.mFlag);
				this.mFlag.assemble();
			}
			else
			{
				this.mFlag = this.Item.Attachable(null, "Armor-Base1A-Helmet", "back_sheathe");
				this.addAttachment(this.mFlag);
				this.mFlag.assemble();
			}
		}
		else if (this.mCarryingFlag == true)
		{
			this.mCarryingFlag = false;
			this.removeAttachment(this.mFlag);
			this.mFlag = null;
		}
	}

	function _checkSounds()
	{
		if (this.mSoundEmitters == null)
		{
			return;
		}

		local newEmitters = [];

		foreach( e in this.mSoundEmitters )
		{
			if (!e.isPlaying())
			{
				e.destroy();
			}
			else
			{
				newEmitters.append(e);
			}
		}

		if (newEmitters.len() == 0)
		{
			this.mSoundEmitters = null;

			if (this.mObjectClass == "Scenery")
			{
				this._sceneObjectManager.removeUpdateListener(this);
			}
		}
		else
		{
			this.mSoundEmitters = newEmitters;
		}
	}

}

class this.SceneObjectSelection extends this.MessageBroadcaster
{
	constructor()
	{
		this.MessageBroadcaster.constructor();
		this.mObjects = [];
	}

	function len()
	{
		return this.mObjects.len();
	}

	function objects( ... )
	{
		if (vargc == 0)
		{
			return clone this.mObjects;
		}

		local objectClass = vargv[0];
		local so;
		local result = [];

		foreach( so in this.mObjects )
		{
			if (so.getObjectClass() == objectClass)
			{
				result.append(so);
			}
		}

		return result;
	}

	function contains( so )
	{
		return this.indexOf(so) != null;
	}

	function indexOf( so )
	{
		local idx;
		local o;

		if (typeof so == "integer")
		{
			foreach( idx, o in this.mObjects )
			{
				if (o.getID() == so)
				{
					return idx;
				}
			}
		}
		else if (typeof so == "string")
		{
			foreach( idx, o in this.mObjects )
			{
				if (o.getNodeName() == so)
				{
					return idx;
				}
			}
		}
		else
		{
			foreach( idx, o in this.mObjects )
			{
				if (o == so)
				{
					return idx;
				}
			}
		}

		return null;
	}

	function add( so )
	{
		local idx = this.indexOf(so);

		if (idx != null)
		{
			return;
		}

		this.mObjects.append(so);
		this.Util.setNodeShowBoundingBox(so.getNode(), true);
		this.broadcastMessage("objectAdded", this, so);
	}

	function remove( so )
	{
		local idx = this.indexOf(so);

		if (idx == null)
		{
			return null;
		}

		so = this.mObjects[idx];
		this.mObjects.remove(idx);
		this.Util.setNodeShowBoundingBox(so.getNode(), false);
		this.broadcastMessage("objectRemoved", this, so);
		return so;
	}

	function clear()
	{
		local so;

		foreach( so in this.mObjects )
		{
			this.broadcastMessage("objectRemoved", this, so);
			this.Util.setNodeShowBoundingBox(so.getNode(), false);
		}

		this.mObjects = [];
	}

	mObjects = null;
}

