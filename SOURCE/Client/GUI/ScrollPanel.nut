this.require("GUI/ScrollButtons");
class this.GUI.ScrollPanel extends this.GUI.ScrollButtons
{
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setAppearance("Container");
		this.mButtonContainer = this.GUI.Component();
		this.mButtonContainer.setLayoutManager(this.GUI.BorderLayout());

		if (vargc < 2)
		{
			this.add(this.mButtonContainer, this.GUI.BorderLayout.EAST);
		}
		else
		{
			this.add(this.mButtonContainer, vargv[1]);
		}

		this.mButtonTop = this.GUI.Component();
		this.mButtonTop.setLayoutManager(this.GUI.BorderLayout());
		this.mButtonContainer.add(this.mButtonTop, this.GUI.BorderLayout.NORTH);
		this.mButtonBottom = this.GUI.Component();
		this.mButtonBottom.setLayoutManager(this.GUI.BorderLayout());
		this.mButtonContainer.add(this.mButtonBottom, this.GUI.BorderLayout.SOUTH);
		this.mPageUp = this.GUI.SmallButton("PageUp");
		this.mPageUp.setPressMessage("onPageUp");
		this.mPageUp.addActionListener(this);
		this.mButtonTop.add(this.mPageUp, this.GUI.BorderLayout.NORTH);
		this.mLineUp = this.GUI.SmallButton("LineUp");
		this.mLineUp.setPressMessage("onLineUp");
		this.mLineUp.addActionListener(this);
		this.mButtonTop.add(this.mLineUp, this.GUI.BorderLayout.SOUTH);
		this.mLineDown = this.GUI.SmallButton("LineDown");
		this.mLineDown.setPressMessage("onLineDown");
		this.mLineDown.addActionListener(this);
		this.mButtonBottom.add(this.mLineDown, this.GUI.BorderLayout.NORTH);
		this.mPageDown = this.GUI.SmallButton("PageDown");
		this.mPageDown.setPressMessage("onPageDown");
		this.mPageDown.addActionListener(this);
		this.mButtonBottom.add(this.mPageDown, this.GUI.BorderLayout.SOUTH);
		this.mButtonContainer.add(this.GUI.Spacer(10, 56), this.GUI.BorderLayout.CENTER);
		this.setGap(5);
		this.setIndent(10);
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			this.attach(vargv[0]);
		}
	}

	function attach( pAttachParent )
	{
		this.mAttachParent = pAttachParent;
		this.addActionListener(this.mAttachParent);
		this.mAttachParent.setScroll(this);
		this.add(pAttachParent, this.GUI.BorderLayout.CENTER);
	}

	function validate()
	{
		this.GUI.Component.validate();
	}

	function getPreferredSize()
	{
		return this.GUI.Component.getPreferredSize();
	}

	function setGap( pGap )
	{
		this.mButtonContainer.insets.left = pGap;
		this.mButtonContainer.insets.right = pGap;
	}

	function setIndent( pIndent )
	{
		this.mButtonContainer.insets.top = pIndent;
		this.mButtonContainer.insets.bottom = pIndent;
	}

	mButtonContainer = null;
	mButtonTop = null;
	mButtonBottom = null;
}

