this.require("GUI/GUI");
class this.GUI.DnDEvent 
{
	static ACTION_NONE = 0;
	static ACTION_COPY = 1;
	static ACTION_MOVE = 2;
	static ACTION_COPY_OR_MOVE = 3;
	static ACTION_LINK = 4;
	mTransferable = null;
	mConsumed = false;
	mSupportedActions = null;
	mAcceptedActions = null;
	mVisual = null;
	mVisualBG = null;
	mDragSource = null;
	mWidget = null;
	mAltDown = false;
	mControlDown = false;
	mShiftDown = false;
	constructor( ... )
	{
		this.mTransferable = null;
		this.mAcceptedActions = 0;
		this.mSupportedActions = 0;
		this.mVisual = null;
		this.mVisualBG = null;
		this.mConsumed = false;

		if (vargc > 0)
		{
			local evt = vargv[0];
			this.mAltDown = evt.isAltDown();
			this.mControlDown = evt.isControlDown();
			this.mShiftDown = evt.isShiftDown();
		}
	}

	function acceptDrag( transferable, supportedActions, ... )
	{
		this.mTransferable = transferable;
		this.mSupportedActions = supportedActions;
		this.mVisual = vargc > 0 ? vargv[0] : null;
		this.mVisualBG = vargc > 1 ? vargv[1] : null;
		this.mDragSource = vargc > 2 ? vargv[2] : null;
		this.mConsumed = true;
	}

	function getDragSource()
	{
		if (this.mDragSource)
		{
			return this.mDragSource;
		}

		return null;
	}

	function getVisual()
	{
		return this.mVisual;
	}

	function getVisualBG()
	{
		return this.mVisualBG;
	}

	function getTransferable()
	{
		return this.mTransferable;
	}

	function getSupportedActions()
	{
		this.mSupportedActions = true;
	}

	function acceptDrop( actions )
	{
		this.mAcceptedActions = actions;
		this.mConsumed = true;
	}

	function consume()
	{
		this.mConsumed = true;
	}

	function getAction()
	{
		return this.mAcceptedActions;
	}

	function isConsumed()
	{
		return this.mConsumed;
	}

	function isAltDown()
	{
		return this.mAltDown;
	}

	function isControlDown()
	{
		return this.mControlDown;
	}

	function isShiftDown()
	{
		return this.mShiftDown;
	}

	function getWidget()
	{
		return this.mWidget;
	}

}

