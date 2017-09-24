/*
 * A port of knight_brave_6 to the Squirrel scripting system
 *
 * 5063 - Taurian Might, 5 might, buff
 * 5157 - Assault, 3 might, add 1 mcharge
 * 5221 - Bash, 1 might, need 1-3 mcharge
 * 32766 - melee
 */

info <- {
	name = "knight_brave_6",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Simple Knight AI"
}

function on_target_lost(targetCID) 
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(5063);
	ai.exec(main);
}

function main() {
	ai.use(32766);
	if(ai.get_might_charge() >= 2) {
		ai.use(5221);
		ai.sleep(1000);
	}
	if(ai.get_might() >= 3) {
		ai.use(5157);
		ai.sleep(1000);
	}
	if(ai.has_target())
		ai.exec(main);
}


