::info <- {
	name = "Fangarians Lair",
	author = "Emerald Icemoon",
	description = "Spawns Grimfrost boss chest and deal with fight environment",
	queue_events = true
}

const CDEF_GRIMFROST = 2756;

cid_grimfrost <- 0;
loc_grimfrost <- Area(1777, 1586, 2151, 2259);
fhc <- 0;
env <- "";

function find_grim() {
	cid_grimfrost = inst.scan_npc(loc_grimfrost, CDEF_GRIMFROST);
	if(cid_grimfrost == 0 || inst.has_status_effect(cid_grimfrost, "DEAD"))
		inst.queue(find_grim, 5000);
	else
		grim_health();
}

function tod(e) {
	if(e != env) {
		env = e;
		inst.set_timeofday(e);
	}
}

function grim_health() {
	if(cid_grimfrost == 0)
		return;

	local health = inst.get_health_pc(cid_grimfrost);

	if(health == -1) {
		inst.exec(find_grim);
	}
	else if(health == 100) {
		if(fhc == 4 && inst.is_at_tether(cid_grimfrost)) {
			tod("Day");
			fhc = 0;
		}
		else
			fhc++;
		inst.queue(grim_health, 5000);
	}
	else {
		tod("Sunset");
		fhc = 0;
		inst.exec(grim_health);
	}
}

function on_kill(cdefid, cid) {
	if(cdefid == CDEF_GRIMFROST) {
		tod("Night");
		inst.spawn(402653612, 0, 0);
		cid_grimfrost = 0;
		inst.exec(find_grim);
	}
}

tod("Day");
inst.exec(find_grim);