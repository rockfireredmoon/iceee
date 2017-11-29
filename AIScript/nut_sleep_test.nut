function on_death() {
	print("I died :(");
	::inst.aiDied();
}

function on_target_acquired(cid) {
	have_target();
}

function have_target() {
	print("Target acquired!\n");
	print("Sleeping 10000!\n");
	
	/*
	 * Wait for 10 seconds. The condition here is for handling when the sleep is 'interrupted'.
  	 * This might be caused by by an external function call.
	 * These might come from instance scripts, or on death of this creature, or
	 * if targets change. A well behaved script should catch this and deal with
	 * it appropriately. It should never sleep again (this may be prevented in 
	 * future versions). 
	 */
	if(ai.sleep(10000)) {
		 print("Interrupt");
	 	 return;
	}
	
	print("Slept 10000!\n");
	
	if(ai.has_target()) {
		print("Still have target, requeue\n");
		ai.queue(have_target, 5000);
	}
	else 
		print("No longer have target\n");
}

function poll() {
	print("POLL!\n");
	ai.queue(poll, 1000);
}

print("RUNING!\n");
ai.queue(poll, 1000);
print("RUN!\n");