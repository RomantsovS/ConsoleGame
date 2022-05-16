#include "idlib/precompiled.h"

#include "Game_local.h"

const idEventDef EV_Explode("<explode>", "");

CLASS_DECLARATION(idEntity, idProjectile)
	EVENT(EV_Explode, idProjectile::Event_Explode)
END_CLASS

/*
================
idProjectile::Spawn
================
*/
void idProjectile::Spawn() {
	physicsObj = std::make_shared<idPhysics_RigidBody>();
	physicsObj->SetSelf(this);
	physicsObj->SetClipModel(std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), 1.0f);
	physicsObj->SetContents(0);
	physicsObj->SetClipMask(0);
	SetPhysics(physicsObj);

	// get the projectile
	projectileDict.Clear();

	std::string projectileName = spawnArgs.GetString("def_projectile");
	if (!projectileName.empty()) {
		const std::shared_ptr<idDeclEntityDef>& projectileDef = gameLocal.FindEntityDef(projectileName, false);
		if (!projectileDef) {
			gameLocal.Warning("Unknown projectile '%s' in weapon '%s'", projectileName.c_str(), name.c_str());
		}
		else {
			const std::string& spawnclass = projectileDef->dict.GetString("spawnclass");
			idTypeInfo* cls = idClass::GetClass(spawnclass);
			if (!cls || !cls->IsType(idProjectile::Type)) {
				gameLocal.Warning("Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass.c_str(),
					projectileName.c_str(), name.c_str());
			}
			else {
				projectileDict = projectileDef->dict;
			}
		}
	}
}

/*
================
idProjectile::GetOwner
================
*/
idEntity* idProjectile::GetOwner() const {
	return owner;
}

/*
================
idProjectile::Create
================
*/
void idProjectile::Create(idEntity* owner, const Vector2& start, const Vector2& dir) {
	idDict args;

	physicsObj->SetOrigin(start);

	physicsObj->GetClipModel()->SetOwner(owner);

	this->owner = owner;

	UpdateVisuals();

	state = projectileState_t::CREATED;
}

/*
=================
idProjectile::Launch
=================
*/
void idProjectile::Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity, const float timeSinceFire, const float launchPower, const float dmgPower) {
	float			fuse{};
	Vector2			velocity;
	float			speed;
	//int				contents;

	spawnArgs.GetVector("velocity", "0 0 0", velocity);

	speed = velocity.Length() * launchPower;

	//damagePower = dmgPower;

	fuse = spawnArgs.GetFloat("fuse");

	Vector2 tmpDir = dir;
	tmpDir.Normalize();

	if (spawnArgs.GetString("can_touch_player").empty()) {
		physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_PROJECTILE));
	}
	else {
		physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_MONSTERCLIP));
	}
	physicsObj->SetClipMask(MASK_PLAYERSOLID);
	physicsObj->SetLinearVelocity(tmpDir * speed + pushVelocity);
	physicsObj->SetOrigin(start);

	if (fuse <= 0) {
		// run physics for 1 second
		RunPhysics();
		PostEventMS(&EV_Remove, spawnArgs.GetInt("remove_time", "1500"));
	}
	else if (spawnArgs.GetBool("detonate_on_fuse")) {
		fuse -= timeSinceFire;
		if (fuse < 0.0f) {
			fuse = 0.0f;
		}
		PostEventSec(&EV_Explode, fuse);
	}

	UpdateVisuals();

	state = projectileState_t::LAUNCHED;
}

/*
================
idProjectile::Think
================
*/
void idProjectile::Think() {
	if (!spawnArgs.GetString("can_touch_player").empty()) {
		idPlayer* player = gameLocal.GetLocalPlayer();

		if (player) {
			bool isPlayerTouchedNow = false;
			for (size_t i = 0; i < physicsObj->GetNumContacts(); ++i) {
				auto& ci = physicsObj->GetContact(i);
				auto& ent = gameLocal.entities[ci.entityNum];
				if (ent.get() == player) {
					isPlayerTouchedNow = true;
					break;
				}
			}
			if (isPlayerTouchedNow && !playerTouched)
				playerTouched = true;
			else if (!isPlayerTouchedNow && playerTouched)
				GetPhysics()->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_SOLID));
		}
	}

	// run physics
	RunPhysics();

	Present();
}

