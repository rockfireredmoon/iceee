this.require("GUI/LayoutManager");
class this.GUI.ListLayoutManager extends this.GUI.LayoutManager
{
	constructor( ... )
	{
		this.mPadding = vargc > 0 ? vargv[0] : 0;
	}

	function preferredLayoutSize( container )
	{
		return container._addInsets(this.layoutContainer(container, true));
	}

	function layoutContainer( container, ... )
	{
		local offsets = container.insets.left + container.insets.right;
		local size = container.getSize();
		local y = 0;
		local sim = vargc > 0 ? vargv[0] : false;

		foreach( c in container.components )
		{
			local prefsize = c.getPreferredSize();

			if (c.isLayoutManaged())
			{
				if (y + prefsize.height > size.height)
				{
					c.setVisible(false);
					continue;
				}

				if (sim == false)
				{
					c.setPosition(container.insets.left, container.insets.top + y);
					c.setSize(size.width - offsets, prefsize.height);
					c.setVisible(true);
				}

				y += prefsize.height + this.mPadding;
			}
		}

		if (y > 0)
		{
			y -= this.mPadding;
		}

		return {
			width = size.width,
			height = y
		};
	}

	mPadding = 0;
}

