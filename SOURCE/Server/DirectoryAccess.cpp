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

Platform_DirectoryReader :: Platform_DirectoryReader()
{
    excludeRel = true;
    cwd = ".";

#ifdef WINDOWS_PLATFORM
    // For Windows, these specific values are not used for object comparison.
    // All that matters is they're different.
    TYPE_DIR = 0;
    TYPE_FILE = 1;
#else
   // For Linux, the search function directly compares objects with these
    // predefined types.
    TYPE_DIR = DT_DIR;
    TYPE_FILE = DT_REG;
#endif
}

Platform_DirectoryReader :: ~Platform_DirectoryReader()
{
    Clear();
}

void Platform_DirectoryReader :: Clear(void)
{
	cwd = ".";
    fileList.clear();
}

void Platform_DirectoryReader :: ReadDirectories(void)
{
    RunScan(TYPE_DIR);
}

void Platform_DirectoryReader :: ReadFiles(void)
{
    RunScan(TYPE_FILE);
}

void Platform_DirectoryReader :: SetDirectory(std::string path)
{
	cwd = path;
}

std::string Platform_DirectoryReader :: GetDirectory()
{
	return cwd;
}

int Platform_DirectoryReader :: FileCount(void)
{
    return fileList.size();
}

bool Platform_DirectoryReader :: CheckInvalidDir(std::string entryName)
{
    if(excludeRel == true)
    {
        if(entryName.compare(".") == 0)
            return true;

        if(entryName.compare("..") == 0)
            return true;
    }
    return false;
}

#ifdef WINDOWS_PLATFORM
// Windows version
void Platform_DirectoryReader :: RunScan(int filter)
{
	_finddata_t fdata;
	intptr_t sHandle;
	char path[512];
	snprintf(path, sizeof(path), "%s\\*.*", cwd.c_str());
	sHandle = _findfirst("*.*", &fdata);
	if(sHandle == -1)
		return;

	int r = 0;
	while(r >= 0)
	{
        bool excluded = false;

        //Windows is different since objects may contain multiple flags.  Regular files
        //may vary (read only, hidden, etc) but folders always have an explicit bit.
        if(filter == TYPE_DIR)
        {
			if(!(fdata.attrib & _A_SUBDIR))
				excluded = true;

			if(excluded == false)
				excluded = CheckInvalidDir(fdata.name);
        }
        else
        {
        	if(fdata.attrib & _A_SUBDIR)
				excluded = true;
        }

        if(excluded == false)
            fileList.push_back(fdata.name);

		r = _findnext(sHandle, &fdata);
	}
	_findclose(sHandle);
}
#else //#ifdef WINDOWS_PLATFORM
//Linux version
void Platform_DirectoryReader :: RunScan(int filter)
{
    DIR *dir = opendir(cwd.c_str());
    if(dir == NULL)
        return;

    dirent *entry;
    while(true)
    {
        entry = readdir(dir);
        if(entry == NULL)
            break;

        bool excluded = false;
        if(entry->d_type != filter)
            excluded = true;
        if(excluded == false && filter == TYPE_DIR)
            excluded = CheckInvalidDir(entry->d_name);

        if(excluded == false)
            fileList.push_back(entry->d_name);
    }
    closedir(dir);
}

#endif //#ifdef WINDOWS_PLATFORM

// This function will fix the slashes in a file name to make it conform
// to a Windows "\" or Linux "/" folder access convention.
std::string Platform::FixPaths(std::string pathName)
{
	size_t len = pathName.length();
	for(size_t i = 0; i < len; i++)
	{
		if(pathName[i] == PLATFORM_FOLDERINVALID)
			pathName[i] = PLATFORM_FOLDERVALID;
	}
	return pathName;
}


void Platform::SetDirectory(std::string path)
{
    PLATFORM_CHDIR(path.c_str());
}

bool Platform::IsAbsolute(std::string str) {
#if defined(_WIN32) || defined(WIN32)
	if (str.size() > 2 && str.substr(1, 2).compare(":\\") == 0) {
		return true;
	}
	return str.find("\\") == 0;
#else
	return str.find("/") == 0;
#endif
}

std::string Platform :: GetDirectory()
{
	char cwd[512];
	PLATFORM_GETCWD(cwd,sizeof(cwd));
	return cwd;
	//return get_current_dir_name();
}

bool Platform::DirExists(std::string path)
{
#ifdef WINDOWS_PLATFORM
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
	    return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
	    return true;
	return false;
#else
	DIR *dir;
	if ((dir = opendir(path.c_str())) != NULL) {
		closedir(dir);
		return true;
	}
	else
		return false;
#endif
}

bool Platform::Delete(std::string path)
{
	return remove(path.c_str()) == 0;
}

bool Platform::FileExists(std::string path)
{
	FILE *input = fopen(path.c_str(), "rb");
	if(input == NULL)
		return false;
	fclose(input);
	return true;
}

unsigned long Platform::GetLastModified(std::string path) {
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

int Platform::SetLastModified(std::string path, unsigned long lastModifiedSec) {
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

void Platform::MakeDirectory(std::string path)
{
#ifdef WINDOWS_PLATFORM
    _mkdir(path.c_str());
#else
	mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

std::string Platform::JoinPath(std::string folder, std::string path)
{
	if(folder.length() == 0 || folder.compare(".") == 0)
		folder = g_WorkingDirectory;
	if(Util::HasBeginning(path, "/"))
		return path;
	else
		return folder + PLATFORM_FOLDERVALID + path;
}



std::string Platform::Filename(std::string path)
{
	STRINGLIST v;
	const std::string p = path;
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, d.c_str(), v);
	if(v.size() == 0)
		return "";
	else
		return v[v.size() - 1];
}

std::string Platform::Basename(std::string path)
{
	STRINGLIST v;
	const std::string p = Filename(path);
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, ".", v);
	if(v.size() == 0)
		return "";
	else {
		std::string t;
		v.erase(v.end() - 1);
		Util::Join(v, d.c_str(), t);
		return t;
	}
}

std::string Platform::Extension(std::string path)
{
	STRINGLIST v;
	const std::string p = Filename(path);
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, ".", v);
	if(v.size() == 0)
		return "";
	else if(v.size() == 1)
		return v[0];
	else
		return v[v.size() - 1];
}

std::string Platform::Dirname(std::string path)
{
	STRINGLIST v;
	const std::string p = path;
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, d.c_str(), v);
	if(v.size() == 0)
		return "";
	else if(v.size() == 1)
		return ".";
	else {
		std::string t;
		v.erase(v.end() - 1);
		Util::Join(v, d.c_str(), t);
		return t;
	}
}

int Platform :: FileCopy(std::string sourceFile, std::string destFile)
{
	FILE *input = fopen(sourceFile.c_str(), "rb");
	if(input == NULL)
		return -1;
	FILE *output = fopen(destFile.c_str(), "wb");
	if(output == NULL)
	{
		fclose(input);
		return -1;
	}
	fseek(input, 0, SEEK_END);
	long filesize = ftell(input);
	fseek(input, 0, SEEK_SET);

	long remain = filesize;
	char buffer[4096];
	while(remain > 0)
	{
		int toRead = sizeof(buffer);
		if(toRead > remain)
			toRead = remain;
		fread(buffer, toRead, 1, input);
		fwrite(buffer, toRead, 1, output);
		remain -= toRead;
	}
	fclose(input);
	fclose(output);
	return 0;
}
