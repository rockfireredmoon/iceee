this.require("Environment_Default");
::Environments.HappyWinter <- {
	TimeOfDay = {
		Sunrise = "HappyWinterSunrise",
		Day = "HappyWinterDay",
		Sunset = "HappyWinterSunset",
		Night = "HappyWinterNight"
	}
};
::Environments.HappyWinterSunrise <- delegate ::Environments.Sunrise : {
	Adjust_Channels = [
		[
			"Cloudy",
			0.0
		],
		[
			"Music",
			0.250000001
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 40
};
::Environments.HappyWinterDay <- delegate ::Environments.CloudyDay : {
	Adjust_Channels = [
		[
			"Cloudy",
			0.0
		],
		[
			"Music",
			0.40000001
		]
	],
	Activate_Music = [
		"Music-ChristmasRap.ogg"
	],
	Ambient_Music = [
		"Music-ChristmasRap.ogg"
	],
	Ambient_Noise = [],
	Ambient_Sound = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Music_Delay = 300
};
::Environments.HappyWinterSunset <- delegate ::Environments.Sunset : {
	Adjust_Channels = [
		[
			"Cloudy",
			0.0
		],
		[
			"Music",
			0.40000001
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 40
};
::Environments.HappyWinterNight <- delegate ::Environments.NightSky : {
	Adjust_Channels = [
		[
			"Crowds",
			0.0
		],
		[
			"Music",
			0.25000001
		]
	],
	Activate_Music = [],
	Ambient_Music = [
		"Music-WishBackground.ogg"
	],
	Ambient_Noise = [
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};
