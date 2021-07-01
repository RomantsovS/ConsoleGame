#ifndef PHYSICS_PHYSICS_PLAYER_H_
#define PHYSICS_PHYSICS_PLAYER_H_

/*
===================================================================================

	Player physics

	Simulates the motion of a player through the environment. Input from the
	player is used to allow a certain degree of control over the motion.

===================================================================================
*/

class idPhysics_PlayerBase : public idPhysics_Actor {

public:
	CLASS_PROTOTYPE(idPhysics_PlayerBase);

	idPhysics_PlayerBase();

	// initialisation
	void SetPlayerInput(const usercmd_t& cmd, const Vector2& forwardVector);
	virtual void SetSpeed(const float newWalkSpeed, const float newCrouchSpeed);
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
	// properties
	float walkSpeed{};

	// player input
	usercmd_t command;

	// run-time variables
	int framemsec{};
	float frametime{};
	float playerSpeed{};
protected:
	void MovePlayer(int msec);

	float CmdScale(const usercmd_t& cmd) const;
	const usercmd_t& GetUserCmd() const { return command; }
	float GetFrameTime() { return frametime; }
	float GetPlayerSpeed() const { return playerSpeed; }
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
