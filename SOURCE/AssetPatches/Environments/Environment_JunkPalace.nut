
::Environments.JunkPalace <- {
	TimeOfDay = {
		Sunrise = "JunkPalaceNormal",
		Day = "JunkPalaceNormal",
		Sunset = "JunkPalaceNormal",
		Night = "JunkPalaceNormal"
	}
};

::Environments.JunkPalaceNormal <- {
	Sun = {
		r = 1.0,
		g = 0.60000002,
		b = 0.5
	},
	Ambient = {
		r = 0.55,
		g = 0.45,
		b = 0.35
	},
	Fog = {
		color = {
			r = 0.45999998,
			g = 0.152999997,
			b = 0.050000001
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.94999999
	},
	Adjust_Channels = [
		[
			"Music",
			0.50000001
		],
		[
			"Drone",
			0.50000001
		],
		[
			"Dungeon",
			0.1
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-ElectricityFan.ogg"
	],
	Ambient_Music = [
	],
	Ambient_Noise = [
		"Sound-Ambient-ElectricityFan.ogg"
	],
	Activate_Music = [],
	Ambient_Sound = [
		"Sound-Ambient-Drill.ogg",
		"Sound-Ambient-MetalHammer.ogg",
		"Sound-Ambient-LongSteam.ogg"
	],
	Ambient_Music_Delay = 180,
	Blend_Time = 2.5
};
