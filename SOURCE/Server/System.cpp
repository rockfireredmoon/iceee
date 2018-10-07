#include "System.h"


#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <stdlib.h>

System::SystemService g_System;

namespace System {
//

SystemService::SystemService() {
}

SystemService::~SystemService() {
}

std::string SystemService::ExecuteCapture(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


int SystemService::Execute(const char* cmd) {
	int st = WEXITSTATUS(system(cmd));
	return st;
}

}
