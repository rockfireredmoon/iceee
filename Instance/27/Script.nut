::info <- {
	name = "Coldstorm Cavern",
	author = "Grethnefar, ported by Emerald Icemoon",
	description = "Handles step-up spawns for Zimbaut and Cyclonas Chest",
	queue_events = true
}

zimbaut_STEPUP <- 0;

function on_kill(cdefid, cid) {
	if(cdefid >= 97 && cdefid <= 102) {
		zimbaut_STEPUP++;
		if(zimbaut_STEPUP >= 7) {
			inst.spawn(1029374, 0, 0);
		}
	}
	else if(cdefid == 1841) {
		// Cyclona's boss chest
		inst.spawn(452985536, 0, 0);
	} 
}
