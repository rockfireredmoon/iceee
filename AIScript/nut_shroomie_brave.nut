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
	print("Target acquired!\n");
	have_target();
}

function have_target() {	
	print("Have target!\n");	
	print("Using!\n");
	ai.use(RANGED_MELEE);	
	print("Getting might charge!\n");
	if(ai.get_might_charge() >= 2) {
		print("Using force blast!\n");
		ai.use(FORCE_BLAST);
		print("Sleeping 1000!\n");
		ai.sleep(1000);
	}
	print("Getting might!\n");
	if(ai.get_might() >= 3) {
		print("Using force bolt!\n");
		ai.use(FORCE_BOLT);
		print("Sleeping 2000!");
		ai.sleep(2000);
	}
	print("Checking for target!\n");
	if(ai.has_target()) {
		print("Requeuing in ZERO!\n");
		ai.queue(have_target, 0);
	}
	print("Leaving!\n");
}