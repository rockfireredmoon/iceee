#include "DebugProfiler.h"
#include <string.h>
#include "Components.h"

DebugProfiler _DebugProfiler;

DebugProfiler::DebugProfiler()
{
	memset(mMsgLobbyCount, 0, sizeof(mMsgLobbyCount));
	memset(mMsgLobbyTime, 0, sizeof(mMsgLobbyTime));
	memset(mMsgGameCount, 0, sizeof(mMsgGameCount));
	memset(mMsgGameTime, 0, sizeof(mMsgGameTime));
}
DebugProfiler::~DebugProfiler()
{
}


void DebugProfiler::GenerateReport(ReportBuffer& output)
{
	size_t i;
	for(i = 0; i < 21; i++)
	{
		if(mMsgLobbyCount[i] > 0)
			output.AddLine("Lobby msg[%d] %d, %lu", i, mMsgLobbyCount[i], mMsgLobbyTime[i]);
	}

	for(i = 0; i < 21; i++)
	{
		if(mMsgGameCount[i] > 0)
			output.AddLine("Game msg[%d] %d, %lu", i, mMsgGameCount[i], mMsgGameTime[i]);
	}
	
	std::map<std::string, ProfileQueryInfo>::iterator it;
	for(it = mQuery.begin(); it != mQuery.end(); ++it)
		output.AddLine("%s : %d, %lu", it->first.c_str(), it->second.mCount, it->second.mTime);

	output.AddLine("Report finished.");
}


TimeObject :: TimeObject()
{
	mStartTime = g_PlatformTime.getMilliseconds();
	mName = NULL;
}

TimeObject :: TimeObject(const char *name)
{
	mStartTime = g_PlatformTime.getMilliseconds();
	mName = name;
}

unsigned long TimeObject :: GetTimePassed(void)
{
	return g_PlatformTime.getMilliseconds() - mStartTime;
}

TimeObject :: ~TimeObject()
{
	if(mName != NULL)
		_DebugProfiler.AddString(mName, g_PlatformTime.getMilliseconds() - mStartTime);
}


/*
TimeObjectHP :: TimeObjectHP()
{
	QueryPerformanceCounter(&mStartTime);
	QueryPerformanceFrequency(&mTicksPerSecond);
	mName = NULL;
}

TimeObjectHP :: TimeObjectHP(const char *name)
{
	QueryPerformanceCounter(&mStartTime);
	QueryPerformanceFrequency(&mTicksPerSecond);
	mName = name;
}

unsigned long TimeObjectHP :: GetTimePassed(void)
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	LARGE_INTEGER pass = ((endTime - mStartTime) / mTicksPerSecond) * 1000;

	return (unsigned long)pass;
}

TimeObjectHP :: ~TimeObjectHP()
{
	if(mName != NULL)
		_DebugProfiler.AddString(mName, g_PlatformTime.getMilliseconds() - mStartTime);
}
*/