#ifndef MISC_H
#define MISC_H

class idStaticEntity : public idEntity {
public:
	CLASS_PROTOTYPE(idStaticEntity);

	idStaticEntity();
	virtual ~idStaticEntity() override;
	idStaticEntity(const idStaticEntity&) = default;
	idStaticEntity& operator=(const idStaticEntity&) = default;
	idStaticEntity(idStaticEntity&&) = default;
	idStaticEntity& operator=(idStaticEntity&&) = default;

	void Spawn();
	void Think() override;

	void WriteToSnapshot(idBitMsg& msg) const override;
	void ReadFromSnapshot(const idBitMsg& msg) override;
private:
	//int spawnTime;
protected:
	void Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) noexcept override;
};

class idSimpleObject : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE(idSimpleObject);

	idSimpleObject();
	virtual ~idSimpleObject() override;
	idSimpleObject(const idSimpleObject&) = default;
	idSimpleObject& operator=(const idSimpleObject&) = default;
	idSimpleObject(idSimpleObject&&) = default;
	idSimpleObject& operator=(idSimpleObject&&) = default;

	void Spawn();
	void Think() override;

	void Remove() noexcept override;

	bool Collide(const trace_t& collision, const Vector2& velocity) noexcept override;
private:
	std::shared_ptr<idPhysics_RigidBody> physicsObj;
};

#endif