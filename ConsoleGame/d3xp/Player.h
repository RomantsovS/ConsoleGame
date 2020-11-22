#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "Actor.h"
#include "../framework/UsercmdGen.h"
#include "physics/Physics_Player.h"

class idPlayer : public idActor
{
public:
	usercmd_t usercmd;
public:
	CLASS_PROTOTYPE(idPlayer);

	idPlayer();
	virtual ~idPlayer();

	void Spawn();

	void Init();
	virtual void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	virtual void SpawnFromSpawnSpot();
	virtual void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);
	void SetClipModel();	// spectator mode uses a different bbox size

	void HandleUserCmds(const usercmd_t& newcmd);

	bool IsLocallyControlled() const;
private:
};

#endif