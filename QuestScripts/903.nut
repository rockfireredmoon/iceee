/*
 * Quest Script 903 - Destroy The Catapults
 *
 * Animates the catapults
 *
 */
 
::info <- {
	name = "903",
	author = "Emerald Icemoon",
	description = "Destroy The Catapults"
}

function on_activated_0() {
	local target = quest.get_target();
	
	/* Run death animation */
	quest.emote_npc(target, "Death");
	
	/* Make it not usable any more */
	inst.remove_status_effect(target, "IS_USABLE");
	inst.remove_status_effect(target, "USABLE_BY_SCRIPT");
	
	/* Effect animation sequence and despawn */
	quest.queue(function() { 
		quest.effect_npc(target, "PyroblastHit");
		quest.queue(function() {
			quest.effect_npc(target, "HellfireHit");
			
			/* Despawn in 2 seconds */
			quest.queue(function() {
				quest.despawn(target);
			}, 2000);
		}, 500);
	}, 200);
}
