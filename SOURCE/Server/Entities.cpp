#include "Entities.h"
#include "Components.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "util/Log.h"
#include "Util.h"
#include "StringUtil.h"
#include <stdlib.h>

using namespace std;

//
// Abstract player class for scripts that run in instances. This includes instance scripts
// and AI scripts
//
AbstractEntity::AbstractEntity() {
}

AbstractEntity::~AbstractEntity() {
}

//
//AbstractEntityReader
//

AbstractEntityReader::AbstractEntityReader() {
	mSection = "";
	mID = "";
	mCatalog = "";
	mMaxDepth = 0;
	mFlat = false;
}

AbstractEntityReader::~AbstractEntityReader() {
}

int AbstractEntityReader::ValueInt(const string &key,
		const int defaultValue) {
	return atoi(Value(key, StringUtil::Format("%d", defaultValue)).c_str());
}

float AbstractEntityReader::ValueFloat(const string &key,
		const float defaultValue) {
	return atof(Value(key, StringUtil::Format("%f", defaultValue)).c_str());
}

unsigned long AbstractEntityReader::ValueULong(const string &key,
		const unsigned long defaultValue) {
	return strtoul(Value(key, StringUtil::Format("%lu", defaultValue)).c_str(), NULL, 0);
}

bool AbstractEntityReader::ValueBool(const string &key,
		const bool defaultValue) {
	string v = Value(key, defaultValue ? "true" : "false");
	return v.compare("1") == 0 || v.compare("true") == 0 || v.compare("TRUE")  == 0|| v.compare("True") == 0;
}

void AbstractEntityReader::Index(const string &section) {
	string s = section;
	if(mSection.size() > 0)
		s = mSection + "/" + s;
	mIndexed[s] = 0;
}

void AbstractEntityReader::Key(const string &catalog, const string &id, bool flat) {
	mCatalog = catalog;
	mID = id;
	mFlat = flat;
}

void AbstractEntityReader::Section(const string &section) {
	if (mSection.size() == 0)
		mSection = section;
	else {
		STRINGLIST l;
		Util::Split(mSection, "/", l);
		l[l.size() - 1] = section;
		Util::Join(l, "/", mSection);
	}
}

void AbstractEntityReader::PushSection(const string &section) {
	if (mSection.size() > 0)
		mSection += "/";
	mSection += section;
}

string AbstractEntityReader::PopSection() {
	auto it = mSection.find_last_of("/");
	if (it == string::npos)
		mSection = "";
	else
		mSection = mSection.substr(0, it);
	return mSection;
}

//
//AbstractEntityWriter
//

AbstractEntityWriter::AbstractEntityWriter() {
	mSection = "";
	mID = "";
	mCatalog = "";
}

AbstractEntityWriter::~AbstractEntityWriter() {
}

void AbstractEntityWriter::Key(const string &catalog, const string &id) {
	mCatalog = catalog;
	mID = id;
}

void AbstractEntityWriter::PushSection(const string &section) {
	if (mSection.size() > 0)
		mSection += "/";
	mSection += section;
}

void AbstractEntityWriter::Section(const string &section) {
	if (mSection.size() == 0)
		mSection = section;
	else {
		STRINGLIST l;
		Util::Split(mSection, "/", l);
		l[l.size() - 1] = section;
		Util::Join(l, "/", mSection);
	}
}

bool AbstractEntityWriter::Value(const std::string &key, const unsigned long value) {
	return Value(key, StringUtil::Format("%lu", value));
}

bool AbstractEntityWriter::Value(const std::string &key, const unsigned int value) {
	return Value(key, StringUtil::Format("%u", value));
}

bool AbstractEntityWriter::Value(const std::string &key, const int value) {
	return Value(key, StringUtil::Format("%d", value));
}

bool AbstractEntityWriter::Value(const std::string &key, const bool value) {
	return Value(key, StringUtil::Format("%d", value ? 1 : 0));
}

bool AbstractEntityWriter::Value(const std::string &key, const float value) {
	return Value(key, StringUtil::Format("%f", value));
}

string AbstractEntityWriter::PopSection() {
	auto it = mSection.find_last_of("/");
	if (it == string::npos)
		mSection = "";
	else
		mSection = mSection.substr(0, it);
	return mSection;
}

//
// TextFileEntityReader
//

TextFileEntityReader::TextFileEntityReader(string filename, int caseConv, int commentStyle) {
	mFilename = filename;
	mLoaded = false;
	mCaseConv = caseConv;
	mCommentStyle = commentStyle;
	mError = false;
}

TextFileEntityReader::~TextFileEntityReader() {
}

bool TextFileEntityReader::Start() {
	return true;
}

bool TextFileEntityReader::Abort() {
	return true;
}

bool TextFileEntityReader::End() {
	return true;
}

