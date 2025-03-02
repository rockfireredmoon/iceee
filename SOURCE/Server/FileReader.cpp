#include <string.h>
#include <stdio.h>
#include "FileReader.h"
#include "util/Log.h"

char Delimit_KeyVal[4] = {'=', 13, 10, 0};   //Standard config delimiters
char Default_Break[2] = {'=', 0};

FileReader :: FileReader()
{
	CommentStyle = 0;
	FileHandle[0] = NULL;
	FileHandle[1] = NULL;
	FilePos = 0;
	ActiveFile = 0;
	LineNumber = 0;
	memset(&DataBuffer[0], 0, BUFFERSIZE);
	memset(&SecBuffer[0], 0, BUFFERSIZE);
	memset(BlockPos, 0, sizeof(BlockPos));
	memset(BlockLen, 0, sizeof(BlockLen));
	Delimiter = &Delimit_KeyVal[0];
}

FileReader :: ~FileReader()
{
	if(FileHandle[0] != NULL)
	{
		fclose(FileHandle[0]);
		FileHandle[0] = NULL;
	}

	if(FileHandle[1] != NULL)
	{
		fclose(FileHandle[1]);
		FileHandle[1] = NULL;
	}

	FilePos = 0;
	ActiveFile = 0;
}

bool FileReader :: FileOpen(void)
{
	if(FileHandle[ActiveFile] == NULL)
		return false;
	if(feof(FileHandle[ActiveFile]))
		return false;

	return true;
}

int FileReader :: OpenText(const fs::path &filename)
{
	if(g_Logs.server->Enabled(el::Level::Trace)) {
		g_Logs.server->trace("Opening text file %v", filename);
	}
	if((FileHandle[File_Primary] = fopen(filename.string().c_str(), "rb")) == NULL)
		return Err_FileInvalid;

	LineNumber = 0;
	FilePos = 0;
	
	return Err_OK;
}

void FileReader :: CloseCurrent(void)
{
	if(FileHandle[ActiveFile] != NULL)
	{
		fclose(FileHandle[ActiveFile]);
		FileHandle[ActiveFile] = NULL;
		ActiveFile--;
	}

	if(ActiveFile < 0)
		ActiveFile = 0;
}

void FileReader :: SeekStart(void)
{
	if(FileHandle[ActiveFile] == NULL)
		return;

	fseek(FileHandle[ActiveFile], 0, SEEK_SET);
	FilePos = 0;
	LineNumber = 0;
}

int FileReader :: ReadLine(void)
{
	char *pos = fgets(DataBuffer, sizeof(DataBuffer), FileHandle[ActiveFile]);
	bool line = false;
	if(pos != NULL)
	{
		int len = strlen(DataBuffer);
		for(int a = len - 1; a >= 0; a--)
		{
			if(DataBuffer[a] == '\n' || DataBuffer[a] == '\r')
			{
				DataBuffer[a] = 0;
				line = true;
			}
			else
				break;
		}
	}
	else
	{
		//Error, possibly end of file.  No string returned so remove the existing string.
		DataBuffer[0] = 0;
	}
	if(line == true)
		LineNumber++;

	if(CommentStyle & Comment_Slash)
	{
		pos = strstr(DataBuffer, "//");
		if(pos != NULL)
			*pos = 0;
	}
	if(CommentStyle & Comment_Semi)
	{
		pos = strchr(DataBuffer, ';');
		if(pos != NULL)
			*pos = 0;
	}
	RemoveTrailingWhitespace();
	return strlen(DataBuffer);
}

int FileReader :: SingleBreak(const char *delimiter)
{
	if(DataBuffer[0] == 0)
		return 0;

	const char *del = delimiter;
	if(del == NULL)
		del = Default_Break;

	int len = strlen(DataBuffer);
	char *spos = strpbrk(DataBuffer, del);
	if(spos != NULL)
	{
		int cpos = spos - DataBuffer;
		DataBuffer[cpos] = 0;
		BlockPos[0] = 0;
		BlockLen[0] = cpos;
		BlockPos[1] = cpos + 1;
		BlockLen[1] = len - cpos - 1;
		BlockPos[2] = 0;
		BlockLen[2] = 0;
		return 2;
	}
	BlockPos[0] = 0;
	BlockLen[0] = len;
	BlockPos[1] = 0;
	BlockPos[1] = 0;
	return 1;
}

/*
int FileReader :: SingleBreak2(char *delimiter)
{
	if(DataBuffer[0] == 0)
		return 0;

	char *del = delimiter;
	if(del == NULL)
		del = Default_Break;

	int len = strlen(DataBuffer);
	char *spos = strpbrk(DataBuffer, del);
	if(spos != NULL)
	{
		int cpos = spos - DataBuffer;
		BlockPos[0] = 0;
		BlockLen[0] = cpos;
		BlockPos[1] = cpos + 1;
		BlockLen[1] = len - cpos - 1;
		return 2;
	}
	return 1;
}
*/

