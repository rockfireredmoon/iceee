#pragma once
#ifndef FORMS_H
#define FORMS_H

#include <vector>
#include <string>
#include <map>

enum FormItemType
{
	BLANK,
	LABEL,
	TEXTFIELD,
	CHECKBOX,
	BUTTON
};

class FormItem {
public:
	std::string mName;
	int mType;
	std::string mValue;
	int mCells;
	int mWidth;
	std::string mStyle;

	FormItem(const FormItem &i) {
		mName = i.mName;
		mType = i.mType;
		mValue = i.mValue;
		mCells = i.mCells;
		mWidth = i.mWidth;
		mStyle = i.mStyle;
	}

	FormItem() {
		mName = "";
		mValue = "";
		mType = 0;
		mCells = 1;
		mWidth = 0;
		mStyle = "";
	}

	FormItem(std::string _name, int _type) {
		mName = _name;
		mType = _type;
		mValue = "";
		mCells = 1;
		mWidth = 0;
		mStyle = "";
	}

	FormItem(std::string _name, int _type, std::string _value) {
		mName = _name;
		mType = _type;
		mValue = _value;
		mCells = 1;
		mWidth = 0;
		mStyle = "";
	}

	FormItem(std::string _name, int _type, std::string _value, int width) {
		mName = _name;
		mType = _type;
		mValue = _value;
		mCells = 1;
		mWidth = width;
		mStyle = "";
	}

	~FormItem() {
	}
	void Clear(void);
};

class FormRow {
public:
	std::string mGroup;
	std::vector<FormItem> mItems;
	int mHeight;

	FormRow() {
		mGroup = "";
		mHeight = 0;
	}
	FormRow(std::string _group) {
		mGroup = _group;
		mHeight = 0;
	}

	FormRow(const FormRow &r) {
		mGroup = r.mGroup;
		mItems = r.mItems;
		mHeight = r.mHeight;
	}

	~FormRow() {
	}
	void AddItem(FormItem item);
	void Clear(void);
};

class FormDefinition {
public:
	std::string mTitle;
	std::string mDescription;
	std::vector<FormRow> mRows;
	int mId;

	FormDefinition() {
		mTitle = "";
		mId = 0;
		mDescription == "";
	}

	FormDefinition(const FormDefinition &f) {
		mTitle = f.mTitle;
		mDescription = f.mDescription;
		mId = f.mId;
		mRows = f.mRows;
	}

	FormDefinition(std::string _title) {
		mTitle = _title;
		mId = 0;
		mDescription == "";
	}

	FormDefinition(std::string _title, std::string _description) {
		mTitle = _title;
		mDescription = _description;
		mId = 0;
	}

	~FormDefinition() {
	}
	void AddRow(FormRow row);
	void Clear(void);
};

#endif /* FORMS_H */
