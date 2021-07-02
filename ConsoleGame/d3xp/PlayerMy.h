#ifndef PLAYER_MY_H
#define PLAYER_MY_H

class PlayerMy : public idPlayer
{
public:
	CLASS_PROTOTYPE(PlayerMy);

	PlayerMy();
	virtual ~PlayerMy();

	void Spawn();
	void Think() override;

	void Init();
	void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	void SpawnFromSpawnSpot();
	void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);
	void SetClipModel();

	bool Collide(const trace_t& collision, const Vector2& velocity) override;
private:
	std::shared_ptr<Physics_PlayerMy> physicsObj; // player physics

	void EvaluateControls();
	void AdjustSpeed();
	void Move();
};

#endif