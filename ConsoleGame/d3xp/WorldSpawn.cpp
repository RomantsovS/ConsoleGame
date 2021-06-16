#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

/*
================
idWorldspawn

Every map should have exactly one worldspawn.
================
*/
CLASS_DECLARATION(idEntity, idWorldspawn)
EVENT(EV_Remove, idWorldspawn::Event_Remove)
END_CLASS

/*
================
idWorldspawn::Spawn
================
*/
void idWorldspawn::Spawn() {
	assert(!gameLocal.world);
	gameLocal.world = std::dynamic_pointer_cast<idWorldspawn>(shared_from_this());
}

/*
================
idWorldspawn::~idWorldspawn
================
*/
idWorldspawn::~idWorldspawn() {
	if (gameLocal.world.get() == this) {
		gameLocal.world = nullptr;
	}
}

/*
================
idWorldspawn::Event_Remove
================
*/
void idWorldspawn::Event_Remove() {
	gameLocal.Error("Tried to remove world");
}
