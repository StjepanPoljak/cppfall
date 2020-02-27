#include "window.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "drawable.h"

Window::Window() {

	struct termios new_term_attr;

	tcgetattr(fileno(stdin), &this->orig_term_attr);
	memcpy(&new_term_attr, &this->orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	this->refreshSize();

	Window::clearScreen();
	Window::cursSet(false);

	this->scene = NULL;

	this->inputThread = new std::thread(Window::waitForInput, this);

	return;
}

bool Window::startScene(SceneSettings* sceneSettings) {

	this->scene = new Scene(this->windowSize, sceneSettings);

	this->scene->reverse = Window::reverse;
	this->scene->restore = Window::restore;
	this->scene->windowCursorPosition = &this->cursorPos;

	return this->scene->start();
}

void Window::refreshSize() {

	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

	this->windowSize.width = size.ws_col;
	this->windowSize.height = size.ws_row - 1;
}

void Window::cleanup() {

	inputThread->detach();
	tcsetattr(fileno(stdin), TCSANOW, &this->orig_term_attr);
	Window::cursSet(true);

	delete this->inputThread;
	if(this->scene) delete this->scene;

	Window::clearScreen();
}

bool Window::onKeyPressed(char key[3]) {

	if (key[0] == 27 && key[1] == 0 && key[2] == 0) {

		return false;
	}

	else if (key[0] == 13 || key[0] == 10 && key[1] == 0 && key[2] == 0) {

		this->scene->sceneMutex.lock();
		if(!this->scene->addObject(Object(this->cursorPos)))
			this->scene->removeObjectAt(this->cursorPos);
		this->scene->sceneMutex.unlock();
	}

	else if (key[0] == 27 && key[1] == 91) {

		Point oldCursorPos = this->cursorPos;

		switch(key[2]) {

			case 65:
				this->cursorPos.y--;
				break;
			case 66:
				this->cursorPos.y++;
				break;
			case 67:
				this->cursorPos.x++;
				break;
			case 68:
				this->cursorPos.x--;
				break;
			default:
				break;
		}

		if(!this->scene->isValidPosition(this->cursorPos)) {

			this->cursorPos = oldCursorPos;
		}

		if(!(oldCursorPos == this->cursorPos)) {

			this->scene->sceneMutex.lock();
			this->scene->drawSingle(oldCursorPos);
			this->scene->drawSingle(this->cursorPos);
			this->scene->sceneMutex.unlock();
		}
	}

	return true;
}

Window::~Window() {

	cleanup();
}

// ******* static ********

void Window::move(Point &p) {

	std::cout << "\033[" << p.y + 1 << ";" << p.x + 1 << "H";
}

void Window::restore(Point &p) {

	Window::move(p);
	cout << "\033[0m\n";
}

void Window::drawChar(Point &p, char c) {

	Window::move(p);
	std::cout << c << "\n";
}

void Window::reverse(Point &p) {

	Window::move(p);
	cout << "\033[7m\n";
}

void Window::clearScreen() {

	std::cout << "\033c"; fflush(stdout);
}

void Window::cursSet(bool state) {

	std::cout << "\033[?25" << (state ? "h" : "l") << "\n";
}

void Window::waitForInput(Window *window) {

	fd_set input_set, output_set;

	for(;;) {
		usleep(50);

		char buff[3];
		memset(buff, 0, 3);
		FD_ZERO(&input_set);
		FD_SET(STDIN_FILENO, &input_set);

		int readn = select(1, &input_set, NULL, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &input_set)) {
			int buffread = read(STDIN_FILENO, buff, 3);

			if(!window->onKeyPressed(buff)) {
				window->cleanup();
				exit(0);
			}
		}
	}
}
