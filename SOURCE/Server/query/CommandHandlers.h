/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef COMMANDHANDLERS_H
#define COMMANDHANDLERS_H

#include "Query.h"
#include "../Globals.h"
#include <vector>

class AbstractCommandHandler : public QueryHandler {
public:
	AbstractCommandHandler(std::string usage, int requiredArgs);
	~AbstractCommandHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
	virtual int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance)=0;
	bool isAllowed(SimulatorThread *sim);
	std::string mUsage;
protected:
	std::vector<int> mAllowedPermissions;
	int mRequiredArgs;
};

class HelpHandler : public AbstractCommandHandler {
public:
	HelpHandler();
	~HelpHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpHandler : public AbstractCommandHandler {
public:
	WarpHandler();
	~WarpHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpInstanceHandler : public AbstractCommandHandler {
public:
	WarpInstanceHandler();
	~WarpInstanceHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class WarpTileHandler : public AbstractCommandHandler {
public:
	WarpTileHandler();
	~WarpTileHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpPullHandler : public AbstractCommandHandler {
public:
	WarpPullHandler();
	~WarpPullHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpGroveHandler : public AbstractCommandHandler {
public:
	WarpGroveHandler();
	~WarpGroveHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpExternalOfflineHandler : public AbstractCommandHandler {
public:
	WarpExternalOfflineHandler();
	~WarpExternalOfflineHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WarpExternalHandler : public AbstractCommandHandler {
public:
	WarpExternalHandler();
	~WarpExternalHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};


class AdjustExpHandler : public AbstractCommandHandler {
public:
	AdjustExpHandler();
	~AdjustExpHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class UnstickHandler : public AbstractCommandHandler {
public:
	UnstickHandler();
	~UnstickHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PoseHandler : public AbstractCommandHandler {
public:
	PoseHandler();
	~PoseHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class Pose2Handler : public AbstractCommandHandler {
public:
	Pose2Handler();
	~Pose2Handler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class EsayHandler : public AbstractCommandHandler {
public:
	EsayHandler();
	~EsayHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class HealthHandler : public AbstractCommandHandler {
public:
	HealthHandler();
	~HealthHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SpeedHandler : public AbstractCommandHandler {
public:
	SpeedHandler();
	~SpeedHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ForceAbilityHandler : public AbstractCommandHandler {
public:
	ForceAbilityHandler();
	~ForceAbilityHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PartyLowestHandler : public AbstractCommandHandler {
public:
	PartyLowestHandler();
	~PartyLowestHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WhoHandler : public AbstractCommandHandler {
public:
	WhoHandler();
	~WhoHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ListShardsHandler : public AbstractCommandHandler {
public:
	ListShardsHandler();
	~ListShardsHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GMWhoHandler : public AbstractCommandHandler {
public:
	GMWhoHandler();
	~GMWhoHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CHWhoHandler : public AbstractCommandHandler {
public:
	CHWhoHandler();
	~CHWhoHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GiveHandler : public AbstractCommandHandler {
public:
	GiveHandler();
	~GiveHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GiveIDHandler : public AbstractCommandHandler {
public:
	GiveIDHandler();
	~GiveIDHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GiveAllHandler : public AbstractCommandHandler {
public:
	GiveAllHandler();
	~GiveAllHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GiveAppHandler : public AbstractCommandHandler {
public:
	GiveAppHandler();
	~GiveAppHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DeleteAllHandler : public AbstractCommandHandler {
public:
	DeleteAllHandler();
	~DeleteAllHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DeleteAboveHandler : public AbstractCommandHandler {
public:
	DeleteAboveHandler();
	~DeleteAboveHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GroveHandler : public AbstractCommandHandler {
public:
	GroveHandler();
	~GroveHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PVPHandler : public AbstractCommandHandler {
public:
	PVPHandler();
	~PVPHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CompleteHandler : public AbstractCommandHandler {
public:
	CompleteHandler();
	~CompleteHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class RefashionHandler : public AbstractCommandHandler {
public:
	RefashionHandler();
	~RefashionHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class BackupHandler : public AbstractCommandHandler {
public:
	BackupHandler();
	~BackupHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class RestoreHandler : public AbstractCommandHandler {
public:
	RestoreHandler();
	~RestoreHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GodHandler : public AbstractCommandHandler {
public:
	GodHandler();
	~GodHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetStatHandler : public AbstractCommandHandler {
public:
	SetStatHandler();
	~SetStatHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScaleHandler : public AbstractCommandHandler {
public:
	ScaleHandler();
	~ScaleHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_command_scale(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PartyAllHandler : public AbstractCommandHandler {
public:
	PartyAllHandler();
	~PartyAllHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PartyQuitHandler : public AbstractCommandHandler {
public:
	PartyQuitHandler();
	~PartyQuitHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CCCHandler : public AbstractCommandHandler {
public:
	CCCHandler();
	~CCCHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class BanHandler : public AbstractCommandHandler {
public:
	BanHandler();
	~BanHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class UnbanHandler : public AbstractCommandHandler {
public:
	UnbanHandler();
	~UnbanHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetPermissionHandler : public AbstractCommandHandler {
public:
	SetPermissionHandler();
	~SetPermissionHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetBuildPermissionHandler : public AbstractCommandHandler {
public:
	SetBuildPermissionHandler();
	~SetBuildPermissionHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetPermissionCHandler : public AbstractCommandHandler {
public:
	SetPermissionCHandler();
	~SetPermissionCHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetBehaviorHandler : public AbstractCommandHandler {
public:
	SetBehaviorHandler();
	~SetBehaviorHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DeriveSetHandler : public AbstractCommandHandler {
public:
	DeriveSetHandler();
	~DeriveSetHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class IGStatusHandler : public AbstractCommandHandler {
public:
	IGStatusHandler();
	~IGStatusHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PartyZapHandler : public AbstractCommandHandler {
public:
	PartyZapHandler();
	~PartyZapHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PartyInviteHandler : public AbstractCommandHandler {
public:
	PartyInviteHandler();
	~PartyInviteHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class RollHandler : public AbstractCommandHandler {
public:
	RollHandler();
	~RollHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ForumLockHandler : public AbstractCommandHandler {
public:
	ForumLockHandler();
	~ForumLockHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ZoneNameHandler : public AbstractCommandHandler {
public:
	ZoneNameHandler();
	~ZoneNameHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DtrigHandler : public AbstractCommandHandler {
public:
	DtrigHandler();
	~DtrigHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SdiagHandler : public AbstractCommandHandler {
public:
	SdiagHandler();
	~SdiagHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SpingHandler : public AbstractCommandHandler {
public:
	SpingHandler();
	~SpingHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class InfoHandler : public AbstractCommandHandler {
public:
	InfoHandler();
	~InfoHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GroveSettingHandler : public AbstractCommandHandler {
public:
	GroveSettingHandler();
	~GroveSettingHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GrovePermissionsHandler : public AbstractCommandHandler {
public:
	GrovePermissionsHandler();
	~GrovePermissionsHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DngScaleHandler : public AbstractCommandHandler {
public:
	DngScaleHandler();
	~DngScaleHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PathLinksHandler : public AbstractCommandHandler {
public:
	PathLinksHandler();
	~PathLinksHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class TargHandler : public AbstractCommandHandler {
public:
	TargHandler();
	~TargHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ElevHandler : public AbstractCommandHandler {
public:
	ElevHandler();
	~ElevHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CycleHandler : public AbstractCommandHandler {
public:
	CycleHandler();
	~CycleHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class TimeHandler : public AbstractCommandHandler {
public:
	TimeHandler();
	~TimeHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SearSizeHandler : public AbstractCommandHandler {
public:
	SearSizeHandler();
	~SearSizeHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class StailSizeHandler : public AbstractCommandHandler {
public:
	StailSizeHandler();
	~StailSizeHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DailyHandler : public AbstractCommandHandler {
public:
	DailyHandler();
	~DailyHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptExecHandler : public AbstractCommandHandler {
public:
	ScriptExecHandler();
	~ScriptExecHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptTimeHandler : public AbstractCommandHandler {
public:
	ScriptTimeHandler();
	~ScriptTimeHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptWakeVMHandler : public AbstractCommandHandler {
public:
	ScriptWakeVMHandler();
	~ScriptWakeVMHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptGCHandler : public AbstractCommandHandler {
public:
	ScriptGCHandler();
	~ScriptGCHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ScriptClearQueueHandler : public AbstractCommandHandler {
public:
	ScriptClearQueueHandler();
	~ScriptClearQueueHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class RotHandler : public AbstractCommandHandler {
public:
	RotHandler();
	~RotHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PVPTeamHandler : public AbstractCommandHandler {
public:
	PVPTeamHandler();
	~PVPTeamHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PVPModeHandler : public AbstractCommandHandler {
public:
	PVPModeHandler();
	~PVPModeHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class InstanceHandler : public AbstractCommandHandler {
public:
	InstanceHandler();
	~InstanceHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class UserAuthResetHandler : public AbstractCommandHandler {
public:
	UserAuthResetHandler();
	~UserAuthResetHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class MaintainHandler : public AbstractCommandHandler {
public:
	MaintainHandler();
	~MaintainHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AchievementsHandler : public AbstractCommandHandler {
public:
	AchievementsHandler();
	~AchievementsHandler() {};
	int handleCommand(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
