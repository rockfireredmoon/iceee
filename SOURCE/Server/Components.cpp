#include <ctime>
#include "Components.h"

#include <string.h>
#include <sys/time.h>
#include "util/Log.h"


char VersionString[] = {"Version 36b : " __DATE__ }; 

unsigned long g_ServerTime = 0;        //Current server time (milliseconds)
unsigned long g_ServerLaunchTime = 0;  //Server launch time (milliseconds)

int g_ServerStatus = 0;            //The current running status of the server.  See the ServerStatus enum.  
unsigned int g_ExitStatus = EXIT_SUCCESS;            //The exit status to be used on normal shutdown. Can be used by wrapper to restart service

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

Platform_CriticalSection :: Platform_CriticalSection(const std::string &name)
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
	lastLock = "";
	lockCount = 0;
	debugName = "";
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
#ifndef HAS_PTHREAD
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

void Platform_CriticalSection :: SetDebugName(const std::string &name)
{
	debugName = name;
	useDebugMessages = true;
	notifyWait = true;
}

void Platform_CriticalSection :: Delete(void)
{
	if(initialized == true)
	{
	#ifndef HAS_PTHREAD
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

void Platform_CriticalSection :: Enter(const std::string &requestDesc)
{
	if(disabled == true)
		return;

#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
	if(lockCount != 0)
	{
		if(notifyWait == true)
			if(useDebugMessages == true)
				g_Logs.server->debug("CriticalSection::Enter[%v] waiting, %v count, request:%v, last:%v", debugName, lockCount, requestDesc, lastLock);
	}
	if(initialized == false)
	{
		if(useDebugMessages == true)
			g_Logs.server->debug("Platform_CriticalSection::Enter[%v] on uninitialized section.", debugName);
		Init();
	}
#endif

#ifndef HAS_PTHREAD
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
			g_Logs.server->debug("TIME PASSED CriticalSection::Enter[%v] %v ms (%v).", debugName, passTime, lastLock);
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
#ifndef HAS_PTHREAD
	LeaveCriticalSection(&cs);
#else
	pthread_mutex_unlock(&mutex);
#endif

#ifdef DEBUG_TIME
	if(useDebugMessages == true)
	{
		long passTime = (long)g_PlatformTime.getMilliseconds() - (long)acquireTime;
		if(passTime > 100)
			g_Logs.server->debug("TIME PASSED CriticalSection::Leave[%v] %v ms (%v).", debugName, passTime, lastLock);
		//g_Log.AddMessageFormatW(MSG_SHOW, "[DEBUG] LEAVE CriticalSection[%s]", debugName);
	}
#endif
	lastLock = "";
}

int Platform_CriticalSection :: GetLockCount(void)
{
	return lockCount;
}

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

void PlatformTime :: Init() {
	mServerLaunchMontonic = getMonotonicMilliseconds();
	g_ServerLaunchTime = getUTCMilliSeconds();
	g_ServerTime = getMilliseconds();
}

unsigned long PlatformTime :: getMilliseconds(void)
{
	return g_ServerLaunchTime + getElapsedMilliseconds();
}

unsigned long PlatformTime :: getMonotonicMilliseconds(void)
{
#ifdef WINDOWS_PLATFORM
	return GetTickCount();
#else
	clock_getres(CLOCK_MONOTONIC_RAW, &timeSpec);
	clock_gettime(CLOCK_MONOTONIC_RAW, &timeSpec);
	return (timeSpec.tv_sec * 1000) + (timeSpec.tv_nsec / 1000000);
#endif
}

unsigned long PlatformTime :: getUTCMilliSeconds()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

unsigned long PlatformTime :: getLocalMilliSeconds()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

unsigned long PlatformTime :: getAbsoluteSeconds(void)
{
	return getMilliseconds() / 1000;
}

unsigned long PlatformTime :: getAbsoluteMinutes(void)
{
	return getAbsoluteSeconds() / 60;
}

unsigned long PlatformTime :: getElapsedMilliseconds(void)
{
	return getMonotonicMilliseconds() - mServerLaunchMontonic;
}

unsigned long PlatformTime :: getPseudoTimeOfDayMilliseconds(void)
{
	//return ( getElapsedMilliseconds() * TIME_FACTOR ) % 86400000;
	return ( getUTCMilliSeconds() * TIME_FACTOR ) % 86400000;
}
