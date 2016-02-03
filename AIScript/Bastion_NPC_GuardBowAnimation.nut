/*
 * A squirrel script version of Bastion_NPC_GuardBowAnimation1. Script
 * targets a single creature and fires an arrow.
 *
 * The script expects at least one parameter, the prop ID for the spawner of the creature 
 * to fire at. Multiple prop IDs maybe supplied as further arguments.
 *
 * The script will pick the phrases to say, and the ability to use based on
 * whether the target is an enemy or not. Friendly phrases are intended for arrow
 * practice, where as enemy phrases are intended for use by archers actively
 * attacking or defending an area.
 *
 * The creature will also randomly "say" phrases from a list defined in this script
 */
 
// Constants
const AB_PRACTICE_ARROW = 5423;
const AB_BATTLE_ARROW = 5432;
const SPEAK_CHANCE = 15;

// Push prop IDs to target
target_propid <- [];
for(local argi = 0 ; argi < __argc ; argi++) {
	target_propid.push(__argv[argi].tointeger());
}

// Script Info
info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Fires arrows at a target",
	idle_speed = 1
}

// Phrases
friendly_phrases <- [
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

enemy_phrases <- [
	"Got 'im, right in the anubians!",
	"Ready .. aim .. fire!",
	"Aim for the whites of their eyes boys!",
	"Hold .... Fire!"
];


function attack_target() {
	// Find CIDs of props
	local propId = target_propid[randmodrng(0, target_propid.len())];
	local target_cid = ai.get_cid_for_prop(propId);
	if(target_cid != 0) {
		// Target
		ai.set_other_target(ai.get_self(), target_cid);
		local phrases;
		
		// Attack
		if(ai.is_target_enemy()) {
			phrases = enemy_phrases;
			ai.use(AB_BATTLE_ARROW);
		}
		else {
			phrases = friendly_phrases;
			ai.use(AB_PRACTICE_ARROW);
		}
		
		// Speak			
		if(randmodrng(0, 100) <= SPEAK_CHANCE) {
			ai.queue(function() {
				ai.speak(phrases[randmodrng(0, phrases.len())]);
			}, 1000);
		}
		
	}
	
	ai.queue(attack_target, 4000 + randmod(1000));
}

if(target_propid == 0)  
	ai.error("Error! AI script Bastion_NPC_GuardBowAnimation script called without any parameter. Requires at least one propID of the target's spawner");
else  
	ai.queue(attack_target, 0);
