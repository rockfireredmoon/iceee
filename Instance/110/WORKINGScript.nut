/*
 * Turns a PVP instance into a game of Capture the Flag
 *
 */

::info <- {
	name = "Capture The Flag - Redemption",
	author = "Emerald Icemoon",
	description = "Turns a PVP instance into a game of Capture the Flag",
	queue_events = true
}

/*
 * These constants are the only variables you might need to change when
 * apply this script to a different instance.
 */

// How many flags to win
const MAX_FLAGS = 3;

const CDEFID_RED_TEAM_REGISTRAR = 7844;
const CDEFID_BLUE_TEAM_REGISTRAR = 7845;
const CDEFID_RED_FLAG = 7840;
const CDEFID_BLUE_FLAG = 7841;
const CDEFID_RED_FLAG_UNGUARDED = 7846;
const CDEFID_BLUE_FLAG_UNGUARDED = 7847;
const PROP_RED_FLAG_SAFE = 1500042;
const PROP_BLUE_FLAG_SAFE =1500041;
const PROP_RED_TEAM_REGISTRAR =1500043;
const PROP_BLUE_TEAM_REGISTRAR =1500040;

// State variables

cid_blue_safe <- 0;
cid_red_safe <- 0;
cid_blue_registrar <- 0;
cid_red_registrar <- 0;
cid_red_flag_unguarded <- 0;
cid_blue_flag_unguarded <- 0;

has_red_flag <- 0;
has_blue_flag <- 0;

red_score <- 0;
blue_score <- 0;

red_team <- [];
blue_team <- [];

// 

/*
 * Start the game
 */
function start_ctf() {
	cid_red_registrar = spawn_retry(PROP_RED_TEAM_REGISTRAR);
	cid_blue_registrar = spawn_retry(PROP_BLUE_TEAM_REGISTRAR);
}

function spawn_retry(prop) {
	local cid = inst.spawn(prop, 0,0);
	if(cid < 1) {
		print("Retrying spawn " + prop + " in 5 seconds\n");
		inst.queue(function() { spawn_retry(prop); }, 5000);
	}
	return cid;
}

function stop_ctf() {
	despawn_red_flag();
	despawn_blue_flag();
	despawn_registrars();
	detach_flags();
	despawn_red_flag_unguarded();
	despawn_blue_flag_unguarded();
	
	red_score = 0;
	blue_score = 0;

	red_team.clear();
	blue_team.clear();
}

function on_halt() {
	print("Halted\n");
	stop_ctf();
}

function detach_flags() {
	if(has_red_flag > 0) {
		// Queued to try and work around a problem with updating appearance while switching instances
		local w = has_red_flag;
		inst.queue(function() {
			inst.detach_item(sourceCId, "Item-CTF_Red", "ctf");
		}, 500);
		has_red_flag = 0;
	}
	if(has_blue_flag > 0) {
		// Queued to try and work around a problem with updating appearance while switching instances
		local w = has_blue_flag;
		inst.queue(function() {
			inst.detach_item(sourceCId, "Item-CTF_Blue", "ctf");
		}, 500);
		has_blue_flag = 0;
	}
}

function despawn_registrars() {
	// TODO check if the server cares if the spawn does not exist
	if(cid_red_registrar > 0) {
		inst.despawn(cid_red_registrar);
		cid_red_registrar = 0;
	}	
	if(cid_blue_registrar > 0) {
		inst.despawn(cid_blue_registrar);
		cid_blue_registrar = 0;
	}
}

function despawn_red_flag_unguarded() {
	if(cid_red_flag_unguarded > 0) {
		inst.despawn(cid_red_flag_unguarded);
		cid_red_flag_unguarded = 0;
	}
}

function despawn_blue_flag_unguarded() {
	if(cid_blue_flag_unguarded > 0) {
		inst.despawn(cid_blue_flag_unguarded);
		cid_blue_flag_unguarded = 0;
	}
}

function despawn_red_flag() {
	if(cid_red_safe > 0) {
		inst.despawn(cid_red_safe);
		cid_red_safe = 0;
	}
}

function despawn_blue_flag() {
	if(cid_blue_safe > 0) {
		inst.despawn(cid_blue_safe);
		cid_blue_safe = 0;
	}
}

