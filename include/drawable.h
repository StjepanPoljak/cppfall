#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <iostream>

class Drawable {

public:
	virtual void draw() = 0;

};

class Rect {

public:
	Rect(unsigned int width, unsigned int height): width(width), height(height) { };
	Rect(): width(0), height(0) { };

	unsigned int width;
	unsigned int height;

	~Rect() { };
};

#endif
