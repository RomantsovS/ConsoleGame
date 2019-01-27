#include "RenderSystem.h"



RenderSystem::RenderSystem()
{
}


RenderSystem::~RenderSystem()
{
}

void RenderSystem::addObject(std::shared_ptr<RenderEntity> object)
{
	objects.push_back(object);
}
