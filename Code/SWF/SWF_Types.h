#ifndef GUI_GUI_TYPES_H_
#define GUI_GUI_TYPES_H_

class idSWFSpriteInstance;
class idSWFTextInstance;

struct swfDisplayEntry_t {
	unsigned short int depth;
	Vector2 pos;
	// if this entry is a sprite, then this will point to the specific instance of that sprite
	std::shared_ptr<idSWFSpriteInstance> spriteInstance;
	// if this entry is text, then this will point to the specific instance of the text
	std::shared_ptr<idSWFTextInstance> textInstance;
};

struct swfRenderState_t {
	Vector2 pos;
};

#endif
