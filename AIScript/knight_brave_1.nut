/*
 * A port of knight_brave_1 to the Squirrel scripting system
 *
 * 5157 - Assault, 3 might, add 1 mcharge
 * 5221 - Bash, 1 might, need 1-3 mcharge
 * 32766 - melee
 */

info <- {
	name = "knight_brave_1",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Simple Knight AI"
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	main();
}

function main() {
	ai.use(32766);
	if(ai.get_might_charge() >= 3) {
		ai.use(5221);
		if(ai.sleep(1000))
			return;
	}
	if(ai.get_might() >= 3) {
		ai.use(5157);
		if(ai.sleep(2000))
			return;
	}
	if(ai.has_target())
		ai.exec(main);
}

