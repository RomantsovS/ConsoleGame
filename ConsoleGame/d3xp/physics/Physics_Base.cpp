#include <precompiled.h>
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics, idPhysics_Base)
END_CLASS

idPhysics_Base::idPhysics_Base() {
	self.reset();
	clipMask = 0;
	ClearContacts();
}

idPhysics_Base::~idPhysics_Base() {
}

void idPhysics_Base::SetSelf(std::shared_ptr<idEntity> e) {
	self = e;
}

void idPhysics_Base::SetClipModel(std::shared_ptr<idClipModel> model, float density, int id, bool freeOld)
{
}

std::shared_ptr<idClipModel> idPhysics_Base::GetClipModel(int id) const
{
	return nullptr;
}

int idPhysics_Base::GetNumClipModels() const
{
	return 0;
}

/*
================
idPhysics_Base::SetClipMask
================
*/
void idPhysics_Base::SetClipMask(int mask, int id) {
	clipMask = mask;
}

/*
================
idPhysics_Base::GetClipMask
================
*/
int idPhysics_Base::GetClipMask(int id) const {
	return clipMask;
}

/*
================
idPhysics_Base::GetBounds
================
*/
const idBounds& idPhysics_Base::GetBounds(int id) const {
	return bounds_zero;
}

/*
================
idPhysics_Base::GetAbsBounds
================
*/
const idBounds& idPhysics_Base::GetAbsBounds(int id) const {
	return bounds_zero;
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

/*
================
idPhysics_Base::PutToRest
================
*/
void idPhysics_Base::PutToRest() {
}

bool idPhysics_Base::IsAtRest() const
{
	return true;
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
	return vec2_origin;
}

void idPhysics_Base::SetLinearVelocity(const Vector2& newLinearVelocity, int id)
{
}

/*
================
idPhysics_Base::GetLinearVelocity
================
*/
const Vector2& idPhysics_Base::GetLinearVelocity(int id) const {
	return vec2_origin;
}

/*
================
idPhysics_Base::DisableClip
================
*/
void idPhysics_Base::DisableClip() {
}

/*
================
idPhysics_Base::EnableClip
================
*/
void idPhysics_Base::EnableClip() {
}

/*
================
idPhysics_Base::UnlinkClip
================
*/
void idPhysics_Base::UnlinkClip() {
}

/*
================
idPhysics_Base::LinkClip
================
*/
void idPhysics_Base::LinkClip() {
}

/*
================
idPhysics_Base::EvaluateContacts
================
*/
bool idPhysics_Base::EvaluateContacts() {
	return false;
}

void idPhysics_Base::ClearContacts()
{
	for (auto iter = contacts.begin(); iter != contacts.end(); ++iter) {
		auto ent = gameLocal.entities[iter->entityNum];
		if (ent) {
			ent->RemoveContactEntity(self.lock());
		}
	}
	contacts.clear();
}

void idPhysics_Base::AddContactEntity(std::shared_ptr<idEntity> e)
{
	bool found = false;

	for (auto iter = contactEntities.begin(); iter != contactEntities.end(); ++iter) {
		auto ent = *iter;
		if (!ent) {
			iter = contactEntities.erase(iter);
		}
		if (ent == e) {
			found = true;
		}
	}
	if (!found) {
		contactEntities.push_back(e);
	}
}

void idPhysics_Base::RemoveContactEntity(std::shared_ptr<idEntity> e)
{
	for (auto iter = contactEntities.begin(); iter != contactEntities.end(); ++iter) {
		auto ent = *iter;
		if (!ent) {
			iter = contactEntities.erase(iter);
			continue;
		}
		if (ent == e) {
			iter = contactEntities.erase(iter);
			return;
		}
	}
}

void idPhysics_Base::AddContactEntitiesForContacts()
{
	for (size_t i = 0; i < contacts.size(); i++) {
		auto ent = gameLocal.entities[contacts[i].entityNum];
		if (ent && ent != self.lock()) {
			ent->AddContactEntity(self.lock());
		}
	}
}

void idPhysics_Base::ActivateContactEntities()
{
	for (auto iter = contactEntities.begin(); iter != contactEntities.end(); ++iter) {
		auto ent = *iter;
		if (ent) {
			ent->ActivatePhysics(self.lock().get());
		}
		else {
			iter = contactEntities.erase(iter);
		}
	}
}

bool idPhysics_Base::IsOutsideWorld() const
{
	if (!gameLocal.clip.GetWorldBounds().Expand(1.0f).IntersectsBounds(GetAbsBounds())) {
		return true;
	}
	return false;
}
