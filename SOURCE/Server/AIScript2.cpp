#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "AIScript2.h"
#include "FileReader.h"
#include "StringList.h"
#include "Instance.h"
#include "Creature.h"
#include "Ability2.h"
#include "Util.h"
#include "DirectoryAccess.h"
#include "Report.h"
#include "Config.h"
#include "DirectoryAccess.h"

const int USE_FAIL_DELAY = 250;  //Milliseconds to wait before retrying a failed script "use" command.


AINutManager aiNutManager;


AINutDef::~AINutDef()
{
}

UseCallback::UseCallback(AINutPlayer *aiNut, int abilityID) {
	mAiNut = aiNut;
	mAbilityID = abilityID;
}
UseCallback::~UseCallback() {
}

void UseCallback::Execute() {
	mAiNut->Use(mAbilityID);
}

AINutPlayer::AINutPlayer()
{
	attachedCreature = NULL;
}

AINutPlayer::~AINutPlayer()
{
}

void AINutPlayer::HaltDerivedExecution()
{
}

bool AINutPlayer::HasTarget() {
	return attachedCreature->CurrentTarget.targ != NULL;
}

void AINutPlayer::Use(int abilityID) {

	if(attachedCreature->ab[0].bPending == false)
	{
		//DEBUG OUTPUT
		if(g_Config.DebugLogAIScriptUse == true)
		{
			const Ability2::AbilityEntry2* abptr = g_AbilityManager.GetAbilityPtrByID(abilityID);
			g_Log.AddMessageFormat("Using: %s", abptr->GetRowAsCString(Ability2::ABROW::NAME));
		}
		//END DEBUG OUTPUT

		int r = attachedCreature->CallAbilityEvent(abilityID, EventType::onRequest);
		if(r != 0)
		{
			//Notify the creature we failed, may need a distance check.
			//The script should wait and retry soon.
			attachedCreature->AICheckAbilityFailure(r);

			if(g_Config.DebugLogAIScriptUse == true)
			{
				const Ability2::AbilityEntry2* abptr = g_AbilityManager.GetAbilityPtrByID(abilityID);
				g_Log.AddMessageFormat("Using: %s   Failed: %d", abptr->GetRowAsCString(Ability2::ABROW::NAME), g_AbilityManager.GetAbilityErrorCode(r));
			}

			if(attachedCreature->AIAbilityFailureAllowRetry(r) == true)
			{
				UseCallback *cb = new UseCallback(this, abilityID);
				AINutPlayer::DoQueue(ScriptCore::NutScriptEvent(cb, USE_FAIL_DELAY));
			}
		}
	}
}

void AINutPlayer::RegisterFunctions() {
	Sqrat::Class<NutPlayer> nutClass(vm, _SC("Core"), true);
	Sqrat::RootTable(vm).Bind(_SC("Core"), nutClass);
	RegisterCoreFunctions(this, &nutClass);
	Sqrat::DerivedClass<AINutPlayer, NutPlayer> aiClass(vm, _SC("AI"));
	Sqrat::RootTable(vm).Bind(_SC("AI"), aiClass);
	RegisterAIFunctions(this, &aiClass);
	Sqrat::RootTable(vm).SetInstance(_SC("ai"), this);
}

