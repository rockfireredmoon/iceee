this.Util <- {};
this._random <- this.Random();
class this.PackageWaiter 
{
	mMedia = null;
	mWaitCount = 0;
	mErrorCount = 0;
	mCallback = null;
	constructor( media, callback )
	{
		this.mMedia = media;
		this.mWaitCount = 1;
		this.mCallback = callback;
	}

	function isReady()
	{
		return this.mWaitCount <= 0;
	}

	function hasErrors()
	{
		return this.mErrorCount > 0;
	}

	function _checkComplete()
	{
		if (this.mWaitCount <= 0)
		{
			if (this.mErrorCount > 0)
			{
				if (this.mCallback != null && "onWaitErrored" in this.mCallback)
				{
					this.mCallback.onWaitErrored(this);
				}
			}
			else if (this.mCallback != null && "onWaitComplete" in this.mCallback)
			{
				this.mCallback.onWaitComplete(this);
			}
		}
	}

	function onPackageComplete( pkg )
	{
		this.mWaitCount--;

		if (this.mCallback != null && "onPackageComplete" in this.mCallback)
		{
			this.mCallback.onPackageComplete(pkg);
		}

		this._checkComplete();
	}

	function onPackageError( pkg, error )
	{
		this.mErrorCount++;
		this.mWaitCount--;

		if (this.mCallback != null && "onPackageError" in this.mCallback)
		{
			this.mCallback.onPackageError(pkg, error);
		}

		this._checkComplete();
	}

	function done()
	{
	}

}

