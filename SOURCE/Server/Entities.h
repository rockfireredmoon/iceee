#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <stack>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class AbstractEntityReader {

public:
	AbstractEntityReader();

	virtual ~AbstractEntityReader();
	virtual bool Start() = 0;
	virtual bool End() = 0;
	virtual bool Abort() = 0;
	virtual string Value(const string &key, string defaultValue = "") =0;
	virtual vector<string> ListValue(const string &key, const string &separator = "") =0;
	virtual vector<string> Sections() =0;
	void PushSection(const string &section);
	void Section(const string &section);
	void Index(const string &section);
	void Key(const string &catalog, const string &id, bool flat = false);
	int ValueInt(const string &key, const int defaultValue = 0);
	float ValueFloat(const string &key, const float defaultValue = 0);
	bool ValueBool(const string &key, const bool defaultValue = false);
	unsigned long ValueULong(const string &key, const unsigned long defaultValue = 0);
	virtual bool Exists() = 0;
	virtual vector<string> Keys() = 0;
	string PopSection();

	int mMaxDepth;
	bool mFlat;
	map<string, int> mIndexed;
	string mSection;
	string mCatalog;
	string mID;
};

class AbstractEntityWriter {

public:
	AbstractEntityWriter();

	virtual ~AbstractEntityWriter();
	virtual bool Start() = 0;
	virtual bool End() = 0;
	virtual bool Abort() = 0;
	virtual bool Value(const string &key, const string &value) = 0;
	virtual bool ListValue(const string &key, vector<string> &value) = 0;

	void Key(const string & catalog, const string &id);
	bool Value(const string &key, const unsigned long value);
	bool Value(const string &key, const int value);
	bool Value(const string &key, const unsigned int value);
	bool Value(const string &key, const bool value);
	bool Value(const string &key, const float value);
	void PushSection(const string &section);
	void Section(const string &section);
	string PopSection();
	string mCatalog;
	string mID;
	string mSection;
};


class AbstractEntity {

public:
	AbstractEntity();

	virtual ~AbstractEntity();

	virtual bool WriteEntity(AbstractEntityWriter *writer) = 0;
	virtual bool ReadEntity(AbstractEntityReader *reader) = 0;
	virtual bool EntityKeys(AbstractEntityReader *reader) = 0;
};
// NULLs aren't an issue.  Much faster than the STL or Boost lex versions.
//struct ciLessLibC : public binary_function<string, string, bool> {
//    bool operator()(const string &lhs, const string &rhs) const {
//        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
//    }
//};
struct ciLessLibC {
    bool operator()(const string &lhs, const string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
    }
};


typedef map<string, vector<string>, ciLessLibC> TEXT_FILE_SECTION_MAP;


class TextFileEntityWriter: public AbstractEntityWriter {
public:
	TextFileEntityWriter(const fs::path &path);
	virtual ~TextFileEntityWriter();

	virtual bool Value(const string &key, const string &value);
	virtual bool ListValue(const string &key, vector<string> &value);

	virtual void PushSection(const string &section);
	virtual void Section(const string &section);

	virtual bool Start();
	virtual bool End();
	virtual bool Abort();

private:
	fs::path mPath;
	FILE *mOutput;
};

class TextFileEntityReader : public AbstractEntityReader {
public:
	TextFileEntityReader(const fs::path &filename, int caseConv, int commentStyle);
	virtual ~TextFileEntityReader();

	virtual string Value(const string &key, string defaultValue = "");
	virtual vector<string> ListValue(const string & key, const string &separator = "");
	virtual vector<string> Sections();
	virtual vector<string> Keys();
	virtual bool Start();
	virtual bool End();
	virtual bool Abort();
	virtual bool Exists();

private:
	bool CheckLoaded();
	string CheckKey(string key);
	bool mLoaded;
	bool mError;
	int mCaseConv;
	int mCommentStyle;
	fs::path mFilename;
	vector<string> mSections;
	map<string, TEXT_FILE_SECTION_MAP> mValues;
};

#endif //ENTITIES_H
