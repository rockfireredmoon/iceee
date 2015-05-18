// Handles remote actions sent to the server through any kind of external control,
// such as HTTP POST.

#ifndef REMOTEACTION_H
#define REMOTEACTION_H

#include "Components.h"
#include "SocketClass3.h"
#include <string>
#include <vector>
#include "Report.h"

typedef std::vector<std::string> STRINGLIST;
typedef std::vector<std::vector<std::string> > MULTISTRING;

class CreatureInstance;

namespace Report
{
	void RefreshThreads(ReportBuffer &report);
	void RefreshTime(ReportBuffer &report);
	int GetSimulatorID(const char *strID);
	void RefreshMods(ReportBuffer &report, const char *simID);
	void RefreshPlayers(ReportBuffer &report);
	void Helper_OutputCreature(ReportBuffer &report, int index, CreatureInstance *obj);
	void RefreshInstance(ReportBuffer &report);
	void RefreshScripts(ReportBuffer &report);
	void RefreshHateProfile(ReportBuffer &report);
	void RefreshCharacter(ReportBuffer &report);
	void RefreshSim(ReportBuffer &report);
	void RefreshProfiler(ReportBuffer &report);
	void RefreshItem(ReportBuffer &report, const char *simID);
	void RefreshItemDetailed(ReportBuffer &report, const char *simID);
	void RefreshPacket(ReportBuffer &report);
}

char *GetDataSizeStr(long value);

int RunRemoteAction(ReportBuffer &report, MULTISTRING &header, MULTISTRING &params);
int RunAccountCreation(MULTISTRING &params);
int RunPasswordReset(MULTISTRING &params);
int RunAccountRecover(MULTISTRING &params);

enum RemoteStatus
{
	REMOTE_COMPLETE      = 0,    //The operation was complete, no further action required.
	REMOTE_REPORT        = 1,    //A report has been generated.
	REMOTE_INVALIDPOST   = -1,   //The HTTP POST is missing required information.
	REMOTE_AUTHFAILED    = -2,   //Authentication failed.  Request denied.
	REMOTE_HANDLER       = -3,   //There is no handler to process that request.
	REMOTE_FAILED        = -4    //Generic failure message.
};


#endif //REMOTEACTION_H
