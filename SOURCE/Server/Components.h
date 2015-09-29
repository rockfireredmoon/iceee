// Keeps track of server components, platform-specific globals, defines,
// debugging, and other stuff.

#pragma once
#ifndef COMPONENTS_H
#define COMPONENTS_H

// Define these depending on the platform and compiler settings below.
// What they are defined as does not matter, just that they're defined.
// Assigning a dummy value makes intellisense show a value to help see
// at a glance if they're defined.
//#define USE_WINDOWS_GUI    1      //These three are all Windows only.
//#define WINDOWS_PLATFORM   1      //Comment out for Linux builds.
//#define USE_SEH_EXCEPTIONS 1      //MSVC specific

#define DEBUG_TIME  1
#define DEBUG_PROFILER 1

//#define PRIVATE_USE_BUILD  1    //Specific compilation options intended for a limited private distribution

#ifdef PRIVATE_USE_BUILD
 #ifndef WINDOWS_PLATFORM
  #error Private build is not for the Linux system.
 #endif
#endif

/*
	USE_WINDOWS_GUI
	If this is defined, the Windows-only GUI will be functional.  If not
	defined, it will run in a console.

	WINDOWS_PLATFORM
	If this is defined, mutexes, server timing, popup messages, and
	multithreading will be Windows-only.  Otherwise uses Linux equivalents.

	USE_SEH_EXCEPTIONS
	Uses Microsoft-specific Structured Exception Handling (__try, __except)
	instead of (try, catch).
*/

/*	COMPILATION TIPS

FOR WINDOWS
	If using Microsoft Visual C++ 2008 Express
		USE_WINDOWS_GUI      OPTIONAL
		WINDOWS_PLATFORM     REQUIRED
		USE_SEH_EXCEPTIONS   PREFERRED
	If using Code::Blocks with the GNU GCC Compiler (and perhaps others)
		USE_WINDOWS_GUI      OPTIONAL
		WINDOWS_PLATFORM     REQUIRED
		USE_SEH_EXCEPTIONS   NOT SUPPORTED

FOR LINUX
	If using Code::Blocks with the GNU GCC Compiler (and perhaps others)
		USE_WINDOWS_GUI      NOT SUPPORTED
		WINDOWS_PLATFORM     NOT SUPPORTED
		USE_SEH_EXCEPTIONS   NOT SUPPORTED
*/

extern unsigned long g_ServerTime;
extern unsigned long g_ServerLaunchTime;

enum ServerStatus
{
	SERVER_STATUS_STOPPED   = 0,
	SERVER_STATUS_RUNNING   = 1,
	SERVER_STATUS_EXCEPTION = 2
};

extern int g_ServerStatus;

extern int ActiveComponents;
void AdjustComponentCount(int mod);

extern char VersionString[];


/////////////////////////////////////////////////////
// Further compiler-specific macros and other junk.

#ifdef WINDOWS_PLATFORM
	#define POPUP_MESSAGE(msg, title)   MessageBox(NULL, msg, title, 0)
#else
	#define POPUP_MESSAGE(msg, title)   printf("%s | %s", title, msg)
#endif


// Microsoft-specific extentions, or features specific to the Microsoft SDK
// and Microsoft Visual C++ 2008.
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
	#define EMPTY_OPERATION __noop
#else
	#define EMPTY_OPERATION ((void)0)
#endif
// End Microsoft-specific extentions.



#ifdef USE_SEH_EXCEPTIONS
	#include <excpt.h>
	int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
	#define BEGINTRY    __try
	#define BEGINCATCH  __except((filter(GetExceptionCode(), GetExceptionInformation())))
#else
	#define BEGINTRY    try
	#define BEGINCATCH  catch(...)
#endif

//
//	Critical sections (mutexes)
//

#ifdef HAS_PTHREAD
	#include <pthread.h>
#else
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif


class Platform_CriticalSection
{
public:
	Platform_CriticalSection();
	Platform_CriticalSection(const char *name);
	~Platform_CriticalSection();

	void Enter(const char *requestDesc);
	void Leave(void);
	int GetLockCount(void);
	void Init(void);
	void Delete(void);
	void Reset(void);
	void SetDebugName(const char *name);

	bool initialized;
	bool notifyWait; //Log a debug message if Enter() is called while lockCount is nonzero.
	bool disabled;   //If true, locking and unlocking will be ignored.

private:
	const char *lastLock;
	volatile int lockCount;  //Set to 1 when Enter() is successful, set to 0 when Leave() is successful. 
	char debugName[32];      //Holds the name of this critical section, used when reporting debug messages
	bool useDebugMessages;   //If set to true, debug messages will be logged.
#ifdef DEBUG_TIME
	unsigned long acquireTime;
#endif

#ifdef HAS_PTHREAD
	pthread_mutex_t mutex;
#else
#ifdef WINDOWS_PLATFORM
	CRITICAL_SECTION cs;
#endif
#endif
	void Clear(void);        //Zero all values, to be called by the constructor.
};



//
//	Creating Threads
//

/* Notes on the #defines in these sections:

	Platform_CreateThread()
	A normal function that initializes a thread for the defined platform.

	PLATFORM_THREADRETURN
	Substitutes the return variable type of a thread's main function prototype.

	PLATFORM_THREADARGS
	Substitutes the argument variable type of a thread's main function.

	PLATFORM_CLOSETHREAD
	Substitutes a thread exit function using a default (NULL) return value.

	These can then be combined and used, as the following example demonstrates:

		PLATFORM_THREADRETURN FunctionName(PLATFORM_THREADARGS argName)
		{
			//This is an example of casting the the (void*) argument to
			//a usable data type, class, struct, or whatever.
			ArbitraryDataType *data = (ArbitraryDataType*) argName;

			...

			PLATFORM_CLOSETHREAD();
		}
*/


#ifdef HAS_PTHREAD
	typedef void*(*PLATFORM_FUNCTIONPTR)(void*);
	#define PLATFORM_THREADRETURN    static void*
	#define PLATFORM_THREADARGS      void*
	#define PLATFORM_CLOSETHREAD(x)  pthread_exit(NULL);
	int Platform_CreateThread(size_t stackSize, void* ptrRoutine, void* ptrArgs, unsigned long *threadID);
#else
	#define PLATFORM_THREADRETURN    DWORD WINAPI
	#define PLATFORM_THREADARGS      LPVOID
	//#define PLATFORM_CLOSETHREAD(x)  __noop;    //Formerly  ExitThread(0)  Turns out this function is intended for C and not C++.  Testing revealed that it could cause memory errors and crashes.
	#define PLATFORM_CLOSETHREAD(x);
	int Platform_CreateThread(size_t stackSize, void* ptrRoutine, void* ptrArgs, DWORD* threadID);
#endif



//
//	Thread Sleeping
//

#ifdef WINDOWS_PLATFORM
	#define PLATFORM_SLEEP(milliseconds)   Sleep(milliseconds)
#else
	#define PLATFORM_SLEEP(milliseconds)   Sleep_Nanosleep(milliseconds);
	void Sleep_Nanosleep(int milliseconds);
#endif


//
//  Time
//
#ifndef WINDOWS_PLATFORM
	//#include <sys/time.h>
#endif

#include <time.h>

class PlatformTime
{
public:
	typedef time_t TIME_VALUE;
	static const unsigned long MAX_TIME = (unsigned long)(~0);
    unsigned long getMilliseconds(void);
	unsigned long getAbsoluteSeconds(void);
	unsigned long getAbsoluteMinutes(void);
	unsigned long getElapsedMilliseconds(void);

private:
#ifndef WINDOWS_PLATFORM
	//timeval timeData;
	timespec timeSpec;
#endif
	time_t timeSecData;
};

namespace Platform
{
	//Need these since maximums vary on 32 or 64 bit systems.
	static const unsigned char MAX_UCHAR = (unsigned char)(~0);
	static const char MAX_CHAR = MAX_UCHAR >> 1;

	static const unsigned short MAX_USHORT = (unsigned short)(~0);
	static const short MAX_SHORT = MAX_USHORT >> 1;

	static const unsigned int MAX_UINT = (unsigned int)(~0);
	static const int MAX_INT = MAX_UINT >> 1;

	static const unsigned long MAX_ULONG = (unsigned long)(~0);
	static const long MAX_LONG = MAX_ULONG >> 1;
}

extern PlatformTime g_PlatformTime;

extern Platform_CriticalSection component_cs;

#endif //COMPONENTS_H
