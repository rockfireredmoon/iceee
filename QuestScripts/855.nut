/*
 * Example script for creating a sidekick for use during a quest
 */


info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "Sidekick Example Quest"
}

sidekick_cid <- 0;

function on_leave() 
	// If the player leaves the quest, despawn sidekick
	despawn_sidekick();

function on_player_death(cid) {
}

function on_unregister() {
	print("Unregister!\n");
	// If the player leaves the instance, a new script instance will be created
	despawn_sidekick();
}

function despawn_sidekick()
	sidekick_cid = quest.remove_sidekick(sidekick_cid);

function on_halt() {
	print("Halt!\n");
	// If the script halts for any other reason, despawn sideback
	despawn_sidekick();
}
	
// Spawn the sidekick when the quest is accepted and the player is in the zone intended for it
if(quest.this_zone() == 1060) {
	sidekick_cid = quest.add_sidekick(7850, SIDEKICK_QUEST, 0);
	print("Spawned! "+ sidekick_cid + "\n");
}
else {
	print("Not in right zone to spawn sidekick");
}