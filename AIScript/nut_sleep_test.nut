function on_death() {
	print("I died :(");
}

function on_target_acquired(cid) {
	have_target();
}

function have_target() {
	print("Target acquired!\n");
	print("Sleeping 10000!\n");
	ai.sleep(10000);
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