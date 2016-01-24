::info <- {
	name = "Giant Rotted Tree",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill(creatureDefID, creatureID)
	if(creatureDefID == 279 && ++kills == 20) 
		inst.spawn(754974951, 994, 0);

