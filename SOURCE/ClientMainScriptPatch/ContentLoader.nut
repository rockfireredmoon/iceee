this.require("GUI/GUI");
this.require("UI/LoadScreen");
this.require("ErrorChecking");
this.require("Vegetation");
this.ContentDef <- {};
this.ClothingDef <- {};
this.AttachableDef <- {};
this.BipedAnimationDef <- {};
this.ModelDef <- {};
class this.AssetReference 
{
	static rxParts = this.regexp("([^#]*)#(.*)$");
	constructor( ... )
	{
		if (vargc >= 2)
		{
			this.setArchive(vargv[0]);
			this.setAsset(vargv[1]);

			if (vargc > 2)
			{
				this.setVars(vargv[2]);
			}
			else
			{
				this.mVars = null;
			}
		}
		else if (vargc == 1)
		{
			this.parse("" + vargv[0], this);
		}
		else
		{
			this.mArchive = null;
			this.mAsset = null;
			this.mVars = null;
		}
	}

	static function parse( refStr, ... )
	{
		local ref = vargc > 0 ? vargv[0] : this.AssetReference();
		local queryStr;
		local res = refStr.find("?");

		if (res != null)
		{
			queryStr = refStr.slice(res + 1);
			refStr = refStr.slice(0, res);
		}

		res = this.rxParts.capture(refStr);

		if (res != null)
		{
			ref.mArchive = refStr.slice(res[1].begin, res[1].end);
			ref.mAsset = refStr.slice(res[2].begin, res[2].end);
		}
		else
		{
			ref.mArchive = null;
			ref.mAsset = refStr;
		}

		if (queryStr != null)
		{
			ref.mVars = this.System.decodeVars(queryStr);
		}
		else
		{
			ref.mVars = null;
		}

		return ref;
	}

	function isCataloged()
	{
		return this.GetAssetArchive(this.mAsset) != null;
	}

	function getArchive()
	{
		return this.mArchive;
	}

	function setArchive( archive )
	{
		if (archive != null && typeof archive != "string")
		{
			throw this.Exception("Invalid archive: " + archive);
		}

		this.mArchive = archive;
		this.mShortName = null;
	}

	function getAsset()
	{
		return this.mAsset;
	}

	function setAsset( asset )
	{
		if (typeof asset != "string")
		{
			throw this.Exception("Invalid asset: " + asset);
		}

		this.mAsset = asset;
		this.mShortName = null;
	}

	function getShortestAssetName()
	{
		if (this.mShortName == null)
		{
			this.mShortName = this.GetShortestAssetName(this.mAsset);

			if (this.mVars && this.mVars.len() > 0)
			{
				this.mShortName += "?" + this.System.encodeVars(this.mVars);
			}
		}

		return this.mShortName;
	}

	function getVars()
	{
		return this.mVars;
	}

	function setVars( vars )
	{
		if (typeof vars == "string")
		{
			vars = this.System.decodeVars(vars);
		}
		else if (vars != null && typeof vars != "table")
		{
			throw this.Exception("Vars must be null, a table, or a string-encoded table");
		}

		this.mVars = vars == null ? null : clone vars;
		this.mShortName = null;
	}

	function dup()
	{
		return this.AssetReference("" + this);
	}

	function _tostring()
	{
		local str = "";

		if (this.mArchive != null)
		{
			str += this.mArchive;
		}

		if (this.mAsset != null)
		{
			if (str.len() > 0)
			{
				str += "#";
			}

			str += this.mAsset;
		}

		if (this.mVars != null && this.mVars.len() > 0)
		{
			if (str.len() > 0)
			{
				str += "?";
			}

			str += this.System.encodeVars(this.mVars);
		}

		return str;
	}

	function _cmp( other )
	{
		return this._compare(this, other);
	}

	static function _compare( a, b )
	{
		local astr = "" + a;
		local bstr = "" + b;

		if (astr < bstr)
		{
			return -1;
		}

		if (astr > bstr)
		{
			return 1;
		}

		return 0;
	}

	mArchive = null;
	mAsset = null;
	mVars = null;
	mShortName = null;
}

