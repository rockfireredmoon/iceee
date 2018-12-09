/*
 * Basic Druid Mystic Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */

const SPIRIT_OF_SOLOMON = 102; 
const MELEE = 32766;
const SOUL_BURST = 366;
const MYSTIC_MISSILE = 278;
const SOUL_NEEDLES = 265;

info <- {
	name = "iceee_druidmystic_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Druid Mystic"
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
		if(ai.get_will_charge() >= 3 && randmod(5) < 3) 	
			ai.use_highest_once(SOUL_BURST);
		else {
			if(!ai.is_on_cooldown("DeathlyDart") && ai.get_will() >= 33 && randmod(5) < 3)
				ai.use_highest_once(DEATHLY_DART);
			else if(ai.get_will() >= 3 && randmod(5) < 3)
				ai.use_highest_once(SOUL_NEEDLES);
		}
	}
	
	ai.exec(main);
}