function check_team(taker) {
	if(red_team.find(taker) >= 0) {
		inst.info("You are already in the red team.");
		return false;
	}
	else if(blue_team.find(taker) >= 0) {
		inst.info("You are already in the blue team.");
		return false;
	}  
	return true;
}

function drop_red_flag() {
	inst.detach_item(has_red_flag, "Item-CTF_Red", "ctf");
	cid_red_flag_unguarded = inst.spawn_at(CDEFID_RED_FLAG_UNGUARDED, inst.get_location(has_red_flag), 0, 1);
	has_red_flag = 0;
}

function drop_blue_flag() {
	inst.detach_item(has_blue_flag, "Item-CTF_Blue", "ctf");
	cid_blue_flag_unguarded = inst.spawn_at(CDEFID_BLUE_FLAG_UNGUARDED, inst.get_location(has_blue_flag), 0, 1);
	has_blue_flag = 0;
}

//
// Generic Interact Events
//

function on_player_death(cid) {
	if(has_red_flag == cid)
		inst.queue(function() { drop_red_flag(); }, 2500);
	else if(has_blue_flag == cid)
		inst.queue(function() { drop_blue_flag(); }, 2500);
}

/*
 * Called when player leaves the instance for whatever
 * reason.
 */
function on_remove(cid) {
	/* If the player the left was carrying the flag, return it to the appropriate base */
	
	// TODO drop the flag from where they left instead (at least for a time)
	print(cid + " LEFT THE INSTANCE!\n"); 

	if(cid == has_red_flag) {
		inst.detach_item(cid, "Item-CTF_Red", "ctf");		
		cid_red_flag_unguarded = inst.spawn_at(CDEFID_RED_FLAG, inst.get_location(cid), 0, 2048);
		has_red_flag = 0;
	}
	
	if(cid == has_blue_flag) {
		inst.detach_item(cid, "Item-CTF_Blue", "ctf");
		cid_blue_flag_unguarded = inst.spawn_at(CDEFID_BLUE_FLAG, inst.get_location(cid), 0, 2048);
		has_blue_flag = 0;
	}
}

function on_use(sourceCId, targetCId, targetCDefId) {

	if(targetCDefId == CDEFID_RED_TEAM_REGISTRAR) {
		if(check_team(sourceCId)) {		
			if(inst.get_party_id(sourceCId) > 0) {
				inst.info("You cannot already be in a party to join a PvP team");
			}
			else {		
				inst.local_broadcast(inst.get_display_name(sourceCId) + " joins the red team");
				red_team.append(sourceCId);
				//if(red_team.len() == 1)
					//cid_red_safe = spawn_retry(PROP_RED_FLAG_SAFE);
				
				// Spawn both for now so testing is easier	
				if(red_team.len() == 1 && blue_team.len() == 0) {
					cid_blue_safe = spawn_retry(PROP_BLUE_FLAG_SAFE);			
					cid_red_safe = spawn_retry(PROP_RED_FLAG_SAFE);
				}
			}
		}
	}
	
	if(targetCDefId == CDEFID_BLUE_TEAM_REGISTRAR) {
		if(check_team(sourceCId)) {
			if(inst.get_party_id(sourceCId) > 0) {
				inst.info("You cannot already be in a party to join a PvP team");
			}
			else {
				inst.local_broadcast(inst.get_display_name(sourceCId) + " joins the blue team");
				blue_team.append(sourceCId);
				//if(blue_team.len() == 1)
					//cid_blue_safe = spawn_retry(PROP_BLUE_FLAG_SAFE);
				
				// Spawn both for now so testing is easier	
				if(blue_team.len() == 1 && red_team.len() == 0) {
					cid_blue_safe = spawn_retry(PROP_BLUE_FLAG_SAFE);			
					cid_red_safe = spawn_retry(PROP_RED_FLAG_SAFE);
				}
			}
		}
	}
	
	if(targetCDefId == CDEFID_RED_FLAG_UNGUARDED) {
		if(red_team.find(sourceCId) >= 0) {
			inst.info(inst.get_display_name(sourceCId) + " returned the Red team's flag");
			cid_blue_safe = inst.spawn(PROP_RED_FLAG_SAFE, 0,0);
			despawn_red_flag_unguarded();
		}
		else if(blue_team.find(sourceCId) >= 0) {
			inst.local_broadcast("Red flag found by " + inst.get_display_name(sourceCId));
			has_red_flag = sourceCId;
			despawn_red_flag_unguarded();
			inst.attach_item(sourceCId, "Item-CTF_Red", "ctf");
		}
		else 
			inst.info("Spectators can't take the flag! (" + sourceCId + ")");
	}
	
	if(targetCDefId == CDEFID_BLUE_FLAG_UNGUARDED) {
		if(blue_team.find(sourceCId) >= 0) {
			inst.info(inst.get_display_name(sourceCId) + " returned the Blue team's flag");
			cid_blue_safe = inst.spawn(PROP_BLUE_FLAG_SAFE, 0,0);
			despawn_blue_flag_unguarded();
		}
		else if(red_team.find(sourceCId) >= 0) {
			inst.local_broadcast("Blue flag found by " + inst.get_display_name(sourceCId));
			has_blue_flag = sourceCId;
			despawn_blue_flag_unguarded();
			inst.attach_item(sourceCId, "Item-CTF_Blue", "ctf");
		}
		else 
			inst.info("Spectators can't take the flag! (" + sourceCId + ")");
	}
}

/*
 * InteractDef script functions. See InteractDef.txt
 */

function red_flag_won(winner, flagBaseCDefID) {
	if(has_red_flag != winner) {
		inst.info("You don't have the red flag!");
	}
	else {	
		blue_score++;
		
		// Spawn a temporary red flag to show its been won
		local temp_flag_cid = inst.spawn_at(CDEFID_RED_FLAG, Vector3I(14240, 183, 11086), 0, 0);
		inst.queue(function() {
			inst.despawn(temp_flag_cid);
			
			// Spawn the red flag back at red base
			if(blue_score < MAX_FLAGS) {
				cid_red_safe = inst.spawn(PROP_RED_FLAG_SAFE, 0,0);
			}
		}, 5000);

		inst.detach_item(winner, "Item-CTF_Red", "ctf");

		if(blue_score == MAX_FLAGS) {
			inst.broadcast(inst.get_display_name(winner) + " landed the winning flag for the Blue Team!  " + blue_score + " to " + red_score + ".");
			stop_ctf();
		}
		else {
			inst.local_broadcast(inst.get_display_name(winner) + " on the Blue Team scores! They now have " + blue_score + " flags");
		}
		has_red_flag = 0;
				
	}
}

function blue_flag_won(winner, flagBaseCDefID) {
	if(has_blue_flag != winner)
		inst.info("You don't have the blue flag!");
	else {
		red_score++;
		
		// Spawn a temporary blue flag to show its been won
		local temp_flag_cid = inst.spawn_at(CDEFID_BLUE_FLAG, Vector3I(13546, 182,11239), 0, 0);
		inst.queue(function() {
			inst.despawn(temp_flag_cid);
			
			// Spawn the red flag back at red base
			if(red_score < MAX_FLAGS) {
				cid_blue_safe = inst.spawn(PROP_BLUE_FLAG_SAFE, 0,0);
			}
		}, 5000);
	
		inst.detach_item(winner, "Item-CTF_Blue", "ctf");
		
		if(red_score == MAX_FLAGS) {
			inst.broadcast(inst.get_display_name(winner) + " landed the winning flag for the Red Team!  " + red_score + " to " + red_score + ".");
			stop_ctf();
		}
		else {
			inst.local_broadcast(inst.get_display_name(winner) + " on the Red Team scores! They now have " + red_score + " flags");
		}
		has_blue_flag = 0;
	}
}

function red_flag_taken(taker, taken) {
	if(red_team.find(taker) >= 0) 
		inst.info("You can't take your own flag!");
	else if(blue_team.find(taker) >= 0) {
		inst.local_broadcast("Red flag taken by " + inst.get_display_name(taker));
		has_red_flag = taker;
		despawn_red_flag();
		inst.attach_item(taker, "Item-CTF_Red", "ctf");
	}
	else {
		inst.info("You are not in a team!");
	}
}

function blue_flag_taken(taker, taken) {
	if(blue_team.find(taker) >= 0) {
		inst.info("You can't take your own flag!");
	}
	else if(red_team.find(taker) >= 0) {
		inst.local_broadcast("Blue flag taken by " + inst.get_display_name(taker));
		has_blue_flag = taker;
		despawn_blue_flag();
		inst.attach_item(taker, "Item-CTF_Blue", "ctf");
	}
	else 
		inst.info("You are not in a team!");
}

inst.queue(start_ctf, 5000);