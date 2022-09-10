#ifndef AI_H
#define AI_H

class idAI : public idActor {
public:
	CLASS_PROTOTYPE(idAI);

	idAI();
	virtual ~idAI() override;
	idAI(const idAI&) = default;
	idAI& operator=(const idAI&) = default;
	idAI(idAI&&) = default;
	idAI& operator=(idAI&&) = default;

	void Spawn();
	void Think() noexcept override;
protected:
	std::shared_ptr<idPhysics_RigidBody> physicsObj;

	void Hide() override;

	// damage
	//virtual bool Pain(idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location);
	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) noexcept override;
};

class AISimple : public idAI {
public:
	CLASS_PROTOTYPE(AISimple);

	AISimple();
	virtual ~AISimple() override;
	AISimple(const AISimple&) = default;
	AISimple& operator=(const AISimple&) = default;
	AISimple(AISimple&&) = default;
	AISimple& operator=(AISimple&&) = default;

	void Spawn();
	void Think() noexcept override;
	
	void Remove() noexcept override;

	bool Collide(const trace_t& collision, const Vector2& velocity) noexcept override;

	void WriteToSnapshot(idBitMsg& msg) const override;
	void ReadFromSnapshot(const idBitMsg& msg) override;
private:
	int lastChangeDirection{};
	int directionChangePeriod = 0;
protected:
	void Hide() override;

	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) noexcept override;
};

#endif