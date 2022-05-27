#ifndef __GAME_PROJECTILE_H__
#define __GAME_PROJECTILE_H__

extern const idEventDef EV_Explode;

class idProjectile : public idEntity {
public:
	CLASS_PROTOTYPE(idProjectile);

	void Spawn();

	void Create(idEntity* owner, const Vector2& start, const Vector2& dir);
	virtual void Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity, const float timeSinceFire = 0.0f, const float launchPower = 1.0f, const float dmgPower = 1.0f);

	idEntity* GetOwner() const;

	void Think() override;

	bool Collide(const trace_t& collision, const Vector2& velocity) noexcept override;
	virtual void Explode(const trace_t& collision, idEntity* ignore);

	static void CacheProjectille(const std::string& objectname, idDeclEntityDef* declEntDef);

	void Damage(idEntity* inflictor, idEntity* attacker, const Vector2& dir, const std::string& damageDefName) override;
protected:
	idEntity* owner;

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

	void Event_Explode();
	void DoDamage(idEntity* ent, const Vector2& dir);
private:
	enum class PlayerTouchState {
		NONE,
		TOUCHED,
		UNTAUCHED
	};
	PlayerTouchState playerTouched = PlayerTouchState::NONE;
	size_t num_firebals = 4;

	idDict projectileDict{};
};

#endif