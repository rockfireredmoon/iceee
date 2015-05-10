#include <list>

#include "ScriptCore.h"

class CreatureInstance;
class ReportBuffer;

class AINutDef: public ScriptCore::NutDef {

public:
	virtual ~AINutDef();

};

class AINutPlayer: public ScriptCore::NutPlayer {
public:
	CreatureInstance *attachedCreature;

	AINutPlayer();
	~AINutPlayer();

	CreatureInstance *ResolveCreatureInstance(int CreatureInstanceID);
	virtual void RegisterFunctions();
	void DebugGenerateReport(ReportBuffer &report);
	void RegisterAIFunctions(NutPlayer *instance, Sqrat::Class<AINutPlayer> *clazz);
	bool HasTarget();
	void Use(int abilityID);
	void HaltDerivedExecution();

	// Exposed to scripts
	void Queue(Sqrat::Function function, int fireDelay);

};

class AINutManager
{
public:
	AINutManager();
	~AINutManager();
	std::list<AINutDef> aiDef;
	std::list<AINutPlayer> aiAct;
	int LoadScripts(void);
	AINutDef* GetScriptByName(const char *name);
	AINutPlayer *AddActiveScript(const char *name);
	void RemoveActiveScript(AINutPlayer *registeredPtr);
};


class UseCallback : public ScriptCore::NutCallback
{
public:
	int mAbilityID;
	AINutPlayer* mAiNut;

	UseCallback(AINutPlayer *aiNut, int abilityID);
	~UseCallback();
	void Execute();
};

extern AINutManager aiNutManager;
