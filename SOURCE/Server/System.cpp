#include "System.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

System::SystemService g_System;

namespace System {
//

SystemService::SystemService() {
}

SystemService::~SystemService() {
}

std::string SystemService::ExecuteCapture(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	return result;
}

int SystemService::Execute(const char* cmd) {
	return WEXITSTATUS(system(cmd));
}

}
