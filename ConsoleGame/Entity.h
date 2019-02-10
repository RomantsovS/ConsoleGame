#ifndef ENTITY_H
#define ENTITY_H

#include "Class.h"
#include "RenderWorld.h"
#include "Dict.h"
#include "Physics_Static.h"

class Entity : public idClass
{
public:
	int entityNumber;			// index into the entity list
	int entityDefNumber;		// index into the entity def list

	ABSTRACT_PROTOTYPE(idEntity);

	Entity();

	virtual ~Entity();

	void Spawn();
	
	// thinking
	virtual void Think();

	// visuals
	virtual void Present();
	virtual renderEntity_s *GetRenderEntity();
	void UpdateVisuals();
	void UpdateModel();
	void UpdateModelTransform();
	virtual void SetColor(const Screen::ConsoleColor &color);

	// physics
	// set a new physics object to be used by this entity
	void SetPhysics(Physics *phys);
	// get the physics object used by this entity
	Physics * GetPhysics() const;
	// restore physics pointer for save games
	void RestorePhysics(Physics *phys);
	// run the physics for this entity
	bool RunPhysics();
	// retrieves the transformation going from the physics origin/axis to the visual origin/axis
	virtual bool GetPhysicsToVisualTransform(Vector2 &origin, Vector2 &axis);

	void setActive(bool param) { active = param; }

	bool isActive() const { return active; }

	Dict spawnArgs;				// key/value pairs used to spawn and initialize entity
protected:
	renderEntity_s renderEntity;
	int modelDefHandle;
private:
	Physics_Static defaultPhysicsObj;					// default physics object
	Physics *physics;

	// Delta values that are set when the server or client disagree on where the render model should be. If this happens,
	// they resolve it through DecayOriginAndAxisDelta()
	Vector2 originDelta;
	Vector2 axisDelta;

	bool active;
	
	// physics
	// initialize the default physics
	void InitDefaultPhysics(const Vector2 &origin, const Vector2 &axis);
	// update visual position from the physics
	void UpdateFromPhysics(bool moveBack);
	// get physics timestep
	virtual int GetPhysicsTimeStep() const;
};

#endif