this.require("Environment_Default");
::Environments.Camelot <- {
	TimeOfDay = {
		Sunrise = "CamelotSunrise",
		Day = "CamelotDay",
		Sunset = "CamelotSunset",
		Night = "CamelotNight"
	}
};
::Environments.CamelotSunrise <- delegate ::Environments.Sunrise : {
	Adjust_Channels = [
		[
			"Camelot",
			0.30000001
		],
		[
			"Citycrowds",
			0.1
		],
		[
			"Forest",
			0.0
		],
		[
			"Music",
			0.40000001
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-Distantcrowds.ogg"
	],
	Ambient_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-Camelot.ogg"
	],
	Activate_Music = [],
	Ambient_Music_Delay = 60,
	Activate_Music_Cooldown = 300.0
};
::Environments.CamelotDay <- delegate ::Environments.CloudyDay : {
	Adjust_Channels = [
		[
			"Camelot",
			0.30000001
		],
		[
			"Citycrowds",
			0.1
		],
		[
			"Forest",
			0.0
		],
		[
			"Music",
			0.40000001
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-Distantcrowds.ogg"
	],
	Ambient_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-Camelot.ogg"
	],
	Activate_Music = [],
	Ambient_Music_Delay = 60,
	Activate_Music_Cooldown = 300.0
};
::Environments.CamelotSunset <- delegate ::Environments.Sunset : {
	Adjust_Channels = [
		[
			"Camelot",
			0.30000001
		],
		[
			"Citycrowds",
			0.1
		],
		[
			"Forest",
			0.0
		],
		[
			"Music",
			0.40000001
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-Distantcrowds.ogg"
	],
	Ambient_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-Camelot.ogg"
	],
	Activate_Music = [],
	Ambient_Music_Delay = 60,
	Activate_Music_Cooldown = 300.0
};
::Environments.CamelotNight <- delegate ::Environments.NightSky : {
	Adjust_Channels = [
		[
			"Night",
			0.02
		],
		[
			"Crowds",
			0.0
		],
		[
			"Camelot",
			0.2
		],
		[
			"Songs",
			0.0
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-Townnight.ogg"
	],
	Ambient_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-Camelot.ogg"
	],
	Activate_Music = [
		"Music-Nightstars.ogg"
	],
	Ambient_Music_Delay = 60
};

::Environments.CamelotCourt <- delegate ::Environments.Sunrise : {
	Adjust_Channels = [
		[
			"Music",
			0.40000001
		]
	],
	Activate_Music = [
		"Music-TheRule.ogg"
	],
	Ambient_Music = [],
	Ambient_Noise = [],
	Ambient_Sound = [],
	Ambient_Music_Delay = 0
};