::info <- {
	name = "Valkals Bloodkeep",
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

    rot = 0;
    pos = null;
    prop_id = null;
}

// Abilities
const HEALING_HAND = 5118;
const SELF_STUN = 24008;
const TRIBUTE = 24005;

// Creature Definition IDs

const CDEF_VALKAL1 = 1385;

// The locations of the room
loc_throne_room <- Area(4160,2556, 4336, 2981);
loc_trap_door <- Point(4363, 2769);
loc_platform_centre <- Point(4300, 2788);
loc_platform_front <- Point(4141, 2790);
loc_room_centre <- Point(3971,2790);

// Adds
vampire_adds <- [
    VampireAdd(1150679, Point(4011,2931), 128),
    VampireAdd(1150680, Point(4011,2631), 0),
    VampireAdd(1150681, Point(4155,2697), 214),
    VampireAdd(1150687, Point(4159,2874), 168),
    VampireAdd(1150683, Point(3861,2901), 92),
    VampireAdd(1150684, Point(3859,2683), 40),
    VampireAdd(1150685, Point(3803,2820), 66),
    VampireAdd(1150686, Point(3805,2762), 63)
];


// Prop IDS

const VALKAL2_SPAWN_PROP = 1150656;
const TRAP_DOOR_SPAWN_PROP = 1150652;

// Item IDS
const BOOK_ITEMID = 8707;
const BOOK_NO = 3;

// Tile locations
valkal2_spawn_tile <- Point(10,3);

// State variables
cid_valkal1 <- 0;
phase <- 0;
valkal1_full_health_count <- 0;
debug <- true;

/* Handles the bookcase at the start of the dungeon, giving the players fair
   warning as to what is to come! */
function on_use_7872(cid, used_cid) {
	if(inst.has_item(cid, BOOK_ITEMID))
		inst.message_to(cid, "You already have the Book of Blood Keep", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the book", 3000, false, function() {
			inst.message_to(cid, "The Book of Blood Keep is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, BOOK_ITEMID);
			inst.open_book(cid, BOOK_NO, 1);
		});
		return true;
	
	}
}

/* Keep scanning for Valkal1 until he spawns. When he does, we start to monitor his health  */ 
function find_valkal1() {
	cid_valkal1 = inst.scan_npc(loc_throne_room, CDEF_VALKAL1);
	if(cid_valkal1 == 0)
		inst.queue(find_valkal1, 5000);
	else
		valkal1_health();
}

