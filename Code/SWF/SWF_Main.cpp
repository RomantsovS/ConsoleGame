#include "idlib/precompiled.h"

/*
===================
idSWF::idSWF
===================
*/
idSWF::idSWF(const std::string& filename_)
    : filename(filename_), isActive(false) {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "idSWF");
#endif  // DEBUG_PRINT_Ctor_Dtor
}

/*
===================
idSWF::~idSWF
===================
*/
idSWF::~idSWF() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  if (isCommonExists) common->DPrintf("%s dtor\n", "idSWF");
#endif  // DEBUG_PRINT_Ctor_Dtor
  globals = nullptr;
  shortcutKeys = nullptr;
}

/*
===================
idSWF::Init
===================
*/
void idSWF::Init() {
  globals = std::make_shared<idSWFScriptObject>();

  mainspriteInstance =
      std::make_shared<idSWFSpriteInstance>(shared_from_this());

  mainspriteInstance->Init();

  mainspriteInstance->name = "mainspriteInstance";
  mainspriteInstance->SetVisible(true);

  mainspriteInstance->PlaceObject(0, true, "menuStart");

  auto spr_inst_prompts = mainspriteInstance->PlaceObject(1, true, "prompts");
  auto spr_inst_prompts_joy1 = spr_inst_prompts->spriteInstance->PlaceObject(
      0, true, "joy1", Vector2(-60, 0));
  spr_inst_prompts_joy1->spriteInstance->PlaceObject(0, false, "txt_info");
  auto spr_inst_prompts_joy2 = spr_inst_prompts->spriteInstance->PlaceObject(
      1, true, "joy2", Vector2(0, -20));
  spr_inst_prompts_joy2->spriteInstance->PlaceObject(0, false, "txt_info");

  auto spr_inst_menuMain = mainspriteInstance->PlaceObject(2, true, "menuMain");
  auto spr_inst_menuMain_info =
      spr_inst_menuMain->spriteInstance->PlaceObject(0, true, "info");
  auto spr_inst_menuMain_options =
      spr_inst_menuMain_info->spriteInstance->PlaceObject(1, true, "options");
  for (size_t i = 0; i < 6; ++i)
    spr_inst_menuMain_options->spriteInstance->PlaceObject(i, true,
                                                           va("item%d", i));

  auto spr_inst_pcBar =
      mainspriteInstance->PlaceObject(3, true, "pcBar", Vector2(0, 5));
  auto spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      0, true, "btn0", Vector2(-100, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
  spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      1, true, "btn1", Vector2(-30, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
  spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      2, true, "btn2", Vector2(70, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
  spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      3, true, "btn3", Vector2(50, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
  spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      4, true, "btn4", Vector2(70, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");
  spr_inst_pcBar_btn = spr_inst_pcBar->spriteInstance->PlaceObject(
      5, true, "btn5", Vector2(80, 0));
  spr_inst_pcBar_btn->spriteInstance->PlaceObject(0, false, "txtVal");

  auto spr_inst_buttons = mainspriteInstance->PlaceObject(4, true, "buttons");
  spr_inst_buttons->spriteInstance->PlaceObject(0, true, "btnUp");
  spr_inst_buttons->spriteInstance->PlaceObject(1, true, "btnDown");
  spr_inst_buttons->spriteInstance->PlaceObject(2, true, "btnLeft");
  spr_inst_buttons->spriteInstance->PlaceObject(3, true, "btnRight");

  auto spr_inst_menuCampaing =
      mainspriteInstance->PlaceObject(5, true, "menuCampaign");
  auto spr_inst_menuCampaing_info =
      spr_inst_menuCampaing->spriteInstance->PlaceObject(0, true, "info");
  spr_inst_menuCampaing_info->spriteInstance->SetVisible(true);

  spr_inst_menuCampaing_info->spriteInstance->PlaceObject(0, true, "btnBack");
  auto spr_inst_menuCampaing_info_options =
      spr_inst_menuCampaing_info->spriteInstance->PlaceObject(1, true,
                                                              "options");
  for (size_t i = 0; i < 8; ++i) {
    auto spr_inst_menuCampaing_info_options_item =
        spr_inst_menuCampaing_info_options->spriteInstance->PlaceObject(
            i, true, va("item%d", i));
    spr_inst_menuCampaing_info_options_item->spriteInstance->PlaceObject(
        0, false, "txtVal", Vector2(-60.0f, 10.0f * i - 15.0f));
  }

  auto spr_inst_menuNewGame =
      mainspriteInstance->PlaceObject(6, true, "menuNewGame");
  auto spr_inst_menuNewGame_info =
      spr_inst_menuNewGame->spriteInstance->PlaceObject(0, true, "info");
  spr_inst_menuNewGame_info->spriteInstance->SetVisible(true);
  spr_inst_menuNewGame_info->spriteInstance->PlaceObject(0, true, "btnBack");
  auto spr_inst_menuNewGame_info_options =
      spr_inst_menuNewGame_info->spriteInstance->PlaceObject(1, true,
                                                             "options");
  for (size_t i = 0; i < 8; ++i) {
    auto spr_inst_menuNewGame_info_options_item =
        spr_inst_menuNewGame_info_options->spriteInstance->PlaceObject(
            i, true, va("item%d", i));
    spr_inst_menuNewGame_info_options_item->spriteInstance->PlaceObject(
        0, false, "txtVal", Vector2(-60.0f, 10.0f * i - 15.0f));
  }

  auto spr_inst_menuPause =
      mainspriteInstance->PlaceObject(7, true, "menuPause");
  auto spr_inst_menuPause_info =
      spr_inst_menuPause->spriteInstance->PlaceObject(0, true, "info");
  spr_inst_menuPause_info->spriteInstance->SetVisible(true);
  auto spr_inst_menuPause_info_options =
      spr_inst_menuPause_info->spriteInstance->PlaceObject(1, true, "options");
  for (size_t i = 0; i < 8; ++i) {
    auto spr_inst_menuPause_info_options_item =
        spr_inst_menuPause_info_options->spriteInstance->PlaceObject(
            i, true, va("item%d", i));
    spr_inst_menuPause_info_options_item->spriteInstance->PlaceObject(
        0, false, "txtVal", Vector2(-60.0f, 10.0f * i - 15.0f));
  }

  auto spr_inst_menuPartyLobby =
      mainspriteInstance->PlaceObject(8, true, "menuPartyLobby");
  auto spr_inst_PartyLobby_info =
      spr_inst_menuPartyLobby->spriteInstance->PlaceObject(0, true, "info");
  spr_inst_PartyLobby_info->spriteInstance->SetVisible(true);

  spr_inst_PartyLobby_info->spriteInstance->PlaceObject(0, true, "btnBack");
  auto spr_inst_menuPartyLobby_info_options =
      spr_inst_PartyLobby_info->spriteInstance->PlaceObject(1, true, "options");
  for (size_t i = 0; i < 8; ++i) {
    auto spr_inst_menuPartyLobby_info_options_item =
        spr_inst_menuPartyLobby_info_options->spriteInstance->PlaceObject(
            i, true, va("item%d", i));
    spr_inst_menuPartyLobby_info_options_item->spriteInstance->PlaceObject(
        0, false, "txtVal", Vector2(-60.0f, 10.0f * i - 15.0f));
  }
  auto spr_inst_PartyLobby_options =
      spr_inst_menuPartyLobby->spriteInstance->PlaceObject(1, true, "options");

  auto spr_inst_menuGameLobby =
      mainspriteInstance->PlaceObject(9, true, "menuGameLobby");
  auto spr_inst_GameLobby_info =
      spr_inst_menuGameLobby->spriteInstance->PlaceObject(0, true, "info");
  spr_inst_GameLobby_info->spriteInstance->SetVisible(true);

  spr_inst_GameLobby_info->spriteInstance->PlaceObject(0, true, "btnBack");
  auto spr_inst_menuGameLobby_info_options =
      spr_inst_GameLobby_info->spriteInstance->PlaceObject(1, true, "options");
  for (size_t i = 0; i < 8; ++i) {
    auto spr_inst_menuGameLobby_info_options_item =
        spr_inst_menuGameLobby_info_options->spriteInstance->PlaceObject(
            i, true, va("item%d", i));
    spr_inst_menuGameLobby_info_options_item->spriteInstance->PlaceObject(
        0, false, "txtVal", Vector2(-60.0f, 10.0f * i - 15.0f));
  }

  auto spr_inst_GameLobby_match_info =
      spr_inst_menuGameLobby->spriteInstance->PlaceObject(
          1, true, "matchInfo", Vector2(0.0f, -30.0f));
  spr_inst_GameLobby_match_info->spriteInstance->SetVisible(true);
  auto spr_inst_GameLobby_match_info_txtMapName =
      spr_inst_GameLobby_match_info->spriteInstance->PlaceObject(0, false,
                                                                 "txtMapName");

#ifdef DEBUG
  shortcutKeys =
      std::shared_ptr<idSWFScriptObject>(DBG_NEW idSWFScriptObject());
#else
  shortcutKeys = std::make_shared<idSWFScriptObject>();
#endif
  scriptFunction_shortcutKeys_clear.Bind(this);
  scriptFunction_shortcutKeys_clear.Call(shortcutKeys.get(), idSWFParmList());
  globals->Set("shortcutKeys", shortcutKeys);

  // mainspriteInstance->RunTo(0);
}

/*
===================
idSWF::Activate
===================
*/
void idSWF::Activate(bool b) noexcept {
  if (!isActive && b) {
    // mainspriteInstance->FreeDisplayList();
  }
  isActive = b;
}

/*
===================
idSWF::idSWFScriptFunction_shortcutKeys_clear::Call
===================
*/
idSWFScriptVar idSWF::idSWFScriptFunction_shortcutKeys_clear::Call(
    idSWFScriptObject* thisObject, const idSWFParmList& parms) {
  gsl::not_null<idSWFScriptObject*> object = pThis->shortcutKeys.get();
  object->Clear();
  // object->Set("clear", shared_from_this());
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
