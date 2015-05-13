paultrigger <- 0;

// Scripted call from the spawn package
function 4_Trailblazer_StepUp() {
	paultrigger++;
	if(paultrigger == 20) {
		inst.spawn(151026296,1098,0);
		paultrigger = 0;
	}
}

function on_kill_1098() {
	inst.spawn(151026296,1099,0);
}