this.require("UI/Screens");
this.require("UI/ActionContainer");
class this.Screens.InteractEdit extends this.GUI.Frame
{
	static mScreenName = "InteractEdit";
	mScript = null;
	mInfo = null;
	tabstop = 4;
	
	constructor()
	{
		this.GUI.Frame.constructor("Interacts");
		
		// Info
		this.mInfo = ::GUI.HTML("<font size=\"22\">Instances script editor. See Wiki for help.</font>");
		this.mInfo.setResize(true);
		this.mInfo.setMaximumSize(500, null);
		this.mInfo.setInsets(0, 4, 0, 4);
		
		// Script
		this.mScript = ::GUI.InputArea("");
		this.mScript.setMultiLine(true);
		
		// Container
		local container = this.GUI.Container(this.GUI.GridLayout(1, 1));
		container.getLayoutManager().setColumns(460);
		container.getLayoutManager().setRows(310);
		container.getLayoutManager().setGaps(0, 4);
		container.setInsets(4, 8, 8, 4);
		container.add(this.mScript);
		
		// Refresh
		local refresh = this.GUI.Button("Refresh");
		refresh.setReleaseMessage("onRefreshPressed");
		refresh.addActionListener(this);
		
		// Submit
		local submit = this.GUI.Button("Submit");
		submit.setReleaseMessage("onSubmitPressed");
		submit.addActionListener(this);
		
		// Kill
		local kill = this.GUI.Button("Stop Script");
		kill.setReleaseMessage("onKillPressed");
		kill.addActionListener(this);
		
		// Run
		local runScript = this.GUI.Button("Run Script");
		runScript.setReleaseMessage("onRunScriptPressed");
		runScript.addActionListener(this);
		
		// Copy
		local copy = this.GUI.Button("Copy All");
		copy.setTooltip("Copy ENTIRE script with contents of clipboard");
		copy.setReleaseMessage("onCopyPressed");
		copy.addActionListener(this);
		
		// Paste
		local paste = this.GUI.Button("Paste All");
		paste.setTooltip("Replace ENTIRE script with contents of clipboard");
		paste.setReleaseMessage("onPastePressed");
		paste.addActionListener(this);
		
		
		// Buttons
		local buttons = this.GUI.Container(this.GUI.BoxLayout());
		buttons.getLayoutManager().setPackAlignment(0.5);
		buttons.add(submit);
		buttons.add(refresh);
		buttons.add(runScript);
		buttons.add(kill);
		buttons.add(copy);
		buttons.add(paste);
		buttons.setInsets(0, 0, 4, 0);
		
		// Content
		local content = this.GUI.Container(this.GUI.BoxLayoutV());
		content.add(this.mInfo);
		content.setInsets(4, 4, 4, 4);
		content.add(container);
		content.add(buttons);
		
		// This
		this.setContentPane(content);
		this.setInsets(4, 4, 4, 4);
		this.setSize(500, 415);
		this.center();
		
		// Init
		this.mScript.setText("");
		::_root.setKeysEnabled(true);
		this._refresh();
	}

	function onCopyPressed( button )
	{	
		this.System.setClipboard(this.mScript.getText());
		this.log.info("Saved script to clipboard.");
	}

	function onPastePressed( button )
	{	
		try
		{
			this.mScript.setText(this.System.getClipboard());
		}
		catch( err )
		{
			this.log.debug("Error while pasting script: " + err);
		}
	}

	function onRunScriptPressed( button )
	{	
		::_Connection.sendQuery("script.run", this, []);
	}

	function onKillPressed( button )
	{	
		::_Connection.sendQuery("script.kill", this, []);
	}

	function onRefreshPressed( button )
	{	
		this._refresh();
	}

	function onSubmitPressed( button )
	{
		::_Connection.sendQuery("script.save", this, [this._collapseTabs(this.mScript.getText())]);
	}
	
	function setVisible( value )
	{
		if (value && !this.isVisible())
		{
			this.mScript.setText("");
			::_Connection.sendQuery("script.load", this, []);
		}

		this.GUI.Frame.setVisible(value);
	}

	function onQueryComplete( qa, results )
	{
		if (qa.query == "script.load")
		{
			local str = "";
			foreach( r in results )
			{
				if(r.len() == 2) 
				{					
					this.mInfo.setText("<font size=\"22\">Instances script for " + r[1] + 
						". Script is " + ( r[0] == "true" ? 
							"<font color=\"00ff00\">Active</font>" : 
							"<font color=\"ff0000\">Inactive</font>"));
				}
				else 
				{
					if(str != "") {
						str += "\n";
					}
					str = str + _expandTabs(r[0]);
				}
			}
			this.mScript.setText(str);
			this.mScript.mCursorStart = 0;
			this.mScript.mCursorEnd = 0;
			this.mScript.buildRows();
			this.mScript.updateCursorPosition();
		}
		else if (qa.query == "script.run" || qa.query == "script.kill" || qa.query == "script.save")
		{		
			this._refresh();
		}
	}
	
	function _expandTabs(text)
	{
        local c = 0;
        local s = "";
        foreach(x in text)
        {
                //print("C: " + c + " X: " + x + " (" + x.tochar() + ") \n");
                if(x == 9)
                {
                        for(local i = c; i < this.tabstop; i++)
                        {
                                s += " ";
                        }
                }
                else
                {
                        s += x.tochar();
                        c++;
                }
                if(c % this.tabstop == 0)
                        c = 0;
        }
        return s;
	}

	function _collapseTabs(text)
	{
        local sp = "";
        local s = "";
        foreach(x in text)
        {
                if(x == 32)
                {
                        sp += " ";
                        if(sp.len() == this.tabstop)
		                {
		                        s += "\t";
		                        sp = "";
		                }
                }
                else
                {
                	    if(sp != "") {
                	    	s += sp;
                	    	sp = "";
                	    }
                        s += x.tochar();
                }
        }
        if(sp.len() > 0)
        {
        	if(sp.len() == this.tabstop)
                s += "\t";
            else
            	s += sp;
        }
        return s;
	}


	function _refresh() 
	{
		::_Connection.sendQuery("script.load", this, []);
	}
}

