/*
 * A squirrel script version of Bastion_NPC_GuardBowAnimation1. Script
 * targets a single creature and fires an arrow.
 *
 * Note the old script relied on cooldown and just constant tried to attack (something).
 * I have no idea how targetting was supposed to happen, so instead this script uses
 * a single ability and it's own timers.
 *
 * The script expects a single parameter, the prop ID for the spawner of the creature 
 * to fire at.
 *
 * The creature will also randomly "say" phrases from a list defined in this script
 */
 
// Constants
const AB_PRACTICE_ARROW = 5423;
const SPEAK_CHANCE = 15;

// State variables
target_propid <- __argc > 0 ? __argv[0].tointeger() : 0;
target_cid <- 0;

// Script Info
info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Fires arrows at a target",
	idle_speed = 1
}

// Phrases
phrases <- [
	"Bullseye!",
	"I aim to please",
	"My accuracy is improving",
	"How did you miss that?",
	"I see you still have your training wheels on your bow",
	"Those anubians will be no match for my sharp eye",
	"Bow to my superior archery skills",
	"HEY! Anyone got an apple we can borrow?",
	"Well at least it landed this side of the wall this time",
	"Aren’t you a little short for an archer?"
];


function attack_target() {
	if(target_cid == 0)
		target_cid = ai.get_cid_for_prop(target_propid);
	else {
		ai.set_other_target(ai.get_self(), target_cid);
		ai.use(AB_PRACTICE_ARROW);
		
		if(randmodrng(0, 100) <= SPEAK_CHANCE) {
			ai.queue(function() {
				ai.speak(phrases[randmodrng(0, phrases.len())]);
			}, 1000);
		}
		
	}
	ai.queue(attack_target, 4000 + randmod(1000));
}

if(target_propid == 0)  
	ai.error("Error! AI script Bastion_NPC_GuardBowAnimation script called without any parameter. Requires propID of the target's spawner");
else  
	ai.queue(attack_target, 0);
