::info <- {
	name = "Anglor Dren",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Anglor Dren's Chest",
	queue_events = true
}

killcount <- 0;

function on_kill(creatureDefID, creatureID) {
	if(creatureDefID == 1995) {
		// Anglor Dren's chest
		inst.spawn(1291845811,0,0);
	}
	else if(array_contains([1993, 1994],creatureDefID)) {
		killcount++;
		if(killcount >= 6) {
			inst.spawn(1291845744,0,0);
		}
	}
}

function on_use_3488(cid, used_cid) {
	if(inst.has_item(cid, 1500000))
		inst.message_to(cid, "You already have the Corsica Lighthouse Note", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the Corsica Lighthouse Note", 3000, false, function() {
			inst.message_to(cid, "The Corsica Lighthouse Note is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, 1500000);
			inst.open_book(cid, 1, 1);
		});
		return true;
	}
}
