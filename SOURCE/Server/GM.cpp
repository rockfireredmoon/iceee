#include "GM.h"
#include "FileReader.h"
#include "Util.h"

#include "Character.h"
#include "Config.h"
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
	Platform::MakeDirectory(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"));
	Platform::MakeDirectory(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Pending"));
	Platform::MakeDirectory(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Closed"));
}

PetitionManager::~PetitionManager() {
}


bool PetitionManager::Take(int petitionId, int sageCharacterId) {
	char idBuf[32];
	char idTxtBuf[32];
	Util::SafeFormat(idBuf, sizeof(idBuf), "%d", sageCharacterId);
	Util::SafeFormat(idTxtBuf, sizeof(idTxtBuf), "%d.txt", sageCharacterId);

	std::string petfile = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), idBuf);
	if(!Platform::DirExists(petfile))
		Platform::MakeDirectory(petfile);

	std::string srcfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Pending"), idTxtBuf);
	std::string targfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), idBuf), idTxtBuf);
	if(Platform::FileCopy(srcfile, targfile) == 0 && remove(srcfile.c_str()) == 0)
		return true;
	g_Logs.data->error("Failed to take petition to %v", srcfile);
	return false;
}

bool PetitionManager::Untake(int petitionId, int sageCharacterId) {

	char idBuf[32];
	char idTxtBuf[32];
	Util::SafeFormat(idBuf, sizeof(idBuf), "%d", sageCharacterId);
	Util::SafeFormat(idTxtBuf, sizeof(idTxtBuf), "%d.txt", sageCharacterId);

	std::string srcfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), idBuf), idTxtBuf);
	std::string targfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Pending"), idTxtBuf);

	if(Platform::FileCopy(srcfile, targfile) == 0 && remove(srcfile.c_str()) == 0)
		return true;
	g_Logs.data->error("Failed to untake petition to %v", srcfile);
	return false;
}

bool PetitionManager::Close(int petitionId, int sageCharacterId) {
	char idBuf[32];
	char idTxtBuf[32];
	Util::SafeFormat(idBuf, sizeof(idBuf), "%d", sageCharacterId);
	Util::SafeFormat(idTxtBuf, sizeof(idTxtBuf), "%d.txt", sageCharacterId);

	std::string srcfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), idBuf), idTxtBuf);
	std::string targfile = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Closed"), idTxtBuf);

	if(Platform::FileCopy(srcfile, targfile) == 0 && remove(srcfile.c_str()) == 0)
		return true;

	g_Logs.data->error("Failed to close petition to %v", srcfile);
	return false;
}

int PetitionManager::NewPetition(int petitionerCDefID, int category, const char *description) {
	char idTxtBuf[256];
	int id = NextPetitionID++;
	Util::SafeFormat(idTxtBuf, sizeof(idTxtBuf), "%d.txt", id);
	std::string filename = Platform::JoinPath(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Pending"), idTxtBuf);

	g_Logs.data->info("Saving petition to %v.", filename);
	FILE *output = fopen(filename.c_str(), "wb");
	if (output == NULL) {
		g_Logs.data->error("Saving petition could not open: %v", filename);
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

	std::string dir = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), "Pending");
	FillPetitions(&v, dir, PENDING);
	if(sageCharacterID != 0) {
		char idbuf[32];
		Util::SafeFormat(idbuf, sizeof(idbuf), "%d", sageCharacterID);
		dir = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "Petitions"), idbuf);
		if(Platform::DirExists(dir)) {
			FillPetitions(&v, dir, TAKEN);
		}
	}
	return v;
}


void PetitionManager::FillPetitions(std::vector<Petition> *petitions, std::string path, PetitionStatus status) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
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

Petition PetitionManager::Load(std::string path, int id) {
	char buffer[256];
	Util::SafeFormat(buffer, sizeof(buffer), "%d.txt", id);
	std::string filename = Platform::JoinPath(path, buffer);
	FileReader lfr;
	Petition newItem;
	if (lfr.OpenText(filename.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", filename);
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

