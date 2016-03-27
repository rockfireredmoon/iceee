#ifndef AISCRIPT2_H
#define AISCRIPT2_H

#include <list>

#include "ScriptCore.h"
#include "InstanceScript.h"

class CreatureInstance;
class ReportBuffer;

class AINutDef: public ScriptCore::NutDef {

public:
	virtual ~AINutDef();

};

class AINutPlayer: public InstanceScript::InstanceNutPlayer {
public:
	CreatureInstance *attachedCreature;

	AINutPlayer();
	virtual ~AINutPlayer();

	CreatureInstance *ResolveCreatureInstance(int CreatureInstanceID);
	virtual void RegisterFunctions();
	void DebugGenerateReport(ReportBuffer &report);
	void RegisterAIFunctions(NutPlayer *instance, Sqrat::DerivedClass<AINutPlayer, InstanceScript::InstanceNutPlayer> *clazz);
	virtual void HaltDerivedExecution();
	virtual void HaltedDerived();
	void Initialize(CreatureInstance *creature, AINutDef *defPtr, std::string &errors);

	// Exposed to scripts
//	void Queue(Sqrat::Function function, int fireDelay);
	bool HasTarget();
	void Use(int abilityID);
	short GetWill();
	short GetWillCharge();
	short GetMight();
	short GetMightCharge();
	short GetLevel();
	bool IsOnCooldown(const char *category);
	bool IsBusy();
	short CountEnemyNear(int range);
	short CountEnemyAt(int range);
	short HealthPercent();
	short TargetHealthPercent();
	void VisualEffect(const char *effect);
	void TargetVisualEffect(const char *effect);
	void Say(const char *effect);
	void InstanceCall(const char *functionName);
	int GetIdleMob(int CDefID);
	int GetTarget();
	int GetSelf();
	int GetSelfDefID();
	void SetOtherTarget(int CID, int targetCID);
	bool IsTargetEnemy();
	bool IsTargetFriendly();
	void SetSpeed(int speed);
	int GetTargetCDefID();
	int GetProperty(const char *name);
	int GetTargetProperty(const char *name);
	void DispelTargetProperty(const char *name, int sign);
	int FindCID(int CDefID);
	void PlaySound(const char *name);
	int GetBuffTier(int abilityGroupID);
	int GetTargetBuffTier(int abilityGroupID);
	bool IsTargetInRange(float distance);
	int GetTargetRange();
	void SetGTAE();
	int GetSpeed(int CID);
	bool IsCIDBusy(int CID);
	void Speak(const char *message);
};

class AINutManager
{
public:
	AINutManager();
	~AINutManager();
	std::list<AINutDef*> aiDef;
	std::list<AINutPlayer*> aiAct;
	int LoadScripts(void);
	AINutDef* GetScriptByName(const char *name);
	AINutPlayer *AddActiveScript(CreatureInstance *creature, AINutDef *def, std::vector<std::string> args, std::string &errors);
	void RemoveActiveScript(AINutPlayer *registeredPtr);
};


class UseCallback : public ScriptCore::NutCallback
{
public:
	int mAbilityID;
	AINutPlayer* mAiNut;

	UseCallback(AINutPlayer *aiNut, int abilityID);
	~UseCallback();
	bool Execute();
};

extern AINutManager aiNutManager;

#endif
