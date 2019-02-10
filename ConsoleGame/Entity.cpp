#include "Entity.h"
#include "Game_local.h"

ABSTRACT_DECLARATION(idClass, Entity)

Entity::Entity()
{
	modelDefHandle = -1;
}

Entity::~Entity()
{
	// we have to set back the default physics object before unbinding because the entity
	// specific physics object might be an entity variable and as such could already be destroyed.
	SetPhysics(NULL);

	gameLocal.UnregisterEntity(this);
}

void Entity::Spawn()
{
	Vector2 origin;
	Vector2 axis;

	gameLocal.RegisterEntity(this, -1, gameLocal.GetSpawnArgs());

	// parse static models the same way the editor display does
	gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &renderEntity);

	InitDefaultPhysics(origin, axis);

	origin = renderEntity.origin;
	axis = renderEntity.axis;
}

void Entity::Think()
{
	RunPhysics();
	Present();
}

void Entity::Present()
{
	/*if (!gameLocal.isNewFrame) {
		return;
	}

	// don't present to the renderer if the entity hasn't changed
	if (!(thinkFlags & TH_UPDATEVISUALS)) {
		return;
	}
	BecomeInactive(TH_UPDATEVISUALS);

	// camera target for remote render views
	if (cameraTarget && gameLocal.InPlayerPVS(this)) {
		renderEntity.remoteRenderView = cameraTarget->GetRenderView();
	}

	// if set to invisible, skip
	if (!renderEntity.hModel || IsHidden()) {
		return;
	}*/

	// add to refresh list
	if (modelDefHandle == -1) {
		modelDefHandle = gameRenderWorld->AddEntityDef(&renderEntity);
	}
	else {
		gameRenderWorld->UpdateEntityDef(modelDefHandle, &renderEntity);
	}
}

renderEntity_s * Entity::GetRenderEntity()
{
	return &renderEntity;
}

void Entity::UpdateVisuals()
{
	UpdateModel();
}

void Entity::UpdateModel()
{
	UpdateModelTransform();
}

void Entity::UpdateModelTransform()
{
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

void Entity::SetColor(const Screen::ConsoleColor & color)
{
	renderEntity.hModel->SetColor(color);

	UpdateVisuals();
}

void Entity::SetPhysics(Physics * phys)
{
	// clear any contacts the current physics object has
	/*if (physics) {
		physics->ClearContacts();
	}*/
	// set new physics object or set the default physics if NULL
	if (phys != NULL) {
		//defaultPhysicsObj.SetClipModel(NULL, 1.0f);
		physics = phys;
		physics->Activate();
	}
	/*else {
		physics = &defaultPhysicsObj;
	}*/
	physics->UpdateTime(gameLocal.time);
	//physics->SetMaster(bindMaster, fl.bindOrientated);
}

Physics * Entity::GetPhysics() const
{
	return physics;
}

void Entity::RestorePhysics(Physics * phys)
{
	physics = phys;
}

bool Entity::RunPhysics()
{
	Entity *part = nullptr;
	bool moved;

	const int startTime = gameLocal.previousTime;
	const int endTime = gameLocal.time;

	part = this;

	// save the physics state of the whole team and disable the team for collision detection
	if (part->physics)
	{
		part->physics->SaveState();
	}

	// move the whole team
	if (part->physics)
	{
		// run physics
		moved = part->physics->Evaluate(GetPhysicsTimeStep(), endTime);

		// if moved or forced to update the visual position and orientation from the physics
		if (moved)
		{
			part->UpdateFromPhysics(false);
		}
	}

	return false;
}

bool Entity::GetPhysicsToVisualTransform(Vector2 & origin, Vector2 & axis)
{
	return false;
}

void Entity::InitDefaultPhysics(const Vector2 & origin, const Vector2 & axis)
{
	/*const char *temp;
	idClipModel *clipModel = NULL;

	// check if a clipmodel key/value pair is set
	if (spawnArgs.GetString("clipmodel", "", &temp)) {
		if (idClipModel::CheckModel(temp)) {
			clipModel = new (TAG_PHYSICS_CLIP_ENTITY) idClipModel(temp);
		}
	}

	if (!spawnArgs.GetBool("noclipmodel", "0")) {

		// check if mins/maxs or size key/value pairs are set
		if (!clipModel) {
			idVec3 size;
			idBounds bounds;
			bool setClipModel = false;

			if (spawnArgs.GetVector("mins", NULL, bounds[0]) &&
				spawnArgs.GetVector("maxs", NULL, bounds[1])) {
				setClipModel = true;
				if (bounds[0][0] > bounds[1][0] || bounds[0][1] > bounds[1][1] || bounds[0][2] > bounds[1][2]) {
					gameLocal.Error("Invalid bounds '%s'-'%s' on entity '%s'", bounds[0].ToString(), bounds[1].ToString(), name.c_str());
				}
			}
			else if (spawnArgs.GetVector("size", NULL, size)) {
				if ((size.x < 0.0f) || (size.y < 0.0f) || (size.z < 0.0f)) {
					gameLocal.Error("Invalid size '%s' on entity '%s'", size.ToString(), name.c_str());
				}
				bounds[0].Set(size.x * -0.5f, size.y * -0.5f, 0.0f);
				bounds[1].Set(size.x * 0.5f, size.y * 0.5f, size.z);
				setClipModel = true;
			}

			if (setClipModel) {
				int numSides;
				idTraceModel trm;

				if (spawnArgs.GetInt("cylinder", "0", numSides) && numSides > 0) {
					trm.SetupCylinder(bounds, numSides < 3 ? 3 : numSides);
				}
				else if (spawnArgs.GetInt("cone", "0", numSides) && numSides > 0) {
					trm.SetupCone(bounds, numSides < 3 ? 3 : numSides);
				}
				else {
					trm.SetupBox(bounds);
				}
				clipModel = new (TAG_PHYSICS_CLIP_ENTITY) idClipModel(trm);
			}
		}

		// check if the visual model can be used as collision model
		if (!clipModel) {
			temp = spawnArgs.GetString("model");
			if ((temp != NULL) && (*temp != 0)) {
				if (idClipModel::CheckModel(temp)) {
					clipModel = new (TAG_PHYSICS_CLIP_ENTITY) idClipModel(temp);
				}
			}
		}
	}*/

	defaultPhysicsObj.SetSelf(this);
	//defaultPhysicsObj.SetClipModel(clipModel, 1.0f);
	defaultPhysicsObj.SetOrigin(origin);
	defaultPhysicsObj.SetAxis(axis);

	physics = &defaultPhysicsObj;
}

void Entity::UpdateFromPhysics(bool moveBack)
{
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

int Entity::GetPhysicsTimeStep() const
{
	return gameLocal.time - gameLocal.previousTime;
}
