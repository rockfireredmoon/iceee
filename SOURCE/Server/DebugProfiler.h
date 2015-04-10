#ifndef DEBUGPROFILER_H
#define DEBUGPROFILER_H

#include "Report.h"
#include <map>

struct ProfileQueryInfo
{
	unsigned long mCount;
	unsigned long mTime;
	ProfileQueryInfo()
	{
		mCount = 0;
		mTime = 0;
	}
};


class DebugProfiler
{
public:
	DebugProfiler();
	~DebugProfiler();
	//inline void IncrementLobbyMsg(unsigned short msg, unsigned long time);
	//inline void IncrementGameMsg(unsigned short msg, unsigned long time);
	//inline void AddQuery(std::string &name, unsigned long time);

inline void IncrementLobbyMsg(unsigned char msg, unsigned long time)
{
	if(msg >= 21)
		return;
	mMsgLobbyCount[msg]++;
	mMsgLobbyTime[msg] += time;
}

inline void IncrementGameMsg(unsigned char msg, unsigned long time)
{
	if(msg >= 21)
		return;
	mMsgGameCount[msg]++;
	mMsgGameTime[msg] += time;
}

inline void AddQuery(std::string &name, unsigned long time)
{
	ProfileQueryInfo *pqi = &mQuery[name];
	pqi->mCount++;
	pqi->mTime += time;
}

inline void AddString(const char * name, unsigned long time)
{
	ProfileQueryInfo *pqi = &mQuery[name];
	pqi->mCount++;
	pqi->mTime += time;
}

	void GenerateReport(ReportBuffer& output);

private:
	unsigned long mMsgLobbyCount[21];
	unsigned long mMsgLobbyTime[21];
	unsigned long mMsgGameCount[21];
	unsigned long mMsgGameTime[21];
	std::map<std::string, ProfileQueryInfo> mQuery;
};


class TimeObject
{
public:
	TimeObject();
	TimeObject(const char *name);
	~TimeObject();

	unsigned long GetTimePassed(void);

private:
	unsigned long mStartTime;
	const char *mName;
};


/*
class TimeObjectHP
{
public:
	TimeObjectHP();
	TimeObjectHP(const char *name);
	~TimeObjectHP();

	unsigned long GetTimePassed(void);

private:
	LARGE_INTEGER mStartTime;
	LARGE_INTEGER mTicksPerSecond;
	const char *mName;
};
*/

extern DebugProfiler _DebugProfiler;

#endif  //#ifndef DEBUGPROFILER_H