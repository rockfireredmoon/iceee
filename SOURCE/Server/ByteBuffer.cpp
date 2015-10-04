#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ByteBuffer.h"

#include <vector>

BinaryBuffer::BinaryBuffer()
{
	mWritePos = 0;
}

BinaryBuffer::~BinaryBuffer()
{
}

void BinaryBuffer::PutByte(int value)
{
	mData.push_back(value & 0xFF);
}

void BinaryBuffer::PutShort(int value)
{
	mData.push_back((value & 0xFF00) >> 8);
	mData.push_back(value & 0x00FF);
}

void BinaryBuffer::PutInteger(int value)
{
	mData.push_back((value & 0xFF000000) >> 24);
	mData.push_back((value & 0x00FF0000) >> 16);
	mData.push_back((value & 0x0000FF00) >> 8);
	mData.push_back(value & 0x000000FF);
}

void BinaryBuffer::PutFloat(float value)
{
	const char *data = (const char*)&value;
	mData.insert(mData.end(), data[0], data[4]);
	/*
	char *data = (char*)&val;
	mData.push_back(data[3]);
	mData.push_back(data[2]);
	mData.push_back(data[1]);
	mData.push_back(data[0]);
	*/
}

void BinaryBuffer::PutStringUTF(const char *tocopy)
{
	int len = strlen(tocopy);
	if(len < 255)
	{
		mData.push_back(len);
		if(len > 0)
			mData.insert(mData.end(), tocopy[0], tocopy[len]);
	}
	else
	{
		mData.push_back(0xFF);
		mData.push_back((len & 0xFF00) >> 8);
		mData.push_back(len & 0x00FF);
		mData.insert(mData.end(), tocopy[0], tocopy[len]);
	}
}

const char* BinaryBuffer::GetData(void)
{
	return reinterpret_cast<const char*>(&mData[0]);
}

unsigned long BinaryBuffer::GetSize(void)
{
	return mData.size();
}

int PutByte(char *buffer, int val)
{
	buffer[0] = val & 0xFF;
	return 1;
}

unsigned char GetByte(const char *buffer, int &advance)
{
	advance += 1;
	return (unsigned char)buffer[0];
}

int PutShort(char *buffer, int val)
{
	buffer[0] = (val & 0xFF00) >> 8;
	buffer[1] = val & 0x00FF;
	return 2;
}

int PutShortReq(void)
{
	//Return the number of bytes needed to store a Short.
	//Using this instead of having magic numbers elsewhere in code.
	return 2;
}

unsigned short GetShort(const char *buffer, int &advance)
{
	short retval = ((unsigned char)buffer[0] << 8) | (unsigned char)buffer[1];
	advance += 2;
	return retval;
}

int PutInteger(char *buffer, int val)
{
	//If the incoming val is decimal 33 (0x00000021), we need to flip the byte order for the outgoing
	//contents so it looks like 0x21000000
	//This goes for other numeric Put*() functions, and applies to their corresponding
	//Get*() functions as well.
	buffer[0] = (val & 0xFF000000) >> 24;
	buffer[1] = (val & 0x00FF0000) >> 16;
	buffer[2] = (val & 0x0000FF00) >> 8;
	buffer[3] = (val & 0x000000FF);

	return 4;
}

int PutIntegerReq(void)
{
	//Return the number of bytes needed to store an Integer.
	//Using this instead of having magic numbers elsewhere in code.
	return 4;
}

int GetInteger(const char *buffer, int &advance)
{
	int retval = ((unsigned char)buffer[0] << 24) | ((unsigned char)buffer[1] << 16) | ((unsigned char)buffer[2] << 8) | (unsigned char)buffer[3];
	advance += 4;
	return retval;
}

int PutFloat(char *buffer, float val)
{
	char *data = (char*)&val;
	buffer[0] = data[3];
	buffer[1] = data[2];
	buffer[2] = data[1];
	buffer[3] = data[0];
	return 4;
}

float GetFloat(const char *buffer, int &advance)
{
	float retval = 0;
	char *data = (char*)&retval;
	data[0] = buffer[3];
	data[1] = buffer[2];
	data[2] = buffer[1];
	data[3] = buffer[0];
	advance += 4;
	return retval;
}

int PutStringUTF(char *buffer, const char *tocopy)
{
	int len = strlen(tocopy);
	if(len < 255)
	{
		buffer[0] = len;
		if(len > 0)
			memcpy(&buffer[1], tocopy, len);
		return len + 1;
	}
	else
	{
		buffer[0] = (char)0xFF;
		buffer[2] = len & 0x00FF;
		buffer[1] = (len & 0xFF00) >> 8;
		memcpy(&buffer[3], tocopy, len);
		return len + 3;
	}
}

// Return the binary size required to hold a string as written
// by PutStringUTF, but doesn't actually write anything.
int PutStringReq(const char *tocopy)
{
	int len = strlen(tocopy);
	if(len < 255)
		return len + 1;
	else
		return len + 3;
}

char *GetStringUTF(const char *buffer, char *recbuf, int bufsize, int &advance)
{
	int len = (unsigned char)buffer[0];
	int pos = 1;
	if(len == 0xFF)
	{
		len = ((unsigned char)buffer[1] << 8) | (unsigned char)buffer[2];
		pos = 3;
	}
	int tocopy = len;
	if(tocopy > bufsize - 1)
		tocopy = bufsize - 1;
	if(tocopy > 0)
		memcpy(recbuf, &buffer[pos], tocopy);
	recbuf[tocopy] = 0;
	advance += len + pos;
	return recbuf;
}

float GetPartFloat(const char *buffer, int &start)
{
	int len = strlen(buffer);

	if(start >= len)
		return 0.0;

	char Temp[16];
	const char *pos = strchr(&buffer[start], ' ');
	if(pos != NULL)
		len = pos - buffer - start;

	if(len > 15)
		len = 15;
	
	strncpy(Temp, &buffer[start], len);
	Temp[len] = 0;
	start += len + 1;
	return (float)atof(Temp);
}
