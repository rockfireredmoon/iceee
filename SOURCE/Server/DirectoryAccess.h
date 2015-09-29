// A platform-independent way to read the contents of folders.

#ifndef DIRECTORYACCESS_H
#define DIRECTORYACCESS_H

#include "Components.h"

#include <vector>
#include <string>

#ifdef WINDOWS_PLATFORM
#include <direct.h>
#else
#include <unistd.h>
#endif

class Platform_DirectoryReader
{
public:
    Platform_DirectoryReader();
    ~Platform_DirectoryReader();

    bool excludeRel;
    std::vector<std::string> fileList;
    void Clear();
    void ReadDirectories();
    void ReadFiles(void);
    std::string GetDirectory();
    void SetDirectory(std::string path);
    int FileCount(void);
    bool CheckInvalidDir(std::string filename);

private:
    int TYPE_DIR;
    int TYPE_FILE;
    void RunScan(int filter);
};



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
	char * FixPaths(char* pathName);
	const char* FixPaths(std::string &pathName);
	void MakeDirectory(const char *path);
    char * GenerateFilePath(char *resultBuffer, const char *folderName, const char *fileName);
	void GenerateFilePath(std::string& resultStr, const char *folderName, const char *fileName);
	int FileCopy(const char *sourceFile, const char *destFile);
	bool Delete(const char *path);
    bool FileExists(const char *sourceFile);
    bool DirExists(const char *path);
    const char * Dirname(const char *path);
    const char * Filename(const char *path);
    const char * Extension(const char *path);
    const char * Basename(const char *path);
    unsigned long GetLastModified(const char *path);
    int SetLastModified(const char *path, unsigned long lastModified);
}

#endif //DIRECTORYACCESS_H