/* Helper function to find closest creature to another (player or NPC) */
function find_closest(cid, player) {

    /* Find all players, sidekicks and enemies of the minion within the range */
    local targets = inst.get_nearby_creature(10000, cid, 
    		player ? TS_ENEMY_ALIVE : TS_NONE, player ? TS_ENEMY_ALIVE : TS_NONE, TS_NONE);
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
   then locate the nearest player and attack them. */
function spawn_adds() {
	if(debug)
		inst.info("Spawing adds");
    foreach(v in vampire_adds) {
		local cid = inst.spawn(v.prop_id, 0, 0);
        inst.walk_then(cid, v.pos, CREATURE_JOG_SPEED, 0, function() {
        	inst.rotate_creature(cid, v.rot);
            attack_closest(cid);
        });
    }
}

/*  Helper function to totally disengage Valkal from the fight and make him invincible */
function disengage_valkal() {
	if(debug)
		inst.info("Disengaging Valkal");
		
	inst.interrupt(cid_valkal1);
	inst.leave_combat(cid_valkal1);
	inst.set_flag(cid_valkal1, SF_NON_COMBATANT, true);
	inst.pause_ai(cid_valkal1);
}

/*  Helper function to return Valkal to the center of the room, and picks the closest player
   to attack */
function valkal_engage() {
	disengage_valkal();
	if(debug)
		inst.info("Engaging");
	inst.walk_then(cid_valkal1, loc_room_centre, CREATURE_JOG_SPEED, 0, function() {
		inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
		inst.resume_ai(cid_valkal1);
        attack_closest(cid_valkal1);
        
        /* Start health check loop again */
		inst.exec(valkal1_health);
	});
}

/* Keeping a creature self healing until he gets to a limit, then queue
 * another function */
function heal(cid, limit, on_healed) {
	if(inst.get_health_pc(cid) < limit) {
    	inst.creature_use(cid, HEALING_HAND);
    	inst.exec(function() {
    		heal(cid, limit, on_healed);
    	});
    }
    else
    	inst.exec(on_healed);
}

/* Starts the sequence of Valkal running from the fight his platform to
   heal */
function heal_sequence() {
	spawn_adds();
    disengage_valkal();
	inst.walk_then(cid_valkal1, loc_platform_centre, CREATURE_JOG_SPEED, 0, function() {
        inst.rotate_creature(cid_valkal1, 192);
        heal(cid_valkal1, 95, valkal_engage); 
	});
}

/* Starts the sequence of Valkal running from the throne room to the
   Sanctum */
function valkal_flee() {

   	inst.creature_chat(cid_valkal1, "s/", "This is not over...");
    disengage_valkal();
	inst.queue(function() {
		inst.walk_then(cid_valkal1, loc_trap_door, CREATURE_RUN_SPEED, 0, function() {
			inst.creature_chat(cid_valkal1, "s/", "... Follow if you dare");
        	inst.rotate_creature(cid_valkal1, 192);
			inst.queue(function() {
				inst.despawn(cid_valkal1)
			}, 1000);

			// Portal
			inst.spawn(TRAP_DOOR_SPAWN_PROP, 0, 0);

			/* Valkal 2. He is in another tile that probably is not loaded, so make sure it is and
			   spawn after a short delay */
			inst.load_spawn_tile(valkal2_spawn_tile);
			inst.queue(function() {
				inst.spawn(VALKAL2_SPAWN_PROP, 0, 0);
			}, 5000);
		});
	}, 1000);
}

/* Starts the Tribute sequence. Runs Valkal to just in front of the platform, picks a player to 
   Tribute, and does so. Valkal is then stunned for 15 seconds before running back to centre
   and picking another player to fight */
function tribute() {
    disengage_valkal();
	inst.walk_then(cid_valkal1, loc_platform_front, CREATURE_JOG_SPEED, 0, function() {
        inst.rotate_creature(cid_valkal1, 192);
        inst.queue(function() {
	        local targets = inst.get_nearby_creature(300, cid_valkal1, TS_ENEMY_ALIVE, TS_NONE, TS_NONE);
	        if(targets.len() > 0) {
	            local cid = targets[randmodrng(0, targets.len())];
	            inst.set_target(cid_valkal1, cid);
	            if(debug)
	            	inst.info("Targetted " + inst.get_display_name(cid));
				inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
	            if(!inst.creature_use(cid_valkal1, TRIBUTE)) {
	            	if(debug)
	                	inst.info("Failed to tribute " + cid);
	            }
	            else {
	                inst.queue(function() {
	            		inst.target_self(cid_valkal1);
		            	if(inst.creature_use(cid_valkal1, SELF_STUN)) {
							inst.set_flag(cid_valkal1, SF_NON_COMBATANT, true);
					        inst.queue(function() {
								inst.leave_combat(cid_valkal1);  
					            valkal_engage();
					        }, 16000);
					    }
		            	else {	     
							inst.leave_combat(cid_valkal1);
	            			if(debug)       	
	                			inst.info("Stunned fail"); 
					        valkal_engage();
					    }
		            }, 6000);
	            }
	        }
	        else {
	            if(debug)       	
	            	inst.info("Found no players");            
				valkal_engage();
			}
	    }, 1000);
    });
    
}

/* Reset the Valkal1 fight if the party wipe out */
function reset_valkal1() {
	phase = 0;
    disengage_valkal();
   	inst.creature_chat(cid_valkal1, "s/", "Will no one face me? ... Cowards");
	valkal1_full_health_count = 0;
	inst.walk_then(cid_valkal1, loc_platform_centre, CREATURE_WALK_SPEED, 0, function() {
		inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
		inst.rotate_creature(cid_valkal1, 192);
	});
}

/* Monitor Valkal1's health and trigger the various fight stages. */
function valkal1_health() {
	if(cid_valkal1 == 0)
		return;

	local health = inst.get_health_pc(cid_valkal1);

	if(health == 100) {
		if(valkal1_full_health_count == 50 && inst.is_at_tether(cid_valkal1)) {
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
    	
	    if(health <= 5 && phase < 10) {
	        // Flee and leave this loop
	        if(debug)
	        	inst.info("Flee!");
	        valkal_flee();
	        return;
	    }
		if(health <= 10 && phase < 9) {
	        // Bringing Down the House 3
	        if(debug)
	        	inst.info("Bringing Down the House 3");
	        phase = 9;        
	    }
	    else if(health <= 20 && phase < 8) {
	        // Bringing Down the House 2
	        if(debug)
	        	inst.info("Bringing Down the House 2");
	        phase = 8;
	    }
	    else if(health <= 25 && phase < 7) {
	        // Heal 3
	        if(debug)
	        	inst.info("Heal 3");
	        phase = 7;
	    }
	    else if(health <= 30 && phase < 6) {
	        // Bringing Down the House 1
	        if(debug)
	        	inst.info("Bringing Down the House 1");
	        phase = 6;
	    }
	    else if(health <= 40 && phase < 5) {
	        // Tribute 3
	        if(debug)
	        	inst.info("Tribute 3");
	        tribute();
	        phase = 5;
	        return;
	    }
	    else if(health <= 50 && phase < 4) {
	        // Heal 2
	        if(debug)
	        	inst.info("Heal 2");
	        phase = 4;
	        heal_sequence();
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
	    else if(health <= 90 && phase < 2) {
	        // Heal 1
	        if(debug)
	        	inst.info("Heal 1");
	        phase = 2;
	        heal_sequence();
	        return;
	    }
	    else if(health <= 95 && phase < 1) {
	        // Tribute 1
	        if(debug)
	        	inst.info("Tribute 1");
	        phase = 1;
	        tribute();
	        return;
	    }
	    
		inst.exec(valkal1_health);
	}

}

/* Initialisation. Start scanning for Valkal1 to spawn */
inst.exec(find_valkal1);
