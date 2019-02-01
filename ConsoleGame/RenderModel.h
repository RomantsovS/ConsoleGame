#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include <list>

#include "Screen.h"
#include "Vector2.h"

class ModelPixel
{
	Vector2 pos;

	Screen::Pixel screenPixel;
};

class RenderModel
{
public:
	RenderModel();
	
	~RenderModel();

	std::list<ModelPixel> modelPixels;
};

#endif