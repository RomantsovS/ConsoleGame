#include "Entity.h"
#include "Game_local.h"

ABSTRACT_DECLARATION(Entity)

Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::Spawn()
{
	Vector2 pos;

	gameLocal.RegisterEntity(this, -1, gameLocal.GetSpawnArgs());

	// parse static models the same way the editor display does
	gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &renderEntity);

	pos = renderEntity.pos;
}

void Entity::Think()
{
}
