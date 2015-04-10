// Tracks data changes and logs them.
#ifndef AUDIT_H
#define AUDIT_H

#include "Scenery2.h"
#include <vector>
#include <string>
#include <map>

class SceneryAudit
{
public:
	enum
	{
		OP_NONE = 0,
		OP_NEW,
		OP_EDIT,
		OP_DELETE,
	};
	SceneryObject mObject;           //A copy of the prop as it exists with the current edit.
	std::string mUsername;
	unsigned long mLastEditTime;     //Server time, in milliseconds, that the object was last edited.
	int mOpType;                     //Corresponds to enum above.

	SceneryAudit();
	void UpdateAudit(const char *username, const SceneryObject *propPtr, int opType);
	void WriteToFile(FILE *output) const;
	bool IsAuditReady(void) const;
};

class ZoneAudit
{
	friend class ZoneDefInfo;  //Only intended to be managed or used within this class.

public:

private:
	ZoneAudit();
	std::map<int, std::list<SceneryAudit> > mSceneryAudit; //Map a prop ID to a list of changes on that prop.  Important!  Lists are used because the allocated object must be static in memory.  Vector reallocation invalidates the pointer used by SceneryObject.
	int mZone;
	static bool mFolderCreated;

	void PerformSceneryAudit(const char *username, int zone, const SceneryObject *sceneryObject, int opType);
	void CreateAuditFolder(void);
	void AutosaveAudits(void);
	bool IsAuditListReady(const std::list<SceneryAudit> &sceneryAudits);
	void WriteAuditList(const std::list<SceneryAudit> &sceneryAudits, FILE *output);
	void Clear(void);
};

/*
class SceneryAuditManager
{
public:
	SceneryAuditManager();
	void PerformSceneryAudit(const char *username, int zone, const SceneryObject *sceneryObject, int opType);
private:
	std::map<int, ZoneAudit> mZoneAudit;   //Map a zone ID to a handler for audits in that zone.
	bool mFolderCreated;
	void CreateFolder(void);
};
*/

#endif //#ifndef AUDIT_H