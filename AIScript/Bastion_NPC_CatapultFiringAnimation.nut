/*
 * A squirrel script version of Bastion_NPC_CatapultFiringAnimation. Script
 * randomly picks a target of a particular CDEFID, and fires a missile at
 * it. The script will pick the appropriate ability depending on whether this
 * is friendly or enemy fire.
 *
 * Note the old script relied on cooldown and just constantly tried to attack (something).
 * I have no idea how targetting was supposed to happen, so instead this script uses
 * a single ability targetted at found creatures and it's own timers
 */
 
const FRIEND_DEFAULT_ABILITY = 5421;
const ENEMY_DEFAULT_ABILITY = 5429;

/* This script takes at least one argument, the first being CreatureDefID of the target.
   A second optional argument may be supplied which is an ability ID to use (if this is
   not provided the ability will be automatically chosen based on hostility between attacker
   and target. */

target_cdefid <- __argc > 0 ? __argv[0].tointeger() : 0;
ability <- __argc > 1 ? __argv[1].tointeger() : null;

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "",
	idle_speed = 1
}

function attack_target() {
	local target_cids = ai.cids(target_cdefid);
	local cid = target_cids.len() == 0 ? 0 : target_cids[randmod(target_cids.len())];
	if(cid > 0) {
		ai.set_other_target(ai.get_self(), cid);
		if(ability == null)
			if(ai.is_target_enemy())
				ai.use(ENEMY_DEFAULT_ABILITY);
			else 
				ai.use(FRIEND_DEFAULT_ABILITY);
	}
	ai.queue(attack_target, 7500 + randmod(5000));
}





if(target_cdefid == 0)  
	ai.error("Error! AI script Bastion_NPC_CatapultFiringAnimation script called without any parameter. Requires propID of the target's spawner");
else  
	ai.queue(attack_target, randmod(5000));
