#ifndef RENDER_CONSOLE_H
#define RENDER_CONSOLE_H

#include "RenderSystem.h"
#include "Screen.h"

class RenderConsole : public RenderSystem
{
public:
	RenderConsole(size_t ht, size_t wd, size_t bordHt, size_t bordWd, Screen::Pixel bordPix = Screen::Pixel('#', Screen::ConsoleColor::White));
	
	~RenderConsole();

	virtual void init();

	virtual void update();

	virtual void clear();
private:
	RenderWorld *renderWorld;

	Screen screen;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;
};

#endif