require("Assemblers/Assembler");


/**
	An assembler that knows how to create the various types of
	scenery that the server knows about. This includes static 
	entities (e.g. trees) and more complicated scenery using
	the CSM instancing model.
	<p>
	These rely on asset references from the server to tell us more
	information	about what exactly it looks like. For instance,
	which mesh file to use, or what CSM file to instantiate.
*/


class Assembler.Scenery extends Assembler.Factory {
	/**
		The "asset" representing this scenery. This includes the asset
		and any "vars" (the query part of an AssetRef).
	*/	
	mAssetRef = null;
	
	// Cached result for isProp()
	mIsProp = null;
	
	// Cached result for isVegetation()
	mIsVegetation = null;
	
	// Cached result for isLight()
	mIsLight = null;
	
	// Flags whether or not the asset is valid and loaded
	mInvalidAsset = true;
	
	// The unique ID of this assembler
	mName = "";
	
	// The ID of the next CSM instance
	static mNextCSMInstanceID = 0;
	
	constructor( type )	{
		Assembler.Factory.constructor("Scenery", type);
		setAsset(type);
		
		mName = "CSM_ASSEMBLER(" + type + ")_" + mNextCSMInstanceID++;
	}

	function getName() {
		return this.mName;
	}

	function getShowNameType() {
		return false;
	}

	// Override
	function getAssemblerAlias() {
		if (mAssemblerAlias)
			return mAssemblerAlias;

		if (mAssetRef)
			return mAssetRef.getAsset();

		return mObjectType;
	}

	function getAsset()	{
		return mAssetRef;
	}

	function reset() {
		// Reset state
		
		mIsVegetation = null;
		mIsProp = null;
		mIsLight = null;
		mInvalidAsset = false;
		
		// Resets archive error flags, dependencies, etc
		
		Assembler.Factory.reset();
	}

	function setAsset( asset ) {
	
		// This is a reference to an asset in an archive,
		// so we must make sure it's loaded first.
		
		
		try {
			mAssetRef = Util.makeAssetRef(asset, true);
		}
		catch( err ) {
			local str = "Cannot assemble " + asset + ": " + err;
			mInvalidAsset = true;
			return;
		}
		
		// Reset state

		reset();
		
		// Add to required archive list
		
		_addRequiredArchive(GetAssetArchive(asset));
		
		// Well, vegetation does, but they declare a function for
		// obtaining them.
		local asset = mAssetRef.getAsset();

		if (asset in ::Vegetation) {
			mIsVegetation = true;
			local deps = GetVegetationDeps(asset);

			foreach( d in deps )
				_addRequiredArchive(d);
		}
		else if (this.Util.startsWith(asset, "Prop-")) {
			mIsProp = true;
		}
		else if (this.Util.startsWith(asset, "Light-"))	{
			mIsLight = true;
		}

		mInvalidAsset = false;
		
		// Reassemble the instances if we've got everything (otherwise
		// a reassembly is queued for when they are fetched).

		if (_checkRequiredArchivesLoaded())
			reassembleInstances();
	}

	function onPackageError( name, error ) {
		local str = "Cannot assemble " + mAssetRef.getAsset() + ": " + error;
		IGIS.error(str);
		
		mInvalidAsset = true;
	}

	function getReady()	{
		return mAssetRef != null;
	}

	function _initAssemblyData( so )
	{
		if (so.mAssemblyData == null)
		{
			so.mAssemblyData = {
				assetNode = null,
				depsWaiter = null
			};
		}
	}

	function _assemble( so )
	{
		// Do we have an asset?
		if (!Util.hasPermission("dev")) {
			if (mAssetRef.getAsset().find("Manipulator-") == 0)
			{
				so._setAssembled(true);
				return true;
			}
		}

		if (mAssetRef == null) {
			throw Exception("No asset defined");
		}

		// Just use a stand-in object if we ran into an archive error
		
		if (mInvalidAsset)
			throw Exception("Invalid asset");


		// Do not allow the scenery to assemble until the terrain underneath the scenery page
		// has been loaded.
		
		local coords = so.getPosition();

		if (!::_sceneObjectManager.getTerrainReady(coords.x, coords.z))
			return false;

		if ((so.getFlags() & SceneObject.PRIMARY) == 0) {
			local pages = ::_sceneObjectManager.getPages();

			foreach( p in pages ) {
				foreach( s in p.getScenery() ) {
					if (::_sceneObjectManager.hasScenery(s)) {
						local scenery = ::_sceneObjectManager.getSceneryByID(s);
						if ((scenery.getFlags() & SceneObject.PRIMARY) != 0)
							return false;
					}
				}
			}
		}

		// We want to nab the vars from the scene object itself (if it has some)
		// rather than rely on the asset defaults that are part of this assembler.
		// TODO: Maybe we should merge instead?
		local ref;

		if (so.mVars != null)
			ref = AssetReference(mAssetRef.getArchive(), mAssetRef.getAsset(), so.mVars);
		else
			ref = mAssetRef;
		
		// Setup the assembly data if necessary

		_initAssemblyData(so);

		if (!_checkRequiredArchivesLoaded())
			return false;
			
		// Wait for the ATS dependencies to load

		local vars = ref.getVars();

		if (vars && "ATS" in vars)
			if (!this._checkAssemblyDeps(so, [ vars["ATS"] ] ) )
				return false;
		
		if (gPrepareResources) {
			local resources = [];
			if (ref.getAsset() in ::Vegetation) {
				foreach( k, v in ::Vegetation[ref.getAsset()] )
					if (k != "-meta-") 
						resources.append(k);
			}
			else 
				resources.append(ref.getAsset());

			if (!_root.prepareResources(resources, getAssemblerName(), ref.getVars()))
				return false;
		}

		// Concatenate the name of the scene object with the name
		// of the assembler to get a unique per-node instance name.
		local name = so.getName() + "/" + getName();

		// if the assembly data is null then we have not started assembling yet
		if (so.mAssemblyData.assetNode == null)
		{
			local pos = so.getPosition() + "";
			local node = AssembleAsset(ref, so.getNode(), false, name);
	
			// If the CSM could not be parsed then we'll use a 'filler' node instead

			if (!node)
				throw this.Exception("Unable to assemble scenery: " + ref);

			so.mAssemblyData.assetNode = node;
			
			// We'll only track the building of the CSM if it is not vegetation
			// Vegetation is more complex to track, and generally does not impact
			// collision any, so we don't care when it assembles.

			if (node && (ref.getAsset() in ::Vegetation) == false) {
				_sceneObjectManager._notifyCSMBuilding(name);
				return false;
			}
		}
		else if (_sceneObjectManager._isCSMBuilding(name))
			// If the scene object is currently building then we'll return false until it's done
			return false;

		// Special case for building mode to keep selections
		// looking proper.		
		local buildingWith = false;

		if (::_buildTool != null && _buildTool.getSelection().contains(so))
			buildingWith = true;

		if (buildingWith)
			Util.setNodeShowBoundingBox(so.getNode(), true);

		// Assembling has completed!
		so._setAssembled(true);
		
		return true;
	}

	function disassemble( so ) {
		if (so.isAssembled() == false)
			return;

		if (so.getEffectsHandler())
			so.getEffectsHandler().onDisassembled();

		if ("assetNode" in so.mAssemblyData)
			if (so.mAssemblyData.assetNode)
				so.mAssemblyData.assetNode.destroy();

		if ("pagedGeometryHandle" in so.mAssemblyData) {
			// If we have a handle, the geometry page will have
			// been set as well.
			so.mGeometryPage.removeComponent(so.mAssemblyData.pagedGeometryHandle);
			so.mGeometryPage = null;
		}

		if ("vegetator" in so.mAssemblyData)
			so.mAssemblyData.vegetator.destroy();

		Assembler.Factory.disassemble(so);
	}

	function _checkAssemblyDeps( so, deps ) {
	
		// Are we already waiting for the dependencies?
		
		local depsWaiter = so.mAssemblyData.depsWaiter;

		if (depsWaiter != null) {
			if (depsWaiter.isReady())
				return true;

			return false;
		}

		so.mAssemblyData.depsWaiter = Util.waitForAssets(deps, null);
		return false;
	}

	function getBaseEntity( so ) {
		return null;
	}

	function getBoundingRadius( so )
	{
 		// Bad guess, but at least it's something. Primarily we just
 		// want to make sure that we show up in a bordering page's
 		// update/load event, so over-estimating isn't the end of
 		// the world.		
 		
		local guess = 100.0;

		if (!mAssetRef)
			return guess;
		
		// This should be abstracted a bit, but for now, let's hack it.
		if (isVegetation())
		{
			local v = this.mAssetRef.getVars();
			if ("sz" in v)
				return v.sz;
		}

		local res = {
			size = 0.0
		};
		local cb = function ( obj ) : ( res )
		{
			if (obj.getMovableType() != "Entity")
				return;

			local objType = obj.getMovableType();
			local name = obj.getName();
			local radius = obj.getBoundingRadius();

			if (radius > res.size)
				res.size = radius;
		};
		Util.visitMovables(so.getNode(), cb);
		return res.size;
	}

	function isVegetation()	{
		return mIsVegetation;
	}

	function isProp() {
		return mIsProp;
	}

	function isLight() {
		return mIsLight;
	}

	function notifyTransformed( so ) {
		if (so.isScenery())
			so.reassemble();
	}

}

