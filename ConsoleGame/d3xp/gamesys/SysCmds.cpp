#pragma hdrstop
#include <precompiled.h>

#include "../Game_local.h"

/*
===================
Cmd_EntityList_f
===================
*/
void Cmd_EntityList_f(const idCmdArgs &args) {
	int e;
	int count;
	size_t size;
	std::string match;

	/*if (args.Argc() > 1) {
		match = args.Args();
		match.Replace(" ", "");
	}
	else {
		match = "";
	}*/

	count = 0;
	size = 0;

	gameLocal.Printf("%4s  %30s %16s %10s %s\n", " Num", "EntityDef", "Class", "pos", "At rest");
	gameLocal.Printf("--------------------------------------------------------------------\n");
	for (e = 0; e < MAX_GENTITIES; e++) {
		auto check = gameLocal.entities[e];

		if (!check) {
			continue;
		}

		/*if (!check->name.Filter(match, true)) {
			continue;
		}*/

		gameLocal.Printf("%4i: %30s %16s [%5.2f %5.2f] %d\n", e,
			check->GetName().c_str(), check->GetClassname().c_str(), check->GetPhysics()->GetOrigin().x,
			check->GetPhysics()->GetOrigin().y, check->GetPhysics()->IsAtRest());

		count++;
		//size += check->spawnArgs.Allocated();
	}

	gameLocal.Printf("...%d entities\t...%d bytes of spawnargs\n", count, size);
}

/*
===================
Cmd_EntityList_f
===================
*/
void Cmd_Quit(const idCmdArgs& args) {
	common->Quit();
}

/*
=================
idGameLocal::InitConsoleCommands

Let the system know about all of our commands
so it can perform tab completion
=================
*/
void idGameLocal::InitConsoleCommands() {
	/*cmdSystem->AddCommand("game_memory", idClass::DisplayInfo_f, CMD_FL_GAME, "displays game class info");
	cmdSystem->AddCommand("listClasses", idClass::ListClasses_f, CMD_FL_GAME, "lists game classes");
	cmdSystem->AddCommand("listThreads", idThread::ListThreads_f, CMD_FL_GAME | CMD_FL_CHEAT, "lists script threads");
	*/
	cmdSystem->AddCommand("listEntities", Cmd_EntityList_f, CMD_FL_GAME | CMD_FL_CHEAT, "lists game entities");
	cmdSystem->AddCommand("quit", Cmd_Quit, CMD_FL_GAME, "quits game");
	/*cmdSystem->AddCommand("listActiveEntities", Cmd_ActiveEntityList_f, CMD_FL_GAME | CMD_FL_CHEAT, "lists active game entities");
	cmdSystem->AddCommand("listMonsters", idAI::List_f, CMD_FL_GAME | CMD_FL_CHEAT, "lists monsters");
	cmdSystem->AddCommand("listSpawnArgs", Cmd_ListSpawnArgs_f, CMD_FL_GAME | CMD_FL_CHEAT, "list the spawn args of an entity", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand("say", Cmd_Say_f, CMD_FL_GAME, "text chat");
	cmdSystem->AddCommand("sayTeam", Cmd_SayTeam_f, CMD_FL_GAME, "team text chat");
	cmdSystem->AddCommand("addChatLine", Cmd_AddChatLine_f, CMD_FL_GAME, "internal use - core to game chat lines");
	cmdSystem->AddCommand("give", Cmd_Give_f, CMD_FL_GAME | CMD_FL_CHEAT, "gives one or more items");
	cmdSystem->AddCommand("centerview", Cmd_CenterView_f, CMD_FL_GAME, "centers the view");
	cmdSystem->AddCommand("god", Cmd_God_f, CMD_FL_GAME | CMD_FL_CHEAT, "enables god mode");
	cmdSystem->AddCommand("notarget", Cmd_Notarget_f, CMD_FL_GAME | CMD_FL_CHEAT, "disables the player as a target");
	cmdSystem->AddCommand("noclip", Cmd_Noclip_f, CMD_FL_GAME | CMD_FL_CHEAT, "disables collision detection for the player");
	cmdSystem->AddCommand("where", Cmd_GetViewpos_f, CMD_FL_GAME | CMD_FL_CHEAT, "prints the current view position");
	cmdSystem->AddCommand("getviewpos", Cmd_GetViewpos_f, CMD_FL_GAME | CMD_FL_CHEAT, "prints the current view position");
	cmdSystem->AddCommand("setviewpos", Cmd_SetViewpos_f, CMD_FL_GAME | CMD_FL_CHEAT, "sets the current view position");
	cmdSystem->AddCommand("teleport", Cmd_Teleport_f, CMD_FL_GAME | CMD_FL_CHEAT, "teleports the player to an entity location", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand("trigger", Cmd_Trigger_f, CMD_FL_GAME | CMD_FL_CHEAT, "triggers an entity", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand("spawn", Cmd_Spawn_f, CMD_FL_GAME | CMD_FL_CHEAT, "spawns a game entity", idCmdSystem::ArgCompletion_Decl<DECL_ENTITYDEF>);
	cmdSystem->AddCommand("damage", Cmd_Damage_f, CMD_FL_GAME | CMD_FL_CHEAT, "apply damage to an entity", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand("remove", Cmd_Remove_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes an entity", idGameLocal::ArgCompletion_EntityName);
	cmdSystem->AddCommand("killMonsters", Cmd_KillMonsters_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes all monsters");
	cmdSystem->AddCommand("killMoveables", Cmd_KillMovables_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes all moveables");
	cmdSystem->AddCommand("killRagdolls", Cmd_KillRagdolls_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes all ragdolls");
	cmdSystem->AddCommand("addline", Cmd_AddDebugLine_f, CMD_FL_GAME | CMD_FL_CHEAT, "adds a debug line");
	cmdSystem->AddCommand("addarrow", Cmd_AddDebugLine_f, CMD_FL_GAME | CMD_FL_CHEAT, "adds a debug arrow");
	cmdSystem->AddCommand("removeline", Cmd_RemoveDebugLine_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes a debug line");
	cmdSystem->AddCommand("blinkline", Cmd_BlinkDebugLine_f, CMD_FL_GAME | CMD_FL_CHEAT, "blinks a debug line");
	cmdSystem->AddCommand("listLines", Cmd_ListDebugLines_f, CMD_FL_GAME | CMD_FL_CHEAT, "lists all debug lines");
	cmdSystem->AddCommand("playerModel", Cmd_PlayerModel_f, CMD_FL_GAME | CMD_FL_CHEAT, "sets the given model on the player", idCmdSystem::ArgCompletion_Decl<DECL_MODELDEF>);
	cmdSystem->AddCommand("testFx", Cmd_TestFx_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests an FX system", idCmdSystem::ArgCompletion_Decl<DECL_FX>);
	cmdSystem->AddCommand("testBoneFx", Cmd_TestBoneFx_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests an FX system bound to a joint", idCmdSystem::ArgCompletion_Decl<DECL_FX>);
	cmdSystem->AddCommand("testLight", Cmd_TestLight_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests a light");
	cmdSystem->AddCommand("testPointLight", Cmd_TestPointLight_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests a point light");
	cmdSystem->AddCommand("popLight", Cmd_PopLight_f, CMD_FL_GAME | CMD_FL_CHEAT, "removes the last created light");
	cmdSystem->AddCommand("testDeath", Cmd_TestDeath_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests death");
	cmdSystem->AddCommand("testSave", Cmd_TestSave_f, CMD_FL_GAME | CMD_FL_CHEAT, "writes out a test savegame");
	cmdSystem->AddCommand("testModel", idTestModel::TestModel_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests a model", idTestModel::ArgCompletion_TestModel);
	cmdSystem->AddCommand("testSkin", idTestModel::TestSkin_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests a skin on an existing testModel", idCmdSystem::ArgCompletion_Decl<DECL_SKIN>);
	cmdSystem->AddCommand("testShaderParm", idTestModel::TestShaderParm_f, CMD_FL_GAME | CMD_FL_CHEAT, "sets a shaderParm on an existing testModel");
	cmdSystem->AddCommand("keepTestModel", idTestModel::KeepTestModel_f, CMD_FL_GAME | CMD_FL_CHEAT, "keeps the last test model in the game");
	cmdSystem->AddCommand("testAnim", idTestModel::TestAnim_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests an animation", idTestModel::ArgCompletion_TestAnim);
	cmdSystem->AddCommand("testParticleStopTime", idTestModel::TestParticleStopTime_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests particle stop time on a test model");
	cmdSystem->AddCommand("nextAnim", idTestModel::TestModelNextAnim_f, CMD_FL_GAME | CMD_FL_CHEAT, "shows next animation on test model");
	cmdSystem->AddCommand("prevAnim", idTestModel::TestModelPrevAnim_f, CMD_FL_GAME | CMD_FL_CHEAT, "shows previous animation on test model");
	cmdSystem->AddCommand("nextFrame", idTestModel::TestModelNextFrame_f, CMD_FL_GAME | CMD_FL_CHEAT, "shows next animation frame on test model");
	cmdSystem->AddCommand("prevFrame", idTestModel::TestModelPrevFrame_f, CMD_FL_GAME | CMD_FL_CHEAT, "shows previous animation frame on test model");
	cmdSystem->AddCommand("testBlend", idTestModel::TestBlend_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests animation blending");
	cmdSystem->AddCommand("reloadScript", Cmd_ReloadScript_f, CMD_FL_GAME | CMD_FL_CHEAT, "reloads scripts");
	cmdSystem->AddCommand("script", Cmd_Script_f, CMD_FL_GAME | CMD_FL_CHEAT, "executes a line of script");
	cmdSystem->AddCommand("listCollisionModels", Cmd_ListCollisionModels_f, CMD_FL_GAME, "lists collision models");
	cmdSystem->AddCommand("collisionModelInfo", Cmd_CollisionModelInfo_f, CMD_FL_GAME, "shows collision model info");
	cmdSystem->AddCommand("reloadanims", Cmd_ReloadAnims_f, CMD_FL_GAME | CMD_FL_CHEAT, "reloads animations");
	cmdSystem->AddCommand("listAnims", Cmd_ListAnims_f, CMD_FL_GAME, "lists all animations");
	cmdSystem->AddCommand("aasStats", Cmd_AASStats_f, CMD_FL_GAME, "shows AAS stats");
	cmdSystem->AddCommand("testDamage", Cmd_TestDamage_f, CMD_FL_GAME | CMD_FL_CHEAT, "tests a damage def", idCmdSystem::ArgCompletion_Decl<DECL_ENTITYDEF>);
	cmdSystem->AddCommand("weaponSplat", Cmd_WeaponSplat_f, CMD_FL_GAME | CMD_FL_CHEAT, "projects a blood splat on the player weapon");
	cmdSystem->AddCommand("saveSelected", Cmd_SaveSelected_f, CMD_FL_GAME | CMD_FL_CHEAT, "saves the selected entity to the .map file");
	cmdSystem->AddCommand("deleteSelected", Cmd_DeleteSelected_f, CMD_FL_GAME | CMD_FL_CHEAT, "deletes selected entity");
	cmdSystem->AddCommand("saveMoveables", Cmd_SaveMoveables_f, CMD_FL_GAME | CMD_FL_CHEAT, "save all moveables to the .map file");
	cmdSystem->AddCommand("saveRagdolls", Cmd_SaveRagdolls_f, CMD_FL_GAME | CMD_FL_CHEAT, "save all ragdoll poses to the .map file");
	cmdSystem->AddCommand("bindRagdoll", Cmd_BindRagdoll_f, CMD_FL_GAME | CMD_FL_CHEAT, "binds ragdoll at the current drag position");
	cmdSystem->AddCommand("unbindRagdoll", Cmd_UnbindRagdoll_f, CMD_FL_GAME | CMD_FL_CHEAT, "unbinds the selected ragdoll");
	cmdSystem->AddCommand("saveLights", Cmd_SaveLights_f, CMD_FL_GAME | CMD_FL_CHEAT, "saves all lights to the .map file");
	cmdSystem->AddCommand("saveParticles", Cmd_SaveParticles_f, CMD_FL_GAME | CMD_FL_CHEAT, "saves all lights to the .map file");
	cmdSystem->AddCommand("clearLights", Cmd_ClearLights_f, CMD_FL_GAME | CMD_FL_CHEAT, "clears all lights");
	cmdSystem->AddCommand("gameError", Cmd_GameError_f, CMD_FL_GAME | CMD_FL_CHEAT, "causes a game error");
*/
}

/*
=================
idGameLocal::ShutdownConsoleCommands
=================
*/
void idGameLocal::ShutdownConsoleCommands() {
	cmdSystem->RemoveFlaggedCommands(CMD_FL_GAME);
}