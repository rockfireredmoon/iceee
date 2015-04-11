this.require("GUI/ScrollButtons");
class this.GUI.VerticalScrollPageComponent extends this.GUI.ScrollButtons
{
	mButtonContainer = null;
	mBottomComponent = null;
	mNextComp = null;
	mPreviousComp = null;
	mPreviousLabel = null;
	mNextLabel = null;
	mPageLabel = null;
	mAttachParent = null;
	mAttachWindowHeight = 0;
	mIndex = 0;
	mClassName = "VerticalScrollPageComponent";
	static C_BUTTON_HEIGHT = 23;
	constructor( ... )
	{
		this.GUI.Component.constructor();
		this.setLayoutManager(this.GUI.BorderLayout());
		this.setAppearance("Container");
		this.mButtonContainer = this.GUI.Component();
		this.mButtonContainer.setLayoutManager(this.GUI.BorderLayout());

		if (vargc < 2)
		{
			this.add(this.mButtonContainer, this.GUI.BorderLayout.SOUTH);
		}
		else
		{
			this.add(this.mButtonContainer, vargv[1]);
		}

		this.mBottomComponent = this._buildButtonComponent();
		this.mButtonContainer.add(this.mBottomComponent, this.GUI.BorderLayout.SOUTH);
		this.setGap(10);
		this.setIndent(5);
		this.mMessageBroadcaster = this.MessageBroadcaster();

		if (vargc > 0)
		{
			this.attach(vargv[0]);
		}
	}

	function refreshToFirstPage()
	{
		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();
		}

		this.setIndex(0);
		this.validate();
		this._updateButton();
	}

	function _buildButtonComponent()
	{
		local previousText = "Previous";
		local nextText = "Next";
		local bottomComponent = this.GUI.Component(this.GUI.GridLayout(1, 3));
		bottomComponent.getLayoutManager().setRows(this.C_BUTTON_HEIGHT);
		bottomComponent.getLayoutManager().setColumns(this.C_BUTTON_HEIGHT + 2 + 50, "*", 50 + this.C_BUTTON_HEIGHT + 2);
		this.mPreviousComp = this.GUI.Component();

		if (this.getIndex() == 0)
		{
			this.mPreviousComp.setVisible(false);
		}

		this.mPageUp = this.GUI.SmallButton("LeftArrow");
		this.mPageUp.setPressMessage("onPageUp");
		this.mPageUp.addActionListener(this);
		this.mPreviousComp.add(this.mPageUp);
		this.mPreviousLabel = this.GUI.Label(previousText);
		this.mPreviousLabel.setSize(50, this.C_BUTTON_HEIGHT);
		this.mPreviousLabel.setPreferredSize(50, this.C_BUTTON_HEIGHT);
		this.mPreviousLabel.setPosition(this.C_BUTTON_HEIGHT + 2, 0);
		this.mPreviousLabel.setTextAlignment(0.5, 0.5);
		this.mPreviousComp.add(this.mPreviousLabel);
		bottomComponent.add(this.mPreviousComp);
		this.mPageLabel = this.GUI.Label("Page");
		this.mPageLabel.setTextAlignment(0.5, 0.5);
		bottomComponent.add(this.mPageLabel);
		this.mNextComp = this.GUI.Component();
		this.mNextLabel = this.GUI.Label(nextText);
		this.mNextLabel.setSize(30, this.C_BUTTON_HEIGHT);
		this.mNextLabel.setPreferredSize(30, this.C_BUTTON_HEIGHT);
		this.mNextLabel.setTextAlignment(0.5, 0.5);
		this.mNextComp.add(this.mNextLabel);
		this.mPageDown = this.GUI.SmallButton("RightArrow");
		this.mPageDown.setPressMessage("onPageDown");
		this.mPageDown.setPosition(this.mNextLabel.getSize().width + 2, 0);
		this.mPageDown.addActionListener(this);
		this.mNextComp.add(this.mPageDown);
		bottomComponent.add(this.mNextComp);
		return bottomComponent;
	}

	function setLabelsColor( color )
	{
		this.mPreviousLabel.setFontColor(color);
		this.mNextLabel.setFontColor(color);
		this.mPageLabel.setFontColor(color);
	}

	function attach( pAttachParent )
	{
		this.mAttachParent = pAttachParent;
		this.addActionListener(this.mAttachParent);
		this.mAttachParent.setScroll(this);
		this.add(pAttachParent, this.GUI.BorderLayout.CENTER);
		this.mAttachWindowHeight = this.mAttachParent.getSize().height;
	}

	function onPageUp( evt )
	{
		evt.setSelectionVisible(false);
		local i = this.mIndex;
		i -= this.mPageSize;

		if (i < 0)
		{
			i = 0;
		}

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();
		}

		this.setIndex(i);
		this._updateButton();
	}

	function onPageDown( evt )
	{
		evt.setSelectionVisible(false);
		local i = this.mIndex;
		i += this.mPageSize;

		if (this.mAttachParent)
		{
			this.mAttachParent.invalidate();
		}

		this.setIndex(i);
		this._updateButton();
	}

	function onLineUp( evt, ... )
	{
	}

	function onLineDown( evt, ... )
	{
	}

	function _calculatePageSize()
	{
		if (typeof this.mAttachParent == "instance" && (this.mAttachParent instanceof this.GUI.HTML))
		{
			local faceHeight = this.mAttachParent.getFont().getHeight();
			local htmlWindowHeight = this.mAttachWindowHeight - this.mIndent * 2;
			this.mPageSize = htmlWindowHeight / faceHeight;
		}
	}

	function _updateButton()
	{
		local rows = {};

		if (typeof this.mAttachParent == "instance" && (this.mAttachParent instanceof this.GUI.HTML))
		{
			this._calculatePageSize();
			rows = this.mAttachParent.getLayoutManager().getRows();

			if (!rows)
			{
				rows = {};
			}
		}

		local pageIndex = this.getIndex() / this.mPageSize + 1;
		local totalPages = rows.len() / this.mPageSize;
		local extraPage = rows.len() % this.mPageSize;

		if (extraPage > 0)
		{
			totalPages = totalPages + 1;
		}

		this.mPageLabel.setText(pageIndex + "/" + totalPages + " Pages");
		this.mButtonContainer.validate();

		if (this.getIndex() < 1)
		{
			this.mPreviousComp.setVisible(false);
		}
		else
		{
			this.mPreviousComp.setVisible(true);
		}

		if (pageIndex < totalPages)
		{
			this.mNextComp.setVisible(true);
		}
		else
		{
			this.mNextComp.setVisible(false);
		}

		if (this.mParentComponent && !this.mBottomComponent.isVisible() && (this.mNextComp.isVisible() || this.mPreviousComp.isVisible()))
		{
			this.mBottomComponent.setVisible(true);
		}

		if (this.mParentComponent && this.mBottomComponent.isVisible() && !this.mNextComp.isVisible() && !this.mPreviousComp.isVisible())
		{
			this.mBottomComponent.setVisible(false);
		}
	}

	function validate()
	{
		this.GUI.Component.validate();
		this._updateButton();
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
		this.mIndent = pIndent;
		this.mButtonContainer.insets.top = pIndent;
		this.mButtonContainer.insets.bottom = pIndent;
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		this.mWidget.addListener(this);
	}

	function _removeNotify()
	{
		this.mWidget.removeListener(this);
		this.GUI.Component._removeNotify();
	}

}

