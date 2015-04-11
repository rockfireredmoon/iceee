#include "Globals.h"
#include "FileReader3.h"
#include "StringList.h"

//Shared between the servers
const int SleepDelayError = 5000;
const int SleepDelayNormal = 15;

const int g_MainThreadID = 1;   //Application-defined thread ID for the main thread.
int g_GlobalThreadID = g_MainThreadID + 1;  //Additional thread IDs start here (HTTP, router, simulator, etc)

const char * StatusPhaseStrings[] = {
	"None",
	"Init",
	"Wait",
	"Ready",
	"Restart",
	"Kick"
};

//Data returned on a query request for "clan.info"
char g_ClanName[] = "Forging Ahead";
char g_ClanMOTD[] = "Recreating EE, one byte at a time.";
char g_ClanLeader[] = "true";

//Data returned on a query request for "clan.list"
int g_ClanMemberCount = 0;                     //Number of clan characters
char *g_ClanMemberList[3][5] = { 
	//Name, level, profession ("0"), online ("true"), rank
	"This Guy",     "100",  "1",  "true", "Leader",
	"That Guy",      "50",  "4",  "true", "Officer",
	"Annoying Guy",  "10",  "2", "false", "Minion"
	//"Cool Guy",      "40",  "1", "false", "Others"
};

float HeroismHealthDivisor = 10000.0F;  //Divide heroism by this to get the bonus health percent

int DamageReductionModiferPerLevel = 100;
int DamageReductionAdditive = 0;
float RegenConMod = 0.3F;
float RegenSpiritMod = 0.15F;
float RegenLevelMod = 0.2F;
float RegenCombatModifier = 0.2F;
float SecondsPerRoundInverse = 0.5F;
int HealthConModifier = 11;
float DodgeDexMod = 0.25F;
float ParryDexMod = 0.015F;
float ParryStrMod = 0.010F;
float ParryMod = 2.0F;
float DeflectPsycheMod = 0.25F;
float BlockDexterityMod = 0.010F;
float BlockStrengthMod = 0.015F;
float BlockMod = 2.0F;
float CritMeleeDexterityMod = 0.01F;
float CritMagicPsycheMod = 0.01F;
float ToHitMeleeDexMod = 0.05F;
float ToHitMagicPsycheMod = 0.05F;

//Facing a target, the maximum radian angle between current rotation and target must
//not exceed this.
//0.785398163      90 degree cone 
//1.570796326      180 degree cone
double g_FacingTarget = 1.570796326;

int g_CreatureListenRange = 1200;  //750;   //Only send creature updates if distance is under this


//See the MAX_LEVEL macro
//const int MaxLevel = 70;   //If MaxLevel changes, the extern in Globals.h must be updated too.
const int LevelRequirements[71][2] = {
        0, 0,  //0
        500, 500,  //1
        700, 1200,  //2
        1100, 2300,  //3
        1700, 4000,  //4
        2500, 6500,  //5
        3700, 10200,  //6
        5500, 15700,  //7
        8200, 23900,  //8
        12100, 36000,  //9
        13200, 49200,  //10
        14400, 63600,  //11
        15700, 79300,  //12
        17200, 96500,  //13
        18700, 115200,  //14
        20400, 135600,  //15
        22300, 157900,  //16
        24400, 182300,  //17
        26600, 208900,  //18
        29000, 237900,  //19
        31700, 269600,  //20
        34600, 304200,  //21
        37700, 341900,  //22
        41200, 383100,  //23
        44900, 428000,  //24
        49000, 477000,  //25
        53500, 530500,  //26
        58400, 588900,  //27
        63700, 652600,  //28
        69600, 722200,  //29
        75900, 798100,  //30
        82900, 881000,  //31
        90400, 971400,  //32
        98700, 1070100,  //33
        107700, 1177800,  //34
        117600, 1295400,  //35
        128300, 1423700,  //36
        140000, 1563700,  //37
        152800, 1716500,  //38
        166800, 1883300,  //39
        182100, 2065400,  //40
        198700, 2264100,  //41
        216900, 2481000,  //42
        236700, 2717700,  //43
        258300, 2976000,  //44
        281900, 3257900,  //45
        307700, 3565600,  //46
        335800, 3901400,  //47
        366500, 4267900,  //48
        400000, 4667900,  //49
        404500, 5072400,  //50
        409000, 5481400,  //51
        413600, 5895000,  //52
        418300, 6313300,  //53
        422900, 6736200,  //54
        427700, 7163900,  //55
        432500, 7596400,  //56
        437300, 8033700,  //57
        442300, 8476000,  //58
        447200, 8923200,  //59
        452200, 9375400,  //60
        457300, 9832700,  //61
        462400, 10295100,  //62
        467600, 10762700,  //63
        472900, 11235600,  //64
        478200, 11713800,  //65
        483500, 12197300,  //66
        489000, 12686300,  //67
        494500, 13180800,  //68
        500000, 13680800,  //69
        504800, 14185600  //70
};

double g_VendorMarkup = 2.5;  //Note: this is hardcoded into the client.

namespace Debug
{
	int LastAbility = 0;
	int CreatureDefID = 0;
	char LastName[64] = {0};
	void *LastPlayer = 0;
	int IsPlayer = 0;

