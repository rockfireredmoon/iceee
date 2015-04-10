
require( "GUI/GUI");
require("UI/Screens");

OptionType <-
{
	[1] = { name = "Help Reference", webpage = "Reference" },
	[2] = { name = "Options", screen = "VideoOptionsScreen"  },
	[3] = { name = "Mod Settings", screen = "ModSettings", hide=true },
	[4] = { name = "In-Game Forum", screen = "IGForum", hide=true },
	[5] = { name = "Item Preview", screen = "PreviewItem", hide=true },
	[6] = { name = "Grove Tools", screen = "GroveTools", hide=true },
	[7] = { name = "Alternate Mod Panel", screen = "ModPanel", hide=true },
	[8] = { name = "Unstick", command = "/unstick" },
	[9] = { name = "Logout", command = "/disconnect" },
}

class Screens.OptionsScreen extends GUI.Frame
{
	static mClassName = "OptionsScreen";
	
	static screenWidth = 200;
	static screenHeight = 345;
	
	constructor()
	{
		GUI.Frame.constructor("Options");
		setSize(screenWidth, screenHeight);
		setPreferredSize(screenWidth, screenHeight);
		
		local baseComp = GUI.Container(GUI.BoxLayoutV());
		baseComp.setInsets(5, 5, 5, 5);		
	
		foreach(key, data in ::OptionType)
		{
			local button = GUI.NarrowButton(data.name);
			button.setFixedSize(180, 32);
			button.setData(key);
			button.addActionListener(this);
			button.setReleaseMessage("_buttonPressed");
			
			baseComp.add(button);
		}
		
		setContentPane(baseComp);
		centerOnScreen();
	}
	
	function _buttonPressed(button)
	{
		local key = button.getData();
		local data = ::OptionType[key];

		if("hide" in data)
		{
			Screens.hide("OptionsScreen");
		}
		
		if( "webpage" in data)
		{
			_URLManager.LaunchURL(data.webpage);
		}
		else if( "command" in data)
		{
			InputCommands.CheckCommand(data.command);
		}
		else if ("screen" in data)
		{
			Screens.toggle(data.screen);	
		}
	}
	
}