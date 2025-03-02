#include "Log.h"
#include "../Config.h"
#include "../DirectoryAccess.h"
#include <chrono>
#include <sstream>
#include <iostream>

//INITIALIZE_EASYLOGGINGPP

namespace el {

	boost::format& formatImpl(boost::format &f) {
		return f;
	}

	template<typename Head, typename ... Tail>
	boost::format& formatImpl(boost::format &f, Head const &head, Tail &&... tail) {
		return formatImpl(f % head, std::forward<Tail>(tail)...);
	}
}

Logger::Logger(std::string channel) {
	mChannel = channel;
}

Logger::~Logger() {
}

bool Logger::Enabled(el::Level lev) {
	// TODO
	return true;
}
void Logger::Flush(void) {
	// TODO
}

std::string Logger::Now() {
	std::stringstream ss;
	std::time_t now_t = std::chrono::system_clock::to_time_t(
	std::chrono::system_clock::now());
	ss << std::put_time(std::localtime(&now_t), "%F %T");
	return ss.str();
}

void Logger::WriteLog(const std::string severity, std::string const &msg) {
	std::cout << "[" << severity << "] " << msg << "\n";
//	using fmt::arg;
//
//	std::lock_guard<std::mutex> lock(mut_print_);
//
//	fmt::print("{severity:8}  {channel:10}  {time:12}         {msg:40}\n",
//		arg("severity", fmt::format("[{}]", severity)),
//		arg("channel", fmt::format("[{}]", mChannel)),
//		arg("time", now()), arg("msg", msg));

}

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
	mOutputToConsole = true;
}

LogManager::~LogManager() {
}

Logger* LogManager::ConfigureLogger(Logger *logger) {
	/* I don't really like how easylogging++'s heirarchical logging is arrange. This makes it
	 * behave more like the Java logging systems I am used to. With Verbose being the highest
	 * level logging, then Trace, Debug, Info, Warning, Error and finally Fatal. Logging is
	 * restricted to the set level and any that are lower in the heirarchy. I.e. Verbose will
	 * show everything.
	 *
	 * The level of Level::Unknown will turn off all logging.
	 //	 */
//	switch (mLevel) {
//	case el::Level::Info:
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "TRUE");
//		break;
//	case el::Level::Error:
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "FALSE");
//		break;
//	case el::Level::Fatal:
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "FALSE");
//		break;
//	case el::Level::Debug:
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "TRUE");
//		break;
//	case el::Level::Trace:
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "TRUE");
//		break;
//	case el::Level::Unknown:
//		logger->configurations()->set(el::Level::Global,
//				el::ConfigurationType::Enabled, "FALSE");
//		break;
//	default:
//		/* Default is like Warning. All levels have verbose exception Unknown (quiet) */
//		logger->configurations()->set(el::Level::Debug,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Error,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Fatal,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Info,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Trace,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Unknown,
//				el::ConfigurationType::Enabled, "FALSE");
//		logger->configurations()->set(el::Level::Verbose,
//				el::ConfigurationType::Enabled, "TRUE");
//		logger->configurations()->set(el::Level::Warning,
//				el::ConfigurationType::Enabled, "TRUE");
//		break;
//	}
//	logger->configurations()->set(el::Level::Global,
//			el::ConfigurationType::ToStandardOutput,
//			mOutputToConsole ? "TRUE" : "FALSE");
//	logger->reconfigure();
	return logger;
}

void LogManager::AddFlag(el::Flags flag) {
	mFlags.push_back(flag);
}

void LogManager::Init(el::Level level, bool outputToConsole,
		std::string configFilename) {
	mLevel = level;
	mOutputToConsole = outputToConsole;

	fs::path logConfig;
	auto paths = g_Config.ResolveLocalConfigurationPath();
	for (auto dir = paths.begin(); dir != paths.end(); ++dir) {
		auto filename = *dir / configFilename;
		if (fs::exists(filename))
			logConfig = filename;
	}
//	if (logConfig.empty())
//		el::Loggers::configureFromGlobal(logConfig.string().c_str());
//	else
//		el::Loggers::configureFromGlobal((
//				g_Config.ResolveLocalConfigurationPath()[0] / configFilename).string().c_str());
//
//	el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
//	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
//	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
////	el::Loggers::addFlag(el::LoggingFlag::HierarchicalLogging);
//
	server = new Logger("server");
	chat = new Logger("chat");
	cheat = new Logger("cheat");
	event = new Logger("event");
	http = new Logger("http");
	router = new Logger("router");
	leaderboard = new Logger("leaderboard");
	simulator = new Logger("simulator");
	data = new Logger("data");
	script = new Logger("script");
	cs = new Logger("cs");
	cluster = new Logger("cluster");
//	server = ConfigureLogger(Loggers::getLogger("server", true));
//	chat = ConfigureLogger(Loggers::getLogger("chat", true));
//	cheat = ConfigureLogger(Loggers::getLogger("cheat", true));
//	event = ConfigureLogger(Loggers::getLogger("event", true));
//	http = ConfigureLogger(Loggers::getLogger("http", true));
//	router = ConfigureLogger(Loggers::getLogger("router", true));
//	leaderboard = ConfigureLogger(Loggers::getLogger("leaderboard", true));
//	simulator = ConfigureLogger(Loggers::getLogger("simulator", true));
//	data = ConfigureLogger(Loggers::getLogger("data", true));
//	script = ConfigureLogger(Loggers::getLogger("script", true));
//	cs = ConfigureLogger(Loggers::getLogger("cs", true));
//	cluster = ConfigureLogger(Loggers::getLogger("cluster", true));

}

void LogManager::FlushAll() {
	server->info("Flushing all logs");
	server->Flush();
	chat->Flush();
	event->Flush();
	cheat->Flush();
	http->Flush();
	router->Flush();
	leaderboard->Flush();
	simulator->Flush();
	data->Flush();
	script->Flush();
	cluster->Flush();
}

void LogManager::CloseAll() {
	delete server;
	delete server;
	delete event;
	delete cheat;
	delete http;
	delete router;
	delete leaderboard;
	delete simulator;
	delete data;
	delete script;
	delete cluster;
}
