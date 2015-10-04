#pragma once

#ifndef PREFERENCES_H
#define PREFERENCES_H

//tutorial.seen
//Contains a list of integer values of the tutorials that have been seen:
// 0 = Basic Movement
// 2 = Quest Indicators
// 11 = Heroism and Luck
// 13 = Binding (?)
// 16 = Henge to Henge travel
// 17 = Mobs
// 23 = Charms
// 27 = Credit shop

enum PrefSaveEnum
{
	PT_String = 0,    //Literal string, enclose in quotation
	PT_Block,         //Block element, contains braces and brackets.  Should be written as is, not enclosed in quotes.
	PT_Bool,          //For values returning "true" or "false"
	PT_Short,         //Standard numeric value
	PT_Integer,       //Standard numeric value
	PT_Float,         //Floating point
};

struct PreferenceDef
{
	short index;      //The index is resolved at load time, to make it easier when adding or removing preferences.
	const char *Name;       //Preference name.  Will be enclosed in quotation marks when written.
	char TypeSave;    //The type of data to save.  Ideally should correspond to their internal types, but for now determines how the output string is formated, whether to enclose in quotes or not.
	const char *Default;    //The default value that is saved to the string.
};

extern const int MaxPref;
extern PreferenceDef PreferenceList[];

int GetPrefIndex(char *PrefName);

#endif //PREFERENCES_H