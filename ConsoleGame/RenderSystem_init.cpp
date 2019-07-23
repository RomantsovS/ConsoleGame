#include "tr_local.h"
#include "ModelManager.h"
#include "Game_local.h"

void idRenderSystemLocal::Init()
{
	// clear all our internal state
	viewCount = 1;		// so cleared structures never match viewCount
	// we used to memset tr, but now that it is a class, we can't, so
	// there may be other state we need to reset

	renderModelManager->Init();

	borderHeight = 1;
	borderWidth = 1;

	height = gameLocal.GetHeight() + borderHeight * 2;
	width = gameLocal.GetWidth() + borderWidth * 2;

	borderPixel = Screen::Pixel('#', Screen::ConsoleColor::White);

	screen = Screen(height, width, Screen::Pixel(' ', Screen::ConsoleColor::Black));
}

/*
===============
idRenderSystemLocal::Shutdown
===============
*/
void idRenderSystemLocal::Shutdown()
{
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