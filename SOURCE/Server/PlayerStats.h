#ifndef PLAYERSTATS_H
#define PLAYERSTATS_H

#include <string>
#include <stdio.h>
#include "FileReader.h"
#include "Entities.h"
#include "json/json.h"

class PlayerStatSet {
public:
	unsigned int TotalKills;
	unsigned int TotalDeaths;
	unsigned int TotalPVPKills;
	unsigned int TotalPVPDeaths;

	PlayerStatSet();

	void Add(PlayerStatSet &other);
	void CopyFrom(PlayerStatSet *other);
	void Clear();
	void SaveToStream(FILE *output);
	void WriteEntity(AbstractEntityWriter *writer);
	void ReadEntity(AbstractEntityReader *reader);
	bool LoadFromStream(FileReader &fr);
	void ReadFromJSON(Json::Value &value);
	void WriteToJSON(Json::Value &value);
};

#endif //PLAYERSTATS_H
