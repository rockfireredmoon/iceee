this.require("GUI/Button");
class this.GUI.BuyMoreBagsButton extends this.GUI.Button
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

		this.setAppearance("BuyMoreBagsButton");
		this.setFixedSize(256, 32);
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = false;
		this.setSelection(true);
	}

	static mClassName = "BuyMoreBagsButton";
}

