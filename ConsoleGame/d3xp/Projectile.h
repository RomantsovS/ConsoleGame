#ifndef __GAME_PROJECTILE_H__
#define __GAME_PROJECTILE_H__

class idProjectile : public idEntity {
public:
	CLASS_PROTOTYPE(idProjectile);

	idProjectile();
	virtual ~idProjectile();

	void Spawn();

	void Create(std::shared_ptr<idEntity> owner, const Vector2& start, const Vector2& dir);
	virtual void Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity, const float timeSinceFire = 0.0f, const float launchPower = 1.0f, const float dmgPower = 1.0f);

	virtual void Think();

	virtual bool Collide(const trace_t& collision, const Vector2& velocity);
	virtual void Explode(const trace_t& collision, std::shared_ptr<idEntity> ignore);
protected:
	std::weak_ptr<idEntity>	owner;

	std::shared_ptr<idPhysics_RigidBody> physicsObj;

	enum class projectileState_t {
		// must update these in script/doom_defs.script if changed
		SPAWNED = 0,
		CREATED = 1,
		LAUNCHED = 2,
		FIZZLED = 3,
		EXPLODED = 4
	};

	projectileState_t state{ projectileState_t::SPAWNED };
private:
	void Event_Explode();

};

#endif