#ifndef PLAYER_CHAIN_H
#define PLAYER_CHAIN_H

class PlayerChain : public idPlayer
{
public:
	CLASS_PROTOTYPE(PlayerChain);

	PlayerChain();
	virtual ~PlayerChain();

	void Spawn();
	void Think() override;
	void Present() override;

	void Init();
	void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	void SpawnFromSpawnSpot();
	void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);

	virtual bool Collide(const trace_t& collision, const Vector2& velocity);
protected:
	void SetModelForId(int id, const std::string& modelName);
private:
	std::shared_ptr<Physics_PlayerChain> physicsObj; // player physics
	
	std::vector<std::shared_ptr<idRenderModel>> modelHandles;
	std::vector<int> modelDefHandles;

	void BuildChain(const std::string& name, const Vector2& origin, float linkLength, int numLinks, const Vector2& dir);
	void AddModel(const idTraceModel& trm, const Vector2& origin, const int id, const float density);
	void AddModel(const Vector2& origin, const int id, const float density);

	void EvaluateControls();
	void AdjustSpeed();
	void Move();
};

#endif