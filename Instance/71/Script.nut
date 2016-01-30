::info <- {
	name = "Zhushis Lair",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill_943()
	// Zhushi's Treasure
	inst.spawn(1191182623, 0, 0);
