/*
 * Valkal main AI (phase 2)
 *
 */
 
/*
 * For both phases
 */
const MELEE = 32766;			//	Melee Auto-attack

/*
 * For throne room phase
 */
const MALICE_BLAST = 494; 		//  50
const DEATHLY_DART = 534;		// 	Deathly Dart 50  [Cooldown=DeathlyDart]
const WITHER = 493;				//	Wither 50	
const SPIRIT_OF_SOLOMON = 487;	//	Spirit of Solomon (15%)  [Group=37]
const CAN_OPENER = 464;			//  Can Opener 50
const WHIPLASH = 213;			//  Whiplash (40% 40)
const SPELLBREAKER = 347;		//  Spellbreaker (40)
const THORS_MIGHTY_BLOW = 344;	//  Thors Mighty Blow (50)

info <- {
	name = "valkal1",
	enabled = true,
	author = "Heathendel",
	description = "Valkals primary AI"
	speed = 10
}

function on_target_lost(target_cid)
	ai.clear_queue();

function on_target_acquired(target_cid) {
	if(!ai.is_on_cooldown("DeathlyDart"))
		ai.use(DEATHLY_DART);
	ai.use(MELEE);
	ai.exec(function() {
		ai.use(SPIRIT_OF_SOLOMON);
	});
	ai.exec(main);
}

function main() {
	if(ai.get_will_charge() >= 3 && !ai.is_on_cooldown("BasicExecute")) {
		ai.use(MALICE_BLAST);
	}
	else if(ai.get_might_charge() >= 3 && !ai.is_on_cooldown("ThorsMightyBlow")) {
		ai.use(THORS_MIGHTY_BLOW);
	}
	else if(ai.get_will() >= 3) {
		if(!ai.is_on_cooldown("DeathlyDart"))
			ai.use(DEATHLY_DART);
		else if(!ai.is_on_cooldown("BasicCharge"))
			ai.use(WITHER);
	}
	else if(ai.get_might() >= 3) {
		if(randmod(10) > 7) {
			if(!ai.is_on_cooldown("Spellbreaker"))
				ai.use(SPELLBREAKER);
			else
				ai.use(WHIPLASH);
		}
		else
			ai.use(CAN_OPENER);
	}
	else {
		ai.use(MELEE);
	}
	ai.exec(main);
}
