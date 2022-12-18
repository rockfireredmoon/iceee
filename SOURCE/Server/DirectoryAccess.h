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
    void SetDirectory(const std::string &path);
    int FileCount(void);
    bool CheckInvalidDir(const std::string &filename);

private:
    int TYPE_DIR;
    int TYPE_FILE;
    std::string cwd;
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
	bool IsAbsolute(const std::string &str);
	std::string GetDirectory();
	void SetDirectory(const std::string &path);
	std::string FixPaths(const std::string &pathName);
	void MakeDirectory(const std::string & path);
	void MakeDirectories(const std::string &path);
	int FileCopy(const std::string &sourceFile, const std::string &destFile);
	bool Delete(const std::string &path);
    bool FileExists(const std::string &sourceFile);
    bool DirExists(const std::string &path);
    std::string JoinPath(const std::string &folder, const std::string &path);
    std::string Dirname(const std::string &path);
    std::string Filename(const std::string &path);
    std::string Extension(const std::string &path);
    std::string Basename(const std::string &path);
    unsigned long GetLastModified(const std::string &path);
    int SetLastModified(const std::string &path, unsigned long lastModified);
}

#endif //DIRECTORYACCESS_H
