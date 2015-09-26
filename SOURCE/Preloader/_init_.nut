/*===========================================================================

	Player Preloader
	
	This is the built-in preloader that comes with the player. The goal is
	to be very lightweight (quick to load) and provide a pretty load screen
	while a more complicated composition is downloaded and started up.

 ===========================================================================*/

/// The meat of the preloading logic simply listens to events from the
/// media cache as it downloads whatever it's downloading.
Preloader <- {
	
	/**
	Center the info panel on the screen.
	*/
	function CenterPanel()
	{
		local w = Widget("Preloader/Panel");
		w.setPosition(
			(Screen.getWidth() - w.getWidth()) / 2,
			(Screen.getHeight() - w.getHeight()) / 2);
	}
	
	/// The current file being downloaded.
	mCurrentFile = "",
	
	/// The start time for the current file. Used to calculate the current
	/// download rate and estimated finish time.
	mStart = -1,
	
	/// The current running average download speed in bytes/millisecond.
	mRate = 0.0,
	
	/// The last time a progress update was sent for the current file.
	mRateMark = -1,
	
	/// A marker used to calculate the latest download rate.
	mPrevBytesLoaded = 0,
	
	/// If an error was encountered, this holds it (copied from onError).
	mLastError = null,
	
	/// A value that goes up and down on a sinusoidal manner every frame
	/// so that we can pulsate the color of a label to show "progress".
	mPulseValue = 0.0,
	
	/// The number of times we have tried downloading the archive
	mAttempts = 0,
	
	/// Take a URL style media reference, and strip off the ugly bits
	/// (including things like passwords!). Returns just the last bit
	/// after the last '/' character.
	function prettyMediaName(media)
	{
		local parts = split(media, "/");
		return parts[parts.len() - 1];
	},
	
	// When a file has been fully downloaded.
	function onComplete(media, file)
	{
		Composition.loadRoot(file, media);
	},
	
	// When an error has occurred (oh noes!)
	function onError(media, error, code)
	{
		mAttempts++;
		if ( mAttempts < 10 )
		{
			if( "src" in _args )
			{
				// Try downloading again...
				_cache.fetch(_args["src"]);
				return;
			}
		}
		
		media = prettyMediaName(media);
		print("[ERROR] Error with " + media + ": " + error);
		mLastError = "Error: " + media + ": " + error;
		local w = Widget("Preloader/Comment");
		w.setParam("colour", "1 0 0");
		w.setText(mLastError);
	},
	
	// When a file has started downloading.
	function onStart(media)
	{
		media = prettyMediaName(media);
		print("[DEBUG] Fetching " + media);
		Widget("Preloader/Task").setText(media);
		mStart = System.currentTimeMillis();
		mRateMark = mStart - 1; // Avoid div-0 later
		mPrevBytesLoaded = 0;
	},
	
	// Periodically throughout the download process. No guarantee
	// on frequency though!
	function onProgress(media, loadedBytes, totalBytes)
	{
		mCurrentFile = media;
		media = prettyMediaName(media);
		
		//print("[DEBUG] Fetching " + media + " (" + loadedBytes + " of " + totalBytes + ")");

		local time = System.currentTimeMillis();

		local rate = (loadedBytes - mPrevBytesLoaded).tofloat() /
				     (time - mRateMark);
		local pctDone = loadedBytes * 100.0 / totalBytes;
		mRateMark = time;
		mPrevBytesLoaded = loadedBytes;

		// Update a running average.
		local alpha = 0.98;
		mRate = alpha * mRate + (1 - alpha) * rate;

		// Our rate is in bytes/millisecond but we want bytes/sec
		
		local bytesPerSecond = mRate * 1000;
		local kBps = bytesPerSecond / 1024.0; // kB/sec is easier on the eyes. ;)
		
		local secondsLeft = (totalBytes - loadedBytes).tofloat() /
							( bytesPerSecond < 1 ? 1 : bytesPerSecond);
		local remaining = "";
		
		// Convert to seconds and minutes.
		if( secondsLeft > 3600 )
		{
			local hours = (secondsLeft / 3600).tointeger();
			local mins = ((secondsLeft - (hours * 3600)) / 60).tointeger();
			remaining = format("%d hour%s %d minute%s",
				hours, hours > 1 ? "s" : "",
				mins, mins > 1 ? "s" : "");
		}
		else if( secondsLeft > 60 )
		{
			local mins = (secondsLeft / 60).tointeger();
			local secs = secondsLeft - (mins * 60);
			remaining = format("%d minute%s %d second%s",
				mins, mins > 1 ? "s" : "",
				secs, secs > 1 ? "s" : "");
		}
		else
		{
			// +1 here to avoid showing 0 seconds left due to roundoff
			remaining = (secondsLeft.tointeger() + 1) + " seconds";
		}
		
		local w = Widget("Preloader/Comment");
		w.setParam("colour", ".9 .9 .9");
		w.setText(format("Progress: %3d%%, Time remaining: %s (@ %.1f KB/s)",
			pctDone.tointeger(), remaining, kBps));
		
		w = Widget("Preloader/ProgressBar");
		local width = w.getParent().getWidth();
		width = (width - 4) * pctDone / 100.0;
		w.setSize(width.tointeger(), w.getHeight());
		
		for( local i = 0; i < 20000; i++ )
		{
		}
	}
	
	// Something to animate and keep things snappy.
	function onEnterFrame()
	{
		mPulseValue += 0.001 * _deltat;	

		local color = Color();
		// Throb between... [.6,.9]
		color.setHSB(0.0, 0.0, (sin(mPulseValue) * 0.3) + 0.6);
		
		local w = Widget("Preloader/Task");
		w.setParam("colour", format("%f %f %f", color.r, color.g, color.b));
	}
	
	// Re-center the loading panel on screen when the size changes.
	function onScreenResize()
	{
		// Center the panel.	
		CenterPanel();
	}
	
	
	function split( str, sep )
	{
		local result = [];
		local start = 0;
	
		while (true)
		{
			local end = str.find(sep, start);
	
			if (end == null)
			{
				result.append(str.slice(start));
				break;
			}
	
			result.append(str.slice(start, end));
			start = end + sep.len();
		}
	
		return result;
	}
	
	function startsWith( str, beginstr ) {
		if (typeof str != "string")	{
			return false;
		}

		if (beginstr.len() == 0) {
			return true;
		}

		if (str.len() < beginstr.len()) {
			return false;
		}

		return str.slice(0, beginstr.len()) == beginstr;
	}
	
};

