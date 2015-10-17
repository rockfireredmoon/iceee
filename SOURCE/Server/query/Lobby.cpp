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
 
#include "Lobby.h"
#include "../Util.h"

int LobbyPingHandler::handleCommand(SimulatorThread *sim) {
	return PrepExt_QueryResponseString(sim->SendBuf, sim->query.ID, "OK");
}

int AccountTrackingHandler::handleCommand(SimulatorThread *sim) {
	/* Query: account.tracking
	   Args : 0 = Seems to be a bit flag corresponding to the following
	          screens:
	          1 = Character Selection
	          2 = Character Creation Page 1 (Gender/Race/Body/Face)
	          4 = Character Creation Page 2 (Detail Colors/Size)
	          8 = Character Creation Page 3 (Class)
	          16 = Character Creation Page 4 (Name, Clothing)
	   Notes: Doesn't seem to be sent in 0.6.0 or 0.8.9, but is sent
	          in 0.8.6.  The proper response for this query is unknown.
	          The query is sent in any forward page changes, but only sends for
			  back pages from 4 to 16.
	*/
	int value = 0;
	if(sim->query.argCount >= 1)
		value = atoi(sim->query.args[0].c_str());
	if(value >= 2)
		sim->characterCreation = true;
	sim->WritePos = PrepExt_QueryResponseString(sim->SendBuf, sim->query.ID, "OK");
	sim->PendingSend = true;
}
