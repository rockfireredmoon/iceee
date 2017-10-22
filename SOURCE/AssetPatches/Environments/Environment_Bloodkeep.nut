::Environments.Bloodkeep <- {
	Sun = {
		r = 0.24999999,
		g = 0.14999999,
		b = 0.14999999
	},
	Ambient = {
		r = 0.2,
		g = 0.1,
		b = 0.10000001
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.05,
			g = 0.0,
			b = 0.0
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Cloudy",
			0.0
		],
		[
			"Crowds",
			0.0
		],
		[
			"Dungeon",
			0.30000001
		],
		[
			"Music",
			1.0
		]
	],
	Ambient_Music = [
		"Music-SomeAmountOfEvil.ogg",
		"Music-LandOfPhantoms.ogg",
	],
	Activate_Music = [
		"Music-DarkTimes.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-Bloodkeep.ogg"
	],
	Ambient_Music_Delay = 300
};
