this.require("GUI/Component");
this.require("GUI/ProgressBar");
this.require("GUI/HTML");
::LoadScreen <- {};
::_LoadScreenGUI <- null;
::gFirstTimeUser <- null;
class this.LoadScreen.GUI extends this.GUI.Component
{
	mLoadAppearance = "LoadScreen";
	mBackground = null;
	mText = "";
	mError = "";
	mAssemblyComplete = 0;
	mAssemblyTotal = 0;
	mMainProgressBar = null;
	mDownloadProgressBar = null;
	mDownloadLabel = null;
	mErrorLabel = null;
	mProgressAnimation = null;
	mAnnouncements = null;
	mTip = null;
	mTipIndex = -1;
	mAnnouncementIndex = -1;
	mFirstTimePlayer = true;
	mAnnouncementTexts = null;
	mTipTexts = null;
	mRandmomTipEvent = null;
	constructor()
	{
		::print("Load Screen Activated");
		::GUI.Component.constructor();
		this.setAppearance("GoldBorder");
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
		::Screen.setOverlayVisible("GUI/LoadScreen", true);
		this.mBackground = ::GUI.Component();
		this.mBackground.setAppearance(this.mLoadAppearance);
		this.mBackground.setSticky("center", "center");
		local width = ::Screen.getHeight() * 1.7708;
		this.mBackground.setSize(width.tointeger(), ::Screen.getHeight());
		this.mBackground.setPosition(this.mBackground.getWidth() / 2 * -1, this.mBackground.getHeight() / 2 * -1);
		this.mBackground.setVisible(true);
		this.add(this.mBackground);
		local panel = ::GUI.Component();
		panel.setAppearance("GoldBorder");
		panel.setSticky("center", "bottom");
		panel.setPosition(-320, -100);
		panel.setSize(640, 90);
		this.add(panel);
		local font = ::GUI.Font("Maiandra", 32);
		local loadTitle = ::GUI.Label("");
		loadTitle.setAppearance("Label");
		loadTitle.setPosition(15, 6);
		loadTitle.setFont(font);
		loadTitle.setFontColor("ffff99");
		loadTitle.setText("Loading...");
		loadTitle.setAutoFit(false);
		panel.add(loadTitle);
		font = ::GUI.Font("MaiandraOutline", 16);
		this.mMainProgressBar = this.GUI.ProgressBar(this.mLoadAppearance + "/MainProgressBar");
		this.mMainProgressBar.setPosition(10, 35);
		this.mMainProgressBar.setSize(610, 31);
		this.mMainProgressBar.setFont(font);
		this.mMainProgressBar.setFontColor("ffff99");
		this.mMainProgressBar.setVisible(true);
		this.mMainProgressBar.setMaskVisible(false);
		this.mMainProgressBar.setLabelVisible(true);
		this.mMainProgressBar.setLabelMode(this.GUI.ProgressBar.FRACTION);
		this.mMainProgressBar.setLabelVisible(false);
		this.mMainProgressBar.setMax(100);
		this.mMainProgressBar.setCurrent(0);
		this.mMainProgressBar.setAnimated(true);
		panel.add(this.mMainProgressBar);
		this.mDownloadProgressBar = this.GUI.ProgressBar(this.mLoadAppearance + "/SmallProgressBar");
		this.mDownloadProgressBar.setPosition(414, 72);
		this.mDownloadProgressBar.setSize(196, 11);
		this.mDownloadProgressBar.setFont(font);
		this.mDownloadProgressBar.setFontColor("ffff99");
		this.mDownloadProgressBar.setVisible(true);
		this.mDownloadProgressBar.setMaskVisible(false);
		this.mDownloadProgressBar.setLabelMode(this.GUI.ProgressBar.FRACTION);
		this.mDownloadProgressBar.setMax(1);
		this.mDownloadProgressBar.setCurrent(0);
		this.mDownloadProgressBar.setAnimated(true);
		this.mDownloadProgressBar.setAnimateReverse(false);
		panel.add(this.mDownloadProgressBar);
		this.mProgressAnimation = this.GUI.ProgressAnimation();
		this.mProgressAnimation.setPosition(600, 9);
		this.mProgressAnimation.setSize(20, 20);
		panel.add(this.mProgressAnimation);
		font = ::GUI.Font("Maiandra", 16);
		this.mDownloadLabel = ::GUI.HTML();
		this.mDownloadLabel.setAppearance("Container");
		this.mDownloadLabel.setPosition(10, 70);
		this.mDownloadLabel.setSize(385, 12);
		this.mDownloadLabel.setFont(font);
		this.mDownloadLabel.setFontColor("ffff99");
		this.mDownloadLabel.setLayoutManager(::GUI.FlowLayout());
		this.mDownloadLabel.getLayoutManager().setAlignment("right");
		this.mDownloadLabel.getLayoutManager().setGaps(0.0, 0.0);
		this.mDownloadLabel.setText("");
		panel.add(this.mDownloadLabel);
		this.mErrorLabel = ::GUI.HTML();
		this.mErrorLabel.setAppearance("Container");
		this.mErrorLabel.setPosition(10, 12);
		this.mErrorLabel.setSize(610, 12);
		this.mErrorLabel.setFont(font);
		this.mErrorLabel.setFontColor("ffff99");
		this.mErrorLabel.setVisible(true);
		this.mErrorLabel.setLayoutManager(::GUI.FlowLayout());
		this.mErrorLabel.getLayoutManager().setAlignment("right");
		this.mErrorLabel.getLayoutManager().setGaps(0.0, 0.0);
		this.mErrorLabel.setText("");
		panel.add(this.mErrorLabel);
		local generalTipOuterComponent = ::GUI.Panel(this.GUI.BoxLayoutV());
		generalTipOuterComponent.setAppearance("PanelTransparent");
		generalTipOuterComponent.getLayoutManager().setGap(10);
		generalTipOuterComponent.setSticky("center", "bottom");
		generalTipOuterComponent.setPosition(-320, -269);
		generalTipOuterComponent.setSize(640, 162);
		generalTipOuterComponent.setPreferredSize(640, 162);
		generalTipOuterComponent.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.0));
		this.add(generalTipOuterComponent);
		local announceBG = ::GUI.Component(null);
		announceBG.setAppearance("GoldBorderTransparent");
		announceBG.setSize(496, 98);
		announceBG.setPreferredSize(496, 98);
		generalTipOuterComponent.add(announceBG);
		local announceComp = ::GUI.Component(this.GUI.BoxLayoutV());
		announceComp.setAppearance("SemiBlack");
		announceComp.setPosition(3, 3);
		announceComp.setSize(490, 92);
		announceComp.setPreferredSize(490, 92);
		announceComp.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.69999999));
		announceBG.add(announceComp);
		local announceFont = ::GUI.Font("Maiandra", 32);
		local announceTitle = ::GUI.Label("Announcements");
		announceTitle.setFont(announceFont);
		announceTitle.setAutoFit(false);
		announceTitle.setFontColor("ffff99");
		announceComp.add(announceTitle);
		local announceTextFont = ::GUI.Font("Maiandra", 18);
		this.mAnnouncements = ::GUI.HTML();
		this.mAnnouncements.setFont(announceTextFont);
		this.mAnnouncements.setSize(480, 70);
		this.mAnnouncements.setPreferredSize(480, 70);
		this.mAnnouncements.setText("Welcome to Earth Eternal!");
		this.mAnnouncements.addActionListener(this);
		announceComp.add(this.mAnnouncements);
		local tipBG = ::GUI.Panel(null);
		tipBG.setAppearance("SilverBorderTransparent");
		tipBG.setSize(640, 56);
		tipBG.setPreferredSize(640, 56);
		generalTipOuterComponent.add(tipBG);
		local tipComp = ::GUI.Component(null);
		tipComp.setAppearance("SemiBlack");
		tipComp.setPosition(3, 3);
		tipComp.setSize(624, 38);
		tipComp.setPreferredSize(624, 38);
		tipComp.setBlendColor(this.Color(0.0, 0.0, 0.0, 0.69999999));
		tipBG.add(tipComp);
		local tipFont = ::GUI.Font("Maiandra", 18);
		this.mTip = this.GUI.HTML();
		this.mTip.setPosition(8, 10);
		this.mTip.setFontColor(this.Color("eeeeee"));
		this.mTip.setFont(tipFont);
		this.mTip.setSize(624, 34);
		this.mTip.setPreferredSize(624, 34);
		this.mTip.addActionListener(this);
		this.mTip.setText("Welcome to Planet Forever - IceEE!");
		tipComp.add(this.mTip);
		this.mAnnouncementTexts = [
			"Welcome to Planet Forever - IceEE!"
		];
		this.mTipTexts = [
			"Welcome to Planet Forever - IceEE!"
		];
		this.validate();
		this.setVisible(true);
		this.setOverlay("GUI/LoadScreen");
	}

	function _addNotify()
	{
		this.GUI.Component._addNotify();
		::_root.addListener(this);
	}

	function _removeNotify()
	{
		this.GUI.Component._removeNotify();
		::_root.removeListener(this);
	}

	function onMousePressed( evt )
	{
		this.GUI._Manager.requestKeyboardFocus(this);
	}

	function setAnnouncementTexts( texts )
	{
		this.mAnnouncementTexts = texts;
		this._randomTip();
	}

	function setTipTexts( texts )
	{
		this.mTipTexts = texts;
		this._randomTip();
	}

	function _randomTip()
	{
		local announcementIndex = 0;
		local tipIndex = 0;

		do
		{
			announcementIndex = this.rand() % this.mAnnouncementTexts.len();
		}
		while (announcementIndex == this.mAnnouncementIndex && this.mAnnouncementTexts.len() > 1);

		do
		{
			tipIndex = this.rand() % this.mTipTexts.len();
		}
		while (tipIndex == this.mTipIndex && this.mTipTexts.len() > 1);

		if (this.mAnnouncementIndex != announcementIndex)
		{
			this.mAnnouncements.setText(this.mAnnouncementTexts[announcementIndex]);
		}

		if (this.mTipIndex != tipIndex)
		{
			this.mTip.setText(this.mTipTexts[tipIndex]);
		}

		this.mAnnouncementIndex = announcementIndex;
		this.mTipIndex = tipIndex;

		if (this.mRandmomTipEvent)
		{
			::_eventScheduler.cancel(this.mRandmomTipEvent);
		}

		this.mRandmomTipEvent = ::_eventScheduler.fireIn(20.0, this, "_randomTip");
	}

	function onLinkClicked( message, data )
	{
		if ("href" in data)
		{
			this.System.openURL(data.href);
		}
	}

	function _updateProgress()
	{
		this.mMainProgressBar.setMax(this.mAssemblyTotal);
		this.mMainProgressBar.setCurrent(this.mAssemblyComplete);
	}

	function _isFirstTimeUser()
	{
		if (::gFirstTimeUser == null)
		{
			local returning = ::_cache.getCookie("ReturningUser");
			::_cache.setCookie("ReturningUser", "true");

			if (returning != "true")
			{
				::gFirstTimeUser = true;
				return true;
			}
			else
			{
				::gFirstTimeUser = false;
				return false;
			}
		}

		return ::gFirstTimeUser;
	}

	function setProgress( complete, total )
	{
		this.mAssemblyComplete = complete;
		this.mAssemblyTotal = total;
		this._updateProgress();
	}

	function onProgress( pBytesCurrent, pBytesTotal, pProgress )
	{
		this.mDownloadProgressBar.setMax(pBytesTotal);
		this.mDownloadProgressBar.setCurrent(pBytesCurrent);
		local bytesTotal = 0;

		if (pBytesTotal > 0)
		{
			bytesTotal = pBytesTotal / 1024;
		}

		this.mText = "Current resource download (" + bytesTotal + " K)";
		this.mDownloadLabel.setText(this.mText);
	}

	function onError( pString )
	{
		this.mError = pString;
	}

	function onScreenResize()
	{
		this.setSize(::Screen.getWidth(), ::Screen.getHeight());
		local width = ::Screen.getHeight() * 1.7708;
		this.mBackground.setSize(width.tointeger(), ::Screen.getHeight());
		this.mBackground.setPosition(this.mBackground.getWidth() / 2 * -1, this.mBackground.getHeight() / 2 * -1);
	}

	function onKeyPressed( evt )
	{
		if (this.isVisible() && (evt.keyCode == this.Key.VK_D || evt.keyCode == this.Key.VK_F2))
		{
			this.log.debug("LoadScreen Debugging Toggle");
			this.EvalCommand("/debug LoadScreen");
		}
	}

	function onUpdateText()
	{
		this.mDownloadLabel.setText(this.mText);
		this.mErrorLabel.setText(this.mError);
	}

	static mClassName = "LoadScreen";
}

