#pragma once
#ifndef GM_H
#define GM_H


#include <vector>
#include <string>

enum PetitionStatus {
	PENDING = 1,
	TAKEN = 2
};

struct Petition
{
	int petitionId;
	int status;
	int category;
	char description[4096];
	int petitionerCDefID;
	int sageCDefID;
	unsigned long timestamp;
	char resolution[4096];

	Petition() { Clear(); }
	~Petition() { }
	void Clear(void);
	void RunLoadDefaults(void);
};

class PetitionManager
{
public:
	int NextPetitionID;

	PetitionManager();
	~PetitionManager();


	bool Take(int pendingPetitionId, int sageCharacterID);
	bool Untake(int pendingPetitionId, int sageCharacterID);
	bool Close(int pendingPetitionId, int sageCharacterID);
	std::vector<Petition> GetPetitions(int sageCharacterID);
	int NewPetition(int petitionerCDefID, int category, const char *description);
private:
	Petition Load(const char *path, int id);
	void FillPetitions(std::vector<Petition> *petitions, const char *path, PetitionStatus status);
};

extern PetitionManager g_PetitionManager;


#endif /* GM_H */
