::info <- {
	name = "Haunted Grove",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Haunted Grove and  Taylin Rotbreath's Chest",
	queue_events = true
}

killcount <- 0;

function on_kill(creatureDefID, creatureID) {
	if(creatureDefID == 2524)
		// spawn Taylin Rotbreath's Treasure
		inst.spawn(1308623828,0,0);
	else if(array_contains([ 2520, 2521, 2522, 2523],creatureDefID) && ++killcount == 8) 
		inst.spawn(1029380,0,0);
}
