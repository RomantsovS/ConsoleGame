#include "Physics_Static.h"

Physics_Static::Physics_Static()
{
}

Physics_Static::~Physics_Static()
{
}

void Physics_Static::SetSelf(Entity * e)
{
	self = e;
}

bool Physics_Static::Evaluate(int timeStepMSec, int endTimeMSec)
{
	/*Vector2 masterOrigin, oldOrigin;
	Vector2 masterAxis, oldAxis;


	if (hasMaster) {
		oldOrigin = current.origin;
		oldAxis = current.axis;

		self->GetMasterPosition(masterOrigin, masterAxis);
		current.origin = masterOrigin + current.localOrigin * masterAxis;
		if (isOrientated) {
			current.axis = current.localAxis * masterAxis;
		}
		else {
			current.axis = current.localAxis;
		}
		if (clipModel) {
			clipModel->Link(gameLocal.clip, self, 0, current.origin, current.axis);
		}

		return (current.origin != oldOrigin || current.axis != oldAxis);
	}*/
	return false;
}

void Physics_Static::UpdateTime(int endTimeMSec)
{
}

int Physics_Static::GetTime() const
{
	return 0;
}

void Physics_Static::Activate()
{
}

void Physics_Static::SaveState()
{
}

void Physics_Static::RestoreState()
{
}

void Physics_Static::SetOrigin(const Vector2 & newOrigin, int id)
{
	/*Vector2 masterOrigin;
	Vector2 masterAxis;*/

	current.localOrigin = newOrigin;

	/*if (hasMaster) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.origin = masterOrigin + newOrigin * masterAxis;
	}
	else {*/
		current.origin = newOrigin;
	/*}

	if (clipModel) {
		clipModel->Link(gameLocal.clip, self, 0, current.origin, current.axis);
	}

	next = ConvertPStateToInterpolateState(current);
	previous = next;*/
}

void Physics_Static::SetAxis(const Vector2 & newAxis, int id)
{
	/*Vector2 masterOrigin;
	Vector2 masterAxis;*/

	current.localAxis = newAxis;

	/*if (hasMaster && isOrientated) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.axis = newAxis * masterAxis;
	}
	else {*/
		current.axis = newAxis;
	/*}

	if (clipModel) {
		clipModel->Link(gameLocal.clip, self, 0, current.origin, current.axis);
	}

	next = ConvertPStateToInterpolateState(current);
	previous = next;*/
}

void Physics_Static::Translate(const Vector2 & translation, int id)
{
	current.localOrigin += translation;
	current.origin += translation;

	/*if (clipModel) {
		clipModel->Link(gameLocal.clip, self, 0, current.origin, current.axis);
	}*/
}

void Physics_Static::Rotate(const Vector2 & rotation, int id)
{
	/*Vector2 masterOrigin;
	Vector2 masterAxis;*/

	//current.origin *= rotation;
	//current.axis *= rotation.ToMat3();
	current.origin = rotation;
	current.axis = rotation;

	/*if (hasMaster) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.localAxis *= rotation.ToMat3();
		current.localOrigin = (current.origin - masterOrigin) * masterAxis.Transpose();
	}
	else {*/
		current.localAxis = current.axis;
		current.localOrigin = current.origin;
	/*}

	if (clipModel) {
		clipModel->Link(gameLocal.clip, self, 0, current.origin, current.axis);
	}*/
}

const Vector2 & Physics_Static::GetOrigin(int id) const
{
	return current.origin;
}

const Vector2 & Physics_Static::GetAxis(int id) const
{
	return current.axis;
}
