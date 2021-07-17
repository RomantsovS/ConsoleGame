#ifndef MISC_H
#define MISC_H

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
protected:
	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) override;
};

class idSimpleObject : public idAnimatedEntity
{
public:
	CLASS_PROTOTYPE(idSimpleObject);

	idSimpleObject();

	virtual ~idSimpleObject() override;

	void Spawn();
	void Think() override;

	void Remove() override;

	bool Collide(const trace_t& collision, const Vector2& velocity) override;
private:
	std::shared_ptr<idPhysics_RigidBody> physicsObj;
};

#endif