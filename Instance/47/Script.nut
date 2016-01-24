::info <- {
	name = "Vespin Lair",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

gwarthim_kills <- 0;
hivequeen_kills <- 0;

function on_kill(creatureDefID, creatureID)
	if(creatureDefID == 426 && ++gwarthim_kills >= 6) 
		inst.spawn(788529871, 1073, 0);
	else if(creatureDefID == 411 && ++hivequeen_kills >= 8) 
		inst.spawn(788529927, 649, 0);
	else if(creatureDefID == 649)
		//  Frelon's Treasure
		inst.spawn(788529747,0,0);
