::info <- {
	name = "Underhenge",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Fleym and Bramblebane's Chest",
	queue_events = true
}

fleym_STEPUP <- 0;

// Bramblebane's boss chest
function on_kill_290() {
	inst.spawn(469763894, 0, 0);
}

// Scripted call from the spawn package
function on_package_kill_1a_dng_uh_sparkwingPit() {
	fleym_STEPUP++;
	if(fleym_STEPUP == 10)
		inst.spawn(469764077, 2429);
}
