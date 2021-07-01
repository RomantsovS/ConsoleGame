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

	// initialisation
	const Vector2& PlayerGetOrigin() const;	// != GetOrigin
public:	// common physics interface
	bool Evaluate(int timeStepMSec, int endTimeMSec) override;

	bool IsAtRest() const override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2& newOrigin, int id = -1) override;
	//void SetAxis(const Vector2& newAxis, int id = -1) override;

	void Translate(const Vector2& translation, int id = -1) override;
	//void Rotate(const Vector2& rotation, int id = -1) override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) override;
	const Vector2& GetLinearVelocity(int id = 0) const override;
private:
	// player physics state
	playerMyPState_t current;
	playerMyPState_t saved;
private:
	float CmdScale(const usercmd_t& cmd) const;
	bool SlideMove(bool gravity, bool stepUp, bool stepDown, bool push);
	void CheckForCollisions(float timeStep);

	void Friction();
	void WalkMove();
	void MovePlayer(int msec);
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
