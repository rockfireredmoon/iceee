// A platform-independent way to read the contents of folders.

#ifndef DIRECTORYACCESS_H
#define DIRECTORYACCESS_H

#include "Components.h"

#include <vector>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;


// Needed for the proper forward/backward slash convention depending on
// the operating system.
#ifdef WINDOWS_PLATFORM
	#define PLATFORM_FOLDERINVALID   '/'
	#define PLATFORM_FOLDERVALID     '\\'
#else
	#define PLATFORM_FOLDERINVALID   '\\'
	#define PLATFORM_FOLDERVALID     '/'
#endif


// Needed for proper naming conventions to prevent warnings or errors.
#ifdef WINDOWS_PLATFORM
	#define PLATFORM_GETCWD   _getcwd
	#define PLATFORM_CHDIR    _chdir
#else
	#define PLATFORM_GETCWD   getcwd
	#define PLATFORM_CHDIR    chdir
#endif


namespace Platform
{
	string FixPaths(const string &pathName);
    unsigned long GetLastModified(const fs::path &path);
    int SetLastModified(const fs::path &path, unsigned long lastModified);
}

#endif //DIRECTORYACCESS_H
