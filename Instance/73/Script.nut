::info <- {
	name = "Rotted Nursery",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Rotted Nursery",
	queue_events = true
}

kills <- 0;

function on_kill_2607()
	// Wither-Root's Treasure
	inst.spawn(1224737309, 0, 0);

function on_package_kill(package_name) 
	// MUST be == 18, as witherroot itself is of this package
	if(package_name == "10_rn_wither-rootSTEPUP" && ++kills == 18) 
		inst.spawn(1224737246, 2607, 0);