void AINutPlayer::RegisterAIFunctions(NutPlayer *instance, Sqrat::Class<AINutPlayer> *clazz)
{
	/* Have to register the functions with THIS class, or the wrong instance will be
	 * invoked from Squirrel
	 *
	 * TODO it might be ok to move these 3 back to the core. Seems my theory was wrong.
	 * Leave them here till the actual cause is found
	 */
	clazz->Func(_SC("queue"), &AINutPlayer::Queue);
	clazz->Func(_SC("broadcast"), &AINutPlayer::Broadcast);
	clazz->Func(_SC("halt"), &AINutPlayer::Halt);

	clazz->Func(_SC("has_target"), &AINutPlayer::HasTarget);
	clazz->Func(_SC("use"), &AINutPlayer::Use);

//	Sqrat::Class<NutPlayer> aiClass(vm, "AI", true);
//	Sqrat::RootTable(vm).Bind(_SC("AI"), aiClass);
//	aiClass.Func(_SC("has_target"), &AINutPlayer::HasTarget);
//	aiClass.Func(_SC("use"), &AINutPlayer::Use);
//	Sqrat::RootTable(vm).SetInstance(_SC("ai"), instance);

	// Instance Location Object, X1/Z1,X2/Z2 location defining a rectangle
//	Sqrat::Class<ScriptObjects::Area> areaClass(vm, "Area", true);
//	areaClass.Ctor<int,int,int,int>();
//	areaClass.Ctor();
//	Sqrat::RootTable(vm).Bind(_SC("Area"), areaClass);
//	areaClass.Var("x1", &ScriptObjects::Area::mX1);
//	areaClass.Var("x2", &ScriptObjects::Area::mX2);
//	areaClass.Var("y1", &ScriptObjects::Area::mY1);
//	areaClass.Var("y2", &ScriptObjects::Area::mY2);
//
//	// Point Object, X/Z location
//	Sqrat::Class<ScriptObjects::Point> pointClass(vm, "Point", true);
//	pointClass.Ctor<int,int>();
//	pointClass.Ctor();
//	Sqrat::RootTable(vm).Bind(_SC("Point"), pointClass);
//	pointClass.Var("x", &ScriptObjects::Point::mX);
//	pointClass.Var("z", &ScriptObjects::Point::mZ);
//
//	Sqrat::Class<InstanceNutPlayer> instance(vm, "Instance", true);
//	Sqrat::RootTable(vm).Bind(_SC("Instance"), instance);
//
//	instance.Func(_SC("broadcast"), &InstanceNutPlayer::Broadcast);
//	instance.Func(_SC("info"), &InstanceNutPlayer::Info);
//	instance.Func(_SC("spawn"), &InstanceNutPlayer::Spawn);
//	instance.Func(_SC("spawnAt"), &InstanceNutPlayer::SpawnAt);
//	instance.Func(_SC("cdef"), &InstanceNutPlayer::CDefIDForCID);
//	instance.Func(_SC("scanForNPCByCDefID"), &InstanceNutPlayer::ScanForNPCByCDefID);
//	instance.Func(_SC("getTarget"), &InstanceNutPlayer::GetTarget);
//	instance.Func(_SC("setTarget"), &InstanceNutPlayer::SetTarget);
//	instance.Func(_SC("ai"), &InstanceNutPlayer::AI);
//	instance.Func(_SC("countAlive"), &InstanceNutPlayer::CountAlive);
//	instance.Func(_SC("healthPercent"), &InstanceNutPlayer::GetHealthPercent);
//	instance.Func(_SC("walk"), &InstanceNutPlayer::Walk);
//	instance.Func(_SC("walkThen"), &InstanceNutPlayer::WalkThen);
//	instance.Func(_SC("chat"), &InstanceNutPlayer::Chat);
//	instance.Func(_SC("creatureChat"), &InstanceNutPlayer::CreatureChat);
//	instance.Func(_SC("despawn"), &InstanceNutPlayer::Despawn);
//	instance.Func(_SC("despawnAll"), &InstanceNutPlayer::DespawnAll);
//	instance.Func(_SC("particleAttach"), &InstanceNutPlayer::ParticleAttach);
//	instance.Func(_SC("particleDetach"), &InstanceNutPlayer::DetachSceneryEffect);
//	instance.Func(_SC("asset"), &InstanceNutPlayer::Asset);
//	instance.Func(_SC("emote"), &InstanceNutPlayer::Emote);
//
//	// Functions that return arrays or tables have to be dealt with differently
//	instance.SquirrelFunc(_SC("cids"), &InstanceNutPlayer::CIDs);
//
//	// Some constants
//	Sqrat::ConstTable(vm).Const(_SC("CREATURE_WALK_SPEED"), CREATURE_WALK_SPEED);
//	Sqrat::ConstTable(vm).Const(_SC("CREATURE_JOG_SPEED"), CREATURE_JOG_SPEED);
//	Sqrat::ConstTable(vm).Const(_SC("CREATURE_RUN_SPEED"), CREATURE_RUN_SPEED);
//
//	Sqrat::RootTable(vm).SetInstance(_SC("inst"), this);
}

CreatureInstance* AINutPlayer :: ResolveCreatureInstance(int CreatureInstanceID)
{
	if(attachedCreature == NULL)
		return NULL;
	if(attachedCreature->actInst == NULL)
		return NULL;
	return attachedCreature->actInst->GetInstanceByCID(CreatureInstanceID);
}

void AINutPlayer :: DebugGenerateReport(ReportBuffer &report)
{
	if(def != NULL)
	{
		report.AddLine("Name:%s", def->scriptName.c_str());
	}
	report.AddLine("active:%d", static_cast<int>(active));
	report.AddLine(NULL);
}

AINutManager :: AINutManager()
{
}

AINutManager :: ~AINutManager()
{
	aiDef.clear();
	aiAct.clear();
}

int AINutManager :: LoadScripts(void)
{
	Platform_DirectoryReader r;
	string dir = r.GetDirectory();
	r.SetDirectory("AIScript");
	r.ReadFiles();
	r.SetDirectory(dir.c_str());

	vector<std::string>::iterator it;
	AINutDef def;
	for(it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if(Util::HasEnding(p, ".nut")) {
			aiDef.push_back(def);
			char buf[128];
			Util::SafeFormat(buf, sizeof(buf), "AIScript/%s", p.c_str());
			Platform::FixPaths(buf);
			aiDef.back().Initialize(buf);
		}
	}

	return 0;
}

AINutDef * AINutManager :: GetScriptByName(const char *name)
{
	if(aiDef.size() == 0)
		return NULL;
	list<AINutDef>::iterator it;
	for(it = aiDef.begin(); it != aiDef.end(); ++it)
	{
		if(it->scriptName.compare(name) == 0)
			return &*it;
	}
	return NULL;
}

void AINutPlayer::Queue(Sqrat::Function function, int fireDelay)
{
	DoQueue(function, fireDelay);
}

AINutPlayer * AINutManager :: AddActiveScript(const char *name)
{
	AINutDef *def = GetScriptByName(name);
	if(def == NULL)
		return NULL;

	AINutPlayer player;
	std::string errors;
	player.Initialize(def, errors);
	if(errors.length() > 0)
		g_Log.AddMessageFormat("Failed to compile %s. %s",def->scriptName.c_str(), errors.c_str());
	aiAct.push_back(player);
	return &aiAct.back();
}

void AINutManager :: RemoveActiveScript(AINutPlayer *registeredPtr)
{
	list<AINutPlayer>::iterator it;
	for(it = aiAct.begin(); it != aiAct.end(); ++it)
	{
		if(&*it == registeredPtr)
		{
			aiAct.erase(it);
			break;
		}
	}
}