function GetAssetCompletions( searchStr )
{
	local vars;
	local pos = searchStr.find("?");

	if (pos != null)
	{
		vars = this.System.decodeVars(searchStr.slice(pos + 1));
		searchStr = searchStr.slice(0, pos);
	}

	searchStr = searchStr.tolower();
	local results = [];
	local asset;
	local archive;

	foreach( asset, archive in ::ComponentIndex )
	{
		if (searchStr.len() == 0 || asset.tolower().find(searchStr) != null)
		{
			if (archive == "")
			{
				archive = asset;
			}

			results.append(this.AssetReference(archive, asset, vars));
		}
	}

	local value;

	foreach( asset, value in ::Vegetation )
	{
		if (searchStr.len() == 0 || asset.tolower().find(searchStr) != null)
		{
			results.append(this.AssetReference("", asset, vars));
		}
	}

	results.sort(this.AssetReference._compare);
	return results;
}

function GetAssetArchive( asset )
{
	if (typeof asset == "array")
	{
		local archives = [];

		foreach( a in asset )
		{
			local archive = this.GetAssetArchive(a);

			if (archive != null)
			{
				this.Util.appendUnique(archives, archive);
			}
		}

		return archives;
	}

	local origAsset = asset;

	if (this.IsInstanceOf(asset, this.AssetReference))
	{
		asset = asset.getAsset();
	}

	if (!asset)
	{
		return null;
	}

	if (asset in ::Vegetation)
	{
		return "";
	}

	foreach( index in [
		::ComponentIndex,
		::ClothingIndex,
		::AttachableIndex,
		::CreatureIndex
	] )
	{
		if (asset in index)
		{
			local a = index[asset];

			if (a == "")
			{
				a = asset;
			}

			return a;
		}
	}

	if (this.IsInstanceOf(origAsset, this.AssetReference))
	{
		return origAsset.getArchive();
	}

	return null;
}

function GetShortestAssetName( asset )
{
	local archive = this.GetAssetArchive(asset);

	if (archive == null)
	{
		return asset;
	}

	local list;
	local ext = this.File.extension(asset);
	local name = this.File.basename(asset, "." + ext);
	local pos = name.find("-");

	if (pos != null)
	{
		local typePrefix = name.slice(0, pos + 1);
		name = name.slice(pos + 1);

		if (this.GetAssetCompletions(name).len() == 1)
		{
			return name;
		}

		name = typePrefix + name;
	}

	if (this.GetAssetCompletions(name).len() == 1)
	{
		return name;
	}

	name += "." + ext;

	if (this.GetAssetCompletions(name).len() == 1)
	{
		return name;
	}

	return asset;
}

function getArchiveDependencies( vArchive )
{
	if (vArchive in ::AssetDependencies)
	{
		return ::AssetDependencies[vArchive];
	}

	return [];
}

class this.ContentRequest extends this.MessageBroadcaster
{
	mMedia = "";
	mMediaPath = "";
	mLocalHandle = null;
	mPkgName = null;
	mPkgCallback = null;
	mError = false;
	mDeps = null;
	mAssetCallbacks = null;
	mPriority = 0;
	mRequestTime = 0;
	mLoader = null;
	mState = null;
	constructor( contentLoader, media, priority )
	{
		this.MessageBroadcaster.constructor();
		this.mLoader = contentLoader;
		this.mRequestTime = this.System.currentTimeMillis();
		this.mState = "PENDING";
		this.mMedia = media;
		this.mPriority = priority;
		this.mError = false;

		if (this.mMedia == null)
		{
			return;
		}
		
		if (this.Util.isDevMode())
		{
			local pos = media.find("-");

			if (pos != null)
			{
				media = media.slice(0, pos) + "/" + media;
			}

			this.mMediaPath = "../../Media/" + media;
		}
		else
		{
			this.mMediaPath = "Media/" + media + ".car";
		}
	}

	function isRequired()
	{
		return this.mPriority >= this.ContentLoader.PRIORITY_REQUIRED;
	}

