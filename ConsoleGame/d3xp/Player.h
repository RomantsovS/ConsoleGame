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
	void Think();

	void Init();
	void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	void SpawnFromSpawnSpot();
	void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);
	void SetClipModel();	// spectator mode uses a different bbox size

	virtual bool Collide(const trace_t& collision, const Vector2& velocity);

	void HandleUserCmds(const usercmd_t& newcmd);

	bool IsLocallyControlled() const;
private:
	std::shared_ptr<idPhysics_Player> physicsObj;			// player physics

	void EvaluateControls();
	void AdjustSpeed();
	void Move();
};

#endif