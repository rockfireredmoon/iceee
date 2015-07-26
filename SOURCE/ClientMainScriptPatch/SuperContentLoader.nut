this.REQUEST_PENDING <- 0;
this.REQUEST_LOADED <- 1;
this.REQUEST_ERROR <- 2;
this.REQUEST_UNKNOWN <- 3;
this.MAX_RETRIES <- 5;
this.PREFETCH_COUNT <- 3;
function CLDebug( string )
{
	this.log.debug(string);
}

function GetFullPath( name )
{
	print("ICE! GetFullPath " + name);
	if (this.Util.isDevMode())
	{
		local pos = name.find("-");

		if (pos != null)
		{
			name = name.slice(0, pos) + "/" + name;
		}

		return "../../Media/" + name;
	}
	else
	{
		return "Media/" + name + ".car";
	}
}

function GetAssetName( asset )
{
	local tmp = this.AssetReference(asset);
	return tmp.getAsset();
}

function GetArchiveName( asset )
{
	local tmp = this.AssetReference(asset);
	local name = this.GetAssetArchive(tmp.getAsset());

	if (name == null || name == "")
	{
		name = tmp.getArchive();
	}

	if (name == null)
	{
		name = tmp.getAsset();
	}

	return name;
}

class this.TerrainLoadedCallback 
{
	mX = 0;
	mZ = 0;
	constructor( x, z )
	{
		this.mX = x;
		this.mZ = z;
	}

	function onPackageComplete( pkg )
	{
		this.log.debug("Notifying Scene Manager that page is now available at " + this.mX + ", " + this.mZ);
		this._scene.notifyPageAvailable(this.mX, this.mZ, true);
		local pageStr = "x" + this.mX + "y" + this.mZ;

		if (pageStr in ::TerrainPageDef)
		{
			local entry = ::TerrainPageDef[pageStr];
			local terrain = ::_sceneObjectManager.getCurrentTerrainBase();

			if (!(terrain in ::TerrainEnvDef))
			{
				::TerrainEnvDef[terrain] <- {};
			}

			::TerrainEnvDef[terrain][pageStr] <- entry;
		}
	}

	function onPackageError( name, error )
	{
		if (this.Util.isDevMode())
		{
			this.log.debug("Unable to load terrain page " + this.mX + ", " + this.mZ + ": " + error);
			this._scene.notifyPageAvailable(this.mX, this.mZ, false);
		}
		else
		{
			::UI.FatalError("Unable to load terrain page " + this.mX + ", " + this.mZ + ": " + error);
			this.reset();
		}
	}

}

class this.Archive 
{
	mTries = 0;
	mError = null;
	mDone = false;
	mLoading = false;
	mStartTime = 0;
	mLoadTime = 0;
	mFetchTime = 0;
	function getLoadTime()
	{
		return this.mLoadTime;
	}

	function getFetchTime()
	{
		return this.mFetchTime;
	}

	function onLoadComplete( name )
	{
		this.mLoadTime += this.System.currentTimeMillis() - this.mStartTime;
		this.mLoading = false;
		this.mDone = this.next();
	}

	function onLoadError( name, error )
	{
		::IGIS.error("Unable to load archive: " + name);
		this.mLoading = false;
		this.mError = error;
		this.mDone = true;
		return;
	}

	function onFetchError( name, error )
	{
		if (this.Util.isDevMode())
		{
			::IGIS.error("Unable to load archive: " + name);
			this.mLoading = false;
			this.mError = error;
			this.mDone = true;
			return;
		}

		this.mTries++;

		if (this.mTries > this.MAX_RETRIES)
		{
			this.mError = error;
			this.mLoading = false;
			this.mDone = true;
			::UI.FatalError("Unable to download archive: " + name + ", reason: " + error);
			::_contentLoader.reset();
		}
		else
		{
			this.log.debug("Retrying fetch of archive: " + name + ", Error: " + error);
			this.mLoading = false;
		}
	}