	function upgradePriority( priority )
	{
		if (this.mState == "FETCHEDONLY" && priority >= this.ContentLoader.PRIORITY_LOW)
		{
			this.mState = "FETCHED";
		}

		if (this.mPriority < priority)
		{
			this.mPriority = priority;
		}

		if (this.mDeps)
		{
			foreach( dep in this.mDeps )
			{
				dep.upgradePriority(priority);
			}
		}
	}

	function downgradePriority( priority )
	{
		if (this.mPriority <= priority)
		{
			return;
		}

		this.mPriority = priority;
	}

	function hasDep( media )
	{
		if (this.mDeps == null)
		{
			return false;
		}

		foreach( dep in this.mDeps )
		{
			if (dep.mMedia == media)
			{
				return true;
			}
		}

		return false;
	}

	function addDep( depReq )
	{
		if (depReq.mMedia == this.mMedia)
		{
			throw "Something is wrong.";
		}

		if (this.mDeps == null)
		{
			this.mDeps = [];
		}
		else
		{
			foreach( p in this.mDeps )
			{
				if (p == depReq)
				{
					return;
				}
			}
		}

		depReq.addListener(this);
		this.mDeps.append(depReq);
	}

	function addAssetCallback( asset, callback )
	{
		local pair = [
			asset,
			callback
		];

		if (this.mState == "ERROR" || this.mState == "LOADED")
		{
			this._fireAssetCallback(pair);
			return;
		}

		if (this.mAssetCallbacks == null)
		{
			this.mAssetCallbacks = [];
		}

		this.mAssetCallbacks.append(pair);
	}

	function _fireAssetCallback( pair )
	{
		local asset = pair[0];

		try
		{
			local callback = pair[1];

			if (this.mState == "ERROR")
			{
				if ("onAssetError" in callback)
				{
					callback.onAssetError(asset, this.mError);
				}
				else
				{
					this.IGIS.error("Error loading " + asset + ": " + this.mError);
				}
			}
			else
			{
				callback.onAssetComplete(asset);
			}
		}
		catch( err )
		{
			this.log.error("Error during " + asset + " asset callback: " + err);
		}
	}

	function raiseAssetCallbacks()
	{
		if (this.mAssetCallbacks)
		{
			foreach( pair in this.mAssetCallbacks )
			{
				this._fireAssetCallback(pair);
			}

			this.mAssetCallbacks = null;
		}
	}

	function raisePackageCallback()
	{
		if (this.mPkgCallback)
		{
			if (this.mState == "ERROR")
			{
				this.log.debug("ContentLoader - Package error: " + this.mPkgName + " : " + this.mError);

				if (this.System.isRemoteDebugging())
				{
					if ("onPackageError" in this.mPkgCallback)
					{
						this.mPkgCallback.onPackageError(this.mPkgName, this.mError);
					}
				}
				else
				{
					try
					{
						this.mPkgCallback.onPackageError(this.mPkgName, this.mError);
					}
					catch( err )
					{
						this.log.error("Error during " + this.mPkgName + ".onPackageError callback: " + err);
					}
				}
			}
			else
			{
				this.log.debug("ContentLoader - Package done: " + this.mPkgName);

				if (this.System.isRemoteDebugging())
				{
					this.mPkgCallback.onPackageComplete(this.mPkgName);
				}
				else
				{
					try
					{
						this.mPkgCallback.onPackageComplete(this.mPkgName);
					}
					catch( err )
					{
						this.log.error("Error during " + this.mPkgName + ".onPackageComplete callback: " + err);
					}
				}
			}
		}
	}

