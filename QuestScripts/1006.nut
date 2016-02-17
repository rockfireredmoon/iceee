/*
 * First sidekick quest 'The Battle of North Beach'
 */

info <- {
	enabled = true,
	author = "Emerald Icemoon",
	description = "The Battle of North Beach"
}

/* A custom class that encapulates details about each survivor */

class Survivor {
	mPropId = 0;
	mPhrase = "";
	mDestination = null;
	
	constructor(propId, phrase, destination) {
		mPropId = propId;
		mPhrase = phrase;
		mDestination = destination;
	}
}

/* An array of survivors */
survivors <- [ 
	Survivor(1133411,"I .. I had made a strategic retreat!", Point(15483,14164.6)), 
	Survivor(1133412,"Thanks for clearing a path, I owe you!", Point(15359.3,13442.6)), 
	Survivor(1133413,"It's hell out there, but intense in here too!", Point(15623,13606.7)),
];


/* Side info */
sidekick_cdefids <- [ 7851, 7852 ];
sidekick_cids <- [];

function on_use_finish_7893(cid) {
	/* A survivor was rescued */

	/* Find th prop ID for this CID, that will determine where they run to */
	foreach(s in survivors) {
		if(inst.get_cid_for_prop(s.mPropId) == cid) {
			// Got it!
			quest.say(cid, s.mPhrase);
			quest.queue(function() {
				inst.walk_then(cid, s.mDestination, 50, 0, function() {
					inst.despawn(cid);
				});
			}, 3000);
			
			// Done
			break;
			
		}
	}
	
}

function on_sidekick_death(cid) {

	/* A sidekick died, just abandon the quest */

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
