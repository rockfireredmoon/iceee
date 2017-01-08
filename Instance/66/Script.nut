::info <- {
	name = "Valkals Bloodkeep",
	author = "Emerald Icemoon",
	description = "Handles the final fight!",
	queue_events = true
}

// Creature Definition IDs

const CDEF_VALKAL1 = 1385;

function on_kill_1385() {
	// Portal
  inst.info("on kill!");
	inst.spawn(1150656, 0, 0);
	inst.spawn(1127135, 0, 0);
}