	function _update( req, error )
	{
		this.log.debug("ContentLoader - Notifying " + this + " that " + req + (error ? " had an error: " + error : " is done"));

		if (req == this)
		{
			this.mError = error;
		}
		else
		{
			req.removeListener(this);
		}

		local depErrors = "";
		local depsPending = 0;

		if (this.mDeps)
		{
			foreach( dep in this.mDeps )
			{
				if (dep.mState == "ERROR")
				{
					depErrors += dep.mMedia + ": " + dep.mError + "\n";
				}
				else if (dep.mState != "LOADED")
				{
					depsPending++;
				}
			}
		}

		if (this.mError == false && (depsPending > 0 || this.mState != "LOADING" && this.mState != "FETCHING"))
		{
			return;
		}

		if (this.mError)
		{
			this.mState = "ERROR";
		}
		else if (depErrors != "")
		{
			this.mState = "ERROR";
			this.mError = "dependencies failed to load: " + depErrors;
		}
		else
		{
			this.mState = "LOADED";
		}

		this.raiseAssetCallbacks();

		if (this.mState == "ERROR")
		{
			this.broadcastMessage("onError", this, this.mError);
		}
		else
		{
			this.broadcastMessage("onComplete", this);
		}

		if (this.mPkgName)
		{
			if (this.mPkgName in this.mLoader.mPackages)
			{
				delete this.mLoader.mPackages[this.mPkgName];
			}

			this.raisePackageCallback();
		}
	}

	function onComplete( req )
	{
		this._update(req, false);
	}

	function onError( req, error )
	{
		this._update(req, error);
	}

	function fetch()
	{
		this.mState = "FETCHING";
		this.log.debug("NOW FETCHING: " + this.mMediaPath);
		print("ICE! NOW FETCHING " + this.mMediaPath + "\n");
		return this._cache.fetch(this.mMediaPath);
	}

	function hasPendingDependencies()
	{
		if (this.mDeps == null)
		{
			return false;
		}

		foreach( d in this.mDeps )
		{
			if (d.isLoaded() == false)
			{
				return true;
			}
		}

		return false;
	}

	function load()
	{
		this.log.debug("LOADING PACKAGE: " + this.mMediaPath);
		this.mState = "LOADING";
		print("ICE! LOADING " + this.mMediaPath + "\n");
		this.Composition.load(this.mMediaPath, this.mLocalHandle);
	}

	static function comparePriority( a, b )
	{
		if (a.mPriority < b.mPriority)
		{
			return -1;
		}

		if (a.mPriority > b.mPriority)
		{
			return 1;
		}

		if (a.mRequestTime < b.mRequestTime)
		{
			return -1;
		}

		if (a.mRequestTime > b.mRequestTime)
		{
			return 1;
		}

		return 0;
	}

	function isLoaded()
	{
		return this.mState == "LOADED" || this.mState == "ERROR";
	}

	function isFetched()
	{
		return this.mState == "FETCHED" || this.mState == "ERROR" || this.mState == "FETCHEDONLY";
	}

	function getState()
	{
		return this.mState;
	}

	function getPriority()
	{
		return this.mPriority;
	}

	function _tostring()
	{
		if (this.mPkgName)
		{
			return "{" + this.mPkgName + "}";
		}

		return this.mMedia;
	}

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
		this.log.debug("Notifing Scene Manager that page is now available at " + this.mX + ", " + this.mZ);
		this._scene.notifyPageAvailable(this.mX, this.mZ);
	}

	function onPackageError( name, error )
	{
		this.log.debug("Unable to load terrain page " + this.mX + ", " + this.mZ + ": " + error);
		this._scene.notifyPageAvailable(this.mX, this.mZ);
	}

}

class this.ContentLoader 
{
	static PRIORITY_REQUIRED = 3000;
	static PRIORITY_NORMAL = 2000;
	static PRIORITY_LOW = 1000;
	static PRIORITY_FETCH = 0;
	mProgressCurrent = 0;
	mProgressTotal = 0;
	mModalLoadScreen = false;
	mRequests = null;
	mPackages = null;
	mRequestsPending = 0;
	mRequiredRequestsPending = 0;
	mLoadingRequests = null;
	mFetchingRequest = null;
	mCapture = false;
	mCapturedRequests = null;
	constructor()
	{
		this.mRequests = {};
		this.mPackages = {};
		this.mLoadingRequests = {};
		this._cache.addListener(this);
		this._root.addListener(this);
		this._scene.setPageLoadingCallback(this);
	}

