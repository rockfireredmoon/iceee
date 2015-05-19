#pragma once
#ifndef SQUIRREL_H
#define SQUIRREL_H

#include "sqrat.h"
#include <string>

namespace Squirrel {

class Printer {
public:
	void PrintTable(std::string *result, Sqrat::Table table);
	void PrintArray(std::string *result, Sqrat::Array array);
	void PrintValue(std::string *result, Sqrat::Object array);
};


}
#endif //#define SQUIRREL_H
