//Random junk in the server that doesn't need to exist but is fun either way.
//Currently this has stuff to troll people.
#ifndef FUN_H
#define FUN_H

#include <vector>
#include <string>

namespace Fun {

class FunReplace {
public:
	FunReplace();
	~FunReplace();
	bool Replace(const char *input, std::string &output);
	void Reset(void);
private:
	void LoadFile(void);
	bool mLoaded;
	std::vector<std::string> phrase;
	std::vector<std::string> replace;
};

extern FunReplace oFunReplace;

} //namespace Fun

#endif