std::string TextFileEntityReader::CheckKey(std::string key) {
	switch(mCaseConv) {
	case Case_Upper:
		return StringUtil::UpperCase(key);
	case Case_Lower:
		return StringUtil::LowerCase(key);
	default:
		return key;
	}
}

bool TextFileEntityReader::Exists() {
	return Platform::FileExists(mFilename);
}

vector<string> TextFileEntityReader::Keys() {
	CheckLoaded();
	STRINGLIST l;
	for(auto a = mValues[mSection].begin(); a != mValues[mSection].end(); ++a) {
		if(std::find(l.begin(), l.end(), a->first) == l.end()) {
			l.push_back(a->first);
		}
	}
	return l;
}

vector<string> TextFileEntityReader::Sections() {
	CheckLoaded();

	/* Now find those that are in the current section path (not recursively either) */
	STRINGLIST l;
	for(auto a = mSections.begin(); a != mSections.end(); ++a) {
		if(Util::HasBeginning(*a, mSection)) {
			string s = (*a).substr(mSection.length() == 0 || (*a).length() == mSection.length() ? 0 : mSection.length() + 1);
			if(s.find_first_of("/") == string::npos)
				l.push_back(*a);
		}
	}

	return l;
}

string TextFileEntityReader::Value(const string &key,
		string defaultValue) {
	if(!CheckLoaded())
		return defaultValue;
	string k = CheckKey(key);
	if(mValues.find(mSection) == mValues.end())
		return defaultValue;
	TEXT_FILE_SECTION_MAP m = mValues[mSection];
	if(m.find(k) == m.end())
		return defaultValue;
	STRINGLIST l = m[k];
	if(l.size() == 0)
		return defaultValue;
	return l[0];
}

vector<string> TextFileEntityReader::ListValue(const string &key, const std::string &separator) {
	if(!CheckLoaded())
		return {};

	if(separator.size() == 0)
		return mValues[mSection][CheckKey(key)];
	else {
		auto v = mValues[mSection][CheckKey(key)];
		STRINGLIST a;
		for(auto it = v.begin(); it != v.end(); ++it) {
			STRINGLIST l;
			Util::Split(*it, ",", l);
			for(auto it2 = l.begin(); it2 != l.end(); ++it2) {
				a.push_back(*it2);
			}
		}
		return a;
	}
}

bool TextFileEntityReader::CheckLoaded() {
	if (!Exists())
		mLoaded = false;

	if (!mLoaded) {
		FileReader lfr;
		if (lfr.OpenText(mFilename.c_str()) != Err_OK) {
			g_Logs.data->error("Failed to open entity file: %v", mFilename);
		} else {
			lfr.CommentStyle = mCommentStyle;
			int r;
			std::string sectionPath;

			while (lfr.FileOpen()) {
				r = lfr.ReadLine();
				if (r > 0) {
					lfr.MultiBreak("=");
					lfr.BlockToStringC(0, mCaseConv);
					if(Util::HasBeginning(lfr.SecBuffer, "[/")) {
						string secName = lfr.SecBuffer;
						secName = secName.substr(2, secName.size() - 3);

						/* Ignore the index for this test */
						std::string testPath = sectionPath;
						auto it = sectionPath.find_last_of("#");
						if(it != string::npos) {
							testPath = testPath.substr(0, it);
						}
						if(mIndexed.find(testPath) != mIndexed.end()) {
							sectionPath = testPath;
						}


						if(Util::HasEnding(sectionPath, secName)) {
							STRINGLIST l;
							Util::Split(sectionPath, "/", l);
							if(l.size() < 2)
								sectionPath = l[0];
							else {
								l.erase(l.end() - 1);
 								Util::Join(l, "/", sectionPath);
							}
						}
						else {
							mError = true;
							g_Logs.data->error("Unexpected section close of %v in %v", secName, mFilename);
							break;
						}
					} else if(Util::HasBeginning(lfr.SecBuffer, "[")) {
						string secName = lfr.SecBuffer;
						secName = secName.substr(1, secName.size() - 2);
						if(sectionPath.size() == 0 || mFlat)
							sectionPath = secName;
						else
							sectionPath = sectionPath + "/" + secName;

						if(mIndexed.find(sectionPath) != mIndexed.end()) {
							sectionPath += "#" + StringUtil::Format("%d", mIndexed[sectionPath]++);
						}

						mValues[sectionPath] = TEXT_FILE_SECTION_MAP();
						mSections.push_back(sectionPath);
					}
					else {
						string k = lfr.SecBuffer;
						lfr.BreakUntil("=", '=');
						string v = lfr.BlockToStringC(1, 0);
						if(mValues[sectionPath].find(k)== mValues[sectionPath].end())
							mValues[sectionPath][k] = {v};
						else
							mValues[sectionPath][k].push_back(v);
					}
				}
			}
			lfr.CloseCurrent();
			mLoaded = true;
		}
	}
	return mLoaded && !mError;
}
