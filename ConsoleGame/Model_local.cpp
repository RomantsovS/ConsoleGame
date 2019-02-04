#include "Model_local.h"

RenderModelStatic::RenderModelStatic()
{
}

RenderModelStatic::~RenderModelStatic()
{
}

const std::list<ModelPixel>& RenderModelStatic::GetJoints() const
{
	return modelPixels;
}

