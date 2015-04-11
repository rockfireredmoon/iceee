this.require("GUI/Button");
class this.GUI.BuyCreditButton extends this.GUI.Button
{
	constructor( pLabel, ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor(pLabel, vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor(pLabel);
		}

		this.setAppearance("BuyCreditButton");
		this.setFixedSize(114, 16);
		this.mUseMouseOverEffect = true;
		this.setSelection(true);
	}

	static mClassName = "BuyCreditButton";
}

