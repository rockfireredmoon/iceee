::Environments.NewCorsica <- {
	TimeOfDay = {
		Sunrise = "NewCorsicaDay",
		Day = "NewCorsicaDay",
		Sunset = "NewCorsicaDay",
		Night = "NewCorsicaDay"
	}
};
::Environments.NewCorsicaDay <- {
	Sun = {
		r = 0.80000001,
		g = 0.40000001,
		b = 0.00000001
	},
	Ambient = {
		r = 0.8,
		g = 0.3,
		b = 0.3
	},
	Sky = [
		"BastionSky"
		"WhiteClouds"
		"SwirlingClouds"
	],
	Fog = {
		color = {
			r = 0.1,
			g = 0.050000001,
			b = 0.050000001,
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.840000001
	},
	Adjust_Channels = [
		[
			"Bastion",
			0.4499999
		],
		[
			"Bastionwind",
			0.2
		],
		[
			"Songs",
			0.0
		],
		[
			"Music",
			0.230001
		]
	],
	Ambient_Music = [
		"Music-Northbeach.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Bastion.ogg"
	],
	Activate_Music = [
		"Music-Northbeach.ogg"
	],
	Ambient_Sound = [
		"Sound-Ambient-Newbwind.ogg"
	],
	Ambient_Music_Delay = 180,
	Activate_Music_Cooldown = 300.0,
	Blend_Time = 2.5
};
