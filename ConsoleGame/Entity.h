#ifndef ENTITY_H
#define ENTITY_H

#include "RenderWorld.h"
#include "Physics.h"

class Entity
{
public:
	Entity();

	virtual ~Entity();

	renderEntity_s *getRenderEntity() { return &renderEntity; };
	//std::shared_ptr<Physics> getPhysics() const { return physics; };

	//void setPhysics(std::shared_ptr<Physics> phys) { physics = phys; };

	virtual void init();
	virtual void think();

	void setActive(bool param) { active = param; }

	bool isActive() const { return active; }
private:
	renderEntity_s renderEntity;
	Physics *physics;

	bool active;
};

#endif