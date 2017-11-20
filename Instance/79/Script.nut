/*
 * A squirrel script version of Alimat's script
 *
 * In this version we maintain a separate trigger / phase counter
 * so re-summoning doesn't mean the player has to go through the
 * entire cycle again, but doesn't mean they have to wait for
 * the length of the entire activation sequence before Alimat
 * actually appears.
 */

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Alimat",
	idle_speed = 1
}

trigger <- 0;
phase <- 0;
handle <- 0;
spawned <- false;

function run_phase() {
	trigger++;
	if(trigger == 1) {
		if(phase == 0) {
			inst.spawn(1325400172,0,0); // Left fire holder
			inst.spawn(1325400193,0,0); // Back mob
			inst.spawn(1325400196,0,0); // Left mob
			inst.spawn(1325400194,0,0); // Forward mob
			inst.spawn(1325400192,0,0); // Right mob
			phase = 1;
		}
	}
	else if(trigger == 2) {
		if(phase == 1) {
			inst.spawn(1325400170,0,0); // Right fire holder
			inst.spawn(1325400191,0,0); // Left mob
			inst.spawn(1325400173,0,0); // Forward mob
			inst.spawn(1325400195,0,0); // Right mob
			phase = 2;
		}
	}
	else if(trigger == 3) {
		if(phase == 2) {
			inst.spawn(1325400171,0,0); // Forward fire holder
			inst.spawn(1325400189,0,0); // Left mob
			inst.spawn(1325400190,0,0); // Right mob
			phase = 3;
		}
	}
	else {
		// Ended, control will pass to on_use_finish_2134
		return;
	}
	handle = inst.queue(run_phase, 7500);
}

function on_interrupt(cid) {
	trigger = 0;
	inst.cancel(handle);
}

function on_use_2134(cid, used_cid) {
	handle = inst.queue(run_phase, 7500);
	return true;
}

function on_use_finish_2134() {
	spawned = true;
	inst.spawn(1325400187,0,0);
}
