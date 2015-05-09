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
}
