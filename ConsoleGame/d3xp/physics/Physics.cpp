#include "Physics.h"

const float CONTACT_EPSILON = idMath::Sqrt(2.0f * (CM_POINT_SIZE / 2.0f) * (CM_POINT_SIZE / 2.0f)); //0.25f; // maximum contact seperation distance

ABSTRACT_DECLARATION(idClass, idPhysics)

idPhysics::~idPhysics()
{
}
