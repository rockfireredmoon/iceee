::Environments.Blank <- {
	TimeOfDay = {
		Sunrise = "BlankAll",
		Day = "BlankAll",
		Sunset = "BlankAll",
		Night = "BlankAll"
	}
};
::Environments.BlankAll <- {
	Sun = {
		r = 0.0000001,
		g = 0.0000001,
		b = 0.0000001
	},
	Ambient = {
		r = 0.0,
		g = 0.0,
		b = 0.0
	}
};
