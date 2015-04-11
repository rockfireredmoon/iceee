this.require("GUI/InputArea");
class this.GUI.CancellableInputArea extends this.GUI.InputArea
{
	static mClassName = "CancellableInputArea";
	mPrevText = null;
	constructor( ... )
	{
		this.GUI.InputArea.constructor();

		if (vargc > 0)
		{
			this.GUI.InputArea.setText(vargv[0]);
		}

		this.mPrevText = this.getText();
	}

	function setText( text )
	{
		this.mPrevText = this.getText();
		this.GUI.InputArea.setText(text);
	}

	function _fireActionPerformed( pMessage )
	{
		if (pMessage == "onInputComplete")
		{
			this.mPrevText = this.getText();
		}
		else if (pMessage == "onInputCancelled")
		{
			this.setText(this.mPrevText);
		}

		this.GUI.InputArea._fireActionPerformed(pMessage);
	}

}

