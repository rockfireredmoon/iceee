#include "FileReader3.h"
#include <string.h>
#include <stdlib.h>

FileReader3 :: FileReader3()
{
	memset(DataBuffer, 0, sizeof(DataBuffer));
	memset(CopyBuffer, 0, sizeof(CopyBuffer));

	memset(BlockPos, 0, sizeof(BlockPos));
	memset(BlockLen, 0, sizeof(BlockLen));

	fileHandle = NULL;
	lineNumber = 0;
	commentDelim = ';';
}

FileReader3 :: ~FileReader3()
{
	if(fileHandle != NULL)
	{
		fclose(fileHandle);
		fileHandle = NULL;
	}
}

int FileReader3 :: OpenFile(const char *filename)
{
	if(fileHandle != NULL)
		return SUCCESS;
	fileHandle = fopen(filename, "rb");
	if(fileHandle == NULL)
		return FAILED;
	return SUCCESS;
}

void FileReader3 :: CloseFile(void)
{
	if(fileHandle != NULL)
	{
		fclose(fileHandle);
		fileHandle = NULL;
	}
}
	
bool FileReader3 :: Readable(void)
{
	if(fileHandle == NULL)
		return false;
	if(feof(fileHandle))
		return false;
	return true;
}


int FileReader3 :: ReadLine(void)
{
	//end of file won't be triggered until after attempting to read data, which means the
	//final line will stay resident in the buffer and processed a second time by loading functions
	//unless removed.
	DataBuffer[0] = 0;

	//Need to reserve an extra character of space for the break functions.
	fgets(DataBuffer, sizeof(DataBuffer) - 1, fileHandle);

	RemoveTrailingWhitespace();
	RemoveComment();

	lineNumber++;

	return strlen(DataBuffer);
}

void FileReader3 :: RemoveTrailingWhitespace(void)
{
	size_t len = strlen(DataBuffer);
	for(size_t pos = len - 1; pos >= 0; pos--)
	{
		switch(DataBuffer[pos])
		{
		case '\r':
		case '\n':
		case ' ':
		case '\t':
			DataBuffer[pos] = 0;
			len--;
			break;
		default:
			return;
		}
	}
}

void FileReader3 :: RemoveComment(void)
{
	if(commentDelim == 0)
		return;
	size_t len = strlen(DataBuffer);
	for(size_t pos = 0; pos < len; pos++)
	{
		if(DataBuffer[pos] == commentDelim)
		{
			DataBuffer[pos] = 0;
			return;
		}
	}
}

void FileReader3 :: SetCommentChar(int delimitChar)
{
	commentDelim = delimitChar;
}

int FileReader3 :: MultiBreak(const char *delim)
{
	char *first = DataBuffer;
	char *last = first + strlen(DataBuffer);
	
	int blockCount = 0;
	while(blockCount < BLOCKCOUNT - 1)
	{
		char *second = strpbrk(first, delim);
		if(second != NULL)
		{
			SetBlock(blockCount++, first - DataBuffer, second - first);
			first = second + 1;
		}
		else
			break;
	}
	if(first < last)
		SetBlock(blockCount++, first - DataBuffer, last - first);

	SetBlock(blockCount, 0, 0);  //Finish with an empty block (don't increment)
	return blockCount;
}

int FileReader3 :: BreakUntil(const char *early, int final)
{
	//Same as multibreak, but with an added cheak
	char *first = DataBuffer;
	char *last = first + strlen(DataBuffer);
	
	int blockCount = 0;
	while(blockCount < BLOCKCOUNT - 1)
	{
		char *second = strpbrk(first, early);
		if(second != NULL)
		{
			SetBlock(blockCount++, first - DataBuffer, second - first);
			first = second + 1;
			if(*second == final)
				break;
		}
		else
			break;
	}
	if(first < last)
		SetBlock(blockCount++, first - DataBuffer, last - first);

	SetBlock(blockCount, 0, 0);  //Finish with an empty block (don't increment)

	return blockCount;
}

int FileReader3 :: SingleBreak(const char *delim)
{
	char *first = DataBuffer;
	char *last = first + strlen(DataBuffer);
	char *second = strpbrk(first, delim);
	int blockCount = 0;
	if(second != NULL)
	{
		SetBlock(blockCount++, first - DataBuffer, second - first);
		first = second + 1;
	}
	if(first < last)  //Need this check otherwise empty blocks will count.
		SetBlock(blockCount++, first - DataBuffer, last - first);
	SetBlock(blockCount, 0, 0);  //Finish with an empty block (don't increment)
	return blockCount;
}

void FileReader3 :: SetBlock(int block, int start, int length)
{
	if(block >= BLOCKCOUNT)
		return;
	BlockPos[block] = start;
	BlockLen[block] = length;
}

int FileReader3 :: BlockToIntC(int block)
{
	strncpy(CopyBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	CopyBuffer[BlockLen[block]] = 0;
	return atoi(CopyBuffer);
}

bool FileReader3 :: BlockToBoolC(int block)
{
	return (BlockToIntC(block) != 0);
}

float FileReader3 :: BlockToFloatC(int block)
{
	strncpy(CopyBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	CopyBuffer[BlockLen[block]] = 0;
	return static_cast<float>(atof(CopyBuffer));
}

char * FileReader3 :: BlockToStringC(int block)
{
	strncpy(CopyBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	CopyBuffer[BlockLen[block]] = 0;
	return CopyBuffer;
}

char * FileReader3 :: BlockToStringC(int block, int convertCase)
{
	strncpy(CopyBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	CopyBuffer[BlockLen[block]] = 0;

	if(convertCase == CASE_UPPER)
	{
		for(int i = 0; i < BlockLen[block]; i++)
			if(CopyBuffer[i] >= 'a' && CopyBuffer[i] <= 'z')
				CopyBuffer[i] -= 32;
	}
	else if(convertCase == CASE_LOWER)
	{
		for(int i = 0; i < BlockLen[block]; i++)
			if(CopyBuffer[i] >= 'A' && CopyBuffer[i] <= 'Z')
				CopyBuffer[i] += 32;
	}
	return CopyBuffer;
}

