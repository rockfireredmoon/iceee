#include <Cluster.h>
#include <Config.h>
#include <Components.h>
#include <Ability2.h>
#include <VirtualItem.h>
#include <util/Log.h>
#include <curl/curl.h>
#include <dirent.h>
#include "Item.h"
#include "Account.h"
#include "StringUtil.h"

INITIALIZE_EASYLOGGINGPP

char GAuxBuf[1024]; //Note, if this size is modified, change all "extern" references
char GSendBuf[32767]; //Note, if this size is modified, change all "extern" references

int main(int argc, char *argv[]) {

	if (PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
		return 0;
	}

	el::Level lvl = el::Level::Warning;
	std::vector<std::string> options;
	std::string command;
	for (int i = 1; i < argc; i++) {
		if(command == "") {
			if (strcmp(argv[i], "-c") == 0) {
				g_Config.LocalConfigurationPath = argv[++i];
			} else if (strcmp(argv[i], "-d") == 0) {
				lvl = el::Level::Debug;
			} else if (strcmp(argv[i], "-i") == 0) {
				lvl = el::Level::Info;
			} else if (strcmp(argv[i], "-q") == 0) {
				lvl = el::Level::Unknown;
			} else {
				command = argv[i];
			}
		}
		else {
			options.push_back(argv[i]);
		}
	}

	g_Logs.Init(lvl);
	g_Logs.data->info("EEAccount");

	curl_global_init(CURL_GLOBAL_DEFAULT);
	g_PlatformTime.Init();

	LoadConfig(
			Platform::JoinPath(g_Config.ResolveLocalConfigurationPath(),
					"ServerConfig.txt"));

	if (g_ClusterManager.Init(
			Platform::JoinPath(g_Config.ResolveLocalConfigurationPath(),
					"Cluster.txt")) < 0) {
		g_Logs.data->error("Failed to connect to cluster.");
		return 1;
	}

	/* We need some static data */
//	g_ItemManager.LoadData();
//	g_AbilityManager.LoadData();

	if(command == "create") {
		if(options.size() != 3) {
			g_Logs.data->error("'create' requires 3 arguments. <userName> <password> <groveName>.");
			return 1;
		}
		std::string regkey = Util::RandomStr(16, false);
		g_AccountManager.ImportKey(regkey.c_str());
		int retval = g_AccountManager.CreateAccount(options[0].c_str(), options[1].c_str(), regkey.c_str(), options[2].c_str());
		if(retval == AccountManager::ACCOUNT_SUCCESS) {
			g_Logs.data->info("Create account %v [%v]", options[0], options[2]);
		}
		else {
			g_Logs.data->error("Failed to create account %v [%v]. %v", options[0], options[2], g_AccountManager.GetErrorMessage(retval));
			return 1;
		}
	}
	else {
		g_Logs.data->error("Unknown operation '%v'", command);
		return 1;
	}

	//
	g_ClusterManager.Shutdown(true);
}
