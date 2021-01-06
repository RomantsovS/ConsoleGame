#ifndef GUI_GUI_TYPES_H_
#define GUI_GUI_TYPES_H_

#include <memory>
#include "../idlib/math/Vector2.h"

class GUISpriteInstance;
class GUITextInstance;

struct guiDisplayEntry_t {
	unsigned short int depth;
	Vector2 pos;
	// if this entry is a sprite, then this will point to the specific instance of that sprite
	std::shared_ptr<GUISpriteInstance> spriteInstance;
	// if this entry is text, then this will point to the specific instance of the text
	std::shared_ptr<GUITextInstance> textInstance;
};

struct guiRenderState_t {
	Vector2 pos;
};

#endif