	function isLoaded( media )
	{
		if (typeof media == "string")
		{
			media = [
				media
			];
		}

		if (media == null)
		{
			return true;
		}

		foreach( m in media )
		{
			if (m == "")
			{
				continue;
			}

			if (!(m in this.mRequests))
			{
				return false;
			}

			if (this.mRequests[m].isLoaded() == false)
			{
				return false;
			}
		}

		return true;
	}

	function startCaptureRequests()
	{
		this.mCapture = true;
		this.mCapturedRequests = {};
	}

	function stopCaptureRequests()
	{
		if (this.mCapture)
		{
			local result = "\nCaptured = [\n";

			foreach( n, r in this.mCapturedRequests )
			{
				result += "          \"" + n + "\",\n";
			}

			result += "          ];\n";
			this.log.debug("----------------------------------------------");
			this.log.debug("Content Loader Capture Results");
			this.log.debug("----------------------------------------------");
			this.log.debug(result);
			this.log.debug("----------------------------------------------");
		}

		this.mCapture = false;
		this.mCapturedRequests = null;
	}

	function load( media, priority, ... )
	{
		local pkgName;
		local callback;

		if (vargc > 0)
		{
			pkgName = vargv[0];
		}

		if (vargc > 1)
		{
			callback = vargv[1];
		}

		this.log.debug("Content loader load called on: " + media + " priority " + priority + " pkgName " + pkgName);

		if (typeof media != "array")
		{
			media = [
				media
			];
		}

		local mediaToLoad = [];

		foreach( aom in media )
		{
			local aref = this.Util.makeAssetRef(aom, false);
			local m;

			if (aref)
			{
				m = aref.getArchive();

				if (this.mCapture && m != "")
				{
					this.mCapturedRequests[m] <- true;
				}
			}
			else if (typeof aom == "string")
			{
				m = aom;
			}
			else
			{
				throw this.Exception("Invalid media: " + aom);
			}

			if (m in ::_ArchiveAlias)
			{
				local alias = ::_ArchiveAlias[m];

				if (typeof alias == "array")
				{
					local _m;

					foreach( _m in alias )
					{
						mediaToLoad.append(_m);
					}
				}
				else
				{
					mediaToLoad.append(alias);
				}
			}
			else
			{
				mediaToLoad.append(m);
			}
		}

		local pkgReq;

		if (pkgName)
		{
			if (pkgName in this.mPackages)
			{
				this.log.debug("Package : " + pkgName + " already exists in packages list");
				pkgReq = this.mPackages[pkgName];

				if (pkgReq.mPkgCallback == null)
				{
					pkgReq.mPkgCallback = callback;
				}
			}
			else if (pkgName in this.mRequests)
			{
				this.log.debug("Package : " + pkgName + " already exists in requests list");
				pkgReq = this.mRequests[pkgName];
				pkgReq.mPkgName = pkgName;
				pkgReq.mPkgCallback = callback;
				this.mPackages[pkgName] <- pkgReq;
			}
			else
			{
				this.log.debug("New content request for " + pkgName);
				pkgReq = this.ContentRequest(this, null, priority);
				pkgReq.mPkgName = pkgName;
				pkgReq.mPkgCallback = callback;
				this.mPackages[pkgName] <- pkgReq;
			}

			pkgReq.upgradePriority(priority);
			local newMediaToLoad = [];

			foreach( m in mediaToLoad )
			{
				if (!pkgReq.hasDep(m))
				{
					newMediaToLoad.append(m);
				}
			}

			mediaToLoad = newMediaToLoad;
		}

		foreach( m in mediaToLoad )
		{
			if (m == "")
			{
				continue;
			}

			if (m.len() == 0)
			{
				continue;
			}

			local req = this._getRequest(m, priority);

			if (pkgReq)
			{
				pkgReq.addDep(req);
			}
		}

		if (pkgReq && !pkgReq.mMedia)
		{
			pkgReq.mState = "LOADING";
			pkgReq.onComplete(pkgReq);
		}

		return pkgReq;
	}

