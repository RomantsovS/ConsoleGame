#ifndef AI_H
#define AI_H

class idAI : public idActor
{
public:
	CLASS_PROTOTYPE(idAI);

	idAI();

	virtual ~idAI() override;

	void Spawn();
	virtual void Think() override;
protected:
	void Hide() override;

	// damage
	//virtual bool Pain(idEntity* inflictor, idEntity* attacker, int damage, const idVec3& dir, int location);
	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) override;
};

class AISimple : public idAI
{
public:
	CLASS_PROTOTYPE(AISimple);

	AISimple();

	virtual ~AISimple() override;

	void Spawn();
	virtual void Think() override;
	
	void Remove() override;

	bool Collide(const trace_t& collision, const Vector2& velocity) override;
private:
	std::shared_ptr<idPhysics_RigidBody> physicsObj;
protected:
	void Hide() override;

	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) override;
};

#endif