::Environments.GrunesUndead <- {
	TimeOfDay = {
		Sunrise = "GrunesUndeadDay",
		Day = "GrunesUndeadDay",
		Sunset = "GrunesUndeadNight",
		Night = "GrunesUndeadNight"
	}
};
::Environments.GrunesUndeadDay <- {
	Sun = {
		r = 0.50000001,
		g = 0.30000001,
		b = 0.50000001
	},
	Ambient = {
		r = 0.25,
		g = 0.25,
		b = 0.25
	},
	Sky = [
		"DarkClouds"
	],
	Fog = {
		color = {
			r = 0.1,
			g = 0.050000001,
			b = 0.1
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Spooky",
			0.2
		],
		[
			"Drone",
			0.80000001
		],
		[
			"Forest",
			0.0
		],
		[
			"Music",
			0.30000001
		]
	],
	Ambient_Music = [
		"Music-GloomHorizon.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-GrunesDrone.ogg"
	],
	Activate_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
::Environments.GrunesUndeadNight <- {
	Sun = {
		r = 0.30000001,
		g = 0.30000001,
		b = 0.30000001
	},
	Ambient = {
		r = 0.25,
		g = 0.25,
		b = 0.25
	},
	Sky = [
		"DarkClouds"
	],
	Fog = {
		color = {
			r = 0.1,
			g = 0.050000001,
			b = 0.1
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Spooky",
			0.2
		],
		[
			"Drone",
			0.80000001
		],
		[
			"Forest",
			0.0
		],
		[
			"Music",
			0.30000001
		]
	],
	Ambient_Music = [
		"Music-GloomHorizon.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-GrunesDrone.ogg"
	],
	Activate_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-GrunesLife.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
