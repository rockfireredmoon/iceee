// A platform-independent way to read the contents of folders.
#include "DirectoryAccess.h"

#include <string.h>  //for strcmp()

#ifdef WINDOWS_PLATFORM
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#endif

#include <stdio.h>
#include "Util.h"

Platform_DirectoryReader :: Platform_DirectoryReader()
{
    excludeRel = true;

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

void Platform_DirectoryReader :: SetDirectory(const char *path)
{
    PLATFORM_CHDIR(path);
}

const char * Platform_DirectoryReader :: GetDirectory()
{
//	string cwd;
//	PLATFORM_GETCWD(cwd.c_str());
	return get_current_dir_name();
}

int Platform_DirectoryReader :: FileCount(void)
{
    return fileList.size();
}

bool Platform_DirectoryReader :: CheckInvalidDir(const char *entryName)
{
    if(excludeRel == true)
    {
        if(strcmp(entryName, ".") == 0)
            return true;

        if(strcmp(entryName, "..") == 0)
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
    DIR *dir = opendir(".");
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
char * Platform::FixPaths(char* pathName)
{
	size_t len = strlen(pathName);
	for(size_t i = 0; i < len; i++)
	{
		if(pathName[i] == PLATFORM_FOLDERINVALID)
			pathName[i] = PLATFORM_FOLDERVALID;
	}
	return pathName;
}

const char* Platform::FixPaths(std::string &pathName)
{
	size_t len = pathName.size();
	for(size_t i = 0; i < len; i++)
	{
		if(pathName[i] == PLATFORM_FOLDERINVALID)
			pathName[i] = PLATFORM_FOLDERVALID;
	}
	return pathName.c_str();
}

bool Platform::DirExists(const char *path)
{
	DIR *dir;
	if ((dir = opendir(path)) != NULL) {
		closedir(dir);
		return true;
	}
	else
		return false;
}

bool Platform::Delete(const char *path)
{
	return remove(path) == 0;
}

bool Platform::FileExists(const char *path)
{
	FILE *input = fopen(path, "rb");
	if(input == NULL)
		return false;
	fclose(input);
	return true;
}

unsigned long Platform::GetLastModified(const char *path) {
	struct stat attrib;
	if(stat(path, &attrib) < 0) {
		return 0;
	}
	return attrib.st_mtim.tv_sec;
}

int Platform::SetLastModified(const char *path, unsigned long lastModifiedSec) {
	struct utimbuf new_times;
	struct stat attrib;
	if(stat(path, &attrib) < 0) {
		return 0;
	}
	new_times.actime = attrib.st_atime;
	new_times.modtime = lastModifiedSec;
	if (utime(path, &new_times) < 0) {
	    return 1;
	}
	return 1;
}

void Platform::MakeDirectory(const char *path)
{
#ifdef WINDOWS_PLATFORM
    _mkdir(path);
#else
	mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

char * Platform::GenerateFilePath(char *resultBuffer, const char *folderName, const char *fileName)
{
	sprintf(resultBuffer, "%s%c%s", folderName, PLATFORM_FOLDERVALID, fileName);
	return resultBuffer;
}

const char * Platform::Filename(const char *path)
{
	STRINGLIST v;
	const std::string p = path;
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, d.c_str(), v);
	if(v.size() == 0)
		return "";
	else
		return v[v.size() - 1].c_str();
}

const char * Platform::Basename(const char *path)
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
		return t.c_str();
	}
}

const char * Platform::Extension(const char *path)
{
	STRINGLIST v;
	const std::string p = Filename(path);
	const std::string d(1, PLATFORM_FOLDERVALID);
	Util::Split(p, ".", v);
	if(v.size() == 0)
		return "";
	else if(v.size() == 1)
		return v[0].c_str();
	else
		return v[v.size() - 1].c_str();
}

const char * Platform::Dirname(const char *path)
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
		return t.c_str();
	}
}

void Platform :: GenerateFilePath(std::string& resultStr, const char *folderName, const char *fileName)
{
	resultStr.clear();
	resultStr.append(folderName);
	resultStr.push_back(PLATFORM_FOLDERVALID);
	resultStr.append(fileName);
}

int Platform :: FileCopy(const char *sourceFile, const char *destFile)
{
	FILE *input = fopen(sourceFile, "rb");
	if(input == NULL)
		return -1;
	FILE *output = fopen(destFile, "wb");
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
