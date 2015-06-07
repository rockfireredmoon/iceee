::info <- {
	name = "Fangarians Lair",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Spawns Grimfrost boss chest",
	queue_events = true
}
 
function on_kill_2756()
	inst.spawn(402653612, 0, 0);
