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

#include <sstream>
#include <mutex>
#include <vector>
#include <boost/format.hpp>

namespace el {
enum Flags {
	ImmediateFlush
};
enum Level {
	Verbose, Fatal, Error, Warning, Info, Trace, Debug, Unknown
};

boost::format& formatImpl(boost::format &f);

template<typename Head, typename ... Tail>
boost::format& formatImpl(boost::format &f, Head const &head, Tail &&... tail);
}

class Logger {
public:
	Logger(std::string channel);
	~Logger();

	template<typename ... Args>
	void trace(const std::string msg, Args ... args) {
		WriteLog("TRACE", msg);
	}

	template<typename ... Args>
	void debug(const std::string msg, Args ... args) {
//		basic_log("DEBUG", fmt::format(msg, args...));
		WriteLog("DEBUG", msg);
	}

	template<typename ... Args>
	void warn(const std::string msg, Args ... args) {
//		basic_log("WARN", fmt::format(msg, args...));
		WriteLog("WARN", msg);
	}

	template<typename ... Args>
	void error(const std::string msg, Args ... args) {
//		basic_log("ERROR", fmt::format(msg, args...));
		WriteLog("ERROR", msg);
	}

	template<typename ... Args>
	void info(const std::string msg, Args ... args) {
//		WriteLog("INFO", fmt::format(msg, args...));
		boost::format f(std::move(msg));
//		auto s = el::formatImpl(f, std::forward<Args>(args)...);
//		WriteLog("INFO", el::formatImpl(f, std::forward<Args>(args)...).str());
		WriteLog("INFO", msg);
	}

	template<typename ... Args>
	void fatal(const std::string msg, Args ... args) {
//		WriteLog("FATAL", fmt::format(msg, args...));
		WriteLog("FATAL", msg);
	}

	template<typename ... Args>
	void verbose(uint8_t verbosity, const std::string msg, Args ... args) {
//		WriteLog("VERBOSE", fmt::format(msg, args...));
		WriteLog("VERBOSE", msg);
	}

	void Flush(void);

	bool Enabled(el::Level lev);

private:
	std::string Now();
	void WriteLog(const std::string severity, std::string const &msg);

private:
	std::mutex mut_print_;
	std::string mChannel;
};

class LogManager {
public:
	LogManager();
	~LogManager();
	void Init(el::Level level, bool outputToConsole,
			std::string configFilename);
	void FlushAll();
	void CloseAll();
	void AddFlag(el::Flags flag);
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
	Logger *cluster;
private:
	el::Level mLevel;
	std::vector<el::Flags> mFlags;
	bool mOutputToConsole;
	Logger* ConfigureLogger(Logger *logger);
};

extern LogManager g_Logs;

#endif
