#pragma once
#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_TIME
#define TIMETRACK(labelString) Debug::TimeTrack  _timetrack(labelString)
#define TIMETRACKF(dummy)  _timetrack.Finish()
#define TIMETRACKP(param1, param2) _timetrack.SetParam(param1, param2)
#else
#define TIMETRACK(labelString)   
#define TIMETRACKF(dummy)
#define TIMETRACKP(param1, param2)
#endif


#include "Components.h"

#include "util/Log.h"
#include <time.h>

namespace Debug
{
	// Instantiation of a TimeTrack object will automatically provide time calculation and debug messages
	// when the object is deconstructed (such as falling out of scope)/
	class TimeTrack
	{
	public:
		TimeTrack(const char *label, int reportTime = 100)
		{
			startTime = g_PlatformTime.getMilliseconds();
			displayLabel = label;
			param1 = 0;
			param2 = 0;
			reportThreshold = reportTime;
		}
		~TimeTrack()
		{
			Finish();
		}
		void Finish(void)
		{
			long timePass = (long)g_PlatformTime.getMilliseconds() - (long)startTime;
			if(timePass > reportThreshold)
			{
				if(param1 != 0 || param2 != 0)
					g_Logs.server->debug("TIME PASS %v, %v ms (%v, %v)", displayLabel, timePass, param1, param2);
				else
					g_Logs.server->debug("TIME PASS %v, %v ms", displayLabel, timePass);
			}
		}
		void SetParam(int p1, int p2)
		{
			param1 = p1;
			param2 = p2;
		}
		unsigned long startTime;
		const char *displayLabel;
		int param1;
		int param2;
		int reportThreshold;
	};


	class TimeTrackStep
	{
	public:
		TimeTrackStep(const char *label)
		{
			lastTime = g_PlatformTime.getMilliseconds();
			displayLabel = label;
		}
		~TimeTrackStep()
		{
			Finish();
		}
		void Update(const char *label)
		{
			displayLabel = label;
			unsigned long curTime = g_PlatformTime.getMilliseconds();
			long timePass = (long)curTime - (long)lastTime;
			g_Logs.server->debug("UPDATE TIME PASS %v, %v ms", displayLabel, timePass);
			lastTime = curTime;
		}
		void Finish(void)
		{
			long timePass = (long)g_PlatformTime.getMilliseconds() - (long)lastTime;
			if(timePass > 5)
				g_Logs.server->debug("FINISH TIME PASS %v, %v ms", displayLabel, timePass);
		}
		unsigned long lastTime;
		const char *displayLabel;
	};
} // namespace Debug

#endif  //#ifndef DEBUG_H

