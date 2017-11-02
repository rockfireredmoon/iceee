
::Environments.Grimfrost <- {
	TimeOfDay = {
		Sunrise = "GrimfrostDescent",
		Day = "GrimfrostDescent",
		Sunset = "GrimfrostFight",
		Night = "GrimfrostFinished"
	}
};

::Environments.GrimfrostDescent <- {
	Sun = {
		r = 0.44999999,
		g = 0.44999999,
		b = 0.44999999
	},
	Ambient = {
		r = 0.2,
		g = 0.2,
		b = 0.30000001
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.1,
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
		],
		[
			"Dragon",
			0.40000001
		]
	],
	Ambient_Music = [
		"Music-DragonAndToast.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Dragon.ogg"
	],
	Activate_Music = [
		"Music-DragonAndToast.ogg"
	],
	Ambient_Music_Delay = 60
};

::Environments.GrimfrostFight <- {
	Sun = {
		r = 0.40000001,
		g = 0.40000001,
		b = 0.40000001
	},
	Ambient = {
		r = 0.25000001,
		g = 0.35000001,
		b = 0.4
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.2,
			b = 0.4
		},
		exp = 0.002,
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
	],
	Ambient_Noise = [
	],
	Activate_Music = [
		"Music-GrimIdol.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 0
};


::Environments.GrimfrostFinished <- {
	Sun = {
		r = 0.40000001,
		g = 0.40000001,
		b = 0.40000001
	},
	Ambient = {
		r = 0.25000001,
		g = 0.35000001,
		b = 0.4
	},
	Sky = [],
	Fog = {
		color = {
			r = 0.2,
			g = 0.2,
			b = 0.4
		},
		exp = 0.002,
		start = 0.050000001,
		end = 0.40000001
	},
	Adjust_Channels = [
		[
			"Music",
			0.50000001
		],
		[
			"Drips",
			0.40000001
		]
	],
	Ambient_Music = [
	],
	Ambient_Noise = [
		"Sound-Ambient-Drips.ogg"
	],
	Activate_Music = [
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 0
};
