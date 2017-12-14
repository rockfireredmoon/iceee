
#include "Scenery2.h"
#include "FileReader3.h"
#include "Util.h"
#include "ZoneDef.h"
#include "Config.h" //Need for loading the strings file.
#include "DebugProfiler.h"
#include "Globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <algorithm>
#include "util/Log.h"

SceneryManager g_SceneryManager;
GlobalSceneryVars g_SceneryVars;

namespace ActiveLocation
{
	bool IsLocationInUse(const CONTAINER& source)
	{
		if(source.size() > 0)
			return true;

		return false;
	}
}

SceneryObject :: SceneryObject(const SceneryObject &so)
{
	extraData = NULL;
	Clear();
	copyFrom(&so);
}

SceneryObject :: SceneryObject()
{
	//Important, need to explicitly initialize as NULL before clear is called, or
	//else Clear() will try to wipe data at an undefined location.
	extraData = NULL;
	Clear();
}

SceneryObject :: ~SceneryObject()
{
	Destroy();
}

void SceneryObject::Destroy(void)
{
	if(extraData != NULL)
	{
		delete extraData;
		extraData = NULL;
	}
}

void SceneryObject :: Clear(void)
{
	ID = 0;
	Util::ClearString(Asset, sizeof(Asset));
	Util::ClearString(Name, sizeof(Name));


	LocationX = 0.0F;
	LocationY = 0.0F;
	LocationZ = 0.0F;

	QuatX = 0.0F;
	QuatY = 0.0F;
	QuatZ = 0.0F;
	QuatW = 0.0F;

	ScaleX = 0.0F;
	ScaleY = 0.0F;
	ScaleZ = 0.0F;

	Flags = 0;

	Layer = 0;
	patrolSpeed = 0;

	Util::ClearString(patrolEvent, sizeof(patrolEvent));

	//The loading functions use a single object clear it to prepare it for a new entry.
	//We want to wipe the extra data, if applicable, but keep it attached to the object
	//so it doesn't have to reallocate.
	if(extraData != NULL)
		extraData->Clear();
}

int SceneryObject :: SetName(const char *buffer)
{
	//Sets the name from a string by copying the contents of the buffer into
	//the Name member.
	memset(Name, 0, sizeof(Name));

	size_t ToCopy = strlen(buffer);
	if(ToCopy > sizeof(Name) - 1)
	{
		ToCopy = sizeof(Name) - 1;
		g_Logs.server->warn("Asset internal name clipped");
	}
	strncpy(Name, buffer, ToCopy);
	return 0;
}

int SceneryObject :: SetAsset(const char *buffer)
{
	//Sets the name from a string by copying the contents of the buffer into
	//the Asset member.

	memset(Asset, 0, sizeof(Asset));
	if(strlen(buffer) > sizeof(Asset) - 1)
		g_Logs.server->warn("Asset resource name clipped");
	Util::SafeCopy(Asset, buffer, sizeof(Asset));

	//Debugging, empty assets cause permanent loading screens
	if(buffer[0] == 0)
		g_Logs.server->error("SetAsset() new data is empty");
	if(Asset[0] == 0)
		g_Logs.server->error("SetAsset() Asset name is empty");

	return 0;
}

int SceneryObject :: SetPatrolEvent(const char *buffer)
{
	//Sets the name from a string by copying the contents of the buffer into
	//the patrolEvent member.
	memset(patrolEvent, 0, sizeof(patrolEvent));

	size_t ToCopy = strlen(buffer);
	if(ToCopy > sizeof(patrolEvent) - 1)
	{
		ToCopy = sizeof(patrolEvent) - 1;
		g_Logs.server->warn("Asset patrolEvent clipped");
	}
	strncpy(patrolEvent, buffer, ToCopy);
	return 0;
}

int SceneryObject :: SetPosition(const char *buffer)
{
	//Fill in the position data from the given string.  The string should contain 3
	//numbers separated by a space.

	int Start = 0;
	LocationX = GetPartFloat(buffer, Start);
	LocationY = GetPartFloat(buffer, Start);
	LocationZ = GetPartFloat(buffer, Start);
	return 0;
}

int SceneryObject :: SetQ(const char *buffer)
{
	//Quaternion (orientation) ?
	int Start = 0;
	QuatW = GetPartFloat(buffer, Start);
	QuatX = GetPartFloat(buffer, Start);
	QuatY = GetPartFloat(buffer, Start);
	QuatZ = GetPartFloat(buffer, Start);
	return 0;
}

int SceneryObject :: SetS(const char *buffer)
{
	int Start = 0;
	float temp;
	temp = GetPartFloat(buffer, Start);
	if(temp != 0.0F)
		ScaleX = temp;
	
	if(Start >= (int)strlen(buffer))
	{
		ScaleY = ScaleX;
		ScaleZ = ScaleX;
	}
	else
	{
		temp = GetPartFloat(buffer, Start);
		if(temp != 0.0F)
			ScaleY = temp;

		temp = GetPartFloat(buffer, Start);
		if(temp != 0.0F)
			ScaleZ = temp;
	}

	if(ScaleX == 0.0F)
		ScaleX = 1.0F;
	if(ScaleY == 0.0F)
		ScaleY = ScaleX;
	if(ScaleZ == 0.0F)
		ScaleZ = ScaleX;

	return 0;
}

void SceneryObject :: copyFrom(const SceneryObject *source)
{
	if(this == source)
		return;

	//Need to preserve the pointer, otherwise a memory leak could occur from
	//a lost pointer, and two props may later try to free the same memory block.
	CreatureSpawnDef *old = extraData;
	memcpy(this, source, sizeof(SceneryObject));
	extraData = old;
	if(source->extraData != NULL)
	{
		if(CreateExtraData() == true)
			extraData->copyFrom(source->extraData);
	}
}

bool SceneryObject :: CreateExtraData(void)
{
	if(extraData == NULL)
	{
		extraData = new(nothrow) CreatureSpawnDef;
		if(extraData != NULL)
			extraData->Clear();
	}

	return (extraData != NULL);
}

void SceneryObject :: SetPropertyCount(int count)
{
	if(CreateExtraData() == false)
		return;
	extraData->propCount = count;
}

void SceneryObject :: SetProperty(int index, const char *propName, int propType, const char *propValue)
{
	if(CreateExtraData() == false)
		return;
	if(index < 0 || index >= CreatureSpawnDef::MAX_PROP)
	{
		g_Logs.server->error("SetProperty index out of range [%v]", index);
		return;
	}
	Util::SafeCopy(extraData->prop[index].name, propName, sizeof(extraData->prop[index].name));
	extraData->prop[index].type = propType;
	Util::SafeCopy(extraData->prop[index].value, propValue, sizeof(extraData->prop[index].value));
}

void SceneryObject :: SetLinkCount(int count)
{
	if(CreateExtraData() == false)
		return;
	extraData->linkCount = count;
}

void SceneryObject :: SetLink(int index, int linkID, int type)
{
	if(CreateExtraData() == false)
		return;
	if(index < 0 || index >= CreatureSpawnDef::MAX_LINK)
	{
		g_Logs.server->error("SetLink index out of range [%v]", index);
		return;
	}
	extraData->link[index].propID = linkID;
	extraData->link[index].type = type;
}