	function onFetchComplete( name, handle )
	{
		local t = this.System.currentTimeMillis();
		this.log.debug("OnFetchComplete : " + name + " " + handle);
		this.mFetchTime += t - this.mStartTime;
		this.mStartTime = t;
		this.load(name, handle);
		this.mLoading = true;
		this.mTries = 0;
	}

	function fetch()
	{
	}

	function contains( name )
	{
		return true;
	}

	function getAllFilePaths()
	{
	}

	function getError()
	{
		return this.mError;
	}

	function next()
	{
		return true;
	}

	function getName()
	{
		return "";
	}

	function getFullName()
	{
		return this.GetFullPath(this.getName());
	}

	function update()
	{
		if (this.mError != null || this.mDone == true)
		{
			return true;
		}

		if (this.mLoading == true)
		{
			return false;
		}

		this.mStartTime = this.System.currentTimeMillis();
		this.mLoading = true;
		this.fetch();
		return this.mDone;
	}

}

class this.FileArchive extends this.Archive
{
	mPath = null;
	mName = null;
	constructor( name, path )
	{
		this.mName = name;
		this.mPath = path;
	}

	function getName()
	{
		return this.mName;
	}

	function contains( name )
	{
		return name == this.getFullName();
	}

	function getAllFilePaths()
	{
		return this.getFullName();
	}

	function load( name, handle )
	{
		::Composition.load(name, handle);
	}

	function fetch()
	{
		::_cache.fetch(this.GetFullPath(this.mPath));
		this.log.debug("Fetching " + this.mPath);
	}

}

class this.VirtualArchive extends this.Archive
{
	mFiles = null;
	mName = null;
	mCurrent = 0;
	constructor( name, paths )
	{
		this.mName = name;
		this.mFiles = paths;
	}

	function next()
	{
		this.mCurrent++;
		return this.mCurrent >= this.mFiles.len();
	}

	function load( name, handle )
	{
		::Composition.load(name, handle);
	}

	function getName()
	{
		return this.mName;
	}

	function fetch()
	{
		if (this.mFiles.len() == 0)
		{
			this.log.debug("EMPTY ARCHIVE: " + this.getName());
			this.mDone = true;
			return;
		}

		::_cache.fetch(this.GetFullPath(this.mFiles[this.mCurrent]));
	}

	function contains( name )
	{
		foreach( n in this.mFiles )
		{
			if (this.GetFullPath(n) == name)
			{
				return true;
			}
		}

		return false;
	}

	function getAllFilePaths()
	{
		local filePaths = [];

		foreach( n in this.mFiles )
		{
			filePaths.append(this.GetFullPath(n));
		}

		return filePaths;
	}

}

class this.DependentArchive extends this.Archive
{
	mFiles = null;
	mName = null;
	mCurrent = 0;
	constructor( name, paths )
	{
		this.mName = name;
		this.mFiles = paths;
	}

	function getName()
	{
		return this.mName;
	}

	function update()
	{
		foreach( n in this.mFiles )
		{
			local status = ::_contentLoader.getAssetStatus(n);

			if (status != this.REQUEST_LOADED)
			{
				this.mError = "Unable to load dependency: " + n;
				return true;
			}
		}

		return true;
	}

}

class this.NullArchive extends this.Archive
{
	mName = null;
	constructor( name )
	{
		this.mError = "Invalid archive name: " + name;
		this.mName = name;
	}

	function getName()
	{
		return this.mName;
	}

	function update()
	{
		return true;
	}

}

class this.QueueEntry 
{
	mAsset = null;
	mAssetRef = null;
	mPriority = null;
	mError = null;
	mState = null;
	mDeps = null;
	constructor( asset, deps, priority )
	{
		this.mState = this.REQUEST_PENDING;
		this.mPriority = priority;
		this.mAsset = asset;
		this.mDeps = deps;
		this.mAssetRef = this.AssetReference(asset);
	}

	function getArchiveName()
	{
		return this.GetArchiveName(this.mAsset);
	}

	function getAsset()
	{
		return this.mAssetRef.getAsset();
	}

	function setPriority( value )
	{
		this.mPriority = value;
	}

	function getPriority()
	{
		return this.mPriority;
	}

