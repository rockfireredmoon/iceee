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
#ifndef LOGS
#define LOGS

#include "easylogging++.h"

using el::Logger;
using el::Loggers;

class LogManager {
public:
	LogManager();
	~LogManager();
	void FlushAll();
	void CloseAll();
	Logger *server;
	Logger *chat;
	Logger *http;
	Logger *event;
	Logger *cheat;
	Logger *leaderboard;
	Logger *router;
	Logger *simulator;
	Logger *data;
	Logger *script;
	Logger *cs;
};

extern LogManager g_Logs;

#endif
