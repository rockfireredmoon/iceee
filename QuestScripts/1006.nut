/*
 * First sidekick quest 'The Battle of North Beach'
 */

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "The Battle of North Beach"
}

sidekick_cdefids <- [ 7851, 7852 ];
sidekick_cids <- [];

function on_leave() 
	despawn_sidekicks();

function on_player_death(cid) {
}

function on_unregister() {
	despawn_sidekicks();
}

function on_halt() {
	despawn_sidekicks();
}

function despawn_sidekicks() {
	foreach(cid in sidekick_cids) {
		quest.remove_sidekick(cid);
	}
	sidekick_cids.clear();
}
	
if(quest.this_zone() == 1060) {
	foreach(cdefid in sidekick_cdefids) {
		local cid = quest.add_sidekick(cdefid, SIDEKICK_QUEST, 0, HATE_SIDEKICK_MORE); 
		quest.queue(function() {
			quest.say(cid, "Ready for orders!");
			quest.emote_npc(cid, "Salute");
		}, 3000);
		sidekick_cids.push(cid);
	}
}
