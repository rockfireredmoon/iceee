/*
 * Quest Script 826 - Havin' A Laugh
 *
 * Script wi
 */

function on_activated_1() {
	quest.info("Chewing gum for the eyes ..");
	local cid = quest.spawn(1128357);
	quest.queue(function() {
		quest.despawn(cid);		
	}, 20000);
}