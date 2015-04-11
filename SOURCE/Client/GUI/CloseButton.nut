this.require("GUI/Button");
class this.GUI.CloseButton extends this.GUI.Button
{
	constructor( ... )
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
		this.setAppearance("CloseButton");
		this.mUseMouseOverEffect = true;
		this.mUseOffsetEffect = true;
		this.setFixedSize(18, 18);
		this.setSelection(true);
	}

	static mClassName = "CloseButton";
}

