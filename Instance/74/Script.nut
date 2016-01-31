::info <- {
	name = "Earthend",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Earthend",
	queue_events = true
}

function on_kill(creatureDefID, creatureID) {
	if(creatureDefID == 2524 && inst.count_alive(2075) == 0) {
		inst.spawn(1241514516,0,0);
	}
	else if(array_contains([ 2071, 2072, 2073 ],creatureDefID) && inst.count_alive(2074) == 0) {
		inst.spawn(1241514515,0,0);
	}
}