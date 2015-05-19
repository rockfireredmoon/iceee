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

const CDEFID_RED_TEAM_REGISTRAR = 7844;
const CDEFID_BLUE_TEAM_REGISTRAR = 7845;
const PROP_RED_FLAG_SAFE = 1500042;
const PROP_BLUE_FLAG_SAFE =1500041;
const PROP_RED_TEAM_REGISTRAR =1500043;
const PROP_BLUE_TEAM_REGISTRAR =1500040;

cid_blue_safe <- 0;
cid_red_safe <- 0;
cid_blue_registrar <- 0;
cid_red_registrar <- 0;

has_red_flag <- 0;
has_blue_flag <- 0;

red_score <- 0;
blue_score <- 0;

red_team <- [];
blue_team <- [];

function start_ctf() {
	cid_red_safe = inst.spawn(PROP_RED_FLAG_SAFE, 0,0);
	cid_blue_safe = inst.spawn(PROP_BLUE_FLAG_SAFE,0,0);
	cid_red_registrar = inst.spawn(PROP_RED_TEAM_REGISTRAR, 0,0);
	cid_blue_registrar = inst.spawn(PROP_BLUE_TEAM_REGISTRAR,0,0);
}

function stop_ctf() {
	despawn_red_flag();
	despawn_blue_flag();
	despawn_registrars();
}

function despawn_registrars() {
	// TODO check if the server cares if the spawn does not exist
	if(cid_red_registrar > 0) {
		inst.despawn(cid_red_registrar);
		cid_red_registrar = 0;
	}	
	if(cid_blue_registrar > 0) {
		inst.despawn(cid_red_registrar);
		cid_blue_registrar = 0;
	}
}

function despawn_red_flag() {
	if(cid_red_safe > 0) {
	   print("despawning " + cid_red_safe + "\n");
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

//
// Generic Interact Events
//
function on_kill(cid, cdefId) {
	inst.broadcast("Kill: " + cid + " " + cdefId);
}

function on_use(sourceCId, targetCDefId) {
	if(targetCDefId == CDEFID_RED_TEAM_REGISTRAR) {
		if(check_team(sourceCId)) {
			inst.broadcast(sourceCId + " joins the red team");
			red_team.append(sourceCId);
		}
	}
	
	if(targetCDefId == CDEFID_BLUE_TEAM_REGISTRAR) {
		if(check_team(sourceCId)) {
			inst.broadcast(sourceCId + " joins the blue team");
			blue_team.append(sourceCId);
		}
	}
}

/*
 * InteractDef script functions. See InteractDef.txt
 */

function blue_flag_won(winner, flagBaseCDefID) {
	if(has_blue_flag != winner)
		inst.info("You don't have the blue flag!");
	else {
		red_score++;
		inst.restore_original_appearance(winner);
		inst.broadcast("Red team scores! They now have " + red_score + " flags");
		has_blue_flag = 0;
		cid_blue_safe = inst.spawn(PROP_BLUE_FLAG_SAFE, 0,0);
	}
}

function red_flag_won(winner, flagBaseCDefID) {
	if(has_red_flag != winner) {
		inst.info("You don't have the red flag!");
	}
	else {
		blue_score++;
		inst.restore_original_appearance(winner);
		inst.broadcast("Blue team scores! They now have " + blue_score + " flags");
		has_red_flag = 0;
		cid_red_safe = inst.spawn(PROP_RED_FLAG_SAFE, 0,0);		
	}
}


function red_flag_taken(taker, taken) {
	print("red flag taken\n");
	if(red_team.find(taker) >= 0) 
		inst.info("You can't take your own flag!");
	else if(blue_team.find(taker) >= 0) {
		inst.broadcast("Red flag taken by " + taker);
		has_red_flag = taker;
		despawn_red_flag();
		inst.attach_item(taker, "Item-CTF_Red", "ctf");
	}
	else {
		inst.info("You are not in a team!");
	}
}

function blue_flag_taken(taker, taken) {
	print("blue flag taken\n");
	if(blue_team.find(taker) >= 0) {
		inst.info("You can't take your own flag!");
	}
	else if(red_team.find(taker) >= 0) {
		inst.broadcast("Blue flag taken by " + taker);
		has_blue_flag = taker;
		despawn_blue_flag();
		inst.attach_item(taker, "Item-CTF_Blue", "ctf");
	}
	else 
		inst.info("You are not in a team!");
}