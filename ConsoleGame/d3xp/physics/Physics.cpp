#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "../Game_local.h"

const float CONTACT_EPSILON = idMath::Sqrt(2.0f * (CM_POINT_SIZE / 2.0f) * (CM_POINT_SIZE / 2.0f)); //0.25f; // maximum contact seperation distance

ABSTRACT_DECLARATION(idClass, idPhysics)
END_CLASS

idPhysics::~idPhysics() {
}
