#ifndef PHYSICS_PHYSICS_PLAYER_MY_H_
#define PHYSICS_PHYSICS_PLAYER_MY_H_

struct playerMyPState_t {
	Vector2 origin;
	Vector2 velocity;
	Vector2 localOrigin;
	//idVec3					pushVelocity;
	//float					stepUp;
	//int movementType;
	//int movementFlags;
	//int movementTime;

	playerMyPState_t() :
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

class Physics_PlayerMy : public idPhysics_PlayerBase {

public:
	CLASS_PROTOTYPE(Physics_PlayerMy);

	Physics_PlayerMy();
	~Physics_PlayerMy();
	Physics_PlayerMy(const Physics_PlayerMy&) = default;
	Physics_PlayerMy& operator=(const Physics_PlayerMy&) = default;
	Physics_PlayerMy(Physics_PlayerMy&&) = default;
	Physics_PlayerMy& operator=(Physics_PlayerMy&&) = default;

	// initialisation
	const Vector2& PlayerGetOrigin() const noexcept;	// != GetOrigin
public:	// common physics interface
	bool Evaluate(int timeStepMSec, int endTimeMSec) noexcept override;

	bool IsAtRest() const noexcept override;

	void SaveState() noexcept override;
	void RestoreState() noexcept override;

	void SetOrigin(const Vector2& newOrigin, int id = -1) noexcept override;
	//void SetAxis(const Vector2& newAxis, int id = -1) override;

	void Translate(const Vector2& translation, int id = -1) noexcept override;
	//void Rotate(const Vector2& rotation, int id = -1) override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) noexcept override;
	const Vector2& GetLinearVelocity(int id = 0) const noexcept override;

	void WriteToSnapshot(idBitMsg& msg) const override;
	void ReadFromSnapshot(const idBitMsg& msg) override;
private:
	// player physics state
	playerMyPState_t current;
	playerMyPState_t saved;
private:
	float CmdScale(const usercmd_t& cmd) const noexcept;
	bool SlideMove(bool gravity, bool stepUp, bool stepDown, bool push);
	void CheckForCollisions(float timeStep);

	void Friction() noexcept;
	void WalkMove();
	void CheckGround();
	void MovePlayer(int msec);
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
