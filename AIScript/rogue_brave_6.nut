/*
 * A port of rogue_brave_6 to the Squirrel scripting system
 *
 * 5036 - Feline's Grace, 5 might, buff
 * 5165 - Assail - 3 might, add 1 mcharge
 * 5166 - Assail - 3 might, add 1 mcharge (same damage)
 * 5225 - Disembowel, 1 might, need 1-3 mcharge 
 * 32766 - melee
 */

info <- {
	name = "rogue_brave_6",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Simple Brave Rogue AI"
}

function on_target_lost(targetCID) 
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(5036);
	ai.exec(main);
}

function main() {
	ai.use(32766);
	if(ai.get_might_charge() >= 2) {
		ai.use(5225);
		if(ai.sleep(1000)) 
			return;
	}
	if(ai.get_might() >= 3) {
		ai.use(5165);
		if(ai.sleep(2000))
			return;
	}
	if(ai.has_target())
		ai.exec(main);
}
