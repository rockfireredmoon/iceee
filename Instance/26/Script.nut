::info <- {
	name = "Down Below",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Down Below",
	queue_events = true
}

hoffy_STEPUP <- 0;
overseerAlwynn_STEPUP <- 0;
overseerFlammus_STEPUP <- 0;
overseerKeyt_STEPUP <- 0;
overseerMallord_STEPUP <- 0;
overseerZanthryn_STEPUP <- 0;
fillus_STEPUP <- 0;
strategistZao_STEPUP <- 0;
henchmanGrakk_STEPUP <- 0;

// Scripted calls from the spawn package

function on_package_kill_1_dng_db_hoffy_k_34_STEPUP() {
	hoffy_STEPUP++;
	if(hoffy_STEPUP == 7)
		inst.spawn(436213159, 3028, 0);
}

function on_package_kill_1_dng_db_overseerAlwynn_STEPUP() {
	overseerAlwynn_STEPUP++;
	if(overseerAlwynn_STEPUP == 11) 
  		inst.spawn(436214309, 3050, 0);
}

// NOTE: 11 spawn points but one is isolated far away
function on_package_kill_1_dng_db_overseerFlammus_STEPUP() {
	overseerFlammus_STEPUP++;
	if(overseerFlammus_STEPUP == 10)
  		inst.spawn(436213441, 3053, 0);
}

function on_package_kill_1_dng_db_overseerKeyt_STEPUP() {
	overseerKeyt_STEPUP++;
	if(overseerKeyt_STEPUP == 11)
  		inst.spawn(436213453, 3052, 0);
}

function on_package_kill_1_dng_db_overseerMallord_STEPUP() {
	overseerMallord_STEPUP++; 
	if(overseerMallord_STEPUP == 11)
		inst.spawn(436213475, 3051, 0);
}

function on_package_kill_1_dng_db_overseerZanthryn_STEPUP() {
	overseerZanthryn_STEPUP++;
	if(overseerZanthryn_STEPUP == 11)
  		inst.spawn_at(436213496, 3049, 0);
}

function on_package_kill_1_dng_db_Fillus_k_35_STEPUP() {
	fillus_STEPUP++;
	if(fillus_STEPUP == 16)
  		inst.spawn(436213685, 3068, 0);
}

// Elian is one of the spawnpoints, so do with 3 instead of 4
function on_package_kill_1_dng_db_StrategistZao_r_35_STEPUP() {
	strategistZao_STEPUP++;
	if(strategistZao_STEPUP == 3)
  		inst.spawn(436213702, 3078, 0);
}

function on_package_kill_1_dng_db_HenchmanGrakk_d_35_STEPUP() {
	henchmanGrakk_STEPUP++; 
	if(henchmanGrakk_STEPUP == 6)
  		inst.spawn(436213716, 3079, 0);
}
