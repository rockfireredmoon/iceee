/*
 * A port of Newb_Knight (Melee only) to the Squirrel scripting system
 *
 * 32766 - melee
 */

info <- {
	name = "Newb_Knight",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Very Simple Knight AI"
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	main();
}

function main() {
	ai.use(32766);
	ai.exec(main);
}

