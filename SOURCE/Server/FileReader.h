#pragma once

#ifndef FILEREADER2_H
#define FILEREADER2_H

#include <stdio.h>
#include <stdlib.h>

extern char Delimit_KeyVal[4];
extern char Default_Break[2];

enum FR2_ReadCase       //Used to convert text case using the GetLineBlock() function.
{
	Case_None = 0,    //No change
	Case_Upper,      //Convert to upper case
	Case_Lower     //Convert to lower case
};

enum ReadCode
{
	FR_Null = 0,    //No text
	FR_Norm,        //Normal text
	FR_BlockStart,  //Start of a block
	FR_BlockEnd,    //End of a block
	FR_EOF          //End of file
};

enum ErrorCode
{
	Err_FileInvalid = -1,
	Err_OK = 0
};

enum _AccessMode
{
	Mode_Text = 1,
	Mode_Binary
};

enum _FileHandle
{
	File_Primary = 0,
	File_Secondary
};

enum CommentType
{
	Comment_Slash = 0x01,
	Comment_Semi  = 0x02,
	Comment_Both  = 0x03
};

class FileReader
{
public:
	FileReader();
	~FileReader();

	static const int BUFFERSIZE = 4096;
	static const int MULTIBLOCKCOUNT = 64;

	//All strings assume text file information and therefore must include the endline characters
	// of 13 and 10.

	char DataBuffer[BUFFERSIZE];
	char SecBuffer[BUFFERSIZE];  //Secondary buffer
	int BlockPos[MULTIBLOCKCOUNT];
	int BlockLen[MULTIBLOCKCOUNT];
	char *Delimiter;

	int CommentStyle;        //Truncates lines if a certain comment style is found

	FILE *FileHandle[2];
	long FilePos;
	
	int ActiveFile;
	int LineNumber;  //Current line number, is incremented by functions that read entire lines

	int OpenText(const char *filename);

	void CloseCurrent(void);
	bool FileOpen(void);

	void SeekStart(void);  //Seeks back to the start of the active file
	int ReadLine(void);    //Reads from the current position to the end of the line, placing the results in the main

	int SingleBreak(const char *delimiter);   //Splits a string at the first occurrence of a character.  Alters DataBuffer.
	int MultiBreak(const char *delimiter);    //Splits at multiple occurrences of a character.  Does not alters DataBuffer.  Use BlockTo___C copy functions.
	int BreakUntil(const char *early, int final);

	char *BlockToString(int block);
	char *BlockToStringC(int block, int convcase);   //Copies a block into SecBuffer, returning a pointer to SecBuffer.  Does not modify DataBuffer, which contains the entire line's string.

	int BlockToInt(int block);
	bool BlockToBool(int block);
	int BlockToIntC(int block);
	unsigned long BlockToULongC(int block);
	bool BlockToBoolC(int block);

	double BlockToDbl(int block);
	double BlockToDblC(int block);
	float BlockToFloatC(int block);

	char *CopyBlock(int block);

	void RemoveTrailingWhitespace(void);
	int RemoveBeginningWhitespace(void);
};

#endif //FILEREADER_H

