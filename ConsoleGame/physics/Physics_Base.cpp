#include "Physics_Base.h"

CLASS_DECLARATION(idPhysics, idPhysics_Base)

idPhysics_Base::idPhysics_Base() {
}

idPhysics_Base::~idPhysics_Base() {
}

void idPhysics_Base::SetSelf(std::shared_ptr<idEntity> e) {
	self = e;
}

bool idPhysics_Base::Evaluate(int timeStepMSec, int endTimeMSec) {
	return false;
}

void idPhysics_Base::UpdateTime(int endTimeMSec) {
}

int idPhysics_Base::GetTime() const {
	return 0;
}

void idPhysics_Base::Activate() {
}

void idPhysics_Base::SaveState() {
}

void idPhysics_Base::RestoreState() {
}

void idPhysics_Base::SetOrigin(const Vector2 & newOrigin, int id) {
}

void idPhysics_Base::SetAxis(const Vector2 & newAxis, int id) {
}

void idPhysics_Base::Translate(const Vector2 & translation, int id) {
}

void idPhysics_Base::Rotate(const Vector2 & rotation, int id) {
}

const Vector2 & idPhysics_Base::GetOrigin(int id) const {
	return vec2_origin;
}

const Vector2 & idPhysics_Base::GetAxis(int id) const {
	return Vector2();
}
