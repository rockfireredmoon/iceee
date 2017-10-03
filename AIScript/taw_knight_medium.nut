/*
 * Squirrel version of taw_knight_medium
 */
 
/* This is the constants section. Useful for 'fixed' numbers and identifiers such
 * as ability codes. These are better than variables as they provided a faster and
 * memory efficient way of self documenting
 */
 
const ASSAULT = 228; 			//  40 
const BASH = 446; 				//  40
const CONCUSSION = 539;			//	40 [Cooldown=Concussion]
const DISARM  = 346;			//	40 [Cooldown=Disarm]
const SPELLBREAKER  = 347;		//	40 [Cooldown=Spellbreak, Might=3]
const TAURIAN_MIGHT = 448;		//	40 [GroupID=44] 
const TAURIAN_MIGHT_15 = 5063;	//	40 (15%) [GroupdID=44]
const MELEE = 32766;			//	Melee Auto-attack	

/* This section gives some information about the script. Not strictly required but
   recommended */
info <- {
	name = "taw_knight_medium",
	enabled = true,
	author = "Heathendel",
	description = "High Level Generic Knight"
	speed = 6
}

/* This function is called when a target is deselected. Nearly all AI scripts
   will have this single line, that just clears out any queued events (waiting abilities). */   
function on_target_lost(target_cid)
	ai.clear_queue();

/* This function is called when a target is selected. All AI scripts will have this function
   and it will usually start the AI sequence. Do all the stuff you want to do at the START of
   the fight in this section (such as cast buffs), before queuing the main AI loop */ 
function on_target_acquired(target_cid) {
	/* Cast special taurian might buff, but do so if the target doesnt have abilties of the provided ability group **/
	if(ai.get_buff_tier(44) == 0)
		ai.use(TAURIAN_MIGHT_15);
	else
		/* Otherwise use the normal buff */
		ai.use(TAURIAN_MIGHT);
	
	
	/* If you are executing more than one ability in a cycle, it is often best to queue any
	   subsequent abilities to the next cycle. Use the exec() function to do this. The exec
	   function either takes an inline function as below (useful for one off tasks), or 
	   the name of a named function (useful for reusing said function).*/ 
	ai.exec(function() {
		/* Activate melee */
		ai.use(MELEE);
	});
	
	/* Start the main AI loop */
	ai.exec(main);
}

/* This function gets called repeatedly all the while the creature has a active target. It is 
   the main body of the AI and decides which abilities to execute and when */ 
function main() {
	/* All abilities require at least 1 might, so do nothing (other than melee) until we have that */
	if(ai.get_might() >= 1) {

		/* Check if we have at least 3 might charges. If so, try an execute move. */
		if(ai.get_might_charge() >= 3) {	
			ai.use(BASH);
		}
		else if(ai.get_might() >= 5) {
			/* With 5 might we can execute Spellbreaker or Disarm */
			if(!ai.is_on_cooldown("Spellbreaker"))
				ai.use(SPELLBREAKER);
			else if(!ai.is_on_cooldown("Disarm"))
				ai.use(DISARM);	
			else
				/* Everything was on cooldown, use assault instead */
				ai.use(ASSAULT);
		}
		else if(ai.get_might() >= 3 && randmod(5) == 0) {	
			/* If we have 3 might, give a 1 in 5 chance of executing Concussion (this is to give
			   the 5 might req. abilities chance to possibly execute */
			if(!ai.is_on_cooldown("Concussion"))
				ai.use(CONCUSSION);
			else
				/* Everything was on cooldown, use assault instead */
				ai.use(ASSAULT);
		}
		else if(randmod(5) == 0) {
			/* We don't have enough might charges or might, so give a one in 5 chance of performing assault on this
			   cycle.  */
			ai.use(ASSAULT);		
		}
	}
	
	/* Finally we requeue the main() function. It will execute again on the next cycle.  The exact amount of
	   time this will be depends on the 'speed' setting of the script. This queued event MAY get removed if
	   the AI loses it's target */ 
	ai.exec(main);
}
