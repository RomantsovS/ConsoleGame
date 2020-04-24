#include "Physics_Base.h"
#include "../Game_local.h"

CLASS_DECLARATION(idPhysics, idPhysics_Base)

idPhysics_Base::idPhysics_Base() {
}

idPhysics_Base::~idPhysics_Base() {
}

void idPhysics_Base::SetSelf(std::shared_ptr<idEntity> e) {
	self = e;
}

std::shared_ptr<idClipModel> idPhysics_Base::GetClipModel(int id) const
{
	return nullptr;
}

int idPhysics_Base::GetNumClipModels() const
{
	return 0;
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
	return vec2_origin;
}

void idPhysics_Base::ClearContacts()
{
	for (auto iter = contacts.begin(); iter != contacts.end(); ++iter) {
		auto ent = gameLocal.entities[iter->entityNum];
		if (ent) {
			ent->RemoveContactEntity(self);
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
		if (ent && ent != self) {
			ent->AddContactEntity(self);
		}
	}
}

void idPhysics_Base::ActivateContactEntities()
{
	for (auto iter = contactEntities.begin(); iter != contactEntities.end(); ++iter) {
		auto ent = *iter;
		if (ent) {
			ent->ActivatePhysics(self);
		}
		else {
			iter = contactEntities.erase(iter);;
		}
	}
}
