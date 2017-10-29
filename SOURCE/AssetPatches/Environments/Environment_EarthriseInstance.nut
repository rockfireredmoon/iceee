::Environments.EarthriseInstance <- {
	Sun = {
		r = 0.64999998,
		g = 0.64999998,
		b = 0.64999998
	},
	Ambient = {
		r = 0.55000001,
		g = 0.44999999,
		b = 0.44999999
	},
	Sky = [
		"SunriseGradient",
		"WhiteClouds"
	],
	Fog = {
		color = {
			r = 0.2,
			g = 0.2,
			b = 0.2
		},
		exp = 0.001,
		start = 0.15000001,
		end = 0.5
	},
	Adjust_Channels = [
		[
			"Firesynth",
			0.80000001
		],
		[
			"Firewind",
			0.2
		],
		[
			"Ambient",
			0.5
		]
	],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Firewind.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-Firesynth.ogg"
	]
};
