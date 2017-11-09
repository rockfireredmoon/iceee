/*
 * Quick warp coords for boss room: /warp 3034 2609
 *
 * The Skrill Hive instance script. This script is primarily for the
 * final fight with the Skrill Queen.
 *
 */

// Creature Definition IDs

const CDEF_SKRILL_QUEEN = 1412;
const CDEF_HATCHLING = 7862;

// The locations of the room
loc_queen_room <- Area(2815,2106, 3278, 2712);

// State
stage <- 99;
cid_skrill_queen <- 0;
env <- "";
fhc <- 0;

function tod(e) {
    if(e != env) {
    	env = e;
    	inst.set_timeofday(e);
    }
}

/*
 * When the skrill queen dies, remove all of her minions and stop checking her
 * health
 */
function on_kill_1412() {
	kill_minions();
	cid_skrill_queen = 0;
	stage = 0;
	tod("Sunrise");	
	inst.clear_queue();
}

function kill_minions() {
	inst.despawn_all(CDEF_HATCHLING);
}

function find_queen() {
	cid_skrill_queen = inst.scan_npc(loc_queen_room, CDEF_SKRILL_QUEEN);
	inst.queue(cid_skrill_queen == 0 ? find_queen : queen_health, 5000);
}

/*
 * The adds should then locate, target and attack the nearest
 * player
 */
function activate_minions(cid_minions) {
	foreach(cid_minion in cid_minions) {
		/* Find all players, sidekicks and enemies of the minion within the range */
		local targets = inst.get_nearby_creature(10000, cid_minion, TS_NONE, TS_ENEMY, TS_NONE);
		local closestCID = -1;
		local closestDistance = 9999999;

		/* Find the closest enemy */
		foreach(t in targets) {
			local dist = inst.get_creature_distance(t, cid_minion);
			if(dist > -1 && dist < closestDistance) {
				closestDistance = dist;
				closestCID = t;
			}
		}

		/* Target and attack the found enemy */
		if(closestCID != -1) {
			inst.set_target(cid_minion, closestCID);
			inst.creature_use(cid_minion, _AB("melee"));
		}
	}
}

function spawn_minions() {
	activate_minions([
		inst.spawn(1127143, CDEF_HATCHLING, 32),
		inst.spawn(1127144, CDEF_HATCHLING, 32),
		inst.spawn(1127141, CDEF_HATCHLING, 32),
		inst.spawn(1127142, CDEF_HATCHLING, 32)
	]);
}

function queen_health() {

	if(cid_skrill_queen == 0)
		return;

	local health = inst.get_health_pc(cid_skrill_queen);

	if(stage == 99 || health == 0) {
		stage = 0;
		inst.queue(find_queen, 5000);
		return;
	}
	
	if(health == -1) {
		stage = 0;
		inst.exec(find_queen);
	}
	else if(health == 100) {
		if(fhc == 4 && inst.is_at_tether(cid_skrill_queen)) {
			stage = 0;
		 	kill_minions();
		}
		else
			fhc++;
			
		inst.queue(queen_health, 5000);
	}
    else {
		tod("Sunset");
		
		if(health <= 10 && stage < 12) {
			stage = 12;
			spawn_minions();
			inst.queue(spawn_minions, 3000);
		}
		else if(health <= 15 && stage < 11) {
			stage = 11;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		else if(health <= 25 && stage < 10) {
			stage = 10;
			spawn_minions();
			inst.queue(spawn_minions, 3000);
		}
		else if(health <= 30 && stage < 9) {
			stage = 9;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		else if(health <= 40 && stage < 8) {
			stage = 8;
			spawn_minions();
			inst.queue(spawn_minions, 3000);
		}
		else if(health <= 45 && stage < 7) {
			stage = 7;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		else if(health <= 55 && stage < 6) {
			stage = 6;
			spawn_minions();
		}
		else if(health <= 60 && stage < 5) {
			stage = 5;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		else if(health <= 70 && stage < 4) {
			stage = 4;
			spawn_minions();
		}
		else if(health <= 75 && stage < 3) {
			stage = 3;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		else if(health <= 85 && stage < 2) {
			stage = 2;
			spawn_minions();
		}
		else if(health <= 90 && stage < 1) {
			stage = 1;
			inst.ai(cid_skrill_queen, "try_swarm");
		}
	}

	inst.queue(queen_health, 1000);
}

tod("Day");
inst.queue(find_queen, 5000);
