#ifndef PLAYER_CHAIN_ENTITY_H
#define PLAYER_CHAIN_ENTITY_H

#include "Player.h"

class PlayerChain : public idPlayer
{
public:
public:
	CLASS_PROTOTYPE(PlayerChain);

	PlayerChain();
	virtual ~PlayerChain();

	void Spawn();
	void Think() override;

	void Init();
	void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	void SpawnFromSpawnSpot();
	void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);

	virtual bool Collide(const trace_t& collision, const Vector2& velocity);
private:
	std::shared_ptr<idPhysics_Player> physicsObj;			// player physics

	void EvaluateControls();
	void AdjustSpeed();
	void Move();
};

#endif