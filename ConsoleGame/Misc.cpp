#include "Misc.h"
#include "Game_local.h"

CLASS_DECLARATION(idEntity, idStaticEntity)

idStaticEntity::idStaticEntity()
{
	spawnTime = 0;
}

idStaticEntity::~idStaticEntity()
{
}

void idStaticEntity::Spawn()
{
	spawnTime = gameLocal.time;
}

void idStaticEntity::Think()
{
	idEntity::Think();
}