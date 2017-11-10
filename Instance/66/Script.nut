/*
 * Valkals' Bloodkeep
 *
 * This is the most ambitious of any script so far. It consists of two separate
 * sequences, different adds, custom abilties and lots of scripted movement.
 *
 * It also calls in AI scripts and demostrates an engage / disengage technique.
 *
 * This script requests a larger VM than usual, but still only 4K. It is
 * doubtful there would be more than one or two of these instances running at
 * any one time.
 */

::info <- {
	name = "Valkals Blood Keep",
	author = "Emerald Icemoon",
	description = "Handles the final fight!",
	queue_events = true,
	vm_size = 4098
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

class TargetPos {
    constructor(pos, rot) {
        this.pos = pos;
        this.rot = rot;
    }
    
    rot = 0;
    pos = null;
}

// Abilities
const HEALING_SCREAM = 24009;
const SELF_STUN = 24008;
const TRIBUTE = 24005;
const BRINGING_DOWN_THE_HOUSE = 24006;
const THOUSAND_BATS = 24012;

// Creature Definition IDs
const CDEF_VALKAL1 = 1385;
const CDEF_VALKAL2 = 3351;
const CDEF_VAJ_1 = 3494;
const CDEF_VAJ_2 = 3495;

// The locations of the room
loc_chamber <- Area(5679, 1842, 5953, 2225);
loc_throne_room <- Area(4160,2556, 4336, 2981);
loc_trap_door <- Point(4363, 2769);
loc_platform_centre <- Point(4300, 2788);
loc_platform_front <- Point(4141, 2790);
loc_room_centre <- Point(3971, 2790);
loc_chamber_room_centre <- Point(5825, 2120);
loc_platform_rot <- 192;

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

valkal_2_adds_left <- [
    VampireAdd(1154484, Point(5700,2044), 11),
    VampireAdd(1154485, Point(5742,2055), 12),
    VampireAdd(1154486, Point(5764,2117), 38),
    VampireAdd(1154487, Point(5771,2215), 89)
];

valkal_2_adds_right <- [
    VampireAdd(1154488, Point(5936,2051), 237),
    VampireAdd(1154489, Point(5900,2046), 238),
    VampireAdd(1154490, Point(5857,2120), 208),
    VampireAdd(1154491, Point(5865,2211), 183)
];

/* Positions valkal 2 walks to while his Vaj's are active */
valkal_2_positions <- [
	TargetPos(Point(5708,1856), 18),
	TargetPos(Point(5748,1912), 6),
	TargetPos(Point(5816,1918), 254),
	TargetPos(Point(5820,1918), 255),
	TargetPos(Point(5891,1917), 239),
	TargetPos(Point(5943,1870), 241)
];

/* Phrases spoken by valkal 2 while Vaj's are active */
valkal_2_phrases <- [
	"Teach them the meaning of pain my son.",
	"You think you'll live? Hah!.",
	"Your blood belongs to me now.",
	"Blood of my blood .. show these little beasts why they can never win.",
	"Fools! you should never have entered here. There can be only one outcome.",
	"See how my family protect me, I am Eternal!",
	"Taste them my kin..."
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
death_toll <- [];
env <- "";
finished <- false;

// Debug
no_adds <- false;
debug <- true;
verbose_debug <- false;
manual_trigger <- false;

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
        return true;
    }
    else
    	return false;
}

/*  Helper function to Spawn Valkals adds. Each one will be instructed to run to a defined place,
   then locate the nearest player and attack them (or anyone who aggros them on the way). */
function spawn_adds(max, adds_list) {
	if(no_adds) {		
		inst.info("Simulate spawn of " + max + " of adds [adds disabled to aid testing]");
		return [];
	}

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
		
	local spawns = [];
    foreach(v in l) {
		local cid = inst.spawn(v.prop_id, 0, 0);
		spawns.append(cid);
		if(debug)
			inst.info("Walking to "  + v.pos + " then attacking. " + cid + " (" + v.prop_id + ")");
        inst.walk_then(cid, v.pos, v.rot, CREATURE_JOG_SPEED, 0, function(res) {
        	if(res == RES_OK) {
	            attack_closest(cid);
	        }
	        else {
				if(debug)
					inst.info("Walk interrupted " + cid + " / " + res);
			}
        });
    }
    
    return spawns;
}

