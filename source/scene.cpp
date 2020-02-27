#include "scene.h"

#include <iostream>
#include <chrono>
#include <algorithm>

Scene::Scene(Rect size, SceneSettings* sceneSettings) {

	this->size = size;
	this->sceneSettings = sceneSettings;

	this->sceneMatrix = vector<vector<SceneElement>>(this->size.width);

	for(auto &col: this->sceneMatrix)

		col = vector<SceneElement>(this->size.height);

	MatrixFunction f = [this](SceneElement& se, Point& pos) {

		this->getSE(pos) = tuple<unsigned int, SceneElementType> (pos.x, SET_EMPTY);
	};

	this->forEachElement(f);
}

void Scene::forEachElement(MatrixFunction& f) {

	Point pos = Point(0, 0);

	for(auto &col: this->sceneMatrix) {
		pos.y = 0;
		for(auto &el: col) {
			f(el, pos);
			pos.y++;
		}
		pos.x++;
	}
}

SceneElement& Scene::getSE(Point &p) {

	return this->sceneMatrix[p.x][p.y];
}

bool Scene::isValidPosition(Point& p) {

	return (p.x >= 0 && p.x < this->size.width && p.y >= 0 && p.y < this->size.height);
}

void Scene::update(Rect size) {

	this->size = size;

	//TODO: decide later what happens on resize, but for now just:
	this->draw();
}

void Scene::step() {

	for(auto& particle: this->particles) {

		auto moveTo = [this, &particle](Point& pTo) {

			auto& p = particle.objectPosition;
			unsigned int prevLocation = get<0>(this->getSE(p));
			get<0>(this->getSE(p)) = 0;
			get<1>(this->getSE(p)) = SET_EMPTY;

			this->drawSingle(p);

			if(isValidPosition(pTo)) {
				particle.objectPosition = pTo;
				get<0>(this->getSE(pTo)) = prevLocation;
				get<1>(this->getSE(pTo)) = SET_PARTICLE;
				this->drawSingle(pTo);
			}
		};

		auto pD = particle.objectPosition + Point(0, 1);

		if(!isValidPosition(pD)) {
			particle.toBeRemoved = true;
			moveTo(pD);
		} else {
			auto& seD = this->getSE(pD);

			if(get<1>(seD) == SET_EMPTY) {
				moveTo(pD);
			}
			else if(get<1>(seD) == SET_OBJECT || get<1>(seD) == SET_PARTICLE) {

				auto pR = particle.objectPosition + Point(1, 0);
				auto pDR = particle.objectPosition + Point(1, 1);
				auto pL = particle.objectPosition + Point(-1, 0);
				auto pDL = particle.objectPosition + Point(-1, 1);

				if(this->isValidPosition(pDL) && this->isValidPosition(pDR)
				&& this->isValidPosition(pL) && this->isValidPosition(pR)
				&& get<1>(this->getSE(pDL)) == SET_EMPTY && get<1>(this->getSE(pDR)) == SET_EMPTY
				&& get<1>(this->getSE(pL)) == SET_EMPTY && get<1>(this->getSE(pR)) == SET_EMPTY)

					moveTo(rand() % 2 == 0 ? pL : pR);

				else if(this->isValidPosition(pDL) && this->isValidPosition(pL)
				     && get<1>(this->getSE(pDL)) == SET_EMPTY && get<1>(this->getSE(pL)) == SET_EMPTY)

					moveTo(pL);

				else if(this->isValidPosition(pDR) && this->isValidPosition(pR)
				     && get<1>(this->getSE(pDR)) == SET_EMPTY && get<1>(this->getSE(pR)) == SET_EMPTY)

					moveTo(pR);

			}
		}
	}

	this->filterParticles(this->particles);
}

bool Scene::start() {

	this->draw();

	unsigned int currTime = 0;
	bool shouldCreate = false;
	unsigned int minTime = this->usecUpdate();

	while(1) {

		if(this->usecUpdate() + currTime <= this->usecCreate()) {
			minTime = this->usecUpdate();
			shouldCreate = (this->usecUpdate() + currTime == this->usecCreate());
		}
		else {
			minTime = currTime - this->usecUpdate();
			shouldCreate = true;
		}

		this_thread::sleep_for(chrono::microseconds(minTime));

		if(shouldCreate)
			currTime = minTime != 0 ? currTime + this->usecUpdate() - this->usecCreate() : 0;
		else
			currTime += minTime;

		this->sceneMutex.lock();

		if(shouldCreate) {

			switch(this->sceneSettings->sceneMode) {
				case SM_EXACT:
					this->addParticle(this->sceneSettings->posUnion.creationPoint, true);
					break;
				case SM_RANDOM:
					break;
			}
		}

		if (!shouldCreate || currTime == 0) {
			this->step();
		}

		this->sceneMutex.unlock();
	}
}

bool Scene::addObject(Object object) {

	auto& se = this->getSE(object.objectPosition);

	if(get<1>(se) == SET_OBJECT) {

		return false;
	}
	else if(get<1>(se) == SET_PARTICLE) {

		this->particles[get<0>(se)].toBeRemoved = true;
		this->filterParticles(this->particles);
	}

	this->objects.push_back(object);

	se = SceneElement(this->objects.size() - 1, SET_OBJECT);

	this->drawSingle(object.objectPosition);

	return true;
}

bool Scene::removeObjectAt(Point& p) {

	auto& se = this->getSE(p);

	if(get<1>(se) != SET_OBJECT) {
		return false;
	}

	this->objects.erase(this->objects.begin() + get<0>(se));

	for(unsigned int location = 0; location < this->objects.size(); location++) {
		auto& objPos = this->objects[location].objectPosition;
		get<0>(this->getSE(objPos)) = location;
		get<1>(this->getSE(objPos)) = SET_OBJECT;
	}

	se = SceneElement(0, SET_EMPTY);

	this->drawSingle(p);

	return true;
}

bool Scene::addParticle(Point& p, bool active) {

	auto &se = this->getSE(p);

	if(get<1>(se) != SET_EMPTY && get<1>(se) != SET_PARTICLE) {
		return false;
	}

	Particle particle = Particle(p);
	this->particles.push_back(particle);

	se = SceneElement(this->particles.size() - 1, SET_PARTICLE);

	this->drawSingle(p);

	return true;
}

void Scene::drawEmpty(Point& position) {

	Object emptyObject = Object(position, ' ');
	emptyObject.draw();

	return;
}

void Scene::drawSingle(Point &p) {

	SceneElement& se = this->getSE(p);

	SceneElementType set = get<1>(se);
	unsigned int loc = get<0>(se);

	if((*this->windowCursorPosition) == p)
		this->reverse(p);

	switch(set) {
		case SET_EMPTY:
			this->drawEmpty(p);
			break;

		case SET_OBJECT:
			this->objects[loc].draw();
			break;

		case SET_PARTICLE:
			this->particles[loc].draw();
			break;
	}

	if((*this->windowCursorPosition) == p)
		this->restore(p);

}

void Scene::draw() {

	MatrixFunction f = [this](SceneElement& se, Point& position) {

		this->drawSingle(position);
		return;
	};

	this->forEachElement(f);
}

void Scene::filterParticles(vector<Particle>& v) {

	v.erase(remove_if(v.begin(),
			  v.end(),
			  [](Particle& p){ return p.toBeRemoved; }),
			  v.end());

	for(unsigned int location = 0; location < v.size(); location++) {
		auto& particle = v[location];
		auto& se = this->getSE(particle.objectPosition);
		get<0>(se) = location;
	}
}

Scene::~Scene() {

}

// ******* static *******

void Scene::setExact(SceneSettings& sceneSettings, Point creationPoint) {

	sceneSettings.sceneMode = SM_EXACT;
	sceneSettings.posUnion.creationPoint = creationPoint;
}

void Scene::setRandom(SceneSettings& sceneSettings, Rect region) {

	sceneSettings.sceneMode = SM_RANDOM;
	sceneSettings.posUnion.region = region;
}

void Scene::setTime(SceneSettings& sceneSettings, unsigned int usecCreate, unsigned int usecUpdate) {

	sceneSettings.usecCreate = usecCreate;
	sceneSettings.usecUpdate = usecUpdate;
}

