/*
 * All scripts should define an 'info' variable, describing itself.
 */
info <- {
	name = "new_example",
	enabled = true,
	author = "Emerald Icemoon",
	description = "A simple melee example script. As soon as target is acquired and aggro gained, the creature will attack using melee."
}


function waitForTarget() {
	if(ai.has_target()) {
		print("Have target!\n");
		ai.use(32766);
	}
	else 
		print("No target!\n");
	ai.queue(waitForTarget, 2000);
}
ai.queue(waitForTarget, 2000);
