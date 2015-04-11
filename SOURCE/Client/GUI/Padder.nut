this.require("GUI/Container");
class this.GUI.Padder extends this.GUI.Container
{
	static mClassName = "Padder";
	constructor( component, ... )
	{
		this.GUI.Container(this.GUI.BorderLayout());

		if (vargc == 1)
		{
			this.setInsets(vargv[0]);
		}
		else if (vargc == 4)
		{
			this.setInsets(vargv[0], vargv[1], vargv[2], vargv[3]);
		}

		this.add(component);
	}

}

