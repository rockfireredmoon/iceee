#pragma once
#ifndef SQUIRREL_H
#define SQUIRREL_H

#include "../sqrat/sqrat.h"
#include <string>

namespace Squirrel {

/**
 * Defines a single point in the XZ plane.
 */
class Point {
public:
	int mX;
	int mZ;

	Point(int x, int z) {
		mX = x;
		mZ = z;
	}

	Point() {
		mX = 0;
		mZ = 0;
	}
};


/**
 * Defines a rectangle region, with the top-left corner at x1,y1 and the bottom
 * right at x2, y2, OR define a point with a radius
 */
class Area {
public:
	int mX1;
	int mX2;
	int mY1;
	int mY2;
	int mRadius;

	Area() {
		mX1 = 0;
		mX2 = 0;
		mY1 = 0;
		mY2 = 0;
		mRadius = 0;
	}

	Area(int x1, int y1, int radius) {
		mX1 = x1 - radius;
		mY1 = y1 - radius;
		mX2 = x1 + radius;
		mY2 = y1 + radius;
		mRadius = radius;
	}

	Area(int x1, int y1, int x2, int y2) {
		mX1 = x1;
		mX2 = x2;
		mY1 = y1;
		mY2 = y2;
		mRadius = 0;
	}

	Point ToPoint() {
		return Point(mX1, mX2);
	}

};

/**
 * Defines a single point with XYZ coordinates.
 */
class Vector3I {
public:
	int mX;
	int mY;
	int mZ;

	Vector3I(int x, int y, int z) {
		mX = x;
		mY = y;
		mZ = z;
	}

	Vector3I() {
		mX = 0;
		mY = 0;
		mZ = 0;
	}

	void Set(int x, int y, int z) {
		mX = x;
		mY = y;
		mZ = z;
	}
};

/**
 * Defines a single point with XYZ coordinates.
 */
class Vector3 {
public:
	float mX;
	float mY;
	float mZ;

	Vector3() {
		mX = 0;
		mY = 0;
		mZ = 0;
	}

	Vector3(float x, float y, float z) {
		mX = x;
		mY = y;
		mZ = z;
	}

	void Set(float x, float y, float z) {
		mX = x;
		mY = y;
		mZ = z;
	}
};

class Printer {
public:
	void PrintTable(std::string *result, Sqrat::Table table);
	void PrintArray(std::string *result, Sqrat::Array array);
	void PrintValue(std::string *result, Sqrat::Object array);
private:
	char mPrintBuffer[1024];

};


}
#endif //#define SQUIRREL_H
