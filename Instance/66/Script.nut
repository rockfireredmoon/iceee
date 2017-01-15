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
const HEALING_BREEZE = 5091;

// Creature Definition IDs

const CDEF_VALKAL1 = 1385;

// The locations of the room
loc_throne_room <- Area(4160,2556, 4336, 2981);
loc_trap_door <- Point(4363, 2769);
loc_platform_centre <- Point(4300, 2788);
loc_platform_front <- Point(4300, 2788);
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

/* Helper function to find closest creature to another */
function find_closest(cid) {

    /* Find all players, sidekicks and enemies of the minion within the range */
    local targets = inst.get_nearby_creature(10000, cid, TS_NONE, TS_ENEMY, TS_NONE);
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

    local closest_cid = find_closest(cid);

    /* Target and attack the found enemy */
    if(closest_cid != -1) {
        inst.set_target(cid, closest_cid);
        inst.creature_use(cid, _AB("melee"));
    }
}

/* Spawn Valkals adds. Each one will be instructed to run to a defined place,
   then locate the nearest player and attack them. */
function spawn_adds() {
    foreach(v in vampire_adds) {
		local cid = inst.spawn(v.prop_id, 0, 0);
        inst.walk_then(cid, v.pos, CREATURE_JOG_SPEED, 0, function() {
            attack_closest(cid);
        });
    }
}

/* Totally disengage Valkal from the fight and make him invincible */
function disengage_valkal() {
	inst.unhate(cid_valkal1);
	inst.clear_target(cid_valkal1);
	inst.interrupt(cid_valkal1);
	inst.set_flag(cid_valkal1, SF_NON_COMBATANT, true);
}

/* Starts the sequence of Valkal running from the fight his platform to
   heal */
function valkal_heal() {
    inst.clear_target(cid_valkal1);
	inst.walk_then(cid_valkal1, loc_platform_centre, CREATURE_RUN_SPEED, 0, function() {
        inst.creature_use(cid_valkal1, HEALING_BREEZE);
        inst.queue(function() {
            inst.creature_use(cid_valkal1, HEALING_BREEZE);
            inst.queue(function() {
                inst.creature_use(cid_valkal1, HEALING_BREEZE);
                inst.queue(function() {
                    valkal_engage();
                }, 5000); 
            }, 5000); 
        }, 5000); 
	});
}

/* Returns Valkal to the center of the room, and picks the closest player
   to attack */
function valkal_engage() {
	inst.set_flag(cid_valkal1, SF_NON_COMBATANT, false);
	inst.walk_then(cid_valkal1, loc_room_centre, CREATURE_RUN_SPEED, 0, function() {
        attack_closest(cid);
	});
}

/* Starts the sequence of Valkal running from the throne room to the
   Sanctum */
function valkal_flee() {

   	inst.creature_chat(cid_valkal1, "s/", "This is not over...");
    disengage_valkal();
	
	foreach(a in inst.all_players()) {
		inst.clear_target(a);
    }
	inst.queue(function() {
		inst.walk_then(cid_valkal1, loc_trap_door, CREATURE_RUN_SPEED, 0, function() {
			inst.creature_chat(cid_valkal1, "s/", "... Follow if you dare");
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
			}, 1000);
		});
	}, 1000);
}

/* Runs Valkal to just in front of the platform, pick a player to Tribute, and 
   do so, before running back to centre and picking another player to fight */
function tribute() {
    disengage_valkal();
	inst.walk_then(cid_valkal1, loc_platform_front, CREATURE_RUN_SPEED, 0, function() {
        local targets = inst.get_nearby_creature(10000, cid_valkal1, TS_NONE, TS_ENEMY, TS_NONE);
        if(targets.len() > 0) {
            inst.info("ERR: Found " + targets.len() + " players to pick from");
            local cid = targets[randmodrng(0, targets.len())];
            inst.info("Picked " + cid);
            inst.set_target(cid_valkal1, cid);
            if(!inst.creature_use(cid_valkal1, _AB("Tribute"))) {
                inst.info("Failed to tribute " + cid);
            }
            else
                inst.info("Tribute OK " + cid);
        }
        else
            inst.info("ERR: Found no players");
        inst.queue(function() {
            valkal_engage();
        }, 1500);
    });
    
}

/* Monitor Valkal1's health and trigger the various fight stages. */
function valkal1_health() {
	if(cid_valkal1 == 0)
		return;

	local health = inst.get_health_pc(cid_valkal1);

    if(health <= 5 && phase < 10) {
        // Flee and leave this loop
        inst.info("Flee!");
        valkal_flee();
        return;
    }
	if(health <= 10 && phase < 9) {
        // Bringing Down the House 3
        inst.info("Bringing Down the House 3");
        phase = 9;        
    }
    else if(health <= 20 && phase < 8) {
        // Bringing Down the House 2
        inst.info("Bringing Down the House 2");
        phase = 8;
    }
    else if(health <= 25 && phase < 7) {
        // Heal 3
        inst.info("Heal 3");
        phase = 7;
    }
    else if(health <= 30 && phase < 6) {
        // Bringing Down the House 1
        inst.info("Bringing Down the House 1");
        phase = 6;
    }
    else if(health <= 40 && phase < 5) {
        // Tribute 3
        inst.info("Tribute 3");
        phase = 5;
    }
    else if(health <= 50 && phase < 4) {
        // Heal 2
        inst.info("Heal 2");
        phase = 4;
    }
    else if(health <= 60 && phase < 3) {
        // Tribute 2
        inst.info("Tribute 2");
        phase = 3;
    }
    else if(health <= 75 && phase < 2) {
        // Heal 1
        inst.info("Heal 1");
        phase = 2;
    }
    else if(health <= 80 && phase < 1) {
        // Tribute 1
        inst.info("Tribute 1");
        tribute();
        phase = 1;
    }

	if(health < 100)
		inst.queue(valkal1_health, 1000);
	else
		inst.queue(valkal1_health, 5000);

}

/* Initialisation. Start scanning for Valkal1 to spawn */
inst.queue(find_valkal1, 1000);
