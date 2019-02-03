#include "Entity.h"
#include "Game.h"

ABSTRACT_DECLARATION(Entity)

Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::Spawn()
{
	gameLocal.RegisterEntity(this, -1, gameLocal.GetSpawnArgs());

	// parse static models the same way the editor display does
	gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &renderEntity);
}

void Entity::Think()
{
}
