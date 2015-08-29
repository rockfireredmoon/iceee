/*
 * Quick warp coords for boss room: /warp 2100 2900
 *
 *
 * The Wailing Crypt instance script. This script is primarily for the
 * final fight with the Wailer.
 *
 * 1. Players must first kill Kyshf. This will activate the 1st brazier in the Wailers room
 * 2. Players must first kill Alabaster. This will activate a 2nd brazier in the Wailers room
 * 3. Wailer spawns adds on 80%, 60%, 40% and 20%. 
 * 4. Wailer trys to cast wailing shield at 70%, 50%, 30% and 10%.
 * 5. With both braziers activated, they will purge the shield when the wailer is within 10m
 *
 * The brazier to use will alternate as the fight progress. The correct brazier to use will 
 * be lit GREEN, the incorrect one will be lit RED.
 */
 
// Creature Definition IDs
 
const CDEF_WAILER = 1259;
const CDEF_BRAZIER = 7777;
const CDEF_GUARDIAN = 7778;

// Prop IDs

const PROP_BRAZIER_1 = 1126558;
const PROP_BRAZIER_2 = 1126556;
const PROP_LIGHT_1 = 1500051;
const PROP_LIGHT_2 = 1500050;
const PROP_ESSENCE_CHEST = 1127658;

// The locations of the room and braziers
loc_brazier_1 <- Area(1948, 2766, 35);
loc_brazier_2 <- Area(2306, 2766, 35);
loc_wailer_room <- Area(1859, 2702, 2393, 3234);

// The current stage the fight is at
stage <- 99;

// The CIDs of various creatures
cid_brazier_1 <-0;
cid_brazier_2 <-0;
cid_wailer <- 0;

// Which brazier to use
use_brazier <- 0;

// Tags used for asset changing effect (GREED/RED brazier fire)
brazier_1_tag <- 0;
brazier_2_tag <- 0;
light_1_tag <- 0;
light_2_tag <- 0;

wailer_message_shown <- false;
brazier_message_shown <- false;

/*
 * When the wailer dies, remove all of his minions and stop checking his
 * health
 */
function on_kill_1259() {
	set_brazier(0);
	inst.broadcast("The Wailer wails no more!");
	kill_minions();
	inst.despawn(cid_brazier_1);
	inst.despawn(cid_brazier_2);
	inst.spawn(PROP_ESSENCE_CHEST, 0, 0);
	cid_wailer = 0;
}

/* Alabaster killed, spawn the flame on one of the braziers.
 * The flame is the creature that has the purge ability. The
 * ability will only be activated if both braziers are lit.
 */
function on_kill_3649() {
	cid_brazier_1 = inst.spawn(PROP_BRAZIER_1, 0, 0);
	if(cid_brazier_1 < 0) 
		/* Keep trying until we can spawn this (probably because no player
		 * is yet in the spawn tile
		 */
		inst.queue(on_kill_3649, 5000);
}

/*
 * Kyshf killed, spawn the flame on one of the braziers.
 * The flame is the creature that has the purge ability. The
 * ability will only be activated if both braziers are lit.
 */  
function on_kill_3648() {
	cid_brazier_2 = inst.spawn(PROP_BRAZIER_2, 0, 0);
	if(cid_brazier_2 < 0)
		/* Keep trying until we can spawn this (probably because no player
		 * is yet in the spawn tile
		 */
		inst.queue(on_kill_3648, 5000);
}

function kill_minions() {
	inst.despawn_all(CDEF_GUARDIAN);
}

function find_wailer() {
	cid_wailer = inst.scan_npc(loc_wailer_room, CDEF_WAILER);
	inst.queue(cid_wailer == 0 ? find_wailer : wailer_health, 5000);
}

/*
 * The adds should now be spawned, but we need them
 * to target whoever is attacking The Wailer and start
 * attacking them.
 */
function activate_minions(cid_minions) {
	local wailer_target = inst.get_target(cid_wailer);
	if(wailer_target > 0) {
		foreach(cid_minion in cid_minions) {
			inst.set_target(cid_minion, wailer_target);
			inst.ai(cid_minion, "tryMelee");
		}
	}
}

function wailer_shield() {
	if(cid_wailer == 0) 
		return;
	
	local in_range_1 = inst.scan_npc(loc_brazier_1, CDEF_WAILER) > 0;
	local in_range_2 = inst.scan_npc(loc_brazier_2, CDEF_WAILER) > 0;
	local in_range = in_range_1 || in_range_2; 
		
		
	if( ( use_brazier == 1 && !in_range_1 ) || ( use_brazier == 2 && !in_range_2 )) {
		if(!wailer_message_shown) {
			wailer_message_shown = true;
			inst.info("The Wailer shimmers as he hides in his chilly realm");
		}	
		inst.ai(cid_wailer, "extTryCastShield");
	}
	
	if(in_range_1 && cid_brazier_1 > 0 && use_brazier == 1) {	
		inst.set_target(cid_brazier_1, cid_wailer);
		inst.ai(cid_brazier_1, "tryWarm");
	}
	
	if(in_range_2 && cid_brazier_2 > 0 && use_brazier == 2) {
		inst.set_target(cid_brazier_2, cid_wailer);
		inst.ai(cid_brazier_2, "tryWarm");
	}
	
	inst.queue(wailer_health, 3000);
}

function spawn_minions() {
	activate_minions([
		inst.spawn(1127142, CDEF_GUARDIAN, 32), 
		inst.spawn(1127141, CDEF_GUARDIAN, 32), 
		inst.spawn(1127140, CDEF_GUARDIAN, 32)
	]);
}

function set_brazier(to_use) {
	use_brazier = to_use;
	if(brazier_1_tag > 0) {
		inst.restore(PROP_BRAZIER_1, brazier_1_tag);
		brazier_1_tag = 0; 
		inst.restore(PROP_LIGHT_1, light_1_tag);
		light_1_tag = 0; 
	}	
	if(brazier_2_tag > 0) {
		inst.restore(PROP_BRAZIER_2, brazier_2_tag);
		brazier_2_tag = 0; 
		inst.restore(PROP_LIGHT_2, light_2_tag);
		light_2_tag = 0;   
	}	
	if(use_brazier > 0) {	
		light_1_tag = inst.asset(PROP_LIGHT_1, use_brazier == 1 ? "Light-Intense?Group=1&COLOR=00FF00" : "Light-Intense?Group=1&COLOR=FF0000", 1);	
		light_2_tag = inst.asset(PROP_LIGHT_2, use_brazier == 2 ? "Light-Intense?Group=1&COLOR=00FF00" : "Light-Intense?Group=1&COLOR=FF0000", 1);
		brazier_1_tag = inst.asset(PROP_BRAZIER_1, use_brazier == 1 ? "Par-Flame_Green-Emitter" : "Par-Flame_Red-Emitter", 1);			
		brazier_2_tag = inst.asset(PROP_BRAZIER_2, use_brazier == 2 ? "Par-Flame_Green-Emitter" : "Par-Flame_Red-Emitter", 1);
	}
}

function clear_brazier_targets() {
	inst.unhate(cid_brazier_1);
	inst.clear_target(cid_brazier_1);
	inst.unhate(cid_brazier_2);
	inst.clear_target(cid_brazier_2);
}

function wailer_health() {

	if(cid_wailer == 0)
		return;
		
	if(stage == 99) {
		stage = 0;
		inst.queue(find_wailer, 5000);
		return;
	}
	
	local health = inst.get_health_pc(cid_wailer);
	
	print("Wailer health: " + health + ", stage " + stage + "\n");
	
	if(stage != 0 && health == 100) {
		/* If we have past the first phase, and the wailer returns to full health
		 * dismiss the adds and return to the waiting for the first phase
		 */
		 kill_minions();
		 set_brazier(0);
		 stage = 0;
		 inst.queue(wailer_health, 3000);
		 return;
	}
	
	if(stage == 0) {
		if(health <= 80) {
			// 80% The Wailer calls 3 forgotten souls to fight the group.
			spawn_minions();
			stage = 1;
		}
		inst.queue(wailer_health, 3000);
		return; 
	}
	
	if(stage == 1) {
		if(health <= 70) {
			/* Between 60 and 70 % The Wailer casts his shield, 10k damage absorption, the group has to draw 
			 * The Wailer to BRAZIER 1 which will become green to purge the shield 
			 */
			
			stage = 2;
			set_brazier(1);
			inst.queue(wailer_shield, 2000);
			return;
		}
	}
	
	if(stage == 2) {
		if(health <= 60) {
			// 60% The Wailer calls 3 more forgotten souls to fight the group.
			clear_brazier_targets();
			set_brazier(0);
			spawn_minions();
			stage = 3;
			wailer_message_shown = false;	
			inst.queue(wailer_health, 3000);
			return;
		}
		inst.queue(wailer_shield, 3000);
		return; 
	}
	
	if(stage == 3) {
		if(health <= 50) {
			/* Between 40 and 50 % The Wailer casts his shield, 10k damage absorption, the group has to draw 
			 * The Wailer to one of the two braziers to purge the shield 
			 */
			stage = 4;
			set_brazier(2);
			inst.queue(wailer_shield, 2000);
			return;
		}
	}
	
	if(stage == 4) {
		if(health <= 40) {
			// 40% The Wailer calls 3 more forgotten souls to fight the group.
			clear_brazier_targets();
			set_brazier(0);
			spawn_minions();
			stage = 5;
			wailer_message_shown = false;
			inst.queue(wailer_health, 3000);
			return;
		}
		inst.queue(wailer_shield, 3000);
		return; 
	}
	
	if(stage == 5) {
		if(health <= 30) {
			/* Between 20 and 30 % The Wailer casts his shield, 10k damage absorption, the group has to draw 
			 * The Wailer to one of the two braziers to purge the shield
			 */
			stage = 6;
			set_brazier(1);
			inst.queue(wailer_shield, 2000);
			return;
		}
	}
	
	if(stage == 6) {
		if(health <= 20) {
			// 20% The Wailer calls 3 more forgotten souls to fight the group.
			spawn_minions();
			clear_brazier_targets();
			set_brazier(0);
			wailer_message_shown = false;
			stage = 7;
			inst.queue(wailer_health, 3000);
			return;
		}
		inst.queue(wailer_shield, 3000);
		return; 
	}
	
	if(stage == 7) {
		if(health <= 10) {
			/* Between 0 and 10 % The Wailer casts his shield, 10k damage absorption, the group has to draw 
			 * The Wailer to one of the two braziers to purge the shield
			 */
			stage = 8;
			set_brazier(2);
			inst.queue(wailer_shield, 2000);
			return;
		}
	}
	
	if(stage == 8) {
		inst.queue(wailer_shield, 2000);
		return;
	}
	
	inst.queue(wailer_health, 3000);
}

inst.queue(find_wailer, 5000);
