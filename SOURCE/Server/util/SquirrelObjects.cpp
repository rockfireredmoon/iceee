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
#include "SquirrelObjects.h"
#include "../Util.h"

namespace Squirrel {

//
// Printer
//
void Printer::PrintTable(std::string *result, Sqrat::Table table) {
	result->append("{");

	int i = 0;
	Sqrat::Table::iterator it;
	while (table.Next(it)) {
		if (i > 0) {
			result->append(",");
		}
		result->append("[\"");
		Sqrat::Object o(it.getKey(), table.GetVM());
		result->append(o.Cast<std::string>());
		result->append("\"]=");
		Sqrat::Object obj(it.getValue(), table.GetVM());
		PrintValue(result, obj);
		i++;
	}

	result->append("}");
}

void Printer::PrintArray(std::string *result, Sqrat::Array array) {
	result->append("[");
	for (int i = 0; i < array.GetSize(); i++) {
		if (i > 0) {
			result->append(",");
		}
		Sqrat::Object obj = array.GetSlot(SQInteger(i));
		PrintValue(result, obj);
	}
	result->append("]");
}

void Printer::PrintValue(std::string *result, Sqrat::Object object) {
	if (object.IsNull()) {
		result->append("null");
	} else {
		if (object.GetType() == OT_TABLE) {
			PrintTable(result, object.Cast<Sqrat::Table>());
		} else if (object.GetType() == OT_ARRAY) {
			PrintArray(result, object.Cast<Sqrat::Array>());
		} else if (object.GetType() == OT_INTEGER) {
			Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer), "%lu",
					object.Cast<unsigned long>());
			result->append(mPrintBuffer);
		} else if (object.GetType() == OT_STRING) {
			result->append("\"" + object.Cast<std::string>() + "\"");
		} else if (object.GetType() == OT_BOOL) {
			result->append(object.Cast<bool>() ? "true" : "false");
		} else if (object.GetType() == OT_FLOAT) {
			Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer), "%f",
					object.Cast<double>());
			result->append(mPrintBuffer);
		} else {
			Vector3 *vec3 = object.Cast<Vector3*>();
			if (vec3 != NULL) {
				Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer),
						"Vector3(%f,%f,%f)", vec3->mX, vec3->mY, vec3->mZ);
				result->append(mPrintBuffer);
			}

		}
	}
}

//
// JsonPrinter
//
void JsonPrinter::PrintArray(Json::Value &value, Sqrat::Array array) {
	for (int i = 0; i < array.GetSize(); i++) {
		Sqrat::Object obj = array.GetSlot(SQInteger(i));
		value.append(PrintValue(obj));
	}
}

Json::Value JsonPrinter::PrintValue(Sqrat::Object object) {
	if(!object.IsNull()) {
		if(object.GetType() == OT_TABLE) {
			Json::Value v;
			PrintTable(v, object.Cast<Sqrat::Table>());
			return v;
		}
		else if(object.GetType() == OT_ARRAY) {
			Json::Value v(Json::arrayValue);
			PrintArray(v, object.Cast<Sqrat::Array>());
			return v;
		}
		else if(object.GetType() == OT_INTEGER) {
			return Json::UInt64(object.Cast<unsigned long>());
		}
		else if(object.GetType() == OT_STRING) {
			return Json::Value(object.Cast<std::string>());
		}
		else if(object.GetType() == OT_BOOL) {
			return Json::Value(object.Cast<bool>());
		}
		else if(object.GetType() == OT_FLOAT) {
			return Json::Value(object.Cast<double>());
		}
		else {
			Vector3 *vec3 = object.Cast<Vector3*>();
			if(vec3 != NULL) {
				Json::Value v3;
				v3["x"] = vec3->mX;
				v3["y"] = vec3->mY;
				v3["z"] = vec3->mZ;
				return v3;
			}
		}
	}
	Json::Value v;
	return v;
}

void JsonPrinter::PrintTable(Json::Value &value, Sqrat::Table table) {
	Sqrat::Table::iterator it;
	while(table.Next(it)) {
		Sqrat::Object o(it.getKey(), table.GetVM());
		std::string key = o.Cast<std::string>();
		Sqrat::Object obj(it.getValue(), table.GetVM());
		value[key] = PrintValue(obj);
	}
}


}
