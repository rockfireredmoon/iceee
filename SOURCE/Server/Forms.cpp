#include "Forms.h"
#include "Util.h"
#include "StringList.h"
#include "Character.h"

void FormDefinition::Clear(void) {
	mTitle = "";
	mRows.clear();
	mId = 0;
}

void FormDefinition::AddRow(FormRow row) {
	mRows.push_back(row);
}

void FormRow::Clear(void) {
	mGroup = "";
	mItems.clear();
}

void FormRow::AddItem(FormItem item) {
	mItems.push_back(item);
}

void FormItem::Clear(void) {
	mName = "";
	mValue = "";
	mType = 0;
}


