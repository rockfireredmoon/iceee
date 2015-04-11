this.require("UI/Screens");
local wasVisible = this.Screens.close("ScriptTest");
class this.Screens.ScriptTest extends this.GUI.Frame
{
	static mScreenName = "ScriptTest";
	mScript = null;
	mError = null;
	constructor()
	{
		this.GUI.Frame.constructor("Script Tester");
		local content = this.GUI.Container(this.GUI.BorderLayout());
		local container = this.GUI.Container(this.GUI.BorderLayout());
		container.add(this.GUI.Label("Script:"));
		container.add(this.GUI.Button("Test!", this, "onTestPressed"), this.GUI.BorderLayout.EAST);
		this.mError = this.GUI.HTML();
		this.mScript = ::GUI.InputArea("");
		content.add(this.mScript, this.GUI.BorderLayout.CENTER);
		content.add(this.mError, this.GUI.BorderLayout.SOUTH);
		content.add(container, this.GUI.BorderLayout.NORTH);
		content.setInsets(4);
		this.setContentPane(content);
		this.setSize(250, 275);
	}

	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, error )
	{
		this.mError.setText(error);
	}

	function onTestPressed( button )
	{
		this.mError.setText("");
		this._Connection.sendQuery("util.testscript", this, [
			this.mScript.getText()
		]);
	}

}


if (wasVisible)
{
	this.Screens.toggle("ScriptTest");
}
