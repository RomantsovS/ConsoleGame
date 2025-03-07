#include "idlib/precompiled.h"

#include "Game_local.h"

idPlayerView::idPlayerView() {
  memset(&view, 0, sizeof(view));
  player = nullptr;
}

void idPlayerView::SetPlayerEntity(idPlayer* playerEnt) { player = playerEnt; }

void idPlayerView::SingleView(const renderView_t* view) {
  // normal rendering
  if (!view) {
    return;
  }

  gameRenderWorld->RenderScene(view);
}

void idPlayerView::RenderPlayerView() {
  auto view = player->GetRenderView();
  SingleView(view.get());
}
