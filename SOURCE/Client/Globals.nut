this.require("Math");
this.require("Util");
this.gServer <- {};
this.gServer.address <- [
	"127.0.0.1",
	"sv1.dev.eartheternal.com"
];
this.gServer.port <- [
	4242,
	4242
];
this.gServerScale <- 1.0;
this.gAvatar <- {};
this.gAvatar.headLight <- false;
this.gCamera <- {};
this.gCamera.height <- 12.0;
this.gCamera.initalZoom <- 60.0;
this.gCamera.minZoom <- 30.0;
this.gCamera.maxZoom <- 125.0;
this.gCamera.stepZoom <- 10.0;
this.gCamera.mouseWheelSensitivity <- 1.0;
this.gCamera.zoomSpeedMin <- 5.0;
this.gCamera.zoomSpeedMultiplayer <- 10.0;
this.gCamera.transparencyDistance <- 15.0;
this.gCamera.initialYaw <- 180.0;
this.gCamera.sensitivity <- 0.15000001;
this.gCamera.fov <- 55.0;
this.gLogEffects <- false;
this.gDefaultCreatureSpeed <- 50;
this.gCamera.farClippingDistance <- 2500.0;
this.gNameFarClip <- 350.0;
this.gNameNearHeight <- 1.5;
this.gNameFarHeight <- 8.1999998;
this.gHideFloatingNames <- true;
this.gMaxSlope <- 0.64999998;
this.gShadows <- false;
this.gShadowDistance <- 400.0;
this.gShadowPoolBaseSize <- 9.0;
this.gServerListenDistance <- 2500.0;
this.gCreatureVisibleRange <- 850.0;
this.gLodBlockSize <- this.gCreatureVisibleRange / 5.0;
this.gPreferenceCharacterUpdate <- false;
this.gPreSimWaiting <- false;
this.gServerCreaturePositionTolerance <- 120.0;
this.gMilisecPerSecond <- 1000;
this.gMilisecPerMinute <- this.gMilisecPerSecond * 60;
this.gMilisecPerHour <- this.gMilisecPerMinute * 60;
this.gMilisecPerDay <- this.gMilisecPerHour * 24;
this.gSecPerMinute <- 60;
this.gSecPerHour <- this.gSecPerMinute * 60;
this.gSecPerDay <- this.gSecPerHour * 24;
this.gCopperPerSilver <- 100;
this.gCopperPerGold <- this.gCopperPerSilver * 100;
this._DummyCount <- 0;
this.gAssertTesting <- true;
this.gShowDebugOverlay <- 1;
this.gTerrainRaiseAmount <- 5.0;
this.gBuildRotateSnap <- this.Math.PI / 4.0;
this.gBuildTranslateSnap <- 5.0;
this.Config <- {
	CURRENT = {}
};
this.gCorkTimeout <- 5000;
this.gPrepareResources <- "prepareResources" in this._root;
