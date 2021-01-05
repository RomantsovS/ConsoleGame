#include "GUI.h"
#include "../renderer/tr_local.h"

/*
========================
idSWF::Render
========================
*/
void GUI::Render(idRenderSystem* gui, int time) {
	if (!IsActive()) {
		return;
	}
	
	const auto sysWidth = renderSystem->GetWidth();
	const auto sysHeight = renderSystem->GetHeight();

	guiRenderState_t renderState;
	renderState.pos.y = static_cast<float>(sysWidth / 2);
	renderState.pos.x = static_cast<float>(sysHeight / 2);

	RenderSprite(gui, mainspriteInstance, renderState, time);
}

/*
========================
idSWF::RenderSprite
========================
*/
void GUI::RenderSprite(idRenderSystem* gui, std::shared_ptr<GUISpriteInstance> spriteInstance, const guiRenderState_t& renderState, int time) {

	if (!spriteInstance) {
		common->Warning("%s: RenderSprite: spriteInstance == NULL", filename.c_str());
		return;
	}
	if (!spriteInstance->IsVisible()) {
		return;
	}

	for (size_t i = 0; i < spriteInstance->displayList.size(); i++) {
		const guiDisplayEntry_t& display = spriteInstance->displayList[i];

		RenderEditText(gui, display.textInstance, renderState, time);
	}
}

/*
========================
idSWF::RenderEditText
========================
*/
void GUI::RenderEditText(idRenderSystem* gui, std::shared_ptr<GUITextInstance> textInstance, const guiRenderState_t& renderState, int time) {
	if (!textInstance) {
		common->Warning("%s: RenderEditText: textInstance == NULL", filename.c_str());
		return;
	}

	if (!textInstance->visible) {
		return;
	}

	gui->DrawString(renderState.pos, textInstance->text, textInstance->color);
}
