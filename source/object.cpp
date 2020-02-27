#include "object.h"

#include "window.h"

bool Point::operator ==(const Point& p) {

	return (this->x == p.x && this->y == p.y);
}

Point Point::operator +(const Point& p) {

	return (Point(this->x + p.x, this->y + p.y));
}

Point Point::operator -(const Point &p) {

	return (Point(this->x - p.x, this->y - p.y));
}

Point Point::reverse() {

	return Point(this->y, this->x);
}

void Object::draw() {

	Window::drawChar(this->objectPosition, this->symbol);
}
