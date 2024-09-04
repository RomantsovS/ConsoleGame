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
	void SetPlayerInput(const usercmd_t& cmd, const Vector2& forwardVector) noexcept;
	virtual void SetSpeed(const float newWalkSpeed, const float newCrouchSpeed) noexcept;
	virtual const Vector2& PlayerGetOrigin() const noexcept;	// != GetOrigin
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
private:
	// properties
	float walkSpeed{};

	// run-time variables
	int framemsec{};
	float frametime{};
	float playerSpeed{};
protected:
	void MovePlayer(int msec) noexcept;

	float CmdScale(const usercmd_t& cmd) const noexcept;
	const usercmd_t& GetUserCmd() const noexcept { return command; }
	float GetFrameTime() noexcept { return frametime; }
	float GetPlayerSpeed() const noexcept { return playerSpeed; }

	// player input
	usercmd_t command;
};

#endif // !PHYSICS_PHYSICS_PLAYER_H_
