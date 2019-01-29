#ifndef RENDER_CONSOLE_H
#define RENDER_CONSOLE_H

#include <memory>

#include "RenderSystem.h"
#include "Screen.h"

class RenderConsole : public RenderSystem
{
public:
	RenderConsole(size_t ht, size_t wd);
	
	~RenderConsole();

	virtual void addObject(std::shared_ptr<RenderEntity> object);

	virtual void init();

	virtual void update();

	virtual void clear();
private:
	std::vector<std::shared_ptr<RenderEntity>> objects;

	std::shared_ptr<Screen> screen;
};

#endif