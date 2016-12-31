/*
 * Quick warp coords for boss room: /warp 3034 2609
 *
 * The Skrill Hive instance script. This script is primarily for the
 * final fight with the Skrill Queen.
 *
 */

// Creature Definition IDs

const CDEF_SKRILL_QUEEN = 7760;
const CDEF_HATCHLING = 7862;

// Prop IDs

// TODO actual prop
const PROP_ESSENCE_CHEST = 1127658;

// The locations of the room
loc_queen_room <- Area(2815,2106, 3278, 2712);

// The current stage the fight is at
stage <- 99;

// The CIDs of various creatures
cid_skrill_queen <- 0;

/*
 * When the skrill queen dies, remove all of her minions and stop checking her
 * health
 */
function on_kill_7760() {
	kill_minions();
	/* inst.spawn(PROP_ESSENCE_CHEST, 0, 0); */
	cid_skrill_queen = 0;
}

function kill_minions() {
	inst.despawn_all(CDEF_HATCHLING);
}

function find_queen() {
	info("Looking for queen ");
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
		    inst.ai(cid_minion, "tryMelee");
        }
	}
}

function spawn_minions() {
	info("Spawning adds");
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

	print("Queen health: " + health + ", stage " + stage + "\n");

	if(health == 0) {
		info("Health zero, assuming mob died");
		stage = 0;
		inst.queue(find_queen, 5000);
		return;
	}

	if(stage != 0 && health == 100) {
		/* If we have past the first phase, and the wailer returns to full health
		 * dismiss the adds and return to the waiting for the first phase
		 */
		 kill_minions();
		 stage = 0;
		 inst.queue(queen_health, 3000);
		 return;
	}


	if(stage == 0) {
		if(health <= 90) {
			stage = 1;
			info("Queen health < 90, trying to cast swarm");
			inst.ai(cid_skrill_queen, "try_swarm");
		}
		inst.queue(queen_health, 3000);
		return;
	}

	if(stage == 1) {
		if(health <= 85) {
			// 85% The queen calls 4 hatchlings to fight the group.
			spawn_minions();
			stage = 2;
		}
		inst.queue(queen_health, 3000);
		return;
	}

	inst.queue(queen_health, 3000);
}

inst.queue(find_queen, 5000);
