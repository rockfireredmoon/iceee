class this.UsableCreatureManager 
{
	cachedCreatureDefIds = null;
	constructor()
	{
		this.cachedCreatureDefIds = {};
		::_Connection.addListener(this);
	}

	function addCreatureDef( defId, useable )
	{
		this.cachedCreatureDefIds[defId] <- useable;
	}

	function hasDefId( id )
	{
		return id in this.cachedCreatureDefIds;
	}

	function isUseable( id )
	{
		if (id in this.cachedCreatureDefIds)
		{
			return this.cachedCreatureDefIds[id] != "N";
		}

		return false;
	}

	function removeFromCache( id )
	{
		if (id in this.cachedCreatureDefIds)
		{
			delete this.cachedCreatureDefIds[id];
		}
	}

	function refreshCache()
	{
		this.cachedCreatureDefIds = {};

		foreach( k, v in ::_sceneObjectManager.getCreatures() )
		{
			v.queryUsable();
		}
	}

	function onQuestAbandoned( questId )
	{
		this.refreshCache();
	}

	function onQuestActCompleted( questId, act )
	{
		this.refreshCache();
	}

	function onQuestCompleted( questId )
	{
		this.refreshCache();
	}

	function onQuestObjectiveUpdate( questId, objective, complete, text )
	{
		this.refreshCache();
	}

	function onQuestJoined( questId )
	{
		this.refreshCache();
	}

}

