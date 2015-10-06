#include "SquirrelObjects.h"
#include "Util.h"

namespace Squirrel {

	void Printer::PrintTable(std::string *result, Sqrat::Table table) {
		result->append("{");

		int i = 0;
		Sqrat::Table::iterator it;
		while(table.Next(it)) {
			if(i > 0) {
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
		for(int i = 0 ; i < array.GetSize(); i++) {
			if(i > 0) {
				result->append(",");
			}
			Sqrat::Object obj = array.GetSlot(SQInteger(i));
			PrintValue(result, obj);
		}
		result->append("]");
	}

	void Printer::PrintValue(std::string *result, Sqrat::Object object) {
		if(object.IsNull()) {
			result->append("null");
		}
		else {
			if(object.GetType() == OT_TABLE) {
				PrintTable(result, object.Cast<Sqrat::Table>());
			}
			else if(object.GetType() == OT_ARRAY) {
				PrintArray(result, object.Cast<Sqrat::Array>());
			}
			else if(object.GetType() == OT_INTEGER) {
				Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer), "%lu", object.Cast<unsigned long>());
				result->append(mPrintBuffer);
			}
			else if(object.GetType() == OT_STRING) {
				result->append("\"" + object.Cast<std::string>() + "\"");
			}
			else if(object.GetType() == OT_BOOL) {
				result->append(object.Cast<bool>() ? "true" : "false");
			}
			else if(object.GetType() == OT_FLOAT) {
				Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer), "%f", object.Cast<double>());
				result->append(mPrintBuffer);
			}
			else {
				Vector3 *vec3 = object.Cast<Vector3*>();
				if(vec3 != NULL) {
					Util::SafeFormat(mPrintBuffer, sizeof(mPrintBuffer), "Vector3(%f,%f,%f)", vec3->mX, vec3->mY, vec3->mZ);
					result->append(mPrintBuffer);
				}

			}
		}
	}
}