bool SceneryObject :: IsExtendedProperty(const char *propertyName)
{
	static const char * extPropNames[16] = {
		"spawnName", "leaseTime", "spawnPackage", "mobTotal",
		"maxActive", "aiModule", "maxLeash", "loyaltyRadius", "dialog",
		"wanderRadius", "despawnTime", "sequential", "spawnLayer",
		"sceneryName", "innerRadius", "outerRadius" };
	for(int i = 0; i < 16; i++)
		if(strcmp(propertyName, extPropNames[i]) == 0)
			return true;
	return false;
}

bool SceneryObject :: SetExtendedProperty(const char *propertyName, const char *propertyValue)
{
	if(CreateExtraData() == false)
		return false;

	if(strcmp(propertyName, "spawnName") == 0)
		Util::SafeCopy(extraData->spawnName, propertyValue, sizeof(extraData->spawnName));
	else if(strcmp(propertyName, "leaseTime") == 0)
		extraData->leaseTime = atoi(propertyValue);
	else if(strcmp(propertyName, "spawnPackage") == 0)
		Util::SafeCopy(extraData->spawnPackage, propertyValue, sizeof(extraData->spawnPackage));
	else if(strcmp(propertyName, "dialog") == 0)
		Util::SafeCopy(extraData->dialog, propertyValue, sizeof(extraData->dialog));
	else if(strcmp(propertyName, "mobTotal") == 0)
		extraData->mobTotal = Util::ClipInt(atoi(propertyValue), 0, 5);
	else if(strcmp(propertyName, "maxActive") == 0)
		extraData->maxActive = Util::ClipInt(atoi(propertyValue), 0, 5);
	else if(strcmp(propertyName, "aiModule") == 0)
		Util::SafeCopy(extraData->aiModule, propertyValue, sizeof(extraData->aiModule));
	else if(strcmp(propertyName, "maxLeash") == 0)
		extraData->maxLeash = atoi(propertyValue);
	else if(strcmp(propertyName, "loyaltyRadius") == 0)
		extraData->loyaltyRadius = atoi(propertyValue);
	else if(strcmp(propertyName, "wanderRadius") == 0)
		extraData->wanderRadius = atoi(propertyValue);
	else if(strcmp(propertyName, "despawnTime") == 0)
		extraData->despawnTime = Util::ClipInt(atoi(propertyValue), 1, Platform::MAX_INT);
	else if(strcmp(propertyName, "sequential") == 0)
	{
		if(strcmp(propertyValue, "True") == 0)
			extraData->sequential = true;
		else if(strcmp(propertyValue, "False") == 0)
			extraData->sequential = false;
		else
			extraData->sequential = atoi(propertyValue);
	}
	else if(strcmp(propertyName, "spawnLayer") == 0)
		Util::SafeCopy(extraData->spawnLayer, propertyValue, sizeof(extraData->spawnLayer));
	else if(strcmp(propertyName, "sceneryName") == 0)
		Util::SafeCopy(extraData->sceneryName, propertyValue, sizeof(extraData->sceneryName));
	else if(strcmp(propertyName, "innerRadius") == 0)
		extraData->innerRadius = atoi(propertyValue);
	else if(strcmp(propertyName, "outerRadius") == 0)
		extraData->outerRadius = atoi(propertyValue);
	else
		return false;
	
	return true;
}

void SceneryObject::ReadFromJSON(Json::Value &value) {
	ID = value.get("id", 0).asInt();
	strcpy(Asset, value.get("asset", "").asCString());
	strcpy(Name, value.get("name", "").asCString());

	Json::Value pos = value["pos"];
	LocationX = pos.get("x", 0).asFloat();
	LocationY = pos.get("y", 0).asFloat();
	LocationZ = pos.get("z", 0).asFloat();

	Json::Value orient = value["orient"];
	QuatX = orient.get("x", 0).asFloat();
	QuatY = orient.get("y", 0).asFloat();
	QuatZ = orient.get("z", 0).asFloat();
	QuatW = orient.get("w", 0).asFloat();

	Json::Value scale = value["scale"];
	ScaleX = scale.get("x", 0).asFloat();
	ScaleY = scale.get("y", 0).asFloat();
	ScaleZ = scale.get("z", 0).asFloat();

	Flags = value["flags"].asInt();
	Layer = value["layer"].asInt();
	patrolSpeed = value["patrolSpeed"].asInt();
	strcpy(patrolEvent, value.get("patrolEvent", "").asCString());

	if(value.isMember("extra")) {
		Json::Value extra = value["extra"];
		if(CreateExtraData()) {
			extraData->facing = extra["facing"].asInt();
			strcpy(extraData->spawnName, value.get("spawnName", "").asCString());
			extraData->leaseTime = extra["leaseTime"].asInt();
			strcpy(extraData->spawnPackage, value.get("spawnPackage", "").asCString());
			extraData->mobTotal = extra["mobTotal"].asInt();
			if(extra.isMember("maxActive"))
				extraData->maxActive = extra["maxActive"].asInt();
			strcpy(extraData->aiModule, value.get("aiModule", "").asCString());
			if(extra.isMember("maxLeash"))
				extraData->maxLeash = extra["maxLeash"].asInt();
			extraData->loyaltyRadius = extra["loyaltyRadius"].asInt();
			extraData->wanderRadius = extra["wanderRadius"].asInt();
			if(extra.isMember("despawnTime"))
				extraData->despawnTime = extra["despawnTime"].asInt();
			extraData->sequential = extra["sequential"].asInt();
			strcpy(extraData->spawnLayer, value.get("spawnLayer", "").asCString());
			if(extra.isMember("links")) {
				Json::Value links = extra["links"];
				int count = 0;
				for(Json::Value::iterator lit = links.begin(); lit != links.end(); ++lit) {
					Json::Value litem = *lit;
					extraData->link[count].propID = litem["prop"].asInt();
					extraData->link[count].type = litem["type"].asInt();
					count++;
				}
			}
		}
	}
}

