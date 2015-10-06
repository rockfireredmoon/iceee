#ifndef INTARRAY_H
#define INTARRAY_H

#include <stddef.h>   //Needed for size_t definition in Code::Blocks
#include <string.h>

template <size_t _Rows, size_t _Columns>
class IntArray
{
public:
	IntArray();
	bool ValidEntry(size_t row, size_t column);
	int GetValue(size_t row, size_t column);
	bool SetValue(size_t row, size_t column, int value);
	static const int INVALID = 0xFFFFFFFF;
private:
	int mData[_Rows][_Columns];
	size_t mRows;
	size_t mColumns;
};

template <size_t _Rows, size_t _Columns>
IntArray<_Rows,_Columns> :: IntArray()
{
	mRows = _Rows;
	mColumns = _Columns;
	memset(mData, 0, _Rows * _Columns * sizeof(int));
}

template <size_t _Rows, size_t _Columns>
bool IntArray<_Rows,_Columns> :: ValidEntry(size_t row, size_t column)
{
	if(row >= mRows || column >= mColumns)
		return false;
	return true;
}

template <size_t _Rows, size_t _Columns>
int IntArray<_Rows,_Columns> :: GetValue(size_t row, size_t column)
{
	if(ValidEntry(row, column) == false)
		return INVALID;
	return mData[row][column];
}

template <size_t _Rows, size_t _Columns>
bool IntArray<_Rows,_Columns> :: SetValue(size_t row, size_t column, int value)
{
	if(ValidEntry(row, column) == false)
		return false;
	mData[row][column] = value;
	return true;
}

#endif //#ifndef INTARRAY_H