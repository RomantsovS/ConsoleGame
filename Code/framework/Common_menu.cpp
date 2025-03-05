#include "idlib/precompiled.h"

#include "Common_local.h"

/*
==============
idCommonLocal::InitializeMPMapsModes
==============
*/
void idCommonLocal::InitializeMPMapsModes() {
  int numMaps = declManager->GetNumDecls(declType_t::DECL_MAPDEF);
  mpGameMaps.clear();
  for (int i = 0; i < numMaps; i++) {
    const std::shared_ptr<idDeclEntityDef> mapDef =
        std::static_pointer_cast<idDeclEntityDef>(
            declManager->DeclByIndex(declType_t::DECL_MAPDEF, i));
    uint32_t supportedModes = 1;

    if (supportedModes != 0) {
      mpGameMaps.push_back({});
      auto& mpMap = mpGameMaps.back();
      mpMap.mapFile = mapDef->GetName();
      mpMap.mapName = mapDef->dict.GetString("name", mpMap.mapFile);
    }
  }
}

/*
==============
idCommonLocal::OnStartHosting
==============
*/
void idCommonLocal::OnStartHosting(idMatchParameters& parms) {
  if ((parms.matchFlags &
       static_cast<int>(matchFlags_t::MATCH_REQUIRE_PARTY_LOBBY)) == 0) {
    return;  // This is the party lobby or a SP match
  }

  // If we were searching for a random match but didn't find one, we'll need to
  // select parameters now
  if (parms.gameMap < 0) {
    // Random mode means any map will do
    parms.gameMap = idLib::frameNumber % mpGameMaps.size();
  }

  parms.mapName = mpGameMaps[parms.gameMap].mapFile;
}

/*
==============
idCommonLocal::StartMainMenu
==============
*/
void idCommonLocal::StartMenu(bool playIntro) {
  if (game && game->Shell_IsActive()) {
    return;
  }

  if (game) {
    game->Shell_Show(true);
    game->Shell_SyncWithSession();
  }
}

/*
===============
idCommonLocal::ExitMenu
===============
*/
void idCommonLocal::ExitMenu() {
  if (game) {
    game->Shell_Show(false);
  }
}

/*
==============
idCommonLocal::MenuEvent

Executes any commands returned by the gui
==============
*/
bool idCommonLocal::MenuEvent(const sysEvent_t* event) {
  if (game && game->Shell_IsActive()) {
    return game->Shell_HandleGuiEvent(event);
  }

  return false;
}

/*
=================
idCommonLocal::GuiFrameEvents
=================
*/
void idCommonLocal::GuiFrameEvents() {
  if (game) {
    game->Shell_SyncWithSession();
  }
}