	int LastTileZone = 0;
	int LastTileX = 0;
	int LastTileY = 0;
	int LastTilePropID = 0;
	void* LastTilePtr = 0;
	void* LastTilePackage = 0;

	int LastFlushSimulatorID = 0;

	CreatureInstance *ActivateAbility_cInst = 0;
	short ActivateAbility_ability = 0;
	int ActivateAbility_ActionType = 0;
	int ActivateAbility_abTargetCount = 0;
	CreatureInstance *ActivateAbility_abTargetList[8] = {0};
	int LastSimulatorID = 0;
}

namespace Global
{
	const int DazeGuaranteeTime = 1000;
	const int MAX_SEND_CHUNK_SIZE = 1000;  //Some operations that write multiple data to the buffer before sending, should not exceed this amount whenever possible.

	const int MAX_HEROISM = 1000;
	const int MAX_BASE_LUCK = 500;

	IntArray<71, 3> ResCostTable;   //Row for each level 0-70.  Columns: [0]=Level, [1]=Option2, [2]=Option3

	const int MIN_LEVEL = 1;
	const int MAX_LEVEL = 70;
	const int AbilityPointTable[71][3] = {
		{0, 0, 0},
		{1, 2, 2},
		{2, 2, 4},
		{3, 2, 6},
		{4, 2, 8},
		{5, 2, 10},
		{6, 2, 12},
		{7, 2, 14},
		{8, 2, 16},
		{9, 2, 18},
		{10, 2, 20},
		{11, 2, 22},
		{12, 2, 24},
		{13, 2, 26},
		{14, 2, 28},
		{15, 2, 30},
		{16, 2, 32},
		{17, 2, 34},
		{18, 2, 36},
		{19, 2, 38},
		{20, 2, 40},
		{21, 2, 42},
		{22, 2, 44},
		{23, 2, 46},
		{24, 2, 48},
		{25, 2, 50},
		{26, 2, 52},
		{27, 2, 54},
		{28, 2, 56},
		{29, 2, 58},
		{30, 4, 62},
		{31, 2, 64},
		{32, 4, 68},
		{33, 2, 70},
		{34, 4, 74},
		{35, 2, 76},
		{36, 4, 80},
		{37, 2, 82},
		{38, 4, 86},
		{39, 2, 88},
		{40, 4, 92},
		{41, 2, 94},
		{42, 4, 98},
		{43, 2, 100},
		{44, 4, 104},
		{45, 2, 106},
		{46, 4, 110},
		{47, 2, 112},
		{48, 4, 116},
		{49, 2, 118},
		{50, 4, 122},
		{51, 2, 124},
		{52, 4, 128},
		{53, 2, 130},
		{54, 4, 134},
		{55, 2, 136},
		{56, 4, 140},
		{57, 2, 142},
		{58, 4, 146},
		{59, 2, 148},
		{60, 4, 152},
		{61, 2, 154},
		{62, 4, 158},
		{63, 2, 160},
		{64, 4, 164},
		{65, 2, 166},
		{66, 4, 170},
		{67, 2, 172},
		{68, 4, 176},
		{69, 2, 178},
		{70, 4, 182},
	};
	int GetAbilityPointsLevelIncrement(int level)
	{
		if(level < MIN_LEVEL || level > MAX_LEVEL)
			return 0;
		return AbilityPointTable[level][1];
	}

	int GetAbilityPointsLevelCumulative(int level)
	{
		if(level < MIN_LEVEL || level > MAX_LEVEL)
			return 0;
		return AbilityPointTable[level][2];
	}

	void LoadResCostTable(const char *filename)
	{
		FileReader3 fr;
		if(fr.OpenFile(filename) != FileReader3::SUCCESS)
		{
			g_Log.AddMessageFormat("[ERROR] Cannot load file [%s]", filename);
			fr.CloseFile();
			return;
		}
		fr.SetCommentChar(';');
		fr.ReadLine();  //Assume first line is a header or comment.
		while(fr.Readable() == true)
		{
			fr.ReadLine();
			int r = fr.MultiBreak("\t");
			if(r >= 3)
			{
				int level = fr.BlockToIntC(0);
				ResCostTable.SetValue(level, 0, level);
				ResCostTable.SetValue(level, 1, fr.BlockToIntC(1));
				ResCostTable.SetValue(level, 2, fr.BlockToIntC(2));
			}
		}
		fr.CloseFile();
	}

	int GetResurrectCost(int playerLevel, int resChoice)
	{
		if(playerLevel < MIN_LEVEL)
			playerLevel = 1;
		else if(playerLevel > MAX_LEVEL)
			playerLevel = MAX_LEVEL;

		int cost = 0;
		switch(resChoice)
		{
		case 0: cost = 0; break;
		case 1: cost = ResCostTable.GetValue(playerLevel, 1); break;
		case 2: cost = ResCostTable.GetValue(playerLevel, 2); break;
		}
		if(cost < 0)
			cost = 0;
		return cost;

		/*
		float divisor = 1;
		switch(resChoice)
		{
		case 0: return 0;
		case 1: divisor = 7.5F; break;
		case 2: divisor = 2.5F; break;
		}
		return static_cast<int>(static_cast<float>(playerLevel * playerLevel * playerLevel) / divisor);
		*/
	}
};
