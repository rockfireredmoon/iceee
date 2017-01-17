/*
 * A port of custom_heathendel_deathdruid40 to the Squirrel scripting system
 */
 
const MALICE_BLAST = 373; 		//  40 
const CREEPING_DEATH = 397; 	//  [Cooldown=CreepingDeath]
const DEADLY_SHOT = 305;			//  40 
const ROOT = 101;				//	6  [Cooldown=ReduceMovementSpeed, Will=5]
const DEATHLY_DART = 533;		// 	Deathly Dart 40  [Cooldown=DeathlyDart] 
const SYPHON = 217;				// 	Syphon 40  [Cooldown=HealthLeech, Will=3]
const NEFRITARIS_AURA = 509;	// 	Nefritari's Aura 40  [Cooldown=DoT, PBAE=150, Will=3]
const WITHER = 492;				//	Wither 40	
const STING = 252;				//	Sting 40
const MELEE = 32766;			//	Melee Auto-attack	
const SPIRIT_OF_SOLOMON = 5045;	//	Spirit of Solomon (15%)  [Group=37] 

info <- {
	name = "custom_heathendel_deathdruid40",
	enabled = true,
	author = "Heathendel",
	description = "High Level Death Druid AI"
	speed = 6
}

allow_root <- false;
last_target <- -1;

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	if(last_target != target_cid) 
		allow_root = true;
	ai.use(MELEE);
	ai.exec(function() {
		ai.use(SPIRIT_OF_SOLOMON);
	});
	ai.exec(main);
}

function main() {
	if(ai.get_will_charge() >= 3) {
		if(!ai.is_on_cooldown("CreepingDeath"))
			ai.use(CREEPING_DEATH);
		else
			ai.use(MALICE_BLAST);
	}
	else if(ai.get_might_charge() >= 3) {
		ai.use(DEADLY_SHOT);
	}
	else if(ai.get_will() >= 3) {
		if(allow_root) {
			if(ai.get_will() >= 5 && !ai.is_on_cooldown("ReduceMovementSpeed") && ai.get_speed(ai.get_self()) == 0) {
				ai.use(ROOT);
				ai.exec(main);
				return;
			}
			else
				allow_root = false;
		}
		
		
		if(!ai.is_on_cooldown("DeathlyDart")) {
			ai.use(DEATHLY_DART);
		}
		else if(!ai.is_on_cooldown("HealthLeech") && ai.health_pc() < 95) {
			ai.use(SYPHON);
		}
		else if(!ai.is_on_cooldown("DoT") && ai.count_enemy_near(150) > 0) {
			ai.use(NEFRITARIS_AURA);
		}
		else 
			ai.use(WITHER);
		
	}
	else if(ai.get_might() >= 3) {
		ai.use(STING);
	}
	
	ai.exec(main);
}
