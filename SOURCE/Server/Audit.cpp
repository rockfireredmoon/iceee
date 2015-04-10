#include "Audit.h"
#include "Util.h"
#include "Config.h"

bool ZoneAudit::mFolderCreated = false;

SceneryAudit::SceneryAudit()
{
	mLastEditTime = 0;
	mOpType = OP_NONE;
}

void SceneryAudit::UpdateAudit(const char *username, const SceneryObject *propPtr, int opType)
{
	if(propPtr == NULL)
		return;

	if(mOpType == OP_NONE)
		mOpType = opType;
	mUsername = username;
	mLastEditTime = g_ServerTime;
	mObject.copyFrom(propPtr);
}
void SceneryAudit::WriteToFile(FILE *output) const
{
	const char *opTypeLabel = "NONE";
	switch(mOpType)
	{
	case OP_NEW: opTypeLabel = "NEW"; break;
	case OP_EDIT: opTypeLabel = "EDIT"; break;
	case OP_DELETE: opTypeLabel = "DELETE"; break;
	}
	fprintf(output, "@AUDIT:user=%s&type=%s&ID=%d\r\n", mUsername.c_str(), opTypeLabel, mObject.ID);
	mObject.WriteToStream(output);
}

bool SceneryAudit::IsAuditReady(void) const
{
	if(g_ServerTime - mLastEditTime > (unsigned long)g_Config.SceneryAuditDelay)
		return true;
	
	return false;
}

ZoneAudit::ZoneAudit(void)
{
	mZone = 0;
}

void ZoneAudit::PerformSceneryAudit(const char *username, int zone, const SceneryObject *sceneryObject, int opType)
{
	if(mZone != zone)
		mZone = zone;

	if(sceneryObject == NULL)
		return;

	std::list<SceneryAudit> &obj = mSceneryAudit[sceneryObject->ID];
	std::list<SceneryAudit>::iterator it;

	for(it = obj.begin(); it != obj.end(); ++it)
	{
		if(it->mOpType == opType)
		{
			it->UpdateAudit(username, sceneryObject, opType);
			return;
		}
	}

	//Important: Scenery objects contain pointer data, so we need to push_back() a default empty prop.
	//Using push_back() will duplicate pointer data, causing double free crashes when the destructors are
	//called.
	size_t currentSize = obj.size();
	obj.push_back(SceneryAudit());
	if(obj.size() != currentSize)
	{
		obj.back().UpdateAudit(username, sceneryObject, opType);
	}
}

void ZoneAudit::CreateAuditFolder(void)
{
	if(mFolderCreated == false)
	{
		Platform::MakeDirectory("Audit");
		mFolderCreated = true;
	}
}

void ZoneAudit::AutosaveAudits(void)
{
	FILE *output = NULL;

	std::map<int, std::list<SceneryAudit> >::iterator propIt;
	
	propIt = mSceneryAudit.begin();
	while(propIt != mSceneryAudit.end())
	{
		if(IsAuditListReady(propIt->second) == true)
		{
			if(output == NULL)
			{
				char fileNameBuf[64];
				Util::SafeFormat(fileNameBuf, sizeof(fileNameBuf), "Scenery_Z_%d.txt", mZone);
				std::string path;
				Platform::GenerateFilePath(path, "Audit", fileNameBuf);

				Platform::MakeDirectory("Audit");
				output = fopen(path.c_str(), "a");
				
				if(output == NULL)
					return;  //File error? abort.
			}
			if(output != NULL)
				WriteAuditList(propIt->second, output);

			propIt->second.clear();
			mSceneryAudit.erase(propIt++);
		}
		else
		{
			++propIt;
		}
	}
	if(output != NULL)
		fclose(output);
}

bool ZoneAudit::IsAuditListReady(const std::list<SceneryAudit> &sceneryAudits)
{
	std::list<SceneryAudit>::const_iterator it;
	for(it = sceneryAudits.begin(); it != sceneryAudits.end(); ++it)
	{
		if(it->IsAuditReady() == false)
			return false;
	}
	return true;
}

void ZoneAudit::WriteAuditList(const std::list<SceneryAudit> &sceneryAudits, FILE *output)
{
	std::list<SceneryAudit>::const_iterator it;
	for(it = sceneryAudits.begin(); it != sceneryAudits.end(); ++it)
	{
		it->WriteToFile(output);
	}
}

void ZoneAudit::Clear(void)
{
	mSceneryAudit.clear();
	mZone = 0;
}


/*
SceneryAuditManager::SceneryAuditManager()
{
	mFolderCreated = false;
}

void SceneryAuditManager::PerformSceneryAudit(const char *username, int zone, const SceneryObject *sceneryObject, int opType)
{
	CreateFolder();
	mZoneAudit[zone].PerformSceneryAudit(username, zone, sceneryObject, opType);
}

void SceneryAuditManager::CreateFolder(void)
{
	if(mFolderCreated == true)
		return;

	Platform::MakeDirectory("Audit");
	mFolderCreated = true;
}
*/
