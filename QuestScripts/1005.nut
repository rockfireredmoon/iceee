/*
 * Quest Script 1005 - A Hopeless Cause
 *
 * Warps player to Corsica
 *
 */
 
::info <- {
	name = "1005",
	author = "Emerald Icemoon",
	description = "A Hopeless Cause"
}

function on_use_finish_7892() {
	quest.clear_queue();
	quest.info("COMPLETE!"); 
	quest.warp_zone(1060);
	
}