#include "Model.h"

RenderModel::~RenderModel()
{
}

ModelPixel::ModelPixel(Vector2 origin, Screen::Pixel pixel)
{
	this->origin = origin;
	this->screenPixel = pixel;
}
