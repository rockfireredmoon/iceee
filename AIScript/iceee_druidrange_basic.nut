/*
 * Basic Druid Range Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */

const TAURIAN_MIGHT = 120;
const STING = 248;
const IMPACT_SHOT = 525;
const DEADLY_SHOT = 301;
const MELEE = 32760;
//const SOUL_BURST = 366;
//const MYSTIC_MISSILE = 278;
//const SOUL_NEEDLES = 265;

info <- {
	name = "iceee_druidrange_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Druid Range"
	speed = 2
}

function on_combat_ready()
	if(!ai.is_on_cooldown("Stat"))
		ai.use_highest_once(TAURIAN_MIGHT);

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	ai.use(MELEE);
	ai.exec(main);
}

function main() {
	if(ai.get_might() >= 1) {
		if(ai.get_might_charge() >= 3 && randmod(5) < 3)
			ai.use_highest_once(DEADLY_SHOT); 	
		else {
			if(!ai.is_on_cooldown("ImpactShot") && ai.get_might() >= 3 && randmod(5) < 3)
				ai.use_highest_once(IMPACT_SHOT);
			else if(ai.get_might() >= 3 && randmod(5) < 3)
				ai.use_highest_once(STING);
		}
	}
	
	ai.exec(main);
}
