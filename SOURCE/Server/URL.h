/*
This class is responsible for loading a list of URLs in key=value format from a file.
A modded client may ask for a list of URLs, which it may need to launch when the user
presses certain buttons, such as to open the webpage.  This system allows the URLs to
be assigned by the server administrator without depending on hardcoded URLs in the
client which may change if hosted on a new domain or file structure.
*/

#ifndef URL_H
#define URL_H

#include <string>
#include <vector>

typedef std::vector<std::string> STRINGLIST;
typedef std::vector<STRINGLIST> MULTISTRING;

class URLManager
{
public:
	std::string GetURL(std::string name);
	const MULTISTRING& GetURLs(void);
	URLManager();

private:
	MULTISTRING mLoadedURLs;
	bool mLoaded;
	
	void LoadFile(void);
};

extern URLManager g_URLManager;

#endif //#ifndef URL_H
