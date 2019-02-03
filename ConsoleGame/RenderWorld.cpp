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

void RenderWorldLocal::AddEntity(const renderEntity_s *ent)
{
	RenderEntityLocal *rEnt = new RenderEntityLocal;
	rEnt->parms = *ent;

	entities.push_back(rEnt);
}

void RenderWorldLocal::RenderScene()
{
	tr.FillBorder();

	for (auto entIter = entities.cbegin(); entIter != entities.cend(); ++entIter)
	{
		tr.Draw((*entIter)->parms);
	}

	tr.Display();
}