class this.LoadScreenManager 
{
	mInGame = false;
	mGUI = null;
	mLastPendingCountAudit = null;
	mListener = null;
	mSentLoadingQuery = false;
	mShowTimer = null;
	mCloseTimer = null;
	mAnnouncementTexts = null;
	mTipTexts = null;
	constructor()
	{
		this._exitFrameRelay.addListener(this);
		this.mTipTexts = [
			"Welcome to Earth Eternal"
		];
		this.mAnnouncementTexts = [
			"Welcome to Earth Eternal"
		];
		this.fetchAnnouncements();
		this.fetchTips();
		this.setLoadScreenVisible(true);
	}

	function fetchAnnouncements()
	{
		local req = this.XMLHttpRequest();
		local self = this;
		req.onreadystatechange = function () : ( self )
		{
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
					local text = this.responseText;
					text = ::Util.replace(text, "\r", "");
					text = ::Util.replace(text, "\n", "");
					self.mAnnouncementTexts = this.Util.split(text, "<hr/>");

					if (self.mGUI)
					{
						self.mGUI.setAnnouncementTexts(self.mAnnouncementTexts);
					}
				}

				return;
			}
		};
		local txt = "";
		
		print("ICE! Fetching loading_announcements " + this.Util.getWebServerRoot() + "loading_announcements\n");
		req.open("GET", this.Util.getWebServerRoot() + "loading_announcements");
		req.send(txt);
		print("ICE! Fetched loading_announcements " + this.Util.getWebServerRoot() + "loading_announcements\n");
	}

	function fetchTips()
	{
		local req = this.XMLHttpRequest();
		local self = this;
		req.onreadystatechange = function () : ( self )
		{
			if (this.readyState == 4)
			{
				if (this.status == 200)
				{
					local text = this.responseText;
					text = ::Util.replace(text, "\r", "");
					text = ::Util.replace(text, "\n", "");
					self.mTipTexts = this.Util.split(text, "<br/>");

					if (self.mGUI)
					{
						self.mGUI.setTipTexts(self.mTipTexts);
					}
				}

				return;
			}
		};
		local txt = "";
		print("ICE! Fetching tips " + this.Util.getWebServerRoot() + "tips\n");
		req.open("GET", this.Util.getWebServerRoot() + "tips");
		req.send(txt);
		print("ICE! Fetched tips " + this.Util.getWebServerRoot() + "tips\n");
	}

	function closeGui()
	{
		::GUI._Manager.requestKeyboardFocus(null);

		if (this.mGUI)
		{
			this.mGUI.destroy();
			this.mGUI = null;
		}

		this.mSentLoadingQuery = false;
	}

	function onExitFrame()
	{
		this.States.event("onUpdateLoadScreen");

		if (this.mGUI)
		{
			this.sendLoadingQuery();
		}
	}

	function sendLoadingDoneQuery()
	{
		if (this.mGUI == null)
		{
			return;
		}

		if (::_Connection.isPlaying())
		{
			::_Connection.sendQuery("client.loading", this, [
				false
			]);
		}

		this.mCloseTimer = null;
		this.closeGui();
	}

	function sendLoadingQuery()
	{
		if (this.mSentLoadingQuery == false && ::_Connection.isPlaying())
		{
			::_Connection.sendQuery("client.loading", {}, [
				true
			]);
			this.mSentLoadingQuery = true;
		}
	}

	function onQueryComplete( qa, results )
	{
	}

	function onQueryError( qa, msg )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
		this.log.debug("Load screen query error: " + msg);
	}

	function onQueryTimeout( qa )
	{
		::_Connection.sendQuery(qa.query, this, qa.args);
		this.log.debug("Load screen query timed out.");
	}

	function showLoadScreen()
	{
		if (this.mGUI != null)
		{
			return;
		}

		this.mGUI = this.LoadScreen.GUI();
		this.mGUI.setTipTexts(this.mTipTexts);
		this.mGUI.setAnnouncementTexts(this.mAnnouncementTexts);
		this.mGUI.requestKeyboardFocus();
		this.sendLoadingQuery();

		if (::_buildTool != null)
		{
			::_buildTool._forwardStop();
			::_buildTool._backwardStop();
			::_buildTool._strafeLeftStop();
			::_buildTool._strafeRightStop();
		}

		if (::_playTool != null && this._avatar != null)
		{
			::_playTool._forwardStop();
			::_playTool._backwardStop();
			::_playTool._strafeLeftStop();
			::_playTool._strafeRightStop();
		}

		this.mShowTimer = null;
	}

	function setLoadScreenVisible( visible, ... )
	{
		local force = vargc > 0 ? vargv[0] : false;

		if (visible)
		{
			if (this.mGUI == null)
			{
				if (!force)
				{
					if (this.mShowTimer == null)
					{
						this.mShowTimer = ::_eventScheduler.fireIn(::_Connection.isPlaying() ? 3.0 : 0.0, this, "showLoadScreen");
					}
				}
				else
				{
					if (this.mShowTimer != null)
					{
						::_eventScheduler.cancel(this.mShowTimer);
						this.mShowTimer = null;
					}

					this.showLoadScreen();
				}
			}

			if (this.mCloseTimer)
			{
				::_eventScheduler.cancel(this.mCloseTimer);
				this.mCloseTimer = null;
			}
		}
		else
		{
			if (this.mCloseTimer == null && this.mGUI != null)
			{
				this.mCloseTimer = ::_eventScheduler.fireIn(0.5, this, "sendLoadingDoneQuery");
			}

			if (this.mShowTimer != null)
			{
				::_eventScheduler.cancel(this.mShowTimer);
				this.mShowTimer = null;
			}
		}
	}

	function getLoadScreenVisible()
	{
		return this.mGUI != null;
	}

	function update( progressCurrent, progressTotal )
	{
		if (this.mGUI)
		{
			this.mGUI.setProgress(progressCurrent, progressTotal);
		}
	}

	function downloadProgress( current, total, text )
	{
		if (this.mGUI)
		{
			this.mGUI.onProgress(current, total, text);
		}
	}

	function setInGame( which )
	{
		this.mInGame = which;
		this.onExitFrame();
	}

	function setListener( listener )
	{
		this.mListener = listener;
	}

}

this._loadScreenManager <- null;
this.LoadScreen.setInGame <- function ( which )
{
	this._loadScreenManager.setInGame(which);
};
this.LoadScreen.update <- function ( progressCurrent, progressTotal )
{
	this._loadScreenManager.update(progressCurrent, progressTotal);
};
this.LoadScreen.downloadProgress <- function ( current, total, text )
{
	this._loadScreenManager.downloadProgress(current, total, text);
};
this.LoadScreen.setListener <- function ( listener )
{
	this._loadScreenManager.setListener(listener);
};
this.LoadScreen.isVisible <- function ()
{
	return this._loadScreenManager.mGUI != null;
};
