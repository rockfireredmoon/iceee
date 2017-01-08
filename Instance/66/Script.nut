::info <- {
	name = "Valkals Bloodkeep",
	author = "Emerald Icemoon",
	description = "Handles the final fight!",
	queue_events = true
}

// Creature Definition IDs

const CDEF_VALKAL1 = 1385;

// The locations of the room
loc_throne_room <- Area(4160,2556, 4336, 2981);
loc_trap_door <- Point(4363, 2769);

// Prop IDS

const VALKAL2_SPAWN_PROP = 1150656;
const TRAP_DOOR_SPAWN_PROP = 1150652;

// State variables
cid_valkal1 <- 0;

function find_valkal1() {
	cid_valkal1 = inst.scan_npc(loc_throne_room, CDEF_VALKAL1);
    if(cid_valkal1 == 0)
        inst.queue(find_valkal1, 5000);
    else
        valkal1_health();
}

function valkal1_health() {
	if(cid_valkal1 == 0)
		return;

	local health = inst.get_health_pc(cid_valkal1);
    if(health < 5) {
        inst.set_flags(cid_valkal1, SF_NON_COMBATANT);
		inst.creature_chat(cid_valkal1, "s/", "This is not over...");
        foreach(a in inst.all_players())
            inst.untarget(a);
        inst.untarget(cid_valkal1);
        //inst.set_status_effect(cid_valkal1, "UNATTACKABLE", 99999);
        //inst.set_status_effect(cid_valkal1, "UNKILLABLE", 99999);
        inst.unhate(cid_valkal1);
        //inst.creature_use(cid_valkal1, _AB("Healing Potion: Level 50"));
        inst.queue(function() {
            inst.walk_then(cid_valkal1, loc_trap_door, CREATURE_RUN_SPEED, 0, function() {
		        inst.creature_chat(cid_valkal1, "s/", "... Follow if you dare");
                inst.despawn(cid_valkal1);

	            // Portal
	            inst.spawn(TRAP_DOOR_SPAWN_PROP, 0, 0);
              
	            // Valkal 2
	            inst.spawn(VALKAL2_SPAWN_PROP, 0, 0);
			});
        }, 1000);    
        return;    
    }

    if(health < 100)
        inst.queue(valkal1_health, 1000);
    else
        inst.queue(valkal1_health, 5000);
    
}

inst.queue(find_valkal1, 1000);

