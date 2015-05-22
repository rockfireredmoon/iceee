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
	if(inst.spawn(PAWL_BABE_SPAWNER_PROP_ID,1099,0) < 1) 
		inst.queue(on_kill_1098, 5000);
}

/*
 * Grunes opening event
 *
 * Each stage intended to be manually called using "script.exec grunes_event_1" etc
 * by the GM
 */
 
grunes_tnt_props <- [ 1500020, 1500021, 1500022, 1500023, 1500024 ];
grunes_rocks <- [ 151058504, 151058507, 151058505, 151058510, 151058508, 151058514 ];
grunes_tnt_cids <- [];
grunes_tnt_steam <- [];
grunes_tnt_explosion <- [];

// Lay TNT
function grunes_lay_tnt() {
	if(grunes_tnt_cids.len() == 0) {
		foreach(prop in grunes_tnt_props) 
			grunes_tnt_cids.append(inst.spawn(prop,0,0));
	}
}

// Steam
function grunes_light_fuse() {	
	if(grunes_tnt_steam.len() == 0) {
		foreach(prop in grunes_tnt_props) 
	    	grunes_tnt_steam.append(inst.effect(prop,"Par-Steam",5,0,10,0));
	    inst.play_sound("Sound-ModSound|Sound-timebomb.ogg");
	    inst.queue(grunes_explode, 5000);
	}
}

// Explosion
function grunes_explode() {	
	if(grunes_tnt_explosion.len() == 0) {
		foreach(prop in grunes_rocks) 
	    	grunes_tnt_explosion.append(inst.effect(prop,"Par-BigExplosion",5,0,10,0));
	    
	    inst.queue(grunes_remove_tnt_and_steam, 1000);
	    inst.queue(grunes_remove_boulders, 5000);
	 }
}

// Remove boulders
function grunes_remove_boulders() {
	grunes_clear_explosion();
	foreach(prop in grunes_rocks) {
    	inst.remove_prop(prop);
	}
}

// Remove explosion
function grunes_clear_explosion() {
	for(local a = 0 ; a < grunes_tnt_explosion.len(); a++) 
		inst.restore(grunes_rocks[a], grunes_tnt_explosion[a]);
	grunes_tnt_explosion.clear();
}

// Remove TNT and steam
function grunes_remove_tnt_and_steam() {
	for(local a = 0 ; a < grunes_tnt_steam.len(); a++) 
		inst.restore(grunes_tnt_props[a], grunes_tnt_steam[a]);		 
	foreach(cid in grunes_tnt_cids)
		inst.despawn(cid);
	grunes_tnt_cids.clear();
	grunes_tnt_steam.clear();
}

// Clean up
function grunes_cleanup() {
	grunes_clear_explosion();
	grunes_remove_tnt_and_steam();
}