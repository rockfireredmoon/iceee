/*
 * Quest Script 826 - Havin' A Laugh
 *
 */
 
::info <- {
	name = "826",
	author = "Emerald Icemoon",
	description = "Havin' A Laugh",
	queue_events = false
}

function on_activated_0() {
	quest.info("Chewing gum for the eyes ..");
	local cid = quest.spawn(1128357);
	quest.queue(function() {
		quest.despawn(cid);		
	}, 20000);
}