#ifndef GUI_GUI_SPRITEINSTANCE_H_
#define GUI_GUI_SPRITEINSTANCE_H_

#include <memory>
#include "../renderer/RenderSystem.h"

class GUI;

class GUISpriteInstance : public std::enable_shared_from_this<GUISpriteInstance> {
public:
	GUISpriteInstance(std::shared_ptr<GUI> _gui);
	~GUISpriteInstance();

	void Init();

	std::shared_ptr<GUI> GetGUI() { if (auto res = gui.lock()) { return res; } else { return nullptr; } }

	std::shared_ptr<GUIScriptObject> GetScriptObject() { return scriptObject; }

	void SetVisible(bool visible);
	bool IsVisible() { return isVisible; }

	void SetXPos(float xPos = -1.0f);

	bool isSprite;

	// sprite instances can be nested
	//std::weak_ptr<GUISpriteInstance> parent;

	// depth of this sprite instance in the parent's display list
	//int depth;

	std::shared_ptr<GUIScriptObject> scriptObject;

	// children display entries
	std::vector<guiDisplayEntry_t> displayList;
	guiDisplayEntry_t* FindDisplayEntry(int depth);

	// name of this sprite instance
	std::string name;

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
	guiDisplayEntry_t* AddDisplayEntry(int depth, bool createText);
private:
	std::weak_ptr<GUI> gui;
	bool isVisible;
};

#endif
