#include "GM.h"
#include "FileReader.h"
#include "Util.h"

#include "Character.h"
#include "Config.h"
#include "Cluster.h"
#include <dirent.h>
#include "util/Log.h"

PetitionManager g_PetitionManager;

Petition::Petition() {
	petitionId = 0;
	status = PetitionStatus::PENDING;
	petitionerCDefID = 0;
	sageCDefID = 0;
	timestamp = 0;
	category = 0;
	description = "";
	resolution = "";
}

Petition::~Petition() {

}

bool Petition :: WriteEntity(AbstractEntityWriter *writer) {
	writer->Key(KEYPREFIX_PETITION, Util::Format("%d", petitionId));
	writer->Value("ID", petitionId);
	writer->Value("Category", category);
	writer->Value("CDefID", petitionerCDefID);
	writer->Value("SageCDefID", sageCDefID);
	writer->Value("Timestamp", timestamp);
	writer->Value("Status", status);
	writer->Value("Description", description);
	writer->Value("Resolution", resolution);
	return true;
}

bool Petition :: EntityKeys(AbstractEntityReader *reader) {
	reader->Key(KEYPREFIX_PETITION, Util::Format("%d", petitionId), true);
	return true;
}

bool Petition :: ReadEntity(AbstractEntityReader *reader) {
	petitionId = reader->ValueInt("ID");
	category = reader->ValueInt("Category");
	petitionerCDefID = reader->ValueInt("CDefID");
	sageCDefID = reader->ValueInt("SageCDefID");
	timestamp = reader->ValueULong("timestamp");
	status = reader->ValueInt("Status");
	description = reader->Value("Description");
	resolution = reader->ValueInt("Resolution");
	return true;
}

void Petition::Clear(void) {
	petitionId = 0;
	category = 0;
	petitionerCDefID = 0;
	sageCDefID = 0;
	timestamp = 0;
	status = 0;
	description = "";
	resolution = "";
}

PetitionManager::PetitionManager() {
}

PetitionManager::~PetitionManager() {
}


bool PetitionManager::Take(int petitionId, int sageCharacterId) {
	Petition p;
	p.petitionId = petitionId;
	if(!g_ClusterManager.ReadEntity(&p)) {
		g_Logs.data->error("Failed to read petition %v (sage: %v) to take it", petitionId, sageCharacterId);
		return false;
	}
	if(p.status != PetitionStatus::PENDING) {
		g_Logs.data->error("Petition %v (sage: %v) is not pending", petitionId, sageCharacterId);
		return false;
	}
	p.status = PetitionStatus::TAKEN;
	if(!g_ClusterManager.WriteEntity(&p)) {
		g_Logs.data->error("Failed to write petition %v (sage: %v) to take it", petitionId, sageCharacterId);
		return false;
	}
	g_ClusterManager.ListRemove(LISTPREFIX_PENDING_PETITIONS, Util::Format("%d", petitionId), true);
	g_ClusterManager.ListAdd(Util::Format("%s:%d", LISTPREFIX_TAKEN_PETITIONS.c_str(), sageCharacterId), Util::Format("%d", petitionId).c_str(), true);
	return true;
}

bool PetitionManager::Untake(int petitionId, int sageCharacterId) {
	Petition p;
	p.petitionId = petitionId;
	if(!g_ClusterManager.ReadEntity(&p)) {
		g_Logs.data->error("Failed to read petition %v (sage: %v) to untake it", petitionId, sageCharacterId);
		return false;
	}
	if(p.status != PetitionStatus::TAKEN) {
		g_Logs.data->error("Petition %v (sage: %v) is not taken", petitionId, sageCharacterId);
		return false;
	}
	p.status = PetitionStatus::PENDING;
	if(!g_ClusterManager.WriteEntity(&p)) {
		g_Logs.data->error("Failed to write petition %v (sage: %v) to untake it", petitionId, sageCharacterId);
		return false;
	}
	g_ClusterManager.ListAdd(LISTPREFIX_PENDING_PETITIONS, Util::Format("%d", petitionId), true);
	g_ClusterManager.ListRemove(Util::Format("%s:%d", LISTPREFIX_TAKEN_PETITIONS.c_str(), sageCharacterId), Util::Format("%d", petitionId).c_str(), true);
	return true;
}

bool PetitionManager::Close(int petitionId, int sageCharacterId) {
	Petition p;
	p.petitionId = petitionId;
	if(!g_ClusterManager.ReadEntity(&p)) {
		g_Logs.data->error("Failed to read petition %v (sage: %v) to close it", petitionId, sageCharacterId);
		return false;
	}
	if(p.status == PetitionStatus::CLOSED) {
		g_Logs.data->error("Petition %v (sage: %v) is already closed", petitionId, sageCharacterId);
		return false;
	}
	p.status = PetitionStatus::CLOSED;
	if(!g_ClusterManager.WriteEntity(&p)) {
		g_Logs.data->error("Failed to write petition %v (sage: %v) to close it", petitionId, sageCharacterId);
		return false;
	}
	g_ClusterManager.ListRemove(LISTPREFIX_PENDING_PETITIONS, Util::Format("%d", petitionId), true);
	g_ClusterManager.ListRemove(Util::Format("%s:%d", LISTPREFIX_TAKEN_PETITIONS.c_str(), sageCharacterId), Util::Format("%d", petitionId).c_str(), true);
	g_ClusterManager.ListAdd(LISTPREFIX_CLOSED_PETITIONS, Util::Format("%d", petitionId));
	return true;
}

int PetitionManager::NewPetition(int petitionerCDefID, int category, const char *description) {
	Petition p;
	p.status = PetitionStatus::PENDING;
	p.petitionId = g_ClusterManager.NextValue(ID_NEXT_PETITION_ID);
	p.category = category;
	p.description = description;
	p.petitionerCDefID = petitionerCDefID;
	p.timestamp = g_ServerTime;
	if(g_ClusterManager.WriteEntity(&p)) {
		g_ClusterManager.ListAdd(LISTPREFIX_PENDING_PETITIONS, Util::Format("%d", p.petitionId));
		return p.petitionId;
	}
	g_Logs.data->error("Failed to saving petition %v to cluster", p.petitionId);
	return -1;
}

std::vector<Petition> PetitionManager::GetPetitions(int sageCharacterID) {
	std::vector<Petition> v;
	STRINGLIST p =  g_ClusterManager.GetList(LISTPREFIX_PENDING_PETITIONS);
	FillPetitions(p, v);
	p = g_ClusterManager.GetList(Util::Format("%s:%d", LISTPREFIX_TAKEN_PETITIONS.c_str(), sageCharacterID));
	FillPetitions(p, v);
	return v;
}

void PetitionManager::FillPetitions(std::vector<std::string> &in, std::vector<Petition> &out) {
	for(auto a = in.begin(); a != in.end(); ++a) {
		Petition p;
		p.petitionId = atoi((*a).c_str());
		if(!g_ClusterManager.ReadEntity(&p))
			g_Logs.data->error("Failed to retrieve petition %v from cluster", p.petitionId);
		else
			out.push_back(p);

	}
}




