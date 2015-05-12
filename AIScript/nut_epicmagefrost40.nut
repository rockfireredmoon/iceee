/*
 * A port of custom_heathendel_epicmagefrost40 to the Squirrel scripting system

 * 517 : Frostbolt 40  [Cooldown=BasicCharge2]
 * 523 : Cryo Blast 40
 * 149 : Frost Mire 50  [Cooldown=Daze]
 * 383 : Coldsnap 40  [Cooldown=ReduceMovementSpeed]
 * 259 : Forcebolt 40
 * 377 : Force Blast 40
 * 219 : Frost Storm 50  [Cooldown=DDAE1, Will=5, GTAE=150]
 * 399 : Theft of Will 50  [Cooldown=Theft, Will=2]
 * 356 : Frost Shield 40  [Cooldown=HealthShield, Will=5, Group=146]
 * 32760 : ranged autoattack

 * 5501 - Void Shield

 * Basic logic:
 * Use the timers to scan targets for AoEs on a less frequent basis so the server
 * is spending less time searching for targets.  If multiple targets are present,
 * reserve Will to let the mob charge up if they're under, since Frost Storm
 * has a higher cost than other skills.
 * If multi targets are not present, or waiting on cooldowns, use single target spells.
 * If Frost Shield is inactive, try to save up Will to cast at the earliest opportunity.
 */


const AOE_TRIGGER_COUNT = 2;

reserve_shield <- false;
reserve_will <- 0;
timestamp <- 0;
mire_targets <- 0;
storm_targets <- 0;
will <- 0;

function main_actions() {
	print("Main action\n");
	if(!ai.is_busy()) {
		if(!ai.has_target()) {
			print("Target lost");
			ai.queue(wait_for_target, 1000);
			return;
		}
		
		check_timers();
		check_will_charge();
		
		if(!check_might_charge()) {
			// If the charge wasn't used, continue
			check_will();
			check_might();
		}
	}
	ai.queue(main_actions, 100);
}

function check_will_charge() {
	print("Checking will charge\n");
	if(ai.get_will_charge() >= 3) 
		ai.use(523);
}

function check_might_charge() {
	print("Checking might charge\n");
	if(ai.get_might_charge() >= 3) {
		ai.use(377);
		return true;
	}
	return false;
}

function check_will() {
	print("Checking will\n");
	will = ai.get_will();
	if(will >= reserve_will) {
		if(reserve_shield) {
			cast_buffs();
		}
		else {
			if(try_theft_of_will()) {
				return;
			}
			if(try_frost_mire()) {
				return;
			}
			if(try_frost_storm()) {
				return;
			}
			if(try_frost_bolt()) {
				return;
			}
			try_cold_snap();
		}
	}
}

function try_theft_of_will() {
	print("Trying theft of will\n");
	if(ai.is_on_cooldown("Theft"))
		return false;
	if(ai.get_target_property("will_charges") < 2) 
		return false;
	ai.use(399);
	return true;
}

function try_frost_mire() {
	print("Trying frost mire\n");
	if(mire_targets < AOE_TRIGGER_COUNT || will < 3 || ai.is_on_cooldown("Daze") || ai.count_enemy_near(80) < AOE_TRIGGER_COUNT)
		return false;
	ai.use(149);
	return true;	
}

function try_frost_storm() {
	print("Trying frost storm\n");
	if(storm_targets < AOE_TRIGGER_COUNT || will < 5 || ai.is_on_cooldown("DDAE1") || ai.count_enemy_at(150) < AOE_TRIGGER_COUNT)
		return false;
	print("Setting GTAE\n");
	ai.set_gtae();
	ai.use(219);
	return true;
}

function try_frost_bolt() {
	print("Trying frost bolt\n");
	if(ai.is_on_cooldown("BasicCharge2")) 
		return false;
	ai.use(517);
	return true;
}

function try_cold_snap() {
	print("Trying cold snap\n");
	if(ai.is_on_cooldown("ReduceMovementSpeed")) 
		return false;
	ai.use(383);
	return true;
}

function try_cast_shield() {
	print("Trying cast shield\n");
	if(!ai.is_on_cooldown("HealthShield") && !ai.get_property("bonus_health") > 15000)
		ai.use(5501);
}

function check_might() {
	print("Checking might\n");
	if(ai.get_might() >= 3) 
		ai.use(259);
}

function check_timers() {
	print("Checking timers\n");
	if(ai.get_server_time() >= timestamp + 2000) {
		timestamp = ai.get_server_time();
		check_multi_targets();
	}
}

function check_multi_targets() {
	print("Checking multi targets\n");
	mire_targets = ai.count_enemy_near(80);
	storm_targets = ai.count_enemy_at(150);
	reserve_will = 3;
	if(storm_targets >= AOE_TRIGGER_COUNT)
		reserve_will = 5;
}

function cast_buffs() {
	print("Casting buffs\n");
	reserve_shield = false;
	if(ai.is_on_cooldown("HealthShield")) {
		reserve_will = 3;
		return;
	}
	
	if(ai.get_buff_tier(146) != 0)
		return;
		
	will = ai.get_will();
	if(will < 5) {
		reserve_will = 5;
		reserve_shield = true;
	}
}

function wait_for_target() {
	print("Waiting for target\n");
	if(ai.has_target()) {
		print("The fight starts!\n");
		cast_buffs();
		ai.use(32760);
		main_actions();
		return;
	}
	
	// No target yet
	ai.queue(wait_for_target, 1000);
}

// Start polling for target
ai.queue(wait_for_target, 0);