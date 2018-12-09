/*
 * Basic Icemage Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */

const FORCEBOLT = 256; 
const FORCE_BLAST = 374;
const ATHENAS_GIFT = 86; 
const FROSTBOLT = 513;
const CRYO_BLAST = 519;
const MELEE = 32760;	

info <- {
	name = "iceee_icemage_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Basic Icemage"
	speed = 2
}

function on_combat_ready()
	if(!ai.is_on_cooldown("Stat"))
		ai.use_highest_once(ATHENAS_GIFT);

function on_target_lost(target_cid)
	ai.clear_queue();
	
function on_target_acquired(target_cid) {
	ai.use(MELEE);
	ai.exec(main);
}

function main() {
	if( ai.get_will() > 0 && ( randmod(5) < 4 || ai.get_might() == 0 ) ) {
		if(ai.get_will_charge() > 4 || ( ai.get_will_charge() >= 3 && randmod(5) < 4) ) 	
			ai.use_highest_once(CRYO_BLAST);
		else 	
			ai.use_highest_once(FROSTBOLT);
	}
	else if(ai.get_might() > 0 && ( randmod(5) < 4 || ai.get_will() == 0 )) {
		if(ai.get_might_charge() > 4 || ( ai.get_might_charge() >= 3 && randmod(5) < 4)) 	
			ai.use_highest_once(FORCE_BLAST);
		else 	
			ai.use_highest_once(FORCEBOLT);
	}
	
	ai.exec(main);
}
