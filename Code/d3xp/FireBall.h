#ifndef __GAME_FIREBALL_H__
#define __GAME_FIREBALL_H__

class FireBall : public idProjectile {
public:
	CLASS_PROTOTYPE(FireBall);

	void Spawn();

	virtual void Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity,
		const float timeSinceFire = 0.0f, const float launchPower = 1.0f, const float dmgPower = 1.0f) override;

	void Think() override;

	void Explode(const trace_t& collision, idEntity* ignore) override;

	projectileState_t state{ projectileState_t::SPAWNED };
private:
};

#endif