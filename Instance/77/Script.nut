::info <- {
	name = "Anglor Dren",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Anglor Dren's Chest",
	queue_events = true
}

killcount <- 0;

function on_kill(creatureDefID, creatureID) {
	if(creatureDefID == 1995) {
		// Anglor Dren's chest
		inst.spawn(1291845811,0,0);
	}
	else if(array_contains([1993, 1994],creatureDefID)) {
		killcount++;
		if(killcount >= 6) {
			inst.spawn(1291845744,0,0);
		}
	}
}