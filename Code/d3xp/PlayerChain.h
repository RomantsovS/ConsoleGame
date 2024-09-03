#ifndef PLAYER_CHAIN_H
#define PLAYER_CHAIN_H

class PlayerChain : public idPlayer {
public:
	CLASS_PROTOTYPE(PlayerChain);

	PlayerChain();
	virtual ~PlayerChain();
	PlayerChain(const PlayerChain&) = default;
	PlayerChain& operator=(const PlayerChain&) = default;
	PlayerChain(PlayerChain&&) = default;
	PlayerChain& operator=(PlayerChain&&) = default;

	void Spawn();
	void Think() override;
	void Present() override;

	void Init();

	bool Collide(const trace_t& collision, const Vector2& velocity) noexcept override;
protected:
	void SetModelForId(int id, const std::string& modelName);
private:
	std::shared_ptr<Physics_PlayerChain> physicsObj; // player physics
	
	std::vector<std::shared_ptr<idRenderModel>> modelHandles;
	std::vector<int> modelDefHandles;

	void BuildChain(const std::string& name, const Vector2& origin, float linkLength, int numLinks, const Vector2& dir);
	void AddModel(const idTraceModel& trm, const Vector2& origin, const int id, const float density);
	void AddModel(const Vector2& origin, const int id, const float density);

	void EvaluateControls() noexcept;
	void AdjustSpeed() noexcept;
	void Move();
};

#endif