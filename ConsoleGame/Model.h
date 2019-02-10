#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include <list>

#include "Screen.h"
#include "Vector2.h"

class ModelPixel
{
public:
	ModelPixel(Vector2 pos, Screen::Pixel pixel);

	Vector2 origin;

	Screen::Pixel screenPixel;
};

class RenderModel
{
public:	
	virtual ~RenderModel();

	virtual const std::list<ModelPixel> &GetJoints() const = 0;

	virtual void SetColor(Screen::ConsoleColor col) = 0;
};

#endif