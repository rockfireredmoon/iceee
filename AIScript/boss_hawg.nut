/*
 * Boss Hawg AI
 *
 * 228 - Assault (lvl 40, req. 3 might, gen. 1)
 * 539 - Concussion (lvl 40, req. 3 might, gen. 1)
 * 5254 - Can Opener (lvl 40, reg. 4 might, gen. 1)
 * 5278 - Disarm (debuf, req. 5 might, Special cooldown cat)
 * 347 - Spellbreaker (debuf, lvl 50, req. 3 might, Spellbreaker cooldown cat)
 * 477 - Overstrike (lvl 50, req. 1 might, req 1-5 charges)
 * 32766 - melee
 */

info <- {
	name = "boss_hawg",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Boss Hawg AI"
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(32766);
	main();
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