void SceneryObject::WriteToJSON(Json::Value &value) {
	value["id"] = ID;
	value["asset"] = Asset;
	value["name"] = Name;

	Json::Value pos;
	pos["x"] = LocationX;
	pos["y"] = LocationY;
	pos["z"] = LocationZ;
	value["pos"] = pos;

	Json::Value orient;
	orient["x"] = QuatX;
	orient["y"] = QuatY;
	orient["z"] = QuatZ;
	orient["w"] = QuatW;
	value["orient"] = orient;

	Json::Value scale;
	scale["x"] = ScaleX;
	scale["y"] = ScaleY;
	scale["z"] = ScaleZ;
	value["scale"] = scale;

	value["flags"] = Flags;
	value["layer"] = Layer;
	value["patrolSpeed"] = patrolSpeed;
	value["patrolEvent"] = patrolEvent;

	if(extraData != NULL) {
		Json::Value extra;

		extra["facing"] = extraData->facing;
		extra["spawnName"] = extraData->spawnName;
		extra["leaseTime"] = extraData->leaseTime;
		extra["spawnPackage"] = extraData->spawnPackage;
		extra["mobTotal"] = extraData->mobTotal;

		if(extraData->maxActive != CreatureSpawnDef::DEFAULT_MAXACTIVE)
			extra["maxActive"] = extraData->maxActive;

		extra["aiModule"] = extraData->aiModule;

		if(extraData->maxLeash != CreatureSpawnDef::DEFAULT_MAXLEASH && extraData->maxLeash != 0)
			extra["maxLeash"] = extraData->maxLeash;

		extra["loyaltyRadius"] = extraData->loyaltyRadius;
		extra["wanderRadius"] = extraData->wanderRadius;

		if(extraData->despawnTime != CreatureSpawnDef::DEFAULT_DESPAWNTIME)
			extra["despawnTime"] = extraData->despawnTime;

		extra["sequential"] = extraData->sequential;
		extra["spawnLayer"] = extraData->spawnLayer;

		if(extraData->linkCount > 0) {
			Json::Value links;
			for(int i = 0; i < extraData->linkCount; i++) {
				if(extraData->link[i].propID != 0) {
					Json::Value link;
					link["prop"] = extraData->link[i].propID;
					link["type"] = extraData->link[i].type;
					links[i] = link;
				}
			}
			extra["links"] = links;
		}

		value["extra"] = extra;
	}
}

void SceneryObject::WriteToStream(FILE *file) const
{
	fprintf(file, "[ENTRY]\r\n");
	fprintf(file, "ID=%d\r\n", ID);
	fprintf(file, "Asset=%s\r\n", Asset);
	fprintf(file, "Name=%s\r\n", Name);
	fprintf(file, "Pos=%g,%g,%g\r\n", LocationX, LocationY, LocationZ);
	fprintf(file, "Orient=%g,%g,%g,%g\r\n", QuatX, QuatY, QuatZ, QuatW);
	fprintf(file, "Scale=%g,%g,%g\r\n", ScaleX, ScaleY, ScaleZ);
	Util::WriteInteger(file, "Flags", Flags);
	Util::WriteInteger(file, "Layer", Layer);
	Util::WriteInteger(file, "patrolSpeed", patrolSpeed);
	Util::WriteString(file, "patrolEvent", patrolEvent);
	if(extraData != NULL)
	{
		Util::WriteInteger(file, "Facing", extraData->facing);
		Util::WriteString(file, "spawnName", extraData->spawnName);
		Util::WriteInteger(file, "leaseTime", extraData->leaseTime);
		Util::WriteString(file, "spawnPackage", extraData->spawnPackage);
		Util::WriteString(file, "dialog", extraData->dialog);
		Util::WriteInteger(file, "mobTotal", extraData->mobTotal);
		if(extraData->maxActive != CreatureSpawnDef::DEFAULT_MAXACTIVE)
			fprintf(file, "maxActive=%d\r\n", extraData->maxActive);
		Util::WriteString(file, "aiModule", extraData->aiModule);
		if(extraData->maxLeash != CreatureSpawnDef::DEFAULT_MAXLEASH && extraData->maxLeash != 0)
			Util::WriteInteger(file, "maxLeash", extraData->maxLeash);
		Util::WriteInteger(file, "loyaltyRadius", extraData->loyaltyRadius);
		Util::WriteInteger(file, "wanderRadius", extraData->wanderRadius);
		if(extraData->despawnTime != CreatureSpawnDef::DEFAULT_DESPAWNTIME)
			Util::WriteInteger(file, "despawnTime", extraData->despawnTime);
		Util::WriteInteger(file, "sequential", extraData->sequential);
		Util::WriteString(file, "spawnLayer", extraData->spawnLayer);
		if(extraData->linkCount > 0)
		{
			fprintf(file, "links_count=%d\r\n", extraData->linkCount);
			for(int i = 0; i < extraData->linkCount; i++)
			{
				if(extraData->link[i].propID != 0)
					fprintf(file, "link=%d,%d\r\n", extraData->link[i].propID, extraData->link[i].type);
			}
		}
	}
	fprintf(file, "\r\n");
}

const char* SceneryObject :: GetSpawnPackageName(void)
{
	if(extraData == NULL)
		return NULL;
	
	return extraData->spawnPackage;
}

bool SceneryObject::ExtractATS(std::string& outputStr) const
{
	const char *start = strstr(Asset, "ATS=");
	if(start == NULL)
		return false;
	start += 4;

	const char *end = strchr(start, '&');
	int len = 0;
	if(end == NULL)
		len = strlen(start);
	else
		len = end - start;
	
	outputStr.assign(start, len);
	return true;
}

void SceneryObject :: AddLink(int PropID, int type)
{
	if(CreateExtraData() == false)
		return;

	if(extraData->linkCount >= CreatureSpawnDef::MAX_LINK)
		return;

	for(int i = 0; i < extraData->linkCount; i++)
	{
		if(extraData->link[i].propID == PropID)
		{
			extraData->link[i].type = type;
			return;
		}
	}
	int slot = extraData->linkCount;
	extraData->link[slot].propID = PropID;
	extraData->link[slot].type = type;
	extraData->linkCount++;
}

void SceneryObject :: RemoveLink(int PropID)
{
	if(extraData == NULL)
		return;

	for(int i = 0; i < extraData->linkCount; i++)
	{
		if(extraData->link[i].propID == PropID)
		{
			extraData->link[i].propID = 0;
			extraData->link[i].type = 0;
			for(int d = i + 1; d < CreatureSpawnDef::MAX_LINK - 1; d++)
			{
				extraData->link[d].propID = extraData->link[d + 1].propID;
				extraData->link[d].type = extraData->link[d + 1].type;
			}
			extraData->linkCount--;
		}
	}
}

bool SceneryObject :: HasLinks(int linkType)
{
	if(extraData == NULL)
		return false;
	for(int i = 0; i < extraData->linkCount; i++)
	{
		if(extraData->link[i].type == linkType)
			return true;
	}
	return false;
}

void SceneryObject :: EnumLinks(int linkType, std::vector<int> &output)
{
	output.clear();
	if(extraData == NULL)
		return;
	for(int i = 0; i < extraData->linkCount; i++)
	{
		if(extraData->link[i].type == linkType)
			output.push_back(extraData->link[i].propID);
	}
}

bool SceneryObject :: IsSpawnPoint(void)
{
	if(strstr(Asset, "Manipulator-SpawnPoint") != NULL)
		return true;
	return false;
}

SceneryPage::SceneryPage()
{
	mTileX = 0;
	mTileY = 0;
	mZone = 0;
	mPendingChanges = 0;
	mLastAccessTime = 0;
	mHasSourceFile = false;
}

SceneryPage::~SceneryPage()
{
	Destroy();
}

void SceneryPage::Destroy(void)
{
	SCENERY_IT it;
	for(it = mSceneryList.begin(); it != mSceneryList.end(); ++it)
		it->second.Destroy();
	mSceneryList.clear();
}

void SceneryPage::NotifyAccess(bool notifyPendingChange)
{
	mLastAccessTime = g_ServerTime;
	if(notifyPendingChange == true)
		mPendingChanges++;
}

