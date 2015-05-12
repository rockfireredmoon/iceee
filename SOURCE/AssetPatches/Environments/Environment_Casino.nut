::Environments.Casino <- {
	TimeOfDay = {
		Sunrise = "CasinoAll",
		Day = "CasinoAll",
		Sunset = "CasinoAll",
		Night = "CasinoAll"
	}
};
::Environments.CasinoAll <- {
        Adjust_Channels = [
                [
                        "Taverncrowds",
                        0.40000001
                ],
                [
                        "Tavern",
                        0.80000001
                ],
                [
                        "Music",
                        0.30000001
                ]
        ],
        Activate_Music = [
                "Music-Hyperfun.ogg"
        ],
        Ambient_Music = [],
        Ambient_Noise = [
                "Sound-Ambient-Taverncrowds.ogg"
        ],
        Ambient_Sound = [
                "Sound-Ambient-Citytavern.ogg"
        ],
        Ambient_Music_Delay = 40
};
