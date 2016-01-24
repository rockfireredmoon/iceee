::info <- {
	name = "Dire Wolf Den",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns and boss treasure",
	queue_events = true
}

kills <- 0;

function on_kill_940()
	// Swirling Madness's Treasure
	inst.spawn(1174406538, 0, 0);
	
function on_package_kill(package_name) 
	if(package_name == "10_fc_gakkSTEPUP" && ++kills == 21) 
		inst.spawn(1174406607, 2664, 0);