	function loadAsset( asset, callback, ... )
	{
		local priority = this.PRIORITY_LOW;

		if (vargc > 0)
		{
			priority = vargv[0];
		}

		local assetRef = this.Util.makeAssetRef(asset, true);
		local media = assetRef.getArchive();

		if (this.mCapture && media != "")
		{
			this.mCapturedRequests[media] <- true;
		}

		if (media in ::_ArchiveAlias)
		{
			local pkgReq;

			if (media in this.mPackages)
			{
				pkgReq = this.mPackages[media];
			}
			else
			{
				pkgReq = this.load(media, priority, media);
			}

			pkgReq.addAssetCallback(assetRef, callback);
			return;
		}

		if (assetRef.getArchive() == "")
		{
			if (callback)
			{
				callback.onAssetComplete(assetRef);
			}

			return;
		}

		local req = this._getRequest(assetRef.getArchive(), priority);
		req.upgradePriority(priority);

		if (callback)
		{
			req.addAssetCallback(assetRef, callback);
		}
	}

	function getLoadingPackage( name )
	{
		if (name in this.mPackages)
		{
			return this.mPackages[name];
		}

		return null;
	}

	function _getRequest( media, priority )
	{
		if (media in this.mRequests)
		{
			local req = this.mRequests[media];
			req.upgradePriority(priority);
			return req;
		}

		local req = this.ContentRequest(this, media, priority);
		this.mRequests[media] <- req;
		this.mRequestsPending++;

		if (req.isRequired())
		{
			this.mRequiredRequestsPending++;
		}

		if (media in ::AssetDependencies)
		{
			foreach( m in ::AssetDependencies[media] )
			{
				if (m == "")
				{
					continue;
				}

				this.log.debug("Pre-fetching dependency of " + media + ", " + m);

				if (m in ::_ArchiveAlias)
				{
					local alias = ::_ArchiveAlias[m];

					if (typeof alias == "array")
					{
						local _m;

						foreach( _m in alias )
						{
							if (_m == "")
							{
								continue;
							}

							req.addDep(this._getRequest(_m, priority));
						}
					}
					else
					{
						if (alias == "")
						{
							continue;
						}

						req.addDep(this._getRequest(alias, priority));
					}
				}
				else
				{
					req.addDep(this._getRequest(m, priority));
				}
			}
		}

		return req;
	}

	function _getSortedQueue( state )
	{
		local queue = [];
		this.mRequiredRequestsPending = 0;

		foreach( media, req in this.mRequests )
		{
			if (req.mState == state)
			{
				queue.append(req);

				if (req.isRequired())
				{
					this.mRequiredRequestsPending++;
				}
			}
		}

		if (queue.len() > 0)
		{
			queue.sort(this.ContentRequest.comparePriority);
		}

		this.mRequestsPending = queue.len();
		return queue;
	}

	function _fetchNext()
	{
		if (this.mFetchingRequest != null)
		{
			return;
		}

		local queue = this._getSortedQueue("PENDING");

		if (queue.len() == 0)
		{
			return;
		}

		this.log.debug("FETCHING NEXT...");
		this.mFetchingRequest = queue.pop();

		if (this.mFetchingRequest.fetch() == false)
		{
			this.log.debug("ERROR: Could not fetch " + this.mFetchingRequest);
			this.mFetchingRequest = null;
		}
	}

	function _findReadyRequest( queue )
	{
		foreach( req in queue )
		{
			if (req.hasPendingDependencies() == false)
			{
				return req;
			}
		}

		return null;
	}

	function _loadNext()
	{
		local queue = this._getSortedQueue("FETCHED");

		if (queue.len() == 0)
		{
			return;
		}

		local req = this._findReadyRequest(queue);

		if (req == null)
		{
			return;
		}

		this.mLoadingRequests["{" + req.mMediaPath + "}"] <- req;
		req.load();
	}

	function onStart( media )
	{
	}

	function getUnloadedRequestCount( vRequiredOnly )
	{
		local total = 0;

		if (vRequiredOnly)
		{
			return this.mRequiredRequestsPending;
		}
		else
		{
			return this.mRequestsPending;
		}

		return total;
	}

	function setLoadScreenMode( modal )
	{
		local resetProgress = false;

		if (modal && !this.mModalLoadScreen)
		{
			resetProgress = true;
		}
		else if (!modal && this.mModalLoadScreen)
		{
			resetProgress = true;
		}

		if (resetProgress)
		{
			this.mProgressTotal = this.getUnloadedRequestCount(modal);
			this.mProgressCurrent = 0;
		}

		this.mModalLoadScreen = modal;
	}

	function resetProgress( required )
	{
		this.mProgressTotal = this.getUnloadedRequestCount(required);
		this.mProgressCurrent = 0;
	}

	function updateLoadScreen()
	{
		::LoadScreen.update(this.mProgressCurrent, this.mProgressTotal);
	}

	function getRequestStatusDebug()
	{
		local str = "";

		foreach( r in this.mRequests )
		{
			if (!r.isLoaded() && r.mPriority >= this.PRIORITY_REQUIRED)
			{
				str += "   - " + r + " : " + r.mState + "\n";
			}
		}

		return str;
	}

	function onProgress( media, bytesLoaded, bytesTotal )
	{
		::LoadScreen.downloadProgress(bytesLoaded, bytesTotal, media);
	}

	function onComplete( media, handle )
	{
		this.log.debug("FETCH HAS COMPLETED: " + media);
		local req = this.mFetchingRequest;
		this.mFetchingRequest = null;

		if (req)
		{
			if (req != null && media != req.mMediaPath)
			{
				throw this.Exception("Internal state error in ContentLoader fetch: " + media + ", " + req.mMediaPath);
			}

			if (req.mPriority >= this.PRIORITY_LOW)
			{
				req.mState = "FETCHED";
			}
			else
			{
				req.mState = "FETCHEDONLY";
				req.raiseAssetCallbacks();
				req.raisePackageCallback();
				this.mProgressCurrent++;
			}

			req.mLocalHandle = handle;
		}
	}

	function onError( media, error )
	{
		local req;

		if (this.mFetchingRequest != null && this.mFetchingRequest.mMediaPath == media)
		{
			req = this.mFetchingRequest;
			this.mFetchingRequest = null;
		}
		else if (media in this.mLoadingRequests)
		{
			local req = this.mLoadingRequests[media];
			delete this.mLoadingRequests[media];
		}
		else
		{
			this.log.warn("Error loading unrequested media file " + media);
		}

		if (req == null)
		{
			return;
		}

		req.onError(req, error);
		this.mProgressCurrent++;
	}

	function onCompositionLoaded( name )
	{
		this.log.debug("Composition Loaded: " + name);

		if (!(name in this.mLoadingRequests))
		{
			this.log.warn(name + " is not a known loading request.");
			return;
		}

		local req = this.mLoadingRequests[name];
		delete this.mLoadingRequests[name];
		this.mRequestsPending--;

		if (req.isRequired())
		{
			this.mRequestsPending--;
		}

		this.mProgressCurrent++;

		if (req != null)
		{
			req.onComplete(req);
		}
		else
		{
			this.log.warn("Req == NULL in onCompositionLoaded: " + name);
		}
	}

	function onRequireTerrainPage( x, z, pattern )
	{
		if (pattern != "")
		{
			this.log.debug("Required Page Event Encountered " + x + ", " + z + ", (" + pattern + ")");
			this.load(pattern + "_x" + x + "y" + z, this.PRIORITY_REQUIRED, pattern + "_x" + x + "y" + z + "_pkg", this.TerrainLoadedCallback(x, z));
		}
		else
		{
			this.log.error("Terrain page event received, but no terrain pattern was provided.");
		}
	}

	function onNearbyTerrainPage( x, z, pattern )
	{
		if (pattern != "")
		{
			this.log.debug("Nearby Page Event Encountered " + x + ", " + z + ", (" + pattern + ")");
			this.load(pattern + "_x" + x + "y" + z, this.PRIORITY_FETCH, pattern + "_x" + x + "y" + z + "_pkg", this.TerrainLoadedCallback(x, z));
		}
		else
		{
			this.log.error("Terrain page event received, but no terrain pattern was provided.");
		}
	}

	function onEnterFrame()
	{
		this._fetchNext();
		this._loadNext();
	}

}

