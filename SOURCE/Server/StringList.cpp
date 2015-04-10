#include "StringList.h"

#include <stdlib.h>
#include <stdio.h>
#include "Util.h"

extern bool g_GlobalLogging;

StringList g_Log(100);

StringList :: StringList()
{
	Filter = MSG_ALL;
	pendingCount = 0;
	memset(formatBuffer, 0, sizeof(formatBuffer));
	LoggingEnabled = true;
}

StringList :: StringList(int max)
{
	stringList.reserve(max);
	Filter = MSG_ALL;
	pendingCount = 0;
	memset(formatBuffer, 0, sizeof(formatBuffer));
	LoggingEnabled = true;
	string_cs.Init();
}

StringList :: ~StringList()
{
	LoggingEnabled = false;
	Destroy();
}

void StringList :: Destroy(void)
{
	stringList.clear();
}

void StringList :: AddMessage(const char *value)
{
	if(LoggingEnabled == false)
		return;
	if(value == NULL)
		return;
	if(value[0] == 0)
		return;

	string_cs.Enter("StringList::AddMessage");
	stringList.push_back(value);
	pendingCount++;
	string_cs.Leave();
}

void StringList :: AddMessageFormat(const char *format, ...)
{
	if(LoggingEnabled == false)
		return;

	string_cs.Enter("StringList::AddMessageFormat");
	va_list args;
	va_start (args, format);
	Util::SafeFormatArg(formatBuffer, sizeof(formatBuffer), format, args);
	va_end (args);

	stringList.push_back(formatBuffer);
	pendingCount++;
	string_cs.Leave();
}

void StringList :: AddMessageFormatArg(const char *format, va_list args)
{
	if(LoggingEnabled == false)
		return;

	string_cs.Enter("StringList::AddMessageFormat");
	Util::SafeFormatArg(formatBuffer, sizeof(formatBuffer), format, args);
	stringList.push_back(formatBuffer);
	pendingCount++;
	string_cs.Leave();
}

void StringList :: AddMessageFormatW(unsigned short messageType, const char *format, ...)
{
	if(LoggingEnabled == false)
		return;
	if(!(Filter & messageType))
		return;
	string_cs.Enter("StringList::AddMessageFormatW");
	va_list args;
	va_start (args, format);
	Util::SafeFormatArg(formatBuffer, sizeof(formatBuffer), format, args);
	va_end (args);

	stringList.push_back(formatBuffer);
	pendingCount++;
	string_cs.Leave();
}

void StringList :: GetThread(const char *request)
{
	string_cs.Enter(request);
}

void StringList :: ReleaseThread(void)
{
	string_cs.Leave();
}