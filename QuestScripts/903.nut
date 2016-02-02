/*
 * Quest Script 903 - Destroy The Catapults
 *
 * Animates the catapults
 *
 */
 
::info <- {
	name = "903",
	author = "Emerald Icemoon",
	description = "Destroy The Catapults"
}

function on_activated_0() {
	quest.emote_npc(quest.get_target(), "Death");
}