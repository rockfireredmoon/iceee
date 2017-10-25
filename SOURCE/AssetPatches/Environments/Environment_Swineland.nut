this.require("Environment_Default");
::Environments.Swineland <- {
	TimeOfDay = {
		Sunrise = "SwinelandSunrise",
		Day = "SwinelandDay",
		Sunset = "SwinelandSunset",
		Night = "SwinelandNight"
	}
};
::Environments.SwinelandSunrise <- delegate ::Environments.Sunrise : {
	Sun = {
		r = 0.8,
		g = 0.8,
		b = 0.3
	},
	Ambient = {
		r = 0.4,
		g = 0.33000001,
		b = 0.20000001
	},
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
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
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-Desertloop.ogg"
	],
	Ambient_Music_Delay = 300
};
::Environments.SwinelandDay <- delegate ::Environments.CloudyDay : {
	Sun = {
		r = 0.84999999,
		g = 0.7,
		b = 0.7
	},
	Ambient = {
		r = 0.49999999,
		g = 0.3999999,
		b = 0.39999999
	},
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
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
		"Sound-Ambient-Desertloop.ogg"
	],
	Ambient_Music_Delay = 40
};
::Environments.SwinelandSunset <- delegate ::Environments.Sunset : {
	Sun = {
		r = 0.30000001,
		g = 0.20000001,
		b = 0.20000001
	},
	Ambient = {
		r = 0.45,
		g = 0.35,
		b = 0.35
	},
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
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
		"Sound-Ambient-Desertloop.ogg"
	],
	Ambient_Music_Delay = 40
};
::Environments.SwinelandNight <- delegate ::Environments.NightSky : {
	Sun = {
		r = 0.50000001,
		g = 0.30000001,
		b = 0.30000001
	},
	Ambient = {
		r = 0.35,
		g = 0.25,
		b = 0.25
	},
	Adjust_Channels = [
		[
			"Desert",
			0.30000001
		],
		[
			"Crowds",
			0.0
		]
	],
	Activate_Music = [],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Desertnightloop.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};
