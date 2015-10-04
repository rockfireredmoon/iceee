#pragma warning(disable: 4996)

#include "Preferences.h"

#include <string.h>

PreferenceDef PreferenceList[] = {  //was 31
	//pref.get                         //Character preferences
	{ 0, "chatwindow.windowSize",        PT_String, "\"Small\"" },   //Preference format confirmed to work when enclosed in quotation marks
	{ 0, "chatwindow.color",             PT_Block,  "" },

	{ 0, "quest.CurrentSelectedQuest",   PT_Short,  "" },

	//0, "quest.QuestMarkerType",        PT_Block, "{[0] = { questId = -1, isSelected = false },[1] = { questId = -1, isSelected = false }, [2] = { questId = -1, isSelected = false },[3] = { questId = -1, isSelected = false }, }",
	//0, "map.LegendItems",              PT_Block, "{[LegendItemTypes.DEFAULT] = false, [LegendItemTypes.YOU] = true, [LegendItemTypes.TOWN_GATE] = false, [LegendItemTypes.QUEST] = true, [LegendItemTypes.QUEST_GIVER] = false, [LegendItemTypes.HENGE] = false, [LegendItemTypes.SANCTUARY] = false, [LegendItemTypes.SHOP] = true, [LegendItemTypes.VAULT] = false, [LegendItemTypes.CITY] = false, [LegendItemTypes.PARTY] = true, [LegendItemTypes.ANIMAL] = true, [LegendItemTypes.DEMON] = true,[LegendItemTypes.DIVINE] = true, [LegendItemTypes.DRAGONKIN] = true,[LegendItemTypes.ELEMENTAL] = true,[LegendItemTypes.MAGICAL] = true,[LegendItemTypes.MORTAL] = true, [LegendItemTypes.UNLIVING] = true }",

	{ 0, "quest.QuestMarkerType",        PT_Block, "" },
	{ 0, "map.LegendItems",              PT_Block, "" },


	{ 0, "chat.BoldText",                PT_String, "true" },  //confirmed style
	{ 0, "chatwindow.chattabs",          PT_Block,  "[{[\"name\"]=\"General\",[\"filters\"]={[\"System\"]=true,[\"Private Channel\"]=true,[\"Clan\"]=true,[\"Say\"]=true,[\"Trade\"]=true,[\"Tell\"]=true,[\"Party\"]=true,[\"Region\"]=true,[\"Friend Notifications\"]=true,[\"Clan Officer\"]=true,[\"Emote\"]=true}}]" } ,
	{ 0, "minimap.ZoomScale",            PT_String, "1024" },  //confirmed style
	{ 0, "quickbar.8",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "map.MapType",                  PT_String, "\"NewBadari\"" }, //Confirmed style (when you zoom in the overworld map)
	
	{ 0, "quickbar.7",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "quickbar.6",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "quickbar.9",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "map.ZoomLevel",                PT_String, "" }, //"World",  //Default of "World" seems to break the pref display (no bold text checkmark)
	{ 0, "quickbar.1",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	
	{ 0, "quickbar.0",                   PT_String, "{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.863333,[\"x\"]=0.33625,[\"positionX\"]=84,[\"positionY\"]=26,[\"snapX\"]=null,[\"locked\"]=true,[\"visible\"]=true,[\"buttons\"]=[\"MACROid:0\",null,null,null,null,null,null,null]}" },
	{ 0, "quickbar.3",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "quickbar.2",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	{ 0, "quickbar.5",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },

	{ 0, "quickbar.4",                   PT_String, "\"{[\"slotsY\"]=1,[\"slotsX\"]=8,[\"snapY\"]=null,[\"y\"]=0.824138,[\"x\"]=0.501866,[\"positionX\"]=269,[\"positionY\"]=474,[\"snapX\"]=null,[\"locked\"]=false,[\"visible\"]=false,[\"buttons\"]=[null,null,null,null,null,null,null,null]}\"" },
	//pref.getA                        //Account preferences
	{ 0, "chat.ignoreList",              PT_Block, "" },
	{ 0, "chat.ProfanityFilter",         PT_String, "false" },  //confirmed style
	{ 0, "tutorial.active",              PT_Bool, "true" },
	{ 0, "tutorial.seen",                PT_Block, "\"[0,2,11,16,17]\"" },

	{ 0, "gameplay.mousemovement",       PT_Bool, "" },
	{ 0, "combat.AutoFaceTarget",        PT_Bool, "" },
	{ 0, "combat.AAToggle",              PT_Bool, "" },
	{ 0, "gameplay.eqcomparisons",       PT_String, "true" },   //confirmed style
	{ 0, "other.BindPopup",              PT_String, "true" },  //confirmed style

	{ 0, "control.Keybindings",          PT_String, "[]" },    //confirmed for empty set
};
const int MaxPref = sizeof(PreferenceList) / sizeof(PreferenceList[0]);


int GetPrefIndex(char *PrefName)
{
	//Searches for a preference name in the lookup table, returning an index into that
	//table.  Returns -1 if not found.
	for(size_t i = 0; i < MaxPref; i++)
		if(strcmp(PreferenceList[i].Name, PrefName) == 0)
			return i;

	return -1;
}
