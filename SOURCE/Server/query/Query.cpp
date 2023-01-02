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

#include "Query.h"
#include "../Util.h"

//
// CommandHandler
//
QueryHandler::~QueryHandler() {
}

//
// CommandManager
//

QueryManager g_QueryManager;

QueryManager::QueryManager() {
}

QueryManager::~QueryManager() {
}

QueryHandler *QueryManager::getQueryHandler(std::string command) {
	if (queryHandlers.find(command) == queryHandlers.end())
		return NULL;
	return queryHandlers[command];
}

QueryHandler *QueryManager::getLobbyQueryHandler(std::string command) {
	if (lobbyQueryHandlers.find(command) == lobbyQueryHandlers.end())
		return NULL;
	return lobbyQueryHandlers[command];
}

//
// QueryResponse
//

QueryResponse::QueryResponse(int queryID) {
	mQueryID = queryID;
	mBufferSet = false;
	mBuffer = nullptr;
	mCommitted = false;
}

QueryResponse::QueryResponse(int queryID, char *buffer) {
	mQueryID = queryID;
	mBuffer = buffer;
	mBufferSet = true;
	mCommitted = false;
}

QueryResponse::~QueryResponse() {
}

vector<string>* QueryResponse::Row() {
	vector<string> l;
	mResponse.push_back(l);
	return &mResponse.back();
}

void QueryResponse::AddRow(initializer_list<reference_wrapper<string>> list) {
	auto l = Row();
	for(auto elem : list) {
		l->push_back(elem.get());
	}
}

unsigned int QueryResponse::Data() {
	return Write(mBuffer);
}

unsigned int QueryResponse::String(const string &str) {
	if(mResponse.size() > 0) {
		throw logic_error("This response contains data.");
	}
	if(!mBufferSet)  {
		throw logic_error("Not constructed with buffer.");
	}
	Row()->push_back(str);
	return Write(mBuffer);
}

unsigned int QueryResponse::Error(const string &err) {
	if(mResponse.size() > 0) {
		throw logic_error("This response contains data.");
	}
	if(!mBufferSet) {
		throw logic_error("Not constructed with buffer.");
	}
	if(mCommitted) {
			throw logic_error("Already committed");
	}
	int wpos = 0;
	wpos += PutByte(&mBuffer[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&mBuffer[wpos], 0);      //Message size
	wpos += PutInteger(&mBuffer[wpos], mQueryID);  //Query response index
	wpos += PutShort(&mBuffer[wpos],  0x7000);   //Negative number indicates error
	wpos += PutStringUTF(&mBuffer[wpos], err);
	PutShort(&mBuffer[1], wpos - 3);
	mCommitted = true;
	return wpos;
}

unsigned int QueryResponse::Null() {
	if(mResponse.size() > 0) {
		throw logic_error("This response contains data.");
	}
	if(!mBufferSet)  {
		throw logic_error("Not constructed with buffer.");
	}
	return Write(mBuffer);
}

unsigned int QueryResponse::Write(char *buffer) {
	if(mCommitted) {
		throw logic_error("Already committed");
	}
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);      //Message size
	wpos += PutInteger(&buffer[wpos], mQueryID);  //Query response index
	wpos += PutShort(&buffer[wpos], mResponse.size());
	for(auto row : mResponse) {
		wpos += PutByte(&buffer[wpos], row.size());
		for(auto col : row) {
			wpos += PutStringUTF(&buffer[wpos], col);
		}
	}
	PutShort(&buffer[1], wpos - 3);
	mCommitted = true;
	return wpos;
}

