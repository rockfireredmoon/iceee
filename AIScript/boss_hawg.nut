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

const SUCCESS_INTERVAL = 2000;
const RETRY_INTERVAL = 1000;

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

    /* Always try Spellbreaker when possible */
    if(ai.get_might() > 3 && !ai.is_on_cooldown("Spellbreaker") && randmodrng(0, 3) == 2) {
        ai.use_once(347);
    	ai.queue(main, SUCCESS_INTERVAL);
    	return;
    }
    
    /* Maybe Overstrike when at least 3 charges and 1 might */
	if(ai.get_might() > 1 && ai.get_might_charge() >= 3 && randmodrng(0, 3) == 2 && !ai.has_buff(6, 72000)) {
        ai.use_once(477);
    	ai.queue(main, SUCCESS_INTERVAL);
    	return;
    }
    
    /* Cycle 0 - Assault */
    if(cycle == 0) {
       if(ai.get_might() >= 3) {
    		ai.use_once(228);
       		cycle = 1;
	    	ai.queue(main, SUCCESS_INTERVAL);
	    	return;
       }
    }
    
    /* Cycle 1 - Concussion */
    if(cycle == 1) {
        if(ai.get_might() >= 3) {
        	cycle = 2;
        	if(ai.use_once(539)) {
		    	ai.queue(main, SUCCESS_INTERVAL);
		    	return;
		    }
		}
    }
    
    /* Cycle 2 - Can Opener */
    if(cycle == 2) {
        if(ai.get_might() >= 4) {
    		ai.use_once(5254);
        	cycle = 3;
	    	ai.queue(main, SUCCESS_INTERVAL);
	    	return;
    	}
    }
    
    /* Cycle 3 - Disarm (if not cooling down and have might) */
	if(cycle == 3) {
        if(ai.get_might() >= 4) {
        	cycle = 0;
        	if(ai.use_once(5278)) {
	    		ai.queue(main, SUCCESS_INTERVAL);
	    		return
	    	}
	    }
	}
	
    ai.queue(main, RETRY_INTERVAL);
}
