#pragma once
#ifndef DEBUGTRACER_H
#define DEBUGTRACER_H

// Undefine to remove functionality.
#define USE_SIMPLE_TRACE

//For file logging of messages through the g_Log system.
#define USE_SERVER_LOG

#ifdef USE_SIMPLE_TRACE

class SimpleTracerCore
{
public:
	SimpleTracerCore();
	SimpleTracerCore(int size);
	~SimpleTracerCore();

	unsigned int *data;
	int index;
	int capacity;
	int Create(int size);
	void Free(void);
	void AddTrace(unsigned int value);
};

// This class is designed to trace an instance of a specific function.
// On normal operation, the constructor is called, writing the opening message.
// When the function exists, the trace object is removed from the stack, calling
// the destructor, which writes a closing message.
// If the function causes abnormal program termination or gets stuck in an infinte
// loop, the destructure won't be called.
class QuickTrace
{
public:
	QuickTrace(const char *message);
	~QuickTrace();
private:
	const char *msg;
};

void DumpTracerFile(const char *filename);

extern SimpleTracerCore g_Tracer;

#define FNS                     0
#define FNE                     255

#define TRACE_INIT(x)           g_Tracer.Create(x)
#define TRACE(fn,val)           g_Tracer.AddTrace(fn << 8 | val)
#define TRACES(fn)              g_Tracer.AddTrace(fn << 8 | FNS)
#define TRACEE(fn)              g_Tracer.AddTrace(fn << 8 | FNE)
#define TRACE_FREE()            g_Tracer.Free()
#define QTRACE(msgStr)          QuickTrace _trace_inst(msgStr)
class TraceHelper
{
public:
	unsigned long fnID;
	TraceHelper(int functionID)
	{
		fnID = functionID;
		TRACES(functionID);
	}
	~TraceHelper()
	{
		TRACEE(fnID);
	}
};
#else  //USE_SIMPLE_TRACE
  #define FNS                     EMPTY_OPERATION
  #define FNE                     EMPTY_OPERATION
  #define TRACE_INIT(x)           EMPTY_OPERATION
  #define TRACE(fn,val)           EMPTY_OPERATION
  #define TRACES(fn)              GENERATE_ERROR
  #define TRACEE(fn)              GENERATE_ERROR
  #define TRACE_FREE()            EMPTY_OPERATION
  #define TRACE_OUTPUT(x)         EMPTY_OPERATION
  #define QTRACE(msgStr)          EMPTY_OPERATION
#endif //USE_SIMPLE_TRACE

#ifdef USE_SERVER_LOG
#include <stdio.h>
#include <ctime>
extern char LogFileName[];
extern FILE *LogHandle;
void Log_Open(void);
void Log_Close(void);
void Log_Write(const char *text);
void Log_WriteFormat(char *format, ...);
void Log_Flush(void);

#define LOG_OPEN() Log_Open()
#define LOG_CLOSE() Log_Close()
#define LOG_WRITE(x) Log_Write(x)
#define LOG_WRITEF(x, ...) Log_WriteFormat(x, __VA_ARGS__)
#define LOG_FLUSH() Log_Flush()
#else
#define LOG_OPEN() __noop
#define LOG_CLOSE() __noop
#define LOG_WRITE(x) __noop
#define LOG_WRITEF(x) __noop
#define LOG_FLUSH() __noop
#endif //USE_SERVER_LOG

#endif //DEBUGTRACER_H
