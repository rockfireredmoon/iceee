// A platform-independent way to read the contents of folders.
#include "DirectoryAccess.h"

#include <string.h>  //for strcmp()

#ifdef WINDOWS_PLATFORM
#include <direct.h>
#include <io.h>
#include <windows.h>
#define EPOCH_DIFF 11644473600LL
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#endif

#include <stdio.h>
#include "Util.h"
#include "Config.h"


std::string Platform::FixPaths(const std::string &pathName)
{
	size_t len = pathName.length();
	std::string s;
	for(size_t i = 0; i < len; i++)
	{
		if(pathName[i] == PLATFORM_FOLDERINVALID)
			s += PLATFORM_FOLDERVALID;
		else
			s += pathName[i];
	}
	return s;
}

unsigned long Platform::GetLastModified(const std::string &path) {
#ifdef WINDOWS_PLATFORM

	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet;

	HANDLE hFile;

	hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return 0;

	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return 0;

	// http://stackoverflow.com/questions/19709580/c-convert-filetime-to-seconds
	__int64* val = (__int64*) &ftWrite;
	return static_cast<unsigned long>(*val) / 10000000 - EPOCH_DIFF;   // epoch is Jan. 1, 1601: 134774 days to Jan. 1, 1970
#else
	struct stat attrib;
	if(stat(path.c_str(), &attrib) < 0) {
		return 0;
	}
	return attrib.st_mtime;
#endif
}

int Platform::SetLastModified(const std::string &path, unsigned long lastModifiedSec) {
#ifdef WINDOWS_PLATFORM

	HANDLE hFile;

	hFile = CreateFile(path.c_str(), GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return 0;

	// http://stackoverflow.com/questions/3585583/convert-unix-linux-time-to-windows-filetime
	unsigned long long qwResult = EPOCH_DIFF;
	qwResult += lastModifiedSec;
	qwResult *= 10000000LL;

	FILETIME ft;
	ft.dwLowDateTime  = (DWORD) (qwResult & 0xFFFFFFFF );
	ft.dwHighDateTime = (DWORD) (qwResult >> 32 );

	if(!SetFileTime(hFile,
		        (LPFILETIME) NULL,
		        (LPFILETIME) NULL,
		        &ft)) {
		return 0;
	}

	return 1;
#else
	struct utimbuf new_times;
	struct stat attrib;
	if(stat(path.c_str(), &attrib) < 0) {
		return 0;
	}
	new_times.actime = attrib.st_atime;
	new_times.modtime = lastModifiedSec;
	if (utime(path.c_str(), &new_times) < 0) {
	    return 1;
	}
	return 1;
#endif
}



