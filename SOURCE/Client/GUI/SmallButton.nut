this.require("GUI/Button");
class this.GUI.SmallButton extends this.GUI.Button
{
	constructor( pIcon, ... )
	{
		if (vargc > 0)
		{
			this.GUI.Button.constructor("", vargv[0]);
		}
		else
		{
			this.GUI.Button.constructor("");
		}

		this.remove(this.mLabel);
		this.setAppearance("SmallButton");
		this.setMaterial(this.mAppearance + "/" + pIcon);
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setFixedSize(25, 23);
		this.setSelection(true);
	}

	static mClassName = "SmallButton";
}

