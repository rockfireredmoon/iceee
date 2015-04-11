this.require("UI/Screens");
local wasVisible = this.Screens.close("PingScreen");
class this.Screens.PingScreen extends this.GUI.Frame
{
	mLegend = null;
	mPingQuery = null;
	mOutput = null;
	mPingSeq = 0;
	mPauseButton = null;
	mRouterTime = null;
	mTraceTime = null;
	mRouterDirectTime = null;
	mInternalTime = null;
	mPaused = false;
	constructor()
	{
		this.GUI.Frame.constructor("Ping");
		this.mLegend = this.GUI.Label("Router,Simulator,S->SQ, SQ->JG, JG->RQ, RQ Time, Before RT, Run Time, After RT, SQ->JG\n" + "  tr=(Trace) rd=(Router Direct) r=(Router Queued)");
		this.mPauseButton = this.GUI.NarrowButton("Pause");
		this.mPauseButton.addActionListener(this);
		this.mOutput = this.GUI.InputArea();
		this.mOutput.setMultiLine(true);
		local cmain = this.GUI.Container(this.GUI.BorderLayout());
		cmain.setInsets(5);
		cmain.add(this.GUI.ScrollPanel(this.mOutput), this.GUI.BorderLayout.CENTER);
		cmain.add(this.mLegend, this.GUI.BorderLayout.NORTH);
		cmain.add(this.mPauseButton, this.GUI.BorderLayout.SOUTH);
		this.setContentPane(cmain);
		this.setSize(650, 350);
	}

	function setVisible( value )
	{
		local wasVisible = this.isVisible();
		this.GUI.Frame.setVisible(value);

		if (value && !wasVisible)
		{
			this.mPingSeq = 0;
			this.mOutput.setText("");
			this.ping();
		}
	}

	function onActionPerformed( button )
	{
		this.mPaused = !this.mPaused;

		if (!this.mPaused)
		{
			this._eventScheduler.fireIn(1.0, this, "ping");
		}
	}

	function output( txt )
	{
		this.mOutput.setText(txt + "\n" + this.mOutput.getText());
	}

	function ping()
	{
		local t = ::System.currentTimeMillis();
		this.log.debug("Pinging!");
		this.mPingSeq += 1;
		::_Connection.sendQuery("util.pingrouterdirect", this, []);
		this.mTraceTime = null;
		this.mRouterTime = null;
		this.mRouterDirectTime = null;
		this.mInternalTime = null;
	}

	function onQueryComplete( qa, results )
	{
		local t = this.System.currentTimeMillis();

		if (qa.query == "util.trace")
		{
			this.mTraceTime = t - qa._sendTime;
		}
		else if (qa.query == "util.pingrouter")
		{
			this.mRouterTime = t - qa._sendTime;
			::_Connection.sendQuery("util.trace", this, []);
		}
		else if (qa.query == "util.pingrouterdirect")
		{
			this.mRouterDirectTime = t - qa._sendTime;
			::_Connection.sendQuery("util.pingrouter", this, []);
		}

		if (!this.mPaused && this.isVisible() && this.mTraceTime != null && this.mRouterTime != null && this.mRouterDirectTime != null)
		{
			local nums = this.Util.join(results[0], ", ");
			this.output("Ping [seq=" + this.mPingSeq + "]: rd=" + this.mRouterDirectTime + "  r=" + this.mRouterTime + "  tr=" + this.mTraceTime + " ms [" + nums + "]");
			this._eventScheduler.fireIn(1.0, this, "ping");
		}
	}

	function onQueryTimeout( qa, results )
	{
		this.output(qa.query + " timed out, retrying...");
		this._eventScheduler.fireIn(1.0, this, "ping");
	}

}


if (wasVisible)
{
	this.Screens.toggle("PingScreen");
}
