#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include <memory>

#include "../renderer/RenderSystem.h"
#include "GUI_Types.h"
#include "GUI_ScriptVar.h"
#include "GUI_ScriptObject.h"
#include "GUI_SpriteInstance.h"
#include "GUI_TextInstance.h"

class GUI : public std::enable_shared_from_this<GUI> {
public:
	GUI(const std::string& filename_);
	~GUI() {}

	void Init();

	bool IsActive() { return isActive; }
	void Activate(bool b);

	void Render(idRenderSystem* gui, int time);
	bool HandleEvent(const sysEvent_t* event);

	GUIScriptVar GetGlobal(const std::string& name) { return globals->Get(name); }
	std::shared_ptr<GUIScriptObject> GetRootObject() { return mainspriteInstance->GetScriptObject(); }

	//----------------------------------
	// SWF_Render.cpp
	//----------------------------------
	void RenderSprite(idRenderSystem* gui, std::shared_ptr<GUISpriteInstance> sprite, const guiRenderState_t& renderState, int time);
	void RenderEditText(idRenderSystem* gui, std::shared_ptr<GUITextInstance> textInstance, const guiRenderState_t& renderState, int time);
private:
	std::string filename;
	bool isActive;

	std::shared_ptr<GUISpriteInstance> mainspriteInstance;

	std::shared_ptr<GUIScriptObject> globals;
	std::shared_ptr<GUIScriptObject> shortcutKeys;
};

#endif
