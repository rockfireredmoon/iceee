this.require("Constants");
class this.LightingManager 
{
	mEnabled = false;
	mQueue = null;
	mLights = null;
	constructor()
	{
		this.mQueue = [];
		this.mLights = {};
		this._root.addListener(this);
	}

	function createSource( name, lightingGroup )
	{
		if (name in this.mLights)
		{
			return this.mLights[name];
		}

		local light = this._scene.createLight(name);
		light.setVisibilityFlags(lightingGroup | this.VisibilityFlags.SCENERY | this.VisibilityFlags.ANY);
		local node = this._scene.getRootSceneNode().createChildSceneNode(name + "/Node");
		node.attachObject(light);
		this.mLights[name] <- light;
		return light;
	}

	function removeSource( name )
	{
		if (!(name in this.mLights))
		{
			return;
		}

		local light = this.mLights[name];
		delete this.mLights[name];
		light.getParentSceneNode().destroy();
	}

	function _updateNodeVisibility( node, topNode )
	{
		return;
		local pos = node.getWorldPosition();
		pos.y += 0.5;
		local groupsMask = this.Light.getLightingGroupsMask();

		foreach( name, light in this.mLights )
		{
			local lightPos = light.getParentSceneNode().getWorldPosition();
			local dir = lightPos - pos;
			local flags = light.getVisibilityFlags() & groupsMask;
			local hits = this._scene.rayQuery(pos, dir, this.QueryFlags.LIGHT_OCCLUDER, true, false, 1, topNode);

			if (hits.len() > 0)
			{
				this.Util.visitMovables(node, function ( mo ) : ( flags )
				{
					local f = mo.getVisibilityFlags();
					mo.setVisibilityFlags(f & ~flags);
				}, 0);
			}
			else
			{
				this.Util.visitMovables(node, function ( mo ) : ( flags )
				{
					local f = mo.getVisibilityFlags();
					mo.setVisibilityFlags(f | flags);
				}, 0);
			}
		}

		foreach( child in node.getChildren() )
		{
			this._updateNodeVisibility(child, topNode);
		}
	}

	function updateVisibility( so )
	{
		if (!so.mNode)
		{
			return;
		}

		this._updateNodeVisibility(so.mNode, so.mNode);
	}

	function queueVisibilityUpdate( so )
	{
		this.mQueue.append(so);
	}

	function onEnterFrame()
	{
		while (this.mQueue.len() > 0)
		{
			local so = this.mQueue[0];
			this.mQueue.remove(0);
			this.updateVisibility(so);
		}
	}

}

