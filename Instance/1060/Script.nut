#!/bin/sq

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Instance script for New Corsica"
}

//
// Some handling for quest 1007 - 'Base Of Operations'
//

/* A custom class that encapsulates details about related props */

class PropRelationship {
	mPropId = 0;
	mBuiltPropId = "";
	
	constructor(propId, builtPropId) {
		mPropId = propId;
		mBuiltPropId = builtPropId;
	}
}

/* An array of towers */
towers <- [ 
	PropRelationship(1000033,1000034),
	PropRelationship(1000035,1000036)
];

function on_use_finish_7894(cid) {
	// Find the tower
	foreach(tower in towers) {
		if(inst.get_cid_for_prop(tower.mPropId) == cid) {
			// Spawn the large version!
			
			local towerCid = inst.spawn_prop(tower.mBuiltPropId);
			
			inst.queue(function() { despawn(cid) }, 1000);
			inst.queue(function() { despawn(towerCid) }, 10000);
			
			// Done
			break;
		}
	}
}

/* An array of barricades */
barricades <- [ 
	PropRelationship(1000045,1000046),
	PropRelationship(1000047,1000048)
];
 
function on_use_finish_7896(cid) {
	// Find the barricade
	foreach(barricade in barricades) {
		if(inst.get_cid_for_prop(barricade.mPropId) == cid) {
			
			local barricadeCid = inst.spawn_prop(barricade.mBuiltPropId);
			
			inst.queue(function() { despawn(cid) }, 1000);
			inst.queue(function() { despawn(barricadeCid) }, 10000);
			
			// Done
			break;
		}
	}
}


/* An array of spike traps */
traps <- [ 
	PropRelationship(1000052,1000053),
	PropRelationship(1000054,1000055)
];
 
function on_use_finish_7898(cid) {
	// Find the trap
	foreach(trap in traps) {
		if(inst.get_cid_for_prop(trap.mPropId) == cid) {
			
			local trapCid = inst.spawn_prop(trap.mBuiltPropId);
			
			inst.queue(function() { despawn(cid) }, 1000);
			inst.queue(function() { despawn(trapCid) }, 10000);
			
			// Done
			break;
		}
	}
}

/* Bring the boats in! */

function boat1In() {
	local move = {
		["type"] = "translate",
		["start"] = [14591.6,1083.23,14997.1],
		["end"] = [14891.9,1083.23,14044]
	};
	local table = {
		["frames"] = [ {
			["duration"] = 10.0,
			["transforms"] = [ move ]
			} ]
	};
	inst.transform(1029106, table);
}

function boat1Out() {
	local move = {
		["type"] = "translate",
		["start"] = [14791.9,1083.23,14544],
		["end"] = [14591.6,1083.23,14997.1]
	};
	local table = {
		["frames"] = [ {
			["duration"] = 10.0,
			["transforms"] = [ move ]
			} ]
	};
	inst.transform(1029106, table);
}

//
// Example ad-hoc quest
//

function on_death(cid, cdefid) {
	if(cdefid == 7876) {
	
		/* Get everyone who actually attacked */
		
		local attacked = inst.get_hated(cid);
	
		/* First send quest invitation to everyone who attacked */
		foreach(i in attacked) {
			inst.join_quest(i, 906, false);
		}
	
		/* Then spawn the ambushers anyway :) */
		local cids = [ inst.spawn(1133310,0,0), inst.spawn(1133311,0,0), inst.spawn(1133309,0,0) ];	
	
		/* Make each of the ambushers pick a random target from everyone who attacked */
		foreach(cid in cids) {
			inst.set_target(cid, attacked[randmodrng(0, attacked.len())]);
			inst.ai(cid, "tryMelee");
		} 

	}
}


