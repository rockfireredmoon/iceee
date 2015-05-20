#pragma once
#ifndef SCRIPTOBJECTS_H
#define SCRIPTOBJECTS_H

/**
 * Self contained objects and structs that are used for server side Squirrel scripting
 */

namespace ScriptObjects {

/**
 * Defines a rectangle region, with the top-left corner at x1,y1 and the bottom
 * right at x2, y2
 */
class Area {
public:
	int mX1;
	int mX2;
	int mY1;
	int mY2;

	Area() {
		mX1 = 0;
		mX2 = 0;
		mY1 = 0;
		mY2 = 0;
	}

	Area(int x1, int y1, int x2, int y2) {
		mX1 = x1;
		mX2 = x2;
		mY1 = y1;
		mY2 = y2;
	}

};

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
 * Defines a single point with XYZ coordinates.
 */
class Vector3 {
public:
	int mX;
	int mY;
	int mZ;

	Vector3(int x, int y, int z) {
		mX = x;
		mY = y;
		mZ = z;
	}

	Vector3() {
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
class Vector3F {
public:
	float mX;
	float mY;
	float mZ;

	Vector3F() {
		mX = 0;
		mY = 0;
		mZ = 0;
	}

	Vector3F(float x, float y, float z) {
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

}
#endif //#define SCRIPTOBJECTS_H
