::info <- {
	name = "Junk Palace",
	author = "Emerald Icemoon",
	description = "Handles missile explosion sequence",
	queue_events = false
}

kills <- 0;
used <- [];
active <- false;
boss_cid <- 0;
boss_home <- Point(0,0);
center <- Point(1457,1865);
phase <- 0;
debug <- false;

// Locations

// Creature Definition IDs

const CDEF_BOSS_HAWG = 1387;
const CDEF_INTERACT_SPHERE = 7863;

// Prop IDs
const MISSILE_1 = 1150559;
const MISSILE_2 = 1150558;
const SPAWN_1 = 1150561;
const SPAWN_2 = 1150560;

function on_kill_1387() {
	// Boss Hawg's Treasure
	if(debug)
		inst.info("boss hawg dead");
	inst.spawn(1154725, 0, 0);
}

function disengage(cid) {
	inst.unhate(cid);
	inst.interrupt(cid);
	inst.set_flag(cid, SF_NON_COMBATANT, true);
}

function find_boss() {
	boss_cid = inst.get_npc_id(CDEF_BOSS_HAWG);
	if(boss_cid == 0) 
		inst.queue(find_boss, 5000);
	else
		found_boss();
}

function found_boss() {
	local vec = inst.get_location(boss_cid);
	boss_home = Point(vec.x, vec.z);
}

function is_launched(v) {
	foreach(c in used)
		if(c == v)
			return true;
	return false;
}
	
function is_usable(cid, cdef_id, by_cid, by_cdef_id) {
	if(cdef_id == CDEF_INTERACT_SPHERE && !is_launched(cid)) {
		return "Y";
	}
	return "N";
}

function on_use(cid, target_cid, target_cdef_id) {
	if(target_cdef_id == CDEF_INTERACT_SPHERE) {
		
		inst.interact(cid, "Pressing the button", 3000, false, function() {
			/* Prevent use while sequence is running */
			if(active)
				return;
			active = true;
		
			phase++;
			
			// Determine missile prop
			local spawn_prop = inst.creature_spawn_prop(target_cid);
			local missile_prop = inst.creature_spawn_prop(target_cid) == SPAWN_1 ? MISSILE_1 : MISSILE_2;
		
			/* Make Boss Hawg run towards the missile to try to disarm */
			if(boss_cid != 0) {
				disengage(boss_cid);
				local missile_loc = inst.get_location(cid);
				inst.creature_chat(boss_cid, "s/", "No! Why did you do that! You'll kill us all..");
				inst.walk_then(boss_cid, Point(missile_loc.x,missile_loc.z), -1, CREATURE_RUN_SPEED * 2, 00, function(res) {
					inst.creature_chat(boss_cid, "s/", "The green wire, the red wire, which is it !?!");
					inst.emote(boss_cid, "Dig_Shovel");
				});
			}			
		
    		local steam = inst.effect(missile_prop,"Par-Steam",50,0,10,0);
    		inst.queue(function() {
    			inst.restore(missile_prop, steam);
    		}, 11000);
    		
    		
		    inst.shake(50, 10, 250);
			inst.info("Starting countdown ..");
			
  			inst.play_sound("Sound-Ambient-Stage1|Sound-Ambient-TenSecondCountdown.ogg");
			
			// Countdown messages
			for(local i = 1 ; i <= 10 ; i++) {
				local z = i;
				inst.queue(function() {				
					inst.info((11 - z) + " ..");
				}, i * 1000);
			}
			
			inst.queue(function() {			 
    			local smoke = inst.effect(missile_prop,"Par-Black_Smoke",4,0,10,0);
    			inst.queue(function() {
    				inst.restore(missile_prop, smoke);
    			}, 4000);
				inst.shake(100, 7, 200); 
			}, 7000);
			
			inst.queue(function() { 
				inst.shake(200, 4, 200);			 
    			local explosion = inst.effect(missile_prop,"Par-Flame",50,0,10,0);
    			inst.queue(function() {
    				inst.remove_prop(missile_prop);
    				
    				/* Stun/Damage everyone in the area of effect */
	    			if(!inst.creature_use(target_cid, _AB("Nuclear Fallout"))) {
	    				if(debug)
	    					inst.info("STUN FAILED :(");
	    			}
	    			
		    		/* The boss gets special treatment. Delay slightly to let to player attack work first */
	    			inst.queue(function() {
    					inst.set_target(target_cid, boss_cid);
	    				
	    				/* Send the boss back home when his stun ends */
	    				if(phase == 1) {
	    					if(!inst.creature_use(target_cid, _AB("Nuclear Sickness"))) {
			    				if(debug)
		                        	inst.info("failed Nuclear Sickness");
	    					}
		    				inst.queue(function() {
								inst.walk_then(boss_cid, boss_home, 70, CREATURE_RUN_SPEED * 2, 00, function(res) {
									inst.creature_chat(boss_cid, "s/", "Hahaha I'm still too strong for you!");
									inst.set_flag(boss_cid, SF_NON_COMBATANT, false);
								});
							}, 5500);
						}
						else if(phase == 2) {
	    					inst.creature_use(target_cid, _AB("Nuclear Death"));
	    					inst.queue(function() {
    							inst.set_target(target_cid, boss_cid);
	    						inst.creature_use(target_cid, _AB("Nuclear Poison"));
	    					}, 500);
		    				inst.queue(function() {
								inst.walk_then(boss_cid, center, -1, CREATURE_RUN_SPEED, 00, function(res) {
									inst.creature_chat(boss_cid, "s/", "Come on! Face me!");
									inst.rotate_creature(boss_cid, 70);
									inst.set_flag(boss_cid, SF_NON_COMBATANT, false);
								});
							}, 5500);
						}
	    				
	    				inst.queue(function() {
	    					inst.despawn(target_cid);
	    				}, 2000);
	    			}, 1000);
    				
    			}, 1000);
				inst.queue(function() {
    				inst.restore(missile_prop, explosion);
    			}, 2000); 
			}, 11000);
			
			inst.queue(function() { active = false; }, 15000);
		});
		used.append(target_cid);
		return true;
	}
}

inst.queue(find_boss, 0);

