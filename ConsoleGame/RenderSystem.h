#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <vector>
#include <memory>

#include "RenderEntity.h"

class RenderSystem
{
public:
	RenderSystem();
	
	~RenderSystem();

	void addObject(std::shared_ptr<RenderEntity> object);

	void update();
private:
	std::vector<std::shared_ptr<RenderEntity>> objects;
};

#endif