	function setError( value )
	{
		this.mError = value;
	}

	function getError()
	{
		return this.mError;
	}

	function getDependencies()
	{
		return this.mDeps;
	}

}

class this.Package 
{
	mAssets = null;
	mCallback = null;
	mName = null;
	constructor( assets, name, callback )
	{
		this.mCallback = callback;
		this.mAssets = assets;
		this.mName = name;
	}

	function getName()
	{
		return this.mName;
	}

	function getAssets()
	{
		return this.mAssets;
	}

	function getCallback()
	{
		return this.mCallback;
	}

}

class this.SuperContentLoader 
{
	mEntries = null;
	mLoadedAssets = null;
	mLoadedArchives = null;
	mLoadTimes = null;
	mArchives = null;
	mArchivesByName = null;
	mCurrentArchive = null;
	mProgressTimer = 0.0;
	mPackages = null;
	mPrefetches = null;
	mCurrentPrefetches = null;
	constructor()
	{
		::_cache.addListener(this);
		::_root.addListener(this);
		::_enterFrameRelay.addListener(this);
		::_scene.setPageLoadingCallback(this);
		this.mArchives = [];
		this.mArchivesByName = {};
		this.mCurrentPrefetches = [];
		this.mPrefetches = {};
		this.mLoadedArchives = {};
		this.mLoadTimes = {};
		this.mLoadedAssets = {};
		this.mPackages = [];
		this.mEntries = [];
	}

	function onProgress( media, bytesLoaded, bytesTotal )
	{
		if (this.mCurrentArchive != null && media == this.mCurrentArchive.getFullName())
		{
			::LoadScreen.downloadProgress(bytesLoaded, bytesTotal, media);
			this.log.debug("Progress: " + bytesLoaded + "/" + bytesTotal);
		}
	}

	function onCompositionLoaded( name )
	{
		if (this.mCurrentArchive == null)
		{
			throw this.Exception("LOGIC ERROR: SuperContentLoader.onCompositionLoaded");
		}

		this.mCurrentArchive.onLoadComplete(name);
		this.flagUpdate();
	}

	function onCompositionLoadError( name, error )
	{
		if (this.mCurrentArchive == null)
		{
			throw this.Exception("LOGIC ERROR: SuperContentLoader.onCompositionLoadError");
		}

		this.mCurrentArchive.onLoadError(name, error);
		this.flagUpdate();
	}

	function onComplete( name, handle )
	{
		local t0 = this.System.currentTimeMillis();
		this.log.debug("Finished fetching archive: " + name);
		this._finishPrefetch(name);
		local t1 = this.System.currentTimeMillis();

		if (this.mCurrentArchive != null && this.mCurrentArchive.contains(name))
		{
			this.mCurrentArchive.onFetchComplete(name, handle);
		}
		else if (this.mCurrentArchive == null)
		{
			this.log.debug("current archive is null, fetch complete not being called " + name);
		}
		else
		{
			local filePaths = this.mCurrentArchive.getAllFilePaths();
			local media = "";

			if (typeof filePaths == "array")
			{
				foreach( filePaths in filePaths )
				{
					media += " " + filePaths;
				}
			}
			else
			{
				media = filePaths;
			}

			this.log.debug(this.mCurrentArchive.getFullName() + " does not contain needed file.  File list : " + media);
		}

		local t2 = this.System.currentTimeMillis();
		this.log.debug("Finished initializing archive: " + name + " in " + (t2 - t0) + " ms (prefetch time = " + (t1 - t0) + " ms)");
		this.flagUpdate();
	}

	function onError( name, error, code )
	{
		this.log.debug("Error fetching archive \'" + name + "\': " + error + ", code: " + code);
		this._finishPrefetch(name);

		if (!this.Util.isDevMode() && code == 404)
		{
			::UI.FatalError("404 File not found: " + name);
			this.reset();
			return;
		}

		if (this.mCurrentArchive != null && this.mCurrentArchive.contains(name))
		{
			this.mCurrentArchive.onFetchError(name, error);
		}

		this.flagUpdate();
	}

	function _finishPrefetch( name )
	{
		if (name in this.mPrefetches)
		{
			delete this.mPrefetches[name];
		}

		local x;

		for( x = 0; x < this.mCurrentPrefetches.len(); x++ )
		{
			if (this.mCurrentPrefetches[x] == name)
			{
				this.log.debug("Finished prefetching " + name + "!");
				this.mCurrentPrefetches.remove(x);
				break;
			}
		}
	}

	function _processPrefetches()
	{
		if (this.mCurrentPrefetches.len() >= this.PREFETCH_COUNT)
		{
			return;
		}

		foreach( k, v in this.mPrefetches )
		{
			if (v == false)
			{
				this.mCurrentPrefetches.append(k);
				this.mPrefetches[k] = true;
				this.log.debug("Prefetching " + k + "...");
				::_cache.fetch(k);

				if (this.mCurrentPrefetches.len() >= this.PREFETCH_COUNT)
				{
					break;
				}
			}
		}
	}

	function prefetch( assets )
	{
		if (this.Util.isDevMode())
		{
			return;
		}

		if (typeof assets != "array")
		{
			assets = [
				assets
			];
		}

		foreach( a in assets )
		{
			local archive = this.GetArchiveName(a);

			if ((archive in this.mPrefetches) == false)
			{
				this.mPrefetches[this.GetFullPath(archive)] <- false;
			}
		}
	}

	function _findEntry( name )
	{
		local asset = this.GetAssetName(name);

		foreach( n in this.mEntries )
		{
			if (n.getAsset() == asset)
			{
				return n;
			}
		}

		return null;
	}

	function isAssetLoaded( name )
	{
		local name = this.GetAssetName(name);
		return name in this.mLoadedAssets;
	}

	function isArchiveLoaded( name )
	{
		return name in this.mLoadedArchives;
	}

	function _findEntryByArchiveName( name )
	{
		foreach( n in this.mEntries )
		{
			if (n.getArchiveName() == name)
			{
				return n;
			}
		}

		return null;
	}

	function _findNextFetchEntry()
	{
		foreach( e in this.mEntries )
		{
			if (e.getState() == this.REQUEST_PENDING)
			{
				return e;
			}
		}

		return null;
	}

	function _fetchNext()
	{
		if (this.mCurrentArchive == null)
		{
			if (this.mArchives.len() == 0)
			{
				return;
			}

			this.mCurrentArchive = this.mArchives[0];
		}

		if (this.mCurrentArchive.update() == false)
		{
			this.flagUpdate();
			return;
		}

		this.log.debug("Finished loading archive: " + this.mCurrentArchive.getName() + " in " + this.mCurrentArchive.getLoadTime() + " milliseconds. Fetch time: " + this.mCurrentArchive.getFetchTime());
		this.mLoaded = this.Math.min(this.mTotal, this.mLoaded + 1);
		local state = this.mCurrentArchive.getError() == null ? this.REQUEST_LOADED : this.REQUEST_ERROR;
		local archiveName = this.mCurrentArchive.getName();
		this.mLoadTimes[archiveName] <- {
			fetch = this.mCurrentArchive.getFetchTime(),
			load = this.mCurrentArchive.getLoadTime(),
			total = this.mCurrentArchive.getFetchTime() + this.mCurrentArchive.getLoadTime()
		};
		this.mLoadedArchives[archiveName] <- state;
		delete this.mArchivesByName[archiveName];
		this.mArchives.remove(0);
		this.mCurrentArchive = null;
		this.flagUpdate();
	}

	function _processPackage( p )
	{
		local assets = p.getAssets();
		local cb = p.getCallback();

		foreach( a in assets )
		{
			local status = this.getAssetStatus(a);

			if (status == this.REQUEST_ERROR)
			{
				try
				{
					if (cb && "onPackageError" in cb)
					{
						cb.onPackageError(p.getName(), "Dependencies failed to load");
					}
				}
				catch( error )
				{
					this.log.debug("Error calling onPackageError: " + error);
				}

				return true;
			}
			else if (status != this.REQUEST_LOADED)
			{
				return false;
			}
		}

		try
		{
			if ("onPackageComplete" in cb)
			{
				cb.onPackageComplete(p.getName());
			}
		}
		catch( error )
		{
			this.log.error("Error calling onPackageComplete: " + error);
		}

		return true;
	}

