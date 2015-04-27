// Constants, for things you might want to tune over the life of the
// script, but are generally static

const BOX_PROP_ID=1126558;
const LIGHT_PROP_ID=1126557;

// You need to declare any variables you might use

spawnCID <- 0;
steamParticle <-0;

/* After declaring constants and variables, declare all of the
 * functions a script might use. These are not executed 
 * immediately (unless explicitly called), but as the result of events
 */
 
//////////////////////////////////////////////////////

function queuedEvent() {	
	
	/* Functions called as the result of an 'event' can themselves
	 * fire more events. A common technique for this would be to
	 * continuously call a function at an interval. This example
	 * does nothing exception print an 'info' message every 10
	 * seconds
	 */
	inst.info("The script is still running (Spawn CID=" + spawnCID + " Effect=" + steamParticle + ")");
	core.queue("queuedEvent", 5000);
}

/*
 * The onFinish function is called when the script exits. Do NOT
 * queue any more events here, they will not be called. Use
 * this function to clear up and return the instance to it's 
 * previous state if applicable
 */
 function onFinish() {
	inst.broadcast("The script has finished!");
}

/*
 * The onKill function is called when a creature in this instance
 * dies. You will be supplied the creature instance ID (CID), and
 * the creature definition ID (if any)
 */
function onKill(cdefId, cid) {
	print("Squirrel Killed " + cdefId + "," + cid);
	
	// Send a broadcast message
	inst.broadcast("I killed something!");
	
	/*
	 * Change the light from red to green 
	 */	 
	inst.asset(LIGHT_PROP_ID, "Light-Intense?Group=1&COLOR=00FF00", 1);
	
	
	/*
	 * Spawn the box prop. The spawn function takes 3 arguments,the
	 * last 2 of which are optional. The first is the prop ID to
	 * spawn and is required. The 2nd argument can be used to force
	 * a different creature ID to that which is defined in the prop,
	 * and the final argument is the 'flag' which determines if
	 * the creature is friendly, neutral etc. 
	 *
	 * The function will return the unique 'CID' which can then be
	 * used in other functions to despawn or otherwise manipulate the
	 * spawned prop. 
	 */
	spawnCID = inst.spawn(BOX_PROP_ID, 0, 0);
	print("SPAWN CID: " + spawnCID);
	
	/*
	 * Attach a steam particle to the spawned box. You must provide the
	 * prop ID, the effect name, the scale of the effect, and an x,y
	 * and z offset of the effect.
	 */
	steamParticle = inst.particleAttach(BOX_PROP_ID,"Par-Steam",5,0,10,0);
	
	// Stop the steam in 20 seconds
	core.queue("stopSteam", 20000);
}

/*
 * Queued from onKill to remove the steam effect 
 */
function stopSteam() {
	inst.particleDetach(BOX_PROP_ID, steamParticle);
	
	/*
	 * This will remove all queued events and stop the script from
	 * executing any further.
	 */
	core.halt(); 
}

/* Any script declared outside of the functions is run once when the
 * script is loaded. Use this to set up any variables, or queue your
 * first event (if you script is continously doing something and not
 * just reacting to built in events such as onKill).
 */
core.queue("queuedEvent", 5000);
