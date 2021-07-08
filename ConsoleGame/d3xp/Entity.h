#ifndef ENTITY_H
#define ENTITY_H

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

	int						health = 0;					// FIXME: do all objects really need health?

	struct entityFlags_s {
		bool				notarget : 1;	// if true never attack or target this entity
		bool				noknockback : 1;	// if true no knockback from hits
		bool				takedamage : 1;	// if true this entity can be damaged
		bool				hidden : 1;	// if true this entity is not visible
		bool				bindOrientated : 1;	// if true both the master orientation is used for binding
		bool				solidForTeam : 1;	// if true this entity is considered solid when a physics team mate pushes entities
		bool				forcePhysicsUpdate : 1;	// if true always update from the physics whether the object moved or not
		bool				selected : 1;	// if true the entity is selected for editing
		bool				neverDormant : 1;	// if true the entity never goes dormant
		bool				isDormant : 1;	// if true the entity is dormant
		bool				hasAwakened : 1;	// before a monster has been awakened the first time, use full PVS for dormant instead of area-connected
		bool				networkSync : 1; // if true the entity is synchronized over the network
		bool				grabbed : 1;	// if true object is currently being grabbed
		bool				skipReplication : 1; // don't replicate this entity over the network.
	} fl = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 };

	ABSTRACT_PROTOTYPE(idEntity);

	idEntity();

	virtual ~idEntity();

	void Spawn();
	
	virtual void Remove();

	void SetName(const std::string name);
	const std::string& GetName() const;

	// thinking
	virtual void Think();
	bool IsActive() const;
	void BecomeActive(int flags);
	void BecomeInactive(int flags);

	// visuals
	virtual void Present();
	virtual renderEntity_t *GetRenderEntity();
	virtual void SetModel(const std::string& modelname);
	void UpdateVisuals();
	void UpdateModel();
	void UpdateModelTransform();
	virtual void SetColor(const Screen::ConsoleColor &color);
	virtual void FreeModelDef();
	virtual void Hide();
	bool IsHidden() const;

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

	// damage
	// applies damage to this entity
	virtual	void Damage(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker,
		const Vector2& dir, const std::string& damageDefName, const float damageScale);
	// notifies this entity that is has been killed
	virtual void Killed(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker, int damage,
		const Vector2& dir);

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

class idAnimatedEntity : public idEntity {
public:
	CLASS_PROTOTYPE(idAnimatedEntity);

	idAnimatedEntity();
	~idAnimatedEntity();

	void Think() override;

	void SetModel(const std::string& modelname) override;
};

#endif