
#include "idlib/precompiled.h"

#include "../d3xp/Game_local.h"
#include "tr_local.h"

idCVar r_skipBackEnd("r_skipBackEnd", "0", CVAR_RENDERER | CVAR_BOOL,
                     "don't draw anything");
idCVar r_update_frame_time("r_update_frame_time", "100",
                           CVAR_SYSTEM | CVAR_RENDERER, "");
idCVar r_console_pos("r_console_pos", "10", CVAR_SYSTEM | CVAR_RENDERER, "");
idCVar screen_width("screen_width", "120", CVAR_SYSTEM | CVAR_INIT, "");
idCVar screen_height("screen_height", "40", CVAR_SYSTEM | CVAR_INIT, "");

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

void idRenderSystemLocal::Clear() {
  frameCount = 0;
  viewCount = 0;

  if (screen) {
    screen->clear();
  }

  // free all the entityDefs, lightDefs, portals, etc
  for (auto& world : worlds) world->FreeWorld();

  worlds.clear();

  if (viewDef) {
    viewDef->renderWorld = nullptr;
    viewDef = nullptr;
  }

  currentColorNativeBytesOrder = 15;

  update_frame = true;
}

void idRenderSystemLocal::Init() {
  Sys_InitInput();

  common->Printf("------- Initializing renderSystem ----\n");

  // clear all our internal state
  viewCount = 1;  // so cleared structures never match viewCount
  // we used to memset tr, but now that it is a class, we can't, so
  // there may be other state we need to reset

  R_InitMaterials();

  renderModelManager->Init();

  // borderWidth = 0;
  // borderHeight = 0;

  width = screen_width.GetInteger();    // +borderWidth * 2;
  height = screen_height.GetInteger();  // +borderHeight * 2;

  borderPixel = Screen::Pixel('#', colorWhite);

  screen = MakeScreen(width, height, Screen::Pixel('\xDB', colorBlack));
  screen->init();

  viewDef = nullptr;

  update_frame = true;
  update_frame_time = r_update_frame_time.GetInteger();

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
    // globalImages->PurgeAllImages();
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
void idRenderSystemLocal::EndLevelLoad() { renderModelManager->EndLevelLoad(); }

bool r_initialized = false;

/*
=============================
R_IsInitialized
=============================
*/
bool R_IsInitialized() noexcept { return r_initialized; }