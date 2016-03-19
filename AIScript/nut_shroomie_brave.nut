/*
 * A squirrel script version of shroomie_brave.
 */
 
const RANGED_MELEE = 32760;
const MELEE = 32766;
const FORCE_BLAST = 5307;
const FORCE_BOLT = 5189;
 
info <- {
	name = "nut_shroomie_brave",
	enabled = true,
	author = "Emerald Icemoon",
	description = ""
}

function on_target_acquired(cid) {
	have_target();
}

function have_target() {	
	ai.use(RANGED_MELEE);
	if(ai.get_might_charge() >= 2) {
		ai.use(FORCE_BLAST);
		if(ai.sleep(1000))
			return;
	}
	if(ai.get_might() >= 3) {
		ai.use(FORCE_BOLT);
		if(ai.sleep(2000))
			return;
	}
	if(ai.has_target())
		ai.queue(have_target, 100);
}