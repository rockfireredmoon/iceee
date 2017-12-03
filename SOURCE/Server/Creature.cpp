#include <math.h>   //For atan2()
#include <algorithm>  //For STL sort
#include <limits>

#include "Ability2.h"
#include "Creature.h"
#include "FileReader.h"
#include "StringList.h"
#include "Instance.h"
#include "Inventory.h"
#include "Util.h"
#include "Globals.h"  //For combat globals
#include "AIScript.h"
#include "AIScript2.h"
#include "DebugTracer.h"
#include "Simulator.h"
#include "Squirrel.h"
#include "Item.h"  //For quest item rewards
#include "Interact.h"  //For interactions with non-quest objects (like warps)
#include "Components.h"
#include "DirectoryAccess.h"
#include "QuestScript.h"
#include "ZoneObject.h"
#include "PartyManager.h"
#include "Config.h"
#include "Quest.h"
#include "Combat.h"
#include "InstanceScale.h"
#include "VirtualItem.h"
#include "Guilds.h"
#include "Stats.h"
#include "Util.h"

#include "Debug.h"
#include "Report.h"

const int TAUNT_BECKON_DISTANCE = 50;           //Taunted creatures will attempt to move at least this close to their target.

const int SLOW_CREATURE_LEVEL_THRESHOLD = 9;     //Creatures less than or equal to this level will move slower per update cycle.
const int SLOW_CREATURE_SPEED_PENALTY =  15;     //Speed amount to slow down mobs that are within the level threshold.

const int INVISIBILITY_STAT_MINIMUM = 50;
const int CREATURE_WALK_SPEED = 20;
const int CREATURE_JOG_SPEED = 60;
const int CREATURE_RUN_SPEED = 100;
const int CREATURE_MOVEMENT_COMBAT_FREQUENCY = 500;     //How long between processing movement steps while in combat.
const int CREATURE_MOVEMENT_NONCOMBAT_FREQUENCY = 1000;  //How long between processing movement steps while out of combat.


const int AGGRO_ELEV_RANGE = 30;      //Only consider targets that are within this vertical range
const int AGGRO_DEFAULT_RANGE = 130;  //150;
const int AGGRO_MAXIMUM_RANGE = 200;  //200
const int AGGRO_MINIMUM_RANGE = 30;
const int AGGRO_LEVEL_MOD = 20;  //30
const int MAX_ATTACK_RANGE = 500;

const int MAX_SPAWNLIST = 75;  //Maximum number of items that can be returned by a "spawn.list" query

double ARMOR_VARIATION_MIN = -0.05;  //Variation on the percentage resist when calculated from armor rating
double ARMOR_VARIATION_MAX = 0.05;  //Variation on the percentage resist when calculated from armor rating

const double Cone_180 = 1.57079633;
const double Cone_90 = 0.785398163;

extern int g_ProtocolVersion;
extern unsigned long g_ServerTime;

CreatureDefManager CreatureDef;
PendingOperation pendingOperations;

extern char GSendBuf[];
extern char GAuxBuf[];

SelectedObject :: SelectedObject()
{
	Clear(true);
}
void SelectedObject :: Clear(bool UNUSED_eraseAutoAttack)
{
	targ = NULL;
	//if(eraseAutoAttack == true)
	//	aaTarg = NULL;
	desiredRange = 0;
	desiredSpeed = 0;
	bInstigated = false;
	DesLocX = 0;
	DesLocZ = 0;
}
bool SelectedObject :: hasTargetCDef(int CDefID)
{
	if(targ == NULL)
		return false;
	return (targ->CreatureDefID == CDefID);
}

void ActiveAbilityInfo :: Duration(int seconds)
{
	//Set the duration to the number of seconds.
	//Recalculate the number of iterations that should occur.
	//Duration is meant to be adjusted before the skill is
	//processed, otherwise it might run longer if the iteration
	//count is reset while the effect is active.
	//NOTE: max is based on the data type's size (currently short)
	if(seconds < 1 || seconds > 32)
	{
		g_Log.AddMessageFormat("[DEBUG] Ability Duration is invalid: %d", seconds);
		seconds = Util::ClipInt(seconds, 1, 32);
	}
	iterationDur = 1000 * seconds;
	if(iterationInt == 0)
	{
		g_Log.AddMessageFormat("[DEBUG] Ability Iterations is zero: %d", abilityID);
		iterationInt = 1000;
	}
	iterations = iterationDur / iterationInt;
}

void ActiveAbilityInfo :: Clear(const char *debugCaller)
{
	//Simple cleanup, clears the target list and deactivates the ability.
	ClearTargetList();

	bPending = false;
	bForce = false;
	bSecondary = false;
	bUnbreakableChannel = false;
	interruptChanceMod = 0;

	mightChargesSpent = 0;
	willChargesSpent = 0;

	willAdjust = 0;
	mightAdjust = 0;
	willChargeAdjust = 0;
	mightChargeAdjust = 0;
	bResourcesSpent = false;

	bParallel = false;
	abilityID = 0;
	type = AbilityType::None;
}

void ActiveAbilityInfo :: TransferTargetList(ActiveAbilityInfo *from)
{
	if(this == from)
	{
		g_Log.AddMessageFormat("[ERROR] TransferTargetList objects are the same.");
		return;
	}
	TargetCount = from->TargetCount;
	memcpy(TargetList, from->TargetList, sizeof(TargetList));
}

void ActiveAbilityInfo :: ClearTargetList(void)
{
	for(int a = 0; a < TargetCount; a++)
		TargetList[a] = NULL;
	TargetCount = 0;
}

void ActiveAbilityInfo :: SetPosition(float newx, float newy, float newz)
{
	x = newx;
	y = newy;
	z = newz;
}

void ActiveAbilityInfo :: SetPosition(int newx, int newy, int newz)
{
	x = static_cast<float>(newx);
	y = static_cast<float>(newy);
	z = static_cast<float>(newz);
}

bool ActiveAbilityInfo :: IsBusy(void)
{
	if(bPending == true)
		return true;
	if(type == AbilityType::Cast || type == AbilityType::Channel)
		return true;

	return false;
}

//**************************************************
//               Appearance Modifiers
//**************************************************
AppearanceModifier::~AppearanceModifier() {}

ReplaceAppearanceModifier::ReplaceAppearanceModifier(std::string replacement) {
	mReplacement = replacement;
}

ReplaceAppearanceModifier::~ReplaceAppearanceModifier() {}

std::string ReplaceAppearanceModifier::Modify(std::string source) {
	return mReplacement;
}

std::string ReplaceAppearanceModifier::ModifyEq(std::string source) {
	return source;
}

AppearanceModifier * ReplaceAppearanceModifier::Clone() {
	return new ReplaceAppearanceModifier(mReplacement);
}

//

AbstractAppearanceModifier::AbstractAppearanceModifier() {
}

AbstractAppearanceModifier::~AbstractAppearanceModifier() {}

std::string AbstractAppearanceModifier::Modify(std::string source) {
	return DoModify(source, false);
}

std::string AbstractAppearanceModifier::ModifyEq(std::string source) {
	return DoModify(source, true);
}

std::string AbstractAppearanceModifier::DoModify(std::string source, bool eq) {
	string currentAppearance = source;
	size_t pos = currentAppearance.find(":");
	if(pos == string::npos)	{
		g_Log.AddMessageFormat("Could not parse exist appearance. %s", currentAppearance.c_str());
		return source;
	}
	string prefix = currentAppearance.substr(0, pos + 1);
	currentAppearance = "this.a <- " + currentAppearance.substr(pos + 1, currentAppearance.size()) + ";";


	// TODO shared vm
	HSQUIRRELVM vm = sq_open(g_Config.SquirrelVMStackSize);
	Sqrat::Script script(vm);

	g_Log.AddMessageFormat("Adjusting appearance. %s", currentAppearance.c_str());

	script.CompileString(_SC(currentAppearance.c_str()));
	if (Sqrat::Error::Occurred(vm)) {
		g_Log.AddMessageFormat("Failed to compile appearance. %s", Sqrat::Error::Message(vm).c_str());
		return source;
	}
	script.Run();

	Sqrat::RootTable rootTable = Sqrat::RootTable(vm);
	Sqrat::Object placeholderObject = rootTable.GetSlot(_SC("a"));
	Sqrat::Table table = placeholderObject.Cast<Sqrat::Table>();

	if(eq)
		ProcessTableEq(&table);
	else
		ProcessTable(&table);

	Squirrel::Printer printer;
	std::string newAppearance = prefix;
	printer.PrintTable(&newAppearance, table);
	g_Log.AddMessageFormat("Attaching item. New appearance is. %s", newAppearance.c_str());

	return newAppearance;
}

//
CreatureAttributeModifier::CreatureAttributeModifier(std::string attribute, std::string value) {
	mAttribute = attribute;
	mValue = value;
}
CreatureAttributeModifier::~CreatureAttributeModifier() {}

void CreatureAttributeModifier::ProcessTable(Sqrat::Table *table) {
	g_Log.AddMessageFormat("Setting attr '%s' to '%s'", mAttribute.c_str(), mValue.c_str());
	table->SetValue(_SC(mAttribute.c_str()), _SC(mValue.c_str()));
}

void CreatureAttributeModifier::ProcessTableEq(Sqrat::Table *table) {
}

AppearanceModifier * CreatureAttributeModifier::Clone() {
	return new CreatureAttributeModifier(mAttribute, mValue);
}


//

AddAttachmentModifier::AddAttachmentModifier(std::string type, std::string node) {
	mType = type;
	mNode = node;
}
AddAttachmentModifier::~AddAttachmentModifier() {}

void AddAttachmentModifier::ProcessTable(Sqrat::Table *table) {
	Sqrat::Object attachments = table->GetSlot(_SC("ea"));
	if(attachments.IsNull()) {
		// No existing attachments
		Sqrat::Array arr(table->GetVM());
		table->SetValue(_SC("ea"), arr);
		attachments = table->GetSlot(_SC("ea"));
	}
	Sqrat::Array attachmentsArr = attachments.Cast<Sqrat::Array>();
	Sqrat::Table item(table->GetVM());
	item.SetValue(_SC("type"), _SC(mType));
	item.SetValue(_SC("node"), _SC(mNode));
	attachmentsArr.Append(item);
}

void AddAttachmentModifier::ProcessTableEq(Sqrat::Table *table) {
}

AppearanceModifier * AddAttachmentModifier::Clone() {
	return new AddAttachmentModifier(mType, mNode);
}

//

NudifyAppearanceModifier::NudifyAppearanceModifier() {}
NudifyAppearanceModifier::~NudifyAppearanceModifier() {}

std::string NudifyAppearanceModifier::Modify(std::string source) {
	return source;
}

std::string NudifyAppearanceModifier::ModifyEq(std::string source) {
	return "{}";
}
AppearanceModifier * NudifyAppearanceModifier::Clone() {
	return new NudifyAppearanceModifier();
}


//**************************************************
//               CreatureDefinition
//**************************************************
CreatureDefinition :: CreatureDefinition()
{
	Clear();
}

CreatureDefinition :: ~CreatureDefinition()
{
}

void CreatureDefinition :: Clear(void)
{
	css.Clear();
	CreatureDefID = 0;
	DefHints = 0;
	DefaultEffects.clear();
	DropRateMult = 1.0;
	DropRateProfile = "";
	NamedMob = false;
	Items.clear();
}

void CreatureDefinition :: CopyFrom(CreatureDefinition& source)
{
	css.CopyFrom(&source.css);
	CreatureDefID = source.CreatureDefID;
	DefHints = source.DefHints;
	DefaultEffects.assign(source.DefaultEffects.begin(), source.DefaultEffects.end());
	DropRateMult = source.DropRateMult;
	DropRateProfile = source.DropRateProfile;
	NamedMob = source.NamedMob;
	Items = source.Items;
}

bool CreatureDefinition :: operator < (const CreatureDefinition& other) const
{
	return (CreatureDefID < other.CreatureDefID);
}

void CreatureDefinition :: SaveToStream(FILE *output)
{
	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "ID=%d\r\n", CreatureDefID);
	if(DefHints > 0)
		fprintf(output, "defHints=%d\r\n", DefHints);

	ConfigString ExtraData;
	if(NamedMob)
		ExtraData.SetKeyValue("namedmob", "");
	if(DropRateProfile.length() > 0)
		ExtraData.SetKeyValue("droprateprofile", DropRateProfile.c_str());
	if(DropRateMult != 1) {
		char buf[20];
		Util::SafeFormat(buf, sizeof(buf), "%f", DropRateMult);
		ExtraData.SetKeyValue("dropratemulti", buf);
	}
	for(std::vector<int>::iterator it = Items.begin() ; it != Items.end(); it++) {
		STRINGLIST str;
		str.push_back("item");
		str.push_back("");
		ExtraData.mData.push_back(str);
	}

	if(!ExtraData.IsEmpty()) {
		std::string str;
		ExtraData.GenerateString(str);
		fprintf(output, "ExtraData=%s\r\n", str.c_str());
	}
	if(DefaultEffects.size() > 0) {
		fprintf(output, "Effects=");
		for(std::vector<int>::iterator it = DefaultEffects.begin(); it != DefaultEffects.end(); it++) {
			if(it != DefaultEffects.begin())
				fprintf(output, ",");
			fprintf(output, "%s", GetStatusNameByID(*it));
		}
		fprintf(output, "\r\n");
	}
	int a;
	for(a = 0; a < NumStats; a++)
		if(isStatZero(a, &css) == false)
			WriteStatToFile(a, &css, output);
}

//**************************************************
//               CreatureInstance
//**************************************************

CreatureInstance :: CreatureInstance()
{
	//Need this because the Clear function will check for non-Null and attempt
	//to delete it.
	aiScript = NULL;
	aiNut = NULL;
	hateProfilePtr = NULL;
	activeLootID = 0;
	scriptMoveEvent = -1;

	spawnGen = NULL;
	spawnTile = NULL;

	Clear();
}

CreatureInstance :: ~CreatureInstance()
{
	RemoveAttachedHateProfile();
	//baseStatusValue.clear();
	activeStatMod.clear();
	activeStatusEffect.clear();
	cooldownManager.Clear();
	buffManager.Clear();
}

void CreatureInstance :: Clear(void)
{

	if(aiNut != NULL)
	{
		aiNutManager.RemoveActiveScript(aiNut);
		aiNut = NULL;

	}
	if(aiScript != NULL)
	{
		aiScriptManager.RemoveActiveScript(aiScript);
		aiScript = NULL;
	}
	if(activeLootID != 0)
	{
		actInst->lootsys.RemoveCreature(activeLootID);
		activeLootID = 0;
	}
	
	CreatureDefID = 0;
	CreatureID = 0;

	actInst = NULL;
	charPtr = NULL;
	spawnGen = NULL;
	spawnTile = NULL;
	simulatorPtr = NULL;
	//Spawner = NULL;

	memset(&statusEffects, 0, sizeof(statusEffects));

	memset(&ZoneString, 0, sizeof(ZoneString));
	CurrentX = 0;
	CurrentY = 0;
	CurrentZ = 0;
	AnchorObject = NULL;

	Heading = 0;
	Rotation = 0;
	Speed = 0;

	dialogIndex = -1;
	timer_dialog = 0;

	memset(&MainDamage, 0, sizeof(MainDamage));
	memset(&OffhandDamage, 0, sizeof(OffhandDamage));
	memset(&RangedDamage, 0, sizeof(RangedDamage));
	memset(&EquippedWeapons, 0, sizeof(EquippedWeapons));

	Faction = 0;
	PartyID = 0;
	serverFlags = 0;

	memset(&CurrentTarget, 0, sizeof(CurrentTarget));

	movementTime = 0;

	LastCharge = 0;
	timer_autoattack = 0;
	timer_mightregen = 0;
	timer_willregen = 0;
	timer_lasthealthupdate = 0;

	css.Clear();

	//memset(&activeAbility, 0, sizeof(activeAbility));
	memset(&ab, 0, sizeof(ab));
	//memset(&TargetList, 0, sizeof(TargetList));
	//TargetCount = 0;
	bOrbChanged = false;

	//baseStatusValue.clear();  UNUSED
	activeStatMod.clear();
	activeStatusEffect.clear();
	implicitActions.clear();
	cooldownManager.Clear();
	buffManager.Clear();

	previousPathNode = 0;
	nextPathNode = 0;
	lastIdleX = 0;
	lastIdleZ = 0;
	tetherNodeX = 0;
	tetherNodeZ = 0;
	tetherFacing = 0;

	ClearAppearanceModifiers();

	transformModifier = NULL;
	transformCreatureId = 0;
	transformAbilityId = 0;

	LastUseDefID = 0;

	swimming = false;
}

bool CreatureInstance :: KillAI(void)
{
	bool killed = false;
	if(aiScript != NULL)
	{
		aiScript->EndExecution();
		aiScriptManager.RemoveActiveScript(aiScript);
		aiScript = NULL;
		killed = true;
	}
	if(aiNut != NULL)
	{
		aiNut->HaltExecution();
		aiNutManager.RemoveActiveScript(aiNut);
		aiNut = NULL;
		killed = true;
	}
	return killed;
}

void CreatureInstance :: UnloadResources(void)
{
	//NOTE: Should only be called by the main thread.
	//Unloads any resources attached to this object (or that this object is specially linked to
	//to, like spawnpoints (but not ability targets, those are separate).
	//Prepares this for safe de-allocation.  Should be explicitly called before deleting a
	//creature from the list.
	
	ClearAppearanceModifiers();

	//Note: the AI Script may be assigned to temporary instances.
	//Temporary instances are local to a function, and are used for initializing
	//data before they are added into the official creature list (via push_back())
	if(actInst == NULL)
		g_Log.AddMessageFormat("[CRITICAL] UnloadResources() called for creature without an instance.");

	g_QuestNutManager.RemoveActiveScripts(CreatureID);

	KillAI();
	RemoveAttachedHateProfile();
	if(serverFlags & ServerFlags::IsPlayer)
	{
		if(activeLootID != 0)
		{
			actInst->tradesys.CancelTransaction(CreatureID, activeLootID, GSendBuf);
			activeLootID = 0;
		}
	}

	if(activeLootID != 0)
	{
		actInst->lootsys.RemoveCreature(activeLootID);
		activeLootID = 0;
	}

	activeStatMod.clear();
	activeStatusEffect.clear();
	implicitActions.clear();
	cooldownManager.Clear();
	buffManager.Clear();

	RemoveFromSpawner(false);
}

void CreatureInstance :: RemoveAttachedHateProfile(void)
{
	if(hateProfilePtr != NULL)
	{
		if(actInst != NULL)
			actInst->hateProfiles.RemoveProfile(hateProfilePtr);
		hateProfilePtr = NULL;
	}
}

void CreatureInstance :: RemoveFromSpawner(bool wasKilled)
{

	//Non player, non sidekicks were generated from a spawner.
	//This will notify the spawners that a respawn may be necessary.

	if(serverFlags & ServerFlags::IsNPC)
	{
		if(spawnGen != NULL)
		{
			//If this spawn package was designed as a spawner
			bool notifyKill = false;
			if(spawnGen->spawnPackage != NULL)
			{
				if(spawnGen->spawnPackage->isScriptCall == true && wasKilled == true)
					actInst->ScriptCallPackageKill(spawnGen->spawnPackage->packageName);
				if(spawnGen->spawnPackage->isSequential == true)
					notifyKill = true;
			}

			if(actInst->spawnsys.NotifyKill(spawnGen, CreatureID) == true || notifyKill == true)
				if(wasKilled == true)
					actInst->ScriptCallKill(CreatureDefID, CreatureID);

			spawnGen = NULL;
			spawnTile = NULL;
		}
	}
}

void CreatureInstance :: CopyFrom(CreatureInstance *source)
{
	//Attempts to create a near duplicate of a the source creature.
	//Required when preserving Creature Instance data between map instances.

	//Safeguard so it doesn't copy itself.
	if(source == this || source == NULL)
		return;

	//Note: AI script pointers are tied to a unique active script instance.
	//They should not be copied.
	//aiScript = source->aiScript;

	CreatureDefID = source->CreatureDefID;
	CreatureID = source->CreatureID;

	//Should probably leave the instance assignment to more important operations.
	//actInst = source->actInst;

	//Another pointer to skip.
	//Spawner = source->Spawner;

	memcpy(&statusEffects, source->statusEffects, sizeof(statusEffects));

	memcpy(&ZoneString, &source->ZoneString, sizeof(ZoneString));
	CurrentX = source->CurrentX;
	CurrentY = source->CurrentY;
	CurrentZ = source->CurrentZ;

	//For safety reasons, don't copy the anchor object since it's a pointer.
	//AnchorObject = NULL;

	Heading = source->Heading;
	Rotation = source->Rotation;
	Speed = source->Speed;

	memcpy(&MainDamage, &source->MainDamage, sizeof(MainDamage));
	memcpy(&OffhandDamage, &source->OffhandDamage, sizeof(OffhandDamage));
	memcpy(&RangedDamage, &source->RangedDamage, sizeof(RangedDamage));
	memcpy(&EquippedWeapons, &source->EquippedWeapons, sizeof(EquippedWeapons));

	Faction = source->Faction;
	PartyID = source->PartyID;
	serverFlags = source->serverFlags;

	//Target contains a pointer, not safe for copying.
	//memset(&CurrentTarget, 0, sizeof(CurrentTarget));

	movementTime = source->movementTime;

	LastCharge = source->LastCharge;
	timer_autoattack = source->timer_autoattack;
	timer_mightregen = source->timer_mightregen;
	timer_willregen = source->timer_willregen;

	css.CopyFrom(&source->css);

	//Ability controls contain target lists.  Those pointers are not safe to copy.
	//memset(&ab, 0, sizeof(ab));
	bOrbChanged = false;

	activeStatMod.assign(source->activeStatMod.begin(), source->activeStatMod.end());
	activeStatusEffect.assign(source->activeStatusEffect.begin(), source->activeStatusEffect.end());
	implicitActions.assign(source->implicitActions.begin(), source->implicitActions.end());
	baseStats.assign(source->baseStats.begin(), source->baseStats.end());
	cooldownManager.CopyFrom(source->cooldownManager);
	buffManager.CopyFrom(source->buffManager);

	// TODO - Em - why the need to clear? this implies it is getting called twice when copying
	ClearAppearanceModifiers();
	for(std::vector<AppearanceModifier*>::iterator it = source->appearanceModifiers.begin(); it != source->appearanceModifiers.end(); ++it) {
		AppearanceModifier *ap = *it;
		AppearanceModifier *cap = (*it)->Clone();
		if(ap == source->transformModifier) {
			transformModifier = cap;
		}
		g_Log.AddMessageFormat("Copied 1 appearance modifiers");
		appearanceModifiers.push_back(cap);
	}
	transformAbilityId = source->transformAbilityId;
	transformCreatureId = source->transformCreatureId;

	LastUseDefID = source->LastUseDefID;

	swimming = source->swimming;

	// TODO Em really not sure about this .. but I am running out of ideas
	charPtr = source->charPtr;
	spawnGen = source->spawnGen;
	spawnTile = source->spawnTile;
	simulatorPtr = source->simulatorPtr;

}

void CreatureInstance :: CopyBuffsFrom(CreatureInstance *source)
{
	activeStatMod.assign(source->activeStatMod.begin(), source->activeStatMod.end());
	activeStatusEffect.assign(source->activeStatusEffect.begin(), source->activeStatusEffect.end());
	implicitActions.assign(source->implicitActions.begin(), source->implicitActions.end());
	baseStats.assign(source->baseStats.begin(), source->baseStats.end());
	cooldownManager.CopyFrom(source->cooldownManager);
	buffManager.CopyFrom(source->buffManager);
}

bool CreatureInstance :: StartAI(std::string &errors)
{
	if(aiNut != NULL || aiScript != NULL)
		return false;

	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CreatureDefID);
	//if(cdef != NULL && (strncmp(css.appearance,  "p1:", 3) != 0)) // && (!(serverFlags & ServerFlags::NeutralInactive)))
	if((cdef != NULL) )//&& !(serverFlags & ServerFlags::Noncombatant))
	{
		const char *scriptName = css.ai_package;
		if(scriptName[0] == 0)
			scriptName = cdef->css.ai_package;

		ScriptCore::NutScriptCallStringParser p(scriptName);
		if(p.mEnabled) {
			AINutDef *def = aiNutManager.GetScriptByName(p.mScriptName.c_str());
			if(def == NULL)
			{
				aiScript = aiScriptManager.AddActiveScript(scriptName);
				if(aiScript == NULL)
					g_Log.AddMessageFormatW(MSG_SHOW, "[WARNING] Could not find script [%s] for instantiated creature [%s]", p.mScriptName.c_str(), cdef->css.display_name);
				else
					aiScript->attachedCreature = this;
			}
			else
			{
				aiNut = aiNutManager.AddActiveScript(this, def, p.mArgs, errors);
				if(aiNut == NULL)
				{
					g_Log.AddMessageFormatW(MSG_SHOW, "[WARNING] While the script %s for instantiated creature [%s] was found, it did not start." , p.mScriptName.c_str(), cdef->css.display_name);
				}
			}
		}
		else
			return false;


		return true;
	}

	return false;
}

void CreatureInstance :: Instantiate(void)
{
	//Initializes certain values based on CSS fields or global
	//defaults.
	//Note: the client won't process NPC movement unless it has health.
	if(css.constitution <= 0)
	{
		css.constitution = 1;
	}
	css.health = CalcRestrictedHealth(-1, true);

	//int cdef = CreatureDef.GetIndex(CreatureDefID);

	if((serverFlags & ServerFlags::KillableProp)==0) {
		if((strlen(css.ai_package) == 0) || (strcmp(css.ai_package, "nothing") == 0))
			SetServerFlag(ServerFlags::NeutralInactive, true);
		if(css.IsPropAppearance() == true)
		{
			SetServerFlag(ServerFlags::NeutralInactive, true);
			SetServerFlag(ServerFlags::Noncombatant, true);
		}
	}

	std::string errors;
	StartAI(errors);

	//Need this or else spawned props may retain aggro status
	if(serverFlags & ServerFlags::NeutralInactive)
		css.aggro_players = 0;

	if(css.aggro_players == 0)
		SetServerFlag(ServerFlags::NeutralInactive, true);

	css.will = ABGlobals::MAX_WILL;
	css.might = ABGlobals::MAX_MIGHT;

	//Character definition files usually set these fields.
	//To allow for customization, only set defaults if they have
	//not been initialized.
	if(css.might_regen == 0.0F)
		css.might_regen = ABGlobals::DEFAULT_MIGHT_REGEN;
	if(css.will_regen == 0.0F)
		css.will_regen = ABGlobals::DEFAULT_WILL_REGEN;

	if(css.offhand_weapon_damage == 0)
		css.offhand_weapon_damage = ABGlobals::DEFAULT_OFFHAND_WEAPON_DAMAGE;
	if(css.casting_setback_chance == 0)
		css.casting_setback_chance = ABGlobals::DEFAULT_CASTING_SETBACK_CHANCE;
	if(css.channeling_break_chance == 0)
		css.channeling_break_chance = ABGlobals::DEFAULT_CHANNELING_BREAK_CHANCE;

	//Always needs to be set for casting speeds to properly display
	//in the client.
	css.mod_attack_speed = ABGlobals::MINIMAL_FLOAT;
	css.mod_casting_speed = ABGlobals::MINIMAL_FLOAT;

	ApplyGlobalInstanceBuffs();

	// TODO floods events
	/*if(actInst != NULL && actInst->nutScriptPlayer != NULL && actInst->nutScriptPlayer->mActive) {
		std::vector<ScriptCore::ScriptParam> parms;
		parms.push_back(ScriptCore::ScriptParam(CreatureID));
		parms.push_back(ScriptCore::ScriptParam(CreatureDefID));
		actInst->nutScriptPlayer->JumpToLabel("on_spawn", parms);
	}*/
}

int CreatureInstance :: GetMitigatedDamage(int damageAmount, int armorRating, int reductionMod)
{
	//Revised function to return the proper damage reduction factoring both the armor rating
	//(typically from equipment totals) and the reduction modifier (a percentage amount, adjusted
	//by certain abilities for higher or lower armor ratings).
	if(css.level < 0)
		return damageAmount;

	//g_Log.AddMessageFormat("mitigating: amount: %d, armor: %d, reduction: %d", damageAmount, armorRating, reductionMod);

	if(reductionMod != 0)
	{
		float multiplier = (float)reductionMod / (float)ABGlobals::INTEGRAL_FRACTION_TOTAL;
		armorRating += (int)((float)armorRating * multiplier);
	}
	
	//Debuffs pushing armor into negatives were causing zero damage.
	if(armorRating < 0)
		armorRating = 0;

	double reduct = ((double)armorRating / ((armorRating + (DamageReductionModiferPerLevel * css.level) + DamageReductionAdditive)));
	//reduct += randdbl(ARMOR_VARIATION_MIN, ARMOR_VARIATION_MAX);
	damageAmount -= (int)((double)damageAmount * reduct);
	if(damageAmount < 0)
		damageAmount = 0;

	//g_Log.AddMessageFormat("returning: amount: %d, armor: %d", damageAmount, armorRating);
	return damageAmount;
}

// TODO: OBSOLETE WITH GetMitigatedDamage();
int CreatureInstance :: GetReducedDamage(int armorRating, int damageAmount)
{
	if(css.level < 0)
		return damageAmount;

	double reduct = ((double)armorRating / ((armorRating + (DamageReductionModiferPerLevel * css.level) + DamageReductionAdditive)));
	reduct += randdbl(ARMOR_VARIATION_MIN, ARMOR_VARIATION_MAX);
	damageAmount -= (int)((double)damageAmount * reduct);
	if(damageAmount < 0)
		damageAmount = 0;
	return damageAmount;
}

// TODO: OBSOLETE WITH GetMitigatedDamage();
int CreatureInstance :: GetResistedDamage(int resistRating, int damageAmount)
{
	//Applies a percentage reduction to a damage amount.  A positive value
	//Indicates reduction.  10 units per 1%.
	//dr_mod_melee, dr_mod_fire, dr_mod_frost, dr_mod_mystic, dr_mod_death
	damageAmount -= (int)(damageAmount * ((double)resistRating / 1000.0));
	if(damageAmount < 0)
		damageAmount = 0;
	return damageAmount;
}

bool CreatureInstance :: IsValidForPVP(void)
{
	return ( serverFlags & ServerFlags::IsPlayer ) && actInst->mMode != PVP::GameMode::PVE_ONLY && _HasStatusList(StatusEffects::PVPABLE) != -1;
}

HateProfile* CreatureInstance :: GetHateProfile(void)
{
	if(hateProfilePtr == NULL)
		if(!(serverFlags & ServerFlags::IsPlayer) || IsValidForPVP())
			hateProfilePtr = actInst->hateProfiles.GetProfile();
	return hateProfilePtr;
}

int CreatureInstance :: GetAdditiveWeaponSpecialization(int amount, int itemEquipSlot)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return 0;

	int slot = Util::ClipInt(itemEquipSlot, 0, 2);
	int type = EquippedWeapons[slot];
	int add = 0;
	switch(type)
	{
	case WeaponType::SMALL: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_small); break;
	case WeaponType::ONE_HAND: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_1h); break;
	case WeaponType::TWO_HAND: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_2h); break;
	case WeaponType::POLE: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_pole); break;
	case WeaponType::WAND: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_wand); break;
	case WeaponType::BOW: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_box); break;
	case WeaponType::THROWN: add = Util::GetAdditiveFromIntegralPercent1000(amount, css.weapon_damage_thrown); break;
	case WeaponType::ARCANE_TOTEM:
	case WeaponType::NONE:
	default:
		add = 0;
	}
	return add;
}

int CreatureInstance :: GetAdjustedDamage(CreatureInstance *attacker, int damageType, int amount)
{
	if(amount == 0)
		return 0;


	if(serverFlags & ServerFlags::IsPlayer)
	{
		if(attacker->serverFlags & ServerFlags::IsPlayer)
		{
			if(CanPVPTarget(attacker) == false)
				return 0;
		}

		/*
		//Hack to disable PvP
		if(attacker->serverFlags & ServerFlags::IsPlayer)
		{
			if(attacker->HasStatus(StatusEffects::PVPABLE) == false)
				return 0;
			if(HasStatus(StatusEffects::PVPABLE) == false)
				return 0;
		}
		*/

		//Check for debug godmode.
		if(HasStatus(StatusEffects::INVINCIBLE))
			return 0;
	}

	//Creatures returning to leash location cannot be damaged.
	if(serverFlags & ServerFlags::LeashRecall)
		return 0;

	//Sidekicks have these flags for nameboard color, but aren't actually immune from
	//damage.  Only NPCs may damage sidekicks.
	if(!(serverFlags & ServerFlags::IsSidekick))
	{
		if(HasStatus(StatusEffects::INVINCIBLE))
			return 0;
		//if(HasStatus(StatusEffects::UNATTACKABLE))
		//	return 0;
	}

	switch(damageType)
	{
	case DamageType::MELEE:
		if(HasStatus(StatusEffects::IMMUNE_DAMAGE_MELEE))
			return 0;
		//amount = GetReducedDamage(css.damage_resist_melee, amount);
		//amount = GetResistedDamage(css.dr_mod_melee, amount);
		amount = GetMitigatedDamage(amount, css.damage_resist_melee, css.dr_mod_melee);
		break;
	case DamageType::FIRE:
		if(HasStatus(StatusEffects::IMMUNE_DAMAGE_FIRE))
			return 0;
		//amount = GetReducedDamage(css.damage_resist_fire, amount);
		//amount = GetResistedDamage(css.dr_mod_fire, amount);
		amount -= Combat::GetPsycheResistReduction(amount, css.psyche);
		amount = GetMitigatedDamage(amount, css.damage_resist_fire, css.dr_mod_fire);
		break;
	case DamageType::FROST:
		if(HasStatus(StatusEffects::IMMUNE_DAMAGE_FROST))
			return 0;
		//amount = GetReducedDamage(css.damage_resist_frost, amount);
		//amount = GetResistedDamage(css.dr_mod_frost, amount);
		amount -= Combat::GetPsycheResistReduction(amount, css.psyche);
		amount = GetMitigatedDamage(amount, css.damage_resist_frost, css.dr_mod_frost);
		break;
	case DamageType::MYSTIC:
		if(HasStatus(StatusEffects::IMMUNE_DAMAGE_MYSTIC))
			return 0;
		//amount = GetReducedDamage(css.damage_resist_mystic, amount);
		//amount = GetResistedDamage(css.dr_mod_mystic, amount);
		amount -= Combat::GetSpiritResistReduction(amount, css.spirit);
		amount = GetMitigatedDamage(amount, css.damage_resist_mystic, css.dr_mod_mystic);
		break;
	case DamageType::DEATH:
		if(HasStatus(StatusEffects::IMMUNE_DAMAGE_DEATH))
			return 0;
		//amount = GetReducedDamage(css.damage_resist_death, amount);
		//amount = GetResistedDamage(css.dr_mod_death, amount);
		amount -= Combat::GetSpiritResistReduction(amount, css.spirit);
		amount = GetMitigatedDamage(amount, css.damage_resist_death, css.dr_mod_death);
		break;
	}
	return amount;
}

int CreatureInstance :: ApplyPostDamageModifiers(CreatureInstance *attacker, int amount, int &absorbedDamage)
{
	if(amount == 0)
		return 0;

	//The final step in damage modification.  To be called after damage mitigation functions
	//have been processed.
	//These modifiers operate on the final result, such as reflecting a percentage of damage
	//or absorbing it.
	if(css.damage_shield > 0)
	{
		attacker->ApplyRawDamage(css.damage_shield);
		amount -= css.damage_shield;
		if(amount < 0)
			amount = 0;
		/*
		int shieldAmount = (int)((float)amount * ((float)css.damage_shield / 100.0F));
		attacker->ApplyRawDamage(shieldAmount);
		amount -= shieldAmount;
		if(amount < 0)
			amount = 0;
		*/
	}
	if(css.bonus_health > 0)
	{
		int subt = Util::ClipInt(amount, 0, css.bonus_health);
		if(subt > 0 && subt == amount)  //Need to reserve at least 1 point of damage so it doesn't register as a miss.
			subt--;
		amount -= subt;

		//Hack to get frost shield to work (ability group:146).
		//Need to update the tally otherwise the stat will be reset when the buffs are
		//next processed.
		SubtractAbilityBuffStat(STAT::BONUS_HEALTH, -1, static_cast<float>(subt));
		//css.bonus_health -= subt;
		absorbedDamage += subt;
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::BONUS_HEALTH);
	}
	return amount;
}

void CreatureInstance :: OnApplyDamage(CreatureInstance *attacker, int amount)
{
	if(amount == 0)
		return;
	//Players are not given hate profiles, since they have no use for them.
	//Experience and objectives are based on enemy kills, not players.
	if(!(serverFlags & ServerFlags::Noncombatant) && (!(serverFlags & ServerFlags::IsPlayer) || IsValidForPVP()))
	{
		HateProfile *hprof = GetHateProfile();
		if(hprof != NULL)
		{
			//If the attacker is a sidekick, get the sidekick's officer.
			//This is so sidekick kills will register as experience for the
			//associated player.
			int CDefID = attacker->CreatureDefID;
			short level = attacker->css.level;
			if(attacker->AnchorObject != NULL)
			{
				CDefID = attacker->AnchorObject->CreatureDefID;
				level = attacker->AnchorObject->css.level;
			}

			//Higher classes of mobs have greatly skewed health.
			//When comparing damage to total health, this will cause an insignifant ratio.
			//Don't factor the rarity when calculating max health, so the hate gained will
			//be more reasonable.
			float maxHealth = (float)((css.constitution * HealthConModifier) + css.base_health);
			if(maxHealth < 1.0F)
				maxHealth = 1.0F;
			float hateAmount = ((float)amount / maxHealth) * 100.0F;  //Bump it over another decimal place.  On mobs with high health, it's not enough.

			if(hateAmount < 1.0F)
				hateAmount = 1.0F;
			
			//Note: the ability table mentions that a gain rate of 100 is 200% of normal. 
			if(attacker->css.hate_gain_rate != 0)
				hateAmount += Util::GetAdditiveFromIntegralPercent100((int)hateAmount, attacker->css.hate_gain_rate);
			hprof->Add(CDefID, level, amount, (int)hateAmount);
			SetServerFlag(ServerFlags::HateInfoChanged, true);
		}
	}
}

void CreatureInstance :: CheckStatusInterruptOnHit(void)
{
	if(HasStatus(StatusEffects::DAZE))
	{
		int index = _HasStatusList(StatusEffects::DAZE);
		if(index >= 0)
			activeStatusEffect[index].expireTime = activeStatusEffect[index].startTime + Global::DazeGuaranteeTime;

		//_RemoveStatusList(StatusEffects::DAZE);
	}
}

void CreatureInstance :: CancelInvisibility(void)
{
	//Certain status effects should be removed on hits.  This is called when ability damage
	//is finalized and applied.

	bool resetDist = false;
	if(HasStatus(StatusEffects::WALK_IN_SHADOWS))
	{
		_RemoveStatusList(StatusEffects::WALK_IN_SHADOWS);
		resetDist = true;
	}
	if(HasStatus(StatusEffects::INVISIBLE))
	{
		_RemoveStatusList(StatusEffects::INVISIBLE);
		resetDist = true;
	}
	if(resetDist == true)
	{
		css.invisibility_distance = 0.0F;
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
	}
}

void CreatureInstance :: CheckInterrupts(void)
{
	//Called whenever raw damage is applied.  If a channel or cast is in progress,
	//checks for setback or interrupt.
	if(ab[0].bPending == true)
	{
		if(ab[0].type == AbilityType::Cast)
		{
			if(randint(1, 1000) <= css.casting_setback_chance)
				CastSetback();
		}
		else if(ab[0].type == AbilityType::Channel)
		{
			int chance = css.channeling_break_chance + ab[0].interruptChanceMod;
			if(randint(1, 1000) <= chance)
				Interrupt();
		}
	}
}

void CreatureInstance :: CheckMovement(float movementStep)
{
	//Check if ability casts or channels should be interrupted.
	CheckMovementInterrupt();

	//Check the distance counters if using an invisibility state.
	if(HasStatus(StatusEffects::WALK_IN_SHADOWS) || HasStatus(StatusEffects::INVISIBLE))
	{
		css.invisibility_distance -= movementStep;
		if(css.invisibility_distance <= 0.0F)
		{
			css.invisibility_distance = 0.0F;
			if(HasStatus(StatusEffects::WALK_IN_SHADOWS))
				_RemoveStatusList(StatusEffects::WALK_IN_SHADOWS);
			else if(HasStatus(StatusEffects::INVISIBLE))
				_RemoveStatusList(StatusEffects::INVISIBLE);
		}
		//SetServerFlag(ServerFlags::InvisDistChanged, true);
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
	}
}

void CreatureInstance :: CheckMovementInterrupt(void)
{
	//Called when player character movement is processed.

	if(ab[0].bPending == false)
		return;
	if(ab[0].type == AbilityType::Cast || ab[0].type == AbilityType::Channel)
		Interrupt();
}

void CreatureInstance :: CastSetback(void)
{
	//Performs a casting setback.  This is applied to the primary ability.
	int size = PrepExt_AbilityActivate(GSendBuf, this, &ab[0], AbilityStatus::SETBACK);
	//actInst->LSendToAllSimulator(GSendBuf, size, -1);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	setbackCount++;
	switch(setbackCount)
	{
	case 1:
		ab[0].fireTime += 1000;
		break;
	case 2:
		ab[0].fireTime += 500;
		break;
	case 3:
	case 4:
		ab[0].fireTime += 250;
		break;
	}
}

bool CreatureInstance :: RegisterHostility(CreatureInstance *attacker, int hostility)
{
	if(attacker == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] RegisterHostility attacker is null");
		return false;
	}

	if(hostility < 1)
		return false;

	if(HasStatus(StatusEffects::DEAD) == true)
		return false;
	if(attacker->HasStatus(StatusEffects::DEAD) == true)
		return false;

	if(serverFlags & ServerFlags::Noncombatant)
		return false;

	Status(StatusEffects::IN_COMBAT, 5);
	Status(StatusEffects::IN_COMBAT_STAND, 5);

	//Everything below is for AI purposes and is not applicable to human players.
	if(serverFlags & ServerFlags::IsPlayer)
		return false;

	SetServerFlag(ServerFlags::LocalActive, true);

	CreatureInstance *oldTarget = CurrentTarget.targ;

	if(aiScript != NULL || aiNut != NULL)
	{
		if(((serverFlags & ServerFlags::CalledBack) == false) && ((serverFlags & ServerFlags::LeashRecall) == false))
		{
			if(CurrentTarget.targ == NULL)
			{
				SelectTarget(attacker);
				CurrentTarget.bInstigated = true;
			}
		}
	}

	/*
	// Only run a loyalty check if the former target was changed
	if(oldTarget == CurrentTarget.targ)
		return;
	*/

	if(serverFlags & ServerFlags::IsNPC)
	{
		if(spawnGen == NULL)
			return true;

		int loyaltyRadius = spawnGen->GetLoyaltyRadius();
		if(loyaltyRadius > 0)
			actInst->SendLoyaltyAggro(attacker, this, loyaltyRadius);
		
		if(spawnGen->HasLoyaltyLinks() == true)
		{
			actInst->SendLoyaltyLinks(attacker, this, spawnGen->spawnPoint);
		}
	}

	return true;
}

void CreatureInstance :: SetCombatStatus(void)
{
	if(HasStatus(StatusEffects::DEAD) == true)
		return;

	if(serverFlags & ServerFlags::Noncombatant)
		return;

	Status(StatusEffects::IN_COMBAT, 5);
	Status(StatusEffects::IN_COMBAT_STAND, 5);
	Untransform();
}

void CreatureInstance :: Untransform(void)
{
	if(transformCreatureId != 0) {
		if(transformAbilityId != 0)
			RemoveBuffsFromAbility(transformAbilityId, true);
		else
			CAF_Untransform();
	}
}

bool CreatureInstance :: NotSilenced(void)
{
	//Fails conditional test if Silenced.
	if(statusEffects[StatusEffectBitData[StatusEffects::SILENCE].arrayIndex] & StatusEffectBitData[StatusEffects::SILENCE].bit)
		return false;

	return true;
}

bool CreatureInstance :: Facing(bool useGroundLocation)
{
	if(ab[0].TargetCount == 0)
		return false;

	if(ab[0].TargetList[0] == this)
		return true;

	int targx = ab[0].TargetList[0]->CurrentX;
	int targz = ab[0].TargetList[0]->CurrentZ;
	
	//Hack for GTAE abilities
	if(useGroundLocation == true)
	{
		targx = (int)ab[0].x;
		targz = (int)ab[0].z;
	}
	int xlen = targx - CurrentX;
	int zlen = targz - CurrentZ;
	double trot = (double)atan2((double)xlen, (double)zlen);
	double crot = ((double)Rotation * DOUBLE_PI / 256.0);

	double ToMove;
	double vsrc = trot + PI;
	double vend = crot + PI;

	if(vsrc <= vend)
		ToMove = vend - vsrc;
	else
		ToMove = ((2 * PI) - vsrc) + vend;

	if((ToMove <= g_FacingTarget) || (ToMove >= (2 * PI) - g_FacingTarget))
		return true;

	return false;
}

bool CreatureInstance :: isTargetInCone(CreatureInstance * target, double amount)
{
	if(target == NULL || target == this)
		return false;

	int xlen = target->CurrentX - CurrentX;
	int zlen = target->CurrentZ - CurrentZ;
	double trot = (double)atan2((double)xlen, (double)zlen);
	double crot = ((double)Rotation * DOUBLE_PI / 256.0);

	double ToMove;
	double vsrc = trot + PI;
	double vend = crot + PI;

	if(vsrc <= vend)
		ToMove = vend - vsrc;
	else
		ToMove = ((2 * PI) - vsrc) + vend;

	if((ToMove <= amount) || (ToMove >= (2 * PI) - amount))
		return true;

	return false;
}

bool CreatureInstance :: CheckBuffLimits(int level, int buffCategory, bool addBuff)
{
	//This emulated function differs from the official game function by
	//the addition of an optional parameter.
	//if <addBuff> is true, the current buff list will be checked for a match.
	//If the buff is not found, it is added.  This allows onActivate() usage to differ
	//from onRequest() [which is strictly check only]
	//Return false if the ability is already in the list.

	// NOT CURRENTLY USING THIS CHECK
	return true;
	//  NOTE: The Add() function will check for buffs and block them instead.
}

bool CreatureInstance :: InRange(float dist, bool useGroundLocation)
{
	//Must have at least one target in the list.
	if(ab[0].TargetCount == 0)
		return false;

	CreatureInstance *target = ab[0].TargetList[0];
	if(target == NULL)
		return false;

	//Quick check for self, always in range.
	if(this == target)
		return true;


	//Hack for GTAE abilities
	if(useGroundLocation == true)
	{
		float locDist = (float)actInst->GetPointRangeXZ(this, ab[0].x, ab[0].z, (int)dist);
		return (locDist <= dist);
	}
	
	//Don't include Y axis since the server doesn't know about elevation differences.
	//Also include hack to check for GTAE cast types which use a point location rather than
	//creature distance.

	float tolerance = GetTotalSize() + target->GetTotalSize();
	float acceptDist = tolerance + dist;

	float actualDist = (float)ActiveInstance::GetPlaneRange(this, target, (int)acceptDist);
	if(actualDist <= acceptDist)
		return true;

	return false;
}

bool CreatureInstance :: InRange_Target(float dist)
{
	if(CurrentTarget.targ == NULL)
		return false;

	float tolerance = GetTotalSize() + CurrentTarget.targ->GetTotalSize();
	float acceptDist = tolerance + dist;

	//Don't include Y axis since the server doesn't know about elevation differences.
	float actualDist = (float)actInst->GetPlaneRange(this, CurrentTarget.targ, (int)acceptDist);
	if(actualDist <= acceptDist)
		return true;

	return false;
}

bool CreatureInstance :: Will(int amount)
{
	return css.will >= amount;
}

bool CreatureInstance :: Might(int amount)
{
	return css.might >= amount;
}

int CreatureInstance :: WillCharge(int min, int max)
{
	if(css.will_charges >= min)
		return Util::ClipInt(css.will_charges, min, max);
	
	return 0;
	/*
	//Test if the current number of will charges are within the required amounts
	if(css.will_charges < min)
		return false;

	int amount = Util::ClipInt(css.will_charges, min, max);
	ab[0].willCharges = (char)amount;
	return true;
	*/
}

int CreatureInstance :: MightCharge(int min, int max)
{
	if(css.might_charges >= min)
		return Util::ClipInt(css.might_charges, min, max);
	
	return 0;
}

int CreatureInstance :: AdjustWill(int amount)
{
	_LimitAdjust(css.will, amount, 0, ABGlobals::MAX_WILL);
	bOrbChanged = true;
	return 1;
}


int CreatureInstance :: Status(int statusID, float durationSec)
{
	//Sets a status effect for a specified duration.
	int ms = 0;
	if(durationSec > 0.0)
		ms = (int)(durationSec * 1000.0);
	else
		ms = -1;
	_AddStatusList(statusID, ms);
	return 0;
}

bool CreatureInstance :: NotStatus(int statusID)
{
	//Fails conditional test if the given status is in effect.
	if(statusEffects[StatusEffectBitData[statusID].arrayIndex] & StatusEffectBitData[statusID].bit)
		return false;

	return true;
}

bool CreatureInstance :: HasStatus(int statusID)
{
	//Fails conditional test if the given status is not in effect.
	if(statusEffects[StatusEffectBitData[statusID].arrayIndex] & StatusEffectBitData[statusID].bit)
		return true;

	return false;
}



void CreatureInstance :: _SetStatusFlag(int statusID)
{
	statusEffects[StatusEffectBitData[statusID].arrayIndex] |= StatusEffectBitData[statusID].bit;
	/*
	if(send == true)
		SendFlags();
	*/
}

void CreatureInstance :: _ClearStatusFlag(int statusID)
{
	statusEffects[StatusEffectBitData[statusID].arrayIndex] &= ~StatusEffectBitData[statusID].bit;
	//if(send == true)
	//	SendFlags();
}

void CreatureInstance :: SendFlags(void)
{
	_CountStatusFlags();
	int size = PrepExt_UpdateMods(GSendBuf, this);
	//actInst->LSendToAllSimulator(GSendBuf, size, -1);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
}

int CreatureInstance :: _CountStatusFlags(void)
{
	int count = 0;
	int a, b;
	for(a = 0; a < MAX_STATUSEFFECTBYTES; a++)
	{
		for(b = 0; b < 32; b++)
			if(statusEffects[a] & (1 << b))
				count++;
	}
	return count;
}

void CreatureInstance :: SetServerFlag(unsigned long flag, bool status)
{

	if(status == true) {
		serverFlags |= flag;
	}
	else {
		serverFlags &= (~(flag));
	}
}

bool CreatureInstance :: Reagent(int itemID, int amount)
{
	//Placeholder.  Just assume we have the reagents needed to cast.
	return true;
}

int CreatureInstance :: _GetBaseStatModIndex(int statID)
{
	for(size_t i = 0; i < baseStats.size(); i++)
		if(baseStats[i].StatID == statID)
			return i;
	return -1;
}


//Use a negative amount for <abgID> for an indescriminate search for any stats granted by
//an ability. 
int CreatureInstance :: _GetExistingModIndex(int type, int groupID, int statID)
{
	for(size_t i = 0; i < activeStatMod.size(); i++)
	{
		if(type == BuffSource::ABILITY)
		{
			if((activeStatMod[i].abilityGroupID == groupID) || (groupID == -1))
				if(activeStatMod[i].modStatID == statID)
					return i;
		}
		else
		{
			if(activeStatMod[i].sourceID == groupID)
				if(activeStatMod[i].modStatID == statID)
					return i;
		}
	}
	return -1;
}

int CreatureInstance :: Add(unsigned char tier, unsigned char buffCategory, int abID, int abgID, int statID, float calcAmount, float descAmount, float durationSec)
{
	//This emulated, centralized function has four additional parameters:
	//tier and buffType, so that CheckBuffLimits can scan the buff array for existing buffs.
	//abID and abgID, so the mod lookup can assign the proper ability ID, and search
	//the proper ability group (so that different tier buffs don't stack).

	//Note: the Add() skill action calls this directly.

	//Also, because this function currently performs the necessary work of adding
	//a fixed stat increase, Amp() and AmpCore() will call this function after
	//calculating the fixed amount from the required stat percentage.

	//descAmount is NOT used for calculations anywhere.  Its sole purpose is to describe the input
	//amount or ratio provided by an ability function, so that the client can properly display the
	//values in the skill icon.

	int r = buffManager.HasBuff(tier, buffCategory);
	if(r >= 0)
		return -1;


	//Check certain stats for debuffs.  If it's a debuff, we'll want to make it slightly more effective
	//by resetting the current timer.
	switch(statID)
	{
	case STAT::MIGHT_REGEN:
		if(calcAmount < 0.0F)
			timer_mightregen = g_ServerTime + 2000;
		break;
	case STAT::WILL_REGEN:
		if(calcAmount < 0.0F)
			timer_willregen = g_ServerTime + 2000;
		break;
	case STAT::MELEE_ATTACK_SPEED:
		if(calcAmount < 0)
			timer_autoattack = g_ServerTime + 2000;
		break;
	}

	RemoveAbilityBuffTypeExcept(buffCategory, abID, abgID);


	ActiveBuff *activeBuff = buffManager.UpdateBuff(tier, buffCategory, abID, abgID, durationSec, initialisingAbilities);
	
	AddBuff(BuffSource::ABILITY, buffCategory, tier, abID, abgID, statID, calcAmount, descAmount, activeBuff->durationS);

	//HACK: the Pierce ability needs to update the stats immediately because it depends on the
	//updated value immediately in within the calling block.
	if(abgID == 116)
	{
		int br = _GetBaseStatModIndex(statID);
		if(br >= 0)
		{
			float total = baseStats[br].fBaseVal + baseStats[br].fModTotal;
			WriteValueToStat(baseStats[br].StatID, total, &css);
		}
	}

	return 1;
}

//Add a new buff entry, or extend an existing one if it matches the same source and group.
//Ability buffs of alternate tiers or of similar ability groups must not stack with each other,
//so they need some way to identify what they are.
//    buffSource   corresponds to BuffSource enum
//  buffCategory   for abilities
//        abTier   ability tier, used to allow overrides of lesser tiers
//          abID   ability ID
//     abGroupID   ability group ID (prevents similar tiers or similar buff types from stacking)
//        statID   ID of the stat to modify
//    calcAmount   Amount to modify the stat by
//   durationSec   Time (seconds) for the buff to last

void CreatureInstance :: AddBuff(int buffSource, int buffCategory, int abTier, int abID, int abGroupID, int statID, float calcAmount, float descAmount, float durationSec)
{
	if(descAmount == 0.0F && calcAmount != 0.0F)
		descAmount = calcAmount;

	unsigned long expireTimeMS = 0;

	//Calculate the internal expiration time.  -1 indicates infinite duration.
	if(Util::DoubleEquivalent(durationSec, -1.0F) == false)
		expireTimeMS = g_ServerTime + ( ( unsigned long)durationSec * 1000UL);
	else
		expireTimeMS = PlatformTime::MAX_TIME;

 	//Check to see if the ability buff already exists.
	int r = _GetExistingModIndex(buffSource, abGroupID, statID);
	if(r >= 0)
	{
		float oldAmount = activeStatMod[r].amount;
		//Replace and update the specific entry.
		activeStatMod[r].amount = calcAmount;
		activeStatMod[r].clientAmount = calcAmount;
		activeStatMod[r].expireTime = expireTimeMS;

		//Update the existing value in the core tally.
		int br = _GetBaseStatModIndex(statID);
		if(br >= 0)
		{
			baseStats[br].fModTotal -= oldAmount;
			baseStats[br].fModTotal += calcAmount;
		}
	}
	else
	{
		//Create a new mod entry.
		ActiveStatMod newMod;
		newMod.abilityID = abID;
		newMod.abilityGroupID = abGroupID;
		newMod.modStatID = statID;
		newMod.amount = calcAmount;
		newMod.clientAmount = descAmount;
		newMod.expireTime = expireTimeMS;
	
		newMod.priority = 1;
		
		char res = StatManager::GetStatType(statID);
		if(res == StatType::FLOAT)
		{
			newMod.priority = 1;
			newMod.clientAmount = descAmount;
		}

		newMod.tier = abTier;
		newMod.buffCategory = buffCategory;

		newMod.sourceType = buffSource;
		newMod.sourceID = 0;

		if(buffSource == BuffSource::ITEM)  //Experimental hack to try and fix the client readouts..
		{
			newMod.priority = 0;
			newMod.abilityID = 0;
			newMod.clientAmount = calcAmount;
		}

		PushStatMod(newMod);


		AddBaseStatMod(statID, calcAmount);
	}
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_MOD | CREATURE_UPDATE_STAT, this, statID);
}

void CreatureInstance :: PushStatMod(ActiveStatMod &newMod) {
	activeStatMod.push_back(newMod);
}

void CreatureInstance :: RemoveStatModsBySource(int buffSource)
{
	size_t pos = 0;
	int count = 0;
	while(pos < activeStatMod.size())
	{
		if(activeStatMod[pos].sourceType == buffSource)
		{
				RemoveBuffIndex(pos);
				count++;
		}
		else
			pos++;
	}
	if(count > 0)
		SendUpdatedBuffs();
}

ActiveBuff * CreatureInstance :: AddMod(unsigned char tier, int buffCategory, short abID, short abgID, double durationSec)
{
	ActiveStatMod newMod;
	newMod.sourceType = BuffSource::ABILITY;
	newMod.abilityID = abID;
	newMod.priority = 1;
	newMod.amount = 0;
	newMod.clientAmount = newMod.amount;

	ActiveBuff * buff = buffManager.UpdateBuff(tier, buffCategory, abID, abgID, durationSec, initialisingAbilities);

	// Set the expire time now, the active buff may have come from a persistent buff
	newMod.expireTime = buff->castEndTimeMS;

	// Signal update of status effects
	PushStatMod(newMod);
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_MOD, this, 0);

	return buff;
}


void CreatureInstance :: AddItemStatMod(int itemID, int statID, float amount)
{
	ActiveStatMod newMod;
	newMod.sourceType = BuffSource::ITEM;
	newMod.sourceID = itemID;
	newMod.modStatID = statID;
	newMod.amount = amount;
	newMod.clientAmount = newMod.amount;
	newMod.expireTime = PlatformTime::MAX_TIME;
	PushStatMod(newMod);
	
	AddBaseStatMod(statID, amount);
}

void CreatureInstance :: ApplyItemStatModFromConfig(int itemID, const std::string configStr)
{
	STRINGLIST modentry;
	STRINGLIST moddata;
	Util::Split(configStr, "&", modentry);
	for(size_t i = 0; i < modentry.size(); i++)
	{
		Util::Split(modentry[i], "=", moddata);
		if(moddata.size() >= 2)
		{
			int index = GetStatIndexByName(moddata[0].c_str());
			if(index >= 0)
			{
				if(StatList[index].isNumericalType() == true)
				{
					int statID = StatList[index].ID;
					float value = static_cast<float>(atof(moddata[1].c_str()));
					AddItemStatMod(itemID, statID, value);
				}
			}
		}
	}
}


//Use a negative amount for <abgID> for an indescriminate search of the first stat. 
void CreatureInstance :: SubtractAbilityBuffStat(int statID, int abgID, float amount)
{
	int r = _GetExistingModIndex(BuffSource::ABILITY, abgID, statID);
	if(r >= 0)
	{
		float newAmount = activeStatMod[r].amount - amount;
		activeStatMod[r].amount = newAmount;
		activeStatMod[r].clientAmount = newAmount;

		//Hack to cancel the Frost Shield ability when the shield is depleted.
		//Due to the official formula, it may not be a full integer, so quit if
		//it falls below a full point.
		if(statID == STAT::BONUS_HEALTH && (newAmount < 1.0F))
		{
			activeStatMod[r].expireTime = g_ServerTime;
			pendingOperations.UpdateList_Add(CREATURE_UPDATE_MOD | CREATURE_UPDATE_STAT, this, statID);
		}
	}

	int br = _GetBaseStatModIndex(statID);
	if(br >= 0)
	{
		baseStats[br].fModTotal -= amount;
		float newTotal = baseStats[br].fBaseVal + baseStats[br].fModTotal;
		WriteValueToStat(baseStats[br].StatID, newTotal, &css);
	}
}

void CreatureInstance :: SendUpdatedBuffs(void)
{
	for(size_t a = 0; a < baseStats.size(); a++)
	{
		//TODO: Both write calls perform a stat index lookup, this is inefficient.
		float total = baseStats[a].fBaseVal + baseStats[a].fModTotal;
		WriteValueToStat(baseStats[a].StatID, total, &css);
	}
	CheckRemovedBuffs();

	int wpos = 0;
	//wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "SendUpdatedBuffs before", INFOMSG_INFO);
	wpos += PrepExt_UpdateMods(&GSendBuf[wpos], this);
	//wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "SendUpdatedBuffs after", INFOMSG_INFO);
	//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);

}

void CreatureInstance :: CheckRemovedBuffs(void)
{
	//Check for buffs that don't exist any more (zero references).
	//Remove buffs and restore timers if applicable.

	size_t i = 0;
	while(i < baseStats.size())
	{
		if(baseStats[i].RefCount == 0)
		{
			//Restore the might/will regeneration timers.
			if(baseStats[i].StatID == STAT::WILL_REGEN)
			{
				if(timer_willregen > g_ServerTime + 2000)
					timer_willregen = g_ServerTime + 2000;
			}
			else if(baseStats[i].StatID == STAT::MIGHT_REGEN)
			{
				if(timer_mightregen > g_ServerTime + 2000)
					timer_mightregen = g_ServerTime + 2000;
			}

			pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, baseStats[i].StatID);
			WriteValueToStat(baseStats[i].StatID, baseStats[i].fBaseVal, &css);

			baseStats.erase(baseStats.begin() + i);
		}
		else
			i++;
	}
}

void CreatureInstance :: AddBaseStatMod(int statID, float amount)
{
	int br = _GetBaseStatModIndex(statID);
	if(br >= 0)
	{
		//Add to the running total.
		baseStats[br].fModTotal += amount;
		baseStats[br].RefCount++;
	}
	else
	{
		//Create a new base entry.
		bool core = false;
		switch(statID)
		{
		case STAT::STRENGTH:
		case STAT::DEXTERITY:
		case STAT::CONSTITUTION:
		case STAT::PSYCHE:
		case STAT::SPIRIT:
			core = true;
		};


		float base = 0;
		if(core == true)
		{
			base = _GetBaseStat(statID);
		}
		else
			base = GetStatValueByID(statID, &css);

		BaseStatData newBase;
		newBase.valueType = StatManager::GetStatType(statID);
		newBase.StatID = statID;
		newBase.fBaseVal = base;
		newBase.fModTotal = amount;
		newBase.RefCount = 1;
		baseStats.push_back(newBase);
	}
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, statID);
}


//Arbitrarily subtract an amount from the base stat.
void CreatureInstance :: SubtractBaseStatMod(int statID, float amount)
{
	int br = _GetBaseStatModIndex(statID);
	if(br >= 0)
	{
		baseStats[br].fModTotal -= amount;
		baseStats[br].RefCount--;
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, statID);
		//RefCount is checked for deletions after the updated stats have
		//been sent the client.
	}
}

void CreatureInstance :: UpdateBaseStatMinimum(int statID, float amount)
{
	int br = _GetBaseStatModIndex(statID);
	if(br >= 0)
	{
		baseStats[br].fBaseVal = amount;
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, statID);
	}
}

// Subtract an active buff and delete it from the active list.  The calling function is
// responsible for iterating or otherwise correctly determinating a valid index.
void CreatureInstance :: RemoveBuffIndex(size_t index)
{
	if(activeStatMod[index].abilityID != 0) {
		g_AbilityManager.ActivateAbility(this, activeStatMod[index].abilityID, EventType::onDeactivate, &ab[0]);
	}
	SubtractBaseStatMod(activeStatMod[index].modStatID, activeStatMod[index].amount);
	activeStatMod.erase(activeStatMod.begin() + index);
}


// Remove all buffs
void CreatureInstance :: RemoveBuffsFromAbility(int abilityID, bool send)
{
	size_t i = 0;
	while(i < activeStatMod.size())
		if(activeStatMod[i].abilityID == abilityID)
			RemoveBuffIndex(i);
		else
			i++;
	buffManager.RemoveBuff(abilityID);

	if(send == true)
		SendUpdatedBuffs();
}

void CreatureInstance :: RemoveAllBuffs(bool send)
{
	while(activeStatMod.size() > 0)
	{
		RemoveBuffIndex(0);
	}

	for(size_t i = 0; i < baseStats.size(); i++)
	{
		float value = baseStats[i].fBaseVal;
		WriteValueToStat(baseStats[i].StatID, value, &css);
	}
	baseStats.clear();
}

void CreatureInstance :: RemoveAllBuffsExceptGroupID(bool send, int abilityGroupID)
{
	size_t pos = 0;
	while(pos < activeStatMod.size())
	{
		if(activeStatMod[pos].abilityGroupID != abilityGroupID)
			RemoveBuffIndex(pos);
		else
			pos++;
	}

	for(size_t i = 0; i < baseStats.size(); i++)
	{
		float value = baseStats[i].fBaseVal;
		WriteValueToStat(baseStats[i].StatID, value, &css);
	}
	baseStats.clear();
}

//Remove all buffs of a certain category that do not match the ability.
void CreatureInstance :: RemoveAbilityBuffTypeExcept(int buffCategory, int abilityID, int abilityGroupID)
{
	//Hack: not sure if it's the new system, but this was being triggered by passive ability buffs
	//without an assigned buff category (buffCategory == 0).
	//This was causing the activation of some passive buffs to cancel existing unrelated passive buffs.
	if(buffCategory == 0)
		return;

	size_t pos = 0;
	bool del = false;
	bool abilityCanceled = false;
	while(pos < activeStatMod.size())
	{
		del = false;
		if(activeStatMod[pos].sourceType == BuffSource::ABILITY && activeStatMod[pos].buffCategory == buffCategory)
		{
			if(activeStatMod[pos].abilityID != abilityID)
				del = true;
		}
		if(del == true)
		{
			RemoveBuffIndex(pos);
			abilityCanceled = true;
		}
		else
			pos++;
	}
	if(abilityCanceled == true)
	{
		int wpos = PrepExt_AbilityEvent(GSendBuf, CreatureID, abilityID, AbilityStatus::INTERRUPTED);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
	}
}

// Note: One of the following conditions should be true for sign:
//  (sign < 0.0F)  ||  (sign > 0.0F)
bool CreatureInstance :: RemoveAbilityBuffWithStat(int statID, float sign)
{
	size_t pos = 0;
	bool del = false;
	bool abilityCanceled = false;
	int foundAbilityID = 0;
	while(pos < activeStatMod.size())
	{
		del = false;
		if(activeStatMod[pos].sourceType == BuffSource::ABILITY && activeStatMod[pos].modStatID == statID)
		{
			if((sign < 0.0F && activeStatMod[pos].amount < 0.0F) || (sign > 0.0F && activeStatMod[pos].amount > 0.0F))
			{
				//if((activeStatMod[pos].abilityID > 0 && activeStatMod[pos].abilityID == foundAbilityID))
				//{
					foundAbilityID = activeStatMod[pos].abilityID;
					RemoveBuffIndex(pos);
					//abilityCanceled = true;
					del = true;
				//}
			}
		}
		if(del == false)
			pos++;
	}
	
	//Check to see if any mods are still active for this ability ID.
	if(foundAbilityID != 0)
	{
		int count = 0;
		for(size_t i = 0; i < activeStatMod.size(); i++)
		{
			if(activeStatMod[i].sourceType == BuffSource::ABILITY && activeStatMod[i].abilityID == foundAbilityID)
				count++;
		}

		//If there are none, notify to remove the buff.
		if(count == 0)
		{
			int wpos = PrepExt_AbilityEvent(GSendBuf, CreatureID, foundAbilityID, AbilityStatus::INTERRUPTED);
			actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
			SendUpdatedBuffs();
			return true;
		}
	}
	return false;
}


int CreatureInstance :: CalcRestrictedHealth(int health, bool addmod)
{
	if(health == -1)
		 health = GetMaxHealth(addmod);
	if(css.max_health_pc != 100) {
		int maxHealth = (int)( ( (float)GetMaxHealth(addmod) / 100.0) * css.max_health_pc );
		if(health > maxHealth)
			health = maxHealth;
	}
	return health;
}

int CreatureInstance :: GetMaxHealth(bool addmod)
{
	int rarity = Util::ClipInt(css.rarity, 0, MAX_RARITY_INDEX);
	
	int health = css.base_health + (css.constitution * HealthConModifier);
	health = (int)((float)health * RarityTypeHealthModifier[rarity]);

	//int health = (int)(css.constitution * HealthConModifier * RarityTypeHealthModifier[rarity]);

	if(addmod == true)
		health += css.health_mod;
	/*
	if(css.heroism > 0 && addmod == true)
		health += Util::GetAdditiveFromIntegralPercent10000(health, css.heroism);
		//health += (int)(((double)css.heroism / 10000.0F) * (double)health);
	*/
	return health;
}

void CreatureInstance :: LimitValueOverflows(void)
{
	const int MAX_SHORT = Platform::MAX_SHORT;
	const int MAX_INT = Platform::MAX_INT;

	int healthValueCap = ((g_Config.UseIntegerHealth == true) ? MAX_INT : MAX_SHORT);
		
	int max = GetMaxHealth(true);
	if(max > healthValueCap)
	{
		//Calculate how much constitution needs to be subtracted to diminish the health just enough
		//so it's below the overflow threshold.
		int overflowHealth = max - healthValueCap;
		int rarity = Util::ClipInt(css.rarity, 0, MAX_RARITY_INDEX);
		float healthPerCon = HealthConModifier * RarityTypeHealthModifier[rarity];
		float trimCon = (overflowHealth / healthPerCon) + 1;
		css.constitution -= (int)trimCon;
		int newHP = CalcRestrictedHealth(-1, true);
		css.health = newHP;
		g_Log.AddMessageFormat("[WARNING] Constitution overflow! OldHP:%d, ConReduct:%d, NewHP:%d", max, (int)trimCon, newHP);
	}

	css.damage_resist_melee = Util::ClipInt(css.damage_resist_melee, 0, MAX_SHORT);
	css.damage_resist_fire = Util::ClipInt(css.damage_resist_fire, 0, MAX_SHORT);
	css.damage_resist_frost = Util::ClipInt(css.damage_resist_frost, 0, MAX_SHORT);
	css.damage_resist_mystic = Util::ClipInt(css.damage_resist_mystic, 0, MAX_SHORT);
	css.damage_resist_death = Util::ClipInt(css.damage_resist_death, 0, MAX_SHORT);

	css.base_damage_melee = Util::ClipInt(css.base_damage_melee, 0, MAX_SHORT);
}

float CreatureInstance :: GetHealthRatio(void)
{
	return static_cast<float>(css.health) / static_cast<float>(GetMaxHealth(true));
}

void CreatureInstance :: RunHealTick(void)
{
	double regen = (css.spirit * RegenSpiritMod) + (css.constitution * RegenConMod) + (css.level * RegenLevelMod);
	regen += css.mod_health_regen;
	regen *= css.health_regen;
	if(HasStatus(StatusEffects::IN_COMBAT))
	{
		regen *= RegenCombatModifier;
		if(regen < 1)
			regen = 1;
	}

	int maxhealth = GetMaxHealth(true);
	int health = css.health + (int)regen;
	if(health > maxhealth)
		health = maxhealth;
	css.health = CalcRestrictedHealth(health, true);
	int size = PrepExt_SendHealth(GSendBuf, CreatureID, css.health);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
}

void CreatureInstance :: Heal(int amount)
{
	if(HasStatus(StatusEffects::DEAD) == true)
		return;

	int maxhealth = GetMaxHealth(true);
	int health = css.health + amount;
	int chealth = css.health;
	if(health > maxhealth)
		health = maxhealth;
	css.health = CalcRestrictedHealth(health, true);
	int size = PrepExt_SendHealth(GSendBuf, CreatureID, css.health);
	//actInst->LSendToAllSimulator(GSendBuf, size, -1);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);

	size = 0;
	amount = health - chealth;
	if(amount != 0)
	{
		size += PutByte(&GSendBuf[size], 4);
		size += PutShort(&GSendBuf[size], 0);
		size += PutInteger(&GSendBuf[size], CreatureID);
		size += PutByte(&GSendBuf[size], 15);
		//if(g_ProtocolVersion > 25)
			size += PutInteger(&GSendBuf[size], CreatureID);
		size += PutInteger(&GSendBuf[size], amount);
		PutShort(&GSendBuf[1], size - 3);
		//actInst->LSendToAllSimulator(GSendBuf, size, -1);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	}
}

bool CreatureInstance :: CanUseAnyWeapon(void)
{
	if((serverFlags & ServerFlags::IsNPC) || (serverFlags & ServerFlags::IsSidekick))
		return true;
	return false;
}

bool CreatureInstance :: hasMainHandWeapon(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_MAIN_HAND] != WeaponType::NONE);
}
bool CreatureInstance :: hasOffHandWeapon(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_OFF_HAND] != WeaponType::NONE);
}
bool CreatureInstance :: hasMeleeWeapon(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_MAIN_HAND] != WeaponType::NONE);
}

bool CreatureInstance :: hasRangedWeapon(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_RANGED] != WeaponType::NONE);
}

bool CreatureInstance :: hasShield(void)
{
	//Placeholder
	return true;
}

bool CreatureInstance :: hasBow(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_RANGED] == WeaponType::BOW);
}

bool CreatureInstance :: hasWand(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return (EquippedWeapons[ItemEquipSlot::WEAPON_RANGED] == WeaponType::WAND);
}

bool CreatureInstance :: has2HorPoleWeapon(void)
{
	if(CanUseAnyWeapon() == true) { return true; }
	return ( (EquippedWeapons[ItemEquipSlot::WEAPON_MAIN_HAND] == WeaponType::TWO_HAND) ||
		(EquippedWeapons[ItemEquipSlot::WEAPON_MAIN_HAND] == WeaponType::POLE) );
}

void CreatureInstance :: AddHate(CreatureInstance *attacker, int amount)
{
	//Note: the original function only takes one parameter, the amount of hate to add.
	//This emulated function needs to know the source of who added the hate.
	HateProfile *hprof = GetHateProfile();
	if(hprof == NULL)
		return;
	if(!(serverFlags & ServerFlags::Noncombatant)) {
		hprof->Add(attacker->CreatureDefID, attacker->css.level, 0, amount);
		hprof->SetImmediateRefresh();
		SetServerFlag(ServerFlags::HateInfoChanged, true);
	}
}

void CreatureInstance :: Taunt(CreatureInstance *attacker, int seconds)
{
	//Note: the original function only takes one parameter, the amount of hate to add.
	//This emulated function needs to know the source of who taunted the creature.

	//Note: register hostility first to instigate linked mobs.  As of implementing this, loyalty
	//instigation will not be invoked if the creature already has a target.
	if(RegisterHostility(attacker, 1)) {

		SelectTarget(attacker);

		HateProfile *hprof = GetHateProfile();
		if(hprof == NULL)
			return;
		hprof->ExtendTauntRelease(seconds);
		SetServerFlag(ServerFlags::HateInfoChanged, true);
		SetServerFlag(ServerFlags::Taunted, true);
	}
}

void CreatureInstance :: Amp(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, float percent, int time)
{
	//Get integer value, run the percent, and forward that amount and everything
	//else into the Add() function, which handles the actual buff management.

	float amount = 0.0F;
	switch(statID)
	{
	case STAT::DAMAGE_RESIST_MELEE:
	case STAT::DAMAGE_RESIST_FIRE:
	case STAT::DAMAGE_RESIST_FROST:
	case STAT::DAMAGE_RESIST_MYSTIC:
	case STAT::DAMAGE_RESIST_DEATH:
		amount = _GetTotalStat(statID, abgID);
		break;

	case STAT::STRENGTH:
	case STAT::DEXTERITY:
	case STAT::CONSTITUTION:
	case STAT::PSYCHE:
	case STAT::SPIRIT:
		if(g_Config.CustomAbilityMechanics == true)
			amount = GetStatValueByID(statID, &css);
		else
			amount = _GetBaseStat(statID);
		break;

	default:
		amount = _GetBaseStat(statID);
	}

	int statIndex = GetStatIndex(statID);
	if(statIndex == -1)
		return;

	amount *= percent;

	//Round integral types, but not floats.  Float stats like MIGHT_REGEN and WILL_REGEN in particular
	//explicitly require floating point values to function correctly.
	if((StatList[statIndex].etype == StatType::SHORT) || (StatList[statIndex].etype == StatType::INTEGER))
		amount = Util::Round(amount);

	Add(tier, buffType, abID, abgID, statID, amount, percent, time);
}

void CreatureInstance :: Set(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, float amount, int time)
{
	//Get integer value, run the percent, and forward that amount and everything
	//else into the Add() function, which handles the actual buff management.

	int statIndex = GetStatIndex(statID);
	if(statIndex == -1)
		return;

	float val = GetStatValueByID(statID, &css);
	val = -val + amount;

	//Round integral types, but not floats.  Float stats like MIGHT_REGEN and WILL_REGEN in particular
	//explicitly require floating point values to function correctly.
	if((StatList[statIndex].etype == StatType::SHORT) || (StatList[statIndex].etype == StatType::INTEGER))
		amount = Util::Round(amount);

	Add(tier, buffType, abID, abgID, statID, amount, val, time);
}

void CreatureInstance :: WalkInShadows(int duration, int counter)
{
	_RemoveStatusList(StatusEffects::WALK_IN_SHADOWS);
	if(serverFlags & ServerFlags::IsPlayer)
	{
		int wpos = PrepExt_UpdateMods(GSendBuf, this);
		actInst->LSendToOneSimulator(GSendBuf, wpos, simulatorPtr);
	}
	_AddStatusList(StatusEffects::WALK_IN_SHADOWS, duration * 1000);
	css.invisibility_distance = GetMaxInvisibilityDistance(true);
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
}

void CreatureInstance :: Invisible(int duration, int counter, int unknown)
{
	_RemoveStatusList(StatusEffects::INVISIBLE);
	if(serverFlags & ServerFlags::IsPlayer)
	{
		int wpos = PrepExt_UpdateMods(GSendBuf, this);
		actInst->LSendToOneSimulator(GSendBuf, wpos, simulatorPtr);
	}
	_AddStatusList(StatusEffects::INVISIBLE, duration * 1000);
	css.invisibility_distance = GetMaxInvisibilityDistance(false);
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
}

void CreatureInstance :: GoSanctuary(void)
{
	UnHate();

	//WorldCoord* sanct = g_ZoneMarkerDataManager.GetNearestSanctuaryInZone(actInst->mZone, CurrentX, CurrentZ); 
	const char *regionName = MapLocation.GetInternalMapName(actInst->mZone, CurrentX, CurrentZ);
	WorldCoord* sanct = g_ZoneMarkerDataManager.GetNearestRegionSanctuaryInZone(regionName, actInst->mZone, CurrentX, CurrentZ); 
	if(sanct != NULL)
	{
		CurrentX = (int)sanct->x;
		CurrentY = (int)sanct->y + SANCTUARY_ELEVATION_ADDITIVE;
		CurrentZ = (int)sanct->z;
		int wpos = PrepExt_CreatureFullInstance(GSendBuf, this);
		//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
	}
	else
	{
		if(serverFlags & ServerFlags::IsPlayer)
		{
			int wpos = PrepExt_SendInfoMessage(GSendBuf, "No sanctuary in range", INFOMSG_ERROR);
			simulatorPtr->AttemptSend(GSendBuf, wpos);
		}
	}
}

void CreatureInstance :: OnUnstick(void)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	//This is a custom function to help assist in logging unsticks.
	Debug::Log("[UNSTICK] %s (%s, %d,%d,%d)", css.display_name, ZoneString, CurrentX, CurrentY, CurrentZ);
	bool quick = charPtr->NotifyUnstick(false);
	if(quick == true)
	{
		Debug::Log("[UNSTICK] Quick");
		
		/*
		if(CheckBuffLimits(10,BuffCategory::DBMove_Debuff_Rez, true) == true)
			AmpCore(10,BuffCategory::DBMove_Debuff_Rez, 10000, 74, -0.5, 120);
		*/
	}
}

void CreatureInstance :: RemoveAltBuff(void)
{
	//As per the skill description (for Suppress):
	//  Removes 1 of the following Buffs from the target : Increased Casting Speed, Increased Attack Speed,
	//  or Increased Movement Speed.
	const int stats[3] =  {STAT::MELEE_ATTACK_SPEED, STAT::MAGIC_ATTACK_SPEED, STAT::MOD_MOVEMENT};
	for(int i = 0; i < 3; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], 1.0F) == true)
			return;
	}
}

void CreatureInstance :: RemoveStatBuff(void)
{
	//As per the skill description (for Corrupt):
	//  Removes 1 Buff to Strength, Dexterity, Psyche, Spirit, Constitution, or Luck from the target.
	const int stats[6] = {
		STAT::STRENGTH, STAT::DEXTERITY, STAT::CONSTITUTION,
		STAT::PSYCHE,   STAT::SPIRIT,    STAT::MOD_LUCK  };
	for(int i = 0; i < 6; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], 1.0F) == true)
			return;
	}
}

void CreatureInstance :: RemoveHealthBuff(void)
{
	// Special skill for shield warmer.
	const int stats[3] = {
		STAT::BASE_HEALTH,    STAT::BONUS_HEALTH,    STAT::CONSTITUTION,
	};
	for(int i = 0; i < 3; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], 1.0F) == true)
			return;
	}
}

void CreatureInstance :: RemoveEtcBuff(void)
{
	//As per the skill description (for Abolish):
	//  Removes 1 Buff from the target, if it can be removed, but cannot be removed by Suppress or Corrupt.
	const int stats[10] = {
		STAT::BASE_DAMAGE_FIRE,    STAT::BASE_DAMAGE_FROST,    STAT::BASE_DAMAGE_MYSTIC,
		STAT::BASE_DAMAGE_DEATH,   STAT::DAMAGE_RESIST_MELEE,  STAT::DAMAGE_RESIST_FIRE,
		STAT::DAMAGE_RESIST_FROST, STAT::DAMAGE_RESIST_MYSTIC, STAT::DAMAGE_RESIST_DEATH,
		STAT::BASE_HEALTH
		/* These are sometimes removed by buff skills like Turtle or Berserk, so don't operate on them.
		STAT::DMG_MOD_MELEE
		STAT::DR_MOD_MELEE,        STAT::DR_MOD_FIRE,
		STAT::DR_MOD_FROST,         STAT::DR_MOD_MYSTIC,       STAT::DR_MOD_DEATH,
		*/
	};
	for(int i = 0; i < 10; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], 1.0F) == true)
			return;
	}
}

void CreatureInstance:: RemoveAltDebuff(void)
{
	//As per the skill description (for Free):
	//  Removes 1 of the following Debuffs: Reduced Casting Speed, Reduced Attack Speed,
	//  Reduced Movement Speed, Fear, Charm, Root. Can be Party Cast.
	const int stats[3] =  {STAT::MELEE_ATTACK_SPEED, STAT::MAGIC_ATTACK_SPEED, STAT::MOD_MOVEMENT};
	const int effects[3] = {StatusEffects::FEAR, StatusEffects::CHARM, StatusEffects::ROOT};
	for(int i = 0; i < 3; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], -1.0F) == true)
			return;
	}
	for(int i = 0; i < 3; i++)
	{
		if(HasStatus(effects[i]) == true)
		{
			_RemoveStatusList(effects[i]);
			return;
		}
	}
}

void CreatureInstance :: RemoveStatDebuff(void)
{
	//As per the skill description (for Cleanse):
	//  Removes 1 Debuff to Strength, Dexterity, Psyche, Spirit, Constitution, or Luck. Can be Party Cast.
	const int stats[6] = {
		STAT::STRENGTH, STAT::DEXTERITY, STAT::CONSTITUTION,
		STAT::PSYCHE,   STAT::SPIRIT,    STAT::MOD_LUCK  };
	for(int i = 0; i < 6; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], -1.0F) == true)
			return;
	}
}

void CreatureInstance :: RemoveEtcDebuff(void)
{
	//As per the skill description (for Purge):
	//  Removes 1 Debuff, if it can be removed, but cannot be removed by Purge or Cleanse. Can be Party Cast.
	const int stats[10] = {
		STAT::BASE_DAMAGE_FIRE,    STAT::BASE_DAMAGE_FROST,    STAT::BASE_DAMAGE_MYSTIC,
		STAT::BASE_DAMAGE_DEATH,   STAT::DAMAGE_RESIST_MELEE,  STAT::DAMAGE_RESIST_FIRE,
		STAT::DAMAGE_RESIST_FROST, STAT::DAMAGE_RESIST_MYSTIC, STAT::DAMAGE_RESIST_DEATH,
		STAT::BASE_HEALTH
		/* These are sometimes removed by buff skills like Turtle or Berserk, so don't operate on them.
		STAT::DMG_MOD_MELEE
		STAT::DR_MOD_MELEE,        STAT::DR_MOD_FIRE,
		STAT::DR_MOD_FROST,         STAT::DR_MOD_MYSTIC,       STAT::DR_MOD_DEATH,
		*/
	};
	for(int i = 0; i < 10; i++)
	{
		if(RemoveAbilityBuffWithStat(stats[i], -1.0F) == true)
			return;
	}
}

void CreatureInstance :: Go(const char *destname)
{
}

void CreatureInstance :: PortalRequest(CreatureInstance *caster, const char *internalname, const char *externalname)
{
	//This emulated function needs an extra parameter to determine who the caster's name is, so it
	//can display a prompt to the user that someone is trying to teleport them.
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	simulatorPtr->pld.SetPortalRequestDest(externalname, 0);

	if(caster == this)
		bcm.AddEvent2(simulatorPtr->InternalID, (long)simulatorPtr, 0, BCM_RunPortalRequest, actInst);
	else
	{
		int wpos = PrepExt_CreatureEventPortalRequest(GSendBuf, CreatureID, caster->css.display_name, externalname);
		actInst->LSendToOneSimulator(GSendBuf, wpos, simulatorPtr);
	}
}

void CreatureInstance :: BindTranslocate(void)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(actInst->mZone, CurrentX, CurrentZ, 250); 
	if(sanct == NULL)
	{
		int wpos = PrepExt_SendInfoMessage(GSendBuf, "You must be near a sanctuary to bind.", INFOMSG_ERROR);
		simulatorPtr->AttemptSend(GSendBuf, wpos);
	}
	else
	{
		charPtr->bindReturnPoint[0] = (int)sanct->x;
		charPtr->bindReturnPoint[1] = (int)sanct->y + SANCTUARY_ELEVATION_ADDITIVE;
		charPtr->bindReturnPoint[2] = (int)sanct->z;
		charPtr->bindReturnPoint[3] = actInst->mZone;
		
		css.translocate_destination = sanct->descName;
		if(sanct->descName.size() != 0)
		{
			SendStatUpdate(STAT::TRANSLOCATE_DESTINATION);
			std::string msg = "You have bound to ";
			msg.append(sanct->descName);
			int wpos = PrepExt_SendInfoMessage(GSendBuf, msg.c_str(), INFOMSG_INFO);
			simulatorPtr->AttemptSend(GSendBuf, wpos);
		}
	}
}

void CreatureInstance :: Resurrect(float healthratio, float luckratio, int abilityID)
{
	//Calculate heroism first since the max health will change.
	bool rezScreen = false;
	int rezIndex = -1;

	switch(abilityID)
	{
	case 10000: rezIndex = 0; rezScreen = true; break;
	case 10001: rezIndex = 1; rezScreen = true; break;
	case 10002: rezIndex = 2; rezScreen = true; break;
	}

	if(serverFlags & ServerFlags::IsPlayer)
	{
		if(actInst->mZoneDefPtr->HasDeathPenalty() == true)
		{
			//int baseluck = (int)((double)css.base_luck * luckratio);
			int heroism = (int)((float)css.heroism * luckratio);
			//css.base_luck = Util::ClipInt(baseluck, 0, MAX_BASE_LUCK);
			css.heroism = Util::ClipInt(heroism, 0, Global::MAX_HEROISM);
			OnHeroismChange();


			//Multiple abilities call this, but only check for and reduce cost if it's an option
			//from the resurrect screen.
			if(rezIndex != -1)
			{
				int cost = Global::GetResurrectCost(css.level, rezIndex);
				AdjustCopper(-cost);
			}
		}
	}

	int maxhealth = GetMaxHealth(true);
	int health = (int)((float)maxhealth * healthratio);

	//Just in case, prevent rounding errors.
	if(health < 1)
		health = 1;

	css.health = CalcRestrictedHealth(health, true);

	//_ClearStatusFlag(StatusEffects::DEAD, true);
	_RemoveStatusList(StatusEffects::DEAD);

	if(actInst->mZoneDefPtr->mArena == true && rezScreen == true)
	{
		_AddStatusList(StatusEffects::UNATTACKABLE, 6000);
		AddBuff(BuffSource::INSTANCE, 0, 0, 0, 0, STAT::MOD_MOVEMENT, 40, 40, 6);
	}

	css.hide_nameboard = 0;
	const short stats[4] = { STAT::HEALTH, STAT::HIDE_NAMEBOARD, STAT::BASE_LUCK, STAT::HEROISM };
	int size = PrepExt_SendSpecificStats(GSendBuf, this, &stats[0], 4);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
}

void CreatureInstance :: TopHated(int amount)
{
}

bool CreatureInstance :: StatValueLessThan(int statID, int amount)
{
	//Implemented into the ability list for Bounder Dash, after 0.8.6.
	if(GetStatValueByID(statID, &css) < (float)amount)
		return true;

	return false;
}

int CreatureInstance :: MWD(void)
{
	int amount = GetMainhandDamage();
	amount += GetOffhandDamage();

	return amount;
	//return (int)(css.strength * 0.3) + css.base_damage_melee + randint(MainDamage[0], MainDamage[1]);
}

int CreatureInstance :: RWD(void)
{
	int amount = (int)(css.strength * 0.3) + css.base_damage_melee + randint(RangedDamage[0], RangedDamage[1]);
	amount += GetAdditiveWeaponSpecialization(amount, ItemEquipSlot::WEAPON_RANGED);
	return amount;
	//return (int)(css.strength * 0.3) + css.base_damage_melee + randint(RangedDamage[0], RangedDamage[1]);
}

int CreatureInstance :: WMD(void)
{
	int amount = (int)(css.strength * 0.3) + css.base_damage_melee + randint(MainDamage[0], MainDamage[1]) + (int)(randint(OffhandDamage[0], OffhandDamage[1]) * ((float)css.offhand_weapon_damage / 1000.0F));
	amount += GetAdditiveWeaponSpecialization(amount, ItemEquipSlot::WEAPON_MAIN_HAND);
	return amount;
	//return (int)(css.strength * 0.3) + css.base_damage_melee + randint(MainDamage[0], MainDamage[1]) + (int)(randint(OffhandDamage[0], OffhandDamage[1]) * ((float)css.offhand_weapon_damage / 1000.0F));
}

void CreatureInstance :: ApplyRawDamage(int amount)
{
	//Subtracts a raw amount of health.  The given amount should already have
	//been processed by resists and modifiers.
	int health = css.health - amount;
	if(health < 0)
		health = 0;

	css.health = CalcRestrictedHealth(health, true);

	//According to a comment in the client code, if the client health is set to zero before
	//it has been flagged as dead (StatusEffects::DEAD) then there can be a 5 second delay
	if(health > 0)
	{
		int size = PrepExt_SendHealth(GSendBuf, CreatureID, css.health);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	}

	if(health == 0)
		_OnDeath();
}

void CreatureInstance :: CheckFallDamage(int elevation)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	//if(elevation > CurrentY)
	//	return;
	//int diff = CurrentY - elevation;
	int diff = elevation;

	int threshold = 50;
	int damage = 0;
	if(diff > threshold)
	{
		damage = diff - threshold;
		damage *= 5;
	}
	if(damage > 0)
	{
		char buffer[64];
		int wpos = PrepExt_SendFallDamage(buffer, damage);
		actInst->LSendToLocalSimulator(buffer, wpos, CurrentX, CurrentZ);
		ApplyRawDamage(damage);
	}
}

// Immediately called when enough damage has been applied to kill this creature.
void CreatureInstance :: _OnDeath(void)
{
	//g_Log.AddMessageFormat("_OnDeath for: %s", css.display_name);

	if(ab[0].bPending == true)
		CancelPending_Ex(&ab[0]);

	// CRASH FIX: moved this to the ProcessDeath() function.  When processing autoattacks,
	// reflected damage could kill the attacker and crash when trying to access the target again.
	//SetAutoAttack(NULL, 0);
	//SelectTarget(NULL);

	pendingOperations.DeathList_Add(this);

	//Get rid of stun since it can prevent the Rez skills from functioning.
	_RemoveStatusList(StatusEffects::STUN);
	_RemoveStatusList(StatusEffects::DAZE);
	_RemoveStatusList(StatusEffects::IN_COMBAT);
	_RemoveStatusList(StatusEffects::IN_COMBAT_STAND);

	_AddStatusList(StatusEffects::DEAD, -1);

	Speed = 0;

	css.hide_nameboard = 1;

	static const short statList = {STAT::HIDE_NAMEBOARD};
	int size = PrepExt_SendSpecificStats(GSendBuf, this, &statList, 1);

	size += PrepExt_GeneralMoveUpdate(&GSendBuf[size], this);  //Stop the creature.
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);

	// The deathTime needs to be updated, because sometimes any previous value 
	// could be low enough to trigger a garbage check and deletion before the
	// death processing was completed.
	deathTime = g_ServerTime;
}

void CreatureInstance :: _OnStun(void)
{
	//Doesn't apply the stun effect, but is called just before it is applied.
	Interrupt();
}

void CreatureInstance :: OnDaze(void)
{
	Interrupt();
}

void CreatureInstance :: ProcessPVPGoal()
{
	if(actInst != NULL && actInst->pvpGame != NULL)
	{
		ActiveParty * team = actInst->pvpGame->GetTeamForPlayer(CreatureID);
		if(team != NULL) {
			team->mPVPGoals++;
			PartyMember * member = team->GetMemberByID(CreatureID);
			if(member != NULL) {
				member->mPVPGoals++;
				actInst->LSendToLocalSimulator(GSendBuf, PrepExt_PVPStatUpdate(GSendBuf, actInst->pvpGame, member), CurrentX, CurrentZ);
			}
		}
	}
}

void CreatureInstance :: PrepareDeath(void)
{
	//Processes a dead creature, finalizing stats, applying loot, distributing
		//experience and quests for players.

		if(aiNut != NULL)
		{
			if(aiNut->JumpToLabel("on_death") == true)
				aiNut->Tick();
			aiNut->FullReset();
		}
		if(aiScript != NULL)
		{
			if(aiScript->JumpToLabel("onDeath") == true)
				aiScript->RunAtSpeed(50);
			aiScript->FullReset();
		}

		actInst->RunDeath(this);
		actInst->EraseIndividualReference(this);
		ab[0].Clear("CreatureInstance :: ProcessDeath");

		SetAutoAttack(NULL, 0);
		SelectTarget(NULL);

		_AddStatusList(StatusEffects::DEAD, -1);

		if(ab[0].IsBusy() == true)
			CancelPending_Ex(&ab[0]);

		//Remove stun one last time since it's possible that other creatures with
		//pending abilities might have re-stunned after the initial death operation from
		//the killing blow.
		if(HasStatus(StatusEffects::STUN))
			_RemoveStatusList(StatusEffects::STUN);

		//g_Log.AddMessageFormat("Death: %s, %d", ptr->css.display_name, ptr->CreatureID);

		//Run any processing if this creature is attached to spawner.
		RemoveFromSpawner(true);

		int wpos = PrepExt_UpdateFullPosition(GSendBuf, this);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);

		//We need to send the health amount after the DEAD flag has been set, otherwise the client might delay animations by 5 seconds.
		SendStatUpdate(STAT::HEALTH);

		actInst->UnHate(CreatureDefID);
}

void CreatureInstance :: ProcessDeath(void)
{

	CREATURE_SEARCH attackerList;
	ResolveAttackers(attackerList);

	HateProfile *hate = GetHateProfile();

	PrepareDeath();

	//Process PVP
	if(actInst->pvpGame != NULL)
	{
		ActiveParty * team = actInst->pvpGame->GetTeamForPlayer(CreatureID);
		if(team != NULL) {
			team->mPVPDeaths++;
			PartyMember * member = team->GetMemberByID(CreatureID);
			if(member != NULL) {
				member->mPVPDeaths++;
				actInst->LSendToLocalSimulator(GSendBuf, PrepExt_PVPStatUpdate(GSendBuf, actInst->pvpGame, member), CurrentX, CurrentZ);
			}

			// Now add a kill for the all the attackers
			std::vector<int> teamsCredited;

			for(size_t i = 0; i < attackerList.size(); i++)
			{
				CreatureInstance *attacker = attackerList[i].ptr;
				ActiveParty * attackerTeam = actInst->pvpGame->GetTeamForPlayer(attacker->CreatureID);
				if(attackerTeam != NULL) {
					std::vector<int>::iterator it = std::find(teamsCredited.begin(), teamsCredited.end(), attackerTeam->mPartyID);
					if(it == teamsCredited.end()) {
						attackerTeam->mPVPKills++;
						teamsCredited.push_back(attackerTeam->mPartyID);
					}
					PartyMember * attackerMember = team->GetMemberByID(attacker->CreatureID);
					if(attackerMember != NULL) {
						attackerMember->mPVPKills++;
						actInst->LSendToLocalSimulator(GSendBuf, PrepExt_PVPStatUpdate(GSendBuf, actInst->pvpGame, attackerMember), CurrentX, CurrentZ);
					}
				}
			}
		}

	}

	//Process experience and quest objectives.
	if(actInst->mZoneDefPtr->mGrove == false)
	{
		if(serverFlags & ServerFlags::IsNPC)
			actInst->NotifyKill(css.rarity);

		//Get highest level from the hate profile.
		int highestLev = 0;
		int highestPartySize = 1;
		if(hate != NULL)
			highestLev = hate->GetHighestLevel();

		if(attackerList.size() > 0) {
			highestLev = GetHighestLevel(attackerList);
			highestPartySize = GetHighestPartySize(attackerList);
		}


		/* Players can only be looted if they are in PVP mode, the attacker is in PVP
		 * mode and the zone is not an arena
		 */
		if(serverFlags & ServerFlags::IsPlayer) {
			/* This is a player death, determine if they were killed during PVP way. Loot
			 * is not given in arenas
			 */
			std::vector<CreatureInstance*> pvpAttackers;

			if(charPtr->Mode == PVP::GameMode::PVP && !actInst->mZoneDefPtr->mArena) {

				/* Don't want loop to be dropped if ANY attacker is not a player */
				bool npcInvolved = false;

				for(size_t i = 0; i < attackerList.size(); i++)	{
					CreatureInstance *attacker = attackerList[i].ptr;
					if(!(attacker->serverFlags & ServerFlags::IsPlayer)) {
						npcInvolved = true;
						break;
					}
					else if(attacker->charPtr->Mode == PVP::GameMode::PVP) {
						pvpAttackers.push_back(attacker);
					}
				}

				if(!npcInvolved && pvpAttackers.size() > 0) {

					char buffer[2048];

					// Pick how many items will be dropped
					int items = randmodrng(g_Config.MinPVPPlayerLootItems, g_Config.MaxPVPPlayerLootItems + 1);

					if(items > 0) {

						// Pick a random item from the players inventory
						InventoryManager *origInv = &charPtr->inventory;

						ActiveLootContainer loot;

						for(int i = 0 ; i < items; i++) {
							if(origInv->CountUsedSlots(INV_CONTAINER) == 0) {
								// Player has nothing to loot
								break;
							}
							InventorySlot *slot = origInv->PickRandomItem(INV_CONTAINER);
							if(slot == NULL)
								break;

							int toLoot = 1;

							// If the slot is a stack, pick a random amount to lose, up to the maximum of 16 (which is max in a chest anyway)
							if(slot->count > 1) {
								toLoot = randmodrng(0, slot->count);
							}


							for(int i = 0 ; i < toLoot && loot.itemList.size() < 16; i++)
								loot.AddItem(slot->IID);

							// Remove from player
							int len = origInv->RemoveItemsAndUpdate(INV_CONTAINER, slot->IID, toLoot, buffer);
							if(len > 0)
								simulatorPtr->AttemptSend(buffer, len);
						}

						if(loot.itemList.size() == 0)
							g_Log.AddMessageFormat("PVP resulted in no loot because losing player had no unbound items");
						else {

							/* Create a temporary creature for the loot. This allows the loot to live after
							 * the player has respawned (and solves some other issues)
							 */
							CreatureInstance* lootInst = actInst->SpawnGeneric(7861, CurrentX, CurrentY, CurrentZ, 0, 0);
							lootInst->deathTime = g_ServerTime;
							lootInst->PrepareDeath();
							loot.CreatureID = lootInst->CreatureID;

							lootInst->activeLootID = actInst->lootsys.AttachLootToCreature(loot, lootInst->CreatureID);

							// Add all PVP attackers as looting creatures as well as the player themselves so they can retrieve the loot if the attacker doesn't take
							lootInst->AddLootableID(CreatureDefID);
							for(std::vector<CreatureInstance*>::iterator it = pvpAttackers.begin(); it != pvpAttackers.end(); ++it) {
								lootInst->AddLootableID((*it)->CreatureDefID);
							}

							// Send loot updates
							if(lootInst->activeLootID != 0) {
								lootInst->SendUpdatedLoot();
							}
						}

					}

				}
			}
		}
		else {
			// Ordinary creature
			CreateLoot(highestLev, highestPartySize);
		}

		// Calculate how many credits should be awarded if the creature 'drops' them.
		int credits = 0;
		char buf[256];

		int creditDrops = 0;

		// Players don't drop credits
		if(!(serverFlags & ServerFlags::IsPlayer)) {
			CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CreatureDefID);
			if(cdef != NULL)
			{
				if(cdef->NamedMob) {
					creditDrops = g_Config.NamedMobCreditDrops;
				}
			}
			if(css.credit_drops > 0) {
				creditDrops = css.credit_drops;
			}
		}

		if(creditDrops > 0) {
			double maxPlayerLevel = 0;
			double alivePlayers = 0;
			for(size_t i = 0; i < attackerList.size(); i++)
			{
				CreatureInstance *attacker = attackerList[i].ptr;
				if(attacker->HasStatus(StatusEffects::DEAD) == false) {
					if(attacker->css.level > maxPlayerLevel)
						maxPlayerLevel = attacker->css.level;
					alivePlayers++;
				}
			}
			int levelDiff = ( css.level - maxPlayerLevel ) + 1;
			credits = creditDrops * levelDiff * css.rarity * alivePlayers;
			if(credits < 0) {
				g_Log.AddMessageFormat("Party earned no credits as level difference too great or nobody was left alive");
			}
			else {
				if(credits == 1)
					Util::SafeFormat(buf, sizeof(buf), "Your party earned 1 credit each");
				else if(credits > 1)
					Util::SafeFormat(buf, sizeof(buf), "Your party earned %d credits each", credits);

				g_Log.AddMessageFormat("This mob drops credits. The team earned %d (max player level %d, difference %d, alive %d)", credits, maxPlayerLevel, levelDiff, alivePlayers);
			}


		}

		for(size_t i = 0; i < attackerList.size(); i++)
		{	
			//ResolveAttackers() performs creature lookups, so the pointer is valid.
			CreatureInstance *attacker = attackerList[i].ptr;

			AddLootableID(attacker->CreatureDefID);

			if(attacker->HasStatus(StatusEffects::DEAD) == false)
			{
				attacker->CheckQuestKill(this);

				if(!(serverFlags & ServerFlags::IsPlayer)) {
					// Only NPC deaths get experience, not PVP
					int exp = GetKillExperience(highestLev);

					// Adjust for other XP bonuses
					if(attacker->css.experience_gain_rate > 0) {
						exp += Util::GetAdditiveFromIntegralPercent100(exp, attacker->css.experience_gain_rate);
					}

					//If a party member, don't give experience if players are too far
					//below the mob level.
					if(attackerList[i].attacked == 0)
						if(attacker->css.level < css.level - 5)
							exp = 1;

					attacker->AddExperience(exp);
					if(credits > 0) {
						attacker->AddCredits(credits);
						int wpos = PrepExt_SendInfoMessage(GSendBuf, buf, INFOMSG_INFO);
						attacker->simulatorPtr->AttemptSend(GSendBuf, wpos);
					}
				}

				if(attackerList[i].attacked == 1)
					attacker->AddHeroismForKill(css.level, css.rarity);
			}
		}

		if(activeLootID != 0)
			SendUpdatedLoot();
	}

	deathTime = g_ServerTime;
}

void CreatureInstance :: AddCreaturePointer(CREATURE_SEARCH& output, CreatureInstance* ptr, int attacked)
{
	if(ptr == NULL)
		return;

	for(size_t i = 0; i < output.size(); i++)
	{
		if(output[i].ptr == ptr)
		{
			if(attacked > output[i].attacked)
				output[i].attacked = attacked;
			return;
		}
	}
	CreatureSearch newItem;
	newItem.ptr = ptr;
	newItem.attacked = attacked;
	output.push_back(newItem);
}

int CreatureInstance :: GetHighestPartySize(CREATURE_SEARCH& creatureList)
{
	int highest = 1;
	int CDefID;
	int instance = actInst->mInstanceID;
	CreatureInstance *lookup;
	for(size_t i = 0; i < creatureList.size(); i++)
		if(creatureList[i].ptr->PartyID != 0) {
			ActiveParty *party = g_PartyManager.GetPartyByID(creatureList[i].ptr->PartyID);
			if(party != NULL) {
				int members = 0;
				for(size_t pm = 0; pm < party->mMemberList.size(); pm++)
				{
					CDefID = party->mMemberList[pm].mCreatureDefID;
					lookup = actInst->GetPlayerByCDefID(CDefID);
					if(lookup == NULL)
						continue;
					if(lookup->actInst->mInstanceID != instance)
						continue;

					if(actInst->GetPlaneRange(this, lookup, PARTY_SHARE_DISTANCE) >= PARTY_SHARE_DISTANCE)
						continue;

					members++;
				}

				if(members > highest)
					highest = members;
			}
		}
	return highest;
}

int CreatureInstance :: GetHighestLevel(CREATURE_SEARCH& creatureList)
{
	int highest = 0;
	for(size_t i = 0; i < creatureList.size(); i++)
		if(creatureList[i].ptr->css.level > highest)
			highest = creatureList[i].ptr->css.level;
	return highest;
}

void CreatureInstance :: ResolveAttackers(CREATURE_SEARCH& results)
{
	HateProfile *hate = GetHateProfile();

	//Search the hate profile attached to this creature, building a list of players
	//that have attacked this creature.  If the player is in a party, search through
	//party members to see which ones are in range and add those as well.
	if(hate == NULL)
		return;

	int instance = actInst->mInstanceID;

	std::vector<int> partyID;
	for(size_t i = 0; i < hate->hateList.size(); i++)
	{
		int CDefID = hate->hateList[i].CDefID;
		CreatureInstance *lookup = actInst->GetPlayerByCDefID(CDefID);
		if(lookup == NULL)
			continue;
		int attacked = (hate->hateList[i].damage > (lookup->GetMaxHealth(false) / 10));
		AddCreaturePointer(results, lookup, attacked);
		if(lookup->PartyID == 0)
			continue;

		ActiveParty *party = g_PartyManager.GetPartyByID(lookup->PartyID);
		if(party == NULL)
			continue;

		for(size_t pm = 0; pm < party->mMemberList.size(); pm++)
		{
			CDefID = party->mMemberList[pm].mCreatureDefID;
			lookup = actInst->GetPlayerByCDefID(CDefID);
			if(lookup == NULL)
				continue;
			if(lookup->actInst->mInstanceID != instance)
				continue;

			if(actInst->GetPlaneRange(this, lookup, PARTY_SHARE_DISTANCE) >= PARTY_SHARE_DISTANCE)
				continue;
			AddCreaturePointer(results, lookup, 0);
		}
	}
}

void CreatureInstance :: _OnModChanged(void)
{
	vector<int> statID;
	vector<float> totalVal;
	for(size_t a = 0; a < activeStatMod.size(); a++)
	{
		int found = -1;
		for(size_t b = 0; b < statID.size(); b++)
		{
			if(statID[b] == activeStatMod[a].modStatID)
			{
				found = b;
				break;
			}
		}
		if(found == -1)
		{
			statID.push_back(activeStatMod[a].modStatID);
			totalVal.push_back(activeStatMod[a].amount);
		}
		else
		{
			totalVal[found] += activeStatMod[a].amount;
		}
	}

	for(size_t a = 0; a < statID.size(); a++)
	{
		char buffer[16];
		sprintf(buffer, "%g", totalVal[a]);
		WriteStatToSet(statID[a], buffer, &css);
		int size = PrepExt_CreatureInstance(GSendBuf, this);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	}

	statID.clear();
	totalVal.clear();
}

void CreatureInstance :: _UpdateHealthMod(void)
{
	//Should be called when health-based stats are changed.  Recalculates the
	//health mod based.
	int maxhealth = GetMaxHealth(false);
	css.health_mod = (int)(((double)css.heroism / 10000.0F) * (double)maxhealth);
	vector<short> statList;
	statList.push_back(STAT::HEALTH_MOD);
	int size = PrepExt_SendSpecificStats(GSendBuf, this, statList);
	//actInst->LSendToAllSimulator(GSendBuf, size, -1);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
}

void CreatureInstance :: UpdateAggroPlayers(short state)
{
	//Only neutral creatures will change their aggro status after they have
	//spawned.  They will change if attacked or if returning to idle.
	if(!(serverFlags & ServerFlags::NeutralInactive))
		return;

	if(css.aggro_players == state)
		return;

	//Probably only friendly NPCs that have this, and it gives them green names.
	//If aggro status is set, they'll switch to red names.
	if(HasStatus(StatusEffects::UNATTACKABLE) || HasStatus(StatusEffects::INVINCIBLE))
		return;

	css.aggro_players = state;
	static short statList = STAT::AGGRO_PLAYERS;
	int wpos = PrepExt_SendSpecificStats(GSendBuf, this, &statList, 1);
	//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}

void CreatureInstance :: AddWDesc(int statID, int unknown1, int unknown2, const char *name)
{
}

void CreatureInstance :: Duration(int amount)
{
	//Moved to ability data.
}

void CreatureInstance :: Harm(int amount)
{
	ApplyRawDamage(amount);
}

bool CreatureInstance :: Behind(void)
{
	if(ab[0].TargetCount <= 0)
		return false;
	if(ab[0].TargetList[0] == NULL)
		return false;

	//If the player is inside the cone of the target, it obviously isn't behind.
	if(ab[0].TargetList[0]->isTargetInCone(this, Cone_180))
		return false;

	return true;
}

void CreatureInstance :: UnHate(void)
{
	actInst->UnHate(CreatureDefID);
}

void CreatureInstance :: LeaveCombat(void)
{
	SetAutoAttack(NULL, 0);
	SelectTarget(NULL);
	if(HasStatus(StatusEffects::IN_COMBAT))
		_RemoveStatusList(StatusEffects::IN_COMBAT);
	if(HasStatus(StatusEffects::IN_COMBAT_STAND))
		_RemoveStatusList(StatusEffects::IN_COMBAT_STAND);
}

void CreatureInstance :: Nullify(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, int duration)
{
	//Only seems to be used for WILL_REGEN
	//For this function we'll just add the inverse of the base stat.

	float amount = _GetBaseStat(statID) * -1;
	Add(tier, buffType, abID, abgID, statID, amount, amount, duration);
}

float CreatureInstance :: _GetBaseStat(int statID)
{
	for(size_t i = 0; i < baseStats.size(); i++)
	{
		if(baseStats[i].StatID == statID)
			return baseStats[i].fBaseVal;
	}
	return GetStatValueByID(statID, &css);
}

float CreatureInstance :: _GetTotalStat(int statID, int abilityGroup)
{
	//This function was created because the ability Amp() function was retrieving a value of
	//zero for armor ratings
	float total = 0.0F;
	int count = 0;
	for(size_t i = 0; i < activeStatMod.size(); i++)
	{
		if(activeStatMod[i].modStatID == statID)
		{
			if(activeStatMod[i].abilityGroupID != abilityGroup)
				total += activeStatMod[i].amount;
			count++;
		}
	}

	//If no modifiers for the stat existed in the list, retrieve the current total
	if(count == 0)
		total = GetStatValueByID(statID, &css);

	return total;
}

void CreatureInstance :: AmpCore(unsigned char tier, unsigned char buffType, int abID, int abgID, float ratio, int duration)
{
	//Get integer values from each core stat, run the percent, and forward everything
	//else into the Add() function, which handles the actual buff management.

	float amount = 0.0F;

	amount = Util::Round(_GetBaseStat(STAT::STRENGTH) * ratio);
	Add(tier, buffType, abID, abgID, STAT::STRENGTH, amount, ratio, duration);

	amount = Util::Round(_GetBaseStat(STAT::DEXTERITY) * ratio);
	Add(tier, buffType, abID, abgID, STAT::DEXTERITY, amount, ratio, duration);

	amount = Util::Round(_GetBaseStat(STAT::CONSTITUTION) * ratio);
	Add(tier, buffType, abID, abgID, STAT::CONSTITUTION, amount, ratio, duration);

	amount = Util::Round(_GetBaseStat(STAT::PSYCHE) * ratio);
	Add(tier, buffType, abID, abgID, STAT::PSYCHE, amount, ratio, duration);

	amount = Util::Round(_GetBaseStat(STAT::SPIRIT) * ratio);
	Add(tier, buffType, abID, abgID, STAT::SPIRIT, amount, ratio, duration);
}

void CreatureInstance :: Interrupt(void)
{
	if(ab[0].bPending == false)
		return;

	//g_Log.AddMessageFormat("************INTERRUPTED %d", ab[0].abilityID);
	if(ab[0].type == AbilityType::Channel && ab[0].bUnbreakableChannel == true)
		return;

	CancelPending_Ex(&ab[0]);
}

void CreatureInstance :: Spin(void)
{
	int newRot = Rotation + 128;
	if(newRot > 255)
		newRot -= 255;
	Rotation = (unsigned char)newRot;
	Heading = Rotation;
	int wpos = PrepExt_GeneralMoveUpdate(GSendBuf, this);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}

bool CreatureInstance :: NearbySanctuary(void)
{
	WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(actInst->mZone, CurrentX, CurrentZ, 250); 
	if(sanct == NULL)
		return false;

	return true;
}

bool CreatureInstance :: Translocate(bool test)
{
	//The parameter is not used in the client ability tables, but added here to allow
	//it to serve as a test for whether a translocate is possible.
	if(!(serverFlags & ServerFlags::IsPlayer))
		return false;

	if(charPtr->bindReturnPoint[3] == 0)
	{
		int wpos = PrepExt_SendInfoMessage(GSendBuf, "You have not bound to a sanctuary.", INFOMSG_ERROR);
		simulatorPtr->AttemptSend(GSendBuf, wpos);
		return false;
	}
	if(test == true)
		return true;

	bcm.AddEvent2(simulatorPtr->InternalID, (long)simulatorPtr, 0, BCM_RunTranslocate, actInst);
	return true;
}

void CreatureInstance :: FullMight(void)
{
	css.might = ABGlobals::MAX_MIGHT;
	SendStatUpdate(STAT::MIGHT);
}

void CreatureInstance :: FullWill(void)
{
	css.will = ABGlobals::MAX_WILL;
	SendStatUpdate(STAT::WILL);
}

bool CreatureInstance :: PercentMaxHealth(double ratio)
{
	double test = (double)GetMaxHealth(true) * ratio;
	return (css.health > (int)test);
}

void CreatureInstance :: HealthSacrifice(int amount)
{
}

void CreatureInstance :: Regenerate(int amount)
{
	Heal(amount);
}

void CreatureInstance :: DoNothing(void)
{
}

void CreatureInstance :: SummonPet(int unknown)
{
}

void CreatureInstance :: Jump(void)
{
}

int CreatureInstance :: AdjustMight(int amount)
{
	_LimitAdjust(css.might, amount, 0, ABGlobals::MAX_MIGHT);
	bOrbChanged = true;
	return 1;
}

int CreatureInstance :: AddWillCharge(int amount)
{
	_LimitAdjust(css.will_charges, amount, 0, ABGlobals::MAX_WILLCHARGES);
	LastCharge = g_ServerTime;
	bOrbChanged = true;
	SendStatUpdate(STAT::WILL_CHARGES);
	return 1;
}
int CreatureInstance :: AddMightCharge(int amount)
{
	_LimitAdjust(css.might_charges, amount, 0, ABGlobals::MAX_MIGHTCHARGES);
	LastCharge = g_ServerTime;
	bOrbChanged = true;
	SendStatUpdate(STAT::MIGHT_CHARGES);
	return 1;
}

void CreatureInstance :: _LimitAdjust(short &value, int add, int min, int max)
{
	value = static_cast<short>(Util::ClipInt(value + add, min, max));
}

void CreatureInstance :: CancelPending(void)
{
	Debug::LastAbility = ab[0].abilityID;
	Debug::CreatureDefID = CreatureDefID;
	Util::SafeCopy(Debug::LastName, css.display_name, sizeof(Debug::LastName));
	Debug::IsPlayer = (serverFlags & ServerFlags::IsPlayer);
	Debug::LastPlayer = NULL;

	//Called when an ability activation needs to interrupt an existing cast.
	bool registeredAbility = false;
	if(serverFlags & ServerFlags::IsPlayer)
	{
		Debug::LastPlayer = simulatorPtr;
		//Fall through the ability IDs.  For quest interactions only, modify scripts.
		//In the end, a cancel event will be sent for all ability IDs.
		switch(ab[0].abilityID)
		{
		case ABILITYID_QUEST_INTERACT_OBJECT:
		case ABILITYID_QUEST_GATHER_OBJECT:
			QuestScript::QuestScriptPlayer *script;
			//TODO: Simulator Revamp
			if(simulatorPtr == NULL)
				break;
			script = actInst->GetSimulatorQuestScript(simulatorPtr);
			if(script != NULL)
				script->mExecuting = false;

			QuestScript::QuestNutPlayer *nut = actInst->GetSimulatorQuestNutScript(simulatorPtr);
			if(nut != NULL) {
				char buffer[64];
				Util::SafeFormat(buffer, sizeof(buffer), "on_use_cancel_%d", CreatureDefID);
				nut->JumpToLabel(buffer);
			}

			break;
		}

		switch(ab[0].abilityID)
		{
		case ABILITYID_QUEST_INTERACT_OBJECT:
		case ABILITYID_QUEST_GATHER_OBJECT:
		case ABILITYID_INTERACT_OBJECT:
			int wpos;
			wpos = 0;
			wpos += PutByte(&GSendBuf[wpos], 4);  //_handleCreatureEventMsg
			wpos += PutShort(&GSendBuf[wpos], 0);  //size
			wpos += PutInteger(&GSendBuf[wpos], CreatureID);
			wpos += PutByte(&GSendBuf[wpos], 11);  //creature "used" event
			wpos += PutStringUTF(&GSendBuf[wpos], "");
			wpos += PutInteger(&GSendBuf[wpos], -1);  //A delay of -1 will interrupt the action
			PutShort(&GSendBuf[1], wpos - 3);  //size
			SendToOneSimulator(GSendBuf, wpos, simulatorPtr);

			registeredAbility = false;
			break;
		}
	}

	if(registeredAbility == false)
	{
		int size = 0;
		if(serverFlags & ServerFlags::IsPlayer)
			size = PrepExt_AbilityActivateEmpty(GSendBuf, this, &ab[0], AbilityStatus::ABILITY_FINISHED);
		size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, &ab[0], AbilityStatus::INTERRUPTED);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	}
	ab[0].Clear("CreatureInstance :: CancelPending");
}

void CreatureInstance :: CancelPending_Ex(ActiveAbilityInfo *ability)
{
	bool registeredAbility = true;
	if(serverFlags & ServerFlags::IsPlayer)
	{
		if(ability->type == AbilityType::Cast)
			simulatorPtr->RunFinishedCast(false);

		switch(ability->abilityID)
		{
		case ABILITYID_QUEST_INTERACT_OBJECT:  //intentional fallthrough
		case ABILITYID_QUEST_GATHER_OBJECT:
			QuestScript::QuestScriptPlayer *script;
			if(simulatorPtr == NULL)
				break;

			if(actInst->nutScriptPlayer != NULL) {
				actInst->nutScriptPlayer->InterruptInteraction(CreatureID);
			}

			std::list<QuestScript::QuestNutPlayer*> l = g_QuestNutManager.GetActiveScripts(CreatureID);
			std::list<QuestScript::QuestNutPlayer*>::iterator it = l.begin();
			for(; it != l.end(); ++it) {
				QuestScript::QuestNutPlayer *player = *it;
				player->InterruptInteraction();
			}

			script = actInst->GetSimulatorQuestScript(simulatorPtr);
			if(script != NULL)
				script->TriggerAbort();

			QuestScript::QuestNutPlayer *nut = actInst->GetSimulatorQuestNutScript(simulatorPtr);
			if(nut != NULL) {
				char buffer[64];
				Util::SafeFormat(buffer, sizeof(buffer), "on_use_abort_%d", CreatureDefID);
				nut->JumpToLabel(buffer);
			}

			//Fall through since all quest/object interactions need to notify the client to
			//cancel the event timer.
			break;
		}

		switch(ability->abilityID)
		{
		case ABILITYID_QUEST_INTERACT_OBJECT:  //intentional fallthrough
		case ABILITYID_QUEST_GATHER_OBJECT:
		case ABILITYID_INTERACT_OBJECT:
			actInst->ScriptCallUseHalt(CreatureID, LastUseDefID);
			int wpos;
			wpos = PrepExt_CancelUseEvent(GSendBuf, CreatureID);
			SendToOneSimulator(GSendBuf, wpos, simulatorPtr);

			registeredAbility = false;
			break;
		}
	}

	if(registeredAbility == true)
	{
		int size = 0;
		if(serverFlags & ServerFlags::IsPlayer)
			size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::ABILITY_FINISHED);
		size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::INTERRUPTED);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
	}
	ability->Clear("CreatureInstance :: CancelPending_Ex");
}

//If any ability is currently running that does not match the supplied ID, cancel it.
void CreatureInstance :: OverrideCurrentAbility(int abilityID)
{
	if(ab[0].bPending == true)
	{
		if(ab[0].abilityID == abilityID)
			return;
		CancelPending_Ex(&ab[0]);
	}
}

void CreatureInstance :: RegisterInstant(int abilityID)
{
	if(HasStatus(StatusEffects::DAZE))
		return;

	/* TODO: OBSOLETE WITH NEW ABILITY SYSTEM
	if(ab[0].bPending == true)
	{
		g_Log.AddMessageFormat("[DEBUG] RegisterInstant checkpoint.  May need to delete this.");

		//TODO: this could potentially be dangerous if it ever reaches this point
		//while an ability is still performing actions on its target list, since this
		//will erase the target list and leave null pointers.
		CancelPending();
	}
	*/

	ab[0].abilityID = abilityID;
	ab[0].type = AbilityType::Instant;
	ab[0].fireTime = g_ServerTime;  //Process function needs a valid time
	//ab[0].mightCharges = 0;
	//ab[0].willCharges = 0;
	ab[0].bPending = true;
	ab[0].bSecondary = false;
	ab[0].bUnbreakableChannel = false;
	ab[0].bParallel = false;
}

void CreatureInstance :: RegisterChannel(int abilityID, int duration, int interval, bool bOverride, int interruptResistMod)
{
	if(HasStatus(StatusEffects::DAZE))
		return;

	int abSlot = 0;
	if(bOverride == true)
		abSlot = 1;


	/* TODO: OBSOLETE WITH NEW ABILITY SYSTEM
	if(ab[abSlot].bPending == true)
	{
		g_Log.AddMessageFormat("Canceled");
		//if(bOverride == false)
		//{
			if(ab[abSlot].abilityID == abilityID)
				return;
			CancelPending_Ex(&ab[abSlot]);
		//}
	}
	*/

	if(interval == 0)
	{
		g_Log.AddMessageFormat("WARNING: RegisterChannel() interval is zero");
		interval = 1000;
	}
	if(duration < interval)
		g_Log.AddMessageFormat("WARNING: RegisterChannel() possible iteration problem (ability: %d, duration: %d, interval: %d", abilityID, duration, interval);

	ab[abSlot].abilityID = abilityID;
	ab[abSlot].type = AbilityType::Channel;
	ab[abSlot].iterationDur = duration;
	ab[abSlot].iterationInt = interval;
	ab[abSlot].iterations = duration / interval;
	if(ab[abSlot].iterations == 0)
	{
		g_Log.AddMessageFormat("WARNING: RegisterChannel() iterations is zero");
		ab[abSlot].iterations = 1;
	}
	ab[abSlot].fireTime = g_ServerTime;

	//Hack for walk in shadows to work.  The skill is activated as a channel but needs to complete the
	//warmup time.  Not sure if this messes up any other skills.
	if(duration == interval)
		ab[abSlot].fireTime += interval;

	//ab[abSlot].mightCharges = 0;
	//ab[abSlot].willCharges = 0;
	ab[abSlot].bPending = true;
	ab[abSlot].bSecondary = true;
	ab[abSlot].bParallel = false;
	ab[abSlot].interruptChanceMod = static_cast<short>(interruptResistMod);

	if(bOverride == true)
	{
		ab[abSlot].bParallel = true;
		ab[abSlot].TransferTargetList(&ab[0]);
		ab[0].bPending = false;
	}
	else
	{
		/* TODO: OBSOLETE WITH NEW ABILITY SYSTEM
		int r = actInst->ActivateAbility(this, abilityID, Action::onActivate);
		if(r != 0)
		{
			ab[abSlot].Clear("CreatureInstance :: RegisterChannel");
			g_Log.AddMessageFormat("Cleared");
			return;
		}
		*/

		//g_Log.AddMessage("Adding");
		bool ground = false;
		if(ab[abSlot].x != 0.0F)
			ground = true;

		int size = 0;
		if(serverFlags & ServerFlags::IsPlayer)
			size = PrepExt_AbilityActivateEmpty(GSendBuf, this,  &ab[abSlot], AbilityStatus::REQUEST_ACCEPTED);

		//Apparently with the new system, it needs an activate request.
//		size += PrepExt_AbilityActivate(&GSendBuf[size], this, &ab[abSlot], AbilityStatus::ACTIVATE, ground);  //was ACTIVATE

		size += PrepExt_AbilityActivate(&GSendBuf[size], this, &ab[abSlot], AbilityStatus::CHANNELING, ground);  //was ACTIVATE
		//actInst->LSendToAllSimulator(GSendBuf, size, -1);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
		if(ab[abSlot].x != 0.0F)
		{
			ab[abSlot].x = 0.0F;
			ab[abSlot].y = 0.0F;
			ab[abSlot].z = 0.0F;
		}
	}

	//memcpy(&activeAbilitySec, &activeAbility, sizeof(ActiveAbilityInfo));
	//ab[0].bPending = false;
}

void CreatureInstance :: RegisterCast(int abilityID, int warmupTime)
{
	if(HasStatus(StatusEffects::DAZE))
		return;

	//Casting speed is positive for a boost, negative for a drain.
	//+100 speed = 10% boost
	int time = warmupTime;
	if(css.magic_attack_speed != 0)
		time -= (int)((float)time * ((float)css.magic_attack_speed * 0.001F));

	//g_Log.AddMessageFormat("magic speed: %d, WarmupTime: %d", css.magic_attack_speed, time);

	ab[0].abilityID = abilityID;
	ab[0].type = AbilityType::Cast;
	ab[0].fireTime = g_ServerTime + time;
	//ab[0].mightCharges = 0;
	//ab[0].willCharges = 0;
	ab[0].bPending = true;
	ab[0].bSecondary = false;
	ab[0].bUnbreakableChannel = false;
	ab[0].bParallel = false;

	//This is part of the Creature Instance since only the primary ability uses it,
	//no sense wasting an extra byte.
	setbackCount = 0;

	int size = 0;
	if(serverFlags & ServerFlags::IsPlayer)
		size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, &ab[0], AbilityStatus::REQUEST_ACCEPTED);
	size += PrepExt_AbilityActivate(&GSendBuf[size], this, &ab[0], AbilityStatus::WARMUP);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
}

bool CreatureInstance :: CanPVPTarget(CreatureInstance *target)
{
	if(this == target)
		return false;

	if(target->HasStatus(StatusEffects::UNATTACKABLE))
		return false;

	if(target->HasStatus(StatusEffects::INVINCIBLE))
		return false;

	//Due to client limitations, both players need PVPABLE status or else one player may not
	//initiate autoattacks against the other, because target selection is still considered
	//friendly for the player without the PVPABLE status.
	bool selfPVP = HasStatus(StatusEffects::PVPABLE);
	bool targPVP = target->HasStatus(StatusEffects::PVPABLE);
	if(selfPVP == true && targPVP == true)
	{
		if(actInst != NULL)
		{
			if(actInst->mZoneDefPtr != NULL)
			{
				if(actInst->mZoneDefPtr->IsPVPArena() == true)
				{
					// In an arena, party members are members of the same, so
					// should be unattackable
					if(PartyID > 0 && PartyID == target->PartyID)
						return false;

					return true;
				}
			}
		}

		//If we get here, we're not in an arena.  Check teams instead.
		//This was added so that special events could be done where specific players could be
		//given PVP status among others instead of creating an environment where everyone can
		//attack each other.

		if(actInst->mZoneDefPtr->mMode == PVP::GameMode::SPECIAL_EVENT && css.pvp_team != target->css.pvp_team)
			return true;

		//If we get here, we are not in an area, and not in a team, check the zone itself to see
		//if PVP is allowed
		if(actInst != NULL && actInst->mZoneDefPtr != NULL && ( actInst->mZoneDefPtr->mMode == PVP::GameMode::PVP || actInst->mZoneDefPtr->mMode == PVP::GameMode::PVP_ONLY))
			return true;

	}


	/* OLD
	if(HasStatus(StatusEffects::PVPABLE) == false)
		return false;
	if(target->HasStatus(StatusEffects::PVPABLE) == false)
		return false;
	*/
	return false;
}
bool CreatureInstance :: _ValidTargetFlag(CreatureInstance *target, int abilityRestrict)
{
	if(target->serverFlags & ServerFlags::LeashRecall)
		return false;
	if((target->serverFlags & ServerFlags::Noncombatant) && (abilityRestrict & TargetStatus::TSB_ENEMY))
		return false;

	if(abilityRestrict & TargetStatus::TSB_ENEMY)
	{
		if((serverFlags & ServerFlags::IsPlayer) && (target->serverFlags & ServerFlags::IsPlayer))
			return CanPVPTarget(target);
	}

	//Disable PVP
	//If both are players, don't add to target list if it calls for an enemy.
	//if(abilityRestrict & TSB_ENEMY)
	//	if((serverFlags & ServerFlags::IsPlayer) && (target->serverFlags & ServerFlags::IsPlayer))
	//		return false;
//
//	if((target == this) && (abilityRestrict & TargetStatus::TSB_ENEMY))
//		return false;

	if(abilityRestrict & TargetStatus::TSB_FRIEND)
	{
		if(target->Faction != Faction)
		{
			//MessageBox(0, "Failed friend check", 0, 0);
			return false;
		}
	}
	else if(abilityRestrict & TargetStatus::TSB_ENEMY)
	{
		if(target == this)
			return false;

		if(target->Faction == Faction)
		{
			if(HasStatus(StatusEffects::PVPABLE) == false)
				return false;
			if(target->HasStatus(StatusEffects::PVPABLE) == false)
				return false;
		}
	}

	if(abilityRestrict & TargetStatus::TSB_ALIVE)
	{
		if(target->css.health > 0)
			return true;
		else
			return false;
	}
	else if(abilityRestrict & TargetStatus::TSB_DEAD)
	{
		if(target->css.health == 0)
			return true;
		else
			return false;
	}

	return true;


	/*
	if((compare == this) && (abilityRestrict & TSB_ENEMY))
		return false;

	if(abilityRestrict & TSB_FRIEND)
	{
		if(compare->Faction != Faction)
			return false;
	}
	else if(abilityRestrict & TSB_ENEMY)
	{
		if(compare->Faction == Faction)
			if(PVPState == false)
				return false;
	}

	if(abilityRestrict & TSB_ALIVE)
	{
		if(css.health > 0)
			return true;
		else
			return false;
	}
	else if(abilityRestrict & TSB_DEAD)
	{
		if(css.health == 0)
			return true;
		else
			return false;
	}

	return true;*/
}

int CreatureInstance :: _AddTargetsInCone(double halfAngle, int targetType, int distance, int abilityRestrict)
{
	int a;
	int count = 0;
#ifdef CREATUREQAV
	for(a = 0; a < (int)actInst->NPCListPtr.size(); a++)
	{
		if(_ValidTargetFlag(actInst->NPCListPtr[a], abilityRestrict) == true)
		{
			if(actInst->GetPlaneRange(this, actInst->NPCListPtr[a], distance) <= distance)
			{
				if(isTargetInCone(actInst->NPCListPtr[a], halfAngle) == true)
				{
					if(count >= MAXTARGET)
						return count;
					ab[0].TargetList[count++] = actInst->NPCListPtr[a];
				}
			}
		}
	}
#else
	ActiveInstance::CREATURE_IT it;
	for(it = actInst->NPCList.begin(); it != actInst->NPCList.end(); ++it)
	{
		if(_ValidTargetFlag(&it->second, abilityRestrict) == true)
		{
			if(actInst->GetPlaneRange(this, &it->second, distance) <= distance)
			{
				if(isTargetInCone(&it->second, halfAngle) == true)
				{
					if(count >= MAXTARGET)
						return count;
					ab[0].TargetList[count++] = &it->second;
				}
			}
		}
	}
#endif
	for(a = 0; a < (int)actInst->PlayerListPtr.size(); a++)
	{
		if(_ValidTargetFlag(actInst->PlayerListPtr[a], abilityRestrict) == true)
		{
			if(actInst->GetPlaneRange(this, actInst->PlayerListPtr[a], distance) <= distance)
			{
				if(isTargetInCone(actInst->PlayerListPtr[a], halfAngle) == true)
				{
					if(count >= MAXTARGET)
						return count;
					ab[0].TargetList[count++] = actInst->PlayerListPtr[a];
				}
			}
		}
	}
	for(a = 0; a < (int)actInst->SidekickListPtr.size(); a++)
	{
		if(_ValidTargetFlag(actInst->SidekickListPtr[a], abilityRestrict) == true)
		{
			if(actInst->GetPlaneRange(this, actInst->SidekickListPtr[a], distance) <= distance)
			{
				if(isTargetInCone(actInst->SidekickListPtr[a], halfAngle) == true)
				{
					if(count >= MAXTARGET)
						return count;
					ab[0].TargetList[count++] = actInst->SidekickListPtr[a];
				}
			}
		}
	}

	return count;
}

//Selected target.
void CreatureInstance :: AddTargetsST(CREATURE_PTR_SEARCH &results, int abilityRestrict)
{
	//Friendly skills like buffs and healing usually default to self if no
	//target is selected.
	if(CurrentTarget.targ != NULL)
	{
		if(_ValidTargetFlag(CurrentTarget.targ, abilityRestrict) == true)
			results.push_back(CurrentTarget.targ);
		else if(this != CurrentTarget.targ)
			if(_ValidTargetFlag(this, abilityRestrict) == true)
				results.push_back(this);
	}
	else
	{
		if(_ValidTargetFlag(this, abilityRestrict) == true)
			results.push_back(this);
	}
}

//Add targets within the viewable cone away from the caster
void CreatureInstance :: AddTargetsConeAngle(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance, double halfAngle)
{
	size_t i;
	for(i = 0; i < actInst->NPCListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->NPCListPtr[i], abilityRestrict) == true)
			//if(actInst->GetPlaneRange(this, actInst->NPCListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->NPCListPtr[i], distance) == true)
				if(isTargetInCone(actInst->NPCListPtr[i], halfAngle) == true)
					results.push_back(actInst->NPCListPtr[i]);

	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], abilityRestrict) == true)
			//if(actInst->GetPlaneRange(this, actInst->PlayerListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->PlayerListPtr[i], distance) == true)
				if(isTargetInCone(actInst->PlayerListPtr[i], halfAngle) == true)
					results.push_back(actInst->PlayerListPtr[i]);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPlaneRange(this, actInst->SidekickListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->SidekickListPtr[i], distance) == true)
				if(isTargetInCone(actInst->SidekickListPtr[i], halfAngle) == true)
					results.push_back(actInst->SidekickListPtr[i]);
}

//Add targets in the radius around the selected ground location.
void CreatureInstance :: AddTargetsGTAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance)
{
	float x = ab[0].x;
	float z = ab[0].z;
	size_t i;
	for(i = 0; i < actInst->NPCListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->NPCListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPointRangeXZ(actInst->NPCListPtr[i], ab[0].x, ab[0].z, distance) <= distance)
			if(actInst->NPCListPtr[i]->IsSelfNearPoint(x, z, distance) == true)
				results.push_back(actInst->NPCListPtr[i]);

	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPointRangeXZ(actInst->PlayerListPtr[i], ab[0].x, ab[0].z, distance) <= distance)
			if(actInst->PlayerListPtr[i]->IsSelfNearPoint(x, z, distance) == true)
				results.push_back(actInst->PlayerListPtr[i]);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], abilityRestrict) == true)
			//if(actInst->GetPointRangeXZ(actInst->SidekickListPtr[i], ab[0].x, ab[0].z, distance) <= distance)
			if(actInst->SidekickListPtr[i]->IsSelfNearPoint(x, z, distance) == true)
				results.push_back(actInst->SidekickListPtr[i]);
}

//Add all targets within a certain distance of the player (AKA caster).
void CreatureInstance :: AddTargetsPBAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance)
{
	size_t i;
	for(i = 0; i < actInst->NPCListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->NPCListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPlaneRange(this, actInst->NPCListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->NPCListPtr[i], distance) == true)
				results.push_back(actInst->NPCListPtr[i]);
	
	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPlaneRange(this, actInst->PlayerListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->PlayerListPtr[i], distance) == true)
				results.push_back(actInst->PlayerListPtr[i]);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], abilityRestrict) == true)
//			if(actInst->GetPlaneRange(this, actInst->SidekickListPtr[i], distance) <= distance)
			if(IsObjectInRange(actInst->SidekickListPtr[i], distance) == true)
				results.push_back(actInst->SidekickListPtr[i]);
}

//Add the current target and all creatures within proximity distance around it.
void CreatureInstance :: AddTargetsSTAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance)
{
	if(CurrentTarget.targ == NULL)
		return;
	results.push_back(CurrentTarget.targ);

	size_t i;
	for(i = 0; i < actInst->NPCListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->NPCListPtr[i], abilityRestrict) == true)
			if(IsObjectInRange(actInst->NPCListPtr[i], distance) == true)
//			if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->NPCListPtr[i], distance) <= distance)
				results.push_back(actInst->NPCListPtr[i]);

	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], abilityRestrict) == true)
			if(IsObjectInRange(actInst->PlayerListPtr[i], distance) == true)
//			if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->PlayerListPtr[i], distance) <= distance)
				results.push_back(actInst->PlayerListPtr[i]);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], abilityRestrict) == true)
			if(IsObjectInRange(actInst->SidekickListPtr[i], distance) == true)
//			if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->SidekickListPtr[i], distance) <= distance)
				results.push_back(actInst->SidekickListPtr[i]);

}

//Add all creatures within proximity distance around the current target, but do not add the
//current target.
void CreatureInstance :: AddTargetsSTXAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance)
{
	if(CurrentTarget.targ == NULL)
		return;

	size_t i;
	for(i = 0; i < actInst->NPCListPtr.size(); i++)
		if(actInst->NPCListPtr[i] != CurrentTarget.targ)
			if(_ValidTargetFlag(actInst->NPCListPtr[i], abilityRestrict) == true)
				if(IsObjectInRange(actInst->NPCListPtr[i], distance) == true)
//				if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->NPCListPtr[i], distance) <= distance)
					results.push_back(actInst->NPCListPtr[i]);

	for(i = 0; i < actInst->PlayerListPtr.size(); i++)
		if(actInst->PlayerListPtr[i] != CurrentTarget.targ)
			if(_ValidTargetFlag(actInst->PlayerListPtr[i], abilityRestrict) == true)
				if(IsObjectInRange(actInst->PlayerListPtr[i], distance) == true)
//				if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->PlayerListPtr[i], distance) <= distance)
					results.push_back(actInst->PlayerListPtr[i]);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(actInst->SidekickListPtr[i] != CurrentTarget.targ)
			if(_ValidTargetFlag(actInst->SidekickListPtr[i], abilityRestrict) == true)
				if(IsObjectInRange(actInst->SidekickListPtr[i], distance) == true)
//				if(actInst->GetPlaneRange(CurrentTarget.targ, actInst->SidekickListPtr[i], distance) <= distance)
					results.push_back(actInst->SidekickListPtr[i]);
}

//Add all party members within range to the target list.
void CreatureInstance :: AddTargetsParty(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance)
{
	size_t i;
	if(serverFlags & ServerFlags::IsPlayer)
	{
		ActiveParty *myParty = g_PartyManager.GetPartyWithMember(CreatureDefID);
		if(myParty != NULL)
		{
			for(i = 0; i < actInst->PlayerListPtr.size(); i++)
			{
				int CDefID = actInst->PlayerListPtr[i]->CreatureDefID;
				if(CDefID != CreatureDefID)  //Skip the caster, we'll add it later.
				{
					//if(actInst->GetPlaneRange(this, actInst->PlayerListPtr[i], distance) <= distance)
					if(IsObjectInRange(actInst->PlayerListPtr[i], distance) == true)
					{
						ActiveParty *thatParty = g_PartyManager.GetPartyWithMember(CDefID);
						if(myParty == thatParty)
							results.push_back(actInst->PlayerListPtr[i]);
					}
				}
			}
		}
	}
	//Always add the caster.  It's here so it works even if not in a party.
	results.push_back(this);

	for(i = 0; i < actInst->SidekickListPtr.size(); i++)
		if(actInst->SidekickListPtr[i]->AnchorObject == this)
			results.push_back(actInst->SidekickListPtr[i]);
}

void CreatureInstance :: AddTargetsSTP(CREATURE_PTR_SEARCH &results, int abilityRestrict)
{
	//No distance check since there's no known distance.
	//Add self if no target selected, otherwise check if the player is in 
	if(CurrentTarget.targ == NULL)
	{
		results.push_back(this);
		return;
	}

	if(_ValidTargetFlag(CurrentTarget.targ, abilityRestrict) == true)
		results.push_back(CurrentTarget.targ);
}


int CreatureInstance :: TransferTargets(const CREATURE_PTR_SEARCH &results, ActiveAbilityInfo &ab)
{
	size_t tcount = results.size();
	if(tcount > MAXTARGET)
		tcount = MAXTARGET;

	for(size_t i = 0; i < tcount; i++)
		ab.TargetList[i] = results[i];
	ab.TargetCount = tcount;

	return tcount;
}

void CreatureInstance :: RegisterImplicit(int eventType, int abID, int abGroup)
{
	//Implicit actions are dealt back to attackers if the conditions are right.

	//If the action is already registered (check ability group) then update the ID.
	//This is so that casting a new ability will replace the existing entry.
	for(size_t i = 0; i < implicitActions.size(); i++)
	{
		if(implicitActions[i].abilityGroup == abGroup)
		{
			implicitActions[i].abilityID = abID;
			return;
		}
	}

	//Not in list, create a new one.
	ImplicitAction action;
	action.eventType = eventType;
	action.abilityID = abID;
	action.abilityGroup = abGroup;
	implicitActions.push_back(action);
}

int CreatureInstance :: CallAbilityEvent(int abilityID, int eventType)
{
	return CallAbilityEvent(abilityID, eventType, false);
}

int CreatureInstance :: CallAbilityEvent(int abilityID, int eventType, bool force)
{
	ab[0].bForce = force;
	return actInst->ActivateAbility(this, abilityID, eventType, &ab[0]);
}

int CreatureInstance :: RequestAbilityActivation(int abilityID)
{
	int result = actInst->ActivateAbility(this, abilityID, EventType::onRequest, NULL);
	if(result != Ability2::ABILITY_SUCCESS)
	{
		//Players may need a message informing them of why the ability failed to activate.
		if((serverFlags & ServerFlags::IsPlayer) && (simulatorPtr != NULL))
		{
			int errorCode = Ability2::AbilityManager2::GetAbilityErrorCode(result);
			g_Log.AddMessageFormat("Failed to activate ability %d because error code %d (%d).", abilityID, errorCode, result);
			simulatorPtr->SendAbilityErrorMessage(errorCode);
		}
		else {
			g_Log.AddMessageFormat("Non-success result %d.", result);
		}

		//Other processing may require conditional processing based on return value.
		return result;
	}
	return Ability2::ABILITY_SUCCESS;
}

void CreatureInstance :: ProcessAbility_Ex(ActiveAbilityInfo *ability)
{
	if(ability->bPending == false)
		return;
	if(g_ServerTime < ability->fireTime)
		return;

	if(HasStatus(StatusEffects::STUN) == true && ability->bParallel == false)
	{
		CancelPending_Ex(ability);
		return;
	}

	int size = 0;
	bool ground = false;
	int r;
	switch(ability->type)
	{
	case AbilityType::Instant:
		r = actInst->ActivateAbility(this, ability->abilityID, EventType::onActivate, ability);

		//With the new system, the activation stage may transfer the operation type to a channel.
		//If the operation type is still Instant, finalize events for normal instant casts.
		if(ability->type == AbilityType::Instant)
		{
			if(r == 0)
			{
				if(ability->x != 0.0F)
					ground = true;
				if(serverFlags & ServerFlags::IsPlayer)
				{
					size = PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::REQUEST_ACCEPTED);
					size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::ABILITY_FINISHED);
				}
				size += PrepExt_AbilityActivate( &GSendBuf[size], this, ability, AbilityStatus::ACTIVATE, ground);
				if(ability->x != 0.0F)
				{
					ability->x = 0.0F;
					ability->y = 0.0F;
					ability->z = 0.0F;
				}
				//actInst->LSendToAllSimulator(GSendBuf, size, -1);
				actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
				ability->Clear("CreatureInstance :: ProcessAbility_Ex (done)");
			}
		}
		break;
	case AbilityType::Cast:
		r = actInst->ActivateAbility(this, ability->abilityID, EventType::onActivate, ability);
		if(r == 0)
		{
			if(ability->x != 0.0F)
				ground = true;

			if(serverFlags & ServerFlags::IsPlayer)
				size += PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::ABILITY_FINISHED);

			size += PrepExt_AbilityActivate(&GSendBuf[size], this, ability, AbilityStatus::ACTIVATE, ground);
			if(ability->x != 0.0F)
			{
				ability->x = 0.0F;
				ability->y = 0.0F;
				ability->z = 0.0F;
			}
			//actInst->LSendToAllSimulator(GSendBuf, size, -1);
			actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
		}
		if(ability->type == AbilityType::Cast)
			ability->Clear("CreatureInstance :: ProcessAbility_Ex (done)");

		if(serverFlags & ServerFlags::IsPlayer)
			simulatorPtr->RunFinishedCast(r == 0);

		break;
	case AbilityType::Channel:
		r = actInst->ActivateAbility(this, ability->abilityID, EventType::onIterate, ability);
		if(r == 0)
		{
			ability->iterations--;
			if(ability->iterations <= 0)
			{
				if(serverFlags & ServerFlags::IsPlayer)
				{
					size = PrepExt_AbilityActivateEmpty(&GSendBuf[size], this, ability, AbilityStatus::ABILITY_FINISHED);
					actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
				}
				ability->Clear("CreatureInstance :: ProcessAbility_Ex (done)");
			}
			else
			{
				ability->fireTime = g_ServerTime + ability->iterationInt;
			}
		}
		else
		{
			ability->Clear("CreatureInstance :: ProcessAbility_Ex (inter)");
		}
		break;
	}
}

void CreatureInstance :: RunActiveAbilities(void)
{
	if(ab[0].bPending == true)
		ProcessAbility_Ex(&ab[0]);
	if(ab[1].bPending == true)
		ProcessAbility_Ex(&ab[1]);
}


bool CreatureInstance :: HasCooldown(int cooldownCategory)
{
	int r = cooldownManager.HasCooldown(cooldownCategory);
	if((r >= 0) && (serverFlags & ServerFlags::IsPlayer))
	{
		if(simulatorPtr->CheckPermissionSimple(Perm_Account, Permission_SelfDiag))
		{
			int remainTime = cooldownManager.cooldownList[r].GetRemainTimeMS();
			char buffer[64];
			Util::SafeFormat(buffer, sizeof(buffer), "Cooldown %d is active (%d ms remain)", cooldownCategory, remainTime);
			simulatorPtr->SendInfoMessage(buffer, INFOMSG_INFO);
		}
	}
	return (r >= 0);
}

void CreatureInstance :: RegisterCooldown(int cooldownCategory, int duration)
{
	cooldownManager.AddCooldown(cooldownCategory, duration, 0);
}

void CreatureInstance :: ForceAbilityActivate(int abilityID, int abilityEvent, int targetID)
{
	if(abilityEvent == 4 && g_Config.DebugVerbose) {
		g_Log.AddMessageFormat("ForceAbilityActivate INTERRUPT ABILITY %d : %d", CreatureDefID, abilityID);
	}

	int wpos = 0;
	wpos += PutByte(&GSendBuf[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&GSendBuf[wpos], 0);

	wpos += PutInteger(&GSendBuf[wpos], CreatureID);   //actorID
	wpos += PutShort(&GSendBuf[wpos], abilityID);     //abId
	wpos += PutByte(&GSendBuf[wpos], abilityEvent);      //event

	wpos += PutInteger(&GSendBuf[wpos], (targetID > 0) ? 1 : 0);   //target_len
	if(targetID > 0)
		wpos += PutInteger(&GSendBuf[wpos], targetID);

	wpos += PutInteger(&GSendBuf[wpos], 0);   //secondary_len
	wpos += PutByte(&GSendBuf[wpos], 0);      //has_ground

	if(abilityEvent == 1)
	{
		wpos += PutByte(&GSendBuf[wpos], ab[0].willChargesSpent);      //willCharges
		wpos += PutByte(&GSendBuf[wpos], ab[0].mightChargesSpent);     //mightCharges
	}

	PutShort(&GSendBuf[1], wpos - 3);       //Set message size
	//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}

void CreatureInstance :: SimulateEffect(const char *effect, CreatureInstance *target)
{
	//Take a pointer to a target since it's easier to check for a NULL pointer
	//here than elsewhere, although it's unlikely that this should ever be
	//called with an invalid pointer.

	int targetID = 0;
	if(target != NULL)
		targetID = target->CreatureID;

	int wpos = PrepExt_SendEffect(GSendBuf, CreatureID, effect, targetID);
	//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}


void CreatureInstance :: SendAutoAttack(int abilityID, int targetID)
{
	int wpos = 0;

	if(serverFlags & ServerFlags::IsPlayer)
	{
		wpos += PrepExt_AbilityEvent(&GSendBuf[wpos], CreatureID, abilityID, AbilityStatus::REQUEST_ACCEPTED);
		wpos += PrepExt_AbilityEvent(&GSendBuf[wpos], CreatureID, abilityID, AbilityStatus::ABILITY_FINISHED);
	}
	int tpos = wpos;
	wpos += PutByte(&GSendBuf[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&GSendBuf[wpos], 0);

	wpos += PutInteger(&GSendBuf[wpos], CreatureID);  //actorID
	wpos += PutShort(&GSendBuf[wpos], abilityID);     //abId
	wpos += PutByte(&GSendBuf[wpos], 1);              //event

	wpos += PutInteger(&GSendBuf[wpos], 1);   //target_len
	wpos += PutInteger(&GSendBuf[wpos], targetID);   //target
	wpos += PutInteger(&GSendBuf[wpos], 0);   //secondary_len
	wpos += PutByte(&GSendBuf[wpos], 0);      //has_ground
	if(g_ProtocolVersion > 20)  //TODO: Unknown protocol version implementation
	{
		wpos += PutByte(&GSendBuf[wpos], 0);      //willCharges
		wpos += PutByte(&GSendBuf[wpos], 0);      //mightCharges
	}
	PutShort(&GSendBuf[tpos + 1], wpos - tpos - 3);       //Set message size
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);


	/*
	int tpos = 0;
	int wpos = 0;

	wpos += PutByte(&GSendBuf[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&GSendBuf[wpos], 0);

	wpos += PutInteger(&GSendBuf[wpos], CreatureID);  //actorID
	wpos += PutShort(&GSendBuf[wpos], abilityID);     //abId
	wpos += PutByte(&GSendBuf[wpos], 7);              //event

	wpos += PutInteger(&GSendBuf[wpos], 0);   //target_len
	wpos += PutInteger(&GSendBuf[wpos], 0);   //secondary_len
	wpos += PutByte(&GSendBuf[wpos], 0);      //has_ground
	PutShort(&GSendBuf[tpos + 1], wpos - tpos - 3);       //Set message size

	tpos = wpos;
	wpos += PutByte(&GSendBuf[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&GSendBuf[wpos], 0);

	wpos += PutInteger(&GSendBuf[wpos], CreatureID);  //actorID
	wpos += PutShort(&GSendBuf[wpos], abilityID);     //abId
	wpos += PutByte(&GSendBuf[wpos], 8);              //event

	wpos += PutInteger(&GSendBuf[wpos], 0);   //target_len
	wpos += PutInteger(&GSendBuf[wpos], 0);   //secondary_len
	wpos += PutByte(&GSendBuf[wpos], 0);      //has_ground
	PutShort(&GSendBuf[tpos + 1], wpos - tpos - 3);       //Set message size

	tpos = wpos;
	wpos += PutByte(&GSendBuf[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&GSendBuf[wpos], 0);

	wpos += PutInteger(&GSendBuf[wpos], CreatureID);  //actorID
	wpos += PutShort(&GSendBuf[wpos], abilityID);     //abId
	wpos += PutByte(&GSendBuf[wpos], 1);              //event

	wpos += PutInteger(&GSendBuf[wpos], 1);   //target_len
	wpos += PutInteger(&GSendBuf[wpos], targetID);   //target
	wpos += PutInteger(&GSendBuf[wpos], 0);   //secondary_len
	wpos += PutByte(&GSendBuf[wpos], 0);      //has_ground
	if(g_ProtocolVersion > 20)  //TODO: Unknown protocol version implementation
	{
		wpos += PutByte(&GSendBuf[wpos], 0);      //willCharges
		wpos += PutByte(&GSendBuf[wpos], 0);      //mightCharges
	}
	PutShort(&GSendBuf[tpos + 1], wpos - tpos - 3);       //Set message size

	//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
	*/
}

void CreatureInstance :: ActivateSavedAbilities(void)
{
	std::vector<ActiveBuff>::iterator it;
	if(g_Config.PersistentBuffs) {
		if(buffManager.buffList.size() > 0)
			/* First copy active buffs to persistent buffs. This is done because is the player logs out
			 * and in again quickly, the character won't have been reloaded yet. If they had buffs
			 * active at the time, just treat them as persistent.
			 */
			buffManager.ActiveToPersistent();
		else
			g_Log.AddMessageFormat("No active buffs, %d persistent buffs", buffManager.persistentBuffList.size());

		initialisingAbilities = true;

		// Target self
		ab[0].TargetCount = 1;
		ab[0].TargetList[0] = this;

		for(it = buffManager.persistentBuffList.begin(); it != buffManager.persistentBuffList.end(); ++it) {
//			CallAbilityEvent(it->abID, EventType::onRequest);
			CallAbilityEvent(it->abID, EventType::onActivate);
		}
		ab[0].bPending = false;
		initialisingAbilities = false;
	}
}


int CreatureInstance :: RemoveCreatureReference(CreatureInstance *target)
{
	//Remove all pointers this object may have to a target creature.  This is typically to be
	//called before the target object will be removed, to prevent dangling pointers.

	int rcount = 0;
	if(CurrentTarget.targ == target)
	{
		SelectTarget(NULL);

		//For creatures, notify them to rescan their hate information to acquire a new
		//target if necessary.  The flag update is needed for the function to proceed.
		if(!(serverFlags & ServerFlags::IsPlayer))
			CheckMostHatedTarget(true);
		rcount++;
	}
	/*
	if(CurrentTarget.aaTarg == ref)
	{
		SetAutoAttack(NULL, -1);
		rcount++;
	}*/
	int a;
	int b;
	int abSlot = 0;
	for(abSlot = 0; abSlot <= 1; abSlot++)
	{
		if(ab[abSlot].bPending == true)
		{
			for(a = 0; a < ab[abSlot].TargetCount; a++)
			{
				if(ab[abSlot].TargetList[a] == target)
				{
					ab[abSlot].TargetList[a] = NULL;  //Works for clearing last slot.
					for(b = a; b < ab[abSlot].TargetCount - 1; b++)
						ab[abSlot].TargetList[b] = ab[abSlot].TargetList[b + 1];
					ab[abSlot].TargetCount--;
					rcount++;
					break;
				}
			}
			if(ab[abSlot].TargetCount == 0)
				CancelPending_Ex(&ab[abSlot]);
		}
	}
	return rcount;
}

void CreatureInstance :: RunDialog(void)
{
	if(timer_dialog != 1) {
		/* Special value of 1 for timer indicates we have checked this NPC before and it doesn't need
		 * dialog
		 */
		if(CurrentTarget.targ == NULL && (timer_dialog == 0 || g_ServerTime > timer_dialog))
		{
			if(spawnGen != NULL && spawnGen->spawnPoint != NULL && spawnGen->spawnPoint->extraData != NULL && spawnGen->spawnPoint->extraData != NULL && strcmp( spawnGen->spawnPoint->extraData->dialog, "") != 0) {
				/* Only run dialog when there is 1) a new timer or timer triggers 2) no target */
				NPCDialogItem *diag = g_NPCDialogManager.GetItem(spawnGen->spawnPoint->extraData->dialog);
				if(diag != NULL && diag->mParagraphs.size() > 0) {
					/* If the index is -1, this is the very first dialog paragraph, so wait for the
					 * initial delay before actually performing it
					 */
					if(dialogIndex == -1) {
						switch(diag->mSequence) {
						case Sequence::SEQUENTIAL:
							dialogIndex = 0;
							break;
						case Sequence::RANDOM:
							dialogIndex = randmodrng(0, diag->mParagraphs.size());
							break;
						}
						timer_dialog = g_ServerTime + randmodrng(diag->mMinInterval, diag->mMaxInterval);
					}
					else {
						int delay = randmodrng(diag->mMinInterval, diag->mMaxInterval);

						/* Otherwise perform this item */
						NPCDialogParagraph para = diag->mParagraphs[dialogIndex];
						switch(para.mType) {
						case ParagraphType::SAY:
							actInst->LSendToAllSimulator(GSendBuf, PrepExt_GenericChatMessage(GSendBuf, CreatureID, css.display_name, "s/", para.mValue.c_str()), -1);
							break;
						case ParagraphType::WAIT:
							delay = atoi(para.mValue.c_str());
							break;
						case ParagraphType::EMOTE:
							actInst->LSendToAllSimulator(GSendBuf, PrepExt_GenericChatMessage(GSendBuf, CreatureID, css.display_name, "emote", para.mValue.c_str()), -1);
							break;
						}

						/* Wait for next action, reseting if needed */
						timer_dialog = g_ServerTime + delay;

						switch(diag->mSequence) {
						case Sequence::SEQUENTIAL:
							dialogIndex++;
							if(dialogIndex >= diag->mParagraphs.size())
								dialogIndex = 0;
							break;
						case Sequence::RANDOM:
							dialogIndex = randmodrng(0, diag->mParagraphs.size());
							break;
						}
					}
				}
				else {
					g_Log.AddMessageFormat("[WARNING] Request for unknown or empty NPC dialog %s", spawnGen->spawnPoint->extraData->dialog);
					/* Never bother again */
					timer_dialog = -1;
				}
			}
			else {
				/* Never bother again */
				timer_dialog = -1;
			}
		}
	}

}

void CreatureInstance :: RunAIScript(void)
{
	if(aiScript != NULL)
	{
		if(CurrentTarget.targ != NULL)
			aiScript->RunAtSpeed(5);
		else
		{
			if(aiScript->CanRunIdle() == true)
				aiScript->RunSingleInstruction();
		}
	}
	else if(aiNut != NULL) {
		if(CurrentTarget.targ != NULL)
			aiNut->Tick();
		else
		{
			if(aiNut->def->CanIdle())
				aiNut->Tick();
		}
	}
}

float CreatureInstance :: GetMaxInvisibilityDistance(bool dexBased)
{
	//Note: stat minimum is 50 in the client, and is converted to meters (stat * 10)
	if(HasStatus(StatusEffects::WALK_IN_SHADOWS) || dexBased == true)
		return Util::ClipIntMin(css.dexterity, 50) * 10.0F;
	if(HasStatus(StatusEffects::INVISIBLE) || dexBased == false)
		return Util::ClipIntMin(css.psyche, 50) * 10.0F;
	return 0.0F;
}

float CreatureInstance :: GetRegenInvisibilityDistance(bool dexBased)
{
	//If the max regen is (stat * 10) then it will take
	//2 seconds every 10%, for 20 second charge time.
	if(HasStatus(StatusEffects::WALK_IN_SHADOWS) || dexBased == true)
		return Util::ClipIntMin(css.dexterity, 50) / 1.5F;
	if(HasStatus(StatusEffects::INVISIBLE) || dexBased == false)
		return Util::ClipIntMin(css.psyche, 50) / 1.5F;
	return 0.0F;
}

//Determine if a creature is capable of responding to threats and performing certain
//triggers, such as block, parry, and dodge.
bool CreatureInstance :: IsCombatReactive(void)
{
	if(HasStatus(StatusEffects::STUN) == true)
		return false;
	if(HasStatus(StatusEffects::DAZE) == true)
		return false;

	return true;
}

void CreatureInstance :: ProcessRegen(void)
{
	if(g_ServerTime < timer_lasthealthupdate)
		return;

	//Don't allow regeneration for players if they're in noncombat stage.
	//In theory this could allow players to regen to full while loading,
	//then log out before they die, to repeat the process.
	if(serverFlags & ServerFlags::IsPlayer)
		if(serverFlags & ServerFlags::Noncombatant)
			return;

	if(Speed == 0)
	{
		if(HasStatus(StatusEffects::WALK_IN_SHADOWS) || HasStatus(StatusEffects::INVISIBLE))
		{
			bool dexBasedRegen = true;
			if(HasStatus(StatusEffects::INVISIBLE))  //Mage invisibility uses psyche
				dexBasedRegen = false;
			float newDist = css.invisibility_distance + GetRegenInvisibilityDistance(dexBasedRegen);
			float maxDist = GetMaxInvisibilityDistance(dexBasedRegen);
			if(newDist >= maxDist)
				newDist = maxDist;
			if(!(Util::FloatEquivalent(css.invisibility_distance, newDist)))
			{
				css.invisibility_distance = newDist;
				//SetServerFlag(ServerFlags::InvisDistChanged, true);
				pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
			}
		}
	}
	/*
	if(serverFlags & ServerFlags::InvisDistChanged)
	{
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::INVISIBILITY_DISTANCE);
		SetServerFlag(ServerFlags::InvisDistChanged, false);
	}
	*/

	int maxhealth = GetMaxHealth(true);
	int health = css.health;
	if(health < maxhealth)
	{
		RunHealTick();
	}
	else if(health > maxhealth)
	{
		css.health = CalcRestrictedHealth(maxhealth, true);
		int wpos = 0;
		wpos = PrepExt_SendHealth(GSendBuf, CreatureID, css.health);
		//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
	}
	timer_lasthealthupdate = g_ServerTime + 2000;
}

void CreatureInstance :: ProcessAutoAttack(void)
{
	if(g_ServerTime < timer_autoattack)
		return;

	if(HasStatus(StatusEffects::STUN))
		return;

	if(HasStatus(StatusEffects::DISARM))
		return;
	if(HasStatus(StatusEffects::DAZE))
		return;

	//Attack speed is adjusted by a percent.  Negative values reduce speed.
	long nextDelay = 2000;
	nextDelay = 2000;
	if(css.melee_attack_speed != 0)
		nextDelay -= (int)(2000.0F * ((float)css.melee_attack_speed / 1000.0F));
	if(nextDelay < 500)
		nextDelay = 500;  //Lower bound.

	//g_Log.AddMessageFormat("timer_autoattack: %d", timer_autoattack);

	int atRes = 0;
	if(HasStatus(StatusEffects::AUTO_ATTACK)) {
		atRes = CallAbilityEvent(ABILITYID_AUTO_ATTACK, EventType::onActivate);
	}
	if(HasStatus(StatusEffects::AUTO_ATTACK_RANGED)) {
		atRes = CallAbilityEvent(ABILITYID_AUTO_ATTACK_RANGED, EventType::onActivate);
	}

	//Ability error handling for mobs, since auto attacks don't have a range check like
	//scripted explicit ability attacks do.  Without this check, creatures with
	//auto-attacks only would stop tracking their target after the initial movement
	//to get within range.
	if(atRes != 0)
	{
		int errCode = atRes & 0xFF;
		if(serverFlags & ServerFlags::IsPlayer)
		{
			//For non-critical errors such as casting requirements, reduce the timer
			//to retry very soon.
			switch(errCode)
			{
			case Ability2::ABILITY_NO_TARGET:
				SetAutoAttack(NULL, -1);
				break;
			case Ability2::ABILITY_GENERIC:
			case Ability2::ABILITY_PENDING:
			case Ability2::ABILITY_INRANGE:
			case Ability2::ABILITY_FACING:
				nextDelay = 200;
			}
		}
		else
		{
			//For NPC creatures, need to set movement requirements.
			switch(errCode)
			{
			case Ability2::ABILITY_NO_TARGET:
				SetAutoAttack(NULL, -1);
				break;
			case Ability2::ABILITY_INRANGE:
				//Need to check existing range.  If it's set, it's probably already trying to move
				//to it's target, and resetting the movement time to this frame will cause
				//hyper accelerated movement steps when following targets.
				if(CurrentTarget.desiredRange == 0)
					movementTime = g_ServerTime;

				CurrentTarget.desiredRange = (atRes >> 8) & 0xFFFF;

				//Reduce the delay until the next attack attempt, to increase response times.
				nextDelay = 200;
				break;
			}
		}
	}

	timer_autoattack = g_ServerTime + nextDelay;
}

void CreatureInstance :: CheckMostHatedTarget(bool forceCheck)
{
	if(forceCheck == true)
		SetServerFlag(ServerFlags::HateInfoChanged, true);

	if(hateProfilePtr == NULL)
		return;
	if(!(serverFlags & ServerFlags::HateInfoChanged))
		return;
	if(g_ServerTime < hateProfilePtr->tauntReleaseTime)  //Taunt holds attention
		return;
	if(hateProfilePtr->CheckRefreshAndUpdate() == false)
		return;

	SetServerFlag(ServerFlags::HateInfoChanged, false);

	int CDefID = 0;
	CreatureInstance *targ = NULL;
	for(size_t i = 0; i < hateProfilePtr->hateList.size(); i++)
	{
		CDefID = hateProfilePtr->hateList[i].CDefID;
		targ = actInst->GetPlayerByCDefID(CDefID);
		if(targ == NULL)
			continue;

		if((serverFlags & ServerFlags::Noncombatant) !=0)
		{
			//Set hate to zero.  On the next sort, this creature will drop to the bottom of the list
			//to help avoid rechecking this creature again during every update cycle.
			hateProfilePtr->hateList[i].hate = 0;
			continue;
		}

		if(targ->HasStatus(StatusEffects::DEAD))
		{
			//Set hate to zero.  On the next sort, this creature will drop to the bottom of the list
			//to help avoid rechecking this creature again during every update cycle.
			hateProfilePtr->hateList[i].hate = 0;
			continue;
		}

		int dist = ActiveInstance::GetPlaneRange(this, targ, AGGRO_MAXIMUM_RANGE);
		if(dist <= AGGRO_MAXIMUM_RANGE)
		{
			SelectTarget(targ);

			if(serverFlags & ServerFlags::Taunted)
			{
				SetServerFlag(ServerFlags::Taunted, false);
				if(dist > TAUNT_BECKON_DISTANCE)
					MoveTo(targ->CurrentX, targ->CurrentZ, TAUNT_BECKON_DISTANCE, 0);
			}
			return;
		}
	}
}

void CreatureInstance :: RunAutoTargetSelection(void)
{
	if(g_ServerTime < actInst->mNextTargetUpdate)
		return;

	CheckMostHatedTarget(false);
	if(CurrentTarget.targ != NULL)
	{
		return;
	}

	//Don't allow targets if mobs are returning to their tether location
	if(serverFlags & ServerFlags::LeashRecall)
		return;

	if((serverFlags & ServerFlags::NeutralInactive) || (serverFlags & ServerFlags::Stationary) ||  (serverFlags & ServerFlags::Noncombatant))
		return;

	CreatureInstance *targ = NULL;
	targ = actInst->GetClosestAggroTarget(this);
	if(targ != NULL)
		SelectTarget(targ);
}

//Return true if an NPC passes all conditions necessary to perform a movement update.
//Assumes the NPC is alive.
bool CreatureInstance :: isNPCReadyMovement(void)
{
	if(serverFlags & ServerFlags::Stationary)
		return false;
	if(g_ServerTime < movementTime)
		return false;
	if(HasStatus(StatusEffects::STUN))
		return false;
	if(HasStatus(StatusEffects::DAZE))
		return false;
	if(HasStatus(StatusEffects::ROOT))
		return false;
	if(ab[0].IsBusy() == true)
		return false;
	return true;
}

void CreatureInstance :: RunProcessingCycle(void)
{
	CheckActiveStatusEffects();
	RunActiveAbilities();

	if(css.health > 0 && NotStatus(StatusEffects::DEAD))
	{
		ProcessRegen();
		CheckUpdateTimers();

		if(serverFlags & ServerFlags::IsNPC)
		{
			if(serverFlags & ServerFlags::LocalActive)
			{
				if(isNPCReadyMovement() == true)
				{
					int r = 0;
					UpdateDestination();
					r += RotateToTarget();
					r += RunMovementStep();
					if(r > 0)
					{

						int size = PrepExt_GeneralMoveUpdate(GSendBuf, this);
						actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
					}
				}
				RunAutoTargetSelection();
				RunAIScript();
				RunDialog();
			}
		}
		else if(serverFlags & ServerFlags::IsPlayer)
		{
			if(serverFlags & ServerFlags::InitAttack)
			{
				actInst->UpdateSidekickTargets(this);
				serverFlags -= ServerFlags::InitAttack;
			}

			if(charPtr->inventory.NextExpunge != 0 && g_ServerTime >= charPtr->inventory.NextExpunge) {
				int r = charPtr->inventory.RemoveExpiredItemsAndUpdate(GSendBuf);
				if(r > 0)
					simulatorPtr->AttemptSend(GSendBuf, r);
			}
		}
		else if(serverFlags & ServerFlags::IsSidekick)
		{
			if(aiScript != NULL || aiNut != NULL)
				if(CurrentTarget.targ != NULL)
					RunAIScript();

			if(CurrentTarget.targ == NULL)
			{
				//Don't select a target if called back, is a noncombatant, or the officer is a noncombatant.
				//Since a player logging in will be marked as noncombatant, and we don't want sidekicks
				//offering free kills while the player is invulnerable.
				if((!(serverFlags & ServerFlags::CalledBack)) && (!(serverFlags & ServerFlags::Noncombatant)))
					if(!(AnchorObject->serverFlags & ServerFlags::Noncombatant))
						SelectTarget(actInst->GetClosestEnemy(this));
			}

			if(g_ServerTime >= movementTime)
				MoveToTarget_Ex2();
		}
	}
	else //Creature is dead
	{
		if(serverFlags & serverFlags & ServerFlags::IsNPC)
			CheckLootTimers();
	}
}

void CreatureInstance :: CheckPathLocation(void)
{
	//Don't calculate paths unless idle.
	if(CurrentTarget.targ != NULL)
		return;

	//The Previous and Next node are both initialized to the spawning point, if it has
	//a link.  If this is ever zero, the spawner hasn't been assigned a linked node,
	//so avoid all remaining conditional checks to find one.
	if(previousPathNode == 0)
		return;

	if(CurrentTarget.DesLocX != 0 && CurrentTarget.DesLocZ != 0)
		return;

	//Need to select a new target using the next node as a search point.
	//SceneryObject *so = g_SceneryManager.GetPropPtr(actInst->mZone, nextPathNode, NULL);
	SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, nextPathNode, NULL);
	if(so == NULL)
	{
		//g_Log.AddMessageFormat("[WARNING] CheckPathLocation() prop not found: %d", nextPathNode);
		return;
	}

	//Prop needs attached data.
	if(so->extraData == NULL)
		return;

	//Needs linked points.
	if(so->extraData->linkCount == 0)
	{
		g_Log.AddMessageFormat("[WARNING] CheckPathLocation() no links on prop: %d", nextPathNode);
		return;
	}

	//Build a list of available nodes, excluding any node that would return to the previous
	//node.
	vector<int> openNode;
	for(int i = 0; i < so->extraData->linkCount; i++)
	{
		if(so->extraData->link[i].type == SceneryObject::LINK_TYPE_PATH && so->extraData->link[i].propID != previousPathNode)
		{
			openNode.push_back(so->extraData->link[i].propID);
			//g_Log.AddMessageFormat("Found %d off %d", so->extraData->link[i].propID, nextPathNode);
		}
	}

	//If there are no new points, defaulting to the previous node is the only choice.
	int newPathNode = previousPathNode;
	if(openNode.size() > 0)
	{
		int rndLink = randint(0, openNode.size() - 1);
		newPathNode = openNode[rndLink];
	}

	//g_Log.AddMessageFormat("newPathNode: %d", newPathNode);

	//Search for the prop if it doesn't match the one we started with.
	SceneryObject *newWaypoint = so;
	if(newPathNode != nextPathNode)
		newWaypoint = g_SceneryManager.GlobalGetPropPtr(actInst->mZone, newPathNode, NULL);
		//newWaypoint = g_SceneryManager.GetPropPtr(actInst->mZone, newPathNode, NULL);

	if(newWaypoint == NULL)
	{
		//g_Log.AddMessageFormat("[WARNING] CheckPathLocation() linked prop not found: %d (from origin: %d, at %g %g, previous: %d)", newPathNode, nextPathNode, so->LocationX, so->LocationZ, previousPathNode);
		return;
	}

	//Now that the target node is selected, save the current node as the previous node.
	//g_Log.AddMessageFormat("Sending to new node: %d", newPathNode);
	previousPathNode = nextPathNode;
	nextPathNode = newPathNode;

	SetServerFlag(ServerFlags::PathNode, true);
	CurrentTarget.DesLocX = (int)newWaypoint->LocationX;
	CurrentTarget.DesLocZ = (int)newWaypoint->LocationZ;
	//tetherNodeX = (int)newWaypoint->LocationX;   Disabled for new test system.  Remove if no problems.
	//tetherNodeZ = (int)newWaypoint->LocationZ;

	//Set to the target point's elevation.  If a spawnpoint is in a dungeon environment (has a ceiling)
	//and proceeds towards a path node down a slope, the elevation will now be in the ceiling, causing
	//the mob to appear stuck there.
	CurrentY = (int)newWaypoint->LocationY;
	
	// *********************************
	// REMOVE THIS COMMENT - JUST EXPERIMENTING VVVVVV
	//*********************************

	Speed = CREATURE_WALK_SPEED;
}

int CreatureInstance :: RotateToTarget(void)
{
	if(CurrentTarget.targ == NULL)
		return 0;

	int oldRot = Rotation;

	int xlen = CurrentTarget.targ->CurrentX - CurrentX;
	int zlen = CurrentTarget.targ->CurrentZ - CurrentZ;
	float rotf = (float)atan2((double)xlen, (double)zlen);
	unsigned char rot = (unsigned char)(rotf * 255.0F / 6.283185F);

	if(oldRot != rot)
	{
		Rotation = rot;
		Heading = rot;
		return 1;
	}
	return 0;
}

int CreatureInstance :: RunMovementStep(void)
{
	int nextMoveTime = CREATURE_MOVEMENT_NONCOMBAT_FREQUENCY;

	if(CurrentTarget.targ != NULL)
		nextMoveTime = CREATURE_MOVEMENT_COMBAT_FREQUENCY;

	if(CurrentTarget.DesLocX == 0 && CurrentTarget.DesLocZ == 0)
	{
		movementTime = g_ServerTime + CREATURE_MOVEMENT_NONCOMBAT_FREQUENCY;
		return 0;
	}

	int xlen = CurrentTarget.DesLocX - CurrentX;
	int zlen = CurrentTarget.DesLocZ - CurrentZ;
	int dist = (int)sqrt((double)((xlen * xlen) + (zlen * zlen)));
	float rotf = (float)atan2((double)xlen, (double)zlen);
	unsigned char rot = (unsigned char)(rotf * 255.0F / 6.283185F);
	Rotation = rot;
	Heading = rot;

	//Go slightly further to prevent any issues with being just barely out of range.
	//The desired range is only used for attacks, so it may be zero.  Compensate for
	//values that might be too low or negative.  For reference, melee range is 30
	//units.
	int desRange = CurrentTarget.desiredRange - 5;
	if(CurrentTarget.targ != NULL)
		if(desRange > 0 && CurrentTarget.targ->Speed != 0)
		{
			//If the target is moving, we'll want to get closer so we're not constantly skirting
			//the edge radius and triggering more movement requirements..
			desRange /= 2;
		}

	int closeEnough = desRange;
	if(closeEnough < 15)
		closeEnough = 15;

	if(dist < closeEnough)
	{
		if(serverFlags & ServerFlags::PathNode)
			SetServerFlag(ServerFlags::PathNode, false);

		if(serverFlags & ServerFlags::LeashRecall)
		{
			SetServerFlag(ServerFlags::LeashRecall, false);
			//_RemoveStatusList(StatusEffects::UNATTACKABLE);
		}

		Speed = 0;
		StopMovement(ScriptCore::Result::OK);
		SetServerFlag(ServerFlags::CalledBack, false);
	}
	else
	{
		//Estimated in game, 500 units in 10 seconds at 100% speed
		// = 112.5 units in 2.25 seconds at 100% speed
		// = 50.0 units in 1.0 seconds.

		int distRemain = dist - desRange;
		if(distRemain > 0)
		{
			//Improve movement accuracy in active combat by reducing the delays between updates.
			int moveSpeed = CREATURE_WALK_SPEED;

			// Scripting can request a particular speed
			if(CurrentTarget.targ != NULL)
			{
				nextMoveTime = CREATURE_MOVEMENT_COMBAT_FREQUENCY;
				moveSpeed = CREATURE_RUN_SPEED;
			}
			else
			{
				nextMoveTime = CREATURE_MOVEMENT_NONCOMBAT_FREQUENCY;
				if(serverFlags & ServerFlags::LeashRecall)
					moveSpeed = CREATURE_RUN_SPEED;
			}

			if(CurrentTarget.desiredSpeed != 0) {
				moveSpeed = CurrentTarget.desiredSpeed;
			}

			int maxSpeed = moveSpeed + css.mod_movement;  //the mod may be negative
			if(css.level <= SLOW_CREATURE_LEVEL_THRESHOLD)
				maxSpeed -= SLOW_CREATURE_SPEED_PENALTY;

			maxSpeed = Util::ClipInt(maxSpeed, 0, 255);  //Maximum byte representation
			Speed = maxSpeed;

			//The number of units moved per second at this speed.
			float distPerSecond = ((float)maxSpeed / 100.0F) * DEFAULT_CREATURE_SPEED;

			//The number of units to move for this update.
			int updateDist = (int)(distPerSecond * ((float)nextMoveTime / 1000.0F));

			//Don't move any further than the required distance.
			if(updateDist > distRemain)
			{
				updateDist = distRemain;

				//By setting the speed to zero here, the client object won't overshoot the
				//target, and the incremental position update will take care of the
				//final visible movement in the client.  Otherwise the creature will
				//keep moving until the next update, and may appear to run past its
				//target.
				Speed = 0;
			}

			//Hack to prevent very small distance updates (such as a required movement of
			//1 unit) from being rounded to zero, causing the creature to get stuck in a
			//movement loop.
			if(updateDist < 5)
				updateDist = 5;

			float angle = (float)Rotation * 6.283185F / 255.0F;

			int oldX = CurrentX;
			int oldZ = CurrentZ;

			CurrentZ += (int)((float)updateDist * cos(angle));
			CurrentX += (int)((float)updateDist * sin(angle));

			int xlen = oldX - CurrentX;
			int zlen = oldZ - CurrentZ;
			double actDist = sqrt(double((xlen * xlen) + (zlen * zlen)));
			//g_Log.AddMessageFormat("Update Dist: %d   Spd:%d   Time:%d   Dist/s:%g   Remain:%d   Actual:%g", updateDist, Speed, nextMoveTime, distPerSecond, distRemain, actDist);
		}
		else
		{
			Speed = 0;
		}
	}
	movementTime = g_ServerTime + nextMoveTime;
	return 1;
}

void CreatureInstance :: UpdateDestination(void)
{
	//Update the destination point based on pathfinding, target location, etc.
	if(CurrentTarget.targ == NULL)
	{
		if(serverFlags & ServerFlags::IsNPC)
		{
			CheckPathLocation();

			//Update the current position no matter where the path might be, as long as the path
			//is being followed.
			if((serverFlags & ServerFlags::PathNode) && !(serverFlags & ServerFlags::LeashRecall))
			{
				tetherNodeX = CurrentX;
				tetherNodeZ = CurrentZ;
			}
		}
	}
	else
	{
		//If the script system has set a desired range, it means the
		//creature needs to be closer to its target.
		if(CurrentTarget.desiredRange != 0 )
		{
			CurrentTarget.DesLocX = CurrentTarget.targ->CurrentX;
			CurrentTarget.DesLocZ = CurrentTarget.targ->CurrentZ;
			Speed = 100;
		}
	}
	CheckLeashMovement();
}

void CreatureInstance :: CheckLeashMovement(void)
{
	//Leash requires a spawner, so we can't proceed without one.
	if(spawnGen == NULL)
		return;
	if(spawnGen->spawnPoint == NULL)
		return;
	if(spawnGen->spawnPoint->extraData == NULL)
		return;

	//If we're already flagged for returning to a leash location, just skip all the stuff in here to
	//prevent recalculating every cycle.
	if(serverFlags & ServerFlags::LeashRecall)
		return;

	int maxLeash = spawnGen->spawnPoint->extraData->GetLeashLength();
	if(actInst->mZoneDefPtr->IsDungeon() == true)
	{
		int leashLimit = actInst->mZoneDefPtr->mMaxLeashRange;
		if(leashLimit > 0 && maxLeash > leashLimit)
			maxLeash = leashLimit;
	}

	int wanderRadius = spawnGen->spawnPoint->extraData->wanderRadius;
	int outerRadius = spawnGen->spawnPoint->extraData->outerRadius;
	if(wanderRadius == 0)
	{
		if(spawnGen->spawnPackage != NULL)
			wanderRadius = spawnGen->spawnPackage->wanderRadius;
	}

	//Use minLeashRange to determine the shortest range where there will be no return at all. actual distance checks since we need to preeserve wanderRadius
	//for scatter distance later on.
	int minLeashRange = wanderRadius;
	if(outerRadius > minLeashRange)
		minLeashRange = outerRadius;

	//Hack to make sure it has free reign to walk wherever the script may need it to go.
	//Otherwise the target location will trigger the "too far out of range, run fast" return style.
	if(serverFlags & ServerFlags::ScriptMovement)
	{
		//We have a specific location we must get to, ignore any leash calculations.
		if((CurrentTarget.targ == NULL) && (CurrentTarget.DesLocX != 0 || CurrentTarget.DesLocZ != 0))
			return;
	}

	if(maxLeash > 0)
	{
		int xlen = abs(CurrentX - tetherNodeX);
		int zlen = abs(CurrentZ - tetherNodeZ);
		if((xlen > maxLeash) || (zlen > maxLeash))
		{
			//Max leash is broken.  Force return to the tether location.
			SelectTarget(NULL);

			Speed = CREATURE_RUN_SPEED;
			MoveTo(tetherNodeX, tetherNodeZ, 0, 0);

			UnHate();
			RemoveAttachedHateProfile();
			SetServerFlag(ServerFlags::LeashRecall, true);
			css.health = CalcRestrictedHealth(-1, true);
			int size = PrepExt_SendHealth(GSendBuf, this->CreatureID, css.health);
			actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
		}
		else
		{
			//If we don't have a target object or target location, check to see if we're outside
			//the minimum range (like wander radius) and return casually.
			if(CurrentTarget.targ == NULL && CurrentTarget.DesLocX == 0 && CurrentTarget.DesLocZ == 0)
			{
				if((xlen > minLeashRange) || (zlen > minLeashRange))
				{
					Speed = CREATURE_JOG_SPEED;
					MoveTo(tetherNodeX, tetherNodeZ, 0, 0);
				}
			}
		}
	}


	//While we're here, calculate a new wander location, but only if we're idle.
	if(CurrentTarget.targ != NULL)
		return;

	if(wanderRadius > 0 && CurrentTarget.DesLocX == 0 && CurrentTarget.DesLocZ == 0)
	{
		MoveTo(tetherNodeX + randint(-wanderRadius, wanderRadius), tetherNodeZ + randint(-wanderRadius, wanderRadius), 0, 0);
		Speed = CREATURE_WALK_SPEED;
	}
}

void CreatureInstance :: StopMovement(int result)
{
	if((CurrentTarget.DesLocX != 0 || CurrentTarget.DesLocZ != 0) && IsAtTether() && tetherFacing != -1) {
		Rotation = tetherFacing;
		Heading = tetherFacing;
	}

	if(scriptMoveEvent != -1) {
		if(actInst != NULL && actInst->nutScriptPlayer != NULL) {
			ScriptCore::NutScriptEvent *nse = actInst->nutScriptPlayer->GetEvent(scriptMoveEvent);
			if(nse != NULL) {
				nse->mCallback->mResult = result;
				ScriptCore::NutCondition *nsc = nse->mCondition;
				nse->mCondition = new ScriptCore::TimeCondition(0);
				delete nsc;
			}
		}
		scriptMoveEvent = -1;
	}
	CurrentTarget.DesLocX = 0;
	CurrentTarget.DesLocZ = 0;
	CurrentTarget.desiredRange = 0;
	CurrentTarget.desiredSpeed = 0;
}
void CreatureInstance :: MoveTo(int x, int z, int range, int speed)
{

	StopMovement(ScriptCore::Result::INTERRUPTED);
	CurrentTarget.DesLocX = x;
	CurrentTarget.DesLocZ = z;
	CurrentTarget.desiredRange = range;
	CurrentTarget.desiredSpeed = speed;
}

void CreatureInstance :: MoveToTarget_Ex2(void)
{
	static const int MOV_REQDIST = 50;
	static const int REQDIST = 100;
	static const int FARDIST = 350;
	static const int YOFFSET = 50;
	static const int CLOSE_SCATTER_RANGE = 15;

	//No destination set, check for sidekick return
	if(CurrentTarget.DesLocX == 0 || CurrentTarget.desiredRange != 0)
	{
		if(CurrentTarget.targ == NULL)
		{
			if(AnchorObject == NULL)
				return;

			int dist = actInst->GetPlaneRange(this, AnchorObject, SANE_DISTANCE);
			//int yoffs = abs(CurrentY - AnchorObject->CurrentY);
			if(dist > FARDIST) // || yoffs > YOFFSET)
			{
				CurrentX = AnchorObject->CurrentX + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
				CurrentY = AnchorObject->CurrentY;
				CurrentZ = AnchorObject->CurrentZ + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
				int size = PrepExt_UpdateFullPosition(GSendBuf, this);
				//actInst->LSendToAllSimulator(GSendBuf, size, -1);
				actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
				movementTime = g_ServerTime + 1000;
				return;
			}
			int cmp = REQDIST;
			if(AnchorObject->Speed != 0)
				cmp = MOV_REQDIST;
			if(dist > cmp)
			{
				//Get angle from the perspective of the host (target is self)
				int xlen = CurrentX - AnchorObject->CurrentX;
				int zlen = CurrentZ - AnchorObject->CurrentZ;
				//int xlen = AnchorObject->CurrentX - CurrentX;
				//int zlen = AnchorObject->CurrentZ - CurrentZ;
				float rotf = (float)atan2((double)xlen, (double)zlen);

				//Take this angle and project a point location away from it that's just within
				//the desired range.
				CurrentTarget.desiredRange = cmp;

				if(AnchorObject->Speed == 0)
				{
					CurrentTarget.DesLocZ = AnchorObject->CurrentZ + (int)((float)(REQDIST - 10) * cos(rotf));
					CurrentTarget.DesLocX = AnchorObject->CurrentX + (int)((float)(REQDIST - 10) * sin(rotf));
				}
				else
				{
					CurrentTarget.DesLocX = AnchorObject->CurrentX + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
					CurrentTarget.DesLocZ = AnchorObject->CurrentZ + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
					timer_autoattack = g_ServerTime;
				}
			}
		}
		else
		{
			if(CurrentTarget.desiredRange != 0)
			{
				int dist = actInst->GetPlaneRange(this, CurrentTarget.targ, SANE_DISTANCE);
				if(dist > CurrentTarget.desiredRange)
				{
					//int xlen = CurrentX - CurrentTarget.targ->CurrentX + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
					//int zlen = CurrentZ - CurrentTarget.targ->CurrentZ + randint(-CLOSE_SCATTER_RANGE, CLOSE_SCATTER_RANGE);
					//float rotf = (float)atan2((double)xlen, (double)zlen);

					float rotf = (float)randdbl(-PI, PI);

					//Take this angle and project a point location away from it that's just within
					//the desired range.

					CurrentTarget.DesLocZ = CurrentTarget.targ->CurrentZ + (int)((float)(CurrentTarget.desiredRange - 5) * cos(rotf));
					CurrentTarget.DesLocX = CurrentTarget.targ->CurrentX + (int)((float)(CurrentTarget.desiredRange - 5) * sin(rotf));
				}
			}
		}
	}

	int nextMoveTime = 1000; //2250;

	int tx;
	int tz;
	if(CurrentTarget.DesLocX == 0)
	{
		if(CurrentTarget.targ != NULL)
		{
			tx = CurrentTarget.targ->CurrentX;
			tz = CurrentTarget.targ->CurrentZ;
			nextMoveTime = 500;
		}
		else if(AnchorObject != NULL)
		{
			tx = AnchorObject->CurrentX;
			tz = AnchorObject->CurrentZ;
		}
	}
	else
	{
		tx = CurrentTarget.DesLocX;
		tz = CurrentTarget.DesLocZ;
		nextMoveTime = 500;
	}

	//Get current distance from target point
	int xlen = tx - CurrentX;
	int zlen = tz - CurrentZ;
	//int xlen = CurrentX - CurrentTarget.DesLocX;
	//int zlen = CurrentZ - CurrentTarget.DesLocZ;
	float rotf = (float)atan2((double)xlen, (double)zlen);

	bool update = false;
	unsigned char rot = (unsigned char)(rotf * 256.0F / 6.283185F);
	if(rot != Rotation)
	{
		Rotation = rot;
		Heading = rot;
		update = true;
	}

	if(CurrentTarget.DesLocX == 0)
	{
		if(update == true)
		{
			//Just update rotation and quit.
			Speed = 0;
			int size = PrepExt_UpdateVelocity(GSendBuf, this);
			//actInst->LSendToAllSimulator(GSendBuf, size, -1);
			actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
		}
		movementTime = g_ServerTime + nextMoveTime;
		return;
	}

	update = true;
	int dist = (int)sqrt((double)((xlen * xlen) + (zlen * zlen)));
	if(dist < MOVEMENT_THRESHOLD)
	{
		//g_Log.AddMessageFormat("Stopping (Dist: %d) (Desired: %d)", (int)dist, CurrentTarget.desiredRange);
		Speed = 0;
		CurrentX = CurrentTarget.DesLocX;
		CurrentZ = CurrentTarget.DesLocZ;
		StopMovement(ScriptCore::Result::OK);
		int size = PrepExt_GeneralMoveUpdate(GSendBuf, this);
		//actInst->LSendToAllSimulator(GSendBuf, size, -1);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);
		movementTime = g_ServerTime + nextMoveTime;
		SetServerFlag(ServerFlags::CalledBack, false);
		return;
	}
	else
	{
		int tSpeed = 100;
		if(AnchorObject != NULL)
			if(AnchorObject->Speed > 100)
				tSpeed = AnchorObject->Speed + 20;
		if(tSpeed <= 255)
			Speed = tSpeed;
		else
			Speed = 255;

		//Estimated in game, 500 units in 10 seconds at 100% speed
		// = 112.5 units in 2.25 seconds at 100% speed
		// = 50.0 units in 1.0 seconds.

		int distRemain = dist;

		float stepAmount = 50.0F * ((float)nextMoveTime / 1000.0F);
		int maxmove = (int)(stepAmount * ((float)Speed / 100.0F));
		if(distRemain > maxmove)
			distRemain = maxmove;

		float angle = (float)Heading * 6.283185F / 256.0F;
		CurrentZ += (int)((float)distRemain * cos(angle));
		CurrentX += (int)((float)distRemain * sin(angle));
		//g_Log.AddMessageFormat("distRemain:%d, dist:%d", distRemain, dist);
		if(distRemain < maxmove)
		{
			//nextMoveTime = (int)(900 * ((float)distRemain / (float)maxmove));
			if(nextMoveTime < 150)
				nextMoveTime = 150;
		}
	}

	movementTime = g_ServerTime + nextMoveTime;
	//g_Log.AddMessageFormat("Movement step:%d", nextMoveTime);

	if(update == true)
	{
		int size = PrepExt_GeneralMoveUpdate(GSendBuf, this);
		//actInst->LSendToAllSimulator(GSendBuf, size, -1);
		actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ);

	}
}



void CreatureInstance :: SetAutoAttack(CreatureInstance *target, int statusID)
{
	//Sets AUTO_ATTACK or AUTO_ATTACK_RANGED, disabling the alternate.
	//Clears them both if statusID doesn't match either of them
	if(target == NULL)
	{
		_RemoveStatusList(StatusEffects::AUTO_ATTACK);
		_RemoveStatusList(StatusEffects::AUTO_ATTACK_RANGED);
		//CurrentTarget.aaTarg = NULL;
		return;
	}

	if(statusID == StatusEffects::AUTO_ATTACK)
	{
		_AddStatusList(StatusEffects::AUTO_ATTACK, -1);
		_RemoveStatusList(StatusEffects::AUTO_ATTACK_RANGED);
		if(serverFlags & ServerFlags::IsPlayer)
			serverFlags |= ServerFlags::InitAttack;
		//CurrentTarget.aaTarg = target;
	}
	else if(statusID == StatusEffects::AUTO_ATTACK_RANGED)
	{
		_RemoveStatusList(StatusEffects::AUTO_ATTACK);
		_AddStatusList(StatusEffects::AUTO_ATTACK_RANGED, -1);
		if(serverFlags & ServerFlags::IsPlayer)
			serverFlags |= ServerFlags::InitAttack;
		//CurrentTarget.aaTarg = target;
	}
	else
	{
		_RemoveStatusList(StatusEffects::AUTO_ATTACK);
		_RemoveStatusList(StatusEffects::AUTO_ATTACK_RANGED);
		//CurrentTarget.aaTarg = NULL;
	}
}

void CreatureInstance :: RequestTarget(int CreatureID)
{
	if(actInst == NULL)
	{
		g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] CreatureInstance::RequestTarget actInst is NULL");
		return;
	}
	SelectTarget(actInst->GetInstanceByCID(CreatureID));
}

bool CreatureInstance :: IsAtTether() {

	int xlen = tetherNodeX - CurrentX;
	int zlen = tetherNodeZ - CurrentZ;
	int dist = (int)sqrt((double)((xlen * xlen) + (zlen * zlen)));
	return dist < MOVEMENT_THRESHOLD;
}

void CreatureInstance :: SelectTarget(CreatureInstance *newTarget)
{
	if(newTarget != CurrentTarget.targ)
	{
		// Inform the AI script a target was lost
		if(CurrentTarget.targ != NULL && aiNut != NULL) {
			std::vector<ScriptCore::ScriptParam> p;
			p.push_back(CurrentTarget.targ->CreatureID);

			/* This MUST run immediately (and not be queued) as the script probably wants to clear out
			 * the queue entirely, which might need to re-activated by on_target_acquired.
			 */
			aiNut->RunFunction("on_target_lost", p, false);
		}

		if(newTarget != NULL)
		{
			if(HasStatus(StatusEffects::DEAD))
				return;

			g_Log.AddMessageFormat("%s is now targeting %s", css.display_name, newTarget->css.display_name);

			//If in a patrol, abort the movement so the ability system can take over.
			if(serverFlags & ServerFlags::PathNode)
			{
				SetServerFlag(ServerFlags::PathNode, false);
				Speed = 0;
				CurrentTarget.DesLocX = 0;
				CurrentTarget.DesLocZ = 0;
			}

			//If currently idle, set its last known idle location.
			if(CurrentTarget.targ == NULL)
			{
				lastIdleX = CurrentX;
				lastIdleZ = CurrentZ;

				//Hack to interrupt paths, otherwise the movement is kinda weird.
				if(serverFlags & ServerFlags::IsNPC)
				{
					if(CurrentTarget.DesLocX != 0 || CurrentTarget.DesLocZ != 0)
					{
						Speed = 0;
						StopMovement(ScriptCore::Result::INTERRUPTED);
					}
				}
			}
			UpdateAggroPlayers(1);
		}
		else
		{
			g_Log.AddMessageFormat("%s is now targeting nothing", css.display_name);
			UpdateAggroPlayers(0);
			CurrentTarget.Clear(false);
		}

		/*
		if(aiScript != NULL)
			aiScript->FullReset();
		*/

		if(!(serverFlags & ServerFlags::IsPlayer))
		{
			if(ab[0].bPending == true)
				CancelPending_Ex(&ab[0]);
		}


		CurrentTarget.targ = newTarget;

		if(aiNut != NULL && CurrentTarget.targ != NULL) {
			std::vector<ScriptCore::ScriptParam> p;
			p.push_back(CurrentTarget.targ->CreatureID);
			aiNut->JumpToLabel("on_target_acquired", p);
		}
	}
}

//Performs certain actions on ability failure.
void CreatureInstance :: AICheckAbilityFailure(int abilityReturnInfo)
{
	int errCode = g_AbilityManager.GetAbilityErrorCode(abilityReturnInfo);
	//Clear target list to avoid leftover pointers in the list. 
	ab[0].Clear("CreatureInstance :: AICheckAbilityFailure");
	switch(errCode)
	{
	case Ability2::ABILITY_NO_TARGET:  //For some reason swarm isn't being updated with a ground location
		if(CurrentTarget.targ != NULL)
		{
			if(ActiveInstance::GetPlaneRange(this, CurrentTarget.targ, MAX_ATTACK_RANGE) > MAX_ATTACK_RANGE)
				SelectTarget(NULL);
		}
		else
			SelectTarget(NULL);
		break;
	case Ability2::ABILITY_INRANGE:
		if(CurrentTarget.desiredRange == 0)
			movementTime = g_ServerTime; //Process the next movement cycle immediately.
		CurrentTarget.desiredRange = g_AbilityManager.GetAbilityErrorParameter(abilityReturnInfo);
		break;
	}
}

//Return true if we should retry an ability cast if it fails for a temporary reason that we can recover
//from.
bool CreatureInstance :: AIAbilityFailureAllowRetry(int abilityReturnInfo)
{
	int errCode = g_AbilityManager.GetAbilityErrorCode(abilityReturnInfo);
	switch(errCode)
	{
	case Ability2::ABILITY_COOLDOWN:  //We can wait for cooldowns to finish
	case Ability2::ABILITY_PENDING:   //.. for prior abilities to finish
	case Ability2::ABILITY_FACING:    //.. to face our target
	case Ability2::ABILITY_INRANGE:   //.. to get in range
	case Ability2::ABILITY_NOTSILENCED: //.. to recover from silence
	case Ability2::ABILITY_MIGHT:     //.. for might to recharge
	case Ability2::ABILITY_WILL:      //.. for will to recharge
	case Ability2::ABILITY_DISARM:    //.. to recover from this status
	case Ability2::ABILITY_STUN:      //.. to recover from this status
	case Ability2::ABILITY_DAZE:      //.. to recover from this status
		return true;
	}
	return false;
}

bool CreatureInstance :: AICheckIfAbilityBusy(void)
{
	return ab[0].bPending;
}

int CreatureInstance :: AIFillCreaturesNear(int range, float x, float z, int playerAbilityRestrict, int npcAbilityRestrict, int sidekickAbilityRestrict, CREATURE_PTR_SEARCH& creatures)
{
	for(size_t i = 0; i < actInst->PlayerListPtr.size(); i++)
	{
		if(actInst->PlayerListPtr[i]->CreatureID == CreatureID)
			continue;
		if(ActiveInstance::GetPointRangeXZ(actInst->PlayerListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], playerAbilityRestrict) == true)
			creatures.push_back(actInst->PlayerListPtr[i]);
	}
	for(size_t i = 0; i < actInst->NPCListPtr.size(); i++)
	{
		if(actInst->NPCListPtr[i]->CreatureID == CreatureID)
			continue;
		if(ActiveInstance::GetPointRangeXZ(actInst->NPCListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->NPCListPtr[i], npcAbilityRestrict) == true)
			creatures.push_back(actInst->NPCListPtr[i]);
	}
	for(size_t i = 0; i < actInst->SidekickListPtr.size(); i++)
	{
		if(actInst->SidekickListPtr[i]->CreatureID == CreatureID)
			continue;
		if(ActiveInstance::GetPointRangeXZ(actInst->SidekickListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], sidekickAbilityRestrict) == true)
			creatures.push_back(actInst->NPCListPtr[i]);
	}
	return creatures.size();
}

int CreatureInstance :: AICountEnemyNear(int range, float x, float z)
{
	int count = 0;
	for(size_t i = 0; i < actInst->PlayerListPtr.size(); i++)
	{
		if(ActiveInstance::GetPointRangeXZ(actInst->PlayerListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->PlayerListPtr[i], TargetStatus::Enemy_Alive) == true)
			count++;
	}
	for(size_t i = 0; i < actInst->NPCListPtr.size(); i++)
	{
		if(ActiveInstance::GetPointRangeXZ(actInst->NPCListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->NPCListPtr[i], TargetStatus::Enemy_Alive) == true)
			count++;
	}
	for(size_t i = 0; i < actInst->SidekickListPtr.size(); i++)
	{
		if(ActiveInstance::GetPointRangeXZ(actInst->SidekickListPtr[i], x, z, range) > range)
			continue;
		if(_ValidTargetFlag(actInst->SidekickListPtr[i], TargetStatus::Enemy_Alive) == true)
			count++;
	}
	return count;
}

int CreatureInstance :: AIGetIdleMob(int creatureDefID)
{
	for(size_t i = 0; i < actInst->NPCListPtr.size(); i++)
	{
		CreatureInstance *obj = actInst->NPCListPtr[i];
		if(obj->CreatureDefID != creatureDefID)
			continue;
		if(obj->Faction != Faction)
			continue;
		if(obj->HasStatus(StatusEffects::DEAD))
			continue;
		if(obj->CurrentTarget.targ != NULL)
			continue;
		if(obj->serverFlags & ServerFlags::LeashRecall)
			continue;
		if(obj->serverFlags & ServerFlags::Noncombatant)
			continue;

		return obj->CreatureID;
	}
	return 0;
}

void CreatureInstance :: AIOtherSetTarget(int creatureID, int creatureIDTarget)
{
	CreatureInstance *obj = actInst->GetNPCInstanceByCID(creatureID);
	CreatureInstance *target = actInst->GetInstanceByCID(creatureIDTarget);
	if(obj != NULL && target != NULL)
	{
		if(obj->serverFlags & ServerFlags::LeashRecall)
			return;
		if(target->serverFlags & ServerFlags::LeashRecall)
			return;

		obj->SelectTarget(target);
	}
}

void CreatureInstance :: AIOtherCallLabel(int creatureID, const char *aiScriptLabel)
{
	CreatureInstance *obj = actInst->GetNPCInstanceByCID(creatureID);
	if(obj != NULL)
	{
		if(obj->aiNut != NULL)
		{
			obj->aiNut->JumpToLabel(aiScriptLabel);
		}
		if(obj->aiScript != NULL)
		{
			obj->aiScript->JumpToLabel(aiScriptLabel);
		}
	}
}

bool CreatureInstance :: AIIsTargetEnemy(void)
{
	CreatureInstance *targ = CurrentTarget.targ;
	if(targ == NULL)
		return false;
	return _ValidTargetFlag(targ, TargetStatus::Enemy_Alive);
}

bool CreatureInstance :: AIIsTargetFriend(void)
{
	CreatureInstance *targ = CurrentTarget.targ;
	if(targ == NULL)
		return false;
	return _ValidTargetFlag(targ, TargetStatus::Friend_Alive);
}

float CreatureInstance :: AIGetProperty(const char *propName, bool useTarget)
{
	CreatureInstance *obj = this;
	if(useTarget == true)
		obj = CurrentTarget.targ;
	if(obj == NULL)
		return 0.0F;
	int r = GetStatIndexByName(propName);
	if(r == -1)
		return 0.0F;

	return GetStatValueByID(StatList[r].ID, &obj->css);
}

int CreatureInstance :: AIGetBuffTier(int abilityGroupID, bool useTarget)
{
	CreatureInstance *obj = this;
	if(useTarget == true)
		obj = CurrentTarget.targ;
	if(obj == NULL)
		return 0;

	for(size_t i = 0; i < obj->activeStatMod.size(); i++)
	{
		if(obj->activeStatMod[i].abilityGroupID == abilityGroupID)
		{
			return obj->activeStatMod[i].tier;
		}
	}
	return 0;
}

int CreatureInstance :: AIGetTargetRange(void)
{
	if(CurrentTarget.targ == NULL)
		return 0;
	return ActiveInstance::GetPlaneRange(this, CurrentTarget.targ, 10000);
}

void CreatureInstance :: AIDispelTargetProperty(const char *propName, int sign)
{
	CreatureInstance *targ = CurrentTarget.targ;
	if(targ == NULL)
		return;
	int r = GetStatIndexByName(propName);
	if(r == -1)
		return;
	targ->RemoveAbilityBuffWithStat(StatList[r].ID, static_cast<float>(sign));
}


void CreatureInstance :: AISetGTAETo(int x, int y, int z)
{
	if(CurrentTarget.targ != NULL)
	{
		x = CurrentTarget.targ->CurrentX;
		y = CurrentTarget.targ->CurrentY;
		z = CurrentTarget.targ->CurrentZ;
	}
	ab[0].SetPosition(x, y, z);
}


void CreatureInstance :: AISetGTAE(void)
{
	AISetGTAETo(CurrentX, CurrentY, CurrentZ);
}


void CreatureInstance :: SendEffect(const char *effectName, int targetCID)
{
	if(actInst != NULL)
	{
		char buffer[512];
		int wpos = PrepExt_SendEffect(buffer, CreatureID, effectName, targetCID);
		BroadcastLocal(buffer, wpos, -1);
	}
}

void CreatureInstance :: SendSay(const char *message)
{
	if(actInst != NULL)
	{
		char buffer[512];
		int wpos = PrepExt_GenericChatMessage(buffer, CreatureID, css.display_name, "s", message);
		BroadcastLocal(buffer, wpos, -1);
	}
}

void CreatureInstance :: SendPlaySound(const char *assetPackage, const char *soundFile)
{
	char buffer[512];

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 100);       //_handleInfoMsg
	wpos += PutShort(&buffer[wpos], 0);      //Placeholder for size

	wpos += PutByte(&buffer[wpos], 4); //Event
	wpos += PutStringUTF(&buffer[wpos], assetPackage);
	wpos += PutStringUTF(&buffer[wpos], soundFile);

	PutShort(&buffer[1], wpos - 3);     //Set size
	BroadcastLocal(buffer, wpos, -1);
}

int CreatureInstance :: _HasStatusList(int statusID)
{
	for(size_t i = 0; i < activeStatusEffect.size(); i++)
		if(activeStatusEffect[i].modStatusID == statusID)
			return i;
	return -1;
}

void CreatureInstance :: _AddStatusList(int statusID, long msDuration)
{
	// WARNING: Because this creature updates the pending list,
	// status effects must be called after a creature has been
	// successfully added into the instance.
	if(statusID == StatusEffects::STUN)
		_OnStun();
	if(statusID == StatusEffects::DAZE)
		OnDaze();

	unsigned long endTime = msDuration;
	if(msDuration == -1)
		endTime = PlatformTime::MAX_TIME;
	else
		endTime += g_ServerTime;

	int r = _HasStatusList(statusID);
	if(r >= 0)
	{
		activeStatusEffect[r].startTime = g_ServerTime;
		activeStatusEffect[r].expireTime = endTime;
	}
	else
	{
		activeStatusEffect.push_back(ActiveStatusEffect(statusID, g_ServerTime, endTime));
		_SetStatusFlag(statusID);
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_MOD, this, 0);
	}
}

void CreatureInstance :: _RemoveStatusList(int statusID)
{
	int r = _HasStatusList(statusID);
	if(r >= 0)
	{
		activeStatusEffect.erase(activeStatusEffect.begin() + r);
		_ClearStatusFlag(statusID);
		pendingOperations.UpdateList_Add(CREATURE_UPDATE_MOD, this, 0);
		if(statusID == StatusEffects::TRANSFORMED)
			CAF_Untransform();
	}
}

int CreatureInstance :: GetStatDurationSec(int index)
{
	if(activeStatMod[index].expireTime == PlatformTime::MAX_TIME)
		return std::numeric_limits<int>::max();
	else {
		int x = ((activeStatMod[index].expireTime - g_ServerTime ) / 1000UL);
		if(x < 0) {
			return std::numeric_limits<int>::max();
		}
		return x;
	}
}

void CreatureInstance :: CheckActiveStatusEffects(void)
{
	//Check status effects for time expirations.

	bool bUpdate = false;

	size_t i = 0;
	while(i < activeStatusEffect.size())
	{
		if(g_ServerTime >= activeStatusEffect[i].expireTime)
		{
			_ClearStatusFlag(activeStatusEffect[i].modStatusID);
			activeStatusEffect.erase(activeStatusEffect.begin() + i);
			bUpdate = true;
		}
		else
		{
			i++;
		}
	}

	if(bUpdate == true)
	{
		SendFlags();
		bUpdate = false;
	}

	i = 0;
	while(i < activeStatMod.size())
	{
		if(g_ServerTime >= activeStatMod[i].expireTime)
		{
			RemoveBuffIndex(i);
			// Before it called to remove all buffs with this ID, then separated ability
			//   and normal buff removal.  Realized this wasn't necessary and just changed
			//  to remove this particular index.
			/*
			if(activeStatMod[i].sourceType == BuffSource::ABILITY)
				RemoveBuffsFromAbility(activeStatMod[i].abilityID, false);
			else
				RemoveBuffIndex(i);
			*/
			bUpdate = true;
		}
		else
		{
			i++;
		}
	}

	if(bUpdate == true)
		SendUpdatedBuffs();

	/*
	do
	{
		bActive = false;
		for(i = 0; i < activeStatusEffect.size(); i++)
		{
			if(g_ServerTime >= activeStatusEffect[i].expireTime)
			{
				_ClearStatusFlag(activeStatusEffect[i].modStatusID);
				activeStatusEffect.erase(activeStatusEffect.begin() + i);
				bActive = true;
				bUpdate = true;
				break;
			}
		}
	} while(bActive == true);

	if(bUpdate == true)
	{
		SendFlags();
		bUpdate = false;
	}

	bool conChanged = false;
	bActive = true;
	do
	{
		bActive = false;
		for(i = 0; i < activeStatMod.size(); i++)
		{
			if(g_ServerTime >= activeStatMod[i].expireTime)
			{
				if(activeStatMod[a].modStatID == STAT::CONSTITUTION)
					conChanged = true;
				BuffRemove(activeStatMod[i].abilityID, false);
				bActive = true;
				bUpdate = true;
				break;
			}
		}
	} while(bActive == true);

	if(bUpdate == true)
		SendUpdatedBuffs();
	*/
}

int CreatureInstance :: GetOrbRegenerationTime(float regenFactor)
{
	//Regeneration rates:
	//  if < 1.0, the speed is reduced and needs a longer delay.
	//  if > 1.0, the speed is increased and needs a shorter delay.
	//  Example: the skill Druid:Shadow Spirit:Lev30 says in the description that it reduces
	//  regeneration rate by 50%.  The ability function is Amp(STAT::MIGHT_REGEN, -0.5)
	//  Normal regeneration rate is 2000 milliseconds.
	//  If the formula were to be: delay = 2000 / regen
	//    then a regen of 0.5 leads to a 4000ms delay, which is NOT accurate.
	//    and a regen of 2 leads to a 1000ms delay, which is accurate
	// So we calculate differently whether above or below 1.0.

	if(regenFactor < 1.0F)
		return (4000 - (int)(2000.0F * regenFactor));

	return (int)(2000.0F / regenFactor);
}

void CreatureInstance :: CheckUpdateTimers(void)
{
	//Don't allow regeneration for players if they're in noncombat stage.
	//In theory this could allow players to regen to full while loading,
	//then log out before they die, to repeat the process.
	if(serverFlags & ServerFlags::IsPlayer)
		if(serverFlags & ServerFlags::Noncombatant)
			return;

	int orbUpdate = 0;
	if(css.might < ABGlobals::MAX_MIGHT)
	{
		if(g_ServerTime >= timer_mightregen)
		{

			/*
			if(css.might_regen > 0.0F)
			{
				orbUpdate |= AdjustMight(1);
				if(css.might_regen < 1.0F)
					timer_mightregen = 4000 - (int)(2000.0F * css.might_regen);
				else
					timer_mightregen = (int)(2000.0F / css.might_regen);
			}
			else
				timer_mightregen = 2000;
			*/

			if(css.might_regen > 0.0F)
			{
				orbUpdate |= AdjustMight(1);
				timer_mightregen = GetOrbRegenerationTime(css.might_regen);
			}
			else
				timer_mightregen = 2000;

			//g_Log.AddMessageFormat("timer_mightregen: %d", timer_mightregen);
			timer_mightregen += g_ServerTime;
			/*
			float regen = css.might_regen;
			if(regen < 0.01F)
			if(regen < 0.01F)
				regen = 0.01F;
			timer_mightregen = (int)(2000.0F / regen);
			timer_mightregen += g_ServerTime;
			*/
		}
	}
	if(css.will < ABGlobals::MAX_WILL)
	{
		if(g_ServerTime >= timer_willregen)
		{
			/*
			if(css.will_regen > 0.0F)
			{
				orbUpdate |= AdjustWill(1);
				if(css.will_regen < 1.0F)
					timer_willregen = 4000 - (int)(2000.0F * css.will_regen);
				else
					timer_willregen = (int)(2000.0F / css.will_regen);
			}
			else
				timer_willregen = 2000;
			*/
			if(css.will_regen > 0.0F)
			{
				orbUpdate |= AdjustWill(1);
				timer_willregen = GetOrbRegenerationTime(css.will_regen);
			}
			else
				timer_willregen = 2000;
			//g_Log.AddMessageFormat("timer_willregen: %d", timer_willregen);
			timer_willregen += g_ServerTime;

			/*
			float regen = css.will_regen;
			if(regen < 0.01F)
			if(regen < 0.01F)
				regen = 0.01F;
			timer_willregen = (int)(2000.0F / regen);
			timer_willregen += g_ServerTime;
			*/
		}
	}

	ProcessAutoAttack();

	if(orbUpdate != 0)
	{
		int wpos = 0;
		wpos = PrepExt_UpdateOrbs(GSendBuf, this);
		//actInst->LSendToAllSimulator(GSendBuf, wpos, -1);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
	}
}

void CreatureInstance :: CheckLootTimers(void)
{
	if(serverFlags & ServerFlags::DeadTransform)
		return;

	if(g_ServerTime < deathTime + 5000)
		return;

	int equipQuality = -1;   //Best quality of armor/weapons
	int otherQuality = -1;   //Best quality of all other items.
	int equipType = 0;       //mType of best armor/weapon.
	int otherType = 0;       //mType of best other item.
	if(activeLootID != 0)
	{
		int r = actInst->lootsys.GetCreature(CreatureID);
		if(r == -1)
			return;

		for(size_t i = 0; i < actInst->lootsys.creatureList[r].itemList.size(); i++)
		{
			int itemID = actInst->lootsys.creatureList[r].itemList[i];
			ItemDef *itemPtr = g_ItemManager.GetPointerByID(itemID);
			if(itemPtr != NULL)
			{
				if(itemPtr->mType == ItemType::ARMOR || itemPtr->mType == ItemType::WEAPON)
				{
					if(itemPtr->mQualityLevel > equipQuality)
					{
						equipQuality = itemPtr->mQualityLevel;
						equipType = itemPtr->mType;
					}
				}
				else
				{
					if(itemPtr->mQualityLevel > otherQuality)
					{
						otherQuality = itemPtr->mQualityLevel;
						otherType = itemPtr->mType;
					}
				}

				/*
				//Tokens are rolled first, and are typically set to green/uncommon rarity.
				//Use >= so that equipment will override the token when determining
				//a chest appearance.
				if(itemPtr->mQualityLevel >= highestQuality)
				{
					highestQuality = itemPtr->mQualityLevel;
					
					//Equipment and armor have lower types than crafting mats and plans, so prioritize them.
					if(type == 0 || itemPtr->mType < type)
					{
						type = itemPtr->mType;
					}
				}
				*/
			}
		}
	}

	//Choose the type and quality based on the best equipment if found, otherwise default to non equipment.
	int chosenQuality = ((equipQuality >= 0) ? equipQuality : otherQuality);
	int chosenType    = ((equipQuality >= 0) ? equipType : otherType);
	const char *prop = LootSystem::tombstone;
	float size = 1.0F;
	if(chosenQuality >= 0)
	{
		const LootSystem::LootProp *lootBag;
		lootBag = LootSystem::getLootBag(chosenType, chosenQuality);
		prop = lootBag->propName;
		size = lootBag->size;
	}

	std::string lootStr;
	LootSystem::BuildAppearanceOverride(lootStr, prop, size, LootSystem::tombstone);

	static const short appStat[2] = {STAT::APPEARANCE_OVERRIDE, STAT::LOOT_SEEABLE_PLAYER_IDS};
	if(activeLootID == 0)
		css.ClearLootSeeablePlayerIDs();

	css.appearance_override = lootStr;
	int wpos = PrepExt_SendSpecificStats(GSendBuf, this, &appStat[0], 2);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);

	SetServerFlag(ServerFlags::DeadTransform, true);
}

int CreatureInstance :: GetKillExperience(int attackerLevel)
{
	int levelDiff = css.level - attackerLevel;
	if(levelDiff > 5)
		levelDiff = 5;
	else if(levelDiff < -5)
		levelDiff = -5;

	int bonus = 0;
	if(levelDiff > 0)
		bonus = 10 * levelDiff;
	else if(levelDiff < 0)
		bonus = 20 * levelDiff;  //levelDiff is negative

	float rMult = 1.0;
	switch(css.rarity)
	{
	case CreatureRarityType::NORMAL:
		rMult = 1.0;
		break;
	case CreatureRarityType::HEROIC:
		rMult = 2.0;
		break;
	case CreatureRarityType::EPIC:
		rMult = 4.0;
		break;
	case CreatureRarityType::LEGEND:
		rMult = 8.0;
		break;
	default:
		rMult = 1.0;
	};

	int adjusted = 100 + bonus;

	if(css.experience_gain_rate > 0)
		adjusted += Util::GetAdditiveFromIntegralPercent100(adjusted, css.experience_gain_rate);
	if(adjusted < 1)
		adjusted = 1;
	adjusted = (int)((float)adjusted * rMult);
	return adjusted;
}
void CreatureInstance :: AddValour(int GuildDefID, int amount)
{
	if(amount == 0) {
		return;
	}

	if(actInst == NULL || charPtr == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] AddValour() active instance or charPtr is NULL.");
		return;
	}

	GuildDefinition *guildDef = g_GuildManager.GetGuildDefinition(GuildDefID);
	GuildRankObject *currentRank = g_GuildManager.GetRank(CreatureDefID, GuildDefID);
	g_CharacterManager.GetThread("CharacterData::AddValour");
	charPtr->AddValour(GuildDefID, amount);
	g_CharacterManager.ReleaseThread();
	GuildRankObject *newRank = g_GuildManager.GetRank(CreatureDefID, GuildDefID);
	if(newRank == NULL) {
		g_Log.AddMessageFormat("[ERROR] Huh, no new rank in guild, joined %d to %d", CreatureDefID, GuildDefID);
		return;
	}
	if(currentRank == NULL || currentRank->valour != newRank->valour)
	{
		char buffer[128];
		Util::SafeFormat(buffer, sizeof(buffer), "You have gained %d valour in %s", amount,guildDef->defName);
		simulatorPtr->SendInfoMessage(buffer, INFOMSG_INFO);
	}
	if(currentRank == NULL || currentRank->rank != newRank->rank)
	{
		charPtr->OnRankChange(newRank->rank);

		simulatorPtr->BroadcastGuildChange(GuildDefID);

		char buffer[128];
		Util::SafeFormat(buffer, sizeof(buffer), "You have been promoted to %s in %s", newRank->title.c_str(),guildDef->defName);
		simulatorPtr->SendInfoMessage(buffer, INFOMSG_INFO);

		char Buffer[1024];
		int size = PrepExt_SendEffect(Buffer, CreatureID, "GuildDing", 0);
		actInst->LSendToAllSimulator(Buffer, size, -1);
	}

	// TODO need to actually inform client somehow to update guild tab, maybe animations?

}

void CreatureInstance :: AddCredits(int amount)
{
	if(actInst == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] AddCredits() active instance is NULL.");
		return;
	}

	if(g_Config.AccountCredits && simulatorPtr != NULL) {
		css.credits = simulatorPtr->pld.accPtr->Credits;
	}

	css.credits += amount;

	if(g_Config.AccountCredits && simulatorPtr != NULL) {
		simulatorPtr->pld.accPtr->Credits = css.credits;
		simulatorPtr->pld.accPtr->PendingMinorUpdates++;
	}

	if(serverFlags & ServerFlags::IsPlayer)
		SendStatUpdate(STAT::CREDITS);
}

void CreatureInstance :: AddExperience(int amount)
{
	if(actInst == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] AddExperience() active instance is NULL.");
		return;
	}
	if(css.level >= g_Config.CapExperienceLevel)
		if(amount > g_Config.CapExperienceAmount)
			amount = g_Config.CapExperienceAmount;

	css.experience += amount;

	int curLevel = css.level;
	int newLevel = curLevel;
	if(curLevel < 0)
		curLevel = 0;
	else if(curLevel > MAX_LEVEL)
		curLevel = MAX_LEVEL;

	while(css.experience >= LevelRequirements[newLevel][1])
	{
		newLevel++;
		if(newLevel >= MAX_LEVEL)
		{
			newLevel = MAX_LEVEL;
			break;
		}
	}

	if(serverFlags & ServerFlags::IsPlayer)
	{
		int wpos = PrepExt_ExperienceGain(GSendBuf, CreatureID, amount);
		wpos += PrepExt_SendExperience(&GSendBuf[wpos], CreatureID, css.experience);
		actInst->LSendToOneSimulator(GSendBuf, wpos, simulatorPtr);
		if(newLevel != curLevel)
			SetLevel(newLevel);
		RemoveNoncombatantStatus("AddExperience");
	}
}

void CreatureInstance :: RemoveNoncombatantStatus(const char *debugCaller)
{
	if(serverFlags & ServerFlags::Noncombatant)
	{
		SetServerFlag(ServerFlags::Noncombatant, false);
		g_Log.AddMessageFormat("Noncombat status removed (%s)", debugCaller);
	}
}

void CreatureInstance :: AddHeroism(int amount)
{
	if(amount < 0)
		return;

	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	int newHeroism = Util::ClipInt(css.heroism + amount, 0, Global::MAX_HEROISM);
	
	//If we were at max heroism before, it will not have changed, so don't resend.
	if(newHeroism != css.heroism)
	{
		css.heroism = newHeroism;
		OnHeroismChange();
	}
}

void CreatureInstance :: AddHeroismForQuest(int amount, int questLevel)
{
	int threshold = questLevel + g_Config.HeroismQuestLevelTolerance;
	if(css.level > threshold)
	{
		amount -= ((css.level - threshold) * g_Config.HeroismQuestLevelPenalty);
	}

	// Adjust for other bonuses
	if(css.heroism_gain_rate > 0) {
		amount += Util::GetAdditiveFromIntegralPercent100(amount, css.heroism_gain_rate);
	}


	AddHeroism(amount);
}

void CreatureInstance :: AddHeroismForKill(int targetLevel, int targetRarity)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	int levelDiff = targetLevel - css.level;
	if(levelDiff > 5)
		levelDiff = 5;

	//1 point of heroism = 0.5 points of luck
	//The base amount should give 1 point even if the mob is one level below the player
	//(level difference of -1)
	//Additionally we'll provide a bonus based on rarity.
	int points = 2 + levelDiff;
	points += targetRarity; 

	if(points < 0)
		return;

	// Adjust for other bonuses
	if(css.heroism_gain_rate > 0) {
		points += Util::GetAdditiveFromIntegralPercent100(points, css.heroism_gain_rate);
	}

	AddHeroism(points);

	/* OLD
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;
	int levelDiff = creatureKilledLevel - css.level;
	if(levelDiff < -1)
		return;

	//If the mob is 1 level below the player (-1 difference), it needs to map to 1 heroism (0.5 luck gained)
	levelDiff += 2;

	levelDiff = Util::ClipInt(levelDiff, 1, 5);
	int newHeroism = Util::ClipInt(css.heroism + levelDiff, 0, Global::MAX_HEROISM);
	if(newHeroism != css.heroism)
	{
		css.heroism = newHeroism;
		OnHeroismChange();
	}
	*/
}

void CreatureInstance :: OnHeroismChange(void)
{
	//In short, max heroism is 1000, max base luck is 500.
	//Base Luck needs to be half of heroism.  Mod Luck holds additional modifiers, like skill bonuses.
	css.heroism = Util::ClipInt(css.heroism, 0, Global::MAX_HEROISM);
	css.base_luck = Util::ClipInt(css.heroism / 2, 0, Global::MAX_BASE_LUCK);

	int modPercent = 0;
	if(css.heroism < 250)
		modPercent = 0;
	else if(css.heroism < 500)
		modPercent = 20;
	else if(css.heroism < 750)
		modPercent = 40;
	else if(css.heroism < 1000)
		modPercent = 60;
	else
		modPercent = 100;

	css.health_mod = Util::GetAdditiveFromIntegralPercent1000(GetMaxHealth(false), modPercent);

	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::HEROISM);
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::BASE_LUCK);
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, STAT::HEALTH_MOD);
}

void CreatureInstance :: SendStatUpdate(int statID)
{
	pendingOperations.UpdateList_Add(CREATURE_UPDATE_STAT, this, statID);
}

void CreatureInstance :: CheckQuestKill(CreatureInstance *target)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	int wpos = charPtr->questJournal.CheckQuestObjective(CreatureID, GSendBuf, QuestObjective::OBJECTIVE_TYPE_KILL, target->CreatureDefID);
	if(wpos > 0)
	{
		//TODO: Simulator revamp
		if(simulatorPtr != NULL)
			simulatorPtr->AttemptSend(GSendBuf, wpos);
	}
}

int CreatureInstance :: ProcessQuestRewards(int QuestID, const std::vector<QuestItemReward>& itemsToGive)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
	{
		g_Log.AddMessageFormat("[ERROR] ProcessQuestRewards() must be a player to receive inventory objects", QuestID);
		return -1;
	}

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qd == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] ProcessQuestRewards() invalid quest ID [%d]", QuestID);
		return -1;
	}

	//TODO: Simulator revamp
	if(simulatorPtr == NULL)
		return -1;

	AddValour(qd->guildId, qd->valourGiven);

	// Adjust Exp taking tomes into account
	int exp = qd->experience;

	// Adjust for other bonuses
	if(css.quest_exp_gain_rate > 0) {
		exp += Util::GetAdditiveFromIntegralPercent100(exp, css.quest_exp_gain_rate);
	}

	AddExperience(exp);
	AddHeroismForQuest(qd->heroism, qd->levelSuggested);
	css.copper += qd->coin;
	short stat = STAT::COPPER;

	int wpos = PrepExt_SendSpecificStats(GSendBuf, this, &stat, 1);
	simulatorPtr->AttemptSend(GSendBuf, wpos);

	wpos = 0;  //From here, we may need to write multiple messages in the buffer.
	for(size_t i = 0; i < itemsToGive.size(); i++)
	{
		int itemID = itemsToGive[i].itemID;
		int count = itemsToGive[i].itemCount;

		ItemDef *itemPtr = g_ItemManager.GetPointerByID(itemID);
		if(itemPtr == NULL)
		{
			wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "Reward item not found in database.", INFOMSG_ERROR);
			simulatorPtr->AttemptSend(GSendBuf, wpos);
			g_Log.AddMessageFormat("[ERROR] ProcessQuestRewards() Reward item [%d] not found in database for quest [%d]", itemID, QuestID);
			return -1;
		}

		int slot = charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot == -1)
		{
			wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "No free inventory space.", INFOMSG_ERROR);
			simulatorPtr->AttemptSend(GSendBuf, wpos);
			return -2;
		}

		InventorySlot *newItem = NULL;
		if(itemPtr->mType == ItemType::SPECIAL && itemPtr->mIvType1 == ItemIntegerType::QUEST_ID) {
			/* Random quest rewards, so we roll for a different item */
			VirtualItemSpawnParams params;
			params.level = qd->levelSuggested;
			params.rarity = itemPtr->mQualityLevel;
			params.namedMob = false;
			params.minimumQuality = itemPtr->mQualityLevel;
			//params.dropRateProfile = g_DropRateProfileManager.GetProfileByName("");
			params.ClampLimits();

			int iid = g_ItemManager.RollVirtualItem(params);
			if(iid != -1) {
				newItem = charPtr->inventory.AddItem_Ex(INV_CONTAINER, iid, count);
				if(newItem != NULL) {
					char buffer[256];
					Util::SafeFormat(buffer, sizeof(buffer), "Your mystery reward is .. %s", newItem->dataPtr->mDisplayName.c_str());
					wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], buffer, INFOMSG_INFO);
				}
			}
		}
		else {
			newItem = charPtr->inventory.AddItem_Ex(INV_CONTAINER, itemPtr->mID, count);
		}
		if(newItem != NULL) {
			simulatorPtr->ActivateActionAbilities(newItem);
			wpos += AddItemUpdate(&GSendBuf[wpos], GAuxBuf, newItem);
		}
		else
		{
			wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "No item.", INFOMSG_ERROR);
			simulatorPtr->AttemptSend(GSendBuf, wpos);
			return -2;
		}
	}

	if(wpos > 0)
		simulatorPtr->AttemptSend(GSendBuf, wpos);

	return 1;
}

int CreatureInstance :: QuestInteractObject(char *buffer, const char *text, int time, bool gather)
{
	Interrupt();
	return PrepInteractObject(buffer, text, time, gather, CurrentTarget.targ);
}

int CreatureInstance :: PrepInteractObject(char *buffer, const char *text, int time, bool gather, CreatureInstance *targ)
{
	if(g_Config.DebugVerbose)
		g_Log.AddMessageFormat("PrepInteractObject %s : %d : %s", text, time, gather ? "gather: " : "activate");

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0);  //size
	wpos += PutInteger(&buffer[wpos], CreatureID);
	wpos += PutByte(&buffer[wpos], 11);  //creature "used" event
	wpos += PutStringUTF(&buffer[wpos], text);
	wpos += PutInteger(&buffer[wpos], time);
	PutShort(&buffer[1], wpos - 3);  //size

	//We can't call RegisterCast, but the client doesn't have a corresponding
	//ability effect.  The server needs to know this information

	//g_Log.AddMessageFormat("[DEBUG] Adding ability event to trigger in: %d ms", objectiveData->ActivateTime);
	if(gather)
		ab[0].abilityID = ABILITYID_QUEST_GATHER_OBJECT;
	else
		ab[0].abilityID = ABILITYID_QUEST_INTERACT_OBJECT;

	ab[0].type = AbilityType::Cast;
	ab[0].fireTime = g_ServerTime + time;
	ab[0].mightChargesSpent = 0;
	ab[0].willChargesSpent = 0;
	ab[0].bPending = true;
	ab[0].bSecondary = false;
	ab[0].bUnbreakableChannel = false;
	ab[0].TargetCount = 1;
	ab[0].TargetList[0] = targ;

	return wpos;
}

int CreatureInstance :: NormalInteractObject(char *outBuf, InteractObject *interactObj)
{
	if(CurrentTarget.targ == NULL)
	{
		g_Log.AddMessageFormat("[WARNING] NormalInteractObject target is NULL");
		return 0;
	}

	Interrupt();

	int wpos = 0;
	wpos += PutByte(&outBuf[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&outBuf[wpos], 0);  //size
	wpos += PutInteger(&outBuf[wpos], CreatureID);
	wpos += PutByte(&outBuf[wpos], 11);  //creature "used" event
	wpos += PutStringUTF(&outBuf[wpos], interactObj->useMessage);
	wpos += PutInteger(&outBuf[wpos], interactObj->useTime);
	PutShort(&outBuf[1], wpos - 3);  //size

	//We can't call RegisterCast, but the client doesn't have a corresponding
	//ability effect.  The server needs to know this information

	//g_Log.AddMessageFormat("[DEBUG] Adding ability event to trigger in: %d ms", objectiveData->ActivateTime);
	ab[0].abilityID = ABILITYID_INTERACT_OBJECT;
	ab[0].type = AbilityType::Cast;
	ab[0].fireTime = g_ServerTime + interactObj->useTime;
	ab[0].mightChargesSpent = 0;
	ab[0].willChargesSpent = 0;
	ab[0].bPending = true;
	ab[0].bSecondary = false;
	ab[0].bUnbreakableChannel = false;
	ab[0].TargetCount = 1;
	ab[0].TargetList[0] = CurrentTarget.targ;

	return wpos;
}

void CreatureInstance :: CheckQuestInteract(CreatureInstance *target)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	if(charPtr == NULL)
		return;
	if(simulatorPtr == NULL)
		return;


	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(target->CreatureDefID);
	/* Is the object an ITEM_GIVER? (Item ids given are in Extra Data) */
	if(cdef != NULL && (cdef->DefHints & CDEF_HINT_ITEM_GIVER) != 0) {
		/* For now only allow use if the player doesn't have any of the items pointed to by
		 * the creatures ExtraData. In this case, pick the first item they don't have and add it their
		 * inventory
		 */

		for(std::vector<int>::iterator it = cdef->Items.begin(); it != cdef->Items.end(); ++it) {
			/* For now we only allow use if the player doesn't already have
			 * the item. There could be other uses for this though. I'll
			 * add logic as and when it's needed
			 */
			int id = (*it);
			if(charPtr->inventory.GetItemPtrByID(id) == NULL) {

				ItemDef *item = g_ItemManager.GetSafePointerByID(id);
				if(item->mID == 0) {
					return;
				}
				else {
					int slot = charPtr->inventory.GetFreeSlot(INV_CONTAINER);
					char buf[512];
					char buf2[256];
					if(slot == -1) {
						Util::SafeFormat(buf2, sizeof(buf2), "Cannot take book, your inventory is full.", item->mDisplayName.c_str());
						int wpos = PrepExt_SendInfoMessage(buf, buf2, INFOMSG_INFO);
						simulatorPtr->AttemptSend(buf, wpos);
						return;
					}
					else {
						InventorySlot *sendSlot = charPtr->inventory.AddItem_Ex(INV_CONTAINER, item->mID, 1);
						if(sendSlot != NULL) {
							simulatorPtr->ActivateActionAbilities(sendSlot);
							int wpos = AddItemUpdate(buf, buf2, sendSlot);
							Util::SafeFormat(buf2, sizeof(buf2), "You now have '%s' in your inventory.", item->mDisplayName.c_str());
							wpos += PrepExt_SendInfoMessage(&buf[wpos], buf2, INFOMSG_INFO);
							simulatorPtr->AttemptSend(buf, wpos);
							target->_RemoveStatusList(StatusEffects::IS_USABLE);
							static const short statList[3] = {STAT::APPEARANCE_OVERRIDE, STAT::LOOTABLE_PLAYER_IDS, STAT::LOOT_SEEABLE_PLAYER_IDS};
							actInst->LSendToLocalSimulator(GSendBuf, PrepExt_SendSpecificStats(GSendBuf, target, &statList[0], 3), target->CurrentX, target->CurrentZ);

						}
					}
				}

				break;
			}
		}

	}

	int wpos = charPtr->questJournal.CreatureUse_Confirmed(target->CreatureID, GSendBuf, target->CreatureDefID);
	if(wpos > 0)
	{
		simulatorPtr->AttemptSend(GSendBuf, wpos);
		QuestScript::QuestScriptPlayer *script = actInst->GetSimulatorQuestScript(simulatorPtr);
		if(script != NULL)
			script->TriggerFinished();

		QuestScript::QuestNutPlayer *nut = actInst->GetSimulatorQuestNutScript(simulatorPtr);
		if(nut != NULL) {
			char buffer[64];
			Util::SafeFormat(buffer, sizeof(buffer), "on_use_finish_%d", target->CreatureDefID);
			nut->JumpToLabel(buffer);
		}

		//actInst->ScriptCallUseFinish(LastUseDefID);
	}
}

void CreatureInstance :: RunQuestObjectInteraction(CreatureInstance *target, bool deleteObject)
{
	if(target == NULL)
		return;
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	if(simulatorPtr == NULL)
		return;

	int instance = actInst->mInstanceID;

	CheckQuestInteract(target);

	/* Determine if target creature is Warp interact as well as a Quest object interaction.
	 * If it is, we don't activate for the rest of the party, they must do it themselves
	 */
	InteractObject *ob = target->actInst == NULL ? NULL : g_InteractObjectContainer.GetObjectByID(target->CreatureDefID, target->actInst->mZone);
	if(ob == NULL || ob->opType != InteractObject::TYPE_WARP) {
		/*
		 * Either no interact object, or there is and it isn't a warp, so activate for the party too
		 */
		ActiveParty *party = g_PartyManager.GetPartyByID(PartyID);
		if(party != NULL) {
			for(size_t i = 0; i < party->mMemberList.size(); i++)
			{
				int CDefID = party->mMemberList[i].mCreatureDefID;
				if(CDefID != CreatureDefID) {
					CreatureInstance *member = actInst->GetPlayerByCDefID(CDefID);
					if(member == NULL)
						continue;
					if(member->actInst == NULL || member->actInst->mInstanceID != instance)
						continue;
					if(actInst->GetPlaneRange(this, member, PARTY_SHARE_DISTANCE) >= PARTY_SHARE_DISTANCE)
						continue;
					member->CheckQuestInteract(target);
				}
			}
		}
	}

	if(deleteObject == true) {
		target->actInst->RemoveNPCInstance(target->CreatureID);
	}
}

void CreatureInstance :: RunObjectInteraction(CreatureInstance *target)
{
	if(target == NULL)
		return;
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	//We need to add this to the message queue, because the instance changing code will
	//remove (and invalidate) this creature instance before the ability processing
	//is complete, potentially causing a crash.
	bcm.AddEvent2(simulatorPtr->InternalID, (long)simulatorPtr, target->CreatureDefID, BCM_RunObjectInteraction, actInst);
}

void CreatureInstance :: SendUpdatedLoot(void)
{
	static const short statList[3] =
	{
		STAT::APPEARANCE_OVERRIDE,
		STAT::LOOTABLE_PLAYER_IDS,
		STAT::LOOT_SEEABLE_PLAYER_IDS,
	};

	if(activeLootID != 0)
	{
		ActiveLootContainer &lootCon = actInst->lootsys.creatureList[activeLootID];
		char convBuf[32];
		int wpos = 0;

		css.ClearLootablePlayerIDs();
		css.ClearLootSeeablePlayerIDs();

		for(size_t i = 0; i < lootCon.lootableID.size(); i++)
		{
			Util::SafeFormat(convBuf, sizeof(convBuf), "%d", lootCon.lootableID[i]);
			if(i > 0)
			{
				css.loot_seeable_player_ids.append(",");
				css.lootable_player_ids.append(",");
			}
			css.loot_seeable_player_ids.append(convBuf);
			css.lootable_player_ids.append(convBuf);
		}

		css.aggro_players = 0;
		css.appearance_override = "0";  //TODO: obsolete?
		wpos = PrepExt_SendSpecificStats(GSendBuf, this, &statList[0], 3);
		actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);

		_AddStatusList(StatusEffects::IS_USABLE, -1);
	}
}

float CreatureInstance :: GetDropRateMultiplier(CreatureDefinition *cdef)
{
	if(cdef == NULL)
		cdef = CreatureDef.GetPointerByCDef(CreatureDefID);

	float dropRateBonus = 1.0F;
	if(cdef != NULL)
	{
		if(cdef->NamedMob)
		{
			if(g_Config.NamedMobDropMultiplier > 0.0F)
				dropRateBonus *= g_Config.NamedMobDropMultiplier;
		}

		float extra = cdef->DropRateMult;
		if(Util::FloatEquivalent(extra, 0.0F) == false)
			dropRateBonus *= extra;
	}


	if(css.drop_gain_rate > 0) {
		dropRateBonus += Util::GetAdditiveFromIntegralPercent100(dropRateBonus, css.drop_gain_rate);
	}


	switch(css.rarity)
	{
	case CreatureRarityType::HEROIC: dropRateBonus *= 2.5F; break;
	case CreatureRarityType::EPIC: dropRateBonus *= 5.0F; break;
	case CreatureRarityType::LEGEND: dropRateBonus *= 10.0F; break;
	}

	if(css.magic_loot_drop_rate > 0)
	{
		float extra = (css.magic_loot_drop_rate + 100) / 100.0F;
		dropRateBonus *= extra;
	}

	if(actInst != NULL)
	{
		if(actInst->scaleProfile != NULL)
		{
			float mult = actInst->scaleProfile->mDropMult;
			if(mult > 0.0F)
				dropRateBonus *= mult;
		}
		if(actInst->mDropRateBonusMultiplier > 0.0F)
		{
			dropRateBonus *= actInst->mDropRateBonusMultiplier;
		}
	}

	dropRateBonus = Util::ClipFloat(dropRateBonus, 0.0F, g_Config.DropRateBonusMultMax);


	//Debug logging, not necessary.
	if(dropRateBonus > 2.0F)
		g_Log.AddMessageFormat("[LOOT] DropRateMult [%s] %d,%d: %g", css.display_name, css.level, css.rarity, dropRateBonus);


	return dropRateBonus;
}

void CreatureInstance :: PlayerLoot(int level, std::vector<DailyProfile> profiles)
{
	//Don't fetch a new loot container if it already has one.
	if(activeLootID != 0)
		return;

	// *** Basic conditional checks passed, generate the loot. ***

	ActiveLootContainer loot;
	loot.CreatureID = CreatureID;

	for(std::vector<DailyProfile>::iterator it = profiles.begin(); it != profiles.end(); ++it) {
		DailyProfile profile = *it;
		switch(profile.rewardType) {
		case RewardType::VIRTUAL:
		{


//			float dropRateBonus = 1;
//
//			//Virtual items.
//			VirtualItemSpawnParams params;
//			if(Util::FloatEquivalent(dropRateBonus, 1.0F) == false)
//				params.SetAllDropMult(dropRateBonus);

			VirtualItemSpawnParams params;
			params.level = level;
			params.rarity = profile.virtualItemReward.minItemRarity;
			params.namedMob = false;
			params.minimumQuality = profile.virtualItemReward.minItemRarity;

			if(profile.virtualItemReward.components.size() > 0) {
				VirtualItemRewardComponent ei = profile.virtualItemReward.components[randmod(profile.virtualItemReward.components.size())];
				params.mEquipType = ei.equipType;
				if(ei.weaponTypes.size() > 0) {
					params.mWeaponType = ei.weaponTypes[randmod(ei.weaponTypes.size())];
				}
			}

			params.dropRateProfile = g_DropRateProfileManager.GetProfileByName(profile.virtualItemReward.dropRateProfileName);
			params.ClampLimits();

			int itemID = g_ItemManager.RollVirtualItem(params);
			if(itemID != -1) {
				loot.AddItem(itemID);
			}

			break;
		}
		case RewardType::ITEM:
			ItemDef *item = NULL;
			int itemID = 0;
			// Have 10 attempts at getting a valid item.
			for(int i = 0 ; i < 10; i++) {
				itemID = profile.itemReward.itemIDs[randmod(profile.itemReward.itemIDs.size())];
				item = g_ItemManager.GetPointerByID(itemID);
				if(item != NULL) {
					loot.AddItem(itemID);
					break;
				}
			}
			if(item == NULL) {
				g_Log.AddMessageFormat("[WARNING] An item (%d) exists in the daily loot table that does not exist. Player wi", itemID);
			}
			break;
		}
	}

	//New drop system.  Uses the drop tables found in the Loot subfolder.
	//Roll the drops then merge them into the single container that will be assigned
	//to the creature.
//	std::vector<int> itemList;
//	DropRollParameters drp;
//	drp.mCreatureDefID = CreatureDefID;
//	drp.mCreatureLevel = level;
//	drp.mPlayerLevel = level;
//	g_DropTableManager.RollDrops(drp, itemList);
//
//	for(size_t i = 0; i < itemList.size(); i++)
//	{
//		loot.AddItem(itemList[i]);
//	}

	if(loot.itemList.size() > 0)
		activeLootID = actInst->lootsys.AttachLootToCreature(loot, CreatureID);
}

void  CreatureInstance :: CreateLoot(int finderLevel, int partySize)
{
	//Called whenever this creature dies.  Generates and associates a list of loot to this creature,
	//if applicable.

	//Hack to prevent unfinished/unimplemented (null appearance) creatures from dropping anything.
	if(PeekAppearance().size() == 0)
		return;

	// Sidekicks cannot be looted at all.
	if(serverFlags & ServerFlags::IsSidekick)
		return;

	// Player loot comes from their inventory
	if(serverFlags & ServerFlags::IsPlayer)
		return;

	//Don't fetch a new loot container if it already has one.
	if(activeLootID != 0)
		return;

	// *** Basic conditional checks passed, generate the loot. ***


	ActiveLootContainer loot;
	loot.CreatureID = CreatureID;

	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CreatureDefID);
	if(cdef == NULL)
		return;

	float dropRateBonus = GetDropRateMultiplier(cdef);



	//Virtual items.
	VirtualItemSpawnParams params;
	if(Util::FloatEquivalent(dropRateBonus, 1.0F) == false)
		params.SetAllDropMult(dropRateBonus);

	params.level = css.level;
	params.rarity = css.rarity;
	params.namedMob = cdef->NamedMob;

	DropRateProfile dropRateProfile;
	if(cdef->DropRateProfile.length() == 0 && actInst != NULL)
		dropRateProfile.CopyFrom(g_DropRateProfileManager.GetProfileByName(actInst->mZoneDefPtr->GetDropRateProfile()));
	else
		dropRateProfile.CopyFrom(g_DropRateProfileManager.GetProfileByName(cdef->DropRateProfile));

	params.dropRateProfile = dropRateProfile;
	params.ClampLimits();


	/* Decide how many random items to actually drop. This is determined by the
	 * drop profile
	 */
	int amount = 0;
	int amountMultiply = 1;
	if(dropRateProfile.GetAmountChance(0) == -1) {
		amountMultiply = partySize;
	}

	/* Work backwards from 6 items to 1, testing if there is a chance to get that
	 * many items
	 */
	for(int i = 6 ; i >= 1 ; i--) {
		int baseRoll = randint_32bit(1, VirtualItemModSystem::DROP_SHARES);
		int compare = dropRateProfile.Amount.QData[i];
		if(baseRoll <= compare) {
			amount = i;
			break;
		}
	}

	/* Didn't get any amount, use the default? */
	if(amount == 0 && dropRateProfile.GetAmountChance(0) > 0) {
		/* Default amount is > 0, so use this as a default as none of the other levels have values */
		amount = dropRateProfile.GetAmountChance(0);
	}

	amount *= amountMultiply;

	for(int i = 0 ; i < amount; i++) {
		loot.AddItem(g_ItemManager.RollVirtualItem(params));
	}

	//New drop system.  Uses the drop tables found in the Loot subfolder.
	//Roll the drops then merge them into the single container that will be assigned
	//to the creature.
	std::vector<int> itemList;
	DropRollParameters drp;
	drp.mCreatureDefID = CreatureDefID;
	drp.mCreatureLevel = css.level;
	drp.mPlayerLevel = finderLevel;
	g_DropTableManager.RollDrops(drp, itemList);

	for(size_t i = 0; i < itemList.size(); i++)
	{
		loot.AddItem(itemList[i]);
	}

	activeLootID = actInst->lootsys.AttachLootToCreature(loot, CreatureID);
}

void CreatureInstance :: AddLootableID(int newLootableID)
{
	//Players cannot be looted.
	if(serverFlags & ServerFlags::IsPlayer)
		return;

	if(activeLootID == 0)
		return;

	actInst->lootsys.creatureList[activeLootID].AddLootableID(newLootableID);
}

void CreatureInstance :: SetLevel(int newLevel)
{
	// This is only for players, because only simulator characters
	// have the associated character information to update.
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	int oldLevel = css.level;
	int abilityPoints = 0;
	if(newLevel > oldLevel)
	{
		for(int lev = oldLevel + 1; lev <= newLevel; lev++)
			abilityPoints += Global::GetAbilityPointsLevelIncrement(lev);

		/*
		int levelsGained = newLevel - css.level;
		int abilityPoints = 0;
		if(levelsGained > 0)
			abilityPoints = levelsGained * ABILITY_POINTS_PER_LEVEL;
		*/
	}

	css.level = newLevel;
	g_Log.AddMessageFormat("Player [%s] had reached level [%d]", css.display_name, css.level);

	if(actInst == NULL)
		return;

	char sbuffer[256];
	Util::SafeFormat(sbuffer, sizeof(sbuffer), "%s is now level %d!", css.display_name, css.level);
	g_SimulatorManager.BroadcastMessage(sbuffer);

	std::vector<short> statList;
	RemoveStatModsBySource(BuffSource::ITEM);
	charPtr->UpdateBaseStats(this, true);
	charPtr->UpdateEquipStats(this);

	SendUpdatedBuffs();

	//Since proper stats are filled in again, proceed.
	int health = CalcRestrictedHealth(-1, true);
	if(css.health > health)
		css.health = health;

	statList.push_back(STAT::LEVEL);
	statList.push_back(STAT::STRENGTH);
	statList.push_back(STAT::DEXTERITY);
	statList.push_back(STAT::CONSTITUTION);
	statList.push_back(STAT::PSYCHE);
	statList.push_back(STAT::SPIRIT);
	statList.push_back(STAT::HEALTH);
	statList.push_back(STAT::HEALTH_MOD);
	statList.push_back(STAT::BASE_STATS);

	if(abilityPoints > 0)
	{
		css.current_ability_points += abilityPoints;
		css.total_ability_points += abilityPoints;
		statList.push_back(STAT::CURRENT_ABILITY_POINTS);
		statList.push_back(STAT::TOTAL_ABILITY_POINTS);
	}

	if(newLevel == MAX_LEVEL)
	{
		css.experience = LevelRequirements[MAX_LEVEL][1];
		statList.push_back(STAT::EXPERIENCE);
	}

	if(charPtr != NULL)
		charPtr->OnLevelChange(newLevel);

	int wpos = PrepExt_SendSpecificStats(GSendBuf, this, statList);
	//actInst->LSendToOneSimulator(GSendBuf, wpos, simulatorPtr);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}

void CreatureInstance :: AdjustCopper(int coinChange)
{
	css.copper += coinChange;
	if(css.copper < 0)
		css.copper = 0;
	SendStatUpdate(STAT::COPPER);
	/*
	if(packetBuffer != NULL)
	{
		static const short statCopper = STAT::COPPER;
		return PrepExt_SendSpecificStats(packetBuffer, this, &statCopper, 1);
	}
	return 0;
	*/
}

//Called by an NPC to determine how close it must be to the target object
//to auto aggro against it.
int CreatureInstance :: GetAggroRange(CreatureInstance *target)
{
	int levelDiff = css.level - target->css.level;
	int aggroRange = AGGRO_DEFAULT_RANGE + (AGGRO_LEVEL_MOD * levelDiff);
	int lowest = AGGRO_MINIMUM_RANGE;
	if(target->HasStatus(StatusEffects::WALK_IN_SHADOWS) || target->HasStatus(StatusEffects::INVISIBLE))
	{
		if(isTargetInCone(target, Cone_90) == false && css.rarity <= CreatureRarityType::HEROIC)
		{
			aggroRange = AGGRO_MINIMUM_RANGE / 2;
			lowest = AGGRO_MINIMUM_RANGE / 2;
		}
		else
		{
			switch(css.rarity)
			{
			case CreatureRarityType::NORMAL: aggroRange /= 4; break;
			case CreatureRarityType::HEROIC: aggroRange /= 2; break;
			}
			lowest = AGGRO_MINIMUM_RANGE;
		}
	}

	int maximum = AGGRO_MAXIMUM_RANGE;
	if(maximum > actInst->mZoneDefPtr->mMaxAggroRange)
		maximum = actInst->mZoneDefPtr->mMaxAggroRange;

	return Util::ClipInt(aggroRange, lowest, maximum);
}

bool CreatureInstance :: IsTransformed()
{
//	return serverFlags & ServerFlags::IsTransformed;
	return transformCreatureId != 0;
}

bool CreatureInstance :: CAF_Untransform()
{
	if(transformModifier == NULL) {
		g_Log.AddMessageFormat("%d not transformed", CreatureDefID);
		return false;
	}
	g_Log.AddMessageFormat("Untransforming %d (app is %s)", CreatureDefID, css.appearance.c_str());
	_ClearStatusFlag(StatusEffects::TRANSFORMED);
	_ClearStatusFlag(StatusEffects::INVISIBLE_EQUIPMENT);
	SetServerFlag(ServerFlags::IsTransformed, false);
	RemoveAppearanceModifier(transformModifier);
	transformCreatureId = 0;
	transformAbilityId = 0;
	transformModifier = NULL;
	return true;
}

bool CreatureInstance :: CAF_Nudify(int durationS)
{
	if(transformModifier != NULL) {
		g_Log.AddMessageFormat("%d already transformed", CreatureDefID);
		return false;
	}
	g_Log.AddMessageFormat("Nudifying %d (app is %s)", CreatureDefID, css.appearance.c_str());
	transformModifier = new NudifyAppearanceModifier();
	_AddStatusList(StatusEffects::INVISIBLE_EQUIPMENT, durationS);
	PushAppearanceModifier(transformModifier);
	return true;

}
bool CreatureInstance :: CAF_Scale(float scale, int durationS)
{
	if(transformModifier != NULL) {
		g_Log.AddMessageFormat("%d already transformed", CreatureDefID);
		return false;
	}
	g_Log.AddMessageFormat("Scaling %d (app is %s) to %f", CreatureDefID, css.appearance.c_str(), scale);
	char buf[64];
	Util::SafeFormat(buf,sizeof(buf),"%f", scale);
	transformModifier = new CreatureAttributeModifier("sz", buf);
	_AddStatusList(StatusEffects::TRANSFORMED, durationS);
	PushAppearanceModifier(transformModifier);
	return true;

}

bool CreatureInstance :: CAF_Transform(int CDefID, int abID, int durationS)
{
	if(transformModifier != NULL) {
		g_Log.AddMessageFormat("%d already transformed into %d", CreatureDefID, transformCreatureId);
		return false;
	}

	g_Log.AddMessageFormat("Transforming %d into %d", CreatureDefID, CDefID);
	_AddStatusList(StatusEffects::TRANSFORMED, durationS);
	SetServerFlag(ServerFlags::IsTransformed, true);

	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef != NULL)
	{
		transformModifier = new ReplaceAppearanceModifier(
				cdef->css.appearance.c_str());
		PushAppearanceModifier(transformModifier);
		transformCreatureId = CDefID;
		transformAbilityId = abID;
		return true;
	}
	else {
		transformCreatureId = 0;
		transformAbilityId= 0;
	}
	return false;

}

int CreatureInstance :: CAF_SummonSidekick(int CDefID, int maxSummon, short abGroupID)
{
	//Summon a sidekick with special conditions.
	//The sidekick is linked with this group, allowing multiple skills to produce
	//different sets of sidekicks.  It also verifies whether the maximum number of
	//summons has been reached for this ability group.

	//Character sidekick information is only available to players.
	if(!(serverFlags & ServerFlags::IsPlayer))
		return -1;

	//Count the number of existing sidekicks that match this profile.
	int count = 0;
	for(size_t i = 0; i < charPtr->SidekickList.size(); i++)
	{
		if(charPtr->SidekickList[i].summonType != SidekickObject::ABILITY)
			continue;

		if(charPtr->SidekickList[i].summonParam != abGroupID)
			continue;

		count++;
	}

	if(count >= maxSummon)
	{
		int wpos = PrepExt_SendInfoMessage(GSendBuf, "You cannot summon any more creatures of that type.", INFOMSG_INFO);
		simulatorPtr->AttemptSend(GSendBuf, wpos);
		return -1;
	}

	SidekickObject skobj(CDefID, SidekickObject::ABILITY, abGroupID);
	charPtr->SidekickList.push_back(skobj);
	CreatureInstance* nsk = actInst->InstantiateSidekick(this, skobj, 0);
	nsk->CurrentX = CurrentX + randint(-50, 50);
	nsk->CurrentZ = CurrentZ + randint(-50, 50);
	if(nsk != NULL)
	{
		actInst->RebuildSidekickList();
		//nsk->CAF_RunSidekickStatFilter(abGroupID);
		return 1;
	}
	return -1;
}

void CreatureInstance :: CAF_RunSidekickStatFilter(int abGroupID)
{
	if(AnchorObject == NULL)
		return;

	switch(abGroupID)
	{
	case SIDEKICK_ABILITY_GROUP_ID:
		StatScaleToLevel(STAT::STRENGTH, AnchorObject->css.level);
		StatScaleToLevel(STAT::DEXTERITY, AnchorObject->css.level);
		StatScaleToLevel(STAT::CONSTITUTION, AnchorObject->css.level);
		StatScaleToLevel(STAT::SPIRIT, AnchorObject->css.level);
		StatScaleToLevel(STAT::PSYCHE, AnchorObject->css.level);
		StatScaleToLevel(STAT::BASE_DAMAGE_MELEE, AnchorObject->css.level);
		StatScaleToLevel(STAT::DAMAGE_RESIST_DEATH, AnchorObject->css.level);
		StatScaleToLevel(STAT::DAMAGE_RESIST_MYSTIC, AnchorObject->css.level);
		StatScaleToLevel(STAT::DAMAGE_RESIST_FIRE, AnchorObject->css.level);
		StatScaleToLevel(STAT::DAMAGE_RESIST_FROST, AnchorObject->css.level);
		StatScaleToLevel(STAT::DAMAGE_RESIST_MELEE, AnchorObject->css.level);
		css.level = AnchorObject->css.level;
		break;
	}
}

void CreatureInstance :: StatScaleToLevel(int statID, int targetLevel)
{
	float value = GetStatValueByID(statID, &css);
	float mult = (float)targetLevel / (float)css.level;
	g_Log.AddMessageFormat("Level %d to %d, value: %g, mult: %g, result: %g", css.level, targetLevel, value, mult, value * mult);
	WriteValueToStat(statID, value * mult, &css);
}

int CreatureInstance :: CAF_RegisterTargetSidekick(int abGroupID)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return 0;

	int count = 0;
	if(serverFlags & ServerFlags::IsTransformed)
	{
		g_Log.AddMessageFormat("Player is transformed.");
		ab[0].TargetList[count++] = this;
	}

	size_t search = 0;

	for(size_t i = 0; i < charPtr->SidekickList.size(); i++)
	{
		if(count == MAXTARGET)
			break;


		if(charPtr->SidekickList[i].summonType != SidekickObject::ABILITY)
			continue;

		if(charPtr->SidekickList[i].summonParam != abGroupID)
			continue;

		for(size_t s = search; s < actInst->SidekickListPtr.size(); s++)
			if(actInst->SidekickListPtr[s]->AnchorObject == this)
				if(actInst->SidekickListPtr[s]->CreatureDefID == charPtr->SidekickList[i].CDefID)
				{
					ab[0].TargetList[count++] = actInst->SidekickListPtr[s];
					search = s + 1;
				}

		if(search >= actInst->SidekickListPtr.size())
			break;
	}
	ab[0].TargetCount = count;
	g_Log.AddMessageFormat("%d targets found", count);
	for(int i = 0; i < count; i++)
		g_Log.AddMessageFormat("[%d] = [%s]", i, ab[0].TargetList[i]->css.display_name);

	return count;
}

void CreatureInstance :: DetachItem(const char *type, const char *node) {
	for(uint i = 0 ; i < appearanceModifiers.size(); i++) {
		AddAttachmentModifier *mod = dynamic_cast<AddAttachmentModifier*>(appearanceModifiers[i]);
		if(mod && mod->mType == type && mod->mNode == node) {
			RemoveAppearanceModifier(mod);
			break;
		}
	}
}

void CreatureInstance :: AttachItem(const char *type, const char *node) {
	PushAppearanceModifier(new AddAttachmentModifier(type, node));
}

void CreatureInstance :: ClearAppearanceModifiers()
{
	std::vector<AppearanceModifier*>::iterator it = appearanceModifiers.begin();
	for(; it != appearanceModifiers.end(); ++it)
		delete *it;
	appearanceModifiers.clear();
}

void CreatureInstance :: RemoveAppearanceModifier(AppearanceModifier *modifier)
{
	g_Log.AddMessageFormat("Removing an appearance modifier for %d from a list of %d", CreatureID, appearanceModifiers.size());
	std::vector<AppearanceModifier*>::iterator it = appearanceModifiers.begin();
	for(; it != appearanceModifiers.end(); ++it) {
		AppearanceModifier *app = *it;
		if(app == modifier) {
			g_Log.AddMessageFormat("Removing a modifier");
			delete *it;
			appearanceModifiers.erase(it);
			break;
		}
	}
	actInst->LSendToLocalSimulator(GSendBuf, PrepExt_UpdateAppearance(GSendBuf, this), CurrentX, CurrentZ);
}

std::string CreatureInstance :: PeekAppearanceEq()
{
	std::string appearance = css.eq_appearance;
	std::vector<AppearanceModifier*>::iterator it = appearanceModifiers.begin();
	for(; it != appearanceModifiers.end(); ++it)
		appearance = (*it)->ModifyEq(appearance);
	return appearance;
}

std::string CreatureInstance :: PeekAppearance()
{
	std::string appearance = css.appearance;
	std::vector<AppearanceModifier*>::iterator it = appearanceModifiers.begin();
	for(; it != appearanceModifiers.end(); ++it)
		appearance = (*it)->Modify(appearance);
	return appearance;
}

void CreatureInstance :: PushAppearanceModifier(AppearanceModifier *modifier)
{
	g_Log.AddMessageFormat("Pushing an appearance modifier to %d in a list of %d, now %d",
			CreatureID, appearanceModifiers.size(), appearanceModifiers.size() + 1);
	appearanceModifiers.push_back(modifier);
	int wpos = PrepExt_UpdateAppearance(GSendBuf, this);
	actInst->LSendToLocalSimulator(GSendBuf, wpos, CurrentX, CurrentZ);
}

void CreatureInstance :: NotifySuperCrit(int TargetCreatureID)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return;

	int wpos = 0;
	wpos += PutByte(&GAuxBuf[wpos], 100);  //_handleModMessage
	wpos += PutShort(&GAuxBuf[wpos], 0);

	wpos += PutByte(&GAuxBuf[wpos], 1);      //event
	wpos += PutInteger(&GAuxBuf[wpos], TargetCreatureID);   //actorID

	PutShort(&GAuxBuf[1], wpos - 3);
	actInst->LSendToOneSimulator(GAuxBuf, wpos, simulatorPtr);

	/*
	int size = PrepExt_GenericChatMessage(GAuxBuf, TargetCreatureID, "", "conOrange", "Super Crit!");
	actInst->LSendToOneSimulator(GAuxBuf, size, simulatorPtr);
	*/
}

int CreatureInstance :: GetMainhandDamage(void)
{
	int amount = (int)(css.strength * 0.3) + css.base_damage_melee + randint(MainDamage[0], MainDamage[1]);
	amount += GetAdditiveWeaponSpecialization(amount, ItemEquipSlot::WEAPON_MAIN_HAND);
	return amount;
}

int CreatureInstance :: GetOffhandDamage(void)
{
	if(!(HasStatus(StatusEffects::CAN_USE_DUAL_WIELD)))
		return 0;

	if(css.profession != Professions::ROGUE)
		return 0;

	//Don't compute any damage if no weapon is equipped, because otherwise we'd still get the
	//damage of the strength bonus.
	if(EquippedWeapons[ItemEquipSlot::WEAPON_OFF_HAND] == 0 || OffhandDamage[1] == 0)
		return 0;

	float damage = (css.strength * 0.3F) + css.base_damage_melee + randint(OffhandDamage[0], OffhandDamage[1]);
	damage *= ((float)css.offhand_weapon_damage / 1000.0F);
	damage += (float)GetAdditiveWeaponSpecialization(static_cast<int>(damage), ItemEquipSlot::WEAPON_OFF_HAND);
	return (int)damage;
}

// Processes a respec action.  Remove active buffs and reset the generic status effects
// that are commonly set by default abilities (which may be class specific).
void CreatureInstance :: Respec(void)
{
	RemoveAllBuffsExceptGroupID(false, 999);

	static const int EffectList[] = {
		StatusEffects::CAN_USE_WEAPON_2H,
		StatusEffects::CAN_USE_WEAPON_1H,
		StatusEffects::CAN_USE_WEAPON_SMALL,
		StatusEffects::CAN_USE_WEAPON_POLE,
		StatusEffects::CAN_USE_WEAPON_BOW,
		StatusEffects::CAN_USE_WEAPON_THROWN,
		StatusEffects::CAN_USE_WEAPON_WAND,
		StatusEffects::CAN_USE_DUAL_WIELD,
		StatusEffects::CAN_USE_PARRY,
		StatusEffects::CAN_USE_BLOCK,
		StatusEffects::CAN_USE_ARMOR_CLOTH,
		StatusEffects::CAN_USE_ARMOR_LIGHT,
		StatusEffects::CAN_USE_ARMOR_MEDIUM,
		StatusEffects::CAN_USE_ARMOR_HEAVY,
		StatusEffects::INVISIBLE,
		StatusEffects::WALK_IN_SHADOWS,
	};
	static const int numEffects = sizeof(EffectList) / sizeof(const int);
	for(int i = 0; i < numEffects; i++)
		_RemoveStatusList(EffectList[i]);

	size_t pos = 0;
	while(pos < implicitActions.size())
	{
		if(implicitActions[pos].abilityID != 0)
			implicitActions.erase(implicitActions.begin() + pos);
		else
			pos++;
	}
}

bool CreatureInstance :: ValidateEquippableWeapon(int mWeaponType)
{
	if(!(serverFlags & ServerFlags::IsPlayer))
		return false;
	return WeaponTypeClassRestrictions::isValid(mWeaponType, css.profession);
}

void CreatureInstance :: OnEquipmentChange(float oldHealthRatio)
{
	//Reapply the new buff totals to the stat set.
	SendUpdatedBuffs();

	//Adding or removing equipment with constitution will update the health total.
	//Calculate the new health value based on the ratio passed to this function.
	//Also update heroism since the bonus must reflect a percentage of health.
	int newMax = GetMaxHealth(true);
	int newHealth = (int)((float)newMax * oldHealthRatio);
	css.health = CalcRestrictedHealth(Util::ClipInt(newHealth, 0, newMax), true);
	SendStatUpdate(STAT::HEALTH);
	OnHeroismChange();
}

// Scale a stat by a multiplier that scales proportionally with level difference.
int CreatureInstance :: GetLevelScaledValue(int originalAmount, int targetLevel, float multPerLevel)
{
	int levelDiff = targetLevel - css.level;
	if(levelDiff < 1)
		levelDiff = 1;
	float generalMultiplier = 1.0F + (levelDiff * multPerLevel);
	return (int)(originalAmount * generalMultiplier);
}

// Scale a stat by multiplying its current value by a multiplier.
int CreatureInstance :: GetScaledValue(int originalAmount, float additionalMult)
{
	if(Util::FloatEquivalent(additionalMult, 0.0F) == true)
		return originalAmount;

	return (int)(originalAmount * additionalMult);
}

void CreatureInstance :: PerformLevelScale(const InstanceScaleProfile *scaleProfile, int targetLevel)
{
	if(scaleProfile == NULL)
		return;

	//Negative values indicate scaling.
	int newLevel = css.level;
	if(scaleProfile->mLevelOffset >= 0)
		newLevel += scaleProfile->mLevelOffset;
	else
		newLevel = targetLevel;


	//Only scale mobs if they're below the requested level
	int formerLevel = css.level;
	if(css.level < newLevel)
		css.level = newLevel;

	int levelDiff = newLevel - formerLevel;
	if(levelDiff < 0)
		levelDiff = 0;

	// mult ^ 0 = 1.0
	// mult ^ 1 = mult
	// mult ^ 2 = mult * mult
	float core = pow(1.0F + scaleProfile->mCoreMultPerLev, levelDiff);
	float armor = pow(1.0F + scaleProfile->mArmorMultPerLev, levelDiff);
	float damage = pow(1.0F + scaleProfile->mDmgMultPerLev, levelDiff);

	//Scale up to level first.
	css.strength = GetScaledValue(css.strength, core);
	css.dexterity = GetScaledValue(css.dexterity, core);
	css.constitution = GetScaledValue(css.constitution, core);
	css.psyche = GetScaledValue(css.psyche, core);
	css.spirit = GetScaledValue(css.spirit, core);

	css.damage_resist_melee = GetScaledValue(css.damage_resist_melee, armor);
	css.damage_resist_fire = GetScaledValue(css.damage_resist_fire, armor);
	css.damage_resist_frost = GetScaledValue(css.damage_resist_frost, armor);
	css.damage_resist_mystic = GetScaledValue(css.damage_resist_mystic, armor);
	css.damage_resist_death = GetScaledValue(css.damage_resist_death, armor);
	
	css.base_damage_melee = GetScaledValue(css.base_damage_melee, damage);

	//Now scale by difficulty bonus multipliers.
	if(scaleProfile->mStatMultBonus > 0.0F)
	{
		css.strength = GetScaledValue(css.strength, scaleProfile->mStatMultBonus);
		css.dexterity = GetScaledValue(css.dexterity, scaleProfile->mStatMultBonus);
		css.psyche = GetScaledValue(css.psyche, scaleProfile->mStatMultBonus);
		css.spirit = GetScaledValue(css.spirit, scaleProfile->mStatMultBonus);
	}

	if(scaleProfile->mConMultBonus > 0.0F)
	{
		css.constitution = GetScaledValue(css.constitution, scaleProfile->mConMultBonus);
	}

	if(scaleProfile->mArmorMultBonus > 0.0F)
	{
		css.damage_resist_melee = GetScaledValue(css.damage_resist_melee, scaleProfile->mArmorMultBonus);
		css.damage_resist_fire = GetScaledValue(css.damage_resist_fire, scaleProfile->mArmorMultBonus);
		css.damage_resist_frost = GetScaledValue(css.damage_resist_frost, scaleProfile->mArmorMultBonus);
		css.damage_resist_mystic = GetScaledValue(css.damage_resist_mystic, scaleProfile->mArmorMultBonus);
		css.damage_resist_death = GetScaledValue(css.damage_resist_death, scaleProfile->mArmorMultBonus);
	}
	
	if(scaleProfile->mDmgMultBonus > 0.0F)
	{
		css.base_damage_melee = GetScaledValue(css.base_damage_melee, scaleProfile->mDmgMultBonus);
	}

	//Adjust health and clamp limits in case they're too high.
	css.health = CalcRestrictedHealth(-1, true);
	LimitValueOverflows();
	
}

/* OBSOLETE
void CreatureInstance :: PerformLevelScale(int targetLevel, float vitalMult, float weaponMult)
{
	if(css.level > targetLevel)
		return;

	css.strength = GetScaledValue(css.strength, targetLevel, vitalMult);
	css.dexterity = GetScaledValue(css.dexterity, targetLevel, vitalMult);
	css.constitution = GetScaledValue(css.constitution, targetLevel, vitalMult);
	css.psyche = GetScaledValue(css.psyche, targetLevel, vitalMult);
	css.spirit = GetScaledValue(css.spirit, targetLevel, vitalMult);

	css.base_damage_melee = GetScaledValue(css.base_damage_melee, targetLevel, vitalMult);
	css.damage_resist_melee = GetScaledValue(css.damage_resist_melee, targetLevel, vitalMult);
	css.damage_resist_fire = GetScaledValue(css.damage_resist_fire, targetLevel, vitalMult);
	css.damage_resist_frost = GetScaledValue(css.damage_resist_frost, targetLevel, vitalMult);
	css.damage_resist_mystic = GetScaledValue(css.damage_resist_mystic, targetLevel, vitalMult);
	css.damage_resist_death = GetScaledValue(css.damage_resist_death, targetLevel, vitalMult);

	css.level = targetLevel;
	css.health = GetMaxHealth(true);
}
*/

void CreatureInstance :: BuildZoneString(int instanceID, int zoneID, int unknownID)
{
	Util::SafeFormat(ZoneString, sizeof(ZoneString), "[%d-%d-%d]", instanceID, zoneID, unknownID);
}

void CreatureInstance :: ApplyGlobalInstanceBuffs(void)
{
	if(g_Config.GlobalMovementBonus != 0)
		AddBuff(BuffSource::INSTANCE, 0, 0, 0, 0, STAT::MOD_MOVEMENT, g_Config.GlobalMovementBonus, g_Config.GlobalMovementBonus, -1);
}

void CreatureInstance :: OnInstanceEnter(const ArenaRuleset &arenaRuleset)
{
	ApplyGlobalInstanceBuffs();


	if(arenaRuleset.mEnabled == false)
		return;

	if(arenaRuleset.mPVPStatus == PVP::GameMode::SPECIAL_EVENT || arenaRuleset.mPVPStatus == PVP::GameMode::PVP_ONLY || (arenaRuleset.mPVPStatus != PVP::GameMode::PVE_ONLY && charPtr->Mode == PVP::GameMode::PVP ) )
	{
		_AddStatusList(StatusEffects::PVPABLE, -1);

		/*
		int wpos = 0;
		wpos += PutByte(&GSendBuf[wpos], 80);  //_handlePVPStatUpdateMessage
		wpos += PutShort(&GSendBuf[wpos], 0);

		wpos += PutInteger(&GSendBuf[wpos], PVPUpdateFlag::PVP_STATE_UPDATE);
		wpos += PutByte(&GSendBuf[wpos], 0);

		wpos += PutByte(&GSendBuf[wpos], PVPGameState::PLAYING);
		PutShort(&GSendBuf[1], wpos);
		simulatorPtr->AttemptSend(GSendBuf, wpos);
		*/
	}
	else
	{
		_RemoveStatusList(StatusEffects::PVPABLE);
	}

	// TODO these mod things seem like a good idea :)
	return;

	if(arenaRuleset.mTurboRunSpeed == true)
		AddBuff(BuffSource::INSTANCE, 0, 0, 0, 0, STAT::MOD_MOVEMENT, 40, 40, -1);
	for(size_t i = 0; i < arenaRuleset.mRuleList.size(); i++)
	{
		const ArenaRule *rule = &arenaRuleset.mRuleList[i];
		bool pass = false;
		switch(rule->mRuleType)
		{
		case ArenaRule::RULE_MOD_CLASS:
			pass = rule->IsMatchProfession(css.profession);
			break;
		case ArenaRule::RULE_MOD_PLAYERS:
			pass = rule->IsMatchDisplayName(css.display_name);
			break;
		}
		if(pass == true)
		{
			g_Log.AddMessageFormat("Matched: %d", rule->mRuleType);
			int StatID = GetStatIndexByName(rule->mStatName.c_str());
			if(StatID == -1)
				continue;
			float additive = 0.0F;
			switch(rule->mStatApplyType)
			{
			case ArenaRule::APPLY_ADDITIVE:
				additive = rule->mStatChange;
				break;
			case ArenaRule::APPLY_MULTIPLY:
				float value = GetStatValueByID(StatID, &css);
				additive = Util::Round(value * rule->mStatChange);
			}
			AddBuff(BuffSource::INSTANCE, 0, 0, 0, 0, StatID, additive, additive, -1);
		}
		else
		{
			g_Log.AddMessageFormat("Failed: %d", rule->mRuleType);
		}
	}
}

void CreatureInstance :: OnInstanceExit(void)
{
	UnHate();

	if(_HasStatusList(StatusEffects::PVPABLE))
	{
		_RemoveStatusList(StatusEffects::PVPABLE);
		_RemoveStatusList(StatusEffects::UNATTACKABLE);
		RemoveStatModsBySource(BuffSource::INSTANCE);
	}
}

void CreatureInstance :: BroadcastUpdateElevationSelf(void)
{
	if(actInst == NULL)
		return;

	int simID = -1;
	if(simulatorPtr != NULL)
		simID = simulatorPtr->InternalID;

	int size = PrepExt_UpdateFullPosition(GSendBuf, this);
	actInst->LSendToLocalSimulator(GSendBuf, size, CurrentX, CurrentZ, simID);

	size += PrepExt_SetAvatar(&GSendBuf[size], CreatureID);
	actInst->LSendToOneSimulator(GSendBuf, size, simID);
}

void CreatureInstance :: BroadcastLocal(const char *buffer, int dataLen, int simulatorFilter)
{
	if(actInst == NULL)
		return;
	actInst->LSendToLocalSimulator(buffer, dataLen, CurrentX, CurrentZ, simulatorFilter);
}

float CreatureInstance :: GetTotalSize(void)
{
	//Default value hardcoded in the client.
	static const float gMinCreatureSize = 5.0f;

	float retval = css.total_size;
	if(retval < gMinCreatureSize)
		retval = gMinCreatureSize;
	return retval;
}

// This is a relatively new range check called by the targetting system which makes better use of object size.
bool CreatureInstance :: IsObjectInRange(CreatureInstance *object, float distance)
{
	if(object == NULL)
		return false;

	//Quick check for self, always in range.
	if(this == object)
		return true;

	float tolerance = GetTotalSize() + object->GetTotalSize();
	float acceptDist = tolerance + distance;

	//Don't include Y axis since the server doesn't know about elevation differences.
	float objDist = ActiveInstance::GetPlaneRange(this, object, static_cast<int>(acceptDist));
	if(objDist <= acceptDist)
		return true;

	return false;
}

int CreatureInstance :: GetDistance(CreatureInstance *object, int threshold)
{
	if(object == NULL)
		return -1;

	//Quick check for self, always in range.
	if(this == object)
		return 0;


	float tolerance = GetTotalSize() + object->GetTotalSize();
	float acceptDist = tolerance + threshold;

	//Don't include Y axis since the server doesn't know about elevation differences.
	return ActiveInstance::GetPlaneRange(this, object, static_cast<int>(acceptDist));
}

bool CreatureInstance :: IsSelfNearPoint(float x, float z, float distance)
{
	float tolerance = GetTotalSize();
	float acceptDist = tolerance + distance;
	float dist = ActiveInstance::GetPointRangeXZ(this, x, z, static_cast<int>(acceptDist));
	if(dist <= acceptDist)
		return true;

	return false;
}

void CreatureInstance :: DebugGenerateReport(ReportBuffer &report)
{
	char ConvBuf[32];
	size_t i;
	for(i = 0; i < NumStats; i++)
		if(isStatZero(i, &css) == false)
			report.AddLine("%s=%s", StatList[i].name, GetStatValueAsString(i, ConvBuf, &css));

	report.AddLine(NULL);

	report.AddLine("BUFF TOTALS");
	for(i = 0; i < baseStats.size(); i++)
	{
		int r = GetStatIndex(baseStats[i].StatID);
		if(r >= 0)
			report.AddLine("%d = %s (%d mods): %g + %g = %g",
				baseStats[i].StatID,
				StatList[r].name,
				baseStats[i].RefCount,
				baseStats[i].fBaseVal,
				baseStats[i].fModTotal,
				baseStats[i].fBaseVal + baseStats[i].fModTotal );
		else
			report.AddLine("UNDEFINED STAT (%d)", baseStats[i].StatID);
	}

	report.AddLine(NULL);
	report.AddLine("STATUS EFFECTS");
	for(i = 0; i < activeStatusEffect.size(); i++)
	{
		int statID = activeStatusEffect[i].modStatusID;
		unsigned long extime = activeStatusEffect[i].expireTime - g_ServerTime;
		if(activeStatusEffect[i].expireTime == PlatformTime::MAX_TIME)
			report.AddLine("%hd = infinite (%s)", activeStatusEffect[i].modStatusID, GetStatusNameByID(statID));
		else
			report.AddLine("%hd = %lu (%s)", activeStatusEffect[i].modStatusID, extime, GetStatusNameByID(statID));
	}

	report.AddLine(NULL);
	report.AddLine("IMPLICIT ACTIONS");
	for(i = 0; i < implicitActions.size(); i++)
	{
		report.AddLine("[%d] = type:%d, abID:%d, abGroup:%d",
			i, 
			implicitActions[i].eventType,
			implicitActions[i].abilityID,
			implicitActions[i].abilityGroup);
	}

	if(aiNut != NULL)
	{
		report.AddLine(NULL);
		report.AddLine("AI NUT");
		aiNut->DebugGenerateReport(report);
	}
	if(aiScript != NULL)
	{
		report.AddLine(NULL);
		report.AddLine("AI SCRIPT");
		aiScript->DebugGenerateReport(report);
	}
}

//**************************************************
//               CreatureDefManager
//**************************************************


CreatureDefManager :: CreatureDefManager()
{
}

CreatureDefManager :: ~CreatureDefManager()
{
	Clear();
}

void CreatureDefManager :: Clear(void)
{
	//PC.clear();
	NPC.clear();
}

void CreatureDefManager :: AddNPCDef(CreatureDefinition &newItem)
{
	CREATURE_MAP::iterator it = NPC.find(newItem.CreatureDefID);
	if(it == NPC.end())
		NPC.insert(NPC.end(), CREATURE_PAIR(newItem.CreatureDefID, newItem));
	else
	{
		g_Log.AddMessageFormat("[WARNING] Overwriting Creature Def: %d", newItem.CreatureDefID);
		it->second.CopyFrom(newItem);
	}
}

CreatureDefinition* CreatureDefManager :: GetPointerByCDef(int CDefID)
{
	CREATURE_MAP::iterator it = NPC.find(CDefID);
	if(it == NPC.end())
		return NULL;
	return &it->second;
}

// A slow lookup intended for some arbitrary data processing
CreatureDefinition* CreatureDefManager :: GetPointerByName(const char *name)
{
	CREATURE_MAP::iterator it;
	for(it = NPC.begin(); it != NPC.end(); ++it)
		if(strcmp(it->second.css.display_name, name) == 0)
			return &it->second;
	return NULL;
}

/*
int CreatureDefManager :: GetIndex(long CDefID)
{
	int a;
	for(a = 0; a < (int)NPC.size(); a++)
		if(NPC[a].CreatureDefID == CDefID)
			return a;
	return -1;
}
*/

void CreatureDefManager :: SaveCreatureTweak(CreatureDefinition *def)
{

	char buffer[256];
	GetIndividualFilename(buffer, sizeof(buffer), def->CreatureDefID);
	FILE *output = fopen(buffer, "wb");
	if(output == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] SaveAccountToStream could not open: %s", buffer);
		return;
	}
	def->SaveToStream(output);
	fflush(output);
	fclose(output);
}

const char * CreatureDefManager :: GetIndividualFilename(char *buffer, int bufsize, int accountID)
{
	Util::SafeFormat(buffer, bufsize, "Creatures\\%08d.txt", accountID);
	Platform::FixPaths(buffer);
	return buffer;
}

int CreatureDefManager :: LoadPackages(const char *listFile)
{
	FileReader lfr;
	if(lfr.OpenText(listFile) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open Creature list file [%s]", listFile);
		return -1;
	}
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			Platform::FixPaths(lfr.DataBuffer);
			LoadFile(lfr.DataBuffer);
		}
	}
	lfr.CloseCurrent();

	/* Now load any creatures from the separate files (as written by creature tweak), replacing those we
	 * have already loaded. The should be periodically moved into the static data files.
	 */
	Platform::MakeDirectory("Creatures");
	Platform_DirectoryReader r;
	string dir = r.GetDirectory();
	r.SetDirectory("Creatures");
	r.ReadFiles();
	r.SetDirectory(dir.c_str());
	vector<std::string>::iterator it;
	char StrBuf[256] = {0};
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			Util::SafeFormat(StrBuf, sizeof(StrBuf), "Creatures/%s", p.c_str());
			LoadFile(StrBuf);
		}
	}


	return 0;
}

int CreatureDefManager :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open CreatureDef file [%s]", filename);
		return -1;
	}

	CreatureDefinition newItem;
	lfr.CommentStyle = Comment_Semi;
	char StrBuf[256] = {0};
	int count = 0;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			lfr.SingleBreak("=");
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.CreatureDefID != 0)
				{
					AddNPCDef(newItem);
					newItem.Clear();
					count++;
				}
			}
			else if(strcmp(lfr.SecBuffer, "ID") == 0)
			{
				newItem.CreatureDefID = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "defHints") == 0)
			{
				newItem.DefHints = lfr.BlockToIntC(1);
			}
			else if(strcmp(lfr.SecBuffer, "ExtraData") == 0)
			{
				ConfigString str(lfr.BlockToStringC(1, 0));
				newItem.NamedMob = str.HasKey("namedmob");
				newItem.DropRateMult = str.GetValueFloatOrDefault("dropratemult", 1);
				newItem.DropRateProfile = "";
				str.GetValueString("droprateprofile", newItem.DropRateProfile);
				for(MULTISTRING::iterator it = str.mData.begin(); it != str.mData.end(); it++) {
					STRINGLIST sl = *it;
					if(sl[0].compare("item") == 0) {
						newItem.Items.push_back(Util::GetInteger(sl[1].c_str()));
					}
				}

			}
			else if(strcmp(lfr.SecBuffer, "Effects") == 0)
			{
				//Restore to re-break
				if(lfr.BlockPos[1] > 0)
					lfr.DataBuffer[lfr.BlockPos[1] - 1] = '=';
				int params = lfr.MultiBreak("=,");

				for(int a = 1; a < params; a++)
				{
					int r = GetStatusIDByName(lfr.BlockToStringC(a, Case_Upper));
					if(r >= 0)
						newItem.DefaultEffects.push_back(r);
					else
						g_Log.AddMessageFormat("[WARNING] Unknown status effect identifier [%s] in file [%s]", lfr.SecBuffer, filename);
				}
			}
			else
			{
				strncpy(StrBuf, lfr.BlockToStringC(0, 0), sizeof(StrBuf) - 1);
				r = WriteStatToSetByName(StrBuf, lfr.BlockToStringC(1, 0), &newItem.css);
				if(r == -1)
				{
					g_Log.AddMessageFormat("[WARNING] Unknown identifier [%s] in file [%s]", StrBuf, filename);
				}
			}
		}
	}
	lfr.CloseCurrent();

	if(newItem.CreatureDefID != 0)
	{
		AddNPCDef(newItem);
		count++;
	}
	return count;
}

int CreatureDefManager :: GetSpawnList(const char *searchType, const char *searchStr, vector<int> &resultList)
{
	int filter = 0;

	if(strcmp(searchType, "Creatures") == 0)
		filter = 1;
	else if(strcmp(searchType, "Packages") == 0)
		filter = 2;
	else if(strcmp(searchType, "NoAppearanceCreatures") == 0)
		filter = 3;

	CREATURE_MAP::iterator it;
	int count = 0;
	for(it = NPC.begin(); it != NPC.end(); ++it)
	{
		bool add = false;
		switch(filter)
		{
		case 0:
			add = true;
			break;
		case 1:
			if(it->second.css.appearance.size() != 0)
				add = true;
			break;
		case 2:
			break;
		case 3:
			if(it->second.css.appearance.size() == 0)
				add = true;
			break;
		}
		if(add == true)
		{
			if(strstr(it->second.css.display_name, searchStr) != NULL)
			{
				resultList.push_back(it->second.CreatureDefID);
				count++;
				if(count >= MAX_SPAWNLIST)
					break;
			}
		}
	}
	return resultList.size();
}


void PendingCreatureUpdate :: addStatUpdate(short statID)
{
	if(statID <= 0)
		return;
	mask |= CREATURE_UPDATE_STAT;
	statUpdate.push_back(statID);
}

PendingOperation :: PendingOperation()
{
}

PendingOperation :: ~PendingOperation()
{
	Free();
}

void PendingOperation :: Free(void)
{
	DeathList.clear();
	UpdateList.clear();
}

bool PendingOperation :: Debug_HasCreature(CreatureInstance *obj)
{
	for(size_t i = 0; i < UpdateList.size(); i++)
		if(UpdateList[i].object == obj)
			return true;
	for(size_t i = 0; i < DeathList.size(); i++)
		if(DeathList[i] == obj)
			return true;
	return false;
}

void PendingOperation :: DeathList_Add(CreatureInstance *obj)
{
	for(size_t i = 0; i < DeathList.size(); i++)
		if(DeathList[i] == obj)
			return;

	DeathList.push_back(obj);
}

void PendingOperation :: DeathList_Process(void)
{
	for(size_t i = 0; i < DeathList.size(); i++)
		DeathList[i]->ProcessDeath();
	DeathList.clear();
}

void PendingOperation :: DeathList_Remove(CreatureInstance *obj)
{
	for(size_t i = 0; i < DeathList.size(); i++)
	{
		if(DeathList[i] == obj)
		{
			DeathList.erase(DeathList.begin() + i);
			return;
		}
	}
}

int PendingOperation :: UpdateList_GetExistingObject(CreatureInstance *obj)
{
	for(size_t i = 0; i < UpdateList.size(); i++)
		if(UpdateList[i].object == obj)
			return i;
	return -1;
}
void PendingOperation :: UpdateList_Add(unsigned short mask, CreatureInstance *obj, short statID)
{
	int r = UpdateList_GetExistingObject(obj);
	if(r == -1)
	{
		//Create new entry.
		PendingCreatureUpdate newItem;
		newItem.mask = mask;
		newItem.object = obj;
		newItem.addStatUpdate(statID);
		UpdateList.push_back(newItem);
	}
	else
	{
		//Entry already exists, update the flags.
		UpdateList[r].mask |= mask;
		UpdateList[r].addStatUpdate(statID);
	}
}

void PendingOperation :: UpdateList_Process(void)
{
	for(size_t a = 0; a < UpdateList.size(); a++)
	{
		if(UpdateList[a].mask & CREATURE_UPDATE_MOD)
			UpdateList[a].object->SendUpdatedBuffs();
		if(UpdateList[a].mask & CREATURE_UPDATE_STAT)
		{
			if(UpdateList[a].statUpdate.size() > 0)
			{
				CreatureInstance *ptr = UpdateList[a].object;
				int wpos = 0;
				//wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "UpdateList_Process before", INFOMSG_INFO);
				wpos += PrepExt_SendSpecificStats(&GSendBuf[wpos], ptr, UpdateList[a].statUpdate);
				//wpos += PrepExt_SendInfoMessage(&GSendBuf[wpos], "UpdateList_Process after", INFOMSG_INFO);
				ptr->actInst->LSendToLocalSimulator(GSendBuf, wpos, ptr->CurrentX, ptr->CurrentZ);
				UpdateList[a].statUpdate.clear();
			}
		}
	}

	UpdateList.clear();
}

void PendingOperation :: UpdateList_Remove(CreatureInstance *obj)
{
	int r = UpdateList_GetExistingObject(obj);
	if(r >= 0)
		UpdateList.erase(UpdateList.begin() + r);
}


int PrepExt_CreatureDef(char *buffer, CreatureDefinition *cdef)
{
	//Prepares a buffer with the data necessary to update a Creature Definition.
	//If defHints is nonzero, override the creature definition's defHint with
	//the given one.
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);  //Placeholder for size

	wpos += PutInteger(&buffer[wpos], 0);     //Leave empty so it knows it's a CreatureDef

	//Mask
	short mask = CREATURE_UPDATE_TYPE | CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	//Because (CreatureDef == 0), need defHints
	wpos += PutShort(&buffer[wpos], cdef->DefHints);     //defHints (1 = CDEF_HINT_PERSONA)
	//_ASSERT(cdef->DefHints <= 48);
	//g_Log.AddMessageFormat("CDef: %d (%s) defHints: %d", cdef->CreatureDefID, cdef->css.display_name, cdef->DefHints);

	if(g_ProtocolVersion > 26)
		wpos += PutStringUTF(&buffer[wpos], "");   //defHintsExtraData?

	//For CREATURE_UPDATE_TYPE
	wpos += PutInteger(&buffer[wpos], cdef->CreatureDefID);   //Map to creature type


	//For CREATURE_UPDATE_STAT
	int StatFlags = SUT_All;
	if(g_ProtocolVersion >= 37)
		StatFlags |= SUT_CDefExt;
	if(cdef->DefHints == 1)
		StatFlags -= SUT_NonPlayer;  //Remove NPC-only stats like aggro

	int spos = wpos;
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for number of stats
	int r = WriteCharacterStats(&cdef->css, buffer, wpos, StatFlags);
	PutShort(&buffer[spos], r);           //Write number of stats

	PutShort(&buffer[1], wpos - 3);       //Set message size

	return wpos;
}


int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, vector<short> &statList)
{
	//Prepares a buffer with the data necessary to update a Creature Definition.
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	//Mask
	short mask = CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	int size = (int)statList.size();
	wpos += PutShort(&buffer[wpos], size);

	for(int a = 0; a < size; a++)
	{
		wpos += PutShort(&buffer[wpos], statList[a]);
		wpos += WriteCurrentStatToBuffer(&buffer[wpos], statList[a], &cInst->css);
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, const short *statList, int statCount)
{
	//Prepares a buffer with the data necessary to update a Creature Definition.
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	//Mask
	short mask = CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	wpos += PutShort(&buffer[wpos], statCount);
	for(int a = 0; a < statCount; a++)
	{
		wpos += PutShort(&buffer[wpos], statList[a]);
		wpos += WriteCurrentStatToBuffer(&buffer[wpos], statList[a], &cInst->css);
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_UpdateMods(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 0x05);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);   //Update creature

	short mask = CREATURE_UPDATE_MOD | CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	if(mask & CREATURE_UPDATE_MOD)
	{
		int a;
		int modCount = (int)cInst->activeStatMod.size();
		wpos += PutShort(&buffer[wpos], modCount);
		for(a = 0; a < modCount; a++)
		{
			if(g_ProtocolVersion > 15)
			{
				int priority = cInst->activeStatMod[a].priority;
				wpos += PutInteger(&buffer[wpos], priority);  //Priority
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID

				if(priority == 1)
					wpos += PutFloat(&buffer[wpos], cInst->activeStatMod[a].clientAmount);
				else
					wpos += PutShort(&buffer[wpos], (short)cInst->activeStatMod[a].clientAmount);

			}
			else
			{
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID
				wpos += PutShort(&buffer[wpos], static_cast<short>(cInst->activeStatMod[a].clientAmount));  //amount
			}
//			wpos += PutInteger(&buffer[wpos], (int)((cInst->activeStatMod[a].expireTime - g_ServerTime) / 1000));  //duration
			wpos += PutInteger(&buffer[wpos], cInst->GetStatDurationSec(a));  //duration
			if(g_ProtocolVersion >= 24)
			{
				wpos += PutStringUTF(&buffer[wpos], ""); //Description?
			}
		}

		wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect.size());
		for(a = 0; a < (int)cInst->activeStatusEffect.size(); a++) {
			wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect[a].modStatusID);
		}
	}

	if(mask & CREATURE_UPDATE_STAT)
	{
		int spos = wpos;
		wpos += PutShort(&buffer[wpos], 0);    //Placeholder for number of stats
		int r = 0;

		for(size_t i = 0; i < cInst->baseStats.size(); i++)
		{
			wpos += PutShort(&buffer[wpos], cInst->baseStats[i].StatID);
			wpos += WriteStatToBuffer(&buffer[wpos], cInst->baseStats[i].StatID, cInst->baseStats[i].fBaseVal + cInst->baseStats[i].fModTotal);
			r++;
		}
		PutShort(&buffer[spos], r);           //Write number of stats
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_UpdateOrbs(char *buffer, CreatureInstance *cInst)
{
	//Prepares a buffer with the data necessary to update a Creature Definition.
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	//Mask
	short mask = CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	wpos += PutShort(&buffer[wpos], 4);   //All orb stats
	wpos += PutShort(&buffer[wpos], STAT::WILL);
	wpos += PutShort(&buffer[wpos], cInst->css.will);
	wpos += PutShort(&buffer[wpos], STAT::WILL_CHARGES);
	wpos += PutShort(&buffer[wpos], cInst->css.will_charges);

	wpos += PutShort(&buffer[wpos], STAT::MIGHT);
	wpos += PutShort(&buffer[wpos], cInst->css.might);
	wpos += PutShort(&buffer[wpos], STAT::MIGHT_CHARGES);
	wpos += PutShort(&buffer[wpos], cInst->css.might_charges);

	PutShort(&buffer[1], wpos - 3);       //Set message size

	return wpos;
}



int PrepExt_UpdateAppearance(char *buffer, CreatureInstance *cInst)
{
	//Prepares a buffer with the data necessary to update a Creature Definition.
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], 0);     //Leave empty so it knows it's a CreatureDef

	short mask = CREATURE_UPDATE_TYPE | CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	wpos += PutShort(&buffer[wpos], 0);     //because ID is 0, needs defHints
	if(g_ProtocolVersion > 26)
		wpos += PutStringUTF(&buffer[wpos], "");   //defHintsExtraData?

	wpos += PutInteger(&buffer[wpos], cInst->CreatureDefID);  //Need ID for CREATURE_UPDATE_TYPE


	int spos = wpos;
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for number of stats

	/* Temporary set appearance stat to the appearance on the top of the stack.
	 * We don't want to ever store this appearance so it is changed back after
	 * the update buffer has been built
	 */
	CharacterStatSet css2(cInst->css);
	css2.SetAppearance(cInst->PeekAppearance().c_str());
	css2.SetEqAppearance(cInst->PeekAppearanceEq().c_str());

	int r = WriteCharacterStats(&css2, buffer, wpos, SUT_Appearance);

	PutShort(&buffer[spos], r);           //Write number of stats

	PutShort(&buffer[1], wpos - 3);       //Set message size

	/* If creature was swimming, reset their swim state */
	if(cInst->swimming) {
		wpos += PrepExt_ModStopSwimFlag(&buffer[wpos], false);
//		wpos += PrepExt_ModStopSwimFlag(&buffer[wpos], true);
	}

	return wpos;
}

int PrepExt_CreatureInstance(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0x05);      //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);   //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);   //Update the player this time
	//g_Log.AddMessageFormat("[Debug] CreatureID: %d", cInst->CreatureID);

	short mask = CREATURE_UPDATE_TYPE | CREATURE_UPDATE_STAT | CREATURE_UPDATE_MOD;
	//short mask = CREATURE_UPDATE_STAT | CREATURE_UPDATE_MOD;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);
	//g_Log.AddMessageFormat("[Debug] Mask: %d", mask);


	if(mask & CREATURE_UPDATE_TYPE)
		wpos += PutInteger(&buffer[wpos], cInst->CreatureDefID);   //Don't need creature hints, but need an ID for the creaturedef again


	//g_Log.AddMessageFormat("[Debug] CreatureDefID: %d", cInst->CreatureDefID);

	//For CREATURE_UPDATE_MOD
	if(mask & CREATURE_UPDATE_MOD)
	{
		int a;
		int modCount = (int)cInst->activeStatMod.size();
		wpos += PutShort(&buffer[wpos], modCount);  //modCount
		for(a = 0; a < modCount; a++)
		{
			if(g_ProtocolVersion > 15)
			{
				int priority = cInst->activeStatMod[a].priority;
				wpos += PutInteger(&buffer[wpos], priority);  //Priority
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID
				if(priority == 1)
					wpos += PutFloat(&buffer[wpos], cInst->activeStatMod[a].clientAmount);
				else
					wpos += PutShort(&buffer[wpos], (short)cInst->activeStatMod[a].clientAmount);
			}
			else
			{
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID
				wpos += PutShort(&buffer[wpos], static_cast<short>(cInst->activeStatMod[a].clientAmount));  //amount
			}
//			wpos += PutInteger(&buffer[wpos], (int)((cInst->activeStatMod[a].expireTime - g_ServerTime) / 1000));  //duration
			wpos += PutInteger(&buffer[wpos], cInst->GetStatDurationSec(a));  //duration
			if(g_ProtocolVersion >= 24)
			{
				wpos += PutStringUTF(&buffer[wpos], ""); //Description?
			}
		}

		wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect.size());
		for(a = 0; a < (int)cInst->activeStatusEffect.size(); a++)
			wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect[a].modStatusID);
	}

	//For CREATURE_UPDATE_STAT
	if(mask & CREATURE_UPDATE_STAT)
	{
		int spos = wpos;
		wpos += PutShort(&buffer[wpos], 0);    //Placeholder for stats

		int StatFlags = SUT_All;
		if(cInst->serverFlags & ServerFlags::IsPlayer)
			StatFlags -= SUT_NonPlayer;  //Remove NPC-only stats like aggro

		/* Temporary set appearance stat to the appearance on the top of the stack.
		 * We don't want to ever store this appearance so it is changed back after
		 * the update buffer has been built
		 */
		CharacterStatSet css2(cInst->css);
		css2.SetAppearance(cInst->PeekAppearance().c_str());
		css2.SetEqAppearance(cInst->PeekAppearanceEq().c_str());

		int r = WriteCharacterStats(&css2, buffer, wpos, StatFlags);

		PutShort(&buffer[spos], r);           //Write number of stats
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size

	return wpos;
}

int PrepExt_CreatureFullInstance(char *buffer, CreatureInstance *cInst)
{
	TIMETRACK("PrepExt_CreatureFullInstance");
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0x05);      //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);   //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);   //Update the player this time

	short mask = CREATURE_UPDATE_TYPE | CREATURE_UPDATE_STAT | CREATURE_UPDATE_MOD | CREATURE_UPDATE_ZONE | CREATURE_UPDATE_ELEVATION | CREATURE_UPDATE_VELOCITY;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	if(mask & CREATURE_UPDATE_TYPE)
		wpos += PutInteger(&buffer[wpos], cInst->CreatureDefID);   //Don't need creature hints, but need an ID for the creaturedef again

	if(mask & CREATURE_UPDATE_ZONE)
	{
		wpos += PutStringUTF(&buffer[wpos], cInst->ZoneString);
		//g_Log.AddMessageFormat("Sending Zone [%s] for [%s]", cInst->CurrentZone, cInst->css.display_name);
		wpos += PutInteger(&buffer[wpos], cInst->CurrentX);   //X
		wpos += PutInteger(&buffer[wpos], cInst->CurrentZ);   //Z
	}
	if(mask & CREATURE_UPDATE_ELEVATION)
		wpos += PutShort(&buffer[wpos], cInst->CurrentY & 0xFFFF);

	if(mask & CREATURE_UPDATE_VELOCITY)
	{
		wpos += PutByte(&buffer[wpos], cInst->Heading);
		wpos += PutByte(&buffer[wpos], cInst->Rotation);
		wpos += PutByte(&buffer[wpos], cInst->Speed);
	}

	if(mask & CREATURE_UPDATE_MOD)
	{
		int a;
		int modCount = (int)cInst->activeStatMod.size();
		wpos += PutShort(&buffer[wpos], modCount);  //modCount
		for(a = 0; a < modCount; a++)
		{
			if(g_ProtocolVersion > 15)
			{
				int priority = cInst->activeStatMod[a].priority;
				wpos += PutInteger(&buffer[wpos], priority);  //Priority
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID
				if(priority == 1)
					wpos += PutFloat(&buffer[wpos], cInst->activeStatMod[a].clientAmount);
				else
					wpos += PutShort(&buffer[wpos], (short)cInst->activeStatMod[a].clientAmount);
			}
			else
			{
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].modStatID);  //ID
				wpos += PutShort(&buffer[wpos], cInst->activeStatMod[a].abilityID);  //Ability ID
				wpos += PutShort(&buffer[wpos], static_cast<short>(cInst->activeStatMod[a].clientAmount));  //amount
			}
//			wpos += PutInteger(&buffer[wpos], (int)((cInst->activeStatMod[a].expireTime - g_ServerTime) / 1000));  //duration
			wpos += PutInteger(&buffer[wpos], cInst->GetStatDurationSec(a));  //duration
			if(g_ProtocolVersion >= 24)
			{
				wpos += PutStringUTF(&buffer[wpos], ""); //Description?
			}
		}

		wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect.size());
		for(a = 0; a < (int)cInst->activeStatusEffect.size(); a++)
			wpos += PutShort(&buffer[wpos], cInst->activeStatusEffect[a].modStatusID);
	}

	//For CREATURE_UPDATE_STAT
	if(mask & CREATURE_UPDATE_STAT)
	{
		int StatFlags = SUT_All;
		if(g_ProtocolVersion >= 37)
			StatFlags |= SUT_CDefExt;
		if(cInst->serverFlags & ServerFlags::IsPlayer)
			StatFlags -= SUT_NonPlayer;  //Remove NPC-only stats like aggro;

		int spos = wpos;
		wpos += PutShort(&buffer[wpos], 0);    //Placeholder for stats

		/* Temporary set appearance stat to the appearance on the top of the stack.
		 * We don't want to ever store this appearance so it is changed back after
		 * the update buffer has been built
		 */
		CharacterStatSet css2(cInst->css);
		css2.SetAppearance(cInst->PeekAppearance().c_str());
		css2.SetEqAppearance(cInst->PeekAppearanceEq().c_str());

		int r = WriteCharacterStats(&css2, buffer, wpos, StatFlags);

		PutShort(&buffer[spos], r);           //Write number of stats
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size

	TIMETRACKF(1);
	return wpos;
}

int PrepExt_CreaturePos(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 0x05);
	wpos += PutShort(&buffer[wpos], 0x0000);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], CREATURE_UPDATE_ZONE);
	else
		wpos += PutShort(&buffer[wpos], CREATURE_UPDATE_ZONE);

	wpos += PutStringUTF(&buffer[wpos], cInst->ZoneString);
	wpos += PutInteger(&buffer[wpos], cInst->CurrentX);
	wpos += PutInteger(&buffer[wpos], cInst->CurrentZ);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_UpdateVelocity(char *buffer, CreatureInstance *cInst)
{
	/* This is for characters other than the player */
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], CREATURE_UPDATE_VELOCITY);
	else
		wpos += PutShort(&buffer[wpos], CREATURE_UPDATE_VELOCITY);

	wpos += PutByte(&buffer[wpos], cInst->Heading);
	wpos += PutByte(&buffer[wpos], cInst->Rotation);
	wpos += PutByte(&buffer[wpos], cInst->Speed);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_VelocityEvent(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&buffer[wpos], 0x0000);
	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);
	wpos += PutByte(&buffer[wpos], 26);

	wpos += PutByte(&buffer[wpos], cInst->Heading);
	wpos += PutByte(&buffer[wpos], cInst->Rotation);
	wpos += PutByte(&buffer[wpos], cInst->Speed);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}


int PrepExt_UpdatePosInc(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], CREATURE_UPDATE_POSITION_INC);
	else
		wpos += PutShort(&buffer[wpos], CREATURE_UPDATE_POSITION_INC);

	wpos += PutShort(&buffer[wpos], cInst->CurrentX & 0xFFFF);
	wpos += PutShort(&buffer[wpos], cInst->CurrentZ & 0xFFFF);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}


int PrepExt_UpdateEquipStats(char *buffer, CreatureInstance *cInst)
{
	//Prepares a buffer with the data necessary to update the eq_appearance stat
	int wpos = 0;

	wpos += PutByte(&buffer[wpos], 5);     //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);    //Placeholder for size

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	short mask = CREATURE_UPDATE_STAT;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	wpos += PutShort(&buffer[wpos], 13);   //Just one stat
	wpos += PutShort(&buffer[wpos], STAT::STRENGTH);
	wpos += PutShort(&buffer[wpos], cInst->css.strength);
	wpos += PutShort(&buffer[wpos], STAT::DEXTERITY);
	wpos += PutShort(&buffer[wpos], cInst->css.dexterity);
	wpos += PutShort(&buffer[wpos], STAT::CONSTITUTION);
	wpos += PutShort(&buffer[wpos], cInst->css.constitution);
	wpos += PutShort(&buffer[wpos], STAT::PSYCHE);
	wpos += PutShort(&buffer[wpos], cInst->css.psyche);
	wpos += PutShort(&buffer[wpos], STAT::SPIRIT);
	wpos += PutShort(&buffer[wpos], cInst->css.spirit);

	wpos += PutShort(&buffer[wpos], STAT::HEALTH);   //If con changed, so did max health
	if(g_Config.UseIntegerHealth == true)
		wpos += PutInteger(&buffer[wpos], cInst->css.health);
	else
		wpos += PutShort(&buffer[wpos], cInst->css.health);

	wpos += PutShort(&buffer[wpos], STAT::HEALTH_MOD);   //If con changed, so did max health
	wpos += PutShort(&buffer[wpos], cInst->css.health_mod);
	wpos += PutShort(&buffer[wpos], STAT::BASE_STATS);   //Base stats for good measure
	wpos += PutStringUTF(&buffer[wpos], cInst->css.base_stats);

	wpos += PutShort(&buffer[wpos], STAT::DAMAGE_RESIST_MELEE);
	wpos += PutShort(&buffer[wpos], cInst->css.damage_resist_melee);
	wpos += PutShort(&buffer[wpos], STAT::DAMAGE_RESIST_FIRE);
	wpos += PutShort(&buffer[wpos], cInst->css.damage_resist_fire);
	wpos += PutShort(&buffer[wpos], STAT::DAMAGE_RESIST_FROST);
	wpos += PutShort(&buffer[wpos], cInst->css.damage_resist_frost);
	wpos += PutShort(&buffer[wpos], STAT::DAMAGE_RESIST_MYSTIC);
	wpos += PutShort(&buffer[wpos], cInst->css.damage_resist_mystic);
	wpos += PutShort(&buffer[wpos], STAT::DAMAGE_RESIST_DEATH);
	wpos += PutShort(&buffer[wpos], cInst->css.damage_resist_death);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}


int PrepExt_GeneralMoveUpdate(char *buffer, CreatureInstance *cInst)
{
	//Combination of velocity, elevation and short position update.
	//Meant to emulate official server position updates for mobs.
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);
	unsigned char mask = CREATURE_UPDATE_ELEVATION | CREATURE_UPDATE_POSITION_INC | CREATURE_UPDATE_VELOCITY;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	//CREATURE_UPDATE_ELEVATION
	wpos += PutShort(&buffer[wpos], cInst->CurrentY & 0xFFFF);

	//CREATURE_UPDATE_POSITION_INC
	wpos += PutShort(&buffer[wpos], cInst->CurrentX & 0xFFFF);
	wpos += PutShort(&buffer[wpos], cInst->CurrentZ & 0xFFFF);

	//CREATURE_UPDATE_VELOCITY
	wpos += PutByte(&buffer[wpos], cInst->Heading);
	wpos += PutByte(&buffer[wpos], cInst->Rotation);
	wpos += PutByte(&buffer[wpos], cInst->Speed);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_UpdateElevation(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], CREATURE_UPDATE_ELEVATION);
	else
		wpos += PutShort(&buffer[wpos], CREATURE_UPDATE_ELEVATION);

	wpos += PutShort(&buffer[wpos], cInst->CurrentY & 0xFFFF);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}

int PrepExt_UpdateFullPosition(char *buffer, CreatureInstance *cInst)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 5);  //_handleCreatureUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);

	short mask = CREATURE_UPDATE_ZONE | CREATURE_UPDATE_ELEVATION;
	if(g_ProtocolVersion < 22)
		wpos += PutByte(&buffer[wpos], mask);
	else
		wpos += PutShort(&buffer[wpos], mask);

	//For CREATURE_UPDATE_ZONE
	wpos += PutStringUTF(&buffer[wpos], cInst->ZoneString);
	wpos += PutInteger(&buffer[wpos], cInst->CurrentX);
	wpos += PutInteger(&buffer[wpos], cInst->CurrentZ);

	//For CREATURE_UPDATE_ELEVATION
	wpos += PutShort(&buffer[wpos], cInst->CurrentY & 0xFFFF);

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}



int PrepExt_AbilityActivate(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent, bool ground)
{

	if(aevent == 4 && g_Config.DebugVerbose) {
		g_Log.AddMessageFormat("PrepExt_AbilityActivate %d : %d", cInst->CreatureID, ability->abilityID);
	}

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);   //actorID
	wpos += PutShort(&buffer[wpos], ability->abilityID);     //abId
	wpos += PutByte(&buffer[wpos], aevent);      //event

	int a;

	if(ability->bSecondary == false)
	{
		wpos += PutInteger(&buffer[wpos], ability->TargetCount);   //primary_len
		for(a = 0; a < ability->TargetCount; a++) {
			if(ability->TargetList[a] == NULL)
				g_Log.AddMessageFormat("[WARNING] Ability %d for event %d has target count of %d, but target %d is NULL.", ability->abilityID, aevent, ability->TargetCount, a);
			else
				wpos += PutInteger(&buffer[wpos], ability->TargetList[a]->CreatureID);   //targets
		}

		wpos += PutInteger(&buffer[wpos], 0);   //secondary_len
	}
	else
	{
		wpos += PutInteger(&buffer[wpos], 0);   //primary len

		wpos += PutInteger(&buffer[wpos], ability->TargetCount);   //secondary_len
		for(a = 0; a < ability->TargetCount; a++)
			wpos += PutInteger(&buffer[wpos], ability->TargetList[a]->CreatureID);   //targets
	}

	wpos += PutByte(&buffer[wpos], ground);      //has_ground
	if(ground == true)
	{
		wpos += PutFloat(&buffer[wpos], ability->x);     //ground.x
		wpos += PutFloat(&buffer[wpos], ability->y);     //ground.y
		wpos += PutFloat(&buffer[wpos], ability->z);     //ground.z
	}

	if(g_ProtocolVersion > 20)
	{
		if(aevent == AbilityStatus::ACTIVATE || aevent == AbilityStatus::CHANNELING)
		{
			wpos += PutByte(&buffer[wpos], ability->willChargesSpent);
			wpos += PutByte(&buffer[wpos], ability->mightChargesSpent);
		}
	}
	else
	{
		if(aevent == AbilityStatus::ACTIVATE)
		{
			wpos += PutByte(&buffer[wpos], ability->willChargesSpent);
			wpos += PutByte(&buffer[wpos], ability->mightChargesSpent);
		}
	}

	PutShort(&buffer[1], wpos - 3);
	return wpos;
}

int PrepExt_AbilityActivateEmpty(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent)
{
	if(aevent == 4 && g_Config.DebugVerbose) {
		g_Log.AddMessageFormat("PrepExt_AbilityActivateEmpty INTERRUPT ABILITY %d : %d", cInst->CreatureID, ability->abilityID);
	}

	//Same as AbilityActivate, but target lists and ground are always zero.
	//Used for the utility messages such as activation requests.

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 60);  //_handleAbilityActivationMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], cInst->CreatureID);   //actorID
	wpos += PutShort(&buffer[wpos], ability->abilityID);     //abId
	wpos += PutByte(&buffer[wpos], aevent);      //event

	wpos += PutInteger(&buffer[wpos], 0);   //target_len
	wpos += PutInteger(&buffer[wpos], 0);   //secondary_len
	wpos += PutByte(&buffer[wpos], 0);      //has_ground

	PutShort(&buffer[1], wpos - 3);
	return wpos;
}
