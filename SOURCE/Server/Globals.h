#ifndef GLOBALS_H
#define GLOBALS_H

#include "IntArray.h"


#define MAXTARGET   8

//Shared between the servers
extern const int SleepDelayError;
extern const int SleepDelayNormal;

extern const int g_MainThreadID;
extern int g_GlobalThreadID;

extern const int DefaultPort;

enum StatusPhaseEnum
{
	Status_None = 0,  //Unused
	Status_Init,      //Need to create the socket.  CreateSocket()
	Status_Wait,      //Waiting for an incoming connection.  Accept()
	Status_Ready,     //Ready and waiting for data.  recv()
	Status_Restart,   //Full restart of the server.
	Status_Kick       //Disconnects the client only.
};

extern const char * StatusPhaseStrings[];



// TAKEN FROM SERVERVARS.H


enum CreatureUpdateEnum
{
	CREATURE_UPDATE_TYPE = 1,
	CREATURE_UPDATE_ZONE = 2,
	CREATURE_UPDATE_POSITION_INC = 4,
	CREATURE_UPDATE_VELOCITY = 8,
	CREATURE_UPDATE_ELEVATION = 16,
	CREATURE_UPDATE_STAT = 32,
	CREATURE_UPDATE_MOD = 64,
	CREATURE_UPDATE_COMBAT = 128,
	
	CREATURE_UPDATE_LOGIN_POSITION = 256  //From 0.8.8
};


enum SceneryUpdateEnum
{
	SCENERY_UPDATE_ASSET        = 1,
	SCENERY_UPDATE_LINKS        = 2,
	SCENERY_UPDATE_POSITION     = 4,
	SCENERY_UPDATE_ORIENTATION  = 8,
	SCENERY_UPDATE_SCALE        = 16,
	SCENERY_UPDATE_PROPERTIES   = 32,
	SCENERY_UPDATE_FLAGS        = 64
};

enum SceneryPropertyEnum
{
   	PROPERTY_INTEGER = 0,
	PROPERTY_FLOAT   = 1,
	PROPERTY_STRING  = 2,
	PROPERTY_SCENERY = 3,
	PROPERTY_NULL    = 4
};

enum CDefHintEnum
{
	CDEF_HINT_PERSONA           	= 1,
	CDEF_HINT_COPPER_SHOPKEEPER		= 2,
	CDEF_HINT_CREDIT_SHOPKEEPER     = 4,
	CDEF_HINT_ESSENCE_VENDOR		= 8,
	CDEF_HINT_QUEST_GIVER			= 16,
	CDEF_HINT_QUEST_ENDER			= 32,
	CDEF_HINT_CRAFTER				= 64,
	CDEF_HINT_CLANREGISTRAR			= 128,
	CDEF_HINT_VAULT					= 256,  //From 0.8.6+
	CDEF_HINT_CREDIT_SHOP			= 512,   //From 0.8.6+
	CDEF_HINT_USABLE				= 1024,  //From 0.8.6+ IceEE  //From 0.8.6+
	CDEF_HINT_USABLE_SPARKLY		= 2048,  //From 0.8.6+ IceEE
	CDEF_HINT_ITEM_GIVER		    = 4096, //From 0.8.6+ IceEE
	CDEF_HINT_AUCTIONEER			= 8192,  //From 0.8.6+ IceEE
};

struct VisibleWeaponSet
{
	enum
	{
		INVALID = -1,
		NONE    =  0,
		MELEE   =  1,
		RANGED  =  2
	};
};

namespace AuthMethod
{
	enum Enum
	{
		EXTERNAL = 0,
		DEV = 1,
		SERVICE
	};
}

// For character item trading.
namespace TradeEventTypes
{
	enum
	{
		REQUEST 			= 0,
		REQUEST_ACCEPTED	= 1,
		REQUEST_CLOSED		= 2,
		ITEM_ADDED			= 3,
		ITEM_REMOVED		= 4,
		CURRENCY_OFFERED	= 5,
		OFFER_MADE			= 6,
		OFFER_ACCEPTED		= 7,
		OFFER_CANCELED		= 8,
		ITEMS_OFFERED		= 9
	};
}

// For character item trading.
namespace CloseReasons
{
	enum
	{
		COMPLETE           = 0,
		TIMEOUT            = 1,
		DISTANCE           = 2,
		CANCELED           = 3,
		INSUFFICIENT_FUNDS = 4,
		INSUFFICIENT_SPACE = 5
	};
}

// For character item trading.
namespace CurrencyCategory
{
	enum
	{
		COPPER  = 0,
		CREDITS = 1
	};
}

extern float HeroismHealthDivisor;

extern int DamageReductionModiferPerLevel;
extern int DamageReductionAdditive;
extern float RegenConMod;
extern float RegenSpiritMod;
extern float RegenLevelMod;
extern float RegenCombatModifier;
extern float SecondsPerRoundInverse;
extern int HealthConModifier;
extern float DodgeDexMod;
extern float ParryDexMod;
extern float ParryStrMod;
extern float ParryMod;
extern float DeflectPsycheMod;
extern float BlockDexterityMod;
extern float BlockStrengthMod;
extern float BlockMod;
extern float CritMeleeDexterityMod;
extern float CritMagicPsycheMod;
extern float ToHitMeleeDexMod;
extern float ToHitMagicPsycheMod;

extern double g_FacingTarget;
extern int g_CreatureListenRange;

//Data returned on a query request for "clan.info"
extern char g_ClanName[];
extern char g_ClanMOTD[];
extern char g_ClanLeader[];

//Data returned on a query request for "clan.list"
extern int g_ClanMemberCount;
extern char *g_ClanMemberList[3][5];   //If changed, must match Globals.cpp definition.

const double PI = 3.1415926535897932384626433832795;
const double DOUBLE_PI = PI * 2.0;  //New code references this.

// TODO - This needs to be moved to data
const int POSTAGE_STAMP_ITEM_ID = 8400;

//extern const int MaxLevel;
extern const int LevelRequirements[71][2];

extern double g_VendorMarkup;

namespace Global
{
	//We can possibly make these configurable variables at some point in the future.
	extern const int MAX_SEND_CHUNK_SIZE;
	extern const int DazeGuaranteeTime;

	extern const int MAX_HEROISM;
	extern const int MAX_BASE_LUCK;

	extern const int AbilityPointTable[71][3];
	int GetAbilityPointsLevelIncrement(int level);
	int GetAbilityPointsLevelCumulative(int level);

	extern IntArray<71, 3> ResCostTable;   //Row for each level 0-70.  Columns: [0]=Level, [1]=Option2, [2]=Option3

	void LoadResCostTable(const char *filename);
	int GetResurrectCost(int playerLevel, int resChoice);
}

class CreatureInstance;
namespace Debug
{

	extern int LastAbility;
	extern int CreatureDefID;
	extern char LastName[64];
	extern void *LastPlayer;
	extern int IsPlayer;

	extern int LastTileZone;
	extern int LastTileX;
	extern int LastTileY;
	extern int LastTilePropID;
	extern void* LastTilePtr;
	extern void* LastTilePackage;

	extern int LastFlushSimulatorID;
	extern CreatureInstance *ActivateAbility_cInst;
	extern short ActivateAbility_ability;
	extern int ActivateAbility_ActionType;
	extern int ActivateAbility_abTargetCount;
	extern CreatureInstance *ActivateAbility_abTargetList[8];
	extern int LastSimulatorID;
}

#endif //GLOBALS_H
