#include "StringList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Util.h"

#include "util/Log.h"

StringList g_Log(100);

StringList :: StringList()
{
	memset(formatBuffer, 0, sizeof(formatBuffer));
}

StringList :: StringList(int max)
{
	memset(formatBuffer, 0, sizeof(formatBuffer));
	string_cs.Init();
}

StringList :: ~StringList()
{
	Destroy();
}

void StringList :: Destroy(void)
{
}

void StringList :: AddMessageFormat(const char *format, ...)
{
	string_cs.Enter("StringList::AddMessageFormat");
	va_list args;
	char buf[4096];
	va_start (args, format);
	std::string f = "[FIXME] ";
	f.append(format);
	Util::SafeFormatArg(buf, sizeof(buf), f.c_str(), args);
	va_end (args);
	g_Logs.server->info(buf);
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