SceneryObject* SceneryPage::AddProp(const SceneryObject& prop, bool notifyPendingChange)
{
	//Note: isUserCreated should be true if the prop was added during run-time by
	//a player.  It should be false if loading resource from a file.

	SceneryObject &obj = mSceneryList[prop.ID];
	obj.copyFrom(&prop);
	NotifyAccess(notifyPendingChange);
	return &obj;
}

bool SceneryPage::DeleteProp(int propID)
{
	SCENERY_IT it = mSceneryList.find(propID);
	if(it == mSceneryList.end())
		return false;
	mSceneryList.erase(it);
	NotifyAccess(true);
	return true;
}

void SceneryPage::LoadScenery(void)
{
	TimeObject to("SceneryPage::LoadScenery");
	LoadSceneryFromFile(GetFileName());
	NotifyAccess(false);
}

void SceneryPage::CheckAutosave(int& debugPagesSaved, int& debugPropsSaved)
{
	if(mPendingChanges == 0)
		return;

	if(mSceneryList.size() == 0)
	{
		RemoveFile(GetFileName());
		mPendingChanges = 0;
	}
	else
	{
		if(mHasSourceFile == false)
		{
			Platform::MakeDirectory(GetFolderName());
		}

		if(SaveFile(GetFileName()) == true)
		{
			mPendingChanges = 0;
			debugPropsSaved += mSceneryList.size();
			mHasSourceFile = true;
		}
	}
	debugPagesSaved++;
}

void SceneryPage::RemoveFile(std::string fileName)
{
	g_Logs.data->info("Removed [%v]", fileName);
	remove(fileName.c_str());
}

bool SceneryPage::SaveFile(std::string fileName)
{
	FILE *output = fopen(fileName.c_str(), "wb");
	if(output == NULL)
	{
		g_Logs.data->error("Could not open file for writing [%v] - %v. %v", fileName, errno, strerror(errno));
		return false;
	}

	SCENERY_IT it;
	for(it = mSceneryList.begin(); it != mSceneryList.end(); ++it)
	{
		g_Logs.data->debug("Saving prop [%v]", it->second.ID);
			it->second.WriteToStream(output);
	}
	fclose(output);
	g_Logs.data->info("Saved prop [%v]", fileName);
	return true;
}

std::string SceneryPage::GetFileName()
{
	char buf[64];
	Util::SafeFormat(buf, sizeof(buf), "%d", mZone);
	char buf2[64];
	Util::SafeFormat(buf2, sizeof(buf2), "x%03dy%03d.txt", mTileX, mTileY);
	std::string p;
	if(mZone >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		p = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Grove"), buf), buf2);
	else
		p = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveVariableDataPath(), "Scenery"), buf), buf2);
	return p;
}

std::string SceneryPage::GetFolderName()
{
	char buf[64];
	Util::SafeFormat(buf, sizeof(buf), "%d");
	std::string p;
	if(mZone >= ZoneDefManager::GROVE_ZONE_ID_DEFAULT)
		p = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Grove"), buf);
	else
		p = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveVariableDataPath(), "Scenery"), buf);
	return p;
}

void SceneryPage::LoadSceneryFromFile(std::string fileName)
{
	FileReader3 fr;
	if(fr.OpenFile(fileName.c_str()) != FileReader3::SUCCESS)
	{
		g_Logs.data->debug("Could not open file to load scenery: [%v]", fileName);
		return;
	}
	fr.SetCommentChar(';');
	SceneryObject prop;
	int propertyIndex = 0;
	int linkIndex = 0;

	while(fr.Readable() == true)
	{
		int r = fr.ReadLine();
		if(r == 0)
			continue;

		r = fr.MultiBreak("=,");
		fr.BlockToStringC(0, FileReader3::CASE_UPPER);
		if(strcmp(fr.CopyBuffer, "[ENTRY]") == 0)
		{
			if(prop.ID != 0)
			{
				if(prop.Name[0] == 0)
					prop.SetName("Untitled");
				AddProp(prop, false);
			}
			prop.Clear();
			propertyIndex = 0;
			linkIndex = 0;
		}
		else if(strcmp(fr.CopyBuffer, "ID") == 0)
			prop.ID = fr.BlockToIntC(1);
		else if(strcmp(fr.CopyBuffer, "ASSET") == 0)
		{
			//The asset string needs to be single broken.
			fr.SingleBreak("=");
			prop.SetAsset(fr.BlockToStringC(1));
		}
		else if(strcmp(fr.CopyBuffer, "NAME") == 0)
			prop.SetName(fr.BlockToStringC(1, 0));
		else if(strcmp(fr.CopyBuffer, "POS") == 0)
		{
			prop.LocationX = fr.BlockToFloatC(1);
			prop.LocationY = fr.BlockToFloatC(2);
			prop.LocationZ = fr.BlockToFloatC(3);
		}
		else if(strcmp(fr.CopyBuffer, "ORIENT") == 0)
		{
			prop.QuatX = fr.BlockToFloatC(1);
			prop.QuatY = fr.BlockToFloatC(2);
			prop.QuatZ = fr.BlockToFloatC(3);
			prop.QuatW = fr.BlockToFloatC(4);
		}
		else if(strcmp(fr.CopyBuffer, "SCALE") == 0)
		{
			prop.ScaleX = fr.BlockToFloatC(1);
			prop.ScaleY = fr.BlockToFloatC(2);
			prop.ScaleZ = fr.BlockToFloatC(3);
		}
		else if(strcmp(fr.CopyBuffer, "FLAGS") == 0)
			prop.Flags = fr.BlockToIntC(1);
		else if(strcmp(fr.CopyBuffer, "LAYER") == 0)
			prop.Layer = fr.BlockToIntC(1);
		else if(strcmp(fr.CopyBuffer, "PATROLSPEED") == 0)
			prop.patrolSpeed = fr.BlockToIntC(1);
		else if(strcmp(fr.CopyBuffer, "PATROLEVENT") == 0)
			prop.SetPatrolEvent(fr.BlockToStringC(1));
		else if(strcmp(fr.CopyBuffer, "PROPS_COUNT") == 0)
			prop.SetPropertyCount(fr.BlockToIntC(1));
		else if(strcmp(fr.CopyBuffer, "PROPERTY") == 0)
		{
			//Calling BlockToStringC() overwrites the previous copy, so we need yet another
			//copy buffer to hold one parameter while we get another.
			char buffer[256];
			Util::SafeCopy(buffer, fr.BlockToStringC(1), sizeof(buffer));
			prop.SetProperty(propertyIndex++, buffer, fr.BlockToIntC(2), fr.BlockToStringC(3)); 
		}
		else if(strcmp(fr.CopyBuffer, "LINKS_COUNT") == 0)
			prop.SetLinkCount(fr.BlockToIntC(1));
		else if(strcmp(fr.CopyBuffer, "LINK") == 0)
		{
			int propID = fr.BlockToIntC(1);
			int linkType = fr.BlockToIntC(2);
			if(propID != 0)   //Fix to prevent null props.
				prop.SetLink(linkIndex++, propID, linkType);
		}
		else if(strcmp(fr.CopyBuffer, "FACING") == 0)
		{
			//Degree rotation facing is generated by the log rips and is used
			//to more easily spawn creatures with predetermined directional
			//facings rather than trying to look at the quaternion rotation of
			//the spawnpoint prop itself.
			if(prop.CreateExtraData() == true)
				prop.extraData->facing = fr.BlockToIntC(1);
		}
		else if(prop.IsExtendedProperty(fr.BlockToStringC(0)) == true)
		{
			char buffer[256];
			Util::SafeCopy(buffer, fr.CopyBuffer, sizeof(buffer));
			prop.SetExtendedProperty(buffer, fr.BlockToStringC(1));
		}
		//BEGIN DEPRECATED, PROVIDED FOR COMPATIBILITY WITH OLD FILES
		else if(strcmp(fr.CopyBuffer, "PX") == 0)
			prop.LocationX = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "PY") == 0)
			prop.LocationY = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "PZ") == 0)
			prop.LocationZ = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "QX") == 0)
			prop.QuatX = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "QY") == 0)
			prop.QuatY = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "QZ") == 0)
			prop.QuatZ = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "QW") == 0)
			prop.QuatW = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "SX") == 0)
			prop.ScaleX = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "SY") == 0)
			prop.ScaleY = fr.BlockToFloatC(1);
		else if(strcmp(fr.CopyBuffer, "SZ") == 0)
			prop.ScaleZ = fr.BlockToFloatC(1);
		//END DEPRECATED
	}

	if(prop.ID != 0)
	{
		if(prop.Name[0] == 0)
			prop.SetName("Untitled");
		AddProp(prop, false);
	}

	fr.CloseFile();
	mHasSourceFile = true;
	//g_Log.AddMessageFormat("Loaded scenery file: [%s]", fileName);
}

SceneryObject* SceneryPage::GetPropPtr(int propID)
{
	SCENERY_IT it = mSceneryList.find(propID);
	if(it != mSceneryList.end())
	{
		NotifyAccess(false);
		return &it->second;
	}
	return NULL;
}

bool SceneryPage::IsTileExpired(void)
{
	return (g_ServerTime >= (mLastAccessTime + SceneryManager::GARBAGE_CHECK_EXPIRE_TIME));
}


SceneryZone::SceneryZone()
{
	mZone = 0;
	mPageSize = DEFAULT_PAGE_SIZE;
}

SceneryZone::~SceneryZone()
{
	Destroy();
}

void SceneryZone::Destroy(void)
{
	PAGEMAP::iterator it;
	for(it = mPages.begin(); it != mPages.end(); ++it)
		it->second.Destroy();
	mPages.clear();
}

void SceneryZone::SetPageKeyFromProp(SceneryPageKey& pageKey, const SceneryObject& prop)
{
	int x = static_cast<int>(prop.LocationX) / mPageSize;
	int y = static_cast<int>(prop.LocationZ) / mPageSize;
	
	/* REMOVED: This didn't seem to work, and also we're blocking negative coordinate scenery anyway.
	//Hack for negative coordinates, we need to drop them down another tile.
	//Otherwise, if a tile size of 1000, integer division of -999/1000 and 999/1000
	//will round to zero.  The client may not expect this.
	if(prop.LocationX < 0.0F)
		x--;
	if(prop.LocationZ < 0.0F)
		y--;
	*/

	pageKey.x = x;
	pageKey.y = y;

	//g_Log.AddMessageFormat("Prop: %d [%s] is in %d,%d (%d)", prop.ID, prop.Asset, pageKey.x, pageKey.y, mPageSize);
}

SceneryObject* SceneryZone::AddProp(const SceneryObject& prop, bool notifyPendingChange)
{
	//Note: notifyPendingChange should be true if the prop was added during run-time by
	//a player.  It should be false if loading resource from a file.

	SceneryPageKey key;
	SetPageKeyFromProp(key, prop);

	SceneryPage *page = GetOrCreatePage(key);
	if(page == NULL)
		return NULL;

	return page->AddProp(prop, notifyPendingChange);
}

SceneryObject* SceneryZone::ReplaceProp(const SceneryObject& prop)
{
	//Search the zone for the prop, since the coordinates on edited props may be in a different page.
	SceneryObject *oldProp = GetPropPtr(prop.ID, NULL);
	if(oldProp == NULL)
	{
		//g_Log.AddMessageFormat("Prop not found(%d,%s)", prop.ID, prop.Asset);
		return NULL;
	}

	SceneryPageKey newKey;
	SceneryPageKey oldKey;
	SetPageKeyFromProp(newKey, prop);
	SetPageKeyFromProp(oldKey, *oldProp);

	SceneryPage *oldPage = GetOrCreatePage(oldKey);
	if(oldPage == NULL)
	{
		//g_Log.AddMessageFormat("Page not found (%d,%d)", oldKey.x, oldKey.y);
		return NULL;
	}

	if(oldKey.Compare(newKey) == true)
	{
		//g_Log.AddMessage("Replaced prop.");
		oldProp->copyFrom(&prop);
		oldPage->NotifyAccess(true);
		return oldProp;
	}
	else
	{
		//g_Log.AddMessage("Delete and created new.");
		oldPage->DeleteProp(prop.ID);
		return AddProp(prop, true);
	}
	return NULL;
}

void SceneryZone::DeleteProp(int propID)
{
	PAGEMAP::iterator it;
	for(it = mPages.begin(); it != mPages.end(); ++it)
		if(it->second.DeleteProp(propID) == true)
			return;
}

bool SceneryZone::UpdateLink(int propID1, int propID2, int type)
{
	if(propID1 == propID2)
		return false;

	SceneryPage *page1 = NULL;
	SceneryPage *page2 = NULL;
	SceneryObject *prop1 = GetPropPtr(propID1, &page1);
	SceneryObject *prop2 = GetPropPtr(propID2, &page2);
	
	if(prop1 == NULL || prop2 == NULL)
		return false;

	if(page1 == NULL || page2 == NULL)
		return false;

	//Add link
	if(type >= 0)
	{
		prop1->AddLink(propID2, type);
		prop2->AddLink(propID1, type);
	}
	else
	{
		prop1->RemoveLink(propID2);
		prop2->RemoveLink(propID1);
	}
	page1->NotifyAccess(true);
	page2->NotifyAccess(true);
	return true;
}

SceneryPage* SceneryZone::GetOrCreatePage(const SceneryPageKey& key)
{
	PAGEMAP::iterator it = mPages.find(key);
	if(it != mPages.end())
		return &it->second;

	return LoadPage(key);
}

SceneryPage* SceneryZone::LoadPage(const SceneryPageKey& key)
{
	SceneryPage &obj = mPages[key];
	obj.mTileX = key.x;
	obj.mTileY = key.y;
	obj.mZone = mZone;
	obj.LoadScenery();
	return &obj;
}

void SceneryZone::CheckAutosave(int& debugPagesSaved, int& debugPropsSaved)
{
	PAGEMAP::iterator it;
	for(it = mPages.begin(); it != mPages.end(); ++it)
		it->second.CheckAutosave(debugPagesSaved, debugPropsSaved);
}

SceneryObject* SceneryZone::GetPropPtr(int propID, SceneryPage** foundPage)
{
	SceneryObject *retProp = NULL;
	PAGEMAP::iterator it;
	for(it = mPages.begin(); it != mPages.end(); ++it)
	{
		retProp = it->second.GetPropPtr(propID);
		if(retProp != NULL)
		{
			if(foundPage != NULL)
				*foundPage = &it->second;
			return retProp;
		}
	}
	return NULL;
}

void SceneryZone::RemoveInactiveTiles(const ActiveLocation::CONTAINER& activeList)
{
	if(mPages.size() == 0)
		return;

	int count = 0;

	PAGEMAP::iterator it;
	it = mPages.begin();
	while(it != mPages.end())
	{
		if(it->second.IsTileExpired() == false)
		{
			++it;
			continue;
		}

		//if(ActiveLocation::IsLocationInUse(activeList, it->second.mTileX, it->second.mTileY, SceneryManager::GARBAGE_CHECK_TILE_RANGE) == false)
		if(ActiveLocation::IsLocationInUse(activeList) == false)
		{
			it->second.Destroy();
			mPages.erase(it++);
			count++;
		}
		else
			++it;
	}
	if(count > 0)
		g_Logs.server->info("Deleted %v inactive scenery tiles from zone %v", count, mZone);
}

size_t SceneryZone::GetTileCount(void)
{
	return mPages.size();
}

SceneryManager::SceneryManager()
{
	mNextAutosaveTime = 0;
	mNextGarbageCheckTime = 0;
	bThreadActive = false;
	cs.Init();
}

SceneryManager::~SceneryManager()
{
	Destroy();
}

void SceneryManager::Destroy(void)
{
	ITERATOR it;
	for(it = mZones.begin(); it != mZones.end(); ++it)
		it->second.Destroy();

	mZones.clear();
	mValidATS.clear();
}

SceneryZone* SceneryManager::FindZone(int zoneID)
{
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return NULL;
	return &it->second;
}

SceneryZone* SceneryManager::GetZone(int zoneID)
{
	return &mZones[zoneID];
}

SceneryZone* SceneryManager::GetOrCreateZone(int zoneID)
{
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
	{
		SceneryZone &obj = mZones[zoneID];

		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(zoneID);
		if(zoneDef == NULL)
		{
			g_Logs.server->error("Zone ID is not defined: %v", zoneID);
			obj.mPageSize = ZoneDefInfo::DEFAULT_PAGESIZE;
		}
		else
		{
			obj.mPageSize = zoneDef->mPageSize;
		}
		obj.mZone = zoneID;
		return &obj;
	}
	return &it->second;
}

SceneryPage* SceneryManager::GetOrCreatePage(int zoneID, int sceneryPageX, int sceneryPageY)
{
	SceneryZone *zone = GetOrCreateZone(zoneID);
	if(zone == NULL)
	{
		g_Logs.server->error("GetOrCreatePage failed to create zone: %v", zoneID);
		return NULL;
	}

	SceneryPageKey key(sceneryPageX, sceneryPageY);
	return zone->GetOrCreatePage(key);
	/*
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return NULL;
	SceneryPageKey key(sceneryPageX, sceneryPageY);
	return it->second.GetOrCreatePage(key);
	*/
}

void SceneryManager::LoadData(void)
{
	char buffer[256];
	LoadStringsFile(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "Valid_ATS.txt"), mValidATS);
	g_Logs.server->info("Marked %v valid ATS files.", mValidATS.size());
}

void SceneryManager::CheckAutosave(bool force)
{
	if(g_ServerTime < mNextAutosaveTime && force == false)
		return;

	int debugPagesSaved = 0;
	int debugPropsSaved = 0;

	GetThread("SceneryPage::CheckAutosave");

	ITERATOR it;
	for(it = mZones.begin(); it != mZones.end(); ++it)
		it->second.CheckAutosave(debugPagesSaved, debugPropsSaved);

	ReleaseThread();

	mNextAutosaveTime = g_ServerTime + g_SceneryAutosaveTime;
	if(debugPropsSaved > 0)
		g_Logs.data->info("Saved %v props in %v pages.", debugPropsSaved, debugPagesSaved);
}

bool SceneryManager::ValidATSEntry(const std::string& atsName)
{
	for(size_t i = 0; i < mValidATS.size(); i++)
		if(mValidATS[i].compare(atsName) == 0)
			return true;
	return false;
}

bool SceneryManager::VerifyATS(const SceneryObject& prop)
{
	//Return true if the asset name passes ATS inspection, or does not contain an ATS.
	std::string ats;
	if(prop.ExtractATS(ats) == false)
		return true;

	if(ValidATSEntry(ats) == true)
		return true;

	return false;
}

void SceneryManager::GetThread(const char *request)
{
	cs.Enter(request);
}

void SceneryManager::ReleaseThread(void)
{
	cs.Leave();
}

SceneryObject* SceneryManager::GlobalGetPropPtr(int zoneID, int propID, SceneryPage** foundPage)
{
	TimeObject to("SceneryManager::GlobalGetPropPtr");

	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return NULL;

	return it->second.GetPropPtr(propID, foundPage);
}

SceneryObject* SceneryManager::AddProp(int zoneID, const SceneryObject& prop)
{
	TimeObject to("SceneryManager::AddProp");

	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return NULL;
	
	//Adding a prop to an arbitrary zone is always a user created object, most likely a player
	//or in rare cases perhaps a custom import.  So AddProp() is called as such.
	return it->second.AddProp(prop, true);
}

SceneryObject* SceneryManager::ReplaceProp(int zoneID, const SceneryObject& prop)
{
	TimeObject to("SceneryManager::ReplaceProp");

	//Note: before replacing a prop, the calling function should unattach any instance spawners
	//(in the case of SpawnPoints) that may be using that prop to prevent dangling pointers.
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return NULL;
	return it->second.ReplaceProp(prop);
}

void SceneryManager::DeleteProp(int zoneID, int propID)
{
	TimeObject to("SceneryManager::DeleteProp");

	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return;
	it->second.DeleteProp(propID);
}

bool SceneryManager::UpdateLink(int zoneID, int propID1, int propID2, int type)
{
	TimeObject to("SceneryManager::UpdateLink");
	
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return false;
	return it->second.UpdateLink(propID1, propID2, type);
}

void SceneryManager :: NotifyChangedProp(int zoneID, int propID)
{
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return;
	SceneryPage *page = NULL;
	SceneryObject *prop = it->second.GetPropPtr(propID, &page);
	if(page == NULL || prop == NULL)
		return;
	page->NotifyAccess(true);
}

void SceneryManager::AddPageRequest(int socket, int queryID, int zone, int x, int y, bool skipQuery, std::list<int> excludedProps)
{
	SceneryPageRequest newItem;
	newItem.socket = socket;
	newItem.queryID = queryID;
	newItem.zone = zone;
	newItem.x = x;
	newItem.y = y;
	newItem.skipQuery = skipQuery;
	newItem.excludedProps.insert(newItem.excludedProps.begin(), excludedProps.begin(), excludedProps.end());
	mPendingPageRequest.push_back(newItem);
}

void SceneryManager::TransferPageRequests(void)
{
	if(mPendingPageRequest.size() == 0)
		return;
	mImmediatePageRequest.assign(mPendingPageRequest.begin(), mPendingPageRequest.end());
	mPendingPageRequest.clear();
}