	function _processPackages()
	{
		local packageList = [];

		foreach( p in this.mPackages )
		{
			if (this._processPackage(p) == false)
			{
				packageList.append(p);
			}
		}

		this.mPackages = packageList;
	}

	function _checkAssetDependencies( names )
	{
		foreach( n in names )
		{
			local status = this.getArchiveStatus(n);

			if (status != this.REQUEST_LOADED)
			{
				return status;
			}
		}

		return this.REQUEST_LOADED;
	}

	function _processAssets()
	{
		local entries = [];

		foreach( a in this.mEntries )
		{
			local depStatus = this._checkAssetDependencies(a.getDependencies());

			if (depStatus == this.REQUEST_PENDING)
			{
				entries.append(a);
				continue;
			}

			this.mLoadedAssets[a.getAsset()] <- depStatus;
		}

		this.mEntries = entries;
	}

	function getAssetStatus( name )
	{
		if (name in this.mLoadedAssets)
		{
			return this.mLoadedAssets[name];
		}

		foreach( a in this.mRequests )
		{
			if (a.getAsset() == name)
			{
				return this.REQUEST_PENDING;
			}
		}

		return this.REQUEST_UNKNOWN;
	}

	function getArchiveStatus( name )
	{
		if (name in this.mLoadedArchives)
		{
			return this.mLoadedArchives[name];
		}

		if (name in this.mArchivesByName)
		{
			return this.REQUEST_PENDING;
		}

		return this.REQUEST_UNKNOWN;
	}

	function onEntryCompleted()
	{
		this.flagUpdate();
	}

	function _queueArchive( name )
	{
		if (this.getArchiveStatus(name) != this.REQUEST_UNKNOWN)
		{
			return;
		}

		local archive;

		if (name in ::Vegetation)
		{
			local deps = [];

			foreach( k, v in ::Vegetation[name] )
			{
				if (k != "-meta-")
				{
					this._queueAsset(k, 0);
					deps.append(k);
				}
			}

			archive = this.DependentArchive(name, deps);
			this.mArchives.append(archive);
			this.mArchivesByName[archive.getName()] <- archive;
			return;
		}

		if (name in ::_ArchiveAlias)
		{
			local deps = [];

			foreach( d in ::_ArchiveAlias[name] )
			{
				if (this.Util.indexOf(deps, d) == null)
				{
					local ad = [];
					this._getDependencies(d, ad);

					foreach( q in ad )
					{
						if (q != name)
						{
							this._queueArchive(q);
						}
					}

					deps.append(d);
				}
			}

			archive = this.Util.isDevMode() ? this.VirtualArchive(name, deps) : this.FileArchive(name, name);
		}
		else
		{
			archive = this.FileArchive(name, name);
		}

		this.mArchives.append(archive);
		this.mArchivesByName[archive.getName()] <- archive;
		this.mTotal++;
	}

	function resetExpected()
	{
		this.mLoaded = 0;
		this.mTotal = 0;
	}

	function updateLoadScreen()
	{
		::_loadScreenManager.setLoadScreenVisible(this.mLoaded != this.mTotal);
		::_loadScreenManager.update(this.mLoaded, this.mTotal);
	}

	mLoaded = 0;
	mTotal = 0;
	function _getDependencies( name, results )
	{
		if (name in ::AssetDependencies)
		{
			local assetDeps = ::AssetDependencies[name];

			foreach( d in assetDeps )
			{
				local archive = this.GetArchiveName(d);

				if (this.getArchiveStatus(archive) != this.REQUEST_UNKNOWN)
				{
					continue;
				}

				this._getDependencies(d, results);
				results.append(archive);
			}
		}
	}

