
::Environments.SkrillQueenLair <- {
	TimeOfDay = {
		Sunrise = "SkrillQueenLairNormal",
		Day = "SkrillQueenLairNormal",
		Sunset = "SkrillQueenLairFight",
		Night = "SkrillQueenLairFinished"
	}
};

::Environments.SkrillQueenLairNormal <- {
	Sun = {
		r = 0.44999999,
		g = 0.44999999,
		b = 0.44999999
	},
	Ambient = {
		r = 0.3,
		g = 0.2,
		b = 0.30000001
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.1,
			b = 0.2
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Music",
			0.30000001
		],
		[
			"Drone",
			0.30000001
		]
	],
	Ambient_Music = [
		"Music-TheHive.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Hive.ogg"
	],
	Activate_Music = [
		"Music-TheHive.ogg"
	],
	Ambient_Music_Delay = 180
};


::Environments.SkrillQueenLairFight <- {
	Sun = {
		r = 0.44999999,
		g = 0.44999999,
		b = 0.44999999
	},
	Ambient = {
		r = 0.5,
		g = 0.4,
		b = 0.5
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.1,
			b = 0.2
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Music",
			0.50000001
		]
	],
	Ambient_Music = [
		"Music-Constance.ogg"
	],
	Activate_Music = [
		"Music-Constance.ogg"
	],
	Ambient_Music_Delay = 0
};
::Environments.SkrillQueenLairFinished <- {
	Sun = {
		r = 0.44999999,
		g = 0.44999999,
		b = 0.44999999
	},
	Ambient = {
		r = 0.5,
		g = 0.5,
		b = 0.50000001
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.1,
			b = 0.2
		},
		exp = 0.001,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Drone",
			0.40000001
		]
	],
	Ambient_Noise = [
		"Sound-Ambient-AlienHum.ogg"
	],
};

