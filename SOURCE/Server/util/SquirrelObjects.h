/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

#pragma once
#ifndef SQUIRRELOBJECTS_H
#define SQUIRRELOBJECTS_H

#include "../sqrat.h"
#include "../json/json.h"
#include <string>
#include <math.h>

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

	bool operator==(Area &other) {
		return mX1 == other.mX1 && mX2 == other.mX2 && mY1 == other.mY1 && mY2 == other.mY2;
	}

	bool Inside(int x, int y) {
		return x >= mX1 && x <= mX2 && y >= mY1 && y <= mY2;
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

	int Distance(Vector3I other) {
		int xlen = abs(mX - other.mX);
		int zlen = abs(mZ - other.mZ);
		int ylen = abs(mY - other.mY);
		double dist = sqrt((double) ((xlen * xlen) + (zlen * zlen)));
		int tdist = (int) dist;
		dist = sqrt((double) ((ylen * ylen) + (tdist * tdist)));
		return (int) dist;
	}

	int DistanceOnPlane(Vector3I other) {
		int xlen = abs(mX - other.mX);
		int zlen = abs(mZ - other.mZ);
		double dist = sqrt((double) ((xlen * xlen) + (zlen * zlen)));
		return (int) dist;
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

	float Distance(Vector3 other) {
		float xlen = abs(mX - other.mX);
		float zlen = abs(mZ - other.mZ);
		float ylen = abs(mY - other.mY);
		double dist = sqrt((double) ((xlen * xlen) + (zlen * zlen)));
		int tdist = (int) dist;
		return sqrt((double) ((ylen * ylen) + (tdist * tdist)));
	}

	float DistanceOnPlane(Vector3 other) {
		int xlen = abs(mX - other.mX);
		int zlen = abs(mZ - other.mZ);
		return sqrt((double) ((xlen * xlen) + (zlen * zlen)));
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

class JsonPrinter {
public:
	void PrintTable(Json::Value &result, Sqrat::Table table);
	void PrintArray(Json::Value &result, Sqrat::Array array);
	Json::Value PrintValue(Sqrat::Object array);
private:
	char mPrintBuffer[1024];
};


}
#endif //#define SQUIRRELOBJECTS_H