	function _queueAsset( name, priority )
	{
		local status = this.getAssetStatus(name);

		if (status == this.REQUEST_PENDING)
		{
			local entry = this._findEntry(name);

			if (entry != null)
			{
				local currentPriority = entry.getPriority();

				if (currentPriority < priority)
				{
					entry.setPriority(priority);
				}

				return;
			}
		}

		if (status != this.REQUEST_UNKNOWN)
		{
			return;
		}

		local deps = [];
		this._getDependencies(name, deps);
		local archive = this.GetArchiveName(name);
		deps.append(archive);

		foreach( d in deps )
		{
			this._queueArchive(d);
		}

		local entry = this.QueueEntry(name, deps, priority);
		this.mEntries.append(entry);
	}

	function load( assets, priority, name, callback )
	{
		if (typeof assets != "array")
		{
			assets = [
				assets
			];
		}

		local entryList = [];

		foreach( a in assets )
		{
			if (a != "")
			{
				local lower = a.tolower();

				if (lower in ::MediaCaseIndex)
				{
					a = ::MediaCaseIndex[lower];
				}

				this._queueAsset(a, priority);
				entryList.append(this.GetAssetName(a));
			}
		}

		this.mPackages.append(this.Package(entryList, name, callback));
		this.flagUpdate();
	}

	function getRequestStatusDebug()
	{
		return "";
		local result = "\n\n\n\n**************************** RequestStatus Dump *******************\n";

		foreach( e in this.mEntries )
		{
			if (e.getPriority() >= this.ContentLoader.PRIORITY_REQUIRED)
			{
				result += e.getArchiveName() + "\n";
			}
		}

		return result;
	}

	function getUnloadedRequestCount( which )
	{
		local count = 0;

		foreach( e in this.mEntries )
		{
			if (e.getPriority() >= this.ContentLoader.PRIORITY_REQUIRED)
			{
				count++;
			}
		}

		return count;
	}

	function resetProgress( which )
	{
	}

	function reset()
	{
		this.mArchives = [];
		this.mArchivesByName = {};
		this.mCurrentArchive = null;
		this.mLoadedArchives = {};
		this.mLoadTimes = {};
		this.mLoadedAssets = {};
		this.mPackages = [];
		this.mEntries = [];
	}

	function getLoadingPackage( name )
	{
		foreach( p in this.mPackages )
		{
			if (p.getName() == name)
			{
				return p;
			}
		}

		return null;
	}

	function onRequireTerrainPage( x, z, pattern )
	{
		if (pattern != "")
		{
			local name = pattern + "_x" + x + "y" + z;

			if ((name in ::TerrainMap) == false)
			{
				this._scene.notifyPageAvailable(x, z, false);
				return;
			}

			if (!this.Util.isDevMode())
			{
				this.load(name, this.ContentLoader.PRIORITY_REQUIRED, pattern + "_x" + x + "y" + z + "_pkg", this.TerrainLoadedCallback(x, z));
			}
			else
			{
				this._scene.notifyPageAvailable(x, z, true);
			}
		}
		else
		{
			this.log.error("Terrain page event received, but no terrain pattern was provided.");
		}
	}

	function onNearbyTerrainPage( x, z, pattern )
	{
		if (this.Util.isDevMode() == false)
		{
			if (pattern != "")
			{
				local name = pattern + "_x" + x + "y" + z;

				if (name in ::TerrainMap)
				{
					this.load(name, this.ContentLoader.PRIORITY_NORMAL, pattern + "_x" + x + "y" + z + "_pkg", this.TerrainLoadedCallback(x, z));
				}
			}
			else
			{
				this.log.error("Terrain page event received, but no terrain pattern was provided.");
			}
		}
	}

	function onEnterFrame()
	{
		if (this.mUpdate == true)
		{
			this.mUpdate = false;
			this._fetchNext();
		}

		this._processPrefetches();
		this._processAssets();
		this._processPackages();
	}

	function flagUpdate()
	{
		this.mUpdate = true;
	}

	function getLoadedArchives()
	{
		return this.mLoadedArchives;
	}

	function getLoadTimes()
	{
		return this.mLoadTimes;
	}

	mUpdate = false;
	mProgressCurrent = 0;
	mProgressTotal = 0;
	mRequests = [];
}

