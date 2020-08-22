#include "tr_local.h"
#include "ModelManager.h"
#include "../d3xp/Game_local.h"

void idRenderSystemLocal::Init()
{
	Sys_InitInput();

	common->Printf("------- Initializing renderSystem ----\n");

	// clear all our internal state
	viewCount = 1;		// so cleared structures never match viewCount
	// we used to memset tr, but now that it is a class, we can't, so
	// there may be other state we need to reset

	renderModelManager->Init();

	borderHeight = 2;
	borderWidth = 4;

	height = game_height.GetInteger() + borderHeight * 2;
	width = game_width.GetInteger() + borderWidth * 2;

	borderPixel = Screen::Pixel('#', Screen::ConsoleColor::White);

	screen = Screen(height, width, Screen::Pixel(' ', Screen::ConsoleColor::Black));
	screen.init();
	
	viewDef = nullptr;

	update_frame = true;
	console.clear();

	common->Printf("renderSystem initialized.\n");
	common->Printf("--------------------------------------\n");
}

/*
===============
idRenderSystemLocal::Shutdown
===============
*/
void idRenderSystemLocal::Shutdown()
{
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
void idRenderSystemLocal::BeginLevelLoad()
{
	renderModelManager->BeginLevelLoad();
}

/*
========================
idRenderSystemLocal::EndLevelLoad
========================
*/
void idRenderSystemLocal::EndLevelLoad()
{
	renderModelManager->EndLevelLoad();
}

static bool r_initialized = false;

/*
=============================
R_IsInitialized
=============================
*/
bool R_IsInitialized() {
	return r_initialized;
}