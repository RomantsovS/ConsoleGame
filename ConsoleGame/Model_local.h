#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

#include "Model.h"

class RenderModelStatic : public RenderModel
{
public:
	RenderModelStatic();
	virtual ~RenderModelStatic();

	virtual const std::list<ModelPixel> &GetJoints() const override;

	virtual void SetColor(Screen::ConsoleColor col) override;
private:
	std::list<ModelPixel> modelPixels;
};

#endif