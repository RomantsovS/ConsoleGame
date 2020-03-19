#include "Physics_RigidBody.h"
#include "../d3xp/Game_local.h"

CLASS_DECLARATION(idPhysics_Base, idPhysics_RigidBody)

idPhysics_RigidBody::idPhysics_RigidBody() {
}

idPhysics_RigidBody::~idPhysics_RigidBody() {
}

bool idPhysics_RigidBody::Evaluate(int timeStepMSec, int endTimeMSec)
{
	return false;
}

void idPhysics_RigidBody::UpdateTime(int endTimeMSec) {
}

int idPhysics_RigidBody::GetTime() const {
	return gameLocal.time;
}

/*
================
idPhysics_RigidBody::SaveState
================
*/
void idPhysics_RigidBody::SaveState() {
	saved = current;
}

/*
================
idPhysics_RigidBody::RestoreState
================
*/
void idPhysics_RigidBody::RestoreState() {
	current = saved;

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position, current.i.orientation);

	EvaluateContacts();
}

/*
================
idPhysics::SetOrigin
================
*/
void idPhysics_RigidBody::SetOrigin(const idVec3 &newOrigin, int id) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localOrigin = newOrigin;
	if (hasMaster) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.i.position = masterOrigin + newOrigin * masterAxis;
	}
	else {
		current.i.position = newOrigin;
	}

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position, clipModel->GetAxis());

	Activate();
}

/*
================
idPhysics::SetAxis
================
*/
void idPhysics_RigidBody::SetAxis(const idMat3 &newAxis, int id) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localAxis = newAxis;
	if (hasMaster && isOrientated) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.i.orientation = newAxis * masterAxis;
	}
	else {
		current.i.orientation = newAxis;
	}

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), clipModel->GetOrigin(), current.i.orientation);

	Activate();
}