// 32766 - melee

function waitForTarget() {
	/*if(ai.has_target()) {
		print("Have target!\n");
		ai.use(32766);
	}
	else 
		print("No target!\n");
	print("Tick\n");
	ai.queue(waitForTarget, 2000);
		*/
		
	print("Tick " + ai.has_target() + "\n");
	ai.queue(waitForTarget, 2000);
}
//ai.queue(waitForTarget, 2000);
print("In the AI script\n");
ai.queue(waitForTarget, 2000);