this._nextPackageNameID <- 0;
this.Util.waitForAssets <- function ( assets, callback, ... )
{
	local priority = vargc > 0 ? vargv[0] : this.ContentLoader.PRIORITY_LOW;

	if (typeof assets != "array")
	{
		assets = [
			assets
		];
	}

	local waiter = this.PackageWaiter(assets, callback);
	this._contentLoader.load(assets, priority, "Package " + this._nextPackageNameID++, waiter);
	return waiter;
};
this.Util.isDevMode <- function ()
{
	local test = this._args;
	return "dev" in this._args;
};
this.Util.hasPermission <- function ( permission )
{
	return this.Util.isDevMode() || ::_accountPermissionGroup == "admin";
};
this.Util.makeAssetRef <- function ( asset, throwOnError )
{
	local ref;

	if (typeof asset == "instance" && (asset instanceof this.AssetReference))
	{
		ref = asset.dup();
	}
	else if (typeof asset == "string")
	{
		ref = this.AssetReference(asset);
	}
	else if (throwOnError)
	{
		throw this.Exception("Invalid asset: " + asset);
	}
	else
	{
		return null;
	}

	if (ref.getArchive() == null)
	{
		local m = this.GetAssetArchive(ref.getAsset());

		if (m == null)
		{
			if (!throwOnError)
			{
				return null;
			}

			this.IGIS.error("Unknown asset: " + asset);
			throw this.Exception("Cannot determine archive for asset: " + asset);
		}

		ref.setArchive(m);
	}

	return ref;
};
this.Util.fuzzyCmp <- function ( v1, v2 )
{
	return this.Math.abs(v1 - v2) < 0.0099999998;
};
this.Util.fuzzyCmpVector3 <- function ( vec1, vec2 )
{
	return this.fuzzyCmp(vec1.x, vec2.x) && this.fuzzyCmp(vec1.y, vec2.y) && this.fuzzyCmp(vec1.z, vec2.z);
};
this.Util.fuzzyCmpQuaternion <- function ( q1, q2 )
{
	return this.fuzzyCmp(q1.x, q2.x) && this.fuzzyCmp(q1.y, q2.y) && this.fuzzyCmp(q1.z, q2.z) && this.fuzzyCmp(q1.w, q2.w);
};
this.Util.convertHTMLtoText <- function ( htmlString )
{
	local strToConvert = {
		[0] = {
			oldStr = "&",
			newStr = "&amp;"
		},
		[1] = {
			oldStr = "<",
			newStr = "&lt;"
		},
		[2] = {
			oldStr = ">",
			newStr = "&gt;"
		}
	};

	foreach( rule in strToConvert )
	{
		htmlString = this.Util.replace(htmlString, rule.oldStr, rule.newStr);
	}

	return htmlString;
};
this.Util.tableKeys <- function ( t, ... )
{
	local keys = [];
	local k;
	local v;

	foreach( k, v in t )
	{
		keys.append(k);
	}

	if (vargc > 1 && vargv[1])
	{
		if (("parent" in t) && t.parent)
		{
			local parentKeys = this.Util.tableKeys(t.parent, false, true);

			foreach( x in parentKeys )
			{
				local found = false;

				foreach( y in keys )
				{
					if (y == x)
					{
						found = true;
					}
				}

				if (!found)
				{
					keys.append(x);
				}
			}
		}
	}

	if (vargc > 0 && vargv[0])
	{
		keys.sort();
	}

	return keys;
};
this.Util.bubbleSort <- function ( list, comparator )
{
	local len = list.len();
	local found;

	do
	{
		found = false;
		local x;

		for( x = 0; x < len - 1; x++ )
		{
			local v0 = list[x + 0];
			local v1 = list[x + 1];
			local result = comparator(v0, v1);

			if (result > 0)
			{
				list[x + 0] = v1;
				list[x + 1] = v0;
				found = true;
			}
		}
	}
	while (found == true);
};
this.Util.convertToType <- function ( value, name )
{
	if (typeof value == name)
	{
		return value;
	}

	switch(name)
	{
	case "string":
		return value.tostring();

	case "integer":
		return value.tointeger();

	case "float":
		return value.tofloat();

	case "bool":
		if (typeof value == "string")
		{
			return this.Util.atob(value);
		}

		return value.tointeger() != 0;
	}

	throw this.Exception("Value of type " + typeof value + " cannot be converted to type " + name);
};
this.Util.join <- function ( array, separator )
{
	local str = "";
	local count = 0;
	local e;

	foreach( e in array )
	{
		if (count > 0)
		{
			str += separator;
		}

		str += "" + e;
		count += 1;
	}

	return str;
};
this.Util.replace <- function ( str, value, newvalue )
{
	local tmp = this.Util.split(str, value);
	return this.Util.join(tmp, newvalue);
};
this.Util.filterArray <- function ( array, value )
{
	local newArray = [];

	foreach( v in array )
	{
		if (v != value)
		{
			newArray.append(v);
		}
	}

	return newArray;
};
this.Util.copyArray <- function ( array )
{
	local newArray = [];

	foreach( v in array )
	{
		newArray.append(v);
	}

	return newArray;
};
this.Util.randomRange <- function ( start, end )
{
	return this.Math.lerp(start, end, this._random.nextFloat());
};
this.Util.addAssetDependencies <- function ( deps, name )
{
	if ((name in ::AssetDependencies) == false)
	{
		return;
	}

	foreach( d in ::AssetDependencies[name] )
	{
		if (this.Util.indexOf(deps, d) == null)
		{
			this.addAssetDependencies(deps, d);
			deps.push(d);
		}
	}
};
this.Util.getAssetDependencies <- function ( name )
{
	local result = [];
	this.Util.addAssetDependencies(result, name);
	return result;
};
this.Util.isInIgnoreList <- function ( name )
{
	local lowerName = name.tolower();
	local ignoreMap = ::Pref.get("chat.ignoreList");

	foreach( k, v in ignoreMap )
	{
		if (k.tolower() == lowerName)
		{
			return true;
		}
	}

	return false;
};
this.Util.stringify <- function ( array )
{
	local tmp = "[";

	foreach( i in array )
	{
		if (typeof i == "string")
		{
			tmp += "\"" + i + "\"";
		}
		else if (typeof i == "integer")
		{
			tmp += i;
		}
		else if (typeof i == "array")
		{
			tmp += this.Util.stringify(i);
		}
		else if (typeof i == "null")
		{
			tmp += "null";
		}
		else
		{
			throw this.Exception("Invalid element type in Util.stringify");
		}

		tmp += ",";
	}

	tmp += "]";
	return tmp;
};
this.Util.split <- function ( str, sep )
{
	local result = [];
	local start = 0;

	while (true)
	{
		local end = str.find(sep, start);

		if (end == null)
		{
			result.append(str.slice(start));
			break;
		}

		result.append(str.slice(start, end));
		start = end + sep.len();
	}

	return result;
};
this.Util.splitQuoteSafe <- function ( str, sep )
{
	local result = [];
	local start = 0;
	local char = 0;
	local tmp = "";
	local quote = false;

	while (start < str.len())
	{
		char = str.slice(start, start + 1);

		if (char == "\"")
		{
			quote = !quote;
		}

		if (char == sep && !quote)
		{
			result.append(tmp);
			tmp = "";
		}
		else
		{
			tmp += char;
		}

		start++;
	}

	if (tmp != "")
	{
		result.append(tmp);
	}

	return result;
};
this.Util.trim <- function ( str )
{
	return this.lstrip(this.rstrip(str));
};
this.Util.limitSignificantDigits <- function ( floatValue, significantDigits )
{
	local multiplier = 1;
	local wholeNumber = floatValue.tointeger();

	for( local i = 0; i < significantDigits; i++ )
	{
		multiplier *= 10;
	}

	local intValue = (floatValue * multiplier).tointeger();
	local newFloatValue = intValue.tofloat() / multiplier;
	return newFloatValue;
};
this.Util.hasRemainder <- function ( floatValue )
{
	return floatValue > floatValue.tointeger().tofloat();
};
this.Util.atob <- function ( str )
{
	if (!(typeof str == "string"))
	{
		return str;
	}

	str = str.tolower();

	if (str == "true" || str == "yes" || str == "on" || str == "1")
	{
		return true;
	}

	return false;
};
this.Util.tableSetOrRemove <- function ( t, key, value )
{
	if (value == null)
	{
		if (key in t)
		{
			delete t[key];
		}

		return false;
	}

	t[key] <- value;
	return true;
};
this.Util.tableSafeGet <- function ( t, key, ... )
{
	if (key in t)
	{
		return t[key];
	}

	if (vargc > 0)
	{
		return vargv[0];
	}

	return null;
};
this.Util.overrideSlots <- function ( dstTable, srcTable )
{
	if (srcTable == null)
	{
		return;
	}

	foreach( k, v in srcTable )
	{
		dstTable[k] <- v;
	}

	return dstTable;
};
this.Util.indexOf <- function ( aggregate, element )
{
	foreach( i, x in aggregate )
	{
		if (x == element)
		{
			return i;
		}
	}

	return null;
};
this.Util.removeIf <- function ( aggregate, functor )
{
	local count = 0;

	if (typeof aggregate == "array")
	{
		for( local i = 0; i < aggregate.len();  )
		{
			local v = aggregate[i];

			if (functor(v))
			{
				aggregate.remove(i);
				count++;
			}
			else
			{
				i++;
			}
		}
	}
	else if (typeof aggregate == "table")
	{
		local keys = this.Util.tableKeys(aggregate);

		foreach( k in keys )
		{
			local v = aggregate[k];

			if (functor(v))
			{
				delete aggregate[k];
				count++;
			}
		}
	}
	else
	{
		throw this.Exception("removeIf only works for array or table types");
	}

	return count;
};
this.Util.uniqueValues <- function ( collection )
{
	if (typeof collection == "table" || typeof collection == "array")
	{
		local values = {};

		foreach( key, val in collection )
		{
			values[val] <- true;
		}

		return this.Util.tableKeys(values);
	}
	else
	{
		throw this.Exception("Not a collection: " + typeof collection);
	}
};
this.Util.appendUnique <- function ( array, value )
{
	foreach( v in array )
	{
		if (v == value)
		{
			return false;
		}
	}

	array.append(value);
	return true;
};
this.Util.endsWith <- function ( str, endstr )
{
	if (typeof str != "string")
	{
		return false;
	}

	if (endstr.len() == 0)
	{
		return true;
	}

	if (str.len() < endstr.len())
	{
		return false;
	}

	return str.slice(str.len() - endstr.len()) == endstr;
};
this.Util.startsWith <- function ( str, beginstr )
{
	if (typeof str != "string")
	{
		return false;
	}

	if (beginstr.len() == 0)
	{
		return true;
	}

	if (str.len() < beginstr.len())
	{
		return false;
	}

	return str.slice(0, beginstr.len()) == beginstr;
};
this.Util.handleMoveItemBack <- function ( qa )
{
	if (qa.args[3] == "")
	{
		return;
	}

	local currentContainerName = qa.args[1];
	local slotIndexTriedToDropTo = qa.args[2].tointeger();
	local currentContainer = ::Util.findMatchingContainer(currentContainerName, slotIndexTriedToDropTo);

	if (currentContainer && currentContainerName == "eq")
	{
		slotIndexTriedToDropTo = 0;
	}

	local containerName = qa.args[3];
	local oldActionButtonIndex = qa.args[4].tointeger();
	local actionContainer = ::Util.findMatchingContainer(containerName, oldActionButtonIndex);

	if (actionContainer && currentContainer)
	{
		if (actionContainer == "eq")
		{
			oldActionButtonIndex = 0;
		}

		local oldActionButtonSlot = actionContainer.getSlotContents(oldActionButtonIndex);

		if (oldActionButtonSlot)
		{
			currentContainer.simulateActionButtonSlotDrop(oldActionButtonSlot, oldActionButtonIndex, actionContainer, slotIndexTriedToDropTo);
		}
	}
};
this.Util.findMatchingContainer <- function ( containerName, ... )
{
	local actionContainer;
	local slotIndex = -1;

	if (vargc > 0)
	{
		slotIndex = vargv[0];
	}

	if (containerName == "eq")
	{
		local eqScreen = this.Screens.get("Equipment", true);

		if (eqScreen)
		{
			actionContainer = eqScreen.findMatchingContainer(slotIndex);
		}
	}
	else if (containerName == "bag_inventory")
	{
		local inventory = this.Screens.get("Inventory", true);

		if (inventory)
		{
			actionContainer = inventory.getBagActionContainer();
		}
	}
	else if (this.Util.startsWith(containerName, "eq"))
	{
		local eqScreen = this.Screens.get("Equipment", true);

		if (eqScreen)
		{
			actionContainer = eqScreen.findMatchingContainerGivenName(containerName);
		}
	}
	else if (containerName == "inventory" || this.Util.startsWith(containerName, "inv"))
	{
		local inventory = this.Screens.get("Inventory", true);

		if (inventory)
		{
			actionContainer = inventory.getMyActionContainer();
		}
	}
	else if (containerName == "creaturetweak_inventory")
	{
		local ct = this.Screens.get("CreatureTweakScreen", true);

		if (ct)
		{
			actionContainer = ct.getActionContainer();
		}
	}
	else if (containerName == "bank" || containerName == "vault")
	{
		local vault = this.Screens.get("Vault", true);

		if (vault)
		{
			actionContainer = vault.getActionContainer();
			
		}
	}
	else if (containerName == "delivery")
	{
		local vault = this.Screens.get("Vault", true);
		if (vault)
			actionContainer = vault.getDeliveryBoxContainer();
	}
	else if (containerName == "stamps")
	{
		local vault = this.Screens.get("Vault", true);
		if (vault)
			actionContainer = vault.getStampContainer();
	}

	return actionContainer;
};
this.Util.getUpdateResizePosition <- function ( oldScreenWidth, oldScreenHeight, posX, posY, currentScreenWidth, currentScreenHeight )
{
	local newPosition = {
		x = 0,
		y = 0
	};

	if (oldScreenWidth == ::Screen.getWidth() && oldScreenHeight == ::Screen.getHeight())
	{
		newPosition.x = posX;
		newPosition.y = posY;
		return newPosition;
	}

	local DIFF = 5;
	local centerPoint = {
		x = currentScreenWidth / 2 + posX,
		y = currentScreenHeight / 2 + posY
	};
	local centerQuadrantSize = {
		width = oldScreenWidth / DIFF,
		height = oldScreenHeight / DIFF
	};
	local centerQuad = {
		x0 = (oldScreenWidth - centerQuadrantSize.width) / 2,
		y0 = (oldScreenHeight - centerQuadrantSize.height) / 2,
		x1 = (oldScreenWidth - centerQuadrantSize.width) / 2 + centerQuadrantSize.width,
		y1 = (oldScreenHeight - centerQuadrantSize.height) / 2 + centerQuadrantSize.height
	};
	local nwQuad = {
		x0 = 0,
		y0 = 0,
		x1 = oldScreenWidth / 2,
		y1 = oldScreenHeight / 2
	};
	local neQuad = {
		x0 = oldScreenWidth / 2,
		y0 = 0,
		x1 = oldScreenWidth,
		y1 = oldScreenHeight / 2
	};
	local swQuad = {
		x0 = 0,
		y0 = oldScreenHeight / 2,
		x1 = oldScreenWidth / 2,
		y1 = oldScreenHeight
	};
	local seQuad = {
		x0 = oldScreenWidth / 2,
		y0 = oldScreenHeight / 2,
		x1 = oldScreenWidth,
		y1 = oldScreenHeight
	};

	if (centerPoint.x >= centerQuad.x0 && centerPoint.x <= centerQuad.x1 && centerPoint.y >= centerQuad.y0 && centerPoint.y <= centerQuad.y1)
	{
		local screenOldCenterPoint = {
			x = oldScreenWidth / 2,
			y = oldScreenHeight / 2
		};
		local xDiff = centerPoint.x.tofloat() - screenOldCenterPoint.x.tofloat();
		local yDiff = centerPoint.y.tofloat() - screenOldCenterPoint.y.tofloat();
		newPosition.x = ::Screen.getWidth().tofloat() / 2 + xDiff.tofloat();
		newPosition.y = ::Screen.getHeight().tofloat() / 2 + yDiff.tofloat();
	}
	else if (centerPoint.x >= nwQuad.x0 && centerPoint.x <= nwQuad.x1 && centerPoint.y >= nwQuad.y0 && centerPoint.y <= nwQuad.y1)
	{
		newPosition.x = centerPoint.x;
		newPosition.y = centerPoint.y;
	}
	else if (centerPoint.x >= neQuad.x0 && centerPoint.x <= neQuad.x1 && centerPoint.y >= neQuad.y0 && centerPoint.y <= neQuad.y1)
	{
		local xDiff = centerPoint.x.tofloat() - oldScreenWidth;
		local yDiff = centerPoint.y.tofloat();
		newPosition.x = ::Screen.getWidth().tofloat() + xDiff.tofloat();
		newPosition.y = yDiff;
	}
	else if (centerPoint.x >= swQuad.x0 && centerPoint.x <= swQuad.x1 && centerPoint.y >= swQuad.y0 && centerPoint.y <= swQuad.y1)
	{
		local xDiff = centerPoint.x.tofloat();
		local yDiff = centerPoint.y.tofloat() - oldScreenHeight;
		newPosition.x = xDiff.tofloat();
		newPosition.y = ::Screen.getHeight().tofloat() + yDiff.tofloat();
	}
	else if (centerPoint.x >= seQuad.x0 && centerPoint.x <= seQuad.x1 && centerPoint.y >= seQuad.y0 && centerPoint.y <= seQuad.y1)
	{
		local xDiff = centerPoint.x.tofloat() - oldScreenWidth;
		local yDiff = centerPoint.y.tofloat() - oldScreenHeight;
		newPosition.x = ::Screen.getWidth().tofloat() + xDiff.tofloat();
		newPosition.y = ::Screen.getHeight().tofloat() + yDiff.tofloat();
	}

	newPosition.x = newPosition.x.tofloat() - currentScreenWidth / 2;
	newPosition.y = newPosition.y.tofloat() - currentScreenHeight / 2;
	newPosition.x = newPosition.x.tointeger();
	newPosition.y = newPosition.y.tointeger();
	return newPosition;
};
this.Util.getNodeXform <- function ( node )
{
	return {
		position = node.getPosition(),
		orientation = node.getOrientation(),
		scale = node.getScale()
	};
};
this.Util.setNodeXform <- function ( node, xform )
{
	if ("position" in xform)
	{
		node.setPosition(xform.position);
	}

	if ("orientation" in xform)
	{
		node.setOrientation(xform.orientation);
	}

	if ("scale" in xform)
	{
		node.setScale(xform.scale);
	}
};
this.Util.setNodeOpacity <- function ( node, opacity )
{
	this.Assert.isInstanceOf(node, this.SceneNode);
	local mo;

	foreach( mo in node.getAttachedObjects() )
	{
		if (mo instanceof this.Entity)
		{
			mo.setOpacity(opacity);
		}
	}

	local child;

	foreach( child in node.getChildren() )
	{
		this.Util.setNodeOpacity(child, opacity);
	}
};
this.Util.setNodeShowBoundingBox <- function ( node, value )
{
	if (node instanceof this.SceneNode)
	{
		node.showBoundingBox(value);
		local child;

		foreach( child in node.getChildren() )
		{
			this.Util.setNodeShowBoundingBox(child, value);
		}
	}
};
this.Util.getNodePositionDebug <- function ( node, ... )
{
	local depth = 0;

	if (vargc > 0)
	{
		depth = vargv[0];
	}

	local indent = "";

	for( local i = 0; i < depth; i++ )
	{
		indent += "  ";
	}

	local str = "";
	str += indent + node.getName() + " -> " + node.getPosition() + "\n";
	local mo;

	foreach( mo in node.getAttachedObjects() )
	{
		str += indent + "   [";
		str += mo.getMovableType() + ": " + mo.getName();
		str += "]\n";
	}

	local n;

	foreach( n in node.getChildren() )
	{
		str += this.Util.getNodePositionDebug(n, depth + 1);
	}

	return str;
};
this.Util.getBoneWorldPosition <- function ( entity, bone, ... )
{
	if (entity instanceof this.SceneObject)
	{
		if (bone == null || bone == "node")
		{
			return entity.getNode().getWorldPosition();
		}

		if (entity.getAssembler() == null)
		{
			return entity.getNode().getWorldPosition();
		}

		entity = entity.getAssembler().getBaseEntity(entity);
	}

	if (entity == null)
	{
		return null;
	}

	if (!(entity instanceof this.Entity))
	{
		throw this.Exception("Cannot get bone position of non-entity: " + entity);
	}

	if (bone == null)
	{
		return entity.getWorldPosition();
	}

	local tester = ::_scene.createSoundEmitter("Util::getBoneWorldPosition()_Helper");
	local pos;

	try
	{
		entity.attachObjectToBone(bone, tester);
		local node = tester.getParentNode();

		if (vargc > 0)
		{
			this.Util.setNodeXform(node, vargv[0]);
		}

		pos = node.getWorldPosition();
	}
	catch( err )
	{
		this.log.error("Error finding bone location: " + err);
		pos = entity.getParentNode().getWorldPosition();
	}

	tester.destroy();
	return pos;
};
this.Util.getFloorHeightAt <- function ( pos, fudge, flags, ... )
{
	local ignoreNode = vargc > 1 ? vargv[1] : null;
	local tmp = this.Vector3(pos.x, pos.y + fudge.tofloat(), pos.z);
	local hits = this._scene.rayQuery(tmp, this.Vector3().NEGATIVE_UNIT_Y, flags, true, false, 1, ignoreNode);

	if (hits.len() == 0)
	{
		tmp.y -= fudge.tofloat();
		hits = this._scene.rayQuery(tmp, this.Vector3().UNIT_Y, flags, true, true, 1, ignoreNode);

		if (hits.len() == 0)
		{
			return null;
		}
	}
	else
	{
		hits[0].t = fudge - hits[0].t;
	}

	if (vargc > 0 && vargv[0])
	{
		return hits[0];
	}

	return hits[0].pos.y;
};
this.Util.getWaterHeightAt <- function ( pos )
{
	local tpos = this.Util.getTerrainPageIndex(pos);

	if (tpos == null)
	{
		return 0;
	}

	return this._scene.getTerrainWaterElevation(tpos.x, tpos.z);
};
this.Util.pointOnFloor <- function ( position, ... )
{
	local ignoreNode = vargc > 0 ? vargv[0] : null;
	local y = this.Util.getFloorHeightAt(position, 0.75, this.QueryFlags.FLOOR, false, ignoreNode);

	if (y == null)
	{
		return position;
	}

	return this.Vector3(position.x, y, position.z);
};
this.Util.safePointOnFloor <- function ( position, ... )
{
	local sceneryIndex = this._sceneObjectManager.getSceneryPageIndex(position);

	if (sceneryIndex)
	{
		local sceneryReady = ::_sceneObjectManager.isSceneryPageReady(sceneryIndex.x, sceneryIndex.z);

		if (sceneryReady)
		{
			local ignoreNode = vargc > 0 ? vargv[0] : null;
			local y = this.Util.getFloorHeightAt(position, 2.5, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false, ignoreNode);
			local tempPos = this.Vector3(position.x, position.y, position.z);
			tempPos.y += 5.0;
			local finalGroundPos = position;
			local box = this.Vector3(2.0, 4.0, 2.0);
			local groundTestDir = this.Vector3(0.0, -5000.0, 0.0);
			local groundTest = this._scene.sweepBox(box, tempPos, tempPos + groundTestDir, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, false);

			if (groundTest.distance < 1.0)
			{
				finalGroundPos = position + groundTestDir * groundTest.distance;
			}
			else
			{
				if (y != null)
				{
					position.y = y;
				}

				return position;
			}

			if (y != null && y > finalGroundPos.y)
			{
				finalGroundPos.y = y;
			}

			return this.Vector3(position.x, finalGroundPos.y, position.z);
		}
	}

	return position;
};
this.Util.getNearestNonCollidingPoint <- function ( position, searchRadius )
{
	local step = 10;
	local foundFreeSpot = false;
	local vectorTests = [
		this.Vector3().UNIT_X,
		this.Vector3().UNIT_Z,
		this.Vector3().NEGATIVE_UNIT_X,
		this.Vector3().NEGATIVE_UNIT_Z
	];
	local freePosition = this.Vector3(0, 0, 0);

	for( local i = 0; i < vectorTests.len() && !foundFreeSpot; i++ )
	{
		for( local j = step; j <= searchRadius && !foundFreeSpot; j += step )
		{
			local vec3Additive = vectorTests[i] * this.Vector3(j, 0, j);
			local vec3TestPoint = position + vec3Additive;
			local box = this.Vector3(2.0, 4.0, 2.0);
			local Intersection = this._scene.sweepBox(box, vec3TestPoint, vec3TestPoint + vectorTests[i], this.QueryFlags.BLOCKING, true, 0.5);

			if (Intersection.distance > 0.0099999998 && this.isSlopeTooSteepToStandOn(vec3TestPoint) == false)
			{
				foundFreeSpot = true;
				freePosition = vec3TestPoint;
			}
		}
	}

	return freePosition;
};
this.Util.isSlopeTooSteepToStandOn <- function ( position )
{
	local floor = this.Util.getFloorHeightAt(position, 10.0, this.QueryFlags.FLOOR | this.QueryFlags.BLOCKING, true);

	if (floor != null)
	{
		if (floor.normal.dot(this.Vector3(0, 1, 0)) < this.gMaxSlope)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	this.log.error("Couldn\'t find a floor at position " + position);
	return false;
};
this.Util.collideAndSlide <- function ( box, stepOffset, start, dir, mask )
{
	local offset = this.Vector3(0.0, box.y + stepOffset, 0.0);
	local end = start + dir;
	local result = this._scene.sweepBox(box, start + offset, end + offset, mask, true, 0.5);
	local finalPos = start + dir * result.distance;
	local ndir = this.Vector3(dir.x, dir.y, dir.z);
	ndir.normalize();

	if (result.distance < 1.0)
	{
		finalPos += result.normal * 0.55000001 * this.Math.abs(ndir.dot(result.normal));
	}

	return {
		pos = finalPos,
		hit = result.distance < 1.0
	};
};
this.Util.rayTestNode <- function ( origin, dir, node )
{
	local res = {
		v = false
	};
	local f2 = function ( obj ) : ( origin, dir, res )
	{
		local result = this._root.objectRayTest(origin, dir, obj);

		if (result >= 0.0)
		{
			res.v = true;
			return false;
		}

		return false;
	};
	this.Util.visitMovables(node, f2);
	return res.v;
};
this.Util.visitMovables <- function ( node, func, ... )
{
	if (node instanceof this.MovableObject)
	{
		if (func(node) == true)
		{
			return;
		}
	}

	if ((node instanceof this.Entity) || (node instanceof this.SceneNode))
	{
		foreach( mo in node.getAttachedObjects() )
		{
			this.Util.visitMovables(mo, func);
		}

		if (node instanceof this.SceneNode)
		{
			local maxDepth = vargc > 0 ? vargv[0] : -1;

			if (maxDepth == 0)
			{
				return;
			}

			if (maxDepth > 0)
			{
				maxDepth -= 1;
			}

			local child;

			foreach( child in node.getChildren() )
			{
				this.Util.visitMovables(child, func, maxDepth);
			}
		}
	}
};
this.Util.visitNodes <- function ( node, func )
{
	func(node);
	local n;

	foreach( n in node.getChildren() )
	{
		this.Util.visitNodes(n, func);
	}
};
this.Util.createDecal <- function ( name, texture, size )
{
	return this._scene.createDecal(name, this._root.createMaterialUsingAliases("StandardDecal", {
		Diffuse = texture
	}), size, size);
};
this.Util.getTerrainPageIndex <- function ( pos )
{
	if (this._scene && "getTerrainPageIndex" in this._scene)
	{
		return this._scene.getTerrainPageIndex(pos);
	}

	return null;
};
this.Util.getTerrainPageName <- function ( pos )
{
	local terrain = ::_sceneObjectManager.getCurrentTerrainBase();

	if (terrain == null)
	{
		return null;
	}

	local tpos = this.Util.getTerrainPageIndex(pos);

	if (tpos == null)
	{
		return "";
	}

	return terrain + "_x" + tpos.x + "y" + tpos.z;
};
function strcasecmp( a, b )
{
	local astr = ("" + a).tolower();
	local bstr = ("" + b).tolower();

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

this.Util.getTerrainPathInfo <- function ()
{
	local base = ::_sceneObjectManager.getCurrentTerrainBase();

	if (!this.Util.startsWith(base, "Terrain-"))
	{
		throw this.Exception("Current terrain looks incorrect: " + base);
	}

	base = base.slice(8);
	local basePath = this._cache.getBaseURL();

	if (this.Util.isDevMode() == false)
	{
		throw this.Exception("Terrain path info available only in dev mode");
	}

	if (basePath.slice(0, 8) != "file:///")
	{
		throw this.Exception("Terrain path unavailble for base URL: " + basePath);
	}

	basePath = basePath.slice(8);
	basePath += "/../../Media/Terrain/Terrain-" + base + "/";
	return [
		base,
		basePath
	];
};
this.Util.toFirstCaps <- function ( string )
{
	if (this.type(string).tolower() == "string")
	{
		if (string)
		{
			local a = string.slice(0, 1);
			a = a.toupper();
			local b = "";

			if (string.len() > 1)
			{
				b = string.slice(1, string.len());
				b.tolower();
			}

			return a + b;
		}
	}
	else
	{
		this.log.error("Util.toFirstCaps - was not passed a string.");
	}
};
this.Util.strColor <- function ( col )
{
	local parts = this.split(col, " ");
	return this.Color(parts[0].tointeger(), parts[1].tointeger(), parts[2].tointeger(), parts[3].tointeger());
};
this.Util.parseHourToTimeStr <- function ( value )
{
	local hrToDay = 24;
	local hours = value;
	local timeTable = {};
	timeTable.d <- 0;
	timeTable.h <- 0;
	timeTable.d = (hours / hrToDay).tointeger();
	hours = hours % hrToDay;
	timeTable.h = hours.tointeger();
	local timeStr = "";

	if (timeTable.d == 1)
	{
		timeStr = timeTable.d + " day ";
	}
	else if (timeTable.d > 1)
	{
		timeStr = timeTable.d + " days ";
	}

	if (timeTable.h == 1)
	{
		timeStr = timeStr + timeTable.h + " hour ";
	}

	if (timeTable.h > 1)
	{
		timeStr = timeStr + timeTable.h + " hours ";
	}

	return timeStr;
};
this.Util.parseMilliToTimeStr <- function ( value )
{
	local timeTable = this.Util.paraseMiliToTable(value);
	local timeStr = "";

	if (timeTable.d == 1)
	{
		timeStr = timeTable.d + " day ";
	}
	else if (timeTable.d > 1)
	{
		timeStr = timeTable.d + " days ";
	}

	if (timeTable.h == 1)
	{
		timeStr = timeStr + timeTable.h + " hr ";
	}
	else if (timeTable.h > 1)
	{
		timeStr = timeStr + timeTable.h + " hrs ";
	}

	if (timeTable.m == 1)
	{
		timeStr = timeStr + timeTable.m + " min ";
	}
	else if (timeTable.m > 1)
	{
		timeStr = timeStr + timeTable.m + " mins ";
	}

	if (timeTable.s == 1)
	{
		timeStr = timeStr + timeTable.s + " sec ";
	}
	else if (timeTable.s > 1)
	{
		timeStr = timeStr + timeTable.s + " secs ";
	}

	return timeStr;
};
this.Util.parseSecToTimeStr <- function ( value )
{
	local timeTable = this.Util.paraseSecToTable(value);
	local timeStr = "";

	if (timeTable.d == 1)
	{
		timeStr = timeTable.d + " day ";
	}
	else if (timeTable.d > 1)
	{
		timeStr = timeTable.d + " days ";
	}

	if (timeTable.h == 1)
	{
		timeStr = timeStr + timeTable.h + " hr ";
	}
	else if (timeTable.h > 1)
	{
		timeStr = timeStr + timeTable.h + " hrs ";
	}

	if (timeTable.m == 1)
	{
		timeStr = timeStr + timeTable.m + " min ";
	}
	else if (timeTable.m > 1)
	{
		timeStr = timeStr + timeTable.m + " mins ";
	}

	if (timeTable.s == 1)
	{
		timeStr = timeStr + timeTable.s + " sec ";
	}
	else if (timeTable.s > 1)
	{
		timeStr = timeStr + timeTable.s + " secs ";
	}

	return timeStr;
};
this.Util.setCertainComponentVisible <- function ( tooltipComp, nameOfComponent, visible )
{
	foreach( childComp in tooltipComp.components )
	{
		local data = childComp.getData();

		if (data == nameOfComponent)
		{
			childComp.setVisible(visible);
		}
	}
};
this.Util.paraseMiliToTable <- function ( value )
{
	local seconds = value;
	local timeTable = {};
	timeTable.d <- 0;
	timeTable.h <- 0;
	timeTable.m <- 0;
	timeTable.s <- 0;
	timeTable.d = (seconds / this.gMilisecPerDay).tointeger();
	seconds = seconds % this.gMilisecPerDay;
	timeTable.h = (seconds / this.gMilisecPerHour).tointeger();
	seconds = seconds % this.gMilisecPerHour;
	timeTable.m = (seconds / this.gMilisecPerMinute).tointeger();
	seconds = seconds % this.gMilisecPerMinute;
	timeTable.s = (seconds / this.gMilisecPerSecond).tointeger();
	return timeTable;
};
this.Util.paraseSecToTable <- function ( value )
{
	local seconds = value;
	local timeTable = {};
	timeTable.d <- 0;
	timeTable.h <- 0;
	timeTable.m <- 0;
	timeTable.s <- 0;
	timeTable.d = (seconds / this.gSecPerDay).tointeger();
	seconds = seconds % this.gSecPerDay;
	timeTable.h = (seconds / this.gSecPerHour).tointeger();
	seconds = seconds % this.gSecPerHour;
	timeTable.m = (seconds / this.gSecPerMinute).tointeger();
	seconds = seconds % this.gSecPerMinute;
	timeTable.s = seconds;
	return timeTable;
};
this.Util.updateMiniMapStickers <- function ()
{
	if (::_avatar == null)
	{
		return;
	}

	foreach( k, so in this._sceneObjectManager.getCreatures() )
	{
		local node = so.getNode();

		if (::_avatar.getID() == so.getID() && ::LegendItemSelected[this.LegendItemTypes.YOU])
		{
			this._root.setMinimapSceneNodeSticker(node, ::LegendItemTypes.YOU);
		}
		else if ((so.getMeta("copper_shopkeeper") || so.getMeta("credit_shopkeeper") || so.getMeta("credit_shop") != null) && ::LegendItemSelected[this.LegendItemTypes.SHOP])
		{
			this._root.setMinimapSceneNodeSticker(node, ::LegendItemTypes.SHOP);
		}
		else if (so.getMeta("quest_giver") && ::LegendItemSelected[this.LegendItemTypes.QUEST_GIVER])
		{
			break;
		}
		else if (!so.isDead() && (so.getStat(this.Stat.CREATURE_CATEGORY) in ::LegendItemSelected) && ::LegendItemSelected[so.getStat(this.Stat.CREATURE_CATEGORY)])
		{
			local creatureCategory = so.getStat(this.Stat.CREATURE_CATEGORY);
			this._root.setMinimapSceneNodeSticker(node, creatureCategory);
		}
		else
		{
			this._root.setMinimapSceneNodeSticker(node, "");
		}
	}
};
this.Util.getProfileSnapshotByTime <- function ()
{
	local profTimes = this.System.profileSnapshot();
	local bytime = function ( a, b ) : ( profTimes )
	{
		local ta = profTimes[a];
		local tb = profTimes[b];

		if (ta < tb)
		{
			return 1;
		}

		if (ta > tb)
		{
			return -1;
		}

		return 0;
	};
	local keys = this.Util.tableKeys(profTimes);
	keys.sort(bytime);
	local result = [];

	foreach( k in keys )
	{
		if (profTimes[k] > 0.001)
		{
			result.append([
				k,
				profTimes[k]
			]);
		}
	}

	return result;
};
this.Util.randomElement <- function ( array )
{
	if (array.len() == 0)
	{
		return null;
	}

	local number = this._random.nextInt(array.len());
	return array[number];
};
this.Util.getRangeOffset <- function ( source, target )
{
	local sourceSize = source.getStat(this.Stat.TOTAL_SIZE);
	local targetSize = target.getStat(this.Stat.TOTAL_SIZE);

	if (sourceSize < this.gMinCreatureSize)
	{
		sourceSize = this.gMinCreatureSize;
	}

	if (targetSize < this.gMinCreatureSize)
	{
		targetSize = this.gMinCreatureSize;
	}

	return sourceSize + targetSize;
};
this.Util.fileExists <- function ( path )
{
	try
	{
		::System.readFile(path);
	}
	catch( err )
	{
		return false;
	}

	return true;
};
this.Util.rfind <- function ( source, str )
{
	local len = str.len();
	local tmp = 0;
	local nl = source.find(str, tmp + 1 + len);

	if (nl == null)
	{
	}
	else
	{
		tmp = nl;
		  // [014]  OP_JMP            0    -12    0    0
	}

	return tmp;
};
this.Util.updateFromVideoSettings <- function ( value )
{
	switch(value)
	{
	case "None":
		local systemInfo = this.System.getSystemInfo();
		local processorPoints = systemInfo.processors * 3;

		if (processorPoints > 9)
		{
			processorPoints = 9;
		}

		local memoryPoints = systemInfo.memory / 1024 * 2;

		if (memoryPoints > 8)
		{
			memoryPoints = 8;
		}

		local shaderModelPoints = systemInfo.shader_model * 2;

		if (shaderModelPoints > 6)
		{
			shaderModelPoints = 6;
		}

		local textureUnitPoints = systemInfo.texture_units / 2;

		if (textureUnitPoints > 4)
		{
			textureUnitPoints = 4;
		}

		local totalPoints = processorPoints + memoryPoints + shaderModelPoints + textureUnitPoints;
		local videoSetting = "Medium";

		if (totalPoints >= 23)
		{
			videoSetting = "High";
		}
		else if (totalPoints >= 10)
		{
			videoSetting = "Medium";
		}
		else
		{
			videoSetting = "Low";
		}

		local gpuType = systemInfo.device_name;

		if (gpuType.find("Intel") != null && gpuType.find("Express") != null)
		{
			this.Pref.set("video.UICache", false);
		}

		this.Pref.set("video.Settings", videoSetting);
		this.updateFromVideoSettings(videoSetting);
		break;

	case "Low":
		::Pref.set("video.CharacterShadows", false);
		::Pref.set("video.TerrainDistance", 1500);
		::Pref.set("video.ClutterDensity", 0.0);
		::Pref.set("video.ClutterDistance", 150.0);
		::Pref.set("video.Splatting", false);
		::Pref.set("video.ClutterVisible", false);
		::Pref.set("video.FSAA", 0);
		::Pref.set("video.Bloom", false);
		break;

	case "Medium":
		::Pref.set("video.CharacterShadows", false);
		::Pref.set("video.Splatting", true);
		::Pref.set("video.TerrainDistance", 2500);
		::Pref.set("video.ClutterDensity", 1.0);
		::Pref.set("video.ClutterDistance", 300.0);
		::Pref.set("video.ClutterVisible", true);
		::Pref.set("video.FSAA", 0);
		::Pref.set("video.Bloom", false);
		break;

	case "High":
		::Pref.set("video.CharacterShadows", true);
		::Pref.set("video.Splatting", true);
		::Pref.set("video.TerrainDistance", 3000);
		::Pref.set("video.ClutterDensity", 2.0);
		::Pref.set("video.ClutterDistance", 400.0);
		::Pref.set("video.ClutterVisible", true);
		::Pref.set("video.FSAA", 2);
		::Pref.set("video.Bloom", true);
		break;

	case "custom":
		break;
	}
};
this.Util.updateUICache <- function ( value )
{
	local nonCachingScreens = [
		"DebugScreen",
		"MainScreen",
		"MiniMapScreen",
		"VideoOptionsScreen",
		"OptionsScreen",
		"MapWindow",
		"QuestTracker"
	];
	local screens = this.Screens.getAllScreens();

	foreach( screenName, screen in screens )
	{
		if (!this.indexOf(nonCachingScreens, screenName))
		{
			screen.setCached(value);
		}
	}
};
this.Util.addItemDataInfo <- function ( textString, itemData, height, heightSize )
{
	if (itemData)
	{
		textString = textString + "<b><font color=\"" + this.Colors.yellow + "\">" + "Item Data" + ":</b></font><br>";
		textString = this.Util.addNewTextLine(textString, "Item Def Id", itemData.mItemDefId, "Bright Red");
		textString = this.Util.addNewTextLine(textString, "Type", ::ItemTypeNameDef[itemData.getType()].name);
		textString = this.Util.addNewTextLine(textString, "Container Id", itemData.getContainerId());
		textString = this.Util.addNewTextLine(textString, "Container Slot", itemData.mContainerSlot);
		textString = this.Util.addNewTextLine(textString, "Time Remaining recieved at", itemData.timeRemainingRecievedAt);
		textString = this.Util.addNewTextLine(textString, "Time Remaining", itemData.getTimeRemaining());
		textString = this.Util.addNewTextLine(textString, "Bound", itemData.mBound);

		if (itemData.getDurability())
		{
			textString = this.Util.addNewTextLine(textString, "Durability", itemData.getDurability());
			height = height + heightSize;
		}

		height = height + heightSize * 8;
	}

	local data = {
		text = textString,
		height = height
	};
	return data;
};
this.Util.addItemDefDataInfo <- function ( textString, itemDefData, height, heightSize )
{
	local isTypeRecipe = false;

	if (itemDefData)
	{
		textString = textString + "<b><font color=\"" + this.Colors.yellow + "\">" + "Item Def Data" + ":</b></font><br>";
		textString = this.Util.addNewTextLine(textString, "Item Def Id", itemDefData.getID(), "Bright Red");
		textString = this.Util.addNewTextLine(textString, "Type", ::ItemTypeNameDef[itemDefData.getType()].name);
		textString = this.Util.addNewTextLine(textString, "Display name", itemDefData.getDisplayName());

		if (itemDefData.getType() == this.ItemType.RECIPE)
		{
			textString = this.Util.addNewTextLine(textString, "Result Item", itemDefData.mResultItem);
			textString = this.Util.addNewTextLine(textString, "Key Component", itemDefData.mKeyComponent);
			textString = this.Util.addNewTextLine(textString, "Craft Component", this.serialize(itemDefData.mCraftComponents));
			height = height + heightSize * 3;
			isTypeRecipe = true;
		}

		if (itemDefData.getQuestName() != "")
		{
			textString = this.Util.addNewTextLine(textString, "Quest Name", itemDefData.getQuestName());
		}

		if (itemDefData.getAppearance())
		{
			textString = this.Util.addNewTextLine(textString, "Appearance", this.serialize(itemDefData.getAppearance()));
		}

		textString = this.Util.addNewTextLine(textString, "Icon", itemDefData.getIcon());
		textString = this.Util.addNewTextLine(textString, "mIvType1", ::ItemIntegerTypeNameMapping[itemDefData.mIvType1]);
		textString = this.Util.addNewTextLine(textString, "mIvMax1", itemDefData.mIvMax1);
		textString = this.Util.addNewTextLine(textString, "mIvType2", ::ItemIntegerTypeNameMapping[itemDefData.mIvType2]);
		textString = this.Util.addNewTextLine(textString, "mIvMax2", itemDefData.mIvMax2);
		textString = this.Util.addNewTextLine(textString, "Container Slots", itemDefData.getContainerSlots());
		textString = this.Util.addNewTextLine(textString, "Level", itemDefData.getLevel());
		textString = this.Util.addNewTextLine(textString, "Binding Type", ::ItemBindingTypeNameMapping[itemDefData.getBindingType()].name);
		textString = this.Util.addNewTextLine(textString, "Equip Type", ::ItemEquipTypeNameMapping[itemDefData.getEquipType()].name + ", " + itemDefData.getEquipType());
		textString = this.Util.addNewTextLine(textString, "Weapon Type", ::WeaponTypeNameMapping[itemDefData.getWeaponType()].name + ", " + itemDefData.getWeaponType());
		textString = this.Util.addNewTextLine(textString, "Weapon Damage Min", itemDefData.mWeaponDamageMin);
		textString = this.Util.addNewTextLine(textString, "Weapon Damage Max", itemDefData.mWeaponDamageMax);
		textString = this.Util.addNewTextLine(textString, "Weapon Extra Damage Type", ::DamageTypeNameMapping[itemDefData.mWeaponExtraDamageType]);
		textString = this.Util.addNewTextLine(textString, "Weapon Extra Damange Rating", itemDefData.mWeaponExtraDamangeRating);
		textString = this.Util.addNewTextLine(textString, "Equip Effect Id", itemDefData.mEquipEffectId);
		textString = this.Util.addNewTextLine(textString, "Use Ability Id", itemDefData.mUseAbilityId);
		textString = this.Util.addNewTextLine(textString, "Action Ability Id", itemDefData.mActionAbilityId);
		textString = this.Util.addNewTextLine(textString, "Armor Type", ::ArmorTypeNameMapping[itemDefData.mArmorType].name + ", " + itemDefData.mArmorType);
		textString = this.Util.addNewTextLine(textString, "Armor Resist Melee", itemDefData.mArmorResistMelee);
		textString = this.Util.addNewTextLine(textString, "Armor Resist Fire", itemDefData.mArmorResistFire);
		textString = this.Util.addNewTextLine(textString, "Armor Resist Frost", itemDefData.mArmorResistFrost);
		textString = this.Util.addNewTextLine(textString, "Armor Resist Mystic", itemDefData.mArmorResistMystic);
		textString = this.Util.addNewTextLine(textString, "Armor Resist Death", itemDefData.mArmorResistDeath);
		textString = this.Util.addNewTextLine(textString, "Weapon Speed", itemDefData.mWeaponSpeed);
		textString = this.Util.addNewTextLine(textString, "Bonus Strength", itemDefData.mBonusStrength);
		textString = this.Util.addNewTextLine(textString, "Bonus Dexterity", itemDefData.mBonusDexterity);
		textString = this.Util.addNewTextLine(textString, "Bonus Constitution", itemDefData.mBonusConstitution);
		textString = this.Util.addNewTextLine(textString, "Bonus Psyche", itemDefData.mBonusPsyche);
		textString = this.Util.addNewTextLine(textString, "Bonus Spirit", itemDefData.mBonusSpirit);
		textString = this.Util.addNewTextLine(textString, "Bonus Will", itemDefData.mBonusWill);
		textString = this.Util.addNewTextLine(textString, "Melee Hit Mod", itemDefData.mMeleeHitMod);
		textString = this.Util.addNewTextLine(textString, "Melee Crit Mod", itemDefData.mMeleeCritMod);
		textString = this.Util.addNewTextLine(textString, "Magic Hit Mod", itemDefData.mMagicHitMod);
		textString = this.Util.addNewTextLine(textString, "Magic Crit Mod", itemDefData.mMagicCritMod);
		textString = this.Util.addNewTextLine(textString, "Parry Mod", itemDefData.mParryMod);
		textString = this.Util.addNewTextLine(textString, "Block Mod", itemDefData.mBlockMod);
		textString = this.Util.addNewTextLine(textString, "Run Speed Mod", itemDefData.mRunSpeedMod);
		textString = this.Util.addNewTextLine(textString, "Regen Health Mod", itemDefData.mRegenHealthMod);
		textString = this.Util.addNewTextLine(textString, "Attack Speed Mod", itemDefData.mAttackSpeedMod);
		textString = this.Util.addNewTextLine(textString, "Cast Speed Mod", itemDefData.mCastSpeedMod);
		textString = this.Util.addNewTextLine(textString, "Healing Mod", itemDefData.mHealingMod);
		textString = this.Util.addNewTextLine(textString, "Quality Level", itemDefData.getQualityLevel());
		textString = this.Util.addNewTextLine(textString, "Min Use Level", itemDefData.getMinUseLevel());

		if (itemDefData.mFlavorText != "")
		{
			textString = this.Util.addNewTextLine(textString, "Flavor Text", itemDefData.mFlavorText);
		}

		textString = this.Util.addNewTextLine(textString, "Special Item Type", ::SpecialItemTypeNameMapping[itemDefData.mSpecialItemType].name + ", " + itemDefData.mSpecialItemType);
		height = height + heightSize * 51;
	}

	local data = {
		text = textString,
		height = height,
		isRecipe = isTypeRecipe
	};
	return data;
};
this.Util.addAbilityDataInfo <- function ( textString, ability, height, heightSize )
{
	if (ability)
	{
		textString = textString + "<b><font color=\"" + this.Colors.yellow + "\">" + "Ability Data" + ":</b></font><br>";
		textString = this.Util.addNewTextLine(textString, "Ability Name", ability.getName());
		textString = this.Util.addNewTextLine(textString, "Group Id", ability.getGroupId());
		textString = this.Util.addNewTextLine(textString, "Tier", ability.getTier());
		textString = this.Util.addNewTextLine(textString, "Use Type", ability.getUseType());
		textString = this.Util.addNewTextLine(textString, "Magic Charge Given", ability.getAddMagicCharge());
		textString = this.Util.addNewTextLine(textString, "Melee Charge Given", ability.getAddMeleeCharge());
		textString = this.Util.addNewTextLine(textString, "Ability Class", ability.getAbilityClass());
		textString = this.Util.addNewTextLine(textString, "Buff Category", ability.getBuffCategory());
		textString = this.Util.addNewTextLine(textString, "Gold Cost", ability.getGoldCost());
		textString = this.Util.addNewTextLine(textString, "Target Criteria", ability.getTargetCriteria());
		textString = this.Util.addNewTextLine(textString, "Hostility", ability.getHostility());
		textString = this.Util.addNewTextLine(textString, "Will Cost", ability.getWill());
		textString = this.Util.addNewTextLine(textString, "Might Cost", ability.getMight());
		textString = this.Util.addNewTextLine(textString, "Min Will Charge", ability.getWillMinCharge());
		textString = this.Util.addNewTextLine(textString, "Max Will Charge", ability.getWillMaxCharge());
		textString = this.Util.addNewTextLine(textString, "Min Might Charge", ability.getMightMinCharge());
		textString = this.Util.addNewTextLine(textString, "Max Might Charge", ability.getMightMaxCharge());
		textString = this.Util.addNewTextLine(textString, "Ability Require Target", ability.doesAbilityRequireTarget());
		textString = this.Util.addNewTextLine(textString, "Special Requirements", this.serialize(ability.getSpecialRequirement()));
		textString = this.Util.addNewTextLine(textString, "Reagents Needed", this.serialize(ability.getReagents()));
		textString = this.Util.addNewTextLine(textString, "Activation Criteria", this.serialize(ability.getActivationCriteria()));
		textString = this.Util.addNewTextLine(textString, "Slot Coordinates", this.serialize(ability.getSlotCoordinates()));
		textString = this.Util.addNewTextLine(textString, "Ownage", ability.getOwnage());
		textString = this.Util.addNewTextLine(textString, "Purchase Points Required", ability.getPurchasePointsRequired());
		textString = this.Util.addNewTextLine(textString, "Purchase Level Required", ability.getPurchaseLevelRequired());
		textString = this.Util.addNewTextLine(textString, "Purchase Abilites Required", this.serialize(ability.getPurchaseAbilitiesRequired()));
		textString = this.Util.addNewTextLine(textString, "Purchase Class Required", ability.getPurchaseClassRequired());
		textString = this.Util.addNewTextLine(textString, "Equipment Requirements", this.serialize(ability.getEquipRequirements()));
		textString = this.Util.addNewTextLine(textString, "Status Requirements", this.serialize(ability.getStatusRequirements()));
		textString = this.Util.addNewTextLine(textString, "Duration", ability.getDuration());
		textString = this.Util.addNewTextLine(textString, "Category", ability.getCategory());
		textString = this.Util.addNewTextLine(textString, "Cooldown Category", ability.getCooldownCategory());
		textString = this.Util.addNewTextLine(textString, "Description", ability.getDescription());
		textString = this.Util.addNewTextLine(textString, "Range", ability.getRange());
		textString = this.Util.addNewTextLine(textString, "Time Until Available", ability.getTimeUntilAvailable());
		textString = this.Util.addNewTextLine(textString, "Time Used", ability.getTimeUsed());
		textString = this.Util.addNewTextLine(textString, "Warm-up Cue", ability.getWarmupCue());
		textString = this.Util.addNewTextLine(textString, "Cooldown Duration", ability.getCooldownDuration());
		textString = this.Util.addNewTextLine(textString, "Warm-up Duration", ability.getWarmupDuration());
		textString = this.Util.addNewTextLine(textString, "Warm-up Duration End Time", ability.getWarmupEndTime());
		textString = this.Util.addNewTextLine(textString, "Warm-up Start Time", ability.getWarmupStartTime());
		textString = this.Util.addNewTextLine(textString, "Warm-up Time Left", ability.getWarmupTimeLeft());
		textString = this.Util.addNewTextLine(textString, "Channel Time Left", ability.getChannelTimeLeft());
		textString = this.Util.addNewTextLine(textString, "Channel Start Time", ability.getChannelStartTime());
		textString = this.Util.addNewTextLine(textString, "Channel End Time", ability.getChannelEndTime());
		textString = this.Util.addNewTextLine(textString, "Visual Cue", ability.getVisualCue());
		height = height + heightSize * 48;
	}

	local data = {
		text = textString,
		height = height
	};
	return data;
};
this.Util.addNewTextLine <- function ( textString, title, value, ... )
{
	local color = "mint";

	if (vargc > 0)
	{
		color = vargv[0];
	}

	textString = textString + "<b>" + title + ":</b> <font color=\"" + this.Colors[color] + "\">" + value + "</font>" + "<br>";
	return textString;
};
