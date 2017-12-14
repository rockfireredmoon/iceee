/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#include "PlayerStats.h"
#include "Util.h"
#include <string.h>

//
// PlayerStatSet
//
PlayerStatSet::PlayerStatSet() {
	Clear();
}

void PlayerStatSet::Clear() {
	TotalDeaths = 0;
	TotalKills = 0;
	TotalPVPDeaths = 0;
	TotalPVPKills = 0;
}

void PlayerStatSet::CopyFrom(PlayerStatSet *other) {
	TotalDeaths = other->TotalDeaths;
	TotalKills = other->TotalKills;
	TotalPVPDeaths = other->TotalPVPDeaths;
	TotalPVPKills = other->TotalPVPKills;
}

void PlayerStatSet::Add(PlayerStatSet &other) {
	TotalDeaths += other.TotalDeaths;
	TotalKills += other.TotalKills;
	TotalPVPDeaths += other.TotalPVPDeaths;
	TotalPVPKills += other.TotalPVPKills;
}

void PlayerStatSet::SaveToStream(FILE *output) {
	Util::WriteInteger(output, "TotalKills", TotalKills);
	Util::WriteInteger(output, "TotalDeaths", TotalDeaths);
	Util::WriteInteger(output, "TotalPVPKills", TotalPVPKills);
	Util::WriteInteger(output, "TotalPVPDeaths", TotalPVPDeaths);
}

bool PlayerStatSet::LoadFromStream(FileReader &fr) {
	char *NameBlock = fr.BlockToStringC(0, Case_Upper);
	if(strcmp(NameBlock, "TOTALKILLS") == 0)
		TotalKills = fr.BlockToIntC(1);
	else if(strcmp(NameBlock, "TOTALDEATHS") == 0)
		TotalDeaths = fr.BlockToIntC(1);
	else if(strcmp(NameBlock, "TOTALPVPKILLS") == 0)
		TotalPVPKills = fr.BlockToIntC(1);
	else if(strcmp(NameBlock, "TOTALPVPDEATHS") == 0)
		TotalPVPDeaths = fr.BlockToIntC(1);
	else
		return false;
	return true;
}

void PlayerStatSet :: ReadFromJSON(Json::Value &value)
{
	TotalKills = value.get("kills", "0").asInt();
	TotalDeaths = value.get("deaths", "0").asInt();
	TotalPVPKills = value.get("pvpKills", "0").asInt();
	TotalPVPDeaths = value.get("pvpDeaths", "0").asInt();
}

void PlayerStatSet :: WriteToJSON(Json::Value &value)
{
	value["kills"] = TotalKills;
	value["deaths"] = TotalDeaths;
	value["pvpKills"] = TotalPVPKills;
	value["pvpDeaths"] = TotalPVPDeaths;
}
