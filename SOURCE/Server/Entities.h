#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <stack>

class AbstractEntityReader {

public:
	AbstractEntityReader();

	virtual ~AbstractEntityReader();
	virtual bool Start() = 0;
	virtual bool End() = 0;
	virtual bool Abort() = 0;
	virtual std::string Value(const std::string &key, std::string defaultValue = "") =0;
	virtual std::vector<std::string> ListValue(const std::string &key, const std::string &separator = "") =0;
	virtual std::vector<std::string> Sections() =0;
	void PushSection(const std::string &section);
	void Section(const std::string &section);
	void Index(const std::string &section);
	void Key(const std::string &catalog, const std::string &id, bool flat = false);
	int ValueInt(const std::string &key, const int defaultValue = 0);
	float ValueFloat(const std::string &key, const float defaultValue = 0);
	bool ValueBool(const std::string &key, const bool defaultValue = false);
	unsigned long ValueULong(const std::string &key, const unsigned long defaultValue = 0);
	virtual bool Exists() = 0;
	virtual std::vector<std::string> Keys() = 0;
	std::string PopSection();

	int mMaxDepth;
	bool mFlat;
	std::map<std::string, int> mIndexed;
	std::string mSection;
	std::string mCatalog;
	std::string mID;
};

class AbstractEntityWriter {

public:
	AbstractEntityWriter();

	virtual ~AbstractEntityWriter();
	virtual bool Start() = 0;
	virtual bool End() = 0;
	virtual bool Abort() = 0;
	virtual bool Value(const std::string &key, const std::string &value) = 0;
	virtual bool ListValue(const std::string &key, std::vector<std::string> &value) = 0;

	void Key(const std::string & catalog, const std::string &id);
	bool Value(const std::string &key, const unsigned long value);
	bool Value(const std::string &key, const int value);
	bool Value(const std::string &key, const bool value);
	bool Value(const std::string &key, const float value);
	void PushSection(const std::string &section);
	void Section(const std::string &section);
	std::string PopSection();
	std::string mCatalog;
	std::string mID;
	std::string mSection;
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
struct ciLessLibC : public std::binary_function<std::string, std::string, bool> {
    bool operator()(const std::string &lhs, const std::string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
    }
};

typedef std::map<std::string, std::vector<std::string>, ciLessLibC> TEXT_FILE_SECTION_MAP;

class TextFileEntityReader : public AbstractEntityReader {
public:
	TextFileEntityReader(std::string filename, int caseConv, int commentStyle);
	virtual ~TextFileEntityReader();

	virtual std::string Value(const std::string &key, std::string defaultValue = "");
	virtual std::vector<std::string> ListValue(const std::string & key, const std::string &separator = "");
	virtual std::vector<std::string> Sections();
	virtual std::vector<std::string> Keys();
	virtual bool Start();
	virtual bool End();
	virtual bool Abort();
	virtual bool Exists();

private:
	bool CheckLoaded();
	std::string CheckKey(std::string key);
	bool mLoaded;
	bool mError;
	int mCaseConv;
	int mCommentStyle;
	std::string mFilename;
	std::vector<std::string> mSections;
	std::map<std::string, TEXT_FILE_SECTION_MAP> mValues;
};

#endif //ENTITIES_H
