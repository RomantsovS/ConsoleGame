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

int RenderWorldLocal::AddEntityDef(const renderEntity_s * re)
{
	// try and reuse a free spot
	int entityHandle;

	auto iter = std::find(entityDefs.begin(), entityDefs.end(), nullptr);

	if (iter == entityDefs.end())
	{
		entityHandle = entityDefs.size();
		entityDefs.resize(entityDefs.size() + 4);
	}
	else
		entityHandle = iter - entityDefs.begin();

	UpdateEntityDef(entityHandle, re);

	return entityHandle;
}

void RenderWorldLocal::UpdateEntityDef(int entityHandle, const renderEntity_s * re)
{
	while (entityHandle >= static_cast<int>(entityDefs.size()))
	{
		entityDefs.resize(entityDefs.size() + 4);
	}

	RenderEntityLocal *def = entityDefs[entityHandle];

	if (def)
	{

	}
	else
	{
		// creating a new one
		def = new RenderEntityLocal;
		entityDefs[entityHandle] = def;

		//def->world = this;
		def->index = entityHandle;
	}

	def->parms = *re;
}

void RenderWorldLocal::FreeEntityDef(int entityHandle)
{
	RenderEntityLocal *def;

	def = entityDefs[entityHandle];
	if (!def)
	{
		return;
	}

	//R_FreeEntityDefDerivedData(def, false, false);

	// if we are playing a demo, these will have been freed
	// in R_FreeEntityDefDerivedData(), otherwise the gui
	// object still exists in the game

	delete def;
	entityDefs[entityHandle] = NULL;
}

void RenderWorldLocal::RenderScene()
{
	tr.FillBorder();

	for (auto entIter = entityDefs.cbegin(); entIter != entityDefs.cend(); ++entIter)
	{
		if(*entIter)
			tr.Draw((*entIter)->parms);
	}

	tr.Display();
}
