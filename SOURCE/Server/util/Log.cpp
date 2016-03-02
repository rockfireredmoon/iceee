#include "Log.h"

LogManager g_Logs;

LogManager::LogManager() {

	el::Loggers::configureFromGlobal("LogConfig.txt");
	el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

	server = Loggers::getLogger("server", false);
	chat = Loggers::getLogger("chat", false);
	cheat = Loggers::getLogger("cheat", false);
	event = Loggers::getLogger("event", false);
	http = Loggers::getLogger("http", false);
	router = Loggers::getLogger("router", false);
	leaderboard = Loggers::getLogger("leaderboard", false);
	simulator = Loggers::getLogger("simulator", false);
	data = Loggers::getLogger("data", false);
	script = Loggers::getLogger("script", false);

}

LogManager::~LogManager() {
}

void LogManager::FlushAll() {
	server->info("Flushing all logs");
	server->flush();
	chat->flush();
	event->flush();
	cheat->flush();
	http->flush();
	router->flush();
	leaderboard->flush();
	simulator->flush();
	data->flush();
	script->flush();
}

void LogManager::CloseAll() {
//	server->
	//server->flush();
}
