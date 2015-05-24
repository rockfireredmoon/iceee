/*
 * Special script for Beta test *
 */

const ACTOR_CDEFID = 7813;
const ANIMATION = "CombatIdle";
const SPEAK_CHANCE = 33;
const EMOTE_CHANCE = 33;

phrases <- [
	"Bark!",
	"*grumbles*",
	"Don't talk to that shroomie, talk to me!",
	"You! Are you here for the beta test?"
];

function say_something() {
	local cid = inst.cids(ACTOR_CDEFID)[0];
	if(randmodrng(0, 100) <= SPEAK_CHANCE) {
		inst.queue(function() {
			inst.emote(cid, ANIMATION);
			inst.creature_chat(cid, "s/", phrases[randmodrng(0, phrases.len())]);
		}, 1000);
	}
	inst.queue(say_something, 10000);
}

inst.queue(say_something, 1000);
