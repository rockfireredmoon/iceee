::Environments.Bloodkeep <- {
	TimeOfDay = {
		Sunrise = "BloodkeepFinale",
		Day = "BloodkeepMain",
		Sunset = "BloodkeepFight",
		Night = "BloodkeepDarkness"
	}
};
::Environments.BloodkeepMain <- {
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
::Environments.BloodkeepFight <- {
	Sun = {
		r = 0.64999999,
		g = 0.14999999,
		b = 0.14999999
	},
	Ambient = {
		r = 0.4,
		g = 0.1,
		b = 0.10000001
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.1,
			g = 0.0,
			b = 0.0
		},
		exp = 0.002,
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
			0.8
		]
	],
	Ambient_Music = [
		"Music-Killers.ogg"],
	Ambient_Noise = [],
	Activate_Music = ["Music-Killers.ogg"],
	Ambient_Sound = [],
	Ambient_Music_Delay = 0
};

::Environments.BloodkeepDarkness <- {
	Sun = {
		r = 0,
		g = 0,
		b = 0
	},
	Ambient = {
		r = 0,
		g = 0,
		b = 0
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.1,
			g = 0.0,
			b = 0.0
		},
		exp = 0.002,
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
			0.2
		]
	],
	Ambient_Music = [
	],
	Ambient_Noise = [],
	Activate_Music = [
	],
	Ambient_Sound = [
		"Sound-Ambient-BloodkeepBackground.ogg"
	],
	Ambient_Music_Delay = 300
};
::Environments.BloodkeepFinale <- {
	Sun = {
		r = 0.24999999,
		g = 0.24999999,
		b = 0.24999999
	},
	Ambient = {
		r = 0.4,
		g = 0.4,
		b = 0.4
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.05,
			g = 0.05,
			b = 0.05
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
	],
	Activate_Music = [
		"Music-Tap.ogg"
	],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 300
};
