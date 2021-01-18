#pragma hdrstop
#include "../idlib/precompiled.h"

/*
===================
idSWF::idSWF
===================
*/
idSWF::idSWF(const std::string& filename_) :
	filename(filename_),
	isActive(false) {
}

/*
===================
idSWF::Init
===================
*/
void idSWF::Init() {
	globals = std::make_shared<idSWFScriptObject>();

	mainspriteInstance = std::make_shared<idSWFSpriteInstance>(shared_from_this());
	mainspriteInstance->Init();
	
	mainspriteInstance->name = "mainspriteInstance";
	mainspriteInstance->SetVisible(true);

	mainspriteInstance->PlaceObject(0, true, "menuStart");
	
	auto spr_inst_prompts = mainspriteInstance->PlaceObject(1, true, "prompts");
	auto spr_inst_prompts_joy1 = spr_inst_prompts->spriteInstance->PlaceObject(0, true, "joy1", Vector2(0, -10));
	spr_inst_prompts_joy1->spriteInstance->PlaceObject(0, false, "txt_info");
	auto spr_inst_prompts_joy2 = spr_inst_prompts->spriteInstance->PlaceObject(1, true, "joy2", Vector2(0, -20));
	spr_inst_prompts_joy2->spriteInstance->PlaceObject(0, false, "txt_info");

	auto spr_inst_menuMain = mainspriteInstance->PlaceObject(2, true, "menuMain");
	auto spr_inst_menuMain_info = spr_inst_menuMain->spriteInstance->PlaceObject(0, true, "info");
	auto spr_inst_menuMain_options = spr_inst_menuMain_info->spriteInstance->PlaceObject(1, true, "options");
	for (size_t i = 0; i < 6; ++i)
		spr_inst_menuMain_options->spriteInstance->PlaceObject(i, true, va("item%d", i));

	auto spr_inst_pcBar = mainspriteInstance->PlaceObject(3, true, "pcBar", Vector2(10, 0));
	auto spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(0, true, "btn0", Vector2(0, -30));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
	spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(1, true, "btn1", Vector2(0, -20));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
	spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(2, true, "btn2", Vector2(0, 0));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
	spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(3, true, "btn3", Vector2(0, 10));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
	spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(4, true, "btn4", Vector2(0, 20));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
	spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(5, true, "btn5", Vector2(0, 30));
	spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");

	auto spr_inst_buttons = mainspriteInstance->PlaceObject(4, true, "buttons");
	spr_inst_buttons->spriteInstance->PlaceObject(0, true, "btnLeft");
	spr_inst_buttons->spriteInstance->PlaceObject(1, true, "btnRight");

	auto spr_inst_menuCampaing = mainspriteInstance->PlaceObject(5, true, "menuCampaign");
	auto spr_inst_menuCampaing_info = spr_inst_menuCampaing->spriteInstance->PlaceObject(0, true, "info");
	spr_inst_menuCampaing_info->spriteInstance->SetVisible(true);

	spr_inst_menuCampaing_info->spriteInstance->PlaceObject(0, true, "btnBack");
	auto spr_inst_menuCampaing_info_options = spr_inst_menuCampaing_info->spriteInstance->PlaceObject(1, true, "options");
	for (size_t i = 0; i < 8; ++i) {
		auto spr_inst_menuCampaing_info_options_item = spr_inst_menuCampaing_info_options->spriteInstance->PlaceObject(i, true, va("item%d", i));
		spr_inst_menuCampaing_info_options_item->spriteInstance->PlaceObject(0, false, "txtVal", Vector2(3.0f * i - 15.0f, -20.0f));
	}

	shortcutKeys = std::make_shared<idSWFScriptObject>();
	scriptFunction_shortcutKeys_clear.Bind(this);
	scriptFunction_shortcutKeys_clear.Call(shortcutKeys.get(), idSWFParmList());
	globals->Set("shortcutKeys", shortcutKeys);
}

/*
===================
idSWF::Activate
===================
*/
void idSWF::Activate(bool b) {
	isActive = b;
}

/*
===================
idSWF::idSWFScriptFunction_shortcutKeys_clear::Call
===================
*/
idSWFScriptVar idSWF::idSWFScriptFunction_shortcutKeys_clear::Call(idSWFScriptObject* thisObject, const idSWFParmList& parms) {
	idSWFScriptObject* object = pThis->shortcutKeys.get();
	object->Clear();
	//object->Set("clear", shared_from_this());
	object->Set("JOY1", "ENTER");
	object->Set("JOY2", "BACKSPACE");
	object->Set("JOY3", "START");
	object->Set("JOY5", "LB");
	object->Set("JOY6", "RB");
	object->Set("JOY9", "START");
	object->Set("JOY10", "BACKSPACE");
	object->Set("K_ENTER", "ENTER");
	object->Set("MWHEELDOWN", "MWHEEL_DOWN");
	object->Set("MWHEELUP", "MWHEEL_UP");
	object->Set("K_TAB", "TAB");


	// FIXME: I'm an RTARD and didn't realize the keys all have "ARROW" after them
	object->Set("LEFTARROW", "LEFT");
	object->Set("RIGHTARROW", "RIGHT");
	object->Set("UPARROW", "UP");
	object->Set("DOWNARROW", "DOWN");


	return idSWFScriptVar();
}
