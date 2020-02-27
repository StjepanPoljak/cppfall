#ifndef WINDOW_H
#define WINDOW_H

#include <thread>

#include <termios.h>

#include "scene.h"

class Window {

public:
	Window();

	void refreshSize();
	bool startScene(SceneSettings*);
	void cleanup();

	bool onKeyPressed(char[3]);

	void reverseAt(Point&);
	void restoreAt(Point&);

	Rect getSize() { return windowSize; };

	static void move(Point&);
	static void drawChar(Point&, char);
	static void reverse(Point&);
	static void restore(Point&);
	static void clearScreen();
	static void cursSet(bool);

	static void waitForInput(Window*);

	~Window();

private:
	Rect windowSize;
	Scene* scene;
	std::thread* inputThread;
	Point cursorPos = Point(0,0);
	struct termios orig_term_attr;
};
#endif
