#pragma once

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <vector>
#include <string>

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
int PutShort(char *buffer, int val);
int PutShortReq(void);
int PutInteger(char *buffer, int val);
int PutIntegerReq(void);
int PutFloat(char *buffer, float val);
int PutStringUTF(char *buffer, const char *tocopy);
int PutStringUTF(char *buffer, const std::string &tocopy);
int PutStringReq(const char *tocopy);

float GetFloat(const char *buffer, size_t &advance);
unsigned char GetByte(const char *buffer, size_t &advance);
unsigned short GetShort(const char *buffer, size_t &advance);
int GetInteger(const char *buffer, size_t &advance);

//The next two functions scan a string with text elements separated by spaces
//the function reads the text until the next space, or end of string if none is found
//the extract substring is converted into a numeric value of the required type and
//returned.
char *GetStringUTF(const char *buffer, char *recbuf, int bufsize, size_t &advance);
float GetPartFloat(const char *buffer, size_t &start);

#endif //BYTEBUFFER_H
