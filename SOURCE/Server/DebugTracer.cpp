#include "DebugTracer.h"
#include "StringList.h"
#include "Util.h"


#ifdef USE_SIMPLE_TRACE

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
//#pragma warning(disable: 4996)

SimpleTracerCore g_Tracer;

SimpleTracerCore :: SimpleTracerCore()
{
	data = NULL;
	index = 0;
	capacity = 0;
}

SimpleTracerCore :: ~SimpleTracerCore()
{
	Free();
}

SimpleTracerCore :: SimpleTracerCore(int size)
{
	data = NULL;
	index = 0;
	capacity = 0;
	Create(size);
}

int SimpleTracerCore :: Create(int size)
{
	if(data != NULL)
		return -1;
	try
	{
		data = new unsigned int[size];
	}
	catch(...)
	{
		return -1;
	}
	if(data == NULL)
		return -1;

	memset(data, 0, sizeof(unsigned int) * size);
	capacity = size;
	index = 0;
	return size;
}

void SimpleTracerCore :: Free(void)
{
	if(data == NULL)
		return;

	delete [] data;
	capacity = 0;
	index = 0;
	data = NULL;
}

void SimpleTracerCore :: AddTrace(unsigned int value)
{
	data[index] = value;
	index++;
	if(index >= capacity)
		index = 0;
}


QuickTrace :: QuickTrace(const char *message)
{
	msg = message;
	fprintf(stderr, "BEGIN: %s\r\n", message);
	fflush(stderr);
	g_Log.AddMessageFormat("[TRACE] BEGIN: %s", message);
}
QuickTrace :: ~QuickTrace()
{
	fprintf(stderr, "END: %s\r\n", msg);
	fflush(stderr);
	g_Log.AddMessageFormat("[TRACE] END: %s", msg);
}

void CheckPoint(char *format, ...)
{
	static char Buffer[1024];

	va_list args;
	va_start (args, format);
	//vsnprintf(Buffer, maxSize, format, args);
	Util::SafeFormatArg(Buffer, sizeof(Buffer), format, args);
	va_end (args);

	//MessageBox(NULL, Buffer, "CheckPoint", MB_OK);
}

#endif //USE_SIMPLE_TRACE


#ifdef USE_SERVER_LOG
char LogFileName[] = "ServerLog.txt";
FILE *LogHandle = NULL;
void Log_Open(void);
void Log_Close(void);
//void Log_Write(char *text);
void Log_WriteFormat(char *format, ...);
void Log_Flush(void);

void Log_Open(void)
{
	LogHandle = fopen(LogFileName, "wb");
	if(LogHandle == NULL)
		g_Log.AddMessageFormat("[ERROR] Could not create server log file.");
}

void Log_Close(void)
{
	if(LogHandle != NULL)
		fclose(LogHandle);
}

void Log_Write(const char *text)
{
	if(LogHandle == NULL)
		return;

	//clock_t thisTime = clock();
	//int milli = (int)(((float)(thisTime % CLOCKS_PER_SEC)) / 1000.0F) * 100.0F);
	time_t timeval;
	time(&timeval);
	tm *curtime = localtime(&timeval);
	fprintf(LogHandle, "%02d:%02d:%02d %s\r\n", curtime->tm_hour, curtime->tm_min, curtime->tm_sec, text);
}

void Log_WriteFormat(char *format, ...)
{
	static char buffer[4096];

	va_list args;
	va_start (args, format);
	//vsnprintf(buffer, maxSize, format, args);
	Util::SafeFormatArg(buffer, sizeof(buffer), format, args);
	va_end (args);

	Log_Write((const char*)buffer);
}

void Log_Flush(void)
{
	if(LogHandle != NULL)
		fflush(LogHandle);
}

#endif //USE_SERVER_LOG
