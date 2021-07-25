#ifndef PHYSICS_PHYSICS_AF_H_
#define PHYSICS_PHYSICS_AF_H_

class idAFBody;
class idPhysics_AF;

//===============================================================
//
//	idAFBody
//
//===============================================================

struct AFBodyPState_t {
	Vector2 worldOrigin; // position in world space
	//idMat3					worldAxis;					// axis at worldOrigin
	Vector2 spatialVelocity; // linear and rotational velocity of body
	//idVec6					externalForce;				// external force and torque applied to body
};


class idAFBody {

	friend class idPhysics_AF;
	friend class Physics_PlayerChain;

public:
	idAFBody();
	idAFBody(const std::string& name, std::shared_ptr<idClipModel> clipModel, float density);
	~idAFBody();
	idAFBody(const idAFBody&) = default;
	idAFBody& operator=(const idAFBody&) = default;
	idAFBody(idAFBody&&) = default;
	idAFBody& operator=(idAFBody&&) = default;

	void Init();
	void SetClipModel(std::shared_ptr<idClipModel> clipModel);
	std::shared_ptr<idClipModel> GetClipModel() const { return clipModel; }
private:
	// properties
	std::string name; // name of body
	std::shared_ptr<idAFBody> parent; // parent of this body
	std::vector<std::shared_ptr<idAFBody>> children; // children of this body
	std::shared_ptr<idClipModel> clipModel; // model used for collision detection
	int clipMask; // contents this body collides with

	// physics state
	AFBodyPState_t state[2];
	AFBodyPState_t* current; // current physics state
	AFBodyPState_t* next; // next physics state
	AFBodyPState_t saved; // saved physics state
	Vector2 atRestOrigin; // origin at rest

	struct bodyFlags_s {
		bool clipMaskSet : 1;	// true if this body has a clip mask set
		bool selfCollision : 1;	// true if this body can collide with other bodies of this AF
	} fl;
};

//===============================================================
//
//	idPhysics_AF
//
//===============================================================

struct AFPState_t {
	int atRest;						// >= 0 if articulated figure is at rest
	float noMoveTime;				// time the articulated figure is hardly moving
	float activateTime;				// time since last activation
	float lastTimeStep;				// last time step
	//idVec6					pushVelocity;			// velocity with which the af is pushed
};

struct AFCollision_t {
	trace_t trace;
	std::shared_ptr<idAFBody> body;
};


class idPhysics_AF : public idPhysics_Base {

public:
	CLASS_PROTOTYPE(idPhysics_AF);

	idPhysics_AF();
	~idPhysics_AF();
	idPhysics_AF(const idPhysics_AF&) = default;
	idPhysics_AF& operator=(const idPhysics_AF&) = default;
	idPhysics_AF(idPhysics_AF&&) = default;
	idPhysics_AF& operator=(idPhysics_AF&&) = default;

	// initialisation
	int AddBody(const std::shared_ptr<idAFBody>& body);	// returns body id
							// get body or constraint id
	int GetBodyId(const std::shared_ptr<idAFBody>& body) const;
	// retrieve body or constraint
	std::shared_ptr<idAFBody> GetBody(const std::string& bodyName) const;
	// delete body or constraint
	void DeleteBody(const int id);

	void UpdateClipModels();
public:	// common physics interface
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const override;
	int GetNumClipModels() const override;

	const idBounds& GetBounds(int id = -1) const override;
	const idBounds& GetAbsBounds(int id = -1) const override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) override;
	void UpdateTime(int endTimeMSec) override;
	int GetTime() const override;

	bool IsAtRest() const override;
	void Activate() override;
	void PutToRest() override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2& newOrigin, int id = -1) override;
	void SetAxis(const Vector2& newAxis, int id = -1) override;

	void Translate(const Vector2& translation, int id = -1) override;
	//void Rotate(const Vector2& rotation, int id = -1) override;

	const Vector2& GetOrigin(int id = 0) const override;
	//const Vector2& GetAxis(int id = 0) const override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) override;
	virtual const Vector2& GetLinearVelocity(int id = 0) const override;

	void DisableClip() override;
	void EnableClip() override;

	void UnlinkClip() override;
	void LinkClip() override;

	bool EvaluateContacts() override;
private:
	std::vector<std::shared_ptr<idAFBody>> bodies; // all bodies
	std::vector<AFCollision_t> collisions; // collisions

	bool enableCollision; // if true collision detection is enabled
	bool selfCollision; // if true the self collision is allowed
	bool comeToRest; // if true the figure can come to rest

	// physics state
	AFPState_t current;
	AFPState_t saved;

private:
	void Evolve(float timeStep);
	idEntity* SetupCollisionForBody(idAFBody* body) const;
	bool CollisionImpulse(float timeStep, idAFBody* body, trace_t& collision);
	bool ApplyCollisions(float timeStep);
	void CheckForCollisions(float timeStep);
	void SwapStates();
	bool TestIfAtRest(float timeStep);
	void Rest();
	void DebugDraw();
	void MoveEachBodiesToPrevOne();
};

#endif