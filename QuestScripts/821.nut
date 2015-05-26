/*
 * Quest script for creating the Behemoth.
 *
 * A bit of a strange one this. If this player dies at the Behemoths hand,
 * then there is no way to re-create him. He also has a despawn time which
 * complicates things further.
 *
 * To combat this, the spawn will be removed if the player dies, and their
 * quest reset.
 */

behemoth_cid <- 0;

function on_activate_0() {
	quest.info("The altar starts to shake");
	quest.sleep(2000);
	quest.info("Your creations' stitched limbs start to twitch");
}

function on_leave() {
	// If the player leaves the quest, despawn behemoth
	despawn_behemoth();
}

function on_player_death(cid) {
	// If the quest player dies, reset the quest
	if(cid == quest.get_source()) {
		quest.info("You failed, maybe you should not mess with that which you cannot control hmm?");
		reset_act();
	}
}

function despawn_behemoth() {
	if(behemoth_cid > 0) {
		quest.despawn(behemoth_cid);
		behemoth_cid = 0;
	}
}

function reset_act() {
	despawn_behemoth();
	quest.reset_objective(0);
}

function on_halt() {
	// If the script halts for any other reason, despawn behemoth
	reset_act();
}

function on_use_finish_1241() {
	quest.info("It's alive!");
	behemoth_cid = quest.spawn(151028053);
	quest.sleep(2000);
	quest.info("Uh oh ...");
	
	// Force a despawn in 5 minutes anyway
	// TODO only despawn if not attacked by party?
	quest.queue(reset_act, 300000);
}