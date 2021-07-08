#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

ABSTRACT_DECLARATION(idClass, idEntity)
END_CLASS

void idGameEdit::ParseSpawnArgsToRenderEntity(const idDict* args, renderEntity_t* renderEntity) {
	memset(renderEntity, 0, sizeof(*renderEntity));

	auto temp = args->GetString("model");

	if (!temp.empty()) {
		const std::shared_ptr<idDeclModelDef> modelDef = std::dynamic_pointer_cast<idDeclModelDef>(declManager->FindType(declType_t::DECL_MODELDEF, temp, false));
		if (modelDef) {
			renderEntity->hModel = modelDef->ModelHandle().lock();
		}
		if (!renderEntity->hModel) {
			renderEntity->hModel = renderModelManager->FindModel(temp);
		}
	}
	/*if (renderEntity->hModel) {
		renderEntity->bounds = renderEntity->hModel->Bounds(renderEntity);
	}
	else {
		renderEntity->bounds.Zero();
	}*/

	args->GetVector("origin", "0 0", renderEntity->origin);
	args->GetVector("axis", "0 0", renderEntity->axis);
}

idEntity::idEntity() :
	originDelta(vec2_origin),
	axisDelta(vec2_origin) {
	entityNumber = ENTITYNUM_NONE;
	//entityDefNumber = -1;

	thinkFlags = 0;

	physics = nullptr;

	modelDefHandle = -1;
}

idEntity::~idEntity() {
	if (thinkFlags) {
		BecomeInactive(thinkFlags);
	}
	activeNode.Remove();
}

void idEntity::Spawn() {
	std::string temp;
	Vector2 origin;
	Vector2 axis;

	spawnNode.SetOwner(shared_from_this());
	activeNode.SetOwner(shared_from_this());

	gameLocal.RegisterEntity(shared_from_this(), -1, gameLocal.GetSpawnArgs());

	// parse static models the same way the editor display does
	gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &renderEntity);

	origin = renderEntity.origin;
	axis = renderEntity.axis;

	// every object will have a unique name
	std::string def_name;
	sprintf(def_name, "%s_%d", GetClassname().c_str(), (entityNumber - MAX_CLIENTS + 1));
	temp = spawnArgs.GetString("name", def_name);
	SetName(temp);

	InitDefaultPhysics(origin, axis);

	SetOrigin(origin);
	SetAxis(axis);

	temp = spawnArgs.GetString("model");
	if (!temp.empty()) {
		SetModel(temp);

		renderEntity.color = static_cast<Screen::ConsoleColor>(spawnArgs.GetInt("color", 15));
	}
}

void idEntity::Remove() {
	// we have to set back the default physics object before unbinding because the entity
	// specific physics object might be an entity variable and as such could already be destroyed.
	SetPhysics(nullptr);

	defaultPhysicsObj->SetClipModel(nullptr, 1.0);
	defaultPhysicsObj = nullptr;

	FreeModelDef();
	gameLocal.UnregisterEntity(shared_from_this());
}

/*
================
idEntity::SetName
================
*/
void idEntity::SetName(const std::string newname) {
	name = newname;
}

/*
================
idEntity::GetName
================
*/
const std::string& idEntity::GetName() const {
	return name;
}

void idEntity::Think() {
	RunPhysics();
	Present();
}
/*
================
idEntity::IsActive
================
*/
bool idEntity::IsActive() const {
	return activeNode.InList();
}

/*
================
idEntity::BecomeActive
================
*/
void idEntity::BecomeActive(int flags) {
	int oldFlags = thinkFlags;
	thinkFlags |= flags;
	if (thinkFlags) {
		if (!IsActive()) {
			activeNode.AddToEnd(gameLocal.activeEntities);
		}
		else if (!oldFlags) {
			// we became inactive this frame, so we have to decrease the count of entities to deactivate
			gameLocal.numEntitiesToDeactivate--;
		}
	}
}

/*
================
idEntity::BecomeInactive
================
*/
void idEntity::BecomeInactive(int flags) {
	if (thinkFlags) {
		thinkFlags &= ~flags;
		if (!thinkFlags && IsActive()) {
			gameLocal.numEntitiesToDeactivate++;
		}
	}

	if ((flags & TH_PHYSICS))
	{
		BecomeActive(TH_UPDATEVISUALS);
	}
}

void idEntity::Present() {
	/*if (!gameLocal.isNewFrame) {
		return;
	}*/

	// don't present to the renderer if the entity hasn't changed
	if (!(thinkFlags & TH_UPDATEVISUALS)) {
		return;
	}
	BecomeInactive(TH_UPDATEVISUALS);

	// camera target for remote render views
	/*if (cameraTarget && gameLocal.InPlayerPVS(this)) {
		renderEntity.remoteRenderView = cameraTarget->GetRenderView();
	}*/

	// if set to invisible, skip
	if (!renderEntity.hModel || IsHidden()) {
		return;
	}

	// add to refresh list
	if (modelDefHandle == -1) {
		modelDefHandle = gameRenderWorld->AddEntityDef(&renderEntity);
	}
	else {
		gameRenderWorld->UpdateEntityDef(modelDefHandle, &renderEntity);
	}
}

renderEntity_t * idEntity::GetRenderEntity() {
	return &renderEntity;
}

void idEntity::SetModel(const std::string& modelname) {
	FreeModelDef();

	renderEntity.hModel = renderModelManager->FindModel(modelname);

	if (renderEntity.hModel) {
		//renderEntity.bounds = renderEntity.hModel->Bounds(&renderEntity);
	}
	else {
		renderEntity.bounds.Zero();
	}

	UpdateVisuals();
}

/*
================
idEntity::FreeModelDef
================
*/
void idEntity::FreeModelDef() {
	if (modelDefHandle != -1) {
		gameRenderWorld->FreeEntityDef(modelDefHandle);
		modelDefHandle = -1;
	}
}

/*
================
idEntity::IsHidden
================
*/
bool idEntity::IsHidden() const {
	return fl.hidden;
}

/*
================
idEntity::Hide
================
*/
void idEntity::Hide() {
	if (!IsHidden()) {
		fl.hidden = true;
		FreeModelDef();
		UpdateVisuals();
	}
}

void idEntity::UpdateVisuals() {
	UpdateModel();
}

void idEntity::UpdateModel() {
	UpdateModelTransform();

	// ensure that we call Present this frame
	BecomeActive(TH_UPDATEVISUALS);
}

void idEntity::UpdateModelTransform() {
	Vector2 origin;
	Vector2 axis;

	if (GetPhysicsToVisualTransform(origin, axis)) {
		renderEntity.axis = axis * GetPhysics()->GetAxis();
		renderEntity.origin = GetPhysics()->GetOrigin() + origin * renderEntity.axis;
	}
	else {
		// Add the deltas here, used for projectiles in MP. These deltas should only affect the visuals.
		renderEntity.axis = GetPhysics()->GetAxis() * axisDelta;
		renderEntity.origin = GetPhysics()->GetOrigin() + originDelta;
	}
}

void idEntity::SetColor(const Screen::ConsoleColor & color) {
	renderEntity.color = color;

	UpdateVisuals();
}

/*
================
idEntity::UpdateAnimationControllers
================
*/
bool idEntity::UpdateAnimationControllers() {
	// any ragdoll and IK animation controllers should be updated here
	return false;
}

void idEntity::SetPhysics(std::shared_ptr<idPhysics> phys) {
	// clear any contacts the current physics object has
	if (physics) {
		physics->ClearContacts();
	}
	// set new physics object or set the default physics if NULL
	if (phys) {
		defaultPhysicsObj->SetClipModel(nullptr, 1.0f);
		physics = phys;
		physics->Activate();
	}
	else {
		physics = defaultPhysicsObj;
	}
	physics->UpdateTime(gameLocal.time);
	//physics->SetMaster(bindMaster, fl.bindOrientated);
}

std::shared_ptr<idPhysics> idEntity::GetPhysics() const {
	return physics;
}

void idEntity::RestorePhysics(std::shared_ptr<idPhysics> phys) {
	physics = phys;
}

bool idEntity::RunPhysics() {
	idEntity *part = nullptr;
	bool moved;

	// don't run physics if not enabled
	if (!(thinkFlags & TH_PHYSICS)) {
		// however do update any animation controllers
		if (UpdateAnimationControllers()) {
			BecomeActive(TH_ANIMATE);
		}
		return false;
	}

	const int startTime = gameLocal.previousTime;
	const int endTime = gameLocal.time;

	part = this;

	// save the physics state of the whole team and disable the team for collision detection
	if (part->physics) {
		part->physics->SaveState();
	}

	// move the whole team
	if (part->physics) {
		// run physics
		moved = part->physics->Evaluate(GetPhysicsTimeStep(), endTime);

		// if moved or forced to update the visual position and orientation from the physics
		if (moved) {
			part->UpdateFromPhysics(false);
		}
	}

	return true;
}

void idEntity::SetOrigin(const Vector2 & org) {
	GetPhysics()->SetOrigin(org);

	UpdateVisuals();
}

void idEntity::SetAxis(const Vector2 & axis) {
	/*if (GetPhysics()->IsType(Physics_Actor::Type)) {
		static_cast<Actor *>(this)->viewAxis = axis;
	}
	else {*/
		GetPhysics()->SetAxis(axis);
	//}

	UpdateVisuals();
}

bool idEntity::GetPhysicsToVisualTransform(Vector2 & origin, Vector2 & axis) {
	return false;
}

bool idEntity::Collide(const trace_t& collision, const Vector2& velocity) {
	// this entity collides with collision.c.entityNum
	return false;
}

void idEntity::ActivatePhysics(std::shared_ptr<idEntity> ent) {
	GetPhysics()->Activate();
}

void idEntity::AddContactEntity(std::shared_ptr<idEntity> ent) {
	GetPhysics()->AddContactEntity(ent);
}

void idEntity::RemoveContactEntity(std::shared_ptr<idEntity> ent) {
	GetPhysics()->RemoveContactEntity(ent);
}

/*
============
Damage

this		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: this=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback in global space
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted

inflictor, attacker, dir, and point can be NULL for environmental effects

============
*/
void idEntity::Damage(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker,
	const Vector2& dir, const std::string& damageDefName, const float damageScale) {
	
	if (!fl.takedamage) {
		return;
	}

	if (!inflictor) {
		inflictor = gameLocal.world;
	}

	if (!attacker) {
		attacker = gameLocal.world;
	}

	const idDict* damageDef = gameLocal.FindEntityDefDict(damageDefName);
	if (damageDef == NULL) {
		gameLocal.Error("Unknown damageDef '%s'\n", damageDefName.c_str());
		return;
	}

	int	damage = damageDef->GetInt("damage");

	// inform the attacker that they hit someone
	//attacker->DamageFeedback(this, inflictor, damage);
	if (damage) {
		// do the damage
		health -= damage;
		if (health <= 0) {
			if (health < -999) {
				health = -999;
			}

			Killed(inflictor, attacker, damage, dir);
		}
		else {
			//Pain(inflictor, attacker, damage, dir, location);
		}
	}
}

/*
============
idEntity::Killed

Called whenever an entity's health is reduced to 0 or less.
This is a virtual function that subclasses are expected to implement.
============
*/
void idEntity::Killed(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker, int damage,
	const Vector2& dir) {
}

void idEntity::InitDefaultPhysics(const Vector2 & origin, const Vector2 & axis) {
	std::string temp;
	std::shared_ptr<idClipModel> clipModel;

	// check if a clipmodel key/value pair is set
	if (spawnArgs.GetString("clipmodel", "", &temp)) {
		if (idClipModel::CheckModel(temp)) {
			clipModel = std::make_shared<idClipModel>(temp);
		}
	}

	if (!spawnArgs.GetBool("noclipmodel", "0")) {

		// check if mins/maxs or size key/value pairs are set
		if (!clipModel) {
			Vector2 size;
			idBounds bounds;
			bool setClipModel = false;

			if (spawnArgs.GetVector("mins", "", bounds[0]) &&
				spawnArgs.GetVector("maxs", "", bounds[1])) {
				setClipModel = true;
				if (bounds[0][0] > bounds[1][0] || bounds[0][1] > bounds[1][1] /* || bounds[0][2] > bounds[1][2]*/) {
					gameLocal.Error("Invalid bounds '%s'-'%s' on entity '%s'", bounds[0].ToString(), bounds[1].ToString(), name.c_str());
				}
			}
			else if (spawnArgs.GetVector("size", "", size)) {
				if ((size.x < 0.0f) || (size.y < 0.0f)) {
					gameLocal.Error("Invalid size '%s' on entity '%s'", size.ToString(), name.c_str());
				}
				bounds[0].Set(size.x * -0.5f, size.y * -0.5f);
				bounds[1].Set(size.x * 0.5f, size.y * 0.5f);
				setClipModel = true;
			}

			if (setClipModel) {
				idTraceModel trm;

				trm.SetupBox(bounds);

				clipModel = std::make_shared<idClipModel>(trm);
			}
		}

		// check if the visual model can be used as collision model
		if (!clipModel) {
			temp = spawnArgs.GetString("model");
			if (!temp.empty()) {
				if (idClipModel::CheckModel(temp)) {
					clipModel = std::make_shared<idClipModel>(temp);
				}
			}
		}
	}

#ifdef DEBUG
	defaultPhysicsObj = std::shared_ptr<idPhysics_Static>(DBG_NEW idPhysics_Static);
#else
	defaultPhysicsObj = std::make_shared<idPhysics_Static>();
#endif
	defaultPhysicsObj->SetSelf(shared_from_this());
	defaultPhysicsObj->SetClipModel(clipModel, 1.0f);
	defaultPhysicsObj->SetOrigin(origin);
	defaultPhysicsObj->SetAxis(axis);

	physics = defaultPhysicsObj;
}

void idEntity::UpdateFromPhysics(bool moveBack) {
	/*if (IsType(Actor::Type)) {
		Actor *actor = static_cast<Actor*>(this);

		// set master delta angles for actors
		if (GetBindMaster()) {
			idAngles delta = actor->GetDeltaViewAngles();
			if (moveBack) {
				delta.yaw -= static_cast<idPhysics_Actor *>(physics)->GetMasterDeltaYaw();
			}
			else {
				delta.yaw += static_cast<idPhysics_Actor *>(physics)->GetMasterDeltaYaw();
			}
			actor->SetDeltaViewAngles(delta);
		}
	}*/

	UpdateVisuals();
}

int idEntity::GetPhysicsTimeStep() const {
	return gameLocal.time - gameLocal.previousTime;
}

CLASS_DECLARATION(idEntity, idAnimatedEntity)
END_CLASS

/*
================
idAnimatedEntity::idAnimatedEntity
================
*/
idAnimatedEntity::idAnimatedEntity() {
}

/*
================
idAnimatedEntity::~idAnimatedEntity
================
*/
idAnimatedEntity::~idAnimatedEntity() {
}

/*
================
idAnimatedEntity::Think
================
*/
void idAnimatedEntity::Think() {
	RunPhysics();
	//UpdateAnimation();
	Present();
}

/*
================
idAnimatedEntity::SetModel
================
*/
void idAnimatedEntity::SetModel(const std::string& modelname) {
	FreeModelDef();

	auto modelDef = std::dynamic_pointer_cast<idDeclModelDef>(declManager->FindType(declType_t::DECL_MODELDEF,
		modelname, false));
	if (!modelDef) {
		return;
	}

	std::shared_ptr<idRenderModel> renderModel = modelDef->ModelHandle().lock();
	if (!renderModel) {
		return;
	}

	renderEntity.hModel = renderModel;
	if (!renderEntity.hModel) {
		idEntity::SetModel(modelname);
		return;
	}

	UpdateVisuals();
}