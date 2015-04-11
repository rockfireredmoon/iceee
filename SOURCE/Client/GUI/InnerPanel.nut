this.require("GUI/Panel");
class this.GUI.InnerPanel extends this.GUI.Panel
{
	static mClassName = "InnerPanel";
	constructor( ... )
	{
		if (vargc > 1)
		{
			this.GUI.Panel.constructor(vargv[0], vargv[1]);
		}
		else if (vargc > 0)
		{
			this.GUI.Panel.constructor(vargv[0]);
		}
		else
		{
			this.GUI.Panel.constructor();
		}

		if (!(vargc > 0 && typeof vargv[0] == "string"))
		{
			this.setAppearance("InnerPanel");
		}

		this.setInsets(2);
	}

}

