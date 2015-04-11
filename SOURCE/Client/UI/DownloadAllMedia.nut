this.require("GUI/FullScreenComponent");
class this.Screens.DownloadAllMedia extends this.GUI.FullScreenComponent
{
	static mClassName = "DownloadAllMedia";
	mQuestion = null;
	mPanel = null;
	mMediaPath = "";
	mAllMedia = null;
	mAllMediaInfo = null;
	mCount = 0;
	NUM_CAR_TO_FETCH = 4;
	AVERAGE_TERRAIN_SIZE = 143360;
	mTotalBytes = 0;
	mCurrentBytes = 0;
	mDownloadProgressBar = null;
	mDownloadLabel = null;
	mDownloadComp = null;
	mStopDownload = false;
	mMessageText = "Earth Eternal will stream necessary content in the background" + " while playing, but for users with slower connections it may be desirable" + " to download all required media packages while outside the game to ensure optimal performance.";
	constructor()
	{
		::GUI.FullScreenComponent.constructor(this.GUI.BoxLayoutV());
		this.getLayoutManager().setAlignment(0.5);
		this.getLayoutManager().setPackAlignment(0.5);
		this.setAppearance("PanelTransparent");
		this._buildScreen();
		this.mAllMedia = [];
		this.mAllMediaInfo = {};
		this.initializeData();
		this.setOverlay("GUI/FullScreenComponentOverlay");
		this._cache.addListener(this);
		this._root.addListener(this);
	}

	function initializeData()
	{
		local terrainToExclude = [
			"Terrain-Blank",
			"Terrain-Blend",
			"Terrain-Templates",
			"Terrain-Filler_Tiles",
			"Terrain-NewBremen",
			"Terrain-Sandbox",
			"Terrain-Sandbox2",
			"Terrain-Templates",
			"Terrain-Test_Desert",
			"Terrain-TestDungeon1C",
			"Terrain-Testing_Gauntlet"
		];
		local allMedia = [];
		local mediaPath;

		foreach( key, v in ::MediaIndex )
		{
			mediaPath = ::GetFullPath(key);
			allMedia.append(mediaPath);
			this.mAllMediaInfo[mediaPath] <- ::MediaData(v[0]);
			this.mTotalBytes = this.mTotalBytes + v[0];
		}

		local terrainCount = 0;

		foreach( key, v in ::TerrainMap )
		{
			mediaPath = ::GetFullPath(key);
			local shouldExclude = false;

			foreach( terrianName in terrainToExclude )
			{
				if (mediaPath.tolower().find(terrianName.tolower()) != null)
				{
					shouldExclude = true;
					break;
				}
			}

			if (!shouldExclude)
			{
				allMedia.append(mediaPath);
				this.mAllMediaInfo[mediaPath] <- ::MediaData(this.AVERAGE_TERRAIN_SIZE);
				terrainCount = terrainCount + 1;
			}
		}

		this.mTotalBytes = this.mTotalBytes + terrainCount * this.AVERAGE_TERRAIN_SIZE;
		local totalBytes = this.mTotalBytes / 1024;
		this.mDownloadProgressBar.setMax(totalBytes);
		this.mAllMedia = allMedia;
	}

	function startDownload()
	{
		this.mCount = 0;
		this.mStopDownload = false;

		if (this.mAllMedia == null || this.mAllMedia.len() <= 0)
		{
			this.log.debug("FINISH downloading all media!");
			this.onDownloadComplete();
		}

		this.handleFetch();
	}

	function handleFetch()
	{
		while (this.mAllMedia && this.mAllMedia.len() > 0 && this.mCount < this.NUM_CAR_TO_FETCH)
		{
			local media = this.mAllMedia.pop();
			this.fetch(media);
			this.mCount = this.mCount + 1;
		}
	}

	function fetch( media )
	{
		this.log.debug("NOW FETCHING: " + media);
		this._cache.fetch(media);
	}

	function onProgress( media, bytesLoaded, bytesTotal )
	{
		if (media in this.mAllMediaInfo)
		{
			local mediaData = this.mAllMediaInfo[media];

			if (mediaData)
			{
				local currentBytes = mediaData.getCurrentBytes();
				local diffBytes = bytesLoaded - currentBytes;
				mediaData.setCurrentBytes(bytesLoaded);
				this._updateCurrentBytes(diffBytes);
			}
		}

		this.log.debug("On Progress media: " + media + " bytesLoaded: " + bytesLoaded + " bytesTotal " + bytesTotal);
	}

	function _updateCurrentBytes( loadedBytes )
	{
		this.mCurrentBytes = this.mCurrentBytes + loadedBytes;
		local currentKB = 0;
		local totalKB = 0;
		currentKB = this.mCurrentBytes / 1024;
		totalKB = this.mTotalBytes / 1024;
		this.mDownloadProgressBar.setMax(totalKB);
		this.mDownloadProgressBar.setCurrent(currentKB);
		this.mDownloadComp.setVisible(true);
		local text = currentKB + "/" + totalKB + " KB)";
		this.mDownloadLabel.setText(text);
	}

	function onComplete( media, handle )
	{
		this.log.debug("FETCH HAS COMPLETED: " + media);
		this._handleMediaDownloaded(media);
	}

	function onError( media, error, code )
	{
		this.log.debug("Error fetching " + media + " my error " + error);
		this._handleMediaDownloaded(media);
	}

