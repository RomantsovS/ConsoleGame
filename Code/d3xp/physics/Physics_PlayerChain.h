#ifndef PHYSICS_PHYSICS_PLAYER_CHAIN_H_
#define PHYSICS_PHYSICS_PLAYER_CHAIN_H_

/*
===================================================================================

	Player chain physics

	Simulates the motion of a player through the environment. Input from the
	player is used to allow a certain degree of control over the motion.

===================================================================================
*/

struct playerPState_t {
	Vector2 origin;
	Vector2 velocity;
	Vector2 localOrigin;
	//idVec3					pushVelocity;
	//float					stepUp;
	//int movementType;
	//int movementFlags;
	//int movementTime;

	playerPState_t() :
		origin(vec2_origin),
		velocity(vec2_origin),
		localOrigin(vec2_origin)
		/*pushVelocity(vec3_zero),
		stepUp(0.0f),
		movementType(0),
		movementFlags(0),
		movementTime(0)*/ {
	}
};

class Physics_PlayerChain : public idPhysics_PlayerBase {

public:
	CLASS_PROTOTYPE(Physics_PlayerChain);

	Physics_PlayerChain();
	~Physics_PlayerChain();
	Physics_PlayerChain(const Physics_PlayerChain&) = default;
	Physics_PlayerChain& operator=(const Physics_PlayerChain&) = default;
	Physics_PlayerChain(Physics_PlayerChain&&) = default;
	Physics_PlayerChain& operator=(Physics_PlayerChain&&) = default;

	// initialisation
	const Vector2& PlayerGetOrigin() const noexcept;	// != GetOrigin

	int AddBody(const std::shared_ptr<idAFBody>& body);	// returns body id
							// get body or constraint id
	int GetBodyId(const std::shared_ptr<idAFBody>& body) const;
	// retrieve body or constraint
	std::shared_ptr<idAFBody> GetBody(const std::string& bodyName) const noexcept;
	std::shared_ptr<idAFBody> GetBody(const int id) const;
	// delete body or constraint
	void DeleteBody(const int id);

	void UpdateClipModels();
public:	// common physics interface
	int GetNumClipModels() const noexcept override;

	const idBounds& GetBounds(int id = -1) const noexcept override;
	const idBounds& GetAbsBounds(int id = -1) const noexcept override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) noexcept override;

	bool IsAtRest() const noexcept override;

	void SaveState() noexcept override;
	void RestoreState() noexcept override;

	void SetOrigin(const Vector2& newOrigin, int id = -1) noexcept override;
	//void SetAxis(const Vector2& newAxis, int id = -1) override;

	void Translate(const Vector2& translation, int id = -1) noexcept override;
	//void Rotate(const Vector2& rotation, int id = -1) override;

	const Vector2& GetOrigin(int id = 0) const noexcept override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) noexcept override;
	const Vector2& GetLinearVelocity(int id = 0) const noexcept override;

	void DisableClip() noexcept override;
	void EnableClip() noexcept override;

	void UnlinkClip() noexcept override;
	void LinkClip() noexcept override;
private:
	std::vector<std::shared_ptr<idAFBody>> bodies; // all bodies
	std::vector<AFCollision_t> collisions; // collisions

	bool enableCollision; // if true collision detection is enabled
	bool selfCollision; // if true the self collision is allowed

	// player physics state
	playerPState_t current;
	playerPState_t saved;
private:
	void Evolve(float timeStep) noexcept;
	std::shared_ptr<idEntity> SetupCollisionForBody(idAFBody* body) const noexcept;
	bool CollisionImpulse(float timeStep, idAFBody* body, trace_t& collision) noexcept;
	bool ApplyCollisions(float timeStep) noexcept;
	void CheckForCollisions(float timeStep);
	void SwapStates() noexcept;
	void MoveEachBodiesToPrevOne();

	void Friction() noexcept;
	void WalkMove() noexcept;
	void MovePlayer(int msec) noexcept;
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
