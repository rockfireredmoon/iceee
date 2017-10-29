::Environments.Default_Cave <- {
	Sun = {
		r = 0.40000001,
		g = 0.40000001,
		b = 0.40000001
	},
	Ambient = {
		r = 0.15000001,
		g = 0.15000001,
		b = 0.2
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.1,
			g = 0.1,
			b = 0.1
		},
		exp = 0.001,
		start = 0.2,
		end = 0.80000001
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
			0.60000002
		],
		[
			"Music",
			0.2
		]
	],
	Ambient_Music = [
		"Music-Cave.ogg",
		"Music-Dungeon_Ambient1.ogg",
		"music-Dungeon_Ambient2.ogg"
	],
	Ambient_Noise = [],
	Activate_Music = [],
	Ambient_Sound = [
		"Sound-Dungeon-Ambient1.ogg"
	],
	Ambient_Music_Delay = 180
};
::Environments.WarmCave <- {
	Sun = {
		r = 0.40000001,
		g = 0.40000001,
		b = 0.40000001
	},
	Ambient = {
		r = 0.15000001,
		g = 0.15000001,
		b = 0.2
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.1,
			b = 0.1
		},
		exp = 0.001,
		start = 0.2,
		end = 0.80000001
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
			0.60000002
		],
		[
			"Music",
			0.2
		]
	],
	Ambient_Music = [
		"Music-Cave.ogg",
		"Music-Dungeon_Ambient1.ogg",
		"Music-Dungeon_Ambient2.ogg"
	],
	Ambient_Noise = [],
	Activate_Music = [],
	Ambient_Sound = [
		"Sound-Dungeon-Ambient1.ogg"
	],
	Ambient_Music_Delay = 180
};
::Environments.LavaCave <- {
	Sun = {
		r = 0.40000001,
		g = 0.40000001,
		b = 0.40000001
	},
	Ambient = {
		r = 0.35000001,
		g = 0.35000001,
		b = 0.1
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.1,
			b = 0.1
		},
		exp = 0.005,
		start = 0.05,
		end = 0.80000001
	},
	Adjust_Channels = [
		[
			"Lava",
			0.250000002
		],
		[
			"Caves",
			0.1
		]
	],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-LavaBubbles.ogg"
	],
	Activate_Music = [],
	Ambient_Sound = [],
	Ambient_Music_Delay = 180
};

::Environments.ForestCavern <- {
	Sun = {
		r = 0.20000001,
		g = 0.40000001,
		b = 0.20000001
	},
	Ambient = {
		r = 0.35000001,
		g = 0.45000001,
		b = 0.3
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.1,
			g = 0.24,
			b = 0.1
		},
		exp = 0.005,
		start = 0.05,
		end = 0.80000001
	},
	Adjust_Channels = [
		[
			"Music",
			0.3
		],
		[
			"Shadows",
			0.2
		]
	],
	Ambient_Music = [
		"Music-NightCave.ogg"
	],
	Ambient_Noise = [],
	Activate_Music = [
		"Music-NightCave.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-InTheShadows.ogg"
	],
	Ambient_Music_Delay = 80
};
