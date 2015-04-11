this.require("GUI/GUI");
class this.GUI.ZOffset extends this.GUI.Component
{
	constructor( depth, ... )
	{
		this.GUI.Component.constructor();
		this.mAppearance = null;
		this.setMaterial(null);
		this.setLayoutExclude(true);

		if (depth < 1)
		{
			throw this.Exception("ZOffset must be created with depth of one or greater");
			return;
		}

		this.mDepth = depth;
		this.mDeepest = this;

		if (this.mDepth > 1)
		{
			local e = this.GUI.ZOffset(this.mDepth - 1);
			e.setPassThru(true);
			this.GUI.Component.add(e);
		}

		local d;
		local tzo = this;

		for( d = this.mDepth - 1; d > 0; d-- )
		{
			tzo = tzo.components[0];
			tzo.setLayoutExclude(true);
			this.mDeepest = tzo;
		}

		this.setSize(this.getSize());

		if (vargc > 0 && vargv[0] != null)
		{
			this.mDeepest.add(vargv[0]);
		}
	}

	function setSize( ... )
	{
		if (vargc == 1 && (typeof vargv[0] == "table" || typeof vargv[0] == "instance"))
		{
			this.GUI.Component.setSize(vargv[0]);

			if (this.components.len() > 0)
			{
				this.components[0].setSize(vargv[0]);
			}
		}
		else if (vargc == 2)
		{
			this.GUI.Component.setSize(vargv[0], vargv[1]);

			if (this.components.len() > 0)
			{
				this.components[0].setSize(vargv[0], vargv[1]);
			}
		}
		else
		{
			throw this.Exception("Invalid arguments to ZOffset.setSize()");
		}
	}

	function getPreferredSize()
	{
		if (this.mDeepest.components.len() > 0)
		{
			return this.mDeepest.components[0].getPreferredSize();
		}
		else
		{
			return this.getMinimumSize();
		}
	}

	function getDeepest()
	{
		return this.mDeepest;
	}

	mDepth = 0;
	mDeepest = null;
	static mClassName = "ZOffset";
}

