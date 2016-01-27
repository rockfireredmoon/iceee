#!/bin/sq

function start_ambushers_906(cid) {

	/* Get everyone who actually attacked */
	local attacked = inst.get_hated(cid);

	/* First send quest invitation to everyone who attacked */
	foreach(i in attacked) {
		inst.invite_quest(i, 906, false);
	}

	/* Allow a very short delay for the player to accept */
	inst.queue(function() {
	
		/* Then spawn the ambushers anyway :) */
		local cids = [ inst.spawn(1133310,0,0), inst.spawn(1133311,0,0), inst.spawn(1133309,0,0) ];	
	
		inst.info("CIDS: " + cids);
	
		/* Make each of the ambushers pick a random target from everyone who attacked */
		foreach(cid in cids) {
			inst.info("Set target " + cid);
			inst.set_target(cid, attacked[randmodrng(0, attacked.len())]);
			inst.info("Set AO " + cid);
			inst.ai(cid, "tryMelee");
		} 
		
	}, 100);

}

function on_death(cid, cdefid) {
	if(cdefid == 7876) {
		start_ambushers_906(cid);
	}
}


