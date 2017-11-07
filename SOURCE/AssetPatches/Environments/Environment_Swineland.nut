this.require("Environment_Default");
::Environments.Swineland <- {
	TimeOfDay = {
		Sunrise = "SwinelandSunrise",
		Day = "SwinelandDay",
		Sunset = "SwinelandSunset",
		Night = "SwinelandNight"
	}
};
::Environments.SwinelandSunrise <- delegate ::Environments.Sunrise : {
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
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
		[
			"Cloudy",
			0.3
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg",
	],
	Ambient_Music_Delay = 300
};
::Environments.SwinelandDay <- delegate ::Environments.CloudyDay : {
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
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
		[
			"Cloudy",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};
::Environments.SwinelandSunset <- delegate ::Environments.Sunset : {
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
	Adjust_Channels = [
		[
			"Desert",
			0.89999998
		],
		[
			"Cloudy",
			0.2
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 300
};
::Environments.SwinelandNight <- delegate ::Environments.NightSky : {
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
	Adjust_Channels = [
		[
			"Desert",
			0.30000001
		],
		[
			"Cloudy",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [
	],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Cloudyday.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};

::Environments.IceSwineland <- {
	TimeOfDay = {
		Sunrise = "IceSwinelandSunrise",
		Day = "IceSwinelandDay",
		Sunset = "IceSwinelandSunset",
		Night = "IceSwinelandNight"
	}
};
::Environments.IceSwinelandSunrise <- delegate ::Environments.Sunrise : {
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
	Fog = {
		color = {
			r = 0.2,
			g = 0.2,
			b = 0.2
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.3
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Icewind.ogg"
	],
	Ambient_Music_Delay = 300
};
::Environments.IceSwinelandDay <- delegate ::Environments.CloudyDay : {
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
	Fog = {
		color = {
			r = 0.3,
			g = 0.3,
			b = 0.3
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Icewind.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};
::Environments.IceSwinelandSunset <- delegate ::Environments.Sunset : {
	Sun = {
		r = 0.30000001,
		g = 0.20000001,
		b = 0.20000001
	},
	Fog = {
		color = {
			r = 0.6,
			g = 0.5,
			b = 0.5
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Ambient = {
		r = 0.45,
		g = 0.35,
		b = 0.35
	},
	Adjust_Channels = [
		[
			"Wind",
			0.2
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-Icewind.ogg"
	],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 300
};
::Environments.IceSwinelandNight <- delegate ::Environments.NightSky : {
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
	Fog = {
		color = {
			r = 0.1,
			g = 0.1,
			b = 0.1
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [
	],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-Icewind.ogg"	
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};

::Environments.RadiationSwineland <- {
	TimeOfDay = {
		Sunrise = "RadiationSwinelandSunrise",
		Day = "RadiationSwinelandDay",
		Sunset = "RadiationSwinelandSunset",
		Night = "RadiationSwinelandNight"
	}
};
::Environments.RadiationSwinelandSunrise <- delegate ::Environments.Sunrise : {
	Sun = {
		r = 0.8,
		g = 0.8,
		b = 0.3
	},
	Ambient = {
		r = 0.3,
		g = 0.43000001,
		b = 0.20000001
	},
	Fog = {
		color = {
			r = 0.0,
			g = 0.3,
			b = 0.0
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.3
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-RadiationDrone.ogg"
	],
	Ambient_Music_Delay = 300
};
::Environments.RadiationSwinelandDay <- delegate ::Environments.CloudyDay : {
	Sun = {
		r = 0.84999999,
		g = 0.7,
		b = 0.7
	},
	Ambient = {
		r = 0.39999999,
		g = 0.4999999,
		b = 0.39999999
	},
	Fog = {
		color = {
			r = 0.0,
			g = 0.3,
			b = 0.0
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-RadiationDrone.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};
::Environments.RadiationSwinelandSunset <- delegate ::Environments.Sunset : {
	Sun = {
		r = 0.20000001,
		g = 0.30000001,
		b = 0.20000001
	},
	Fog = {
		color = {
			r = 0.0,
			g = 0.3,
			b = 0.0
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Ambient = {
		r = 0.45,
		g = 0.35,
		b = 0.35
	},
	Adjust_Channels = [
		[
			"Wind",
			0.2
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [],
	Ambient_Music = [],
	Ambient_Noise = [
		"Sound-Ambient-RadiationDrone.ogg"
	],
	Ambient_Sound = [
	],
	Ambient_Music_Delay = 300
};
::Environments.RadiationSwinelandNight <- delegate ::Environments.NightSky : {
	Sun = {
		r = 0.30000001,
		g = 0.50000001,
		b = 0.30000001
	},
	Ambient = {
		r = 0.25,
		g = 0.35,
		b = 0.25
	},
	Fog = {
		color = {
			r = 0.0,
			g = 0.3,
			b = 0.0
		},
		exp = 0.001,
		start = 0.30000001,
		end = 0.89999998
	},
	Adjust_Channels = [
		[
			"Wind",
			0.4
		],
		[
			"Music",
			0.3
		]
	],
	Activate_Music = [
	],
	Ambient_Music = [
		"Music-CrossingTheChasm.ogg"
	],
	Ambient_Noise = [
		"Sound-Ambient-RadiationDrone.ogg"
	],
	Ambient_Sound = [],
	Ambient_Music_Delay = 300
};

