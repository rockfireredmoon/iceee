this.Assembler <- {};
this.gNextErrorID <- 0;
this.require("ContentLoader");
class this.Assembler.Factory 
{
	mSceneryAssembler = null;
	mAssemblerAlias = null;
	mObjectClass = null;
	mObjectType = null;
	mInstances = null;
	mAssembling = false;
	mRequiredArchives = null;
	mArchivesWaiter = null;
	mRequiredArchivesReady = false;
	mRequiredArchivesError = null;
	mShowNameType = true;
	mRequiredResources = null;
	constructor( objectClass, objectType )
	{
		this.mObjectClass = objectClass;
		this.mObjectType = objectType;
		this.mInstances = [];
		this.mRequiredArchives = [];
	}

	function getAssemblerDesc()
	{
		local str = this.getAssemblerAlias();
		str += "[";

		if (this.mRequiredArchivesReady)
		{
			str += "R";
		}
		else if (this.mRequiredArchivesError)
		{
			str += "E";
		}
		else if (this.mRequiredArchives)
		{
			str += "r";
		}

		str += "]";
		return str;
	}

	function getAssemblerName()
	{
		return "Assembler." + this.mObjectClass + "(" + this.getAssemblerDesc() + ")";
	}

	function getPropName()
	{
		if (!this.mSceneryAssembler || !this.mSceneryAssembler.mObjectType)
		{
			return null;
		}

		return this.mSceneryAssembler.mObjectType.tostring();
	}

	function getAssemblerAlias()
	{
		return this.mAssemblerAlias == null ? this.mObjectType : this.mAssemblerAlias;
	}

	function getReady()
	{
		return false;
	}

	function destroy()
	{
	}

	function reset()
	{
		this.mRequiredArchivesError = null;
		this.mRequiredArchives = [];
		this.mArchivesWaiter = null;
	}

	function setAssemblerAlias( alias )
	{
		this.mAssemblerAlias = alias;
	}

	function getObjectType()
	{
		return this.mObjectType;
	}

	function addManagedInstance( obj )
	{
		this.mInstances.append(obj);
		return obj;
	}

	function getInstanceCount()
	{
		return this.mInstances.len();
	}

	function removeManagedInstance( obj )
	{
		local i;

		for( i = 0; i < this.mInstances.len(); i++ )
		{
			if (this.mInstances[i] == obj)
			{
				this.mInstances.remove(i);
				return;
			}
		}
	}

	function reassembleInstances( ... )
	{
		local assembleNow = vargc > 0 ? vargv[0] : false;

		foreach( obj in this.mInstances )
		{
			local foo = "" + obj;
			obj.reassemble(assembleNow);
		}
	}

	function assemble( obj )
	{
		if (obj.isAssembled())
		{
			return true;
		}

		if (false)
		{
			return this._assemble(obj);
		}
		else
		{
			return this._assemble(obj);
			  // [018]  OP_POPTRAP        1      0    0    0
			  // [019]  OP_JMP            0     27    0    0
	
			// TODO - Em VERY not sure about this
			//try
			//{
			//	this.log.error("Error assembling " + $[stack offset 1] + ": " + this._assemble(obj));
			//	this.mRequiredArchivesError = this._assemble(obj);
			//	this.onError($[stack offset 1], this._assemble(obj));
			//	this.disassemble($[stack offset 1]);
			//	this.createErrorNode($[stack offset 1].getNode());
			//}
			//catch( err )
			//{
			//	this.log.error(err);
			//}
		}

		return false;
	}

	function getBody()
	{
		return null;
	}

	function onError( obj, error )
	{
	}

	function createErrorNode( parentNode )
	{
		local name = parentNode.getName() + "/ErrorNode/" + ( this.gNextErrorID++);
		local sphere = this._scene.createEntity(name, "Manipulator-QuestionMark.mesh");
		sphere.setVisibilityFlags(this.VisibilityFlags.ANY);
		sphere.setQueryFlags(this.QueryFlags.ANY | this.QueryFlags.MANIPULATOR);
		sphere.setEmissive(this.Color(1.0, 0.0, 0.0, 1.0));
		local node = parentNode.createChildSceneNode(name);
		this.log.debug("attachObject(Attaching Error Node)");
		node.attachObject(sphere);
		return node;
	}

	function reassemble( obj )
	{
		if (obj.isAssembled())
		{
			this.disassemble(obj);
		}

		return this.assemble(obj);
	}

	function _assemble( obj )
	{
		throw this.Exception("_assemble() not overridden");
	}

	function disassemble( obj )
	{
		obj.mAssemblyData = null;
		local node = obj.getNode();

		if (node != null)
		{
			foreach( n in node.getChildren() )
			{
				if (n.getName().find("/ErrorNode/") != null)
				{
					obj.getNode().removeChild(n);
				}
			}
		}

		obj._setAssembled(false);
	}

