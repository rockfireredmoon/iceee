/*
 * Valkals' Bloodkeep
 *
 * This is the most ambitious of any script so far. It consists of two separate
 * sequences, different adds, custom abilties and lots of scripted movement.
 *
 * It also calls in AI scripts and demostrates an engage / disengage technique.
 */

::info <- {
	name = "Valkals Blood Keep",
	author = "Emerald Icemoon",
	description = "Handles the final fight!",
	queue_events = true
}

// Supporting classes
class VampireAdd {
    constructor(prop_id, pos, rot) {
        this.prop_id = prop_id;
        this.pos = pos;
        this.rot = rot;
    }
    
    function tile() {
    	return Point(this.pos.x / SPAWN_TILE_SIZE, this.pos.z / SPAWN_TILE_SIZE);
    }

    rot = 0;
    pos = null;
    prop_id = null;
}

// Abilities
const HEALING_SCREAM = 24009;
const SELF_STUN = 24008;
const TRIBUTE = 24005;
const BRINGING_DOWN_THE_HOUSE = 24006;
const VALKALS_INFERNO = 24010;

// Creature Definition IDs
const CDEF_VALKAL1 = 1385;
const CDEF_VALKAL2 = 3351;
const CDEF_CANDELABRA_1 = 7873;

// The locations of the room
loc_chamber <- Area(5679, 1842, 5953, 2225);
loc_throne_room <- Area(4160,2556, 4336, 2981);
loc_trap_door <- Point(4363, 2769);
loc_platform_centre <- Point(4300, 2788);
loc_chamber_platform_centre <- Point(5821, 1861);
loc_chamber_platform_front <- Point(5821, 1936);
loc_platform_front <- Point(4141, 2790);
loc_room_centre <- Point(3971, 2790);
loc_chamber_room_centre <- Point(5825, 2120);
loc_platform_rot <- 4;

// Adds

valkal_1_mehirim_adds <- [
    VampireAdd(1150681, Point(4155,2697), 214),
    VampireAdd(1150687, Point(4159,2874), 168),
    VampireAdd(1150683, Point(3861,2901), 92),
    VampireAdd(1150684, Point(3859,2683), 40),
];

valkal_1_palatine_adds <- [
    VampireAdd(1150679, Point(4011,2931), 128),
    VampireAdd(1150680, Point(4011,2631), 0),
    VampireAdd(1150685, Point(3803,2820), 66),
    VampireAdd(1150686, Point(3805,2762), 63)
];

valkal_2_adds <- [
    VampireAdd(1154484, Point(5700,2044), 11),
    VampireAdd(1154485, Point(5742,2055), 12),
    VampireAdd(1154486, Point(5764,2117), 38),
    VampireAdd(1154487, Point(5771,2215), 89),
    VampireAdd(1154488, Point(5936,2051), 237),
    VampireAdd(1154489, Point(5900,2046), 238),
    VampireAdd(1154490, Point(5857,2120), 208),
    VampireAdd(1154491, Point(5865,2211), 183)
];

// Prop IDS

const VALKAL2_SPAWN_PROP = 1150652;
const TRAP_DOOR_SPAWN_PROP = 1150656;

// Item IDS
const BOOK_ITEMID = 8707;
const BOOK_NO = 3;

// Tile locationsd
valkal2_spawn_tile <- Point(10,3);

// State variables
cid_valkal1 <- 0;
cid_valkal2 <- 0;
phase <- 0;
valkal1_full_health_count <- 0;
valkal2_full_health_count <- 0;
lit_candelabras <- [];

// Debug
debug <- true;
manual_trigger <- false;

function is_usable(cid, cdef_id, by_cid, by_cdef_id) {
	print("is_usable " + cid + "/" + cdef_id + "\n");
	if(cdef_id == CDEF_CANDELABRA_1 && !array_contains(lit_candelabras, cid)) {
		return "Y";
	}
	return "N";
}

function on_use(cid, target_cid, target_cdef_id) {
	if(target_cdef_id == CDEF_CANDELABRA_1) {
		inst.interact(cid, "Lighting the candelabra", 3000, false, function() {
			lit_candelabras.append(cid);			
			local candelabra_prop = inst.creature_spawn_prop(target_cid);
			inst.asset(candelabra_prop, "CL-Candelabra2", 1.5);
			return true;
		});
		return true;
	}
}

