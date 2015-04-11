this.require("GUI/LayoutManager");
class this.GUI.CardLayout extends this.GUI.LayoutManager
{
	function preferredLayoutSize( container )
	{
		local c;
		local maxW = 0;
		local maxH = 0;

		foreach( c in container.components )
		{
			if (c.isLayoutManaged())
			{
				local sz = c.getPreferredSize();
				maxW = this.Math.max(maxW, sz.width);
				maxH = this.Math.max(maxH, sz.height);
			}
		}

		return container._addInsets({
			width = maxW,
			height = maxH
		});
	}

	function layoutContainer( container )
	{
		local c;

		foreach( c in container.components )
		{
			if (this.getConstraint(container, c) == this.mCard)
			{
				c._setHidden(false);
				c.setPosition(container.insets.left, container.insets.top);
				local sz = container.getSize();
				sz.width -= container.insets.right + container.insets.left;
				sz.height -= container.insets.top + container.insets.bottom;
				c.setSize(sz);
				c.validate();
			}
			else
			{
				c._setHidden(true);
			}
		}
	}

	function show( card, container )
	{
		this.mCard = card;
		this.layoutContainer(container);
	}

	mCard = null;
	static mClassName = "CardLayout";
}