	function getAttachPointDef( name )
	{
		return null;
	}

	function getBaseEntity( so )
	{
		return null;
	}

	function getBoundingRadius( so )
	{
		local e = this.getBaseEntity(so);
		return e ? e.getBoundingRadius() : 10.0;
	}

	function notifyTransformed( so )
	{
	}

	function _addRequiredArchive( media )
	{
		if (typeof media != "string")
		{
			throw this.Exception("Invalid required archive: " + media);
		}

		foreach( m in this.mRequiredArchives )
		{
			if (m == media)
			{
				return;
			}
		}

		this.mRequiredArchives.append(media);
		this.mRequiredArchivesReady = false;
		this.mRequiredArchivesError = null;
		this.mArchivesWaiter = null;
	}

	function getArchiveError()
	{
		return this.mRequiredArchivesError;
	}

	function _checkRequiredArchivesLoaded( ... )
	{
		if (this.mRequiredArchivesReady)
		{
			return true;
		}

		if (this.mRequiredArchivesError != null)
		{
			return false;
		}

		if (this.mArchivesWaiter != null)
		{
			if (this.mArchivesWaiter.isReady())
			{
				this.mRequiredArchivesReady = true;
				return true;
			}

			return false;
		}

		this.mArchivesWaiter = this.Util.waitForAssets(this.mRequiredArchives, null);
		return false;
	}

	function _addRequiredResource( resourceName )
	{
		if (this.mRequiredResources == null)
		{
			this.mRequiredResources = [];
		}

		this.mRequiredResources.append(resourceName);
	}

	function _checkRequiredResourcesPrepared()
	{
		if (this.mRequiredResources == null)
		{
			return true;
		}

		if (!this.gPrepareResources)
		{
			return true;
		}

		if (this._root.prepareResources(this.mRequiredResources, this.getAssemblerName()))
		{
			return true;
		}

		return false;
	}

	function getSceneryAssembler()
	{
		return null;
	}

	function hasError()
	{
		return this.mRequiredArchivesError != null;
	}

	function _tostring()
	{
		return this.getAssemblerName();
	}

	function getShowNameType()
	{
		return this.mShowNameType;
	}

	function setShowNameType( value )
	{
		this.mShowNameType = value;
		this._refreshInstancesNameLabel();
	}

	function _refreshInstancesNameLabel()
	{
		foreach( obj in this.mInstances )
		{
			local val = obj.mShowName;
			obj.setShowName(false);
			obj.setShowName(val);
		}
	}

	function getConfig()
	{
		return null;
	}

}

function GetAssembler( kind, type, ... )
{
	local assTable;

	if (!(kind in this.Assembler))
	{
		throw this.Exception("No assembler defined for " + kind + " objects");
	}

	if (!(kind in this._Assemblers))
	{
		assTable = {};
		this._Assemblers[kind] <- assTable;
	}
	else
	{
		assTable = this._Assemblers[kind];
	}

	if (type)
	{
		type = type.tostring();
	}

	if (type in assTable)
	{
		return assTable[type];
	}

	if (vargc > 0 && !vargv[0])
	{
		return null;
	}

	local a = this.Assembler[kind](type);
	assTable[type] <- a;
	return a;
}

function FindAssemblerByAlias( kind, alias )
{
	if (!(kind in this._Assemblers))
	{
		throw this.Exception("Unknown assembler type: " + kind);
	}

	foreach( type, ass in this._Assemblers[kind] )
	{
		if (ass.getAssemblerAlias() == alias)
		{
			return ass;
		}
	}

	return null;
}

function GetAssemblers( kind )
{
	if (!(kind in this._Assemblers))
	{
		return {};
	}

	return this._Assemblers[kind];
}

function AssembleAsset( assetRef, ... )
{
	local t0 = this.System.currentTimeMillis();
	local parentNode = vargc > 0 ? vargv[0] : null;
	local interactiveMode = vargc > 1 ? vargv[1] : false;
	local name = vargc > 2 ? vargv[2] : "";
	local a = assetRef.getAsset();
	local node;
	local addToParent = true;

	if (a in ::Vegetation)
	{
		addToParent = false;
		local vars = assetRef.getVars();
		local vegetator = this.CSMXMLVegetator(a, vars, parentNode);
		vegetator.create();
		node = vegetator.getNode();
	}
	else if (this.GetAssetArchive(a))
	{
		addToParent = false;
		local createComponentInstanceT0 = this.System.currentTimeMillis();
		local getAssetT0 = this.System.currentTimeMillis();
		local theAsset = assetRef.getAsset();
		local theAssetVars = assetRef.getVars();
		local getAssetElapsed = this.System.currentTimeMillis() - getAssetT0;

		if (getAssetElapsed > 50)
		{
			this.log.warn("*** [hitch] createComponentInstance(getAssetElapsed):" + name + " took a long time: " + getAssetElapsed);
		}

		node = this._scene.createComponentInstance(theAsset, parentNode.createChildSceneNode(), interactiveMode, theAssetVars, name, {
			Entity = {
				queryFlags = this.QueryFlags.LIGHT_OCCLUDER | this.QueryFlags.ANYTHING
			},
			visibilityFlags = this.VisibilityFlags.ANY | this.VisibilityFlags.PROPS | this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_1 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3
		});
		local elapsed = this.System.currentTimeMillis() - createComponentInstanceT0;

		if (elapsed > 50)
		{
			this.log.warn("*** [hitch] createComponentInstance:" + name + " took a long time: " + elapsed);
		}
	}
	else
	{
		this.log.error("Do not know how to assemble asset: " + assetRef);
		return null;
	}

	if (node && parentNode && addToParent)
	{
		parentNode.addChild(node);
	}

	local functionTime = this.System.currentTimeMillis() - t0;

	if (functionTime > 50)
	{
		this.log.warn("*** [hitch] AssembleAsset" + name + " took a long time: " + functionTime);
	}

	return node;
}

