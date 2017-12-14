#include "Fun.h"
#include "FileReader.h"

namespace Fun {

FunReplace oFunReplace;

FunReplace::FunReplace() {
	mLoaded = false;
}

FunReplace::~FunReplace() {
}

void FunReplace::LoadFile(void) {
	phrase.clear();
	replace.clear();
	mLoaded = true;
	FileReader lfr;
	if (lfr.OpenText("FunChatReplace.txt") != Err_OK)
		return;
	while (lfr.FileOpen() == true) {
		lfr.ReadLine();
		int r = lfr.SingleBreak(";");
		if (r >= 2) {
			phrase.push_back(lfr.BlockToString(0));
			replace.push_back(lfr.BlockToString(1));
		}
	}
	lfr.CloseCurrent();
}

bool FunReplace::Replace(const char *input, std::string &output) {
	if (mLoaded == false)
		LoadFile();

	std::string searchStr = input;
	output = input;

	for (size_t i = 0; i < searchStr.length(); i++)
		if (searchStr[i] >= 'A' && searchStr[i] <= 'Z')
			searchStr[i] += 32;

	int changes = 0;
	bool search = false;
	size_t pos = 0;
	do {
		search = false;
		for (size_t i = 0; i < phrase.size(); i++) {
			pos = searchStr.find(phrase[i]);
			if (pos != std::string::npos) {
				search = true;
				int len = phrase[i].length();
				searchStr.replace(pos, len, replace[i]);
				output.replace(pos, len, replace[i]);
				changes++;
			}
		}
	} while (search == true);

	return (changes > 0);
}

void FunReplace::Reset(void) {
	mLoaded = false;
}

} //namespace Fun

