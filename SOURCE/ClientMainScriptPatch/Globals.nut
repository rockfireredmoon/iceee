//All global variables need to be defined in the config.nut file.

require("Math");
require("Util");


LogLevel <- {
	LEVEL_ERROR = 1,
	LEVEL_WARN = 2,
	LEVEL_INFO = 3,
	LEVEL_DEBUG = 4,
	LEVEL_TRACE = 5
};


//Stores global variables related to the connection to the server.
gServer <- {};

//multiplue addresses to the server can be stored and each will
//be attempted in the order they appear
gServer.address <- [
	"127.0.0.1",
	"213.138.112.253"
];
gServer.port <- [
	4242,
	4242
];

// The logging level
gLogLevel <- LogLevel.LEVEL_WARN;

gServerScale <- 1.0;

//Avatar related variables
gAvatar <- {};

// adds an omni light over the head of the avatar lighting up the area around them.
gAvatar.headLight <- false;

//CAMERA
gCamera <- {};

// Controls the height the camera looks at when orbiting a parent node.
// It is relative to the pivot point of the parent and shifts the camera's target
// by the number of units indicated.

//Height of where the camera is looking relative to the avatar's feet
gCamera.height <- 12.0;
gCamera.initalZoom <- 60.0;

//The minimum camera can zoom in to the character
gCamera.minZoom <- 30.0;
//The maximum the camera can zoom out from the character
gCamera.maxZoom <- 125.0;
//The amounst the camera zooms out when pressing the page up or down keys
gCamera.stepZoom <- 10.0;
//the amount the camera zooms in or out with each step of the mouse wheel.
gCamera.mouseWheelSensitivity <- 1.0;
//the minimum speed of the zoom change
gCamera.zoomSpeedMin <- 5.0;
//the zoom speed multiplyer
gCamera.zoomSpeedMultiplayer <- 10.0;
//Transparency Distance
gCamera.transparencyDistance <- 15.0;
// Initial yaw
gCamera.initialYaw <- 180.0;
// Camera mouse sensitivity
gCamera.sensitivity <- 0.15000001;
// Camera FOVy setting (in degrees).
gCamera.fov <- 55.0;
gLogEffects <- false;

// Number of units per second that 100% movement should go
gDefaultCreatureSpeed <- 50;

//Clipping Distance
gCamera.farClippingDistance <- 2500.0;

// Distance at which names disappear
gNameFarClip <- 350.0;

// When names are the "closest", they are this big (world units).
// TODO: These should instead be specified in screen relative sizes
// and the necessary world size(s) should be figured out using the
// camera projection info. 
gNameNearHeight <- 1.5;

// When names are the farthest away, they are this large in world units.
gNameFarHeight <- 8.1999998;

// Hides names above peoples heads
gHideFloatingNames <- true;

// The maximum slope the player can climb
gMaxSlope <- 0.65; //7; NJS: Moved this down a tiny bit to compensate 
				   // for floating point error not allowing you to get up the stairs in the camelot library.

//Shadows
gShadows <- false;
gShadowDistance <- 400.0;
gShadowPoolBaseSize <- 9.0; // Roughly the size of a character's shadow

//The distance at which point the server stops
//sending position updates, this is a reflection
//of the value stored on the server.  Changing this
//number will not alter the listening distance, just the
//distance at which point mobile objects disappear.
gServerListenDistance <- 2500.0;

// Controls when creatures are visible and fade in. Must be < gServerListenDistance
// or you'll obviously get weird stuff happening.
gCreatureVisibleRange <- 850.0;
gLodBlockSize <- gCreatureVisibleRange / 5.0;

//Stores when player character preferences have been updated
gPreferenceCharacterUpdate <- false;
gPreSimWaiting <- false;

// Distance tolerance for position updates received from the server.
// Anything closer than this and we'll "fake it".
// TODO: Should be somewhat closer to 50, but we've got ugly lag spikes atm...
gServerCreaturePositionTolerance <- 120.0;

gMilisecPerSecond <- 1000;
gMilisecPerMinute <- gMilisecPerSecond * 60;
gMilisecPerHour <- gMilisecPerMinute * 60;
gMilisecPerDay <- gMilisecPerHour * 24;

gSecPerMinute <- 60;
gSecPerHour <- gSecPerMinute * 60;
gSecPerDay <- gSecPerHour * 24;

gCopperPerSilver <- 100;
gCopperPerGold <- gCopperPerSilver * 100;

_DummyCount <- 0;

//ASSERT
gAssertTesting <- true;
gShowDebugOverlay <- 1;

/**
	Controls how much the terrain raises/lowers in a single brush step.
*/
gTerrainRaiseAmount <- 5.0;

gBuildRotateSnap <- Math.PI / 4.0;
gBuildTranslateSnap <- 5.0;

/**
	A namespace holding the user-settable configuration options. This also adds
	support for saving and loading from a local cookie.
*/
Config <- {
	/**
		The current configuration, as loaded/initialized by the saved
		settings. Changes should update this, and it's serialized to
		and from disk in order to maintain them between sessions.
	*/
	CURRENT = {}
};

/**
	Time out for sceneobject corking
*/
gCorkTimeout <- 5000;

gPrepareResources <- "prepareResources" in _root;