	function _handleMediaDownloaded( media )
	{
		if (media in this.mAllMediaInfo)
		{
			local mediaData = this.mAllMediaInfo[media];

			if (mediaData)
			{
				local currentBytes = mediaData.getCurrentBytes();
				local totalBytes = mediaData.getTotalBytes();
				local diffBytes = totalBytes - currentBytes;
				mediaData.setCurrentBytes(totalBytes);
				this._updateCurrentBytes(diffBytes);
			}
		}

		if (this.mStopDownload)
		{
			return;
		}

		this.mCount = this.mCount - 1;

		if (this.mCount == 0)
		{
			if (this.mAllMedia.len() <= 0)
			{
				this.log.debug("FINISH downloading all media!");
				this.onDownloadComplete();
			}
			else
			{
				this.handleFetch();
			}
		}
	}

	function _buildScreen()
	{
		this.setInsets(5, 10, 5, 10);
		this.mPanel = this.GUI.Component(this.GUI.BoxLayoutV());
		this.mPanel.getLayoutManager().setExpand(false);
		this.mPanel.getLayoutManager().setAlignment(0.5);
		this.mPanel.getLayoutManager().setGap(5);
		this.mPanel.setInsets(10, 20, 5, 20);
		this.mPanel.setAppearance("Panel");
		this.add(this.mPanel);
		local titleLabel = this.GUI.Label("Currently downloading media files...");
		titleLabel.setFont(this.GUI.Font("Maiandra", 28));
		this.mPanel.add(titleLabel);
		local messageHTML = this.GUI.HTML(this.mMessageText);
		messageHTML.setSize(350, 70);
		messageHTML.setPreferredSize(350, 70);
		this.mPanel.add(messageHTML);
		local font = ::GUI.Font("MaiandraOutline", 16);
		this.mDownloadProgressBar = this.GUI.ProgressBar("LoadScreen/SmallProgressBar");
		this.mDownloadProgressBar.setPosition(414, 72);
		this.mDownloadProgressBar.setSize(196, 11);
		this.mDownloadProgressBar.setFont(font);
		this.mDownloadProgressBar.setFontColor("ffff99");
		this.mDownloadProgressBar.setVisible(true);
		this.mDownloadProgressBar.setMaskVisible(false);
		this.mDownloadProgressBar.setLabelVisible(true);
		this.mDownloadProgressBar.setLabelMode(this.GUI.ProgressBar.PERCENTAGE);
		this.mDownloadProgressBar.setMax(1);
		this.mDownloadProgressBar.setCurrent(0);
		this.mDownloadProgressBar.setAnimated(true);
		this.mDownloadProgressBar.setAnimateReverse(false);
		this.mPanel.add(this.mDownloadProgressBar);
		this.mDownloadComp = this.GUI.Component(this.GUI.BoxLayout());
		this.mDownloadComp.getLayoutManager().setExpand(false);
		this.mDownloadComp.getLayoutManager().setAlignment(0.5);
		this.mDownloadComp.setSize(200, 20);
		this.mDownloadComp.setPreferredSize(200, 20);
		this.mPanel.add(this.mDownloadComp);
		local downloadTitleLabel = this.GUI.Label("Downloaded (");
		downloadTitleLabel.setFont(this.GUI.Font("Maiandra", 18));
		this.mDownloadComp.add(downloadTitleLabel);
		this.mDownloadLabel = this.GUI.Label("");
		this.mDownloadLabel.setFont(this.GUI.Font("Maiandra", 18));
		this.mDownloadComp.add(this.mDownloadLabel);
		this.mDownloadComp.setVisible(false);
		local spacer = this.GUI.Spacer(10, 10);
		this.mPanel.add(spacer);
		local cancelButton = this.GUI.NarrowButton("Cancel");
		cancelButton.addActionListener(this);
		cancelButton.setPressMessage("cancel");
		this.mPanel.add(cancelButton);
		this.setVisible(true);
	}

	function cancel( evt )
	{
		if (this.mQuestion == null)
		{
			this.mQuestion = ::GUI.MessageBox.showYesNo("Cancelling will stop downloading all the needed art, terrain files. Are you sure you want to stop downloading?", this);
			this.mPanel.setVisible(false);
		}
	}

	function onShow()
	{
		this.requestKeyboardFocus();

		if (this.mQuestion == null)
		{
			this.mPanel.setVisible(true);
		}
	}

	function onActionSelected( window, text )
	{
		this.mPanel.setVisible(true);
		this.mQuestion = null;

		if (text == "Yes")
		{
			this.mStopDownload = true;
			this.setVisible(false);
		}
	}

	function onClose()
	{
		if (this.mQuestion)
		{
			this.mQuestion.close();
			this.mQuestion = null;
		}
	}

	function onDownloadComplete()
	{
		this.setVisible(false);
		local popupBox = this.GUI.MessageBox.show("All media files have finished downloading.");
	}

}

class this.MediaData 
{
	mTotalBytes = 0;
	mCurrentBytes = 0;
	constructor( totalBytes )
	{
		this.mTotalBytes = totalBytes;
	}

	function getTotalBytes()
	{
		return this.mTotalBytes;
	}

	function setCurrentBytes( bytes )
	{
		this.mCurrentBytes = bytes;
	}

	function getCurrentBytes()
	{
		return this.mCurrentBytes;
	}

}

