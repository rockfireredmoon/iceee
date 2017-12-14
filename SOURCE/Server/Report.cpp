#include <stdarg.h>
#include <string.h>
#include "Report.h"
#include "Util.h"

const char * ReportBuffer::NEWLINE_N = "\n";
const char * ReportBuffer::NEWLINE_RN = "\r\n";
const char * ReportBuffer::NEWLINE_WEB = "<br>\r\n";

ReportBuffer :: ReportBuffer()
{
	InitDefaults();
	maxSize = 4096;
	outString.reserve(1024);
}

ReportBuffer :: ReportBuffer(const char *newLineStyle)
{
	InitDefaults();
	maxSize = 4096;
	outString.reserve(1024);
	SetNewLineStyle(newLineStyle);
}

ReportBuffer :: ReportBuffer(size_t customMaxSize, const char *newLineStyle)
{
	InitDefaults();
	if(customMaxSize == 0)
		customMaxSize = outString.max_size();
	maxSize = customMaxSize;
	SetNewLineStyle(newLineStyle);
}

ReportBuffer :: ~ReportBuffer()
{
	outString.clear();
}

void ReportBuffer :: InitDefaults(void)
{
	memset(PrepBuf, 0, sizeof(PrepBuf));
	prepSize = 0;
	
	newLine = NEWLINE_N;
	wasTruncated = false;
}

void ReportBuffer :: AddLine(const char *format, ...)
{
	if(outString.size() > maxSize)
		return;

	if(format == NULL)
	{
		AppendNewLine();
		return;
	}

	va_list args;
	va_start (args, format);
	prepSize = Util::SafeFormatArg(PrepBuf, sizeof(PrepBuf), format, args);
	va_end (args);
	
	if(prepSize > 0)
	{
		AppendPrep();
		AppendNewLine();
	}
}

void ReportBuffer :: AppendLine(const char *format, ...)
{
	if(outString.size() > maxSize)
		return;

	if(format == NULL)
	{
		AppendNewLine();
		return;
	}

	va_list args;
	va_start (args, format);
	prepSize = Util::SafeFormatArg(PrepBuf, sizeof(PrepBuf), format, args);
	va_end (args);

	if(prepSize > 0)
		AppendPrep();
}

void ReportBuffer :: AppendPrep(void)
{
	outString.append(PrepBuf);
}

void ReportBuffer :: AppendNewLine(void)
{
	outString.append(newLine);
}

const char* ReportBuffer :: getData(void)
{
	return outString.c_str();
}

const char* ReportBuffer :: getTrimData(size_t length)
{
	if(outString.size() >= length)
	{
		outString.erase(length, outString.size());
		wasTruncated = true;
	}
	return outString.c_str();
}

void ReportBuffer :: SetNewLine(const char *str)
{
	if(str == NULL)
		newLine = NEWLINE_N;
	newLine = str;
}

bool ReportBuffer :: WasTruncated(void)
{
	return wasTruncated;
}

void ReportBuffer :: Clear(void)
{
	outString.clear();
	memset(PrepBuf, 0, sizeof(PrepBuf));
	prepSize = 0;
	wasTruncated = false;
}

void ReportBuffer :: SetMaxSize(size_t newSize)
{
	if(newSize == 0)
		newSize = outString.max_size();
	
	maxSize = newSize;
	if(outString.size() > maxSize)
	{
		outString.erase(maxSize, outString.size());
		wasTruncated = true;
	}
	else
	{
		wasTruncated = false;
	}
}

void ReportBuffer :: SetNewLineStyle(const char *newLineStyle)
{
	if(newLineStyle != NULL)
		newLine = newLineStyle;
	else
		newLine = NEWLINE_N;
}

void ReportBuffer :: Truncate(size_t limitSize, const char *label)
{
	if(outString.size() <= limitSize)
		return;

	int extra = 0;
	if(label != NULL)
	{
		extra = strlen(label);
		extra += strlen(newLine);
	}
	int pos = (int)limitSize - extra - 1; 
	if(pos < 0)
		pos = 0;

	outString.erase(pos, outString.size());

	if((outString.size() + extra) > limitSize)
		return;

	if(label != NULL)
	{
		AppendNewLine();
		outString.append(label);
	}
}

size_t ReportBuffer :: getLength(void)
{
	return outString.size();
}
