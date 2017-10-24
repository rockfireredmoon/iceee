::Environments.Swineland <- {
	TimeOfDay = {
		Sunrise = "SwinelandSunrise",
		Day = "SwinelandDay",
		Sunset = "SwinelandSunset",
		Night = "SwinelandNight"
	}
};
::Environments.SwinelandSunrise <- {
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
	Sky = [
		"SunriseGradient",
		"RedClouds"
	],
	Adjust_Channels = [
		[
			"Spooky",
			0.2
		],
		[
			"Music",
			0.30000001
		]
	],
	Fog = {
		color = {
			r = 1.0,
			g = 0.1,
			b = 0.1
		},
		exp = 0.006,
		start = 0.5,
		end = 0.94999999
	},
	Ambient_Music = [
		"Music-DarkWalk.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
::Environments.SwinelandDay <- {
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
	Sky = [
		"RedGradient",
		"RedClouds"
	],
	Adjust_Channels = [
		[
			"Spooky",
			0.4
		],
		[
			"Music",
			0.30000001
		]
	],
	Fog = {
		color = {
			r = 0.80000002,
			g = 0.69999999,
			b = 0.69999998
		},
		exp = 0.001,
		start = 0.40000001,
		end = 0.94999999
	},
	Ambient_Music = [
		"Music-DarkWalk.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
::Environments.SwinelandSunset <- {
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
	Sky = [
		"SunsetGradient",
		"RedClouds" 
	],
	Adjust_Channels = [
		[
			"Spooky",
			0.2
		]
	],
	Ambient_Music = [],
	Ambient_Noise = [],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 0
};
::Environments.SwinelandNight <- {
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
	Sky = [
		"RedClouds"
	],
	Adjust_Channels = [
		[
			"Spooky",
			0.4
		],
		[
			"Music",
			0.30000001
		]
	],
	Fog = {
		color = {
			r = 0.150000001,
			g = 0,
			b = 0
		},
		exp = 0.005,
		start = 0.050000001,
		end = 0.40000001
	},
	Activate_Music = [],
	Ambient_Music = [
		"Music-DarkWalk.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