int FileReader :: MultiBreak(const char *delimiter)
{
	if(DataBuffer[0] == 0)
		return 0;

	const char *del = delimiter;
	if(del == NULL)
		del = Default_Break;

	int len = strlen(DataBuffer);
	int CurBlock = 0;
	int Start = 0;
	char *spos = NULL;
	int cpos = 0;


	while(Start < len)
	{
		spos = strpbrk(&DataBuffer[Start], del);
		if(spos != NULL)
		{
			//A block was found
			cpos = spos - DataBuffer;
			BlockPos[CurBlock] = Start;
			BlockLen[CurBlock] = cpos - Start;
			Start = cpos + 1;
			CurBlock++;
			if(CurBlock >= MULTIBLOCKCOUNT - 1)
				break;

			//Scan to the beginning of the next non-breaking block
			while(Start < len)
			{
				spos = strpbrk(&DataBuffer[Start], del);
				if(spos != NULL)
				{
					cpos = spos - DataBuffer;
					if(cpos != Start)
						break;
					else
						Start++;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			//There's still a block remaining between this and the end of the string
			if(Start < len)
			{
				BlockPos[CurBlock] = Start;
				BlockLen[CurBlock] = len - Start;
				CurBlock++;
				if(CurBlock >= MULTIBLOCKCOUNT - 1)
					break;
				Start = len;
			}
		}
	}

	BlockPos[CurBlock] = 0;
	BlockLen[CurBlock] = 0;

	return CurBlock;
}

int FileReader :: BreakUntil(const char *early, int final)
{
	//Splits a string using the delimiters in <early> but breaking when the character
	//in <final> is matched.  This allows strings like "data.0=12.34" to be broken.

	int epos = strlen(DataBuffer);
	int lpos = 0;
	int cpos = 0;
	char *spos = NULL;
	int count = 0;
	do
	{
		spos = strpbrk(&DataBuffer[lpos], early);
		if(spos != NULL)
		{
			cpos = spos - DataBuffer;
			BlockPos[count] = lpos;
			BlockLen[count] = cpos - lpos;

			cpos++;
			lpos = cpos;
			count++;
			if(*spos == final)
				break;
		}
	} while(spos != NULL && cpos < epos);

	if(cpos < epos)
	{
		BlockPos[count] = cpos;
		BlockLen[count] = epos - cpos;
		count++;
	}

	if(count > 0)
	{
		BlockPos[count] = BlockPos[count - 1] + BlockLen[count - 1];
		BlockLen[count] = 0;
	}
	else
	{
		BlockPos[0] = 0;
		BlockLen[0] = 0;
	}
	return count;
}

char* FileReader :: BlockToString(int block)
{
	return &DataBuffer[BlockPos[block]];
}

char* FileReader :: BlockToStringC(int block, int convcase)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;

	int a;
	if(convcase == Case_Upper)
		for(a = 0; a < BlockLen[block]; a++)
			if(SecBuffer[a] >= 'a' && SecBuffer[a] <= 'z')
				SecBuffer[a] -= 32;

	if(convcase == Case_Lower)
		for(a = 0; a < BlockLen[block]; a++)
			if(SecBuffer[a] >= 'A' && SecBuffer[a] <= 'Z')
				SecBuffer[a] += 32;

	return SecBuffer;
}

int FileReader :: BlockToInt(int block)
{
	return atoi(&DataBuffer[BlockPos[block]]);
}

bool FileReader :: BlockToBool(int block)
{
	int val = atoi(&DataBuffer[BlockPos[block]]);
	if(val > 0)
		return true;

	return false;
}

int FileReader :: BlockToIntC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	return atoi(SecBuffer);
}


long FileReader :: BlockToLongC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	return strtol(SecBuffer, NULL, 10);
}
unsigned long FileReader :: BlockToULongC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	return strtoul(SecBuffer, NULL, 10);
}

bool FileReader :: BlockToBoolC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	int r = atoi(SecBuffer);
	if(r == 0)
		return false;

	return true;
}

double FileReader :: BlockToDbl(int block)
{
	return atof(&DataBuffer[BlockPos[block]]);
}

double FileReader :: BlockToDblC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	return atof(SecBuffer);
}

float FileReader :: BlockToFloatC(int block)
{
	strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BlockLen[block]);
	SecBuffer[BlockLen[block]] = 0;
	return static_cast<float>(atof(SecBuffer));
}

char* FileReader :: CopyBlock(int block)
{
	if(BlockLen[block] > 0)
	{
		strncpy(SecBuffer, &DataBuffer[BlockPos[block]], BUFFERSIZE - 1);
		SecBuffer[BlockLen[block]] = 0;
	}
	else
	{
		SecBuffer[0] = 0;
	}

	return SecBuffer;
}

void FileReader :: RemoveTrailingWhitespace(void)
{
	int len = strlen(DataBuffer);
	int pos = 0;
	for(pos = len - 1; pos >= 0; pos--)
	{
		if(DataBuffer[pos] == ' ')
			DataBuffer[pos] = 0;
		else if(DataBuffer[pos] == '\t')
			DataBuffer[pos] = 0;
		else
			break;
	}
}

int FileReader :: RemoveBeginningWhitespace(void)
{
	int pos = 0;
	int len = strlen(DataBuffer);
	int fpos = 0;
	for(pos = 0; pos < len; pos++)
	{
		if(DataBuffer[pos] != ' ' && DataBuffer[pos] != '\t')
		{
			fpos = pos;
			break;
		}
	}
	if(fpos > 0)
	{
		for(pos = 0; pos < len - fpos; pos++)
			DataBuffer[pos] = DataBuffer[pos + fpos];
		DataBuffer[len - fpos] = 0;
		len -= fpos;
	}

	return len;
}
