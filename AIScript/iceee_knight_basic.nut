/*
 * Basic Knight Script for new overworld mob Ai.
 *
 * This script automatically adjusts the ability tier in use
 * based on the creatures level (so we don't need different
 * scripts for different levelled creatures).
 *
 * https://github.com/rockfireredmoon/iceee/issues/347
 */
 
const ASSAULT = 224; 			//  1 
const BLENDER = 333; 			//  20 
const CONCUSSION = 535;			//	1 [Cooldown=Concussion]
const TAURIAN_MIGHT = 120;		//	6 [GroupID=44] 
const MELEE = 32766;			//	Melee Auto-attack	

info <- {
	name = "iceee_knight_basic",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Basic Knight"
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
		if(ai.get_might_charge() >= 3) 	
			ai.use_highest_once(BLENDER);
		else if(ai.get_might() >= 3 && randmod(5) < 3) {	
			if(!ai.is_on_cooldown("Concussion"))
				ai.use_highest_once(CONCUSSION);
			else
				ai.use_highest_once(ASSAULT);
		}
		else if(randmod(5) < 3)
			ai.use_highest_once(ASSAULT);		
	}
	
	ai.exec(main);
}
