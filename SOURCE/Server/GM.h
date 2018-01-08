#pragma once
#ifndef GM_H
#define GM_H


#include <vector>
#include <string>
#include "Entities.h"

static std::string KEYPREFIX_PETITION = "Petition";
static std::string ID_NEXT_PETITION_ID = "NextPetitionID";
static std::string LISTPREFIX_PENDING_PETITIONS = "PendingPetitions";
static std::string LISTPREFIX_TAKEN_PETITIONS = "TakenPetitions";
static std::string LISTPREFIX_CLOSED_PETITIONS = "ClosedPetitions";

enum PetitionStatus {
	PENDING = 1,
	TAKEN = 2,
	CLOSED = 3
};

class Petition: public AbstractEntity {
public:
	int petitionId;
	int status;
	int category;
	std::string description;
	int petitionerCDefID;
	int sageCDefID;
	unsigned long timestamp;
	std::string resolution;

	Petition();
	~Petition();

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
	void Clear(void);
};

class PetitionManager
{
public:
	PetitionManager();
	~PetitionManager();


	bool Take(int pendingPetitionId, int sageCharacterID);
	bool Untake(int pendingPetitionId, int sageCharacterID);
	bool Close(int pendingPetitionId, int sageCharacterID);
	std::vector<Petition> GetPetitions(int sageCharacterID);
	int NewPetition(int petitionerCDefID, int category, const char *description);
private:
	void FillPetitions(std::vector<std::string> &in, std::vector<Petition> &out);
};

extern PetitionManager g_PetitionManager;


#endif /* GM_H */
