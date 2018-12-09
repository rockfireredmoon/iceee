/*
 * Basic Druid Death Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */

const SPIRIT_OF_SOLOMON = 102; 
const MELEE = 32766;
const DEATHLY_DART = 529;
const WITHER = 271;
const MALICE_BLAST = 370;

info <- {
	name = "iceee_druiddeath_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Druid Death"
	speed = 2
}

function on_combat_ready()
	if(!ai.is_on_cooldown("Stat"))
		ai.use_highest_once(SPIRIT_OF_SOLOMON);

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	ai.use(MELEE);
	ai.exec(main);
}

function main() {
	if(ai.get_will() >= 1) {
		if(ai.get_will_charge() >= 4 && randmod(5) < 3) 	
			ai.use_highest_once(MALICE_BLAST);
		else {
			if(!ai.is_on_cooldown("DeathlyDart") && ai.get_will() >= 33 && randmod(5) < 3)
				ai.use_highest_once(DEATHLY_DART);
			else if(ai.get_will() >= 4 && randmod(5) < 3)
				ai.use_highest_once(WITHER);
		}
	}
	
	ai.exec(main);
}
