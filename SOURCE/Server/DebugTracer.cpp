#include "DebugTracer.h"

#include "Util.h"
#include "util/Log.h"


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
	g_Logs.server->trace("BEGIN: %v", message);
}
QuickTrace :: ~QuickTrace()
{
	fprintf(stderr, "END: %s\r\n", msg);
	fflush(stderr);
	g_Logs.server->trace("END: %v", msg);
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


