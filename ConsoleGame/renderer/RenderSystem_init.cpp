#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"
#include "../d3xp/Game_local.h"

// Vista OpenGL wrapper check
#include "../sys/win32/win_local.h"

idCVar r_skipBackEnd("r_skipBackEnd", "0", CVAR_RENDERER | CVAR_BOOL, "don't draw anything");

/*
=================
R_InitMaterials
=================
*/
void R_InitMaterials() {
	/*tr.defaultMaterial = declManager->FindMaterial("_default", false);
	if (!tr.defaultMaterial) {
		common->FatalError("_default material not found");
	}*/
	tr.charSetMaterial = declManager->FindMaterial("textures/bigchars");
}

void idRenderSystemLocal::Init() {
	Sys_InitInput();

	common->Printf("------- Initializing renderSystem ----\n");

	// clear all our internal state
	viewCount = 1;		// so cleared structures never match viewCount
	// we used to memset tr, but now that it is a class, we can't, so
	// there may be other state we need to reset

	R_InitMaterials();

	renderModelManager->Init();

	borderHeight = 2;
	borderWidth = 2;

	height = game_height.GetInteger() + borderHeight * 2;
	width = game_width.GetInteger() + borderWidth * 2;

	borderPixel = Screen::Pixel('#', colorWhite);

	screen = Screen(height + max_debug_text.GetInteger() * 10, width, Screen::Pixel(' ', colorBlack));
	screen.init();
	
	viewDef = nullptr;

	update_frame = update_info = true;

	r_initialized = true;

	common->Printf("renderSystem initialized.\n");
	common->Printf("--------------------------------------\n");
}

/*
===============
idRenderSystemLocal::Shutdown
===============
*/
void idRenderSystemLocal::Shutdown() {
	common->Printf("idRenderSystem::Shutdown()\n");

	if (R_IsInitialized()) {
		//globalImages->PurgeAllImages();
	}

	renderModelManager->Shutdown();

	Clear();
}

/*
========================
idRenderSystemLocal::BeginLevelLoad
========================
*/
void idRenderSystemLocal::BeginLevelLoad() {
	renderModelManager->BeginLevelLoad();
}

/*
========================
idRenderSystemLocal::EndLevelLoad
========================
*/
void idRenderSystemLocal::EndLevelLoad() {
	renderModelManager->EndLevelLoad();
}

bool r_initialized = false;

/*
=============================
R_IsInitialized
=============================
*/
bool R_IsInitialized() {
	return r_initialized;
}