#ifndef OBJECT_H
#define OBJECT_H

#include "drawable.h"

class Point {

public:
	Point(int x, int y): x(x), y(y) { };
	Point(): x(0), y(0) { };

	int x;
	int y;

	bool operator ==(const Point& p);
	Point operator +(const Point& p);
	Point operator -(const Point& p);

	Point reverse();

	~Point() { };
};

enum Direction {
	D_DOWN,
	D_LEFT,
	D_RIGHT
};

class Object: Drawable {

public:
	Object(Point pos, char c): objectPosition(pos), symbol(c) { };
	Object(int x, int y, char c): objectPosition(x, y), symbol(c) { };
	Object(int x, int y): Object(x, y, '#') { };
	Object(Point pos): Object(pos.x, pos.y) { };

	Point objectPosition;

	void draw();

	~Object() { };

private:
	char symbol;
};

class Particle: public Object {

public:
	Particle(int x, int y, Direction direction): Object(x, y, 'O'), direction(direction) { };
	Particle(int x, int y): Particle(x, y, D_DOWN) { };
	Particle(Point pos): Particle(pos.x, pos.y) { };

	Direction direction;
	bool toBeRemoved = false;

	~Particle() { };
};

#endif
