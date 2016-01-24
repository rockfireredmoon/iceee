::info <- {
	name = "Rolsburg Mine",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill_2378()
	// Vangar One Eye's Treasure
	inst.spawn(1090520592, 0, 0);
	
function on_package_kill(package_name) 
	if(package_name == "2_rm_ArkadosSTEPUP" && ++kills == 10) 
		inst.spawn(1090520584, 2685, 0);
