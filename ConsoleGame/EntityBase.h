#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H

#include <memory>

#include "Entity.h"

class EntityBase : public Entity
{
public:
	EntityBase();
	
	virtual ~EntityBase() = 0;

	std::shared_ptr<RenderEntity> getRenderEntity() const { return renderEntity; };
	std::shared_ptr<Physics> getPhysics() const { return physics; };

	void setRenderEntity(std::shared_ptr<RenderEntity> rend) { renderEntity = rend; };
	void setPhysics(std::shared_ptr<Physics> phys) { physics = phys; };

	virtual void init() override;
	virtual void update() override;

	void setActive(bool param) { active = param; }

	bool isActive() const { return active; }
private:
	std::shared_ptr<RenderEntity> renderEntity;
	std::shared_ptr<Physics> physics;

	bool active;
};

#endif