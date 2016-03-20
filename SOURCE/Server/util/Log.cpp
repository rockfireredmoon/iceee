#include "Log.h"

LogManager g_Logs;

LogManager::LogManager() {

	el::Loggers::configureFromGlobal("LogConfig.txt");
	el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
	el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
	el::Loggers::setLoggingLevel(el::Level::Info);

	server = Loggers::getLogger("server", true);
	chat = Loggers::getLogger("chat", true);
	cheat = Loggers::getLogger("cheat", true);
	event = Loggers::getLogger("event", true);
	http = Loggers::getLogger("http", true);
	router = Loggers::getLogger("router", true);
	leaderboard = Loggers::getLogger("leaderboard", true);
	simulator = Loggers::getLogger("simulator", true);
	data = Loggers::getLogger("data", true);
	script = Loggers::getLogger("script", true);
	cs = Loggers::getLogger("cs", true);

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
