#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

#include "Model.h"
class RenderModelStatic : public RenderModel
{
public:
	RenderModelStatic();
	virtual ~RenderModelStatic();

	virtual const std::list<ModelPixel> &GetJoints() const;
private:
	std::list<ModelPixel> modelPixels;
};

#endif