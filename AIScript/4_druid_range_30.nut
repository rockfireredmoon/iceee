/*
 * A port of 4_druid_range_30 to the Squirrel scripting system
 *
 * 5044 - Root, 6 will
 * 5045 - Spirit of Solomon, 5 will, buff
 * 5198 - Soul Needles, 3 will, add 1 wcharge
 * 5299 - Soul Burst, 1 will, need 1-3 wcharge
 * 32760 - ranged melee
 * 32766 - melee
 */

info <- {
	name = "4_druid_range_30",
	enabled = true,
	author = "Emerald Icemoon",
	description = "Simple druid ranged AI"
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	ai.use(5045);
	if(ai.get_level() > 10) {
		ai.use(32760);
	}
	main();
}

function main() {
	ai.use(32766);
	if(ai.get_will_charge() >= 3) {
		ai.use(5299);
		if(ai.sleep(1000))
			return;
	}
	if(ai.get_will() >= 10) {
		ai.use(5044);
		if(ai.sleep(2000))
			return;
	}
	if(ai.get_will() >= 3) {
		ai.use(5198);
		if(ai.sleep(2000))
			return;
	}
	
	if(ai.has_target())
		ai.exec(main);
}

