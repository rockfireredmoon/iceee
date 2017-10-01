::info <- {
	name = "Southend Passage",
	author = "Emerald Icemoon",
	description = "Handles books only",
	queue_events = true
}

function on_use_3489(cid, used_cid) {
	if(inst.has_item(cid, 1500001))
		inst.message_to(cid, "You already have page 1 of the Southend Passage Parchment", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the Southend Passage Parchment", 3000, false, function() {
			inst.message_to(cid, "Page 1 of the Southend Passage Parchment is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, 1500001);
			inst.open_book(cid, 2, 1);
		});
		return true;
	}
}

function on_use_3490(cid, used_cid) {
	if(inst.has_item(cid, 1500002))
		inst.message_to(cid, "You already have page 2 of the Southend Passage Parchment", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the Southend Passage Parchment", 3000, false, function() {
			inst.message_to(cid, "Page 2 of the Southend Passage Parchment is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, 1500002);
			inst.open_book(cid, 2, 2);
		});
		return true;
	}
}

function on_use_3491(cid, used_cid) {
	if(inst.has_item(cid, 1500003))
		inst.message_to(cid, "You already have page 3 of the Southend Passage Parchment", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the Southend Passage Parchment", 3000, false, function() {
			inst.message_to(cid, "Page 3 of the Southend Passage Parchment is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, 1500003);
			inst.open_book(cid, 2, 3);
		});
		return true;
	}
}
