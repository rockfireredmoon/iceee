::info <- {
	name = "Dark Depths",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

taintmaw_kills <- 0;
marblightclaw_kills <- 0;

function on_kill(creatureDefID, creatureID)
	if(array_contains([966,967],creatureDefID)  && ++taintmaw_kills >= 9) 
		inst.spawn(805308154, 970, 0);
	else if(array_contains([960,964,965,968,2640],creatureDefID) && ++marblightclaw_kills >= 7) 
		inst.spawn(805308827, 0, 0);
	else if(creatureDefID == 975)
		//  Vitiator's Treasure (from Aram Norr)
		inst.spawn(805308084,0,0);
		