function AssembleCSMFromString( text, ... )
{
	local t0 = this.System.currentTimeMillis();
	local parentNode = vargc > 0 ? vargv[0] : null;
	local interactiveMode = vargc > 1 ? vargv[1] : false;
	local name = vargc > 2 ? vargv[2] : "";
	local node;
	node = this._scene.createComponentInstanceFromString(text, parentNode.createChildSceneNode(), interactiveMode, null, name, {
		Entity = {
			queryFlags = this.QueryFlags.LIGHT_OCCLUDER | this.QueryFlags.ANYTHING
		},
		visibilityFlags = this.VisibilityFlags.ANY | this.VisibilityFlags.PROPS | this.VisibilityFlags.LIGHT_GROUP_0 | this.VisibilityFlags.LIGHT_GROUP_1 | this.VisibilityFlags.LIGHT_GROUP_2 | this.VisibilityFlags.LIGHT_GROUP_3
	});
	return node;
}

this._Assemblers <- {};
function InitTestAssemblers()
{
	local i;
	local a;
	local name;
	local def;
	a = this.GetAssembler("Creature", "Big Fat Shroomie");
	a.configure("n1:" + this.System.encodeVars({
		m = "Horde-Shroomie",
		sz = 5.0
	}));
	a = this.GetAssembler("Creature", "Hoppicus");
	a.configure("c1:" + this.System.encodeVars({
		r = "b",
		g = "m",
		sz = 1.0,
		sk = this.System.encodeVars({
			base = "997d68",
			highlight = "ac735a",
			detail = "534741",
			eye = "00adef",
			nose = "7c7873"
		}),
		c = this.System.encodeVars({
			chest = "Armor-Chain1",
			leggings = "Armor-Chain1",
			belt = "Armor-Chain1"
		})
	}));
	a = this.GetAssembler("Creature", "Hoppicus-Manual");
	a.setBody("Biped-Male", "Biped-Bounder_Male-Body");
	a.setHead("Biped-Bounder_Male-Head", "Biped-Bounder_Male-Head");
	a.setAttachmentPointDefaults(::ContentDef["Biped-Bounder_Male"].AttachmentPoints);
	a.setSize(1.0);
	a.setSkinColors({
		base = "997d68",
		highlight = "ac735a",
		detail = "534741",
		eye = "00adef",
		nose = "7c7873"
	});
	a.addDetail({
		name = "left_forearm",
		mesh = "Biped-Male-Forearm_Left.mesh",
		bone = "Bone-LeftForearm",
		texture = "body"
	});
	a.addDetail({
		name = "right_forearm",
		mesh = "Biped-Male-Forearm_Right.mesh",
		bone = "Bone-RightForearm",
		texture = "body"
	});
	a.addDetail({
		name = "left_ear",
		mesh = "Biped-Bounder_Male-Ear_Left.mesh",
		bone = "Bone-LeftEar",
		texture = "head"
	});
	a.addDetail({
		name = "right_ear",
		mesh = "Biped-Bounder_Male-Ear_Right.mesh",
		bone = "Bone-RightEar",
		texture = "head"
	});
	a.setClothing({
		chest = "Armor-Chain1",
		leggings = "Armor-Chain1",
		arms = "Armor-Chain1",
		collar = "Armor-Chain1",
		gloves = "Armor-Chain1",
		boots = "Armor-Chain1",
		belt = "Armor-Chain1"
	});
}

this.Assembler.flush <- function ( force )
{
	foreach( k, v in ::_Assemblers )
	{
		local tmp = {};

		foreach( k2, v2 in v )
		{
			if (force == false && v2.getInstanceCount() > 0)
			{
				tmp[k2] <- v2;
			}
			else
			{
				this.log.debug("Flushing " + k + " assembler: " + k2);
				v2.destroy();
			}
		}

		::_Assemblers[k] = tmp;
	}
};
