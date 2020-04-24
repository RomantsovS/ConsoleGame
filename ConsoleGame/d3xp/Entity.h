#ifndef ENTITY_H
#define ENTITY_H

#include "../d3xp/gamesys/Class.h"
#include "../renderer/RenderWorld.h"
#include "../idlib/Dict.h"
#include "physics/Physics_Static.h"
#include "../idlib/containers/LinkList.h"
#include "../cm/CollisionModel.h"

// Think flags
enum {
	TH_ALL = -1,
	TH_THINK = 1,		// run think function each frame
	TH_PHYSICS = 2,		// run physics each frame
	TH_ANIMATE = 4,		// update animation each frame
	TH_UPDATEVISUALS = 8,		// update renderEntity
	TH_UPDATEPARTICLES = 16
};

class idEntity : public idClass, public std::enable_shared_from_this<idEntity>
{
public:
	int entityNumber;			// index into the entity list
	//int entityDefNumber;		// index into the entity def list

	idLinkList<idEntity> spawnNode; // for being linked into spawnedEntities list
	idLinkList<idEntity> activeNode; // for being linked into activeEntities list

	std::string name; // name of entity
	idDict spawnArgs; // key/value pairs used to spawn and initialize entity

	int thinkFlags; // TH_? flags

	ABSTRACT_PROTOTYPE(idEntity);

	idEntity();

	virtual ~idEntity();

	void Spawn();
	
	void SetName(const std::string name);
	const std::string GetName() const;

	// thinking
	virtual void Think();
	bool IsActive() const;
	void BecomeActive(int flags);
	void BecomeInactive(int flags);

	// visuals
	virtual void Present();
	virtual renderEntity_t *GetRenderEntity();
	virtual void SetModel(std::string modelname);
	void UpdateVisuals();
	void UpdateModel();
	void UpdateModelTransform();
	virtual void SetColor(const Screen::ConsoleColor &color);
	virtual void FreeModelDef();

	// animation
	virtual bool UpdateAnimationControllers();

	// physics
	// set a new physics object to be used by this entity
	void SetPhysics(std::shared_ptr<idPhysics> phys);
	// get the physics object used by this entity
	std::shared_ptr<idPhysics> GetPhysics() const;
	// restore physics pointer for save games
	void RestorePhysics(std::shared_ptr<idPhysics> phys);
	// run the physics for this entity
	bool RunPhysics();
	// set the origin of the physics object (relative to bindMaster if not NULL)
	void SetOrigin(const Vector2 &org);
	// set the axis of the physics object (relative to bindMaster if not NULL)
	void SetAxis(const Vector2 &axis);
	// retrieves the transformation going from the physics origin/axis to the visual origin/axis
	virtual bool GetPhysicsToVisualTransform(Vector2 &origin, Vector2 &axis);
	// called from the physics object when colliding, should return true if the physics simulation should stop
	virtual bool Collide(const trace_t& collision, const Vector2& velocity);
	// activate the physics object, 'ent' is the entity activating this entity
	virtual void ActivatePhysics(std::shared_ptr<idEntity> ent);
	// add a contact entity
	virtual void AddContactEntity(std::shared_ptr<idEntity> ent);
	// remove a touching entity
	virtual void RemoveContactEntity(std::shared_ptr<idEntity> ent);
protected:
	renderEntity_t renderEntity;
	int modelDefHandle;
private:
	std::shared_ptr<idPhysics_Static> defaultPhysicsObj; // default physics object
	std::shared_ptr<idPhysics> physics;

	// Delta values that are set when the server or client disagree on where the render model should be. If this happens,
	// they resolve it through DecayOriginAndAxisDelta()
	Vector2 originDelta;
	Vector2 axisDelta;
	
	// physics
	// initialize the default physics
	void InitDefaultPhysics(const Vector2 &origin, const Vector2 &axis);
	// update visual position from the physics
	void UpdateFromPhysics(bool moveBack);
	// get physics timestep
	virtual int GetPhysicsTimeStep() const;
};

#endif