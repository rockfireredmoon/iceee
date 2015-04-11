this.FunctionBarTypes <- {
	EQUIPMENT = 1,
	INVENTORY = 2,
	ABILITY = 3,
	QUEST_LOG = 4,
	FRIEND = 5,
	CONFIG = 6,
	HELP = 7,
	CREDITMARKET = 8
};
this.FunctionBarOrder <- [
	this.FunctionBarTypes.EQUIPMENT,
	this.FunctionBarTypes.INVENTORY,
	this.FunctionBarTypes.ABILITY,
	this.FunctionBarTypes.QUEST_LOG,
	this.FunctionBarTypes.FRIEND,
	this.FunctionBarTypes.CONFIG,
	this.FunctionBarTypes.HELP,
	this.FunctionBarTypes.CREDITMARKET
];
this.FunctionBarItems <- {
	[this.FunctionBarTypes.EQUIPMENT] = {
		icon = "CharacterScreenButton",
		tooltip = "Character Sheet",
		screen = "Equipment"
	},
	[this.FunctionBarTypes.INVENTORY] = {
		icon = "InventoryScreenButton",
		tooltip = "Inventory",
		screen = "Inventory"
	},
	[this.FunctionBarTypes.ABILITY] = {
		icon = "AbilityScreenButton",
		tooltip = "Ability",
		screen = "AbilityFrame"
	},
	[this.FunctionBarTypes.QUEST_LOG] = {
		icon = "QuestLogScreenButton",
		tooltip = "Quest Journal",
		screen = "QuestJournal"
	},
	[this.FunctionBarTypes.FRIEND] = {
		icon = "FriendScreenButton",
		tooltip = "Contacts",
		screen = "SocialWindow"
	},
	[this.FunctionBarTypes.CONFIG] = {
		icon = "ConfigurationOptionsButton",
		tooltip = "Options",
		screen = "OptionsScreen"
	},
	[this.FunctionBarTypes.HELP] = {
		icon = "HelpAndSupportButton",
		tooltip = "Help",
		webpage = "http://www.eartheternal.com/knowledgebase"
	},
	[this.FunctionBarTypes.CREDITMARKET] = {
		icon = "SocialScreenButton",
		tooltip = "Buy Credits",
		webpage = "http://www.eartheternal.com/credits"
	}
};
class this.GUI.FunctionsBar extends this.GUI.Component
{
	static nClassName = "FunctionsBar";
	static ICON_SIZE = 28;
	static GAP_SIZE = 4;
	constructor()
	{
		this.GUI.Container.constructor(this.GUI.BoxLayout());
		this.getLayoutManager().setGap(this.GAP_SIZE);

		foreach( barData in this.FunctionBarOrder )
		{
			local button = this.GUI.ImageButton();
			button.setSize(this.ICON_SIZE, this.ICON_SIZE);
			button.setPreferredSize(this.ICON_SIZE, this.ICON_SIZE);
			button.setAppearance(this.FunctionBarItems[barData].icon);
			button.setGlowImageName("SelectGlowFunctionBar");
			button.setTooltip(this.FunctionBarItems[barData].tooltip);
			button.addActionListener(this);
			button.setPressMessage("_onToggleScreen");
			button.setData(barData);
			this.add(button);
		}

		this.setSize((this.ICON_SIZE + this.GAP_SIZE) * (this.FunctionBarOrder.len() + 1) - this.GAP_SIZE, this.ICON_SIZE);
		this.setPreferredSize((this.ICON_SIZE + this.GAP_SIZE) * (this.FunctionBarOrder.len() + 1) - this.GAP_SIZE, this.ICON_SIZE);
	}

	function _onToggleScreen( button )
	{
		if (button && button.getData() && button.getData() != "")
		{
			local key = button.getData();
			local data = this.FunctionBarItems[key];

			if ("webpage" in data)
			{
				this.System.openURL(data.webpage);
			}
			else if ("command" in data)
			{
				this.InputCommands.CheckCommand(data.command);
			}
			else if ("screen" in data)
			{
				this.Screens.toggle(data.screen);
			}
		}
	}

}

