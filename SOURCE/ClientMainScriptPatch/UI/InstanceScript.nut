require("UI/Screens");
require("UI/ActionContainer");

class Screens.InstanceScript extends GUI.Frame
{
	static mScreenName = "InstanceScript";
	mScript = null;
	mInfo = null;
	mRefresh = null;
	mSubmit = null;
	mRun = null;
	mKill = null;
	mType = null;
	tabstop = 4;
	
	constructor()
	{
		GUI.Frame.constructor("Script Editor");
		
		// Info
		mInfo = ::GUI.HTML("<font size=\"22\">Instances script editor. See Wiki for help.</font>");
		mInfo.setResize(true);
		mInfo.setMaximumSize(500, null);
		mInfo.setInsets(0, 4, 0, 4);
		
		// Script
		mScript = ::GUI.InputArea("");
		mScript.setMultiLine(true);
		mScript.addActionListener(this);
		
		// Container
		local container = GUI.Container(this.GUI.GridLayout(1, 1));
		container.getLayoutManager().setColumns(460);
		container.getLayoutManager().setRows(310);
		container.getLayoutManager().setGaps(0, 4);
		container.setInsets(4, 8, 8, 4);
		container.add(mScript);
		
		// Refresh
		mRefresh = GUI.Button("Refresh");
		mRefresh.setReleaseMessage("onRefreshPressed");
		mRefresh.addActionListener(this);
		
		// Submit
		mSubmit = GUI.Button("Submit");
		mSubmit.setReleaseMessage("onSubmitPressed");
		mSubmit.addActionListener(this);
		
		// Kill
		mKill = GUI.Button("Stop Script");
		mKill.setReleaseMessage("onKillPressed");
		mKill.addActionListener(this);
		
		// Run
		mRun = GUI.Button("Run Script");
		mRun.setReleaseMessage("onRunScriptPressed");
		mRun.addActionListener(this);
		
		// Copy
		local copy = GUI.Button("Copy All");
		copy.setTooltip("Copy ENTIRE script with contents of clipboard");
		copy.setReleaseMessage("onCopyPressed");
		copy.addActionListener(this);
		
		// Paste
		local paste = GUI.Button("Paste All");
		paste.setTooltip("Replace ENTIRE script with contents of clipboard");
		paste.setReleaseMessage("onPastePressed");
		paste.addActionListener(this);
		
		
		// Buttons
		local buttons = GUI.Container(GUI.BoxLayout());
		buttons.getLayoutManager().setPackAlignment(0.5);
		buttons.add(mSubmit);
		buttons.add(mRefresh);
		buttons.add(mRun);
		buttons.add(mKill);
		buttons.add(copy);
		buttons.add(paste);
		buttons.setInsets(0, 0, 4, 0);
		
		// Type
		mType = GUI.DropDownList();
		mType.addChoice("Instance script");
		mType.addChoice("Quest script");
		mType.addChoice("AI script");
		mType.addSelectionChangeListener({
			t = this,
			function onSelectionChange( list ) {
				this.t.resetState();
			}
		});
		
		// Top
		local top = GUI.Container(GUI.BorderLayout());
		top.setInsets(3, 3, 3, 3);
		top.add(mInfo, GUI.BorderLayout.CENTER);
		top.add(mType, GUI.BorderLayout.WEST);
		
		// Content
		local content = GUI.Container(GUI.BoxLayoutV());
		content.add(top);
		content.setInsets(4, 4, 4, 4);
		content.add(container);
		content.add(buttons);
		
		// This
		setContentPane(content);
		setInsets(4, 4, 4, 4);
		setSize(500, 415);
		center();
		
		// Init
		mScript.setText("");
		::_root.setKeysEnabled(true);
		_refresh();
	}
	
	function resetState() {	
		mSubmit.setEnabled(false);
		mRefresh.setText("Refresh");
		mRun.setEnabled(true);
		mKill.setEnabled(true);
	}
	
	function onTextChanged( text ) {
		mSubmit.setEnabled(true);
		mRefresh.setText("Cancel");
		mRun.setEnabled(false);
		mKill.setEnabled(false);
	}

	function onCopyPressed( button )
	{	
		System.setClipboard(mScript.getText());
		log.info("Saved script to clipboard.");
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
			
			resetState();
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

