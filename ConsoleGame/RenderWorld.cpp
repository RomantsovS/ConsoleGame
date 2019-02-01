#include "tr_local.h"

RenderWorld::RenderWorld()
{
}

RenderWorld::~RenderWorld()
{
}

RenderWorldLocal::RenderWorldLocal()
{
}

RenderWorldLocal::~RenderWorldLocal()
{
}

void RenderWorldLocal::addEntity(const renderEntity_s *ent)
{
	RenderEntityLocal *rEnt = new RenderEntityLocal;
	rEnt->parms = *ent;

	entities.push_back(rEnt);
}

void RenderWorldLocal::renderScene()
{
	tr.fillBorder();

	for (auto entIter = entities.cbegin(); entIter != entities.cend(); ++entIter)
	{
		tr.draw((*entIter)->parms);
	}
}
