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
#include "JsonHelpers.h"
#include "SquirrelObjects.h"

namespace JsonHelpers {

int SquirrelAppearanceToJSONAppearance(string source, Json::Value &value) {

	string currentAppearance = source;
	size_t pos = currentAppearance.find(":");
	if(pos == string::npos)	{
		g_Log.AddMessageFormat("Could not parse exist appearance. %s", currentAppearance.c_str());
		return -1;
	}
	string prefix = currentAppearance.substr(0, pos + 1);
	currentAppearance = "this.a <- " + currentAppearance.substr(pos + 1, currentAppearance.size()) + ";";


	// TODO shared vm
	HSQUIRRELVM vm = sq_open(g_Config.SquirrelVMStackSize);
	Sqrat::Script script(vm);

	g_Log.AddMessageFormat("Adjusting appearance. %s", currentAppearance.c_str());

	script.CompileString(_SC(currentAppearance.c_str()));
	if (Sqrat::Error::Occurred(vm)) {
		g_Log.AddMessageFormat("Failed to compile appearance. %s", Sqrat::Error::Message(vm).c_str());
		return -1;
	}
	script.Run();

	Sqrat::RootTable rootTable = Sqrat::RootTable(vm);
	Sqrat::Object placeholderObject = rootTable.GetSlot(_SC("a"));
	Sqrat::Table table = placeholderObject.Cast<Sqrat::Table>();

	Squirrel::JsonPrinter p;
	p.PrintTable(value, table);

	return 0;
}
}
