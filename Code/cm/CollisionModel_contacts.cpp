#include "idlib/precompiled.h"

#include "CollisionModel_local.h"

/*
==================
idCollisionModelManagerLocal::Contacts
==================
*/
int idCollisionModelManagerLocal::Contacts(
    contactInfo_t* contacts, const int maxContacts, const Vector2& start,
    const Vector2& dir, const float depth, const idTraceModel* trm,
    int contentMask, int model, const Vector2& modelOrigin) {
  trace_t results;
  Vector2 end;

  // same as Translation but instead of storing the first collision we store all
  // collisions as contacts
  idCollisionModelManagerLocal::getContacts = true;
  idCollisionModelManagerLocal::contacts = contacts;
  idCollisionModelManagerLocal::maxContacts = maxContacts;
  idCollisionModelManagerLocal::numContacts = 0;
  end = start + dir * depth;
  idCollisionModelManagerLocal::Translation(&results, start, end, trm,
                                            contentMask, model, modelOrigin);
  /*if (dir.SubVec3(1).LengthSqr() != 0.0f) {
          // FIXME: rotational contacts
  }*/
  idCollisionModelManagerLocal::getContacts = false;
  idCollisionModelManagerLocal::maxContacts = 0;

  return idCollisionModelManagerLocal::numContacts;
}