#include "idlib/precompiled.h"

#include "Game_local.h"
#include "../framework/Common_local.h"

static const int SNAP_GAMESTATE = 0;
static const int SNAP_SHADERPARMS = 1;
static const int SNAP_PORTALS = 2;
static const int SNAP_PLAYERSTATE = SNAP_PORTALS + 1;
static const int SNAP_PLAYERSTATE_END = SNAP_PLAYERSTATE + MAX_PLAYERS;
static const int SNAP_ENTITIES = SNAP_PLAYERSTATE_END;
static const int SNAP_ENTITIES_END = SNAP_ENTITIES + MAX_GENTITIES;
static const int SNAP_LAST_CLIENT_FRAME = SNAP_ENTITIES_END;
static const int SNAP_LAST_CLIENT_FRAME_END = SNAP_LAST_CLIENT_FRAME + MAX_PLAYERS;

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
	ss.SetTime(fast.time);

	std::array<std::byte, MAX_ENTITY_STATE_SIZE> buffer;
	idBitMsg msg;

	// Add all entities to the snapshot
	for (auto ent = spawnedEntities.Next(); ent; ent = ent->spawnNode.Next()) {
		/*if (ent->GetSkipReplication()) {
			continue;
		}*/

		msg.InitWrite(buffer.data(), sizeof(buffer));

		msg.WriteUShort(spawnIds[ent->entityNumber]);
		msg.WriteLong(ent->GetType()->typeNum);
		msg.WriteLong(ent->entityDefNumber);
		msg.WriteLong(ent->entityNumber);

		ent->WriteToSnapshot(msg);

		ss.S_AddObject(SNAP_ENTITIES + ent->entityNumber, msg);
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

	// read all entities from the snapshot
	for (int o = 0; o < ss.NumObjects(); o++) {
		idBitMsg msg;
		int snapObjectNum = ss.GetObjectMsgByIndex(o, msg);
		if (snapObjectNum < 0) {
			idassert(false);
			continue;
		}

		/*if (snapObjectNum < SNAP_ENTITIES || snapObjectNum >= SNAP_ENTITIES_END) {
			continue;
		}*/

		//int entityNumber = snapObjectNum - SNAP_ENTITIES;

		int spawnId = msg.ReadUShort();
		int typeNum = msg.ReadLong();
		int entityDefNumber = msg.ReadLong();
		int entityNumber = msg.ReadLong();

		if (msg.GetSize() == 0) {
			entities[entityNumber] = nullptr;
			continue;
		}

		idTypeInfo* typeInfo = idClass::GetType(typeNum);
		if (!typeInfo) {
			idLib::Error("Unknown type number %d for entity %d with class number %d", typeNum, entityNumber, 0/*entityDefNumber*/);
		}

		auto& ent = entities[entityNumber];

		// if there is no entity or an entity of the wrong type
		if (!ent || ent->GetType()->typeNum != typeNum || ent->entityDefNumber != entityDefNumber || spawnId != spawnIds[entityNumber]) {
			ent = nullptr;

			spawnCount = spawnId;

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

				if (entityDefNumber >= 0) {
					if (entityDefNumber >= declManager->GetNumDecls(declType_t::DECL_ENTITYDEF)) {
						Error("server has %d entityDefs instead of %d", entityDefNumber, declManager->GetNumDecls(declType_t::DECL_ENTITYDEF));
					}
					const std::string& classname = declManager->DeclByIndex(declType_t::DECL_ENTITYDEF, entityDefNumber, false)->GetName();
					args.Set("classname", classname);
					if (!SpawnEntityDef(args, &ent) || !entities[entityNumber] || entities[entityNumber]->GetType()->typeNum != typeNum) {
						Error("Failed to spawn entity with classname '%s' of type '%s'", classname.c_str(), typeInfo->classname.c_str());
					}
				}
				else {
					ent = SpawnEntityType(*typeInfo, &args, true);
					if (!entities[entityNumber] || entities[entityNumber]->GetType()->typeNum != typeNum) {
						Error("Failed to spawn entity of type '%s'", typeInfo->classname.c_str());
					}
				}
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

		// read the class specific data from the snapshot
		if (msg.GetRemainingReadBytes() > 0) {
			ent->ReadFromSnapshot_Ex(msg);
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

	// run prediction on all active entities
	for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next()) {
		ent->thinkFlags |= TH_PHYSICS;

		if (ent->entityNumber != GetLocalClientNum()) {
			ent->ClientThink();
		}
		else {
			RunAllUserCmdsForPlayer(cmdMgr, ent->entityNumber);
		}
	}

	// service any pending events
	idEvent::ServiceEvents();

	BuildReturnValue(ret);
}