/* Keep scanning for Valkal1 until he spawns. When he does, we start to monitor his health  */ 
function find_valkal1() {
	cid_valkal1 = inst.scan_npc(loc_throne_room, CDEF_VALKAL1);
	if(cid_valkal1 == 0)
		inst.queue(find_valkal1, 5000);
	else {
		if(debug)
			inst.info("Found Valkal 1");
		valkal1_health();
	}
}

/* Keep scanning for Valkal2 until he spawns. When he does, we start to monitor his health  */ 
function find_valkal2() {
	cid_valkal2 = inst.scan_npc(loc_chamber, CDEF_VALKAL2);
	if(cid_valkal2 == 0)
		inst.queue(find_valkal2, 5000);
	else {
		if(debug)
			inst.info("Found Valkal 2");
		valkal2_health();
	}
}

/* Helper function to find closest creature to another (player or NPC) */
function find_closest(cid, player) {

    /* Find all players, sidekicks and enemies of the minion within the range */
    local targets = inst.get_nearby_creature(10000, cid, 
    		player ? TS_ENEMY_ALIVE : TS_NONE, player ? TS_ENEMY_ALIVE : TS_NONE, player ? TS_ENEMY_ALIVE : TS_NONE);
    local closest_cid = -1;
    local closest_distance = 9999999;

    /* Find the closest enemy */
    foreach(t in targets) {
        local dist = inst.get_creature_distance(t, cid);
        if(dist > -1 && dist < closest_distance) {
            closest_distance = dist;
            closest_cid = t;
        }
    }
    
    return closest_cid;
}

/* Helper function to find closest creature and melee attack it  */
function attack_closest(cid) {

    local closest_cid = find_closest(cid, true);

    /* Target and attack the found enemy */
    if(closest_cid != -1) {
        inst.set_target(cid, closest_cid);
        inst.creature_use(cid, _AB("melee"));
    }
}

/*  Helper function to Spawn Valkals adds. Each one will be instructed to run to a defined place,
   then locate the nearest player and attack them (or anyone who aggros them on the way). */
function spawn_adds(max, adds_list) {

    max = max.tointeger();
	if(debug)
		inst.info("Spawing " + max + " adds");

    local l = [];
    foreach(v in adds_list)
        l.append(v);

    for(local i = 0 ; i < adds_list.len() - max ; i++)
        l.remove(randmodrng(0, l.len()));

	if(debug)
		inst.info("Picking from " + l.len() + " adds");
		
    foreach(v in l) {
		local cid = inst.spawn(v.prop_id, 0, 0);
		if(debug)
			inst.info("Walking to "  +v.pos + " then Spawning " + cid + " (" + v.prop_id + ")");
        inst.walk_then(cid, v.pos, -1, CREATURE_JOG_SPEED, 0, function(res) {
        
			if(debug)
				inst.info("Walking to "  + v.pos + " then Spawning " + cid);
			
        	if(res == RES_OK) {
	        	inst.rotate_creature(cid, v.rot);
	            attack_closest(cid);
	        }
	        else {
				if(debug)
					inst.info("Spawn failed " + cid + " / " + res);
			}
        });
    }
}

/*  Helper function to totally disengage Valkal from the fight and make him invincible */
function disengage_valkal(cid) {
	if(debug)
		inst.info("Disengaging Valkal");
		
	inst.interrupt(cid);
	inst.leave_combat(cid);
	inst.set_flag(cid, SF_NON_COMBATANT, true);
	inst.pause_ai(cid);
}

/*  Helper function to return Valkal 1 to the center of the room, and picks the closest player
   to attack */
function valkal_1_engage() {
	if(debug)
		inst.info("Engaging");
	inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
	inst.resume_ai(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_room_centre, -1, CREATURE_JOG_SPEED, 0, function(res) {
		if(res == RES_OK) {
			/* If we got to the centre of the room without aggro attack the closes */
        	attack_closest(cid_valkal1);
        }
        
        /* Start health check loop again */
		inst.exec(valkal1_health);
	});
}

/*  Helper function to return Valkal 2 to the center of the room, and picks the closest player
   to attack */
function valkal_2_engage() {
	if(debug)
		inst.info("Engaging");
	inst.set_flag(cid_valkal2, SF_NON_COMBATANT, false);
	inst.resume_ai(cid_valkal2);
	inst.walk_then(cid_valkal2, loc_chamber_room_centre, -1, CREATURE_JOG_SPEED, 0, function(res) {
		if(res == RES_OK) {
			/* If we got to the centre of the room without aggro attack the closes */
        	attack_closest(cid_valkal2);
        }
        
        /* Start health check loop again */
		inst.exec(valkal2_health);
	});
}

/* Keeping a creature self healing until they gets to a limit, then queue
 * another function */
function heal(cid, limit, on_healed) {
	if(inst.get_health_pc(cid) < limit) {
    	inst.creature_use(cid, HEALING_SCREAM);
    	inst.exec(function() {
    		heal(cid, limit, on_healed);
    	});
    }
    else
    	inst.exec(on_healed);
}

/* Starts the sequence of Valkal 1 running from the fight his platform to
   heal. This Valkal spawns NO adds */
function valkal_1_heal_sequence() {
    if(debug)
		inst.info("Heal sequence");
    disengage_valkal(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_platform_centre, -1, CREATURE_JOG_SPEED, 0, function(res) {
        inst.rotate_creature(cid_valkal1, loc_platform_rot);
        heal(cid_valkal1, 95, valkal_1_engage); 
	});
}

/* Starts the sequence of Valkal 2 running from the fight his platform to
   heal. This Valkal also spawns some adds while he does */
function valkal_2_heal_sequence() {
    if(debug)
		inst.info("Heal sequence");
	spawn_adds(valkal_2_adds.len() / 2, valkal_2_adds);
    disengage_valkal(cid_valkal2);
	inst.walk_then(cid_valkal2, loc_chamber_platform_centre, -1, CREATURE_JOG_SPEED, 0, function(res) {
        inst.rotate_creature(cid_valkal2, loc_platform_rot);
        heal(cid_valkal2, 95, valkal_2_engage); 
	});
}

/* Valkal 2. He is in another tile that probably is not loaded, so make sure it is and
   spawn after a short delay */
function spawn_valkal_2() {
	/* Reset the phase */
	phase = 0;

    if(debug)
		inst.info("Spawning Valkal2");
	inst.load_spawn_tile(valkal2_spawn_tile);
	
	/* Preload add tiles as well */
	foreach(v in valkal_2_adds) {
		inst.load_spawn_tile(v.tile());
    }
    
	inst.queue(function() {
		inst.spawn(VALKAL2_SPAWN_PROP, 0, 0);
		/* Wait a bit more, and start looking for Valkal 2 */
		inst.queue(find_valkal2, 3000);
	}, 5000);
}

/* Starts the sequence of Valkal running from the throne room to the
   Sanctum */
function valkal_flee() {

   	inst.creature_chat(cid_valkal1, "s/", "This is not over...");
    disengage_valkal(cid_valkal1);
	inst.queue(function() {
		inst.walk_then(cid_valkal1, loc_trap_door, -1, CREATURE_RUN_SPEED, 0, function(res) {
			inst.creature_chat(cid_valkal1, "s/", "... Follow if you dare");
        	inst.rotate_creature(cid_valkal1, 192);
			inst.queue(function() {
				inst.despawn(cid_valkal1)
			}, 1000);

			// Portal
			inst.spawn(TRAP_DOOR_SPAWN_PROP, 0, 0);
            spawn_valkal_2();
		});
	}, 1000);
}

/* Starts the Tribute sequence. Runs Valkal to just in front of the platform, picks a player to 
   Tribute, and does so. Valkal is then stunned for 15 seconds before running back to centre
   and picking another player to fight */
function tribute() {
    if(debug)
		inst.info("Tribute");
	spawn_adds(valkal_2_adds.len() / 2, valkal_2_adds);
    disengage_valkal(cid_valkal2);
	inst.walk_then(cid_valkal2, loc_chamber_platform_front, 192, CREATURE_JOG_SPEED, 0, function(res) {
        if(debug)
        	inst.info("Valkal home");
        inst.queue(function() {
        	inst.info("Finding targets");
	        local targets = inst.get_nearby_creature(300, cid_valkal2, TS_ENEMY_ALIVE, TS_ENEMY_ALIVE, TS_ENEMY_ALIVE);
            foreach(l in targets) {
                inst.info("  " + l + " " + inst.get_display_name(l));
            }
	        if(targets.len() > 0) {
	            local cid = targets[randmodrng(0, targets.len())];
	            inst.set_target(cid_valkal2, cid);
	            if(debug)
	            	inst.info("Targetted " + inst.get_display_name(cid));
				inst.set_flag(cid_valkal2, SF_NON_COMBATANT, false);
	            if(!inst.creature_use(cid_valkal2, TRIBUTE)) {
	            	if(debug)
	                	inst.info("Failed to tribute " + cid);
	            }
	            else {
	                inst.queue(function() {
	            		inst.target_self(cid_valkal2);
		            	if(inst.creature_use(cid_valkal2, SELF_STUN)) {
							inst.set_flag(cid_valkal2, SF_NON_COMBATANT, true);
					        inst.queue(function() {
								inst.leave_combat(cid_valkal2);  
					            valkal_2_engage();
					        }, 16000);
					    }
		            	else {	     
							inst.leave_combat(cid_valkal2);
	            			if(debug)       	
	                			inst.info("Stunned fail"); 
					        valkal_2_engage();
					    }
		            }, 6000);
	            }
	        }
	        else {
	            if(debug)       	
	            	inst.info("Found no players");            
				valkal_2_engage();
			}
	    }, 1000);
    });
}

/* Starts the Bringing Down The House sequence. Runs Valkal to just in front of the platform to
   cast on the the centre of the room */
function bringing_down_the_house() {
    disengage_valkal(cid_valkal2);
	inst.walk_then(cid_valkal2, loc_chamber_platform_front, 192, CREATURE_JOG_SPEED, 0, function(res) {
		inst.set_creature_gtae(cid_valkal2);
        if(!inst.creature_use(cid_valkal2, BRINGING_DOWN_THE_HOUSE)) {
        	if(debug)
            	inst.info("Failed to bring down the house");
            valkal_2_engage();
        }
        else {
            inst.queue(valkal_2_engage, 10000);
	    }
    });
}

/* Starts the Valkal's Inferno sequence. Runs Valkal to just in front of the platform to
   cast on the the centre of the room */
function valkals_inferno() {
    disengage_valkal(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_platform_front, 192, CREATURE_JOG_SPEED, 0, function(res) {
        if(!inst.creature_use(cid_valkal1, VALKALS_INFERNO)) {
        	if(debug)
            	inst.info("Failed to inferno");
            valkal_1_engage();
        }
        else {
            inst.queue(valkal_1_engage, 10000);
	    }
    });
}

/* Reset the Valkal1 fight if the party wipe out */
function reset_valkal1() {
	phase = 0;
    disengage_valkal(cid_valkal1);
   	inst.creature_chat(cid_valkal1, "s/", "Will no one face me? ... Cowards");
	valkal1_full_health_count = 0;
	inst.walk_then(cid_valkal1, loc_platform_centre, 192, CREATURE_WALK_SPEED, 0, function(res) {
		inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
	});
}

/* Reset the Valkal2 fight if the party wipe out */
function reset_valkal2() {
	phase = 0;
    disengage_valkal(cid_valkal2);
   	inst.creature_chat(cid_valkal2, "s/", "You've failed! You've all failed! ... You'll ALWAYS fail ....");
	valkal2_full_health_count = 0;
	inst.walk_then(cid_valkal2, loc_chamber_platform_centre, 192, CREATURE_WALK_SPEED, 0, function(res) {
		inst.set_flag(cid_valkal2, SF_NON_COMBATANT, false);
	});
}

/* Monitor Valkal1's health and trigger the various fight stages. */
function valkal1_health() {
	if(cid_valkal1 == 0 || manual_trigger)
		return;

	local health = inst.get_health_pc(cid_valkal1);

	if(health == -1) {
		/* Valkal creature has disappeared (maybe an admin resetting a spawn?) 
		 * so reset this stage entirely */
		phase = 0;
		inst.info("Resetting Valkal1");
		inst.exec(find_valkal1);
	}
	else if(health == 100) {
		if(valkal1_full_health_count == 12 && inst.is_at_tether(cid_valkal1)) {
			// After a while at full health, reset entirely
			reset_valkal1();
		}
		else
			valkal1_full_health_count++;
			
		// Slower check
		inst.queue(valkal1_health, 5000);
	}
    else {
    	valkal1_full_health_count = 0;
    	
	    if(health <= 5 && phase < 9) {
	        // Flee and leave this loop
	        if(debug)
	        	inst.info("Flee!");
	        valkal_flee();
	        return;
	    }
	    else if(health <= 15 && phase < 8) {
	        if(debug)
	        	inst.info("Inferno");
	        phase = 8;
			valkals_inferno();
			return;
	    }
	    else if(health <= 30 && phase < 7) {
	        if(debug)
	        	inst.info("Spawn 5");
	        phase = 7;
			spawn_adds(valkal_1_mehirim_adds.len(), valkal_1_mehirim_adds);
			spawn_adds(valkal_1_palatine_adds.len() / 2, valkal_1_palatine_adds);
	    }
	    else if(health <= 40 && phase < 6) {
	        if(debug)
	        	inst.info("Spawn 4");
	        phase = 6;
			spawn_adds(valkal_1_mehirim_adds.len() / 2, valkal_1_mehirim_adds);
			spawn_adds(valkal_1_palatine_adds.len() / 2, valkal_1_palatine_adds);
	    }
	    else if(health <= 50 && phase < 5) {
	        if(debug)
	        	inst.info("Heal 2");
	        phase = 5;
	        valkal_1_heal_sequence();
	        return;
	    }
	    else if(health <= 60 && phase < 4) {
	        if(debug)
	        	inst.info("Spawn 3");
	        phase = 4;
			spawn_adds(valkal_1_mehirim_adds.len(), valkal_1_mehirim_adds);
	    }
	    else if(health <= 75 && phase < 3) {
	        if(debug)
	        	inst.info("Heal 1");
	        phase = 3;
	        valkal_1_heal_sequence();
	        return;
	    }
	    else if(health <= 80 && phase < 2) {
	        if(debug)
	        	inst.info("Spawn 2");
	        phase = 2;
			spawn_adds(valkal_1_palatine_adds.len() / 2, valkal_1_palatine_adds);
	    }
	    else if(health <= 90 && phase < 1) {
	        if(debug)
	        	inst.info("Spawn 1");
	        phase = 1;
			spawn_adds(valkal_1_mehirim_adds.len(), valkal_1_mehirim_adds);
	    }
	    
		inst.exec(valkal1_health);
	}
}

/* Monitor Valkal2's health and trigger the various fight stages. */
function valkal2_health() {
	if(cid_valkal2 == 0 || manual_trigger)
		return;

	local health = inst.get_health_pc(cid_valkal2);
	if(debug)
		inst.info("Valkal at " + health + ". Phase " + phase + " FHC " + valkal2_full_health_count);

	if(health == -1) {
		/* Valkal creature has disappeared (maybe an admin resetting a spawn?) 
		 * so reset this stage entirely */
		phase = 0;
		inst.info("Resetting Valkal2");
		inst.exec(find_valkal2);
	}
	else if(health == 100) {
		if(valkal2_full_health_count == 12 && inst.is_at_tether(cid_valkal2)) {
			// After a while at full health, reset entirely
			reset_valkal2();
		}
		else
			valkal2_full_health_count++;
			
		// Slower check
		inst.queue(valkal2_health, 5000);
	}
    else {
    	valkal1_full_health_count = 0;
		if(health <= 10 && phase < 9) {
	        // Bringing Down the House 3
	        if(debug)
	        	inst.info("Bringing Down the House 3");
	        phase = 9;        
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 20 && phase < 8) {
	        // Bringing Down the House 2
	        if(debug)
	        	inst.info("Bringing Down the House 2");
	        phase = 8;
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 25 && phase < 7) {
	        // Heal 3
	        if(debug)
	        	inst.info("Heal 3");
	        phase = 7;
	        valkal_2_heal_sequence();
	        return;
	    }
	    else if(health <= 30 && phase < 6) {
	        // Bringing Down the House 1
	        if(debug)
	        	inst.info("Bringing Down the House 1");
	        phase = 6;
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 40 && phase < 5) {
	        // Tribute 3
	        if(debug)
	        	inst.info("Tribute 3");
	        phase = 5;
	        tribute();
	        return;
	    }
	    else if(health <= 50 && phase < 4) {
	        // Heal 2
	        if(debug)
	        	inst.info("Heal 2");
	        phase = 4;
	        valkal_2_heal_sequence();
	        return;
	    }
	    else if(health <= 60 && phase < 3) {
	        // Tribute 2
	        if(debug)
	        	inst.info("Tribute 2");
	        phase = 3;
	        tribute();
	        return;
	    }
	    else if(health <= 75 && phase < 2) {
	        // Heal 1
	        if(debug)
	        	inst.info("Heal 1");
	        phase = 2;
	        valkal_2_heal_sequence();
	        return;
	    }
	    else if(health <= 80 && phase < 1) {
	        // Tribute 1
	        if(debug)
	        	inst.info("Tribute 1");
	        phase = 1;
	        tribute();
	        return;
	    }
	    
		inst.exec(valkal2_health);
	}

}

/* Debug function that can be run externally to set the phase */
function debug_setphase(p) {
	inst.info("Phase is now " + p);
	phase = p.tointeger();
}

/* Initialisation. Start scanning for Valkal1 to spawn */
inst.exec(find_valkal1);