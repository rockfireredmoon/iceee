/*
 * Skrill Queen AI
 *
 * 229 - assualt (charge, requires 3 might)
 * 291 - shield bash (execute, requires 1 might)
 * 5146 - whiplash (debuf,charge, requires 4 might)
 * 5277 - thors mighty blow (execute requires 1 might)
 * 283 - pierce (charge, requires 3 might)
 * 32766 - melee
 * 5149 - swarm (5 will)
 */

info <- {
	name = "skrill_queen",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Skrill Queen AI"
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(32766);
	main();
}

function try_swarm() {
    ai.set_gtae();
    ai.use(5149);
}

function main() {
	ai.use(32766);

    /* When we have at least 4 might, cast the debuf */
	if(ai.get_might() >= 4 && !ai.is_on_cooldown("MeleeSpeedDebuff")) {
		ai.use(5146);
	}
	
    /* When we have at least 3 might, there is 50% chance we attack */
	if(ai.get_might() >= 3 && randmodrng(0, 2) == 0) {
        if(randmodrng(0, 2) > 0)
            ai.use(229);
        else
    		ai.use(283);
	}
	
    /* When we have at least 3 charges, there is 1 in 3 chance we use it */
	if(ai.get_might() > 1 && ai.get_might_charge() >= 3 && randmodrng(0, 3) == 2) {
        if(randmodrng(0, 2) > 0)
            ai.use(291);
        else
    		ai.use(5277);
	}
	ai.queue(main, 1000);
}

