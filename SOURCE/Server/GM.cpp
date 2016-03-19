#include "GM.h"
#include "FileReader.h"
#include "Util.h"

#include "Character.h"
#include <dirent.h>
#include "util/Log.h"

PetitionManager g_PetitionManager;

void Petition::Clear(void) {
	petitionId = 0;
	category = 0;
	petitionerCDefID = 0;
	sageCDefID = 0;
	timestamp = 0;
	status = 0;
	memset(description, 0, sizeof(description));
	memset(resolution, 0, sizeof(resolution));
}

void Petition::RunLoadDefaults(void) {
}

PetitionManager::PetitionManager() {
	NextPetitionID = 1;
	char tempStrBuf[100];
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions");
	Platform::FixPaths(tempStrBuf);
	Platform::MakeDirectory(tempStrBuf);
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\Pending");
	Platform::FixPaths(tempStrBuf);
	Platform::MakeDirectory(tempStrBuf);
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\Closed");
	Platform::FixPaths(tempStrBuf);
	Platform::MakeDirectory(tempStrBuf);
}

PetitionManager::~PetitionManager() {
}


bool PetitionManager::Take(int petitionId, int sageCharacterId) {
	char tempStrBuf[100];
	char tempStrBuf2[100];
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\%d", sageCharacterId);
	Platform::FixPaths(tempStrBuf);
	if(!Platform::DirExists(tempStrBuf))
		Platform::MakeDirectory(tempStrBuf);
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\Pending\\%d.txt", petitionId);
	Platform::FixPaths(tempStrBuf);
	Util::SafeFormat(tempStrBuf2, sizeof(tempStrBuf2), "Petitions\\%d\\%d.txt", sageCharacterId, petitionId);
	Platform::FixPaths(tempStrBuf2);
	if(Platform::FileCopy(tempStrBuf, tempStrBuf2) == 0 && remove(tempStrBuf) == 0)
		return true;
	g_Logs.data->error("Failed to take petition to %v", tempStrBuf);
	return false;
}
bool PetitionManager::Untake(int petitionId, int sageCharacterId) {
	char tempStrBuf[100];
	char tempStrBuf2[100];
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\%d\\%d.txt", sageCharacterId, petitionId);
	Platform::FixPaths(tempStrBuf);
	Util::SafeFormat(tempStrBuf2, sizeof(tempStrBuf2), "Petitions\\Pending\\%d.txt", petitionId);
	Platform::FixPaths(tempStrBuf2);
	if(Platform::FileCopy(tempStrBuf, tempStrBuf2) == 0 && remove(tempStrBuf) == 0)
		return true;
	g_Logs.data->error("Failed to untake petition to %v", tempStrBuf);
	return false;
}
bool PetitionManager::Close(int petitionId, int sageCharacterId) {
	char tempStrBuf[100];
	char tempStrBuf2[100];
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions\\%d\\%d.txt", sageCharacterId, petitionId);
	Platform::FixPaths(tempStrBuf);
	Util::SafeFormat(tempStrBuf2, sizeof(tempStrBuf2), "Petitions\\Closed\\%d.txt", petitionId);
	Platform::FixPaths(tempStrBuf2);
	if(Platform::FileCopy(tempStrBuf, tempStrBuf2) == 0 && remove(tempStrBuf) == 0)
		return true;
	g_Logs.data->error("Failed to close petition to %v", tempStrBuf);
	return false;
}

int PetitionManager::NewPetition(int petitionerCDefID, int category, const char *description) {
	char buffer[256];
	int id = NextPetitionID++;
	Util::SafeFormat(buffer, sizeof(buffer), "Petitions\\Pending\\%d.txt", id);
	Platform::FixPaths(buffer);
	g_Logs.data->info("Saving petition to %v.", buffer);
	FILE *output = fopen(buffer, "wb");
	if (output == NULL) {
		g_Logs.data->error("Saving petition could not open: %v", buffer);
		return -1;
	}
	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "ID=%d\r\n", id);
	time_t  timev;
	time(&timev);
	fprintf(output, "Timestamp=%lu\r\n", timev);
	fprintf(output, "Category=%d\r\n", category);
	fprintf(output, "Petitioner=%d\r\n", petitionerCDefID);

	string r = description;
	Util::ReplaceAll(r, "\r\n", "\\r\\n");
	Util::ReplaceAll(r, "\n", "\\n");
	fprintf(output, "Description=%s\r\n", r.c_str());
	fprintf(output, "\r\n");

	fflush(output);
	fclose(output);

	return id;
}

std::vector<Petition> PetitionManager::GetPetitions(int sageCharacterID) {
	std::vector<Petition> v;
	char tempStrBuf[100];
	Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions/Pending");
	Platform::FixPaths(tempStrBuf);
	FillPetitions(&v, tempStrBuf, PENDING);
	if(sageCharacterID != 0) {
		Util::SafeFormat(tempStrBuf, sizeof(tempStrBuf), "Petitions/%d", sageCharacterID);
		Platform::FixPaths(tempStrBuf);
		if(Platform::DirExists(tempStrBuf)) {
			FillPetitions(&v, tempStrBuf, TAKEN);
		}
	}
	return v;
}


void PetitionManager::FillPetitions(std::vector<Petition> *petitions, const char *path, PetitionStatus status) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			string s = string(ent->d_name);
			Util::RemoveStringsFrom(".txt", s);
			int id = atoi(s.c_str());
			if(id > 0) {
				g_Logs.data->info("Found petition %v", id);
				Petition p = Load(path, id);
				p.status = status;
				petitions->push_back(p);
			}
		}
		closedir(dir);
	} else {
		g_Logs.data->error("Failed to open Petitions directory %v, does it exist?", path);
	}
}

Petition PetitionManager::Load(const char *path, int id) {
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "%s\\%d.txt", path, id);
	Platform::FixPaths(buffer);

	FileReader lfr;
	Petition newItem;
	if (lfr.OpenText(buffer) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", buffer);
	} else {
		lfr.CommentStyle = Comment_Semi;
		int r = 0;
		while (lfr.FileOpen() == true) {
			r = lfr.ReadLine();
			lfr.SingleBreak("=");
			lfr.BlockToStringC(0, Case_Upper);
			if (r > 0) {
				if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
					//
					if (newItem.petitionId != 0) {
						g_Logs.data->warn("Petition file %v has more than one ENTRY", buffer);
						newItem.RunLoadDefaults();
						break;
					}
				}
				else if (strcmp(lfr.SecBuffer, "ID") == 0)
					newItem.petitionId = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "SAGE") == 0)
					newItem.sageCDefID = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "PETITIONER") == 0)
					newItem.petitionerCDefID = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "TIMESTAMP") == 0)
					newItem.timestamp = lfr.BlockToULongC(1);
				else if (strcmp(lfr.SecBuffer, "CATEGORY") == 0)
					newItem.category = lfr.BlockToIntC(1);
				else if (strcmp(lfr.SecBuffer, "DESCRIPTION") == 0) {
					string r = lfr.BlockToStringC(1, 0);
					Util::ReplaceAll(r, "\\r\\n", "\r\n");
					Util::ReplaceAll(r, "\\n", "\n");
					Util::SafeCopy(newItem.description, r.c_str(),
							sizeof(newItem.description));
				} else if (strcmp(lfr.SecBuffer, "RESOLUTION") == 0) {
					string r = lfr.BlockToStringC(1, 0);
					Util::ReplaceAll(r, "\\r\\n", "\r\n");
					Util::ReplaceAll(r, "\\n", "\n");
					Util::SafeCopy(newItem.resolution, r.c_str(),
							sizeof(newItem.resolution));
				} else {
					g_Logs.data->warn("Petition file %v has unknown pair %v",
							buffer, lfr.SecBuffer);
				}
			}
		}
	}
	if (newItem.petitionId != 0) {
		newItem.RunLoadDefaults();
	}
	return newItem;
}

