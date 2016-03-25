/*
 * Dialog script for Bounder Day quest givers
 */
 

// Script Info
info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Dialog script for Bounder day quest givers",
	idle_speed = 1
}

/* The minimum and maximum number of seconds between a phrase. Remember
 that a phrase will stay on the screen for a short amount time. To short a minimum
 delay and the phrases might stack up. */

const MIN_SECONDS = 8;
const MAX_SECONDS = 32;

/* The phrases to say. This is a 'table' or 'arrays'. Each table
 * element has a key of creature_DEFID. The value of the element
 * is an array of possible phrases that creature might say.
 */
phrases <- {
	// Hopps
	creature_4500 = [
		"PHRASE 1 - Happy Bounders Day!",
		"PHRASE 2 - Eggs, eggs, everywhere! How many can you find?",
		"PHRASE 3 - Rumor is, one of us has hidden a Golden Egg somewhere in Camelot. Can you find it?"
	],
	// Lightfoot
	creature_4501 = [
		"PHRASE 1 - Happy Bounders Day!",
		"PHRASE 2 - Ah! There's nothing better than Emerald Eggs and a Shamrock!",
		"PHRASE 3 - Rumor is, one of us has hidden a Golden Egg somewhere in Camelot. Can you find it?"
	],
	// Cottentail
	creature_4502 = [
		"PHRASE 1 - Happy Bounders Day!",
		"PHRASE 2 - The Great Forest looks so lovely in Springtime!",
		"PHRASE 3 - Rumor is, one of us has hidden a Golden Egg somewhere in Camelot. Can you find it?"
	]
};

/* The main function. This won't get called until it is queued once */

function say_something() {
	/* Say a random phrase for this creature */	
	ai.speak(phrases["creature_" + ai.get_self_def_id()][randmodrng(0, phrases.len())]);
	
	/* Requeue this function for some random point between MIN_SECONDS and MAX_SECONDS */		
	ai.queue(say_something, ( MIN_SECONDS + randmodrng(0, MAX_SECONDS) ) * 1000);
}

/* We need to queue the function once to get it started */
ai.queue(say_something, 0);
