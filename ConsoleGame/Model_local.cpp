#include "Model_local.h"

RenderModelStatic::RenderModelStatic()
{
	modelPixels.emplace_back(Vector2(), Screen::Pixel('*', Screen::ConsoleColor::White));
}

RenderModelStatic::~RenderModelStatic()
{
}

const std::list<ModelPixel>& RenderModelStatic::GetJoints() const
{
	return modelPixels;
}

void RenderModelStatic::SetColor(Screen::ConsoleColor col)
{
	for (auto iter = modelPixels.begin(); iter != modelPixels.end(); ++iter)
	{
		iter->screenPixel.color = col;
	}
}

