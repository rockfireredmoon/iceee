this.require("Assemblers/Assembler");
class this.Assembler.Scenery extends this.Assembler.Factory
{
	mAssetRef = null;
	mIsProp = null;
	mIsVegetation = null;
	mIsLight = null;
	mInvalidAsset = true;
	mName = "";
	static mNextCSMInstanceID = 0;
	constructor( type )
	{
		this.Assembler.Factory.constructor("Scenery", type);
		this.setAsset(type);
		this.mName = "CSM_ASSEMBLER(" + type + ")_" + this.mNextCSMInstanceID++;
	}

	function getName()
	{
		return this.mName;
	}

	function getShowNameType()
	{
		return false;
	}

	function getAssemblerAlias()
	{
		if (this.mAssemblerAlias)
		{
			return this.mAssemblerAlias;
		}

		if (this.mAssetRef)
		{
			return this.mAssetRef.getAsset();
		}

		return this.mObjectType;
	}

	function getAsset()
	{
		return this.mAssetRef;
	}

	function reset()
	{
		this.mIsVegetation = null;
		this.mIsProp = null;
		this.mIsLight = null;
		this.mInvalidAsset = false;
		this.Assembler.Factory.reset();
	}

	function setAsset( asset )
	{
		try
		{
			this.mAssetRef = this.Util.makeAssetRef(asset, true);
		}
		catch( err )
		{
			local str = "Cannot assemble " + asset + ": " + err;
			this.mInvalidAsset = true;
			return;
		}

		this.reset();
		this._addRequiredArchive(this.GetAssetArchive(asset));
		local asset = this.mAssetRef.getAsset();

		if (asset in ::Vegetation)
		{
			this.mIsVegetation = true;
			local deps = this.GetVegetationDeps(asset);

			foreach( d in deps )
			{
				this._addRequiredArchive(d);
			}
		}
		else if (this.Util.startsWith(asset, "Prop-"))
		{
			this.mIsProp = true;
		}
		else if (this.Util.startsWith(asset, "Light-"))
		{
			this.mIsLight = true;
		}

		this.mInvalidAsset = false;

		if (this._checkRequiredArchivesLoaded())
		{
			this.reassembleInstances();
		}
	}

	function onPackageError( name, error )
	{
		local str = "Cannot assemble " + this.mAssetRef.getAsset() + ": " + error;
		this.IGIS.error(str);
		this.mInvalidAsset = true;
	}

	function getReady()
	{
		return this.mAssetRef != null;
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
		if (!this.Util.hasPermission("dev"))
		{
			if (this.mAssetRef.getAsset().find("Manipulator-") == 0)
			{
				so._setAssembled(true);
				return true;
			}
		}

		if (this.mAssetRef == null)
		{
			throw this.Exception("No asset defined");
		}

		if (this.mInvalidAsset)
		{
			throw this.Exception("Invalid asset");
		}

		local coords = so.getPosition();

		if (!::_sceneObjectManager.getTerrainReady(coords.x, coords.z))
		{
			return 76;
		}

		if ((so.getFlags() & this.SceneObject.PRIMARY) == 0)
		{
			local pages = ::_sceneObjectManager.getPages();

			foreach( p in pages )
			{
				foreach( s in p.getScenery() )
				{
					if (::_sceneObjectManager.hasScenery(s))
					{
						local scenery = ::_sceneObjectManager.getSceneryByID(s);

						if ((scenery.getFlags() & this.SceneObject.PRIMARY) != 0)
						{
							return 75;
						}
					}
				}
			}
		}

		local ref;

		if (so.mVars != null)
		{
			ref = this.AssetReference(this.mAssetRef.getArchive(), this.mAssetRef.getAsset(), so.mVars);
		}
		else
		{
			ref = this.mAssetRef;
		}

		this._initAssemblyData(so);

		if (!this._checkRequiredArchivesLoaded())
		{
			return 50;
		}

		local vars = ref.getVars();

		if (vars && "ATS" in vars)
		{
			if (!this._checkAssemblyDeps(so, [
				vars.ATS
			]))
			{
				return 50;
			}
		}

		if (this.gPrepareResources)
		{
			local resources = [];

			if (ref.getAsset() in ::Vegetation)
			{
				foreach( k, v in ::Vegetation[ref.getAsset()] )
				{
					if (k != "-meta-")
					{
						resources.append(k);
					}
				}
			}
			else
			{
				resources.append(ref.getAsset());
			}

			if (!this._root.prepareResources(resources, this.getAssemblerName(), ref.getVars()))
			{
				return 25;
			}
		}

		local name = so.getName() + "/" + this.getName();

		if (so.mAssemblyData.assetNode == null)
		{
			local pos = so.getPosition() + "";
			local node = this.AssembleAsset(ref, so.getNode(), false, name);

			if (!node)
			{
				throw this.Exception("Unable to assemble scenery: " + ref);
			}

			so.mAssemblyData.assetNode = node;

			if (node && (ref.getAsset() in ::Vegetation) == false)
			{
				this._sceneObjectManager._notifyCSMBuilding(name);
				return 50;
			}
		}
		else if (this._sceneObjectManager._isCSMBuilding(name))
		{
			return 26;
		}

		local buildingWith = false;

		if (::_buildTool != null && this._buildTool.getSelection().contains(so))
		{
			buildingWith = true;
		}

		if (buildingWith)
		{
			this.Util.setNodeShowBoundingBox(so.getNode(), true);
		}

		so._setAssembled(true);
		return true;
	}

	function disassemble( so )
	{
		if (so.isAssembled() == false)
		{
			return;
		}

		if (so.getEffectsHandler())
		{
			so.getEffectsHandler().onDisassembled();
		}

		if ("assetNode" in so.mAssemblyData)
		{
			if (so.mAssemblyData.assetNode)
			{
				so.mAssemblyData.assetNode.destroy();
			}
		}

		if ("pagedGeometryHandle" in so.mAssemblyData)
		{
			so.mGeometryPage.removeComponent(so.mAssemblyData.pagedGeometryHandle);
			so.mGeometryPage = null;
		}

		if ("vegetator" in so.mAssemblyData)
		{
			so.mAssemblyData.vegetator.destroy();
		}

		this.Assembler.Factory.disassemble(so);
	}

	function _checkAssemblyDeps( so, deps )
	{
		local depsWaiter = so.mAssemblyData.depsWaiter;

		if (depsWaiter != null)
		{
			if (depsWaiter.isReady())
			{
				return true;
			}

			return false;
		}

		so.mAssemblyData.depsWaiter = this.Util.waitForAssets(deps, null);
		return false;
	}

	function getBaseEntity( so )
	{
		return null;
	}

	function getBoundingRadius( so )
	{
		local guess = 100.0;

		if (!this.mAssetRef)
		{
			return guess;
		}

		if (this.isVegetation())
		{
			local v = this.mAssetRef.getVars();

			if ("sz" in v)
			{
				return v.sz;
			}
		}

		local res = {
			size = 0.0
		};
		local cb = function ( obj ) : ( res )
		{
			if (obj.getMovableType() != "Entity")
			{
				return;
			}

			local objType = obj.getMovableType();
			local name = obj.getName();
			local radius = obj.getBoundingRadius();

			if (radius > res.size)
			{
				res.size = radius;
			}
		};
		this.Util.visitMovables(so.getNode(), cb);
		return res.size;
	}

	function isVegetation()
	{
		return this.mIsVegetation;
	}

	function isProp()
	{
		return this.mIsProp;
	}

	function isLight()
	{
		return this.mIsLight;
	}

	function notifyTransformed( so )
	{
		if (so.isScenery())
		{
			so.reassemble();
		}
	}

}

