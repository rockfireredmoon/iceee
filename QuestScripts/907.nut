/*
 * Quest Script 907 - A Moral Choice
 *
 */
 
::info <- {
	name = "907",
	author = "Emerald Icemoon",
	description = "A Moral Choice",
	queue_events = false
}

function on_objective_complete_0_0() {
	
	/* Called when player rescues the npc */
	local cid = quest.get_target();
	
	/* Emote the NPC */
	quest.emote_npc(cid, "Excited");
	
	/* Find the Chest */
	local inst = quest.get_instance();
	foreach(cid in inst.cids(7879)) {
		quest.info("The loot chest burns in the fire! Never mind though, you rescued the  innocent beast,");

		// Send 5 separate FireboltHit effects, then despawn the chest		
		for(local a = 0 ; a < 5000 ; a += 1000) {
			quest.queue(function() {
				quest.effect_npc(cid, "FireboltHit");
			}, a);
		}
		quest.queue(function() {
			quest.despawn(cid);
		}, 5000);
	}
	
	/* 2 seconds later, have the emote speak his thanks */
	quest.queue(function() {
		quest.say(cid, "Thank you so much! I am forever in your debt.");
		
		/* 3 seconds later, despawn him */
		quest.queue(function() {
			quest.despawn(cid);
		}, 3000);
		
	}, 2000);
}

function on_objective_complete_0_1() {
	
	/* Called when player loots the chest instead */
	local cid = quest.get_target();
		
	/* Find the NPC */
	local inst = quest.get_instance();
	foreach(cid in inst.cids(7878)) {
	
		/* Emote the NPC and plead with player, but it's too late */
		quest.emote_npc(cid, "Beg");
		quest.say(cid, "What .. what are you doing! Help me ... argghh");
		quest.queue(function() {	
			quest.info("The beast burns in the fire! At least you got your loot though, huh?");
	
			/* Send 5 separate FireboltHit effects, then despawn the player */
					
			for(local a = 0 ; a < 5000 ; a += 1000) {
				quest.queue(function() {
					quest.effect_npc(cid, "FireboltHit");
				}, a);
			}
			
			quest.queue(function() {
				quest.despawn(cid);
			}, 5000);
		}, 2000);
	}
	
	/* Just despawn the chest */
	quest.despawn(cid);
}