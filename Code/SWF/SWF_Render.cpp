#include "idlib/precompiled.h"

#include "../renderer/tr_local.h"

/*
========================
idSWF::Render
========================
*/
void idSWF::Render(idRenderSystem* gui, int time) {
  if (!IsActive()) {
    return;
  }

  const auto sysWidth = renderSystem->GetWidth();
  const auto sysHeight = renderSystem->GetHeight();

  swfRenderState_t renderState;
  renderState.pos.y = static_cast<float>(sysHeight / 2);
  renderState.pos.x = static_cast<float>(sysWidth / 2);

  RenderSprite(gui, mainspriteInstance.get(), renderState, time);
}

/*
========================
idSWF::RenderSprite
========================
*/
void idSWF::RenderSprite(idRenderSystem* gui,
                         idSWFSpriteInstance* spriteInstance,
                         const swfRenderState_t& renderState, int time) {
  if (!spriteInstance) {
    common->Warning("%s: RenderSprite: spriteInstance == NULL",
                    filename.c_str());
    return;
  }
  if (!spriteInstance->IsVisible()) {
    return;
  }

  for (size_t i = 0; i < spriteInstance->displayList.size(); i++) {
    const swfDisplayEntry_t& display = spriteInstance->displayList[i];

    swfRenderState_t renderState2;
    renderState2.pos = renderState.pos + display.pos;

    if (display.spriteInstance)
      RenderSprite(gui, display.spriteInstance.get(), renderState2, time);
    else
      RenderEditText(gui, display.textInstance.get(), renderState2, time);
  }
}

/*
========================
idSWF::RenderEditText
========================
*/
void idSWF::RenderEditText(gsl::not_null<idRenderSystem*> gui,
                           idSWFTextInstance* textInstance,
                           const swfRenderState_t& renderState, int time) {
  if (!textInstance) {
    common->Warning("%s: RenderEditText: textInstance == NULL",
                    filename.c_str());
    return;
  }

  if (!textInstance->visible) {
    return;
  }

  /*gui->DrawBigStringExt(static_cast<int>(renderState.pos.x),
                        static_cast<int>(renderState.pos.y), textInstance->text,
                        textInstance->color, true);*/
  gui->DrawString(renderState.pos, textInstance->text, textInstance->color);
}
