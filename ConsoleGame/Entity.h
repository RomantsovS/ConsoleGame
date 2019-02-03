#ifndef ENTITY_H
#define ENTITY_H

#include "Class.h"
#include "RenderWorld.h"
#include "Physics.h"

class Entity : public idClass
{
public:
	ABSTRACT_PROTOTYPE(idEntity);

	Entity();

	virtual ~Entity();

	void Spawn();
	virtual void Think();

	renderEntity_s *getRenderEntity() { return &renderEntity; };
	//std::shared_ptr<Physics> getPhysics() const { return physics; };

	//void setPhysics(std::shared_ptr<Physics> phys) { physics = phys; };

	void setActive(bool param) { active = param; }

	bool isActive() const { return active; }

	Dict spawnArgs;				// key/value pairs used to spawn and initialize entity
private:
	renderEntity_s renderEntity;
	Physics *physics;

	bool active;
};

#endif