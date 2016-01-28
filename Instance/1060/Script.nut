#!/bin/sq

function on_death(cid, cdefid) {
	if(cdefid == 7876) {
	
		/* Get everyone who actually attacked */
		
		local attacked = inst.get_hated(cid);
	
		/* First send quest invitation to everyone who attacked */
		foreach(i in attacked) {
			inst.join_quest(i, 906, false);
		}
	
		/* Then spawn the ambushers anyway :) */
		local cids = [ inst.spawn(1133310,0,0), inst.spawn(1133311,0,0), inst.spawn(1133309,0,0) ];	
	
		/* Make each of the ambushers pick a random target from everyone who attacked */
		foreach(cid in cids) {
			inst.set_target(cid, attacked[randmodrng(0, attacked.len())]);
			inst.ai(cid, "tryMelee");
		} 

	}
}


