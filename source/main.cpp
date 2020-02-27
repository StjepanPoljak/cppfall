#include <iostream>
#include <signal.h>

#include "window.h"

Window* window;

void intHandler(int sig) {

	window->cleanup();
	exit(0);
}

int main(int argc, char* argv[]) {

	SceneSettings sceneSettings = SceneSettings();

	signal(SIGINT, intHandler);

	Window mainWindow = Window();
	window = &mainWindow;

	Scene::setExact(sceneSettings, Point(mainWindow.getSize().width / 2, 0));

	if(!mainWindow.startScene(&sceneSettings)) {

		return 1;
	}

	return 0;
}
