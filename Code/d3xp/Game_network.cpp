#include "idlib/precompiled.h"

#include "Game_local.h"
#include "..\framework\Common_local.h"

/*
================
idGameLocal::SyncPlayersWithLobbyUsers
================
*/
void idGameLocal::SyncPlayersWithLobbyUsers(bool initial) {
	// spawn the player
	SpawnPlayer(0);
}

/*
================
idGameLocal::ServerWriteSnapshot

  Write a snapshot of the current game state
================
*/
void idGameLocal::ServerWriteSnapshot(idSnapShot& ss) {
	std::array<std::byte, MAX_ENTITY_STATE_SIZE> buffer;
	idBitMsg msg;

	msg.InitWrite(buffer.data(), sizeof(buffer));

	msg.WriteLong(spawnedEntities.Num());

	// Add all entities to the snapshot
	for (auto ent = spawnedEntities.Next(); ent; ent = ent->spawnNode.Next()) {
		/*if (ent->GetSkipReplication()) {
			continue;
		}*/

		msg.WriteLong(ent->GetType()->typeNum);
		//msg.WriteLong(ent->entityDefNumber);
		msg.WriteLong(ent->entityNumber);

		ent->WriteToSnapshot(msg);
	}
}

/*
================
idGameLocal::ClientReadSnapshot
================
*/
void idGameLocal::ClientReadSnapshot(const idSnapShot& ss) {
	if (GetLocalClientNum() < 0) {
		return;
	}

	// clear any debug lines from a previous frame
	gameRenderWorld->DebugClearLines(time);

	SelectTimeGroup(false);

	// so that StartSound/StopSound doesn't risk skipping
	//isNewFrame = true;

	// clear the snapshot entity list
	//snapshotEntities.Clear();

	idBitMsg msg(ss.GetData(), ss.GetSize());

	int num = msg.ReadLong();

	// read all entities from the snapshot
	for (int o = 0; o < num; o++) {
		

		//int spawnId = msg.ReadBits(32 - GENTITYNUM_BITS);
		int typeNum = msg.ReadLong();
		//int entityDefNumber = msg.ReadLong();
		int entityNumber = msg.ReadLong();

		idTypeInfo* typeInfo = idClass::GetType(typeNum);
		if (!typeInfo) {
			idLib::Error("Unknown type number %d for entity %d with class number %d", typeNum, entityNumber, 0/*entityDefNumber*/);
		}

		auto ent = entities[entityNumber];

		// if there is no entity or an entity of the wrong type
		if (!ent || ent->GetType()->typeNum != typeNum /* || ent->entityDefNumber != entityDefNumber || spawnId != spawnIds[entityNumber]*/) {
			ent = nullptr;

			//spawnCount = spawnId;

			if (entityNumber < MAX_CLIENTS) {
				commonLocal.GetUCmdMgr().ResetPlayer(entityNumber);
				SpawnPlayer(entityNumber);
				ent = entities[entityNumber];
				ent->FreeModelDef();
			}
			else {
				idDict args;
				args.SetInt("spawn_entnum", entityNumber);
				args.Set("name", va("entity%d", entityNumber));

				/*if (entityDefNumber >= 0) {
					if (entityDefNumber >= declManager->GetNumDecls(DECL_ENTITYDEF)) {
						Error("server has %d entityDefs instead of %d", entityDefNumber, declManager->GetNumDecls(DECL_ENTITYDEF));
					}
					const char* classname = declManager->DeclByIndex(DECL_ENTITYDEF, entityDefNumber, false)->GetName();
					args.Set("classname", classname);
					if (!SpawnEntityDef(args, &ent) || !entities[entityNumber] || entities[entityNumber]->GetType()->typeNum != typeNum) {
						Error("Failed to spawn entity with classname '%s' of type '%s'", classname, typeInfo->classname);
					}
				}
				else {*/
					ent = SpawnEntityType(*typeInfo, &args, true);
					if (!entities[entityNumber] || entities[entityNumber]->GetType()->typeNum != typeNum) {
						Error("Failed to spawn entity of type '%s'", typeInfo->classname.c_str());
					}
				//}
				if (ent) {
					// Fixme: for now, force all think flags on. We'll need to figure out how we want dormancy to work on clients
					// (but for now since clientThink is so light weight, this is ok)
					ent->BecomeActive(TH_ANIMATE);
					ent->BecomeActive(TH_THINK);
					ent->BecomeActive(TH_PHYSICS);
				}
				/*if (entityNumber < MAX_CLIENTS && entityNumber >= numClients) {
					numClients = entityNumber + 1;
				}*/
			}
		}
	}

	// process entity events
	//ClientProcessEntityNetworkEventQueue();
}

/*
================
idGameLocal::ClientRunFrame
================
*/
void idGameLocal::ClientRunFrame(idUserCmdMgr& cmdMgr, bool lastPredictFrame, gameReturn_t& ret) {
	// update the game time
	previousTime = FRAME_TO_MSEC(framenum);
	framenum++;
	time = FRAME_TO_MSEC(framenum);

	auto player = static_cast<idPlayer*>(entities[GetLocalClientNum()].get());
	if (!player) {

		// service any pending events
		idEvent::ServiceEvents();

		return;
	}

	// service any pending events
	idEvent::ServiceEvents();

	BuildReturnValue(ret);
}