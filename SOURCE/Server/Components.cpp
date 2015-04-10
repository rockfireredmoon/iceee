#include <ctime>
#include "Components.h"
#include "StringList.h"
#include <string.h>

char VersionString[] = {"Version 36b : " __DATE__ }; 

unsigned long g_ServerTime = 0;        //Current server time (milliseconds)
unsigned long g_ServerLaunchTime = 0;  //Server launch time (milliseconds)

int g_ServerStatus = 0;            //The current running status of the server.  See the ServerStatus enum.  

int ActiveComponents = 0;          //A mutex counter, often incremented by child threads.  If zero, all threads are considered closed 
Platform_CriticalSection component_cs("CS_COMPONENT");
PlatformTime g_PlatformTime;

void AdjustComponentCount(int mod)
{
	component_cs.Enter("AdjustComponentCount");
	ActiveComponents += mod;
	component_cs.Leave();
}

#ifdef USE_SEH_EXCEPTIONS
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif


//
//	Critical sections
//

Platform_CriticalSection :: Platform_CriticalSection()
{
	Clear();
}

Platform_CriticalSection :: Platform_CriticalSection(const char *name)
{
	Clear();
	SetDebugName(name);
	Init();
}

void Platform_CriticalSection :: Clear(void)
{
	initialized = false;
	notifyWait = false;
	disabled = false;
	lastLock = NULL;
	lockCount = 0;
	memset(debugName, 0, sizeof(debugName));
	useDebugMessages = false;
	acquireTime = 0;
}

Platform_CriticalSection :: ~Platform_CriticalSection()
{
	Delete();
	/*
	if(initialized == true)
	{
	#ifdef WINDOWS_PLATFORM
		DeleteCriticalSection(&cs);
	#else
		pthread_mutex_destroy(&mutex);
	#endif
		initialized = false;
	}*/
}

void Platform_CriticalSection :: Init(void)
{
	if(initialized == false)
	{
#ifdef WINDOWS_PLATFORM
	InitializeCriticalSection(&cs);
#else
	pthread_mutexattr_t mutexattr;

	//This seems to fix a crash, although it wasn't in the web example I took this
	//code from.
	pthread_mutexattr_init(&mutexattr);

	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&mutex, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
#endif

	initialized = true;
	}
}

void Platform_CriticalSection :: SetDebugName(const char *name)
{
	strncpy(debugName, name, sizeof(debugName) - 1);
	useDebugMessages = true;
	notifyWait = true;
}

void Platform_CriticalSection :: Delete(void)
{
	if(initialized == true)
	{
	#ifdef WINDOWS_PLATFORM
		DeleteCriticalSection(&cs);
	#else
		pthread_mutex_destroy(&mutex);
	#endif

		initialized = false;
	}
}

void Platform_CriticalSection :: Reset(void)
{
	lockCount = 0;
	Delete();
#ifdef DEBUG_TIME
	acquireTime = 0;
#endif
	Init();
}

void Platform_CriticalSection :: Enter(const char *requestDesc)
{
	if(disabled == true)
		return;

#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
	if(lockCount != 0)
	{
		if(notifyWait == true)
			if(useDebugMessages == true)
				g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] CriticalSection::Enter[%s] waiting, %d count, request:%s, last:%s", debugName, lockCount, requestDesc, lastLock);
	}
	if(initialized == false)
	{
		if(useDebugMessages == true)
			g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Platform_CriticalSection::Enter[%s] on uninitialized section.", debugName);
		Init();
	}
#endif
		
#ifdef WINDOWS_PLATFORM
	EnterCriticalSection(&cs);
#else
	pthread_mutex_lock(&mutex);
#endif
	lockCount = 1;
	lastLock = requestDesc;

#ifdef DEBUG_TIME
	if(useDebugMessages == true)
	{
		long passTime = (long)g_PlatformTime.getMilliseconds() - (long)startTime;
		if(passTime > 100)
			g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] TIME PASSED CriticalSection::Enter[%s] %ld ms (%s).", debugName, passTime, lastLock);
		//g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] ENTER CriticalSection[%s]", debugName);
	}
	acquireTime = startTime;
#endif
}

void Platform_CriticalSection :: Leave(void)
{
	if(disabled == true)
		return;

	lockCount = 0;
#ifdef WINDOWS_PLATFORM
	LeaveCriticalSection(&cs);
#else
	pthread_mutex_unlock(&mutex);
#endif

#ifdef DEBUG_TIME
	if(useDebugMessages == true)
	{
		long passTime = (long)g_PlatformTime.getMilliseconds() - (long)acquireTime;
		if(passTime > 100)
			g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] TIME PASSED CriticalSection::Leave[%s] %ld ms (%s).", debugName, passTime, lastLock);
		//g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] LEAVE CriticalSection[%s]", debugName);
	}
#endif
	lastLock = NULL;
}

int Platform_CriticalSection :: GetLockCount(void)
{
	return lockCount;
}


//
//	Creating Threads
//
//  Note: for thread creation, windows returns a handle (nonzero) and Linux
//  returns zero on success.  These functions are modified to always return nonzero
//  on success.

#ifdef WINDOWS_PLATFORM

int Platform_CreateThread(size_t stackSize, void* ptrRoutine, void* ptrArgs, DWORD* threadID)
{
	HANDLE res = CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)ptrRoutine, ptrArgs, 0, threadID);
	return (res != NULL);
}

#else //#ifdef WINDOWS_PLATFORM

int Platform_CreateThread(size_t stackSize, void* ptrRoutine, void* ptrArgs, unsigned long *threadID)
{
	pthread_t thread;
	pthread_attr_t threadAttr;
	struct sched_param param;
	pthread_attr_init(&threadAttr);
	pthread_attr_setstacksize(&threadAttr, stackSize);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
	int res = pthread_create(&thread, &threadAttr, (PLATFORM_FUNCTIONPTR)ptrRoutine, ptrArgs);
	if(res != 0)
		return 0;
	return 1;
}

#endif //#ifdef WINDOWS_PLATFORM





//
//	Thread Sleeping
//

// Only for Linux.
#ifndef WINDOWS_PLATFORM

void Sleep_Nanosleep(int milliseconds)
{
    timespec required;
	required.tv_sec = milliseconds / 1000; //milliseconds / 1000;
	required.tv_nsec = (milliseconds % 1000) * (1000 * 1000);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &required, &required);
}

#endif //#ifndef WINDOWS_PLATFORM



unsigned long PlatformTime :: getMilliseconds(void)
{
#ifdef WINDOWS_PLATFORM
	return GetTickCount();
#else
	clock_gettime(CLOCK_MONOTONIC, &timeSpec);
	return (timeSpec.tv_sec * 1000) + (timeSpec.tv_nsec / 1000000);
	// OBSOLETE, CAUSED PROBLEMS
	//gettimeofday(&timeData, 0);
	//return (timeData.tv_sec * 1000) + (timeData.tv_usec / 1000);
#endif
}

unsigned long PlatformTime :: getAbsoluteSeconds(void)
{
	time(&timeSecData);
	return static_cast<unsigned long>(timeSecData);
}

unsigned long PlatformTime :: getAbsoluteMinutes(void)
{
	time(&timeSecData);
	return static_cast<unsigned long>(timeSecData / 60);
}

unsigned long PlatformTime :: getElapsedMilliseconds(void)
{
	return g_ServerTime - g_ServerLaunchTime;
}