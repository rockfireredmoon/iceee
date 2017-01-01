/*
 * Boss Hawg AI
 *
 * 228 - Assault (lvl 40, req. 3 might, gen. 1)
 * 539 - Concussion (lvl 40, req. 3 might, gen. 1)
 * 5254 - Can Opener (lvl 40, reg. 4 might, gen. 1)
 * 5278 - Disarm (debuf, req. 5 might, Special cooldown cat)
 * 347 - Spellbreaker (debuf, lvl 50, req. 3 might, Spellbreaker cooldown cat)
 * 477 - Overstrike (exec lvl 50, req. 1 might, req 1-5 charges)
 * 32766 - melee
 */

info <- {
	name = "boss_hawg",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Boss Hawg AI"
}

cycle <- 0;

function on_target_lost(targetCID) {
	ai.clear_queue();
    cycle = 0;
}

function on_target_acquired(targetCID) {
	ai.use(32766);
	main();
}

function main() {
	ai.use(32766);

    /* Always try Spellbreaker when possible */
    if(ai.get_might() > 3 && !ai.is_on_cooldown("Special")) {
        ai.use(347);
    }
    /* Maybe Overstrike when at least 3 charges and 1 might */
	else if(ai.get_might() > 1 && ai.get_might_charge() >= 3 && randmodrng(0, 3) == 2) {
        ai.use(477);
    }
    /* Cycle 0 - Assault */
    else if(cycle == 0) {
        if(ai.get_might() >= 3)
    		ai.use(347);
        cycle = 1;
    }
    /* Cycle 1 - Concussion */
    else if(cycle == 1) {
        if(ai.get_might() >= 3)
    		ai.use(539);
        cycle = 2;
    }
    /* Cycle 2 - Can Opener */
    else if(cycle == 2) {
        if(ai.get_might() >= 4)
    		ai.use(539);
        cycle = 3;
    }
    /* Cycle 3 - Disarm (if not cooling down and have might) */
	else if(cycle == 3) {
        if(ai.get_might() >= 4 && !ai.is_on_cooldown("Special"))
    		ai.use(5146);
        cycle = 0;
	}

    ai.queue(main, 2000);
}