Preloader.CenterPanel();

Screen.setBackgroundColor(Color("000000"));
_cache.addListener(Preloader);
_root.addListener(Preloader);

Screen.setOverlayVisible("Preloader/Overlay", true);

local xargs = {};
foreach( k, v in _args )
{
	if(k == "src") {
	
		/* A hacky way to get parameters set via the URL. Using the 0.8.6 client,
		 * I cannot find a way to pass arguments. Greths documentation suggests something
		 * like <path>?<name>=value would work in 0.8.6, and /arg:<name>=value would work
		 * in EER+. 
		 *
		 * However this doesn't work for me. The first parameter and the base name 
		 * (EarthEternal.car) get mixed up. 
		 *
		 * I found instead a # can be used as a separate. Anything after this will get
		 * split (comma separated) in name value pairs (equal separated).
		 */
	
		local parts = Preloader.split(v, "#");
		if(parts.len() > 1) {
			_args[k] = parts[0];
			foreach(arg in Preloader.split(parts[1], ",")) {
				local nvp = Preloader.split(arg, "=");
				if(nvp.len() > 1) 
					xargs[nvp[0]] <- nvp[1];
				else  
					xargs[nvp[0]] <- true;
			}
		} 
	}
}	
local tmpCache = MediaCache("tmpcache", "http://localhost");
try {
	tmpCache.setCookie("xargs", serialize(xargs));
}
catch(e) {
	print("ICE! Failed to set cookie. " + e + "\n");
}
	

if( "src" in _args )
{
	// Right. Begin the download!
	_cache.fetch(_args["src"]);
}

