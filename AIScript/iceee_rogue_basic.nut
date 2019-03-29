/*
 * Basic Rogue Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */
 
const DISEMBOWL = 292; 
const ASSAIL = 232; 
const PIERCE = 281;
const FELINES_GRACE = 93; 
const MELEE = 32766;	

info <- {
	name = "iceee_rogue_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Basic Rogue"
	speed = 2
}

function on_combat_ready()
	if(!ai.is_on_cooldown("Stat"))
		ai.use_highest_once(FELINES_GRACE);

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	ai.use(MELEE);
	ai.exec(main);
}

function main() {
	if(ai.get_might() >= 1) {
		if(ai.get_might_charge() >= 3)	
			ai.use_highest_once(ASSAIL);
		else if(ai.get_might() >= 3 && randmod(5) < 3) {	
			if(!ai.is_on_cooldown("Pierce"))
				ai.use_highest_once(PIERCE);
			else
				ai.use_highest_once(DISEMBOWL);
		}
		else if(randmod(5) < 3)
			ai.use_highest_once(DISEMBOWL);		
	}
	
	ai.exec(main);
}
