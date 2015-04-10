//Contains a list of message strings. Strings added to the list are dynamically allocated
//and copied from their source destination.  Allows assignment of arbitrary strings
//or formatted output, provided the internal FormatBuffer array is large enough to store
//the data.

#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <stdarg.h>
#include <vector>
#include <string>

#include "Components.h"

enum FilterType
{
	MSG_SHOW   = (1 << 0),   //Always show
	MSG_CRIT   = (1 << 1),   //Critical error
	MSG_ERROR  = (1 << 2),   //Non-critical error
	MSG_WARN   = (1 << 3),   //Warning
	MSG_DIAG   = (1 << 4),   //Standard diagnostic
	MSG_DIAGV  = (1 << 5),   //Verbose diagnostic
	MSG_DIAGEX = (1 << 6),   //Extreme verbose diagnostic
	MSG_ALL    = 0xFFFF
};

class StringList
{
public:
	StringList();
	StringList(int max);
	~StringList();
	void Destroy(void);

	void AddMessage(const char *value);
	void AddMessageFormat(const char *format, ...);
	void AddMessageFormatArg(const char *format, va_list args);
	void AddMessageFormatW(unsigned short flags, const char *format, ...);

	bool LoggingEnabled;
	unsigned int Filter;                  //Bit flag for filtering messages
	char formatBuffer[4096];              //Local buffer for generating a string
	std::vector<std::string> stringList;  //List of strings pending in the output buffer
	volatile int pendingCount;            //Number of strings in the queue for the main thread to process
	Platform_CriticalSection string_cs; 

	void GetThread(const char *request);
	void ReleaseThread(void);
};

extern StringList g_Log;

#endif  //STRINGLIST_H