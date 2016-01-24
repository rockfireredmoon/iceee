/*
 * Instance script for whole of Anglorum
 *
 * Processes :-
 *		Step-up for elements
 */

info <- {
	name = "anglorum",
	enabled = true,
	author = "Emerald",
	description = "Anglorum Instance Script"
	speed = 25
}

class StepUpTracker {
	mDelay = 0;
	mRequiredKills = 0;
	mLastKill = 0;
	mSpawnerPropId = 0;
	mCreatureId = 0;
	mKills = 0;
	mText = "";
	
	constructor(delay, requiredKills, spawnerPropId, creatureId, text)
	{
		mDelay = delay;
		mRequiredKills = requiredKills;
		mSpawnerPropId = spawnerPropId;
		mCreatureId = creatureId; 
		mText = text;
	}
	
	function on_package_kill() {
	
		local now = inst.get_server_time();
	
		// If there was too long a gap between kills, reset the count
		if(now > mLastKill + mDelay)
			mKills = 0;
		mLastKill = now;
		
		mKills++;
		
		if(mKills >= mRequiredKills) {
			local spawnId = inst.spawn(mSpawnerPropId, mCreatureId, 0);
			if(spawnId > -1) {
				inst.creature_chat(spawnId, "s", mText);
			}
			mKills = 0;
		}
	}
}

trackers <- {
	["1a_waterElementalCamp01_general_STEPUP"] = StepUpTracker(600000, 15, 1358959400, 1639, "Who disturbs my water children!?"),
	["1a_earthElementalCamp01_general_STEPUP"] = StepUpTracker(600000, 8, 1358960407, 1690, "Who dare attack the earth of my earth!?"),
	["1a_hedgehog/warthogCamp01_trees_STEPUP"] = StepUpTracker(600000, 7, 1358960662, 1674, "Who bothers my prickly offspring!?"),
	["1a_earthElementalCamp02_general_STEPUP"] = StepUpTracker(600000, 20, 1358961865, 1668, "Who dare attack the earth of my earth!?")
};

function on_package_kill(packageName) {
	if(packageName in trackers) {
		trackers[packageName].on_package_kill();
	}
}