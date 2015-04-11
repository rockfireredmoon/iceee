this.GateTrigger <- {};
this.GateTrigger.GateTypes <- {
	PLAY_STAGE_1 = "PlayStage1",
	PLAY_STAGE_2 = "PlayStage2",
	PLAY_STAGE_3 = "PlayStage3",
	PLAY_STAGE_4 = "PlayStage4",
	PLAY_STAGE_1_DEP = "PlayStage1_Dep",
	PLAY_STAGE_2_DEP = "PlayStage2_Dep",
	PLAY_STAGE_3_DEP = "PlayStage3_Dep",
	PLAY_STAGE_4_DEP = "PlayStage4_Dep"
};
this.GateTrigger.TriggerTypes <- {
	LOCATION = 1,
	LEVEL = 2,
	QUEST_COMPLETE = 3,
	QUEST_ACT_COMPLETE = 4
};
this.GateTrigger.FetchTriggers <- {
	[this.GateTrigger.GateTypes.PLAY_STAGE_2] = {
		location = [
			"Bastion",
			"Mushroom Isle"
		],
		fetched = false
	},
	[this.GateTrigger.GateTypes.PLAY_STAGE_3] = {
		location = [
			"Corsica",
			"Lighthouse"
		],
		fetched = false
	},
	[this.GateTrigger.GateTypes.PLAY_STAGE_4] = {
		location = [
			"Earthrise",
			"Earthend",
			"Southend Passage"
		],
		fetched = false
	}
};
this.GateTrigger.LoadTriggers <- {
	[this.GateTrigger.GateTypes.PLAY_STAGE_1_DEP] = {
		location = [
			"Bastion"
		],
		loaded = false
	},
	[this.GateTrigger.GateTypes.PLAY_STAGE_2_DEP] = {
		location = [
			"Corsica",
			"Lighthouse"
		],
		loaded = false
	},
	[this.GateTrigger.GateTypes.PLAY_STAGE_3_DEP] = {
		location = [
			"Earthrise",
			"Earthend",
			"Southend Passage"
		],
		loaded = false
	},
	[this.GateTrigger.GateTypes.PLAY_STAGE_4_DEP] = {
		not_location = [
			"Mushroom Isle",
			"Bastion",
			"Corsica",
			"Lighthouse",
			"Earthrise",
			"Earthend",
			"Southend Passage"
		],
		loaded = false
	}
};
class this.LoadGateTrigger 
{
	constructor()
	{
	}

	function load()
	{
	}

	function fetch()
	{
	}

}

class this.LocationLoadGateTrigger extends this.LoadGateTrigger
{
	mClassName = "LocationLoadGateTrigger";
	mNameOfVariable = "location";
	mNameOfSecondVarible = "not_location";
	constructor()
	{
	}

	function load( data )
	{
		foreach( gateName, gate in this.GateTrigger.LoadTriggers )
		{
			if (!gate.loaded)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( loc in gate[this.mNameOfVariable] )
					{
						if (data == loc)
						{
							this.LoadGate.Require(gateName);
						}
					}
				}

				if (this.mNameOfSecondVarible in gate)
				{
					local shouldLoad = true;

					foreach( loc in gate[this.mNameOfSecondVarible] )
					{
						if (data == loc)
						{
							shouldLoad = false;
							break;
						}
					}

					if (shouldLoad)
					{
						this.LoadGate.Require(gateName);
					}
				}
			}
		}
	}

	function fetch( data )
	{
		foreach( gateName, gate in this.GateTrigger.FetchTriggers )
		{
			if (!gate.fetched)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( loc in gate[this.mNameOfVariable] )
					{
						if (data == loc)
						{
							this.LoadGate.Prefetch(gateName);
						}
					}
				}

				if (this.mNameOfSecondVarible in gate)
				{
					local prefetch = true;

					foreach( loc in gate[this.mNameOfSecondVarible] )
					{
						if (data == loc)
						{
							prefetch = false;
							break;
						}
					}

					if (prefetch)
					{
						this.LoadGate.Prefetch(gateName);
					}
				}
			}
		}
	}

}

class this.LevelLoadGateTrigger extends this.LoadGateTrigger
{
	mClassName = "LevelLoadGateTrigger";
	mNameOfVariable = "level";
	constructor()
	{
	}

	function load( data )
	{
		foreach( gateName, gate in this.GateTrigger.LoadTriggers )
		{
			if (!gate.loaded)
			{
				if (this.mNameOfVariable in gate)
				{
					if (data >= gate[this.mNameOfVariable])
					{
						this.LoadGate.Require(gateName);
					}
				}
			}
		}
	}

	function fetch( data )
	{
		foreach( gateName, gate in this.GateTrigger.FetchTriggers )
		{
			if (!gate.fetched)
			{
				if (this.mNameOfVariable in gate)
				{
					if (data >= gate[this.mNameOfVariable])
					{
						this.LoadGate.Prefetch(gateName);
					}
				}
			}
		}
	}

}

class this.QuestCompleteGateTrigger extends this.LoadGateTrigger
{
	mClassName = "QuestCompleteGateTrigger";
	mNameOfVariable = "quest_complete";
	constructor()
	{
	}

	function load( data )
	{
		foreach( gateName, gate in this.GateTrigger.LoadTriggers )
		{
			if (!gate.loaded)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( questId in gate[this.mNameOfVariable] )
					{
						if (data == questId)
						{
							this.LoadGate.Require(gateName);
						}
					}
				}
			}
		}
	}

	function fetch( data )
	{
		foreach( gateName, gate in this.GateTrigger.FetchTriggers )
		{
			if (!gate.fetched)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( questId in gate[this.mNameOfVariable] )
					{
						if (data == questId)
						{
							this.LoadGate.Prefetch(gateName);
						}
					}
				}
			}
		}
	}

}

class this.QuestActCompleteGateTrigger extends this.LoadGateTrigger
{
	mClassName = "QuestActCompleteGateTrigger";
	mNameOfVariable = "quest_act_complete";
	constructor()
	{
	}

	function load( data )
	{
		foreach( gateName, gate in this.GateTrigger.LoadTriggers )
		{
			if (!gate.loaded)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( questId, questActNumber in gate[this.mNameOfVariable] )
					{
						if (data.questId == questId && data.actNumber == questActNumber)
						{
							this.LoadGate.Require(gateName);
						}
					}
				}
			}
		}
	}

	function fetch( data )
	{
		foreach( gateName, gate in this.GateTrigger.FetchTriggers )
		{
			if (!gate.fetched)
			{
				if (this.mNameOfVariable in gate)
				{
					foreach( questId, questActNumber in gate[this.mNameOfVariable] )
					{
						if (data.questId == questId && data.actNumber == questActNumber)
						{
							this.LoadGate.Prefetch(gateName);
						}
					}
				}
			}
		}
	}

}

