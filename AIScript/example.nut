/*
 * All scripts should define an 'info' variable, describing itself.
 */
info <- {
	name = "example",
	enabled = true,
	author = "Emerald Icemoon",
	description = "A simple melee example script. As soon as target is acquired and aggro gained, the creature will attack using melee."
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	main();
}

function tryMelee() {
	// For backwards compatibility
	ai.use(32766);
}

function main() {
	tryMelee();
	if(ai.has_target())
		ai.exec(main);
}
