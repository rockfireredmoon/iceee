//Implements a rotating storage list.  It uses a preset array of elements to store values.
//Writing items increases the pending item count and advances the write position.
//Reading items decreases the pending item count and advances the read position.
//If the list is full, continued writing will overwrite the oldest elements and increase
//an overflow counter.
//The read and write positions will wrap around to zero if they exceed the boundary
//of the array size.

#pragma once

#ifndef ROTATINGLIST_H
#define ROTATINGLIST_H

#include <vector>

template<class T>
class RotatingList
{
public:
	RotatingList();
	RotatingList(int max);
	~RotatingList();

	std::vector<T> AllocList;

	int MaxSize;         //Maximum element size of AllocList, the Read and Write indexes will be contained within this size.
	int ReadPos;         //Active index that will return that earliest unread message.
	int WritePos;        //Active index that will receive the next written message.
	int PendingRead;     //Number of messages written since the last read operation.
	int OverflowCount;   //Number of messages dropped that couldn't fit due to a full PendingRead queue.

	void Resize(int max);

	void Destroy(void);

	int GetReadIndex(void);
	int GetWriteIndex(void);
	void IncrementPending(void);
	int PeekIndex(void);

	T& operator [] (int index);
};

template<class T>
RotatingList<T> :: RotatingList()
{
	MaxSize = 0;
	ReadPos = 0;
	WritePos = 0;
	PendingRead = 0;
	OverflowCount = 0;
}

template<class T>
RotatingList<T> :: RotatingList(int max)
{
	Resize(max);
}

template<class T>
RotatingList<T> :: ~RotatingList()
{
	Destroy();
}


template<class T>
void RotatingList<T> :: Resize(int max)
{
	//Resize the array.  All reading and writing positions are reset. 
	AllocList.resize(max);
	MaxSize = max;
	ReadPos = 0;
	WritePos = 0;
	PendingRead = 0;
	OverflowCount = 0;
}

template<class T>
void RotatingList<T> :: Destroy(void)
{
	//Clear the elements in the list.
	AllocList.clear();
}

template<class T>
int RotatingList<T> :: GetReadIndex(void)
{
	//Return the current read index and advance the read position.
	if(PendingRead == 0)
		return -1;

	int OldRead = ReadPos;
	ReadPos++;
	if(ReadPos > MaxSize - 1)
		ReadPos = 0;

	PendingRead--;

	return OldRead;
}

template<class T>
int RotatingList<T> :: GetWriteIndex(void)
{
	//Return the current write index and advance the write position.
	//To be used if a function needs to edit the array externally.
	//Based off the code in AddItem but modified slightly.
	//There is no assignment, and the pending count is NOT incremented, which must
	//be incremented externally.

	int OldWrite = WritePos;
	WritePos++;
	if(WritePos > MaxSize - 1)
		WritePos = 0;

	return OldWrite;
}

template<class T>
void RotatingList<T> :: IncrementPending(void)
{
	//To be used by an external call to increment the pending count.
	PendingRead++;
	if(PendingRead > MaxSize - 1)
	{
		OverflowCount++;
		PendingRead = MaxSize - 1;
		ReadPos++;
		if(ReadPos > MaxSize - 1) 
			ReadPos = 0;
	}
}

template<class T>
int RotatingList<T> :: PeekIndex(void)
{
	//Returns the current read position, but does not advance it.
	if(PendingRead == 0)
		return -1;

	return ReadPos;
}

template<class T>
T& RotatingList<T> :: operator[] (int index)
{
	//Return the raw data at the given array index, regardless of read position.
	return AllocList[index];
}

#endif  //ROTATINGLIST_H