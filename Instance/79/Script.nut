/*
 * A squirrel script version of Alimat's script
 */
 
info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Alimat",
	idle_speed = 1
}

trigger <- 0;

function on_use_2134() {
	inst.spawn(10000001,0,0);
		
	inst.sleep(7500);
	
	// Phase 1
	if(trigger == 0) {
		inst.spawn(1325400172,0,0); // Left fire holder
		inst.spawn(1325400193,0,0); // Back mob
		inst.spawn(1325400196,0,0); // Left mob
		inst.spawn(1325400194,0,0); // Forward mob
		inst.spawn(1325400192,0,0); // Right mob
		trigger = 1;
	}
	inst.sleep(7500);
	
	// Phase 2	
	if(trigger == 1) {
		inst.spawn(1325400170,0,0); // Right fire holder
		inst.spawn(1325400191,0,0); // Left mob
		inst.spawn(1325400173,0,0); // Forward mob
		inst.spawn(1325400195,0,0); // Right mob
		trigger = 2;
	}
	
	inst.sleep(7500);
	if(trigger == 2) {
		inst.spawn(1325400171,0,0); // Forward fire holder
		inst.spawn(1325400189,0,0); // Left mob
		inst.spawn(1325400190,0,0); // Right mob
		trigger = 3;
	}
}

function on_use_finish_2134() {
	if(trigger == 3) {
		inst.spawn(1325400187,0,0);
		trigger = 4;
	}
}
