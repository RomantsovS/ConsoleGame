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
};

#endif