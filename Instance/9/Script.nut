/*
 * Instance script for whole of Europe
 *
 * Processes:-
 *
 *  Pawl/Babe - Must kill 20 trailblazers to spawn first Pawl, then
 *  when he is killed, Babe is spawned.
 */


// Pawl/Babe and the Trail Blazers

const RESET_TRAILBLAZERS_DELAY = 180000; // 3 minutes
const KILLS_TO_SPAWN_PAWL =  5;
const PAWL_BABE_SPAWNER_PROP_ID = 151026296;

trailblazer_kills <- 0;
last_trailblazer_kill <- 0;

function on_package_kill_4_Trailblazer_StepUp() {
	local now = inst.get_server_time();

	// If there was too long a gap between kills, reset the count
	if(now > last_trailblazer_kill + RESET_TRAILBLAZERS_DELAY)
		trailblazer_kills = 0;
	last_trailblazer_kill = now;

	trailblazer_kills++;
	if(trailblazer_kills >= KILLS_TO_SPAWN_PAWL) {
		inst.spawn(PAWL_BABE_SPAWNER_PROP_ID,1098,0);
		trailblazer_kills = 0;
	}
}

function on_kill_1098() {
		inst.spawn(PAWL_BABE_SPAWNER_PROP_ID,1099,0);
}