#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

CLASS_DECLARATION(idEntity, idActor)
END_CLASS

idActor::idActor() {
}

idActor::~idActor() {
}

void idActor::Spawn() {
	// spawn any attachments we might have
	auto kv = spawnArgs.MatchPrefix("def_attach");
	while (kv)
	{
		idDict args;

		std::shared_ptr<idEntity> ent;

		args.Set("classname", kv->second);

		gameLocal.SpawnEntityDef(args, &ent);
		if (!ent)
		{
			gameLocal.Error("Couldn't spawn '%s' to attach to entity '%s'", kv->second.c_str(), name.c_str());
		}
		else
		{
			Attach(ent);
		}
		kv = spawnArgs.MatchPrefix("def_attach", kv->first);
	}
}
#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

/*
================
idActor::Attach
================
*/
void idActor::Attach(std::shared_ptr<idEntity> ent) {
	Vector2 origin;
	idAttachInfo attach;
	attachments.push_back(attach);
	Vector2 originOffset;

	originOffset = ent->spawnArgs.GetVector("origin");

	attach.ent = ent;

	ent->SetOrigin(origin + originOffset);
}
