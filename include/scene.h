#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <tuple>
#include <functional>
#include <mutex>
#include <thread>

#include "object.h"
#include "drawable.h"

using namespace std;

enum SceneElementType {

	SET_OBJECT,
	SET_PARTICLE,
	SET_PARTICLE_INACTIVE,
	SET_EMPTY
};

enum SceneMode {

	SM_EXACT,
	SM_RANDOM
};

union PosUnion {

	PosUnion(): creationPoint(Point(0,0)) { };

	Point creationPoint;
	Rect region;

	~PosUnion() { };
};

typedef struct SceneSettings {

	SceneSettings(): sceneMode(SM_EXACT),
			 posUnion(),
			 usecCreate(300000),
			 usecUpdate(100000) { };

	SceneMode sceneMode;
	PosUnion posUnion;
	unsigned int usecCreate;
	unsigned int usecUpdate;

	~SceneSettings() { };

} SceneSettings;

typedef tuple<unsigned int, SceneElementType> SceneElement;
typedef vector<vector<SceneElement>> SceneMatrix;
typedef function<void(SceneElement&, Point&)> MatrixFunction;

class Scene: Drawable {

public:
	Scene(Rect, SceneSettings*);

	mutex sceneMutex;

	Point* windowCursorPosition;
	function<void(Point&)> reverse = NULL;
	function<void(Point&)> restore = NULL;

	bool addObject(Object);
	bool removeObjectAt(Point&);
	bool addParticle(Point&, bool);
	SceneElement& getSE(Point&);

	bool isValidPosition(Point&);
	void update(Rect);
	void draw();
	void step();
	bool start();
	void drawSingle(Point&);

	unsigned int usecCreate() { return this->sceneSettings->usecCreate; };
	unsigned int usecUpdate() { return this->sceneSettings->usecUpdate; };

	static void setExact(SceneSettings&, Point);
	static void setRandom(SceneSettings&, Rect);
	static void setTime(SceneSettings&, unsigned int, unsigned int);

	~Scene();

private:
	Rect size;
	SceneSettings* sceneSettings = NULL;

	vector<Object> objects;
	vector<Particle> particles;
	vector<Particle> inactiveParticles;
	SceneMatrix sceneMatrix;

	void drawEmpty(Point&);
	void forEachElement(MatrixFunction&);
	void filterParticles(vector<Particle>&);
};

#endif
