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

function on_sidekick_death(cid) {
	if(array_contains(sidekick_cids, cid))  {
		quest.info("You failed the quest, return to Corporal Anderson to try again");
		quest.exec(function() { quest.abandon() });
	}
}

function on_leave() 
	despawn_sidekicks();

function on_player_death(cid) {
	quest.info("You failed the quest, return to Corporal Anderson to try again");
	quest.exec(function() { quest.abandon() });
}

function on_unregister() 
	despawn_sidekicks();

function on_halt() 
	despawn_sidekicks();

function despawn_sidekicks() {
	foreach(cid in sidekick_cids)
		quest.remove_sidekick(cid);
	sidekick_cids.clear();
}
	
if(quest.this_zone() == 1060) {
	foreach(cdefid in sidekick_cdefids) {
		local cid = quest.add_sidekick(cdefid, SIDEKICK_QUEST, 0, HATE_SIDEKICK_MORE);
		quest.queue(function() {
			quest.sidekicks_defend();
			quest.queue(function() {
				quest.say(cid, "I've got your back!");
				quest.emote_npc(cid, "Salute");
			}, 2000);
		}, 2000);
		sidekick_cids.push(cid);
	}
}
