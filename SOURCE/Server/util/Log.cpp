#include "Log.h"
#include "../Config.h"
#include "../DirectoryAccess.h"

LogManager g_Logs;

LogManager::LogManager() {
	server = NULL;
	chat = NULL;
	cheat = NULL;
	event = NULL;
	http = NULL;
	router = NULL;
	leaderboard = NULL;
	simulator = NULL;
	data = NULL;
	script = NULL;
	cs = NULL;
	cluster = NULL;
	mLevel = el::Level::Warning;
}

LogManager::~LogManager() {
}

Logger * LogManager::ConfigureLogger(Logger *logger) {
	/* I don't really like how easylogging++'s heirarchical logging is arrange. This makes it
	 * behave more like the Java logging systems I am used to. With Verbose being the highest
	 * level logging, then Trace, Debug, Info, Warning, Error and finally Fatal. Logging is
	 * restricted to the set level and any that are lower in the heirarchy. I.e. Verbose will
	 * show everything.
	 *
	 * The level of Level::Unknown will turn off all logging.
	 */
	switch(mLevel) {
	case el::Level::Info:
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "TRUE");
		break;
	case el::Level::Error:
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "FALSE");
		break;
	case el::Level::Fatal:
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "FALSE");
		break;
	case el::Level::Debug:
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "TRUE");
		break;
	case el::Level::Trace:
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "TRUE");
		break;
	case el::Level::Unknown:
		logger->configurations()->set(el::Level::Global, el::ConfigurationType::Enabled, "FALSE");
		break;
	default:
		/* Default is like Warning. All levels have verbose exception Unknown (quiet) */
		logger->configurations()->set(el::Level::Debug, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Error, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Fatal, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Info, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Trace, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Unknown, el::ConfigurationType::Enabled, "FALSE");
		logger->configurations()->set(el::Level::Verbose, el::ConfigurationType::Enabled, "TRUE");
		logger->configurations()->set(el::Level::Warning, el::ConfigurationType::Enabled, "TRUE");
		break;
	}
	logger->reconfigure();
	return logger;
}

void LogManager::Init(el::Level level) {
	mLevel = level;
	el::Loggers::configureFromGlobal(Platform::JoinPath(g_Config.ResolveLocalConfigurationPath(), "LogConfig.txt").c_str());

	el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
//	el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);

	server = ConfigureLogger(Loggers::getLogger("server", true));
	chat = ConfigureLogger(Loggers::getLogger("chat", true));
	cheat = ConfigureLogger(Loggers::getLogger("cheat", true));
	event = ConfigureLogger(Loggers::getLogger("event", true));
	http = ConfigureLogger(Loggers::getLogger("http", true));
	router = ConfigureLogger(Loggers::getLogger("router", true));
	leaderboard = ConfigureLogger(Loggers::getLogger("leaderboard", true));
	simulator = ConfigureLogger(Loggers::getLogger("simulator", true));
	data = ConfigureLogger(Loggers::getLogger("data", true));
	script = ConfigureLogger(Loggers::getLogger("script", true));
	cs = ConfigureLogger(Loggers::getLogger("cs", true));
	cluster = ConfigureLogger(Loggers::getLogger("cluster", true));


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
	cluster->flush();
}

void LogManager::CloseAll() {
//	server->
	//server->flush();
}
