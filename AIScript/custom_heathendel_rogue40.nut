/*
 * A port of custom_heathendel_rogue40 to the Squirrel scripting system
 *
 * 236 		: Assail 40
 * 336 		: Hemorrhage 40  [Cooldown=Hemorrhage]
 * 96 		: Feline's Grace  [Group=34]
 * 5036 	: Feline's Grace (15% bonus)  [Group=34]
 * 282 		: Pierce 40  [Cooldown=Pierce]
 * 296 		: Disembowel 40
 * 32766 	: Melee autoattack
 */

info <- {
	name = "custom_heathendel_rogue40",
	enabled = true,
	author = "Heathendel",
	description = "High Level Rogue AI"
	speed = 6
}

function cast_buffs() {
	if(ai.get_buff_tier(34) == 0) {
		ai.use(5036);
	}
}

function on_target_lost(targetCID)
	ai.clear_queue();

function on_target_acquired(targetCID) {
	cast_buffs();
	ai.use(32766);
	main();
}

function main() {
	if(ai.get_might_charge() >= 3) {
		if(!ai.is_on_cooldown("Hemorrhage")) {
			ai.use(336);
			ai.exec(main);
			return;
		}
		ai.use(296);
	}
	
	if(ai.get_might() >= 3) {
		if(!ai.is_on_cooldown("Pierce")) {
			ai.use(282);
			ai.exec(main);
			return;
		}
		ai.use(236);
	}
	
	ai.use(32766);
	ai.exec(main);
}