/*  Helper function to totally disengage Valkal from the fight and make him invincible */
function disengage_valkal(cid) {
	if(debug)
		inst.info("Disengaging Valkal");
		
	inst.pause_ai(cid);
	inst.leave_combat(cid);
	inst.unhate(cid);
	inst.interrupt(cid);
	inst.set_flag(cid, SF_NON_COMBATANT, true);
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
        	if(!attack_closest(cid_valkal1)) {
        		reset_valkal1();
        	}
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
			/* If we got to the centre of the room without aggro attack the closest */
        	if(!attack_closest(cid_valkal2)) {
        		reset_valkal2();
        		
				if(debug)
					inst.info("Reset after failed attack");
					
					
        		heal(cid_valkal2, 100, function() {
        			valkal2_health();
        		});
        		return; 
        	}
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
   heal. */
function valkal_1_heal_sequence() {
    if(debug)
		inst.info("Heal sequence");
    disengage_valkal(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_platform_centre, loc_platform_rot, CREATURE_JOG_SPEED, 0, function(res) {
        heal(cid_valkal1, 95, valkal_1_engage); 
	});
}

/* Starts the sequence of Valkal 2 running from the fight his platform to
   heal. Once healed he will wait for the all of the spawns in the
   list provided to be killed before returning to the action 
 */
function valkal_2_heal_sequence(spawns) {
    if(debug)
		inst.info("Heal sequence");
    disengage_valkal(cid_valkal2);
	inst.walk_then(cid_valkal2, valkal_2_positions[3].pos, valkal_2_positions[3].rot, CREATURE_JOG_SPEED, 0, function(res) {
        heal(cid_valkal2, 95, function() {
        	if(debug)
        		inst.info("Looking for spot to walk to");
       		valkal_2_pick_spot(spawns);
        }); 
	});
}

/* Return true if every spawn in the list is dead */
function is_list_dead(l) {
    if(debug)
		inst.info("is_list_dead " + l.len());
	local dead = 0;
	foreach(s in l) {
	    if(debug)
			inst.info("test " + s);
		if(array_contains(death_toll, s))
			dead++;
	    if(debug)
			inst.info("dead now " + dead);
	}
	return dead == l.len();
}

/* Pick a spot for valkal 2 to walk to while the list of spawns are not yet dead */
function valkal_2_pick_spot(spawns) {
	if(is_list_dead(spawns))  {
	    if(debug)
			inst.info("list of " + spawns.len() + " now dead, engaging");      	
    	valkal_2_engage();
	}
    else {
	    if(debug)
			inst.info("pick spot");      	
			
    	/* Pick a spot to run to */
	    local spot = valkal_2_positions[randmodrng(0, valkal_2_positions.len())];
		inst.walk_then(cid_valkal2, spot.pos, spot.rot, CREATURE_RUN_SPEED, 0, function(res) {
			
	    	/* Pick a phrase to say  */
	    	local phrase = valkal_2_phrases[randmodrng(0, valkal_2_phrases.len())];
	    	
			inst.creature_chat(cid_valkal2, "s/", phrase);
        	
        	/* Pick another spot */    	
	    	inst.queue(function() {
	    		valkal_2_pick_spot(spawns);
	    	}, 5000);
		});
    }	
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
	foreach(v in valkal_2_adds_left)
		inst.load_spawn_tile(v.tile());
    foreach(v in valkal_2_adds_right)
    	inst.load_spawn_tile(v.tile());
    
	inst.queue(function() {
		inst.spawn(VALKAL2_SPAWN_PROP, 0, 0);
		/* Wait a bit more, and start looking for Valkal 2 */
		inst.queue(find_valkal2, 3000);
	}, 5000);
}

/* Remove the fight scene environment if it is set */
function tod(e) {
    if(e != env) {
    	env = e;
    	inst.set_timeofday(e);
    }
}

/* Starts the sequence of Valkal running from the throne room to the
   Sanctum */
function valkal_flee() {

   	inst.creature_chat(cid_valkal1, "s/", "This is not over...");
    disengage_valkal(cid_valkal1);
    tod("Day");
	inst.queue(function() {
		inst.walk_then(cid_valkal1, loc_trap_door, 192, CREATURE_RUN_SPEED, 0, function(res) {
			inst.creature_chat(cid_valkal1, "s/", "... Follow if you dare");
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
    disengage_valkal(cid_valkal2);
	inst.walk_then(cid_valkal2, valkal_2_positions[2].pos, valkal_2_positions[2].rot, CREATURE_RUN_SPEED, 0, function(res) {
        if(debug)
        	inst.info("Valkal home");
        inst.queue(function() {
        	inst.info("Finding targets");
	        local targets = inst.get_nearby_creature(300, cid_valkal2, TS_ENEMY_ALIVE, TS_ENEMY_ALIVE, TS_ENEMY_ALIVE);
	        if(debug) {
	            foreach(l in targets) {
	                inst.info("  " + l + " " + inst.get_display_name(l));
	            }
	        }
	        if(targets.len() > 0) {
	            local cid = targets[randmodrng(0, targets.len())];
	            inst.set_target(cid_valkal2, cid);
	            if(debug)
	            	inst.info("Targetted " + inst.get_display_name(cid));
				
	            if(!inst.creature_use(cid_valkal2, TRIBUTE)) {
	            	if(debug)
	                	inst.info("Failed to tribute " + cid);
	            }
	            else {
	                inst.queue(function() {
	            		inst.target_self(cid_valkal2);
		            	if(inst.creature_use(cid_valkal2, SELF_STUN)) {
					        inst.queue(function() {
								inst.leave_combat(cid_valkal2);  
					            valkal_2_engage();
					        }, 12000);
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
	inst.walk_then(cid_valkal2, valkal_2_positions[2].pos, 192, CREATURE_JOG_SPEED, 0, function(res) {
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

/* Starts the Valkal's desperation sequence. Runs Valkal to just in front of the platform to
   cast on the the centre of the room */
function valkals_desperation() {
    disengage_valkal(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_platform_front, 192, CREATURE_JOG_SPEED, 0, function(res) {
		inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
		inst.resume_ai(cid_valkal1);
		inst.set_creature_gtae(cid_valkal1);
        if(!inst.creature_use(cid_valkal1, THOUSAND_BATS)) {
        	if(debug)
            	inst.info("Failed to THOUSAND_BATS");
            valkal_1_engage();
        }
        else {
            inst.queue(valkal_1_engage, 10000);
	    }
    });
}

/* Reset the Valkal1 fight if the party wipes out */
function reset_valkal1() {
	if(debug)
		inst.info("Restting valkal 1");
	phase = 0;
	tod("Day");
    disengage_valkal(cid_valkal1);
   	inst.creature_chat(cid_valkal1, "s/", "Will no one face me? ... Cowards");
	valkal1_full_health_count = 0;
	inst.walk_then(cid_valkal1, loc_platform_centre, 192, CREATURE_WALK_SPEED, 0, function(res) {
		inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
	});
}

/* Reset the Valkal2 fight if the party wipes out */
function reset_valkal2() {
	if(debug)
		inst.info("Restting valkal 2");
	phase = 0;
	death_toll = [];
	tod("Day");
    disengage_valkal(cid_valkal2);
   	inst.creature_chat(cid_valkal2, "s/", "You've failed! You've all failed! ... You'll ALWAYS fail ....");
	valkal2_full_health_count = 0;
	inst.walk_then(cid_valkal2, valkal_2_positions[2].pos, valkal_2_positions[2].rot, CREATURE_RUN_SPEED, 0, function(res) {
		inst.set_flag(cid_valkal2, SF_NON_COMBATANT, false);
	});
}

/* Monitor Valkal1's health and trigger the various fight stages. */
function valkal1_health() {
	if(cid_valkal1 == 0 || manual_trigger)
		return;

	local health = inst.get_health_pc(cid_valkal1);
	if(verbose_debug)
		inst.info("Valkal1 at " + health + ". Phase " + phase + " FHC " + valkal1_full_health_count);

	if(health == -1) {
		/* Valkal creature has disappeared (maybe an admin resetting a spawn?) 
		 * so reset this stage entirely */
		phase = 0;
		inst.info("Resetting Valkal1");
		inst.exec(find_valkal1);
	}
	else if(health == 100) {
		if(valkal1_full_health_count == 4 && inst.is_at_tether(cid_valkal1)) {
			// After a while at full health, reset entirely
			reset_valkal1();
		}
		else
			valkal1_full_health_count++;
			
		// Slower check
		inst.queue(valkal1_health, 5000);
	}
    else {
		tod("Sunset");
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
	        	inst.info("Desperation");
	        phase = 8;
			valkals_desperation();
			return;
	    }
	    else if(health <= 30 && phase < 7) {
	        if(debug)
	        	inst.info("Spawn 5");
	        phase = 7;
			spawn_adds(valkal_1_mehirim_adds.len(), valkal_1_mehirim_adds);
	    }
	    else if(health <= 40 && phase < 6) {
	        if(debug)
	        	inst.info("Spawn 4");
	        phase = 6;
			spawn_adds(valkal_1_mehirim_adds.len() / 2, valkal_1_mehirim_adds);
	    }
	    else if(health <= 50 && phase < 5) {
	        if(debug)
	        	inst.info("Heal 2");
	        phase = 5;
	        valkal_1_heal_sequence();
			spawn_adds(valkal_1_palatine_adds.len() / 2, valkal_1_palatine_adds);
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
			spawn_adds(valkal_1_palatine_adds.len() / 2, valkal_1_palatine_adds);
	        return;
	    }
	    else if(health <= 80 && phase < 2) {
	        if(debug)
	        	inst.info("Spawn 2");
	        phase = 2;
	    }
	    else if(health <= 90 && phase < 1) {
	        if(debug)
	        	inst.info("Spawn 1");
	        phase = 1;
			spawn_adds(valkal_1_mehirim_adds.len() / 2, valkal_1_mehirim_adds);
	    }
	    
		inst.exec(valkal1_health);
	}
}

/* Monitor Valkal2's health and trigger the various fight stages. */
function valkal2_health() {
	if(cid_valkal2 == 0 || manual_trigger || finished)
		return;

	local health = inst.get_health_pc(cid_valkal2);
	if(verbose_debug)
		inst.info("Valkal2 at " + health + ". Phase " + phase + " FHC " + valkal2_full_health_count);

	if(health == -1) {
		/* Valkal creature has disappeared (maybe an admin resetting a spawn?) 
		 * so reset this stage entirely */
		phase = 0;
		inst.info("Resetting Valkal2");
		inst.exec(find_valkal2);
	}
	else if(health == 100) {
		if(phase > 0) {
			if(valkal2_full_health_count == 40 && inst.is_at_tether(cid_valkal2)) {
				// After a while at full health, reset entirely
				reset_valkal2();
			}
			else
				valkal2_full_health_count++;
		}
			
		// Slower check
		inst.queue(valkal2_health, 5000);
	}
    else {
		tod("Sunset");
    		
    	valkal2_full_health_count = 0;

		if(health <= 10 && phase < 8) {
	        // Bringing Down the House 3
	        if(debug)
	        	inst.info("Bringing Down the House 3");
	        phase = 8;        
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 20 && phase < 7) {
	        // Bringing Down the House 2
	        if(debug)
	        	inst.info("Bringing Down the House 2");
	        phase = 7;
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 25 && phase < 6) {
	        // Heal 2
	        if(debug)
	        	inst.info("Heal 2");
	        phase = 6;
	        valkal_2_heal_sequence(spawn_adds(1, valkal_2_adds_right));
	        return;
	    }
	    else if(health <= 30 && phase < 5) {
	        // Bringing Down the House 1
	        if(debug)
	        	inst.info("Bringing Down the House 1");
	        phase = 5;
	        bringing_down_the_house();
	        return;
	    }
	    else if(health <= 40 && phase < 4) {
	        // Tribute 3
	        if(debug)
	        	inst.info("Tribute 3");
	        phase = 4;
	        tribute();
	        return;
	    }
	    else if(health <= 50 && phase < 3) {
	        // Heal 1
	        if(debug)
	        	inst.info("Heal 1");
	        phase = 3;
	        valkal_2_heal_sequence(spawn_adds(1, valkal_2_adds_left));
	        return;
	    }
	    else if(health <= 60 && phase < 2) {
	        // Tribute 2
	        if(debug)
	        	inst.info("Tribute 2");
	        phase = 2;
	        tribute();
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

/* Handle deaths */
function on_kill(cdefid, cid) {
	if(debug && phase != 0)
		inst.info("Death - " + cdefid + " / " + cid);
	if(cdefid == CDEF_VALKAL2) {
		/* Valkal 2 is dead! */
        finished = true;
		tod("Sunrise");	
		inst.broadcast("Valkal has been defeated! The victorious team consisted of ...");
		foreach(idx, cid in inst.all_players()) {
            local pn = inst.get_display_name(cid);
			inst.queue(function() {
				inst.broadcast(pn);
			}, ( idx + 1 ) * 6000);
		}
		inst.spawn(1154835, 0, 0);
	}
	else if(cdefid == CDEF_VAJ_1) {
		death_toll.append(cid);
   		inst.creature_chat(cid_valkal1, "s/", "No! This cannot be! My son ..");
	}
	else if(cdefid == CDEF_VAJ_2) {
		death_toll.append(cid);
   		inst.creature_chat(cid_valkal1, "s/", "You .. you will pay .. I swear ...");
	}
	
}

/* Debug function that can be run externally to set the phase */
function debug_setphase(p) {
	inst.info("Phase is now " + p);
	phase = p.tointeger();
}

/* Initialisation. Start scanning for Valkal1 to spawn */
tod("Day");
inst.exec(find_valkal1);
