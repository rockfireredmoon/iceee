#pragma once
#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>

namespace System
{

class SystemService
{
public:
	SystemService();
	~SystemService();
	std::string ExecuteCapture(const char* cmd);
	int Execute(const char* cmd);
};

} //namespace System

extern System::SystemService g_System;

#endif //#ifndef SYSTEM_H
