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
	speed = 6
}

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	ai.use_highest(TAURIAN_MIGHT);
	ai.exec(function() {
		ai.use(MELEE);
	});
	
	ai.exec(main);
}

function main() {
	if(ai.get_might() >= 1) {

		/* Check if we have at least 3 might charges. If so, try an execute move. */
		if(ai.get_might_charge() >= 3) {	
			ai.use_highest(BLENDER);
		}
		
		if(ai.get_might() >= 3 && randmod(5) == 0) {	
			/* If we have 3 might, give a 1 in 5 chance of executing Concussion (this is to give
			   the 5 might req. abilities chance to possibly execute */
			if(!ai.is_on_cooldown("Concussion"))
				ai.use_highest(CONCUSSION);
			else
				/* Everything was on cooldown, use assault instead */
				ai.use_highest(ASSAULT);
		}
		else if(randmod(5) == 0) {
			/* We don't have enough might charges or might, so give a one in 5 chance of performing assault on this
			   cycle.  */
			ai.use_highest(ASSAULT);		
		}
	}
	
	ai.exec(main);
}
