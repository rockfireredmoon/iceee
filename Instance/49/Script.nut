::info <- {
	name = "Djinn Temple",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill(creatureDefID, creatureID)
	if(creatureDefID in [813,814,816,817,820] && ++kills >= 27) 
		inst.spawn(822084777, 2659, 0);
	else if(creatureDefID == 711)
		inst.spawn(822085111,0,0);
