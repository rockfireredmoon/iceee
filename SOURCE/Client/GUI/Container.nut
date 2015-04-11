this.require("GUI/Component");
this.require("GUI/FlowLayout");
class this.GUI.Container extends this.GUI.Component
{
	constructor( ... )
	{
		if (vargc > 0)
		{
			this.GUI.Component.constructor(vargv[0]);
		}
		else
		{
			this.GUI.Component.constructor(this.GUI.FlowLayout());
		}

		for( local i = 1; i < vargc; i++ )
		{
			if (typeof vargv[i] == "array")
			{
				foreach( c in vargv[i] )
				{
					this.add(c);
				}
			}
			else
			{
				this.add(vargv[i]);
			}
		}

		this.mAppearance = "Container";
	}

}

