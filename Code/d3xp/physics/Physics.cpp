#include "idlib/precompiled.h"


#include "../Game_local.h"

const float CONTACT_EPSILON = 0.25f; // maximum contact seperation distance

ABSTRACT_DECLARATION(idClass, idPhysics)
END_CLASS

idPhysics::~idPhysics() {
}
