::info <- {
	name = "Southend Passage",
	author = "Emerald Icemoon",
	description = "Handles books only",
	queue_events = true
}

function do_on_use(cid, book_item_id, book_page) {
	if(inst.has_item(cid, book_item_id))
		inst.message_to(cid, "You already have page " + book_page + " of the Southend Passage Parchment", INFOMSG_INFO);
	else {
		inst.interact(cid, "Taking the Southend Passage Parchment", 3000, false, function() {
			inst.message_to(cid, "Page " + book_page + " of the Southend Passage Parchment is now in your inventory.", INFOMSG_INFO);
			inst.give_item(cid, book_item_id);
			inst.open_book(cid, 2, book_page);
		});
		return true;
	}
}

function on_use_3489(cid, used_cid) do_on_use(cid, 1500001, 1);
function on_use_3490(cid, used_cid) do_on_use(cid, 1500002, 2);
function on_use_3491(cid, used_cid) do_on_use(cid, 1500003, 3);
