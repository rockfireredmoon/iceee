/*
 * A squirrel script version of Bastion_NPC_CatapultFiringAnimation. Script
 * randomly picks a target of a particular CDEFID, and fires a missile at
 * it. The target must be of the same type hostility as the missile (e.g. E+E or F+F)
 *
 * Note the old script relied on cooldown and just constantly tried to attack (something).
 * I have no idea how targetting was supposed to happen, so instead this script uses
 * a single ability targetted at found creatures and it's own timers
 */
 
const AB_CATAPULT = 5421;
const CDEFID_TARGET = 3397;

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = ""
}

function attack_target() {
	local target_cids = ai.cids(CDEFID_TARGET);
	local cid = target_cids.len() == 0 ? 0 : target_cids[randmod(target_cids.len())];
	if(cid > 0) {
		ai.set_other_target(ai.get_self(), cid);
		ai.use(AB_CATAPULT);
	}
	ai.queue(attack_target, 7500 + randmod(5000));
}


ai.queue(attack_target, randmod(5000));