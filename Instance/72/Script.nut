::info <- {
	name = "Rotted Maze",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill_949()
	// Lost One's Treasure
	inst.spawn(1207961763, 0, 0);
	
function on_kill_946()
	// Rotted King's Treasure
	inst.spawn(1207960970, 0, 0);

function on_package_kill(package_name) 
	if(package_name == "10_rk_MossmaSTEPUP" && ++kills == 13) 
		inst.spawn(1207961205, 1490, 0);
