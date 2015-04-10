require("States/CharacterSelectionState");


/*  NOTE:
    The server's MAX_CHARACTER_SLOTS constant must be updated to match.
*/

if("CharacterSelectionState" in States)
{
	if("MAX_CHARACTERS" in States.CharacterSelectionState)
	{
		States.CharacterSelectionState["MAX_CHARACTERS"] <- 6;
		log.info("[MOD] Set MAX_CHARACTERS");
	}

	if("mMaxCharacters" in States.CharacterSelectionState)
	{
		States.CharacterSelectionState["mMaxCharacters"] <- 6;
		log.info("[MOD] Set mMaxCharacters");
	}
}
