#!/bin/sq

/* Array of CDefID used for the lamp posts. There may be as many instances of each as you wish. As soon
   as one is clicked, the rest will light. */
LAMP_POST_CDEFID <- [ 7880, 7881, 7882, 7883, 7884, 7885, 7886 ];

/* Array of Light props that are associated with each lamp post. The size and indexes match LAMP_POST_CDEFID.
   Each light prop will be near at least one lamp (if there are multiple they should used the same CDefID) */
LIGHT_PROPS <- [ 1275068514, 1275068515, 1275068516, 1275068517, 1275068518, 1275068519, 1275068520 ];

/* Array of CDefID uses for the NPC's that are being escorted */
ESCORTED_CDEFID <- [ 7878 ];

/* Table of locations each NPC will walk to as they are lit. Accounts for 3 NPCs). The 
   location is where they will be returned to if no lights are lit. There is an additional position at
   the end of the array which is the point past the furthest lightable lamp they will walk to */

POSITIONS <- [
	[ Point(850,695), Point(875,695), Point(909,692) ],
	[ Point(888,941), Point(873,933), Point(853,952) ],
	[ Point(856,1197), Point(861,1183), Point(871,1161) ],
	[ Point(588,1145), Point(564,1166), Point(593,1188) ],
	[ Point(585,1623), Point(601,1644), Point(615,1669) ],
	[ Point(889,1658), Point(865,1658), Point(831,1670) ],
	[ Point(863,2214), Point(863,2214), Point(864,2165) ],
	[ Point(655,2211), Point(677,2210), Point(708,2220) ]
	];

/* Tracks the currently lit lamp posts */
lit <- {};

/*
 * Function to determine which lamp post the NPC's may walk to, i.e the
 * furthest completely lit path. A value of zero indicates no lamps are
 * yet lit
 */
function get_max_lamp() {
	local i = 0;
	for( ; i < LAMP_POST_CDEFID.len() ; i++) {
		if(!(LAMP_POST_CDEFID[i] in lit)) 
			return i;
	}
	return i;
}

function configure_movement() {
	local positions = POSITIONS[get_max_lamp()];
	
	
	local idx = 0;
	foreach(cdefid in ESCORTED_CDEFID) {
		foreach(cid in inst.cids(cdefid)) {
		
			inst.info("Walking " + cid + " to " + positions[idx].x + "," + positions[idx].z); 
			inst.walk_then(cid, positions[idx], CREATURE_RUN_SPEED, 0, function() { 
			});
			
			/* Move the index to the next location to use. If there are more creatures than positions,
			 * start again and re-use the same ones. TODO add offset for each loop? */
			 
			idx++;
			if(idx == positions.len()) 
				idx = 0;
		}
	} 
	
}

/*
 * Called when creatures are used. Will determine if the creature is a lamp post, and whether to
 * light or extinguish it.
 */
function on_use(sourceCID, targetCId, usedCDefID) {
	//if(usedCDefID in LAMP_POST_CDEFID) {
	
	foreach(l in LAMP_POST_CDEFID) {
		if(l == usedCDefID) {
			if(usedCDefID in lit) {
				inst.info("You extinguish the lamp, the darkness and fear creeps back in ..");
				delete lit[usedCDefID];		
			}
			else {
				inst.info("You light the lamp, burning back the darkness and fear ..");
				lit[usedCDefID] <- true;
			}
			
			
			configure_all_lamp_posts();
			configure_movement();
			
			break;
		}
	}
}

/*
 * Function to configure all known lamp posts based on the current state. If
 * lamps have been lit, the lit asset will be set (and vice versa), and any
 * associated lighting will be turned on or off
 */
function configure_all_lamp_posts() {
	for(local i = 0; i < LAMP_POST_CDEFID.len(); i++) {
	
		local cdefid = LAMP_POST_CDEFID[i];
		local islit = cdefid in lit;
		
		if(islit)
			inst.unremove_prop(LIGHT_PROPS[i]);
		else 
			inst.remove_prop(LIGHT_PROPS[i]);
				 
		foreach(cid in inst.cids(cdefid)) {
			inst.set_status_effect(cid, "IS_USABLE", -1);
			inst.set_status_effect(cid, "USABLE_BY_SCRIPT", -1);
			local propId = inst.get_spawn_prop(cid);
			if(propId > -1) {
				if(islit) {
					inst.asset(propId, "Prop-Lamp_Post_NB1", 1);
				}
				else if(!islit) {
					inst.asset(propId, "Prop-Lamp_Post_NB_Unlit", 1);
				}
			}
		}
	}
}

/* Initialisation */
configure_all_lamp_posts();
configure_movement();
