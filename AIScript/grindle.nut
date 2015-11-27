/*
 *Script: Physical= Auto Melee, Whiplash, Assault, Can Opener, Thor's Mighty Blow, Blender
 *        Magical= Wither, Deathly Dart, Damnation, Malice Blast, Frost mire
 *        Unique= Syphon All, Happy Holidays ( Holiday themed Aoe) Untouchable (spells and melee 15seconds)
 */

function on_target_acquired() {
	cast_buffs();
	ai.use(32760);
	main_actions();
	return;
}

function main_actions() {
	if(!ai.has_target()) {
		return;
	}
	if(!ai.is_busy()) {		
		check_timers();
		check_will_charge();
		if(!check_might_charge()) {
			check_will();
			check_might();
		}
	}
	ai.queue(main_actions, 100);
}
