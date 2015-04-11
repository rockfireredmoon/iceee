this.require("GUI/Component");
class this.GUI.Panel extends this.GUI.Component
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.FlowLayout());
		this.mAppearance = "Panel";
		this.setInsets(5);

		if (vargc > 0)
		{
			if (vargv[0] == null)
			{
				this.setLayoutManager(null);
			}
			else if (typeof vargv[0] == "instance" && (vargv[0] instanceof this.GUI.LayoutManager))
			{
				this.setLayoutManager(vargv[0]);
			}
			else if (typeof vargv[0] == "string")
			{
				this.mAppearance = vargv[0];
			}
		}

		if (vargc > 1)
		{
			this.mPanelDraggable = vargv[1];
		}
	}

	static mClassName = "Panel";
}

