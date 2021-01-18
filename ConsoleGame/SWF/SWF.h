#ifndef SWF_SWF_H_
#define SWF_SWF_H_

#include "SWF_Types.h"
#include "SWF_ScriptVar.h"
#include "SWF_ScriptObject.h"
#include "SWF_ParmList.h"
#include "SWF_ScriptFunction.h"
#include "SWF_SpriteInstance.h"
#include "SWF_TextInstance.h"

class idSWF : public std::enable_shared_from_this<idSWF> {
public:
	idSWF(const std::string& filename_);
	~idSWF() {}

	void Init();

	bool IsActive() { return isActive; }
	void Activate(bool b);

	void Render(idRenderSystem* gui, int time);
	bool HandleEvent(const sysEvent_t* event);

	idSWFScriptVar GetGlobal(const std::string& name) { return globals->Get(name); }
	std::shared_ptr<idSWFScriptObject> GetRootObject() { return mainspriteInstance->GetScriptObject(); }

	//----------------------------------
	// SWF_Render.cpp
	//----------------------------------
	void RenderSprite(idRenderSystem* gui, std::shared_ptr<idSWFSpriteInstance> sprite, const swfRenderState_t& renderState, int time);
	void RenderEditText(idRenderSystem* gui, std::shared_ptr<idSWFTextInstance> textInstance, const swfRenderState_t& renderState, int time);
private:
	std::string filename;
	bool isActive;

	std::shared_ptr<idSWFSpriteInstance> mainspriteInstance;

	std::shared_ptr<idSWFScriptObject> globals;
	std::shared_ptr<idSWFScriptObject> shortcutKeys;

#define SWF_NATIVE_FUNCTION_SWF_DECLARE( x ) \
	class idSWFScriptFunction_##x : public idSWFScriptFunction_Nested< idSWF > { \
	public: \
		idSWFScriptVar Call( idSWFScriptObject * thisObject, const idSWFParmList & parms ); \
	} scriptFunction_##x;

	SWF_NATIVE_FUNCTION_SWF_DECLARE(shortcutKeys_clear);
};

#endif