/*
=================
idProjectile::Collide
=================
*/
bool idProjectile::Collide(const trace_t& collision, const Vector2& velocity) noexcept {
	idEntity* ignore;
	std::string damageDefName;
	Vector2 dir{};

	if (state == projectileState_t::EXPLODED || state == projectileState_t::FIZZLED) {
		return true;
	}

	const bool isHitscan = spawnArgs.GetBool("net_instanthit");

	// get the entity the projectile collided with
	std::shared_ptr<idEntity> ent = gameLocal.entities[collision.c.entityNum];

	// direction of projectile
	dir = velocity;
	dir.Normalize();

	SetOrigin(collision.endpos);

	// unlink the clip model because we no longer need it
	GetPhysics()->UnlinkClip();

	damageDefName = spawnArgs.GetString("def_damage");

	ignore = nullptr;

	// if the hit entity takes damage
	if (ent->fl.takedamage) {
		if (!damageDefName.empty()) {
			ent->Damage(this, owner, dir, damageDefName);

			ignore = ent.get();
		}
	}

	Explode(collision, ignore);

	return true;
}

/*
================
idProjectile::Explode
================
*/
void idProjectile::Explode(const trace_t& collision, idEntity* ignore) {
	int removeTime;

	if (state == projectileState_t::EXPLODED || state == projectileState_t::FIZZLED) {
		return;
	}

	Hide();

	// default remove time
	removeTime = spawnArgs.GetInt("remove_time", "1500");

	fl.takedamage = false;
	physicsObj->SetContents(0);
	physicsObj->PutToRest();

	if (!spawnArgs.GetString("def_projectile").empty()) {
		size_t num_projectiles = spawnArgs.GetInt("num_projectiles");
		for (size_t j = 0; j < num_firebals; ++j) {
			for (size_t i = 0; i < num_projectiles; ++i) {
				std::shared_ptr<idEntity> ent;

				gameLocal.SpawnEntityDef(projectileDict, &ent);

				if (!ent || !ent->IsType(idProjectile::Type)) {
					const std::string projectileName = spawnArgs.GetString("def_projectile");
					gameLocal.Error("'%s' is not an idProjectile", projectileName.c_str());
					return;
				}

				std::shared_ptr<idProjectile> proj = std::static_pointer_cast<idProjectile>(ent);

				Vector2 size = spawnArgs.GetVector("size");
				Vector2 projSize = projectileDict.GetVector("size");
				Vector2 shift = spawnArgs.GetVector(va("projectile%d_shift", i));
				shift.x *= (size.x + projSize.x) / 2 * (j + 1);
				shift.y *= (size.y + projSize.y) / 2 * (j + 1);

				proj->Create(owner, GetPhysics()->GetOrigin() + shift, vec2_origin);
				proj->Launch(GetPhysics()->GetOrigin() + shift, vec2_origin, vec2_origin);
			}
		}
	}

	state = projectileState_t::EXPLODED;

	CancelEvents(&EV_Explode);
	PostEventMS(&EV_Remove, removeTime);
}

/*
================
idProjectile::Event_Explode
================
*/
void idProjectile::Event_Explode() { 
	trace_t collision;

	memset(&collision, 0, sizeof(collision));
	collision.endpos = GetPhysics()->GetOrigin();
	//collision.c.point = GetPhysics()->GetOrigin();
	Explode(collision, NULL);
}

void idProjectile::CacheProjectille(const std::string& objectname, idDeclEntityDef* declEntDef) {
	std::string projectileName = declEntDef->dict.GetString("def_projectile");
	if (!projectileName.empty()) {
		const std::shared_ptr<idDeclEntityDef>& projectileDef = gameLocal.FindEntityDef(projectileName, false);
		if (!projectileDef) {
			gameLocal.Warning("Unknown projectile '%s' in weapon '%s'", projectileName.c_str(), objectname.c_str());
		}
		else {
			const std::string& spawnclass = projectileDef->dict.GetString("spawnclass");
			idTypeInfo* cls = idClass::GetClass(spawnclass);
			if (!cls || !cls->IsType(idProjectile::Type)) {
				gameLocal.Warning("Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')", spawnclass.c_str(),
					projectileName.c_str(), objectname.c_str());
			}
		}
	}
}