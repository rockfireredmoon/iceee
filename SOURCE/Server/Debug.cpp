#include "Debug.h"
#include <stdio.h>

namespace Debug
{
	FileLogger eventLogger("EventLog.log");
	FileLogger cheatLogger("CheatLog.log");

	/*
	FileLogger :: FileLogger()
	{
		output = NULL;
		nextAutoSave = 0;
	}
	*/
	FileLogger :: FileLogger(const char *filename)
	{
		mFileName = filename;
		Open();
		mNextAutoSave = 0;
	}
	FileLogger :: ~FileLogger(void)
	{
		Close();
	}
	void FileLogger :: Open()
	{
		if(mOutputStream == NULL)
			mOutputStream = fopen(mFileName, "a");
	}
	void FileLogger :: Close(void)
	{
		if(mOutputStream != NULL)
		{
			fclose(mOutputStream);
			mOutputStream = NULL;
		}
	}
	void FileLogger :: Flush(void)
	{
		if(mOutputStream != NULL)
			fflush(mOutputStream);
	}
	void FileLogger :: Log(const char *format, ...)
	{
		if(mOutputStream != NULL)
		{
			time_t curtime;
			time(&curtime);
			strftime(mTimeBuf, sizeof(mTimeBuf), "%x %X", localtime(&curtime));
			fprintf(mOutputStream, "%s ", mTimeBuf);

			va_list args;
			va_start (args, format);
			vfprintf(mOutputStream, format, args);
			va_end (args);
			fprintf(mOutputStream, "\r\n");
		}
	}
	void FileLogger :: Log(const char *format, va_list args)
	{
		if(mOutputStream != NULL)
		{
			time_t curtime;
			time(&curtime);
			strftime(mTimeBuf, sizeof(mTimeBuf), "%x %X", localtime(&curtime));
			fprintf(mOutputStream, "%s ", mTimeBuf);
			vfprintf(mOutputStream, format, args);
			fprintf(mOutputStream, "\r\n");
		}
	}
	void FileLogger :: CheckAutoSave(bool force)
	{
		if(g_ServerTime >= mNextAutoSave || force == true)
		{
			Flush();
			mNextAutoSave = g_ServerTime + AUTOSAVE_TIME;
		}
	}
	void Init(void)
	{
		eventLogger.Open();
		cheatLogger.Open();
	}
	void Shutdown(void)
	{
		eventLogger.Flush();
		eventLogger.Close();
		cheatLogger.Flush();
		cheatLogger.Close();
	}
	void Log(const char *format, ...)
	{
		va_list args;
		va_start (args, format);
		eventLogger.Log(format, args);
		va_end (args);
	}
	void CheckAutoSave(bool force)
	{
		eventLogger.CheckAutoSave(force);
	}
}