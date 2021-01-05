#ifndef GUI_GUI_SPRITEINSTANCE_H_
#define GUI_GUI_SPRITEINSTANCE_H_

#include <memory>
#include "../renderer/RenderSystem.h"
#include "GUI_Types.h"
#include "GUI_ScriptObject.h"

class GUISpriteInstance : public std::enable_shared_from_this<GUISpriteInstance> {
public:
	GUISpriteInstance();
	~GUISpriteInstance();

	void Init();

	std::shared_ptr<GUIScriptObject> GetScriptObject() { return scriptObject; }

	void SetVisible(bool visible);
	bool IsVisible() { return isVisible; }

	void SetXPos(float xPos = -1.0f);

	// sprite instances can be nested
	//std::weak_ptr<GUISpriteInstance> parent;

	// depth of this sprite instance in the parent's display list
	//int depth;

	std::shared_ptr<GUIScriptObject> scriptObject;

	// children display entries
	std::vector<guiDisplayEntry_t> displayList;
	guiDisplayEntry_t* FindDisplayEntry(int depth);

	//----------------------------------
	// SWF_PlaceObject.cpp
	//----------------------------------
	void PlaceObject();
	void RemoveObject();

	//----------------------------------
	// SWF_SpriteInstance.cpp
	//----------------------------------
	void RunTo(int frameNum);

	//void FreeDisplayList();
	guiDisplayEntry_t* AddDisplayEntry(int depth);
private:
	bool isVisible;
};

#endif
