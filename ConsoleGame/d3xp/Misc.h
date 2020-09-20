#ifndef MISC_H
#define MISC_H

#include "Entity.h"
#include "physics/Physics_RigidBody.h"

class idStaticEntity : public idEntity
{
public:
	CLASS_PROTOTYPE(idStaticEntity);

	idStaticEntity();

	virtual ~idStaticEntity() override;

	void Spawn();
	virtual void Think() override;
private:
	//int spawnTime;
};

class idSimpleObject : public idEntity
{
public:
	CLASS_PROTOTYPE(idSimpleObject);

	idSimpleObject();

	virtual ~idSimpleObject() override;

	void Spawn();
	virtual void Think() override;

	virtual void Remove();

	virtual bool Collide(const trace_t& collision, const Vector2& velocity) override;
private:
	std::shared_ptr<idPhysics_RigidBody> physicsObj;
};

#endif