#ifndef PHYSICS_PHYSICS_PLAYER_H_
#define PHYSICS_PHYSICS_PLAYER_H_

#include "Physics_Actor.h"
#include "../../framework/UsercmdGen.h"

/*
===================================================================================

	Player physics

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

class idPhysics_Player : public idPhysics_Actor {

public:
	CLASS_PROTOTYPE(idPhysics_Player);

	idPhysics_Player();

	// initialisation
	void SetSpeed(const float newWalkSpeed, const float newCrouchSpeed);
	void SetPlayerInput(const usercmd_t& cmd, const Vector2& forwardVector);
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
	playerPState_t current;
	playerPState_t saved;

	// properties
	float walkSpeed;

	// player input
	usercmd_t command;

	// run-time variables
	int framemsec;
	float frametime;
private:
	bool SlideMove();
	void Friction();
	void WalkMove();
	void MovePlayer(int msec);
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
