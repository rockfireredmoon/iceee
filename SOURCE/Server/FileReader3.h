#ifndef FILEREADER3_H
#define FILEREADER3_H

#include <stdio.h>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class FileReader3
{
public:

	//Return codes
	static const int FAILED = -1;
	static const int SUCCESS = 0;

	//Array sizes
	static const int BUFFERSIZE = 4096;
	static const int BLOCKCOUNT = 64;

	//Case conversion parameters to the BlockToString functions.
	static const int CASE_NONE = 0;
	static const int CASE_UPPER = 1;
	static const int CASE_LOWER = 2;

	char DataBuffer[BUFFERSIZE];            //Holds a single line of raw, unbroken text as read directly from the file.
	char CopyBuffer[BUFFERSIZE];            //Holds a copy of the retrieved block, used without modifying the contents of the original input. 
	
	FileReader3();
	~FileReader3();

	int OpenFile(const fs::path &filename);     //Open a file for reading.
	void CloseFile(void);                   //Close the file.
	bool Readable(void);                    //Check if the file is still readable (both open and not end-of-file).

	long lineNumber;                        //Line number that was last read.

	int ReadLine(void);                     //Read a single raw line from the file.  Trailing newline and whitespace are removed.
	void SetCommentChar(int delimitChar);   //Set the character that defines a comment.  All characters including the comment and beyond are ignored for breaks.
	int MultiBreak(const char *delim);      //Break a full string into an arbitrary number of blocks while using a one or more delimiters.
	int BreakUntil(const char *early, int final);  //Works like multibreak, but stops breaking a particular character is found.  Useful for stuff like: Sentence.1=Foobar.
	int SingleBreak(const char *delim);     //Split a line into a maximum of two blocks.
	int BlockToIntC(int block);             //Copy block, return integer.
	bool BlockToBoolC(int block);           //Copy block, return bool
	float BlockToFloatC(int block);         //Copy block, return float.
	char *BlockToStringC(int block);        //Copy a block's string data and return a pointer to the string.
	char *BlockToStringC(int block, int convertCase);  //As above, but performs a case conversion on standard alphabetical characters (A-Z).

private:
	FILE *fileHandle;
	char commentDelim;

	//Holds the buffer position and length that each tokenized block can be found.
	int BlockPos[BLOCKCOUNT];
	int BlockLen[BLOCKCOUNT];

	void RemoveTrailingWhitespace(void);
	void RemoveComment(void);
	void SetBlock(int block, int start, int length);
};

#endif //#ifndef FILEREADER3_H
