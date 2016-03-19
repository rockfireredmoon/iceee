/*
 * A port of druid_death_brave_6 to the Squirrel scripting system
 *
 * 5045 - Spirit of Solomon, 5 will, buff
 * 5201 - Trauma, 3 might, add 1 mcharge
 * 5204 - Wither, 4 will, add wcharge
 * 5303 - Malice Blast, 1 will, need 1-3 wcharge
 * 32760 - ranged melee
 * 32766 - melee
 * 5234 - Deadly Shot, 1 might, need 1-3 mcharge  (not found in ability scan)
 */

info <- {
	name = "druid_death_brave_6",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Simple Death Druid AI"
}

function on_target_lost(targetCID) 
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(5045);
	ai.exec(main);
}

function main() {
	ai.use(32760);
	if(ai.get_will_charge() >= 2) {
		ai.use(5303);
		if(ai.sleep(1000)) 
			return;
	}
	if(ai.get_will() >= 4) {
		ai.use(5204);
		if(ai.sleep(2000))
			return;
	}
	if(ai.get_might_charge() >= 2) {
		ai.use(5234);
		if(ai.sleep(1000)) 
			return;
	}
	if(ai.get_might() >= 3) {
		ai.use(5201);
		if(ai.sleep(2000))
			return;
	}
	if(ai.has_target())
		ai.exec(main);
}