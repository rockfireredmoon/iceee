class this.CreatureDef 
{
	mStats = null;
	mMeta = null;
	mAssembler = null;
	mIsPlayer = false;
	mID = 0;
	constructor( id )
	{
		this.mAssembler = this.GetAssembler("Creature", id);
		this.mStats = {};
		this.mMeta = {};
		this.mID = id;
	}

	function getID()
	{
		return this.mID;
	}

	function setStat( name, value )
	{
		if ((name in this.mStats) && this.mStats[name] == value)
		{
			return;
		}

		this.mStats[name] <- value;
		this.updateAssembler(name, value);

		foreach( k, v in ::_sceneObjectManager.getCreatures() )
		{
			if (v.getCreatureDef() == this)
			{
				v.onCreatureDefUpdate(name, value);
			}
		}
	}

	function getStat( name )
	{
		if (name in this.mStats)
		{
			return this.mStats[name];
		}

		return null;
	}

	function setPlayer( which )
	{
		this.mIsPlayer = which;
	}

	function isPlayer()
	{
		return this.mIsPlayer;
	}

	function setMeta( name, value )
	{
		if (name in this.mMeta)
		{
			this.mMeta[name] = value;
		}
		else
		{
			this.mMeta[name] <- value;
		}
	}

	function getMeta( name )
	{
		if (name in this.mMeta)
		{
			return this.mMeta[name];
		}

		return null;
	}

	function updateAssembler( name, value )
	{
		if (name == this.Stat.APPEARANCE)
		{
			this.mAssembler.configure(value);
		}
		else if (name == this.Stat.EQ_APPEARANCE)
		{
			this.mAssembler.setEquipmentAppearance(value);
		}
		else if (name == this.Stat.DISPLAY_NAME)
		{
			this.mAssembler._refreshInstancesNameLabel();
		}
	}

	function getAssembler()
	{
		return this.mAssembler;
	}

	function getStats()
	{
		return this.mStats;
	}

	function getMetaData()
	{
		return this.mMeta;
	}

}

class this.CreatureDefManager 
{
	mCreatureDefs = null;
	constructor()
	{
		this.mCreatureDefs = {};
	}

	function getCreatureDef( id )
	{
		if (id in this.mCreatureDefs)
		{
			return this.mCreatureDefs[id];
		}

		local def = this.CreatureDef(id);
		this.mCreatureDefs[id] <- def;
		return def;
	}

	function hasCreatureDef( id )
	{
		return id in this.mCreatureDefs;
	}

	function removeCreatureDef( id )
	{
		if (id in this.mCreatureDefs)
		{
			this.log.debug("Removing CreatureDef " + id + "...");
			delete this.mCreatureDefs[id];
		}
	}

	function reset()
	{
		this.mCreatureDefs = {};
	}

}

this._creatureDefManager <- this.CreatureDefManager();
