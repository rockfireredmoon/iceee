#pragma once

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <vector>

#ifndef WINDOWS_PLATFORM
	#include <sys/types.h>
#endif



// Note: Not sure if this actually works.  Was intended to replace the unsafe buffer calls for
// something that can expand to allow arbitrarily large sizes.  However, performance benchmarks
// on this system rated much slower than the unsafe raw buffer calls.
class BinaryBuffer
{
public: 
	std::vector<unsigned char> mData;
	int mWritePos;

	BinaryBuffer();
	~BinaryBuffer();
	void PutByte(int value);
	void PutShort(int value);
	void PutInteger(int value);
	void PutFloat(float value);
	void PutStringUTF(const char *tocopy);

	const char *GetData(void);
	unsigned long GetSize(void);
};


int PutByte(char *buffer, int val);
unsigned char GetByte(const char *buffer, int &advance);
int PutShort(char *buffer, int val);
int PutShortReq(void);
unsigned short GetShort(const char *buffer, int &advance);
int PutInteger(char *buffer, int val);
int PutIntegerReq(void);
int GetInteger(const char *buffer, int &advance);
int PutFloat(char *buffer, float val);
float GetFloat(const char *buffer, int &advance);
int PutStringUTF(char *buffer, const char *tocopy);
int PutStringReq(const char *tocopy);
char *GetStringUTF(const char *buffer, char *recbuf, int bufsize, int &advance);

//The next two functions scan a string with text elements separated by spaces
//the function reads the text until the next space, or end of string if none is found
//the extract substring is converted into a numeric value of the required type and
//returned.
float GetPartFloat(const char *buffer, int &start);

#endif //BYTEBUFFER_H