void SceneryManager::ProcessPageRequests(void)
{
	TimeObject to("SceneryManager::ProcessPageRequests");

	GetThread("SceneryManager::HandlePageRequests");
	TransferPageRequests();
	ReleaseThread();

	size_t pending = mImmediatePageRequest.size();
	if(pending == 0)
		return;

	//Build a list of packets to send out so we only have to acquire the thread
	//once at the end.
	std::list<PacketManager::PACKET_PAIR> outgoingPackets;
	for(size_t i = 0; i < pending; i++)
	{
		SendPageRequest(mImmediatePageRequest[i], outgoingPackets);
	}
	
	if(outgoingPackets.size() > 0)
	{
		//Add the outgoing packets to the queue.
		std::list<PacketManager::PACKET_PAIR>::iterator it;

		g_PacketManager.GetThread("SceneryManager::HandlePageRequests(s)");

		for(it = outgoingPackets.begin(); it != outgoingPackets.end(); ++it)
			if(it->second.mData.size() > 0)
				g_PacketManager.AddOutgoingPacket2(it->first, it->second);

		g_PacketManager.ReleaseThread();
	}

	mImmediatePageRequest.clear();
}

void SceneryManager::SendPageRequest(const SceneryPageRequest& request, std::list<PacketManager::PACKET_PAIR>& outgoingPackets)
{
	TimeObject to("SceneryManager::SendPageRequest");

	STRINGLIST queryRows;
	Packet data;
	int wpos = 0;
	char idBuf[32];

	GetThread("SceneryManager::HandlePageRequests[page]");

	SceneryPage *page = GetOrCreatePage(request.zone, request.x, request.y);

	if(page == NULL)
	{
		g_Logs.server->error("SendPageRequest retrieved NULL page");
		
		wpos = PrepExt_QueryResponseNull(prepBuf, request.queryID);
		data.Assign(prepBuf, wpos);
		outgoingPackets.push_back(PacketManager::PACKET_PAIR(request.socket, data));

		ReleaseThread();
		return;
	}

	SceneryPage::SCENERY_IT it;

	for(it = page->mSceneryList.begin(); it != page->mSceneryList.end(); ++it)
	{
		if( (std::find(request.excludedProps.begin(), request.excludedProps.end(), it->second.ID) != request.excludedProps.end()))
			// Excluded prop, probably excluded as the result of a script prop removal
			continue;

		//Build the list of scenery ID strings to form the response to the scenery.list query.
		//No need to save row data unless the query is required.
		if(request.skipQuery == false)
		{
			sprintf(idBuf, "%d", it->second.ID);
			queryRows.push_back(idBuf);
		}
		
		wpos += PrepExt_UpdateScenery(&prepBuf[wpos], &it->second);


		if(wpos > Global::MAX_SEND_CHUNK_SIZE)
		{
			data.Assign(prepBuf, wpos);
			outgoingPackets.push_back(PacketManager::PACKET_PAIR(request.socket, data));
			wpos = 0;
		}
	}
	if(wpos > 0)
	{
		data.Assign(prepBuf, wpos);
		outgoingPackets.push_back(PacketManager::PACKET_PAIR(request.socket, data));
	}

	//Done accessing the scenery data itself, no need to hold the thread any longer.
	//All the remaining stuff is using a resident list of query IDs to form into a response
	//packet.
	ReleaseThread();

	//Now build the query response if the client has requested it.
	if(request.skipQuery == true) {
		return;
	}

	//Reset the packet buffer and data.
	wpos = 0;
	data.Clear();

	//Get the size of the response
	int sizeReq = 6;  //Query ID (4 bytes) + row count (2 bytes)

	for(size_t s = 0; s < queryRows.size(); s++)
	{
		sizeReq++;  //1 string per row
		sizeReq += PutStringReq(queryRows[s].c_str());
	}

	wpos += PutByte(&prepBuf[wpos], 1);         //_handleQueryResultMsg
	wpos += PutShort(&prepBuf[wpos], sizeReq);  //Message size

	wpos += PutInteger(&prepBuf[wpos], request.queryID);
	wpos += PutShort(&prepBuf[wpos], queryRows.size());
	for(size_t s = 0; s < queryRows.size(); s++)
	{
		wpos += PutByte(&prepBuf[wpos], 1);
		wpos += PutStringUTF(&prepBuf[wpos], queryRows[s].c_str());
		if(wpos > Global::MAX_SEND_CHUNK_SIZE)
		{
			data.Append(prepBuf, wpos);
			wpos = 0;
		}
	}
	if(wpos > 0)
		data.Append(prepBuf, wpos);

	outgoingPackets.push_back(PacketManager::PACKET_PAIR(request.socket, data));
}


void SceneryManager::ThreadProc(SceneryManager *object)
{
	object->ThreadMain();
	PLATFORM_CLOSETHREAD(0);
}

void SceneryManager::ThreadMain(void)
{
	bThreadActive = true;

	while(bThreadActive == true)
	{
		if(mPendingPageRequest.size() > 0)
			ProcessPageRequests();

		RunGarbageCheck();

		PLATFORM_SLEEP(1);
	}
}

void SceneryManager::LaunchThread(void)
{
	if(Platform_CreateThread(0, (void*)ThreadProc, &g_SceneryManager, NULL) == 0)
		g_Logs.server->error("SceneryManager::LaunchThread: error creating thread");
	else
		g_Logs.server->info("SceneryManager::LaunchThread: successful");
}

void SceneryManager::ShutdownThread(void)
{
	bThreadActive = false;
}

bool SceneryManager::IsGarbageCheckReady(void)
{
	if(g_ServerTime >= mNextGarbageCheckTime)
	{
		mNextGarbageCheckTime = g_ServerTime + GARBAGE_CHECK_SCAN_DELAY;
		return true;
	}
	return false;
}

void SceneryManager::RunGarbageCheck(void)
{
	if(mActiveLocations.size() == 0)
		return;

	ActiveLocation::CONTAINER perZone;

	ITERATOR zoneit;

	GetThread("SceneryManager::RunGarbageCheck");
	zoneit = mZones.begin();
	while(zoneit != mZones.end())
	{
		//Build a list of all active locations that match this zone.
		for(size_t i = 0; i < mActiveLocations.size(); i++)
		{
			if(mActiveLocations[i] == zoneit->second.mZone)
				perZone.push_back(mActiveLocations[i]);

			/*
			if(mActiveLocations[i].mZoneID == zoneit->second.mZone)
				perZone.push_back(mActiveLocations[i]);
			*/
		}

		//Pass the filtered active locations to the zone's garbage checker.
		//Since unmatched zones are filtered out, fewer iterations are needed for each tile.
		zoneit->second.RemoveInactiveTiles(perZone);
		if(perZone.size() > 0)
			perZone.clear();

		if(zoneit->second.GetTileCount() == 0)
		{
			g_Logs.server->info("Removing inactive Zone: %v", zoneit->second.mZone);
			mZones.erase(zoneit++);
		}
		else
			zoneit++;
	}
	mActiveLocations.clear();

	ReleaseThread();
}


//Note: Calls to this function should have a thread guard.
void SceneryManager::TransferActiveLocations(const ActiveLocation::CONTAINER& source)
{
	mActiveLocations.assign(source.begin(), source.end());
}

//This is a debug function for specialized searches, we'll do all the work here.
void SceneryManager::EnumPropsInRange(int zoneID, int posX, int posZ, int radius, std::vector<SceneryObject*>& searchResults)
{
	ITERATOR it = mZones.find(zoneID);
	if(it == mZones.end())
		return;

	SceneryZone::PAGEMAP::iterator pit;
	SceneryPage::SCENERY_IT sit;
	for(pit = it->second.mPages.begin(); pit != it->second.mPages.end(); ++pit)
	{
		for(sit = pit->second.mSceneryList.begin(); sit != pit->second.mSceneryList.end(); ++sit)
		{
			int xlen = abs(posX - (int)sit->second.LocationX);
			int zlen = abs(posZ - (int)sit->second.LocationZ);
			if(xlen < radius && zlen < radius)
				searchResults.push_back(&sit->second);
		}
	}
}


int PrepExt_UpdateScenery(char *buffer, SceneryObject *so)
{
	unsigned char mask = SCENERY_UPDATE_ASSET | SCENERY_UPDATE_POSITION | \
		SCENERY_UPDATE_ORIENTATION | SCENERY_UPDATE_SCALE | SCENERY_UPDATE_FLAGS;

	/*

	unsigned char mask = SCENERY_UPDATE_ASSET | SCENERY_UPDATE_POSITION | \
		SCENERY_UPDATE_ORIENTATION | SCENERY_UPDATE_SCALE | SCENERY_UPDATE_FLAGS | \
		SCENERY_UPDATE_LINKS | SCENERY_UPDATE_PROPERTIES;
	*/

	if(so->extraData != NULL)
	{
		if(so->extraData->linkCount > 0)
			mask |= SCENERY_UPDATE_LINKS;
		if(so->extraData->spawnPackage[0] != 0)
			mask |= SCENERY_UPDATE_PROPERTIES;
	}

	/*	SCENERY_UPDATE_PROPERTIES; */

	//unsigned char mask = SCENERY_UPDATE_ASSET | SCENERY_UPDATE_POSITION;

	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 41);  //_handleSceneryUpdateMsg
	wpos += PutShort(&buffer[wpos], 0);

	wpos += PutInteger(&buffer[wpos], so->ID);  //ID
	wpos += PutByte(&buffer[wpos], mask);  //Mask

	if(mask & SCENERY_UPDATE_ASSET)
	{
		wpos += PutStringUTF(&buffer[wpos], so->Asset);
		if(g_ProtocolVersion >= 23)
		{
			//Layer
			wpos += PutStringUTF(&buffer[wpos], "");
		}
	}
	if(mask & SCENERY_UPDATE_POSITION)
	{
		wpos += PutFloat(&buffer[wpos], so->LocationX);
		wpos += PutFloat(&buffer[wpos], so->LocationY);
		wpos += PutFloat(&buffer[wpos], so->LocationZ);
	}

	if(mask & SCENERY_UPDATE_ORIENTATION)
	{
		wpos += PutFloat(&buffer[wpos], so->QuatX);
		wpos += PutFloat(&buffer[wpos], so->QuatY);
		wpos += PutFloat(&buffer[wpos], so->QuatZ);
		wpos += PutFloat(&buffer[wpos], so->QuatW);
	}

	if(mask & SCENERY_UPDATE_SCALE)
	{
		wpos += PutFloat(&buffer[wpos], so->ScaleX);
		wpos += PutFloat(&buffer[wpos], so->ScaleY);
		wpos += PutFloat(&buffer[wpos], so->ScaleZ);
	}

	if(mask & SCENERY_UPDATE_FLAGS)
	{
		wpos += PutInteger(&buffer[wpos], so->Flags);
	}


	if(mask & SCENERY_UPDATE_LINKS)
	{
		wpos += PutShort(&buffer[wpos], so->extraData->linkCount);  //count
		for(int a = 0; a < so->extraData->linkCount; a++)
		{
			wpos += PutInteger(&buffer[wpos], so->extraData->link[a].propID);
			wpos += PutByte(&buffer[wpos], so->extraData->link[a].type);
		}
	}

	if(mask & SCENERY_UPDATE_PROPERTIES)
	{
		//Format:
		//scenery name [string]
		//number of properties [int]
		//[for each property]
		//  property name [string]
		//  property type [byte] (PROPERTY_INTEGER, FLOAT, STRING, SCENERY, NULL)
		//  property data [varies]
		//    - PROPERTY_INTEGER [int]
		//    - PROPERTY_FLOAT [float]
		//    - PROPERTY_STRING [string]
		//    - PROPERTY_SCENERY [int]
		//    - PROPERTY_NULL [unknown, unused?)

		//Spawn points always seem to have 3 property listings.
		//The names and order follow this format, but sometimes
		//the logged values sometimes use FLOAT instead of INTEGER
		//property types, although it seems like values can be
		//expressed with integers without any conflicts.
		wpos += PutStringUTF(&buffer[wpos], so->extraData->sceneryName);
		wpos += PutInteger(&buffer[wpos], 4);  //property count

		wpos += PutStringUTF(&buffer[wpos], "innerRadius");
		wpos += PutByte(&buffer[wpos], PROPERTY_INTEGER);
		wpos += PutInteger(&buffer[wpos], so->extraData->innerRadius);

		wpos += PutStringUTF(&buffer[wpos], "package");
		wpos += PutByte(&buffer[wpos], PROPERTY_STRING);
		wpos += PutStringUTF(&buffer[wpos], so->extraData->spawnPackage);

		wpos += PutStringUTF(&buffer[wpos], "dialog");
		wpos += PutByte(&buffer[wpos], PROPERTY_STRING);
		wpos += PutStringUTF(&buffer[wpos], so->extraData->dialog);

		wpos += PutStringUTF(&buffer[wpos], "outerRadius");
		wpos += PutByte(&buffer[wpos], PROPERTY_INTEGER);
		wpos += PutInteger(&buffer[wpos], so->extraData->outerRadius);

		/*
		wpos += PutStringUTF(&buffer[wpos], so->extraData->sceneryName);
		wpos += PutInteger(&buffer[wpos], so->extraData->propCount);
		for(int a = 0; a < so->extraData->propCount; a++)
		{
			wpos += PutStringUTF(&buffer[wpos], so->extraData->prop[a].name);
			wpos += PutByte(&buffer[wpos], so->extraData->prop[a].type);
			switch(so->extraData->prop[a].type)
			{
			case PROPERTY_INTEGER:
			case PROPERTY_SCENERY:
				int iconv;
				iconv = atoi(so->extraData->prop[a].value);
				wpos += PutInteger(&buffer[wpos], iconv);
				break;
			case PROPERTY_FLOAT:
				float fconv;
				fconv = (float)atof(so->extraData->prop[a].value);
				wpos += PutFloat(&buffer[wpos], fconv);
				break;
			case PROPERTY_STRING:
				wpos += PutStringUTF(&buffer[wpos], so->extraData->prop[a].value);
				break;
			}
		}
		*/
	}

	PutShort(&buffer[1], wpos - 3);       //Set message size
	return wpos;
}
