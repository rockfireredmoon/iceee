class this.GUI.CCProgressBar extends this.GUI.Component
{
	static mClassName = "CCProgressBar";
	static ProgressBarPosition = [
		18,
		93,
		168,
		243
	];
	static PROGRESS_BAR_Y = 8;
	static PROGRESS_BAR_BG = "CC_Progress_Bar";
	static PROGRESS_BAR_BG_WIDTH = 331;
	static PROGRESS_BAR_BG_HEIGHT = 31;
	static PROGRESS_BAR_FILL_IMAGE = "CC_Progress_Bar_Fill";
	static PROGRESS_BAR_FILL_WIDTH = 72;
	static PROGRESS_BAR_FILL_HEIGHT = 16;
	static NUM_PROGRESS_FILLS = 4;
	mProgressBarFill = null;
	constructor()
	{
		this.mProgressBarFill = [];
		this.GUI.Component.constructor(this.GUI.FlowLayout());
		local progressBar = this.GUI.Container(null);
		progressBar.setAppearance(this.PROGRESS_BAR_BG);
		progressBar.setSize(this.PROGRESS_BAR_BG_WIDTH, this.PROGRESS_BAR_BG_HEIGHT);
		progressBar.setPreferredSize(this.PROGRESS_BAR_BG_WIDTH, this.PROGRESS_BAR_BG_HEIGHT);
		progressBar.setPosition(0, 0);
		this.add(progressBar);

		for( local i = 0; i < this.NUM_PROGRESS_FILLS; i++ )
		{
			local progressBarFill = this.GUI.Container(this.GUI.BoxLayoutV());
			progressBarFill.setSize(this.PROGRESS_BAR_FILL_WIDTH, this.PROGRESS_BAR_FILL_HEIGHT);
			progressBarFill.setPreferredSize(this.PROGRESS_BAR_FILL_WIDTH, this.PROGRESS_BAR_FILL_HEIGHT);
			progressBarFill.setPosition(this.ProgressBarPosition[i], this.PROGRESS_BAR_Y);
			progressBarFill.setAppearance(this.PROGRESS_BAR_FILL_IMAGE);
			local stepCount = i + 1;
			local progressBarLabel = this.GUI.Label("Step " + stepCount.tostring());
			progressBarLabel.setFont(::GUI.Font("MaiandraOutline", 18));
			progressBarLabel.setFontColor(this.Colors.white);
			progressBarFill.add(progressBarLabel);
			this.mProgressBarFill.append(progressBarFill);
			progressBar.add(progressBarFill);
		}
	}

	function fillProgressBar( number )
	{
		if (number <= 0 && this.mProgressBarFill.len() < this.NUM_PROGRESS_FILLS)
		{
			return;
		}

		if (number > this.NUM_PROGRESS_FILLS)
		{
			number = this.NUM_PROGRESS_FILLS;
		}

		this.clearProgressBar();

		for( local i = 0; i < number; i++ )
		{
			this.mProgressBarFill[i].setVisible(true);
		}
	}

	function clearProgressBar()
	{
		foreach( i, progressBarComp in this.mProgressBarFill )
		{
			progressBarComp.setVisible(false);
		}
	}

}

