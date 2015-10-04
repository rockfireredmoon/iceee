#ifndef REPORT_H
#define REPORT_H

// A central system for aggregating a block of formatted text information which can then be dumped
// to an arbitrary source as one item.
// It allows arbitrary newlines, like "\n" for console output or "<br>\r\n" for a web page.
#include <string>

class ReportBuffer
{
public:
	ReportBuffer();
	ReportBuffer(const char *newLineStyle);
	ReportBuffer(size_t customMaxSize, const char *newLineStyle);
	~ReportBuffer();

	void AddLine(const char *format, ...);         //Adds a line of text.  Newlines are automatically appended when necessry.
	void AppendLine(const char *format, ...);      //Adds a line of text.  Newlines are automatically appended when necessry.
	const char* getData(void);               //Retrieve a pointer to the string.
	const char* getTrimData(size_t length);  //Trim the output if it exceeds this length, then return a pointer to the string.
	void SetNewLine(const char *str);        //Set the report to use a different string of newline characters.
	bool WasTruncated(void);                 //Test if the returned string by getTrimData() was truncated.
	void Clear(void);                        //Erase the buffer.
	void SetMaxSize(size_t newSize);         //Explicitly set the maximum buffer size.
	void SetNewLineStyle(const char *newLineStyle);  //Explicitly set the newline style.
	void Truncate(size_t limitSize, const char *label);
	size_t getLength(void);

	static const char *NEWLINE_N;
	static const char *NEWLINE_RN;
	static const char *NEWLINE_WEB;

private:
	size_t maxSize;          //If the data length exceeds this, consider the buffer to be full and ignore new data.
	std::string outString;   //Contains all output.
	bool wasTruncated;       //If getTrimData() truncated the result, this is set. 

	const char *newLine;     //The selected newline format.

	char PrepBuf[4096];      //Used as a local output buffer for generating text strings with vsprintf()
	int prepSize;            //String size contained in PrepBuf

	void AppendPrep();       //Appends the the contents of PrepBuf to the generated data.
	void AppendNewLine();    //Appends a newline to the generated data.
	void InitDefaults(void);
};

#endif  //#ifndef REPORT_H