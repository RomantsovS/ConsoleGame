#include "idlib/precompiled.h"
#include "../Game_local.h"

const static int NUM_LOBBY_OPTIONS = 8;

enum class gameLobbyCmd_t {
  GAME_CMD_START,
  GAME_CMD_INVITE,
  GAME_CMD_SETTINGS,
  GAME_CMD_TOGGLE_PRIVACY,
};

/*
========================
idMenuScreen_Shell_GameLobby::Initialize
========================
*/
void idMenuScreen_Shell_GameLobby::Initialize(
    std::shared_ptr<idMenuHandler> data) noexcept {
  idMenuScreen::Initialize(data);

  if (data) {
    menuGUI = data->GetGUI();
  }

  SetSpritePath("menuGameLobby");

  options = std::make_shared<idMenuWidget_DynamicList>();
  options->SetNumVisibleOptions(NUM_LOBBY_OPTIONS);
  options->SetSpritePath(GetSpritePath(), "info", "options");
  options->SetWrappingAllowed(true);
  AddChild(options);

  while (options->GetChildren().size() < NUM_LOBBY_OPTIONS) {
    std::shared_ptr<idMenuWidget_Button> const buttonWidget =
        std::make_shared<idMenuWidget_Button>();
    buttonWidget->Initialize(data);
    options->AddChild(buttonWidget);
  }
  options->Initialize(data);

  lobby = std::make_unique<idMenuWidget_LobbyList>();
  lobby->SetNumVisibleOptions(8);
  lobby->SetSpritePath(GetSpritePath(), "options");
  lobby->SetWrappingAllowed(true);
  lobby->Initialize(data);

  btnBack = std::make_shared<idMenuWidget_Button>();
  btnBack->Initialize(data);
  btnBack->SetLabel("#str_swf_multiplayer");
  btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
  btnBack->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS)
      .Set(widgetAction_t::WIDGET_ACTION_GO_BACK);
  AddChild(btnBack);

  AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN)
      .Set(std::make_shared<idWidgetActionHandler>(
          options,
          actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER,
          widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN));
  AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP)
      .Set(std::make_shared<idWidgetActionHandler>(
          options,
          actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER,
          widgetEvent_t::WIDGET_EVENT_SCROLL_UP));
  AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE)
      .Set(std::make_shared<idWidgetActionHandler>(
          options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
          widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE));
  AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE)
      .Set(std::make_shared<idWidgetActionHandler>(
          options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
          widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE));
}

/*
========================
idMenuScreen_Shell_GameLobby::Update
========================
*/
void idMenuScreen_Shell_GameLobby::Update() noexcept {
  /*idLobbyBase& activeLobby = session->GetActivePlatformLobbyBase();
  if (lobby) {

          if (activeLobby.GetNumActiveLobbyUsers() != 0) {
                  if (lobby->GetFocusIndex() >=
  activeLobby.GetNumActiveLobbyUsers()) {
                          lobby->SetFocusIndex(activeLobby.GetNumActiveLobbyUsers()
  - 1); lobby->SetViewIndex(lobby->GetViewOffset() + lobby->GetFocusIndex());
                  }
          }
  }*/

  std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();
  if (BindSprite(root.get())) {
    std::shared_ptr<idSWFTextInstance> heading =
        GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
    if (heading) {
      heading->SetText("MULTIPLAYER");  // MULTIPLAYER
    }
  }

  if (privateGameLobby && options) {
    if (session->GetActivePlatformLobbyBase().IsHost() && !isHost) {
      menuOptions.clear();
      std::vector<std::string> option;

      isHost = true;
      isPeer = false;

      option.push_back("Start match");  // Start match
      menuOptions.push_back(option);
      option.clear();

      int index = 0;
      options->GetChildByIndex(index)->ClearEventActions();
      options->GetChildByIndex(index)
          ->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS)
          .Set(widgetAction_t::WIDGET_ACTION_COMMAND,
               static_cast<int>(gameLobbyCmd_t::GAME_CMD_START), 0);
      index++;

      options->SetListData(menuOptions);

    } else if (session->GetActivePlatformLobbyBase().IsPeer()) {
      if (!isPeer) {
        menuOptions.clear();
        std::vector<std::string> option;

        options->SetListData(menuOptions);
      }

      isPeer = true;
      isHost = false;
    }
  }

  if (auto spMenuData = menuData.lock()) {
    std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
    if (cmdBar) {
      cmdBar->ClearAllButtons();
      idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
      buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
      buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_GO_BACK);

      buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
      buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);

      /*lobbyUserID_t luid;
      if (isHost && CanKickSelectedPlayer(luid)) {
              buttonInfo =
      cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY4); buttonInfo->label
      = "#str_swf_kick";
              buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_JOY4_ON_PRESS);
      }*/
    }
  }

  if (btnBack) {
    btnBack->BindSprite(root.get());
  }

  idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_GameLobby::ShowScreen
========================
*/
void idMenuScreen_Shell_GameLobby::ShowScreen() {
  if (options) {
    options->SetFocusIndex(0);
    options->SetViewIndex(0);
  }

  isHost = false;
  isPeer = false;

  idMatchParameters matchParameters =
      session->GetActivePlatformLobbyBase().GetMatchParms();

  // Make sure map name is up to date.
  if (matchParameters.gameMap >= 0) {
    matchParameters.mapName =
        common->GetMapList()[matchParameters.gameMap].mapFile;
  }

  // privateGameLobby = MatchTypeIsPrivate(matchParameters.matchFlags);

  if (!privateGameLobby) {  // Public Game Lobby
    menuOptions.clear();
    std::vector<std::string> option;

    if (options) {
      option.push_back("#str_swf_invite_friends");  // Invite Friends
      menuOptions.push_back(option);
      option.clear();

      int index = 0;
      options->GetChildByIndex(index)->ClearEventActions();
      options->GetChildByIndex(index)
          ->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS)
          .Set(widgetAction_t::WIDGET_ACTION_COMMAND,
               static_cast<int>(gameLobbyCmd_t::GAME_CMD_INVITE), 0);

      options->SetListData(menuOptions);
    }

    /*longCountdown = Sys_Milliseconds() + WAIT_START_TIME_LONG;
    longCountRemaining = longCountdown;
    shortCountdown = Sys_Milliseconds() + WAIT_START_TIME_SHORT;*/
  }

  std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();
  if (BindSprite(root.get())) {
    /*idSWFSpriteInstance* waitTime =
    GetSprite()->GetScriptObject()->GetNestedSprite("waitTime"); if (waitTime !=
    NULL) { waitTime->SetVisible(!privateGameLobby);
    }*/
  }

  idMenuScreen::ShowScreen();

  if (lobby) {
    lobby->SetFocusIndex(0);
  }

  // session->UpdateMatchParms(matchParameters);
}

/*
========================
idMenuScreen_Shell_GameLobby::HideScreen
========================
*/
void idMenuScreen_Shell_GameLobby::HideScreen() { idMenuScreen::HideScreen(); }

/*
========================
idMenuScreen_Shell_GameLobby::CanKickSelectedPlayer
========================
*/
// bool idMenuScreen_Shell_GameLobby::CanKickSelectedPlayer(lobbyUserID_t& luid)
// {
//
//	idMatchParameters matchParameters =
// session->GetActivePlatformLobbyBase().GetMatchParms(); 	const int
// playerId = lobby->GetFocusIndex();
//
//	// can't kick yourself
//	idLobbyBase& activeLobby = session->GetActivePlatformLobbyBase();
//	luid = activeLobby.GetLobbyUserIdByOrdinal(playerId);
//	if (session->GetSignInManager().GetMasterLocalUser() ==
// activeLobby.GetLocalUserFromLobbyUser(luid)) { 		return false;
//	}
//
//	return true;
// }

/*
========================
idMenuScreen_Shell_GameLobby::HandleAction h
========================
*/
bool idMenuScreen_Shell_GameLobby::HandleAction(idWidgetAction& action,
                                                const idWidgetEvent& event,
                                                idMenuWidget* widget,
                                                bool forceHandled) {
  auto spMenuData = menuData.lock();
  if (!spMenuData) {
    return false;
  }

  if (spMenuData->ActiveScreen() !=
      static_cast<int>(shellAreas_t::SHELL_AREA_GAME_LOBBY)) {
    return false;
  }

  widgetAction_t actionType = action.GetType();
  const idSWFParmList& parms = action.GetParms();

  switch (actionType) {
    /*case widgetAction_t::WIDGET_ACTION_JOY4_ON_PRESS: {
            idLobbyBase& activeLobby = session->GetActivePlatformLobbyBase();
            lobbyUserID_t luid;
            if (CanKickSelectedPlayer(luid)) {
                    activeLobby.KickLobbyUser(luid);
            }
            return true;
    }*/
    case widgetAction_t::WIDGET_ACTION_GO_BACK: {
      session->Cancel();

      return true;
    }
    case widgetAction_t::WIDGET_ACTION_COMMAND: {
      if (!options) {
        return true;
      }

      int selectionIndex = options->GetFocusIndex();
      if (parms.size() > 1) {
        selectionIndex = parms[1]->ToInteger();
      }

      if (selectionIndex != options->GetFocusIndex()) {
        options->SetViewIndex(options->GetViewOffset() + selectionIndex);
        options->SetFocusIndex(selectionIndex);
      }

      switch (static_cast<gameLobbyCmd_t>(parms[0]->ToInteger())) {
        case gameLobbyCmd_t::GAME_CMD_START: {
          std::shared_ptr<idMenuHandler_Shell> handler =
              std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
          if (handler) {
            handler->SetTimeRemaining(0);
          }
          break;
        }
      }
      return true;
    }
    case widgetAction_t::WIDGET_ACTION_START_REPEATER: {
      if (options) {
        return true;
      }

      if (parms.size() == 4) {
        int selectionIndex = parms[3]->ToInteger();
        if (selectionIndex != options->GetFocusIndex()) {
          options->SetViewIndex(options->GetViewOffset() + selectionIndex);
          options->SetFocusIndex(selectionIndex);
        }
      }
      break;
    }
  }

  return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuScreen_Shell_GameLobby::UpdateLobby
========================
*/
void idMenuScreen_Shell_GameLobby::UpdateLobby() {
  auto spMenuData = menuData.lock();

  if (spMenuData && spMenuData->ActiveScreen() !=
                        static_cast<int>(shellAreas_t::SHELL_AREA_GAME_LOBBY)) {
    return;
  }

  // Keep this menu in sync with the session host/peer status.
  if (session->GetActivePlatformLobbyBase().IsHost() && !isHost) {
    Update();
  }

  if (session->GetActivePlatformLobbyBase().IsPeer() && !isPeer) {
    Update();
  }

  if (!privateGameLobby) {
    int ms = 0;
    if (session->GetActivePlatformLobbyBase().IsHost()) {
      std::shared_ptr<idMenuHandler_Shell> handler =
          std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
      if (handler) {
        /*if (session->GetActivePlatformLobbyBase().IsLobbyFull()) {
                longCountdown = Sys_Milliseconds() + longCountRemaining;
                int timeRemaining = shortCountdown - Sys_Milliseconds();
                if (timeRemaining < 0) {
                        timeRemaining = 0;
                }
                ms = (int)ceilf(timeRemaining / 1000.0f);
                handler->SetTimeRemaining(timeRemaining);
        }
        else if (session->GetActivePlatformLobbyBase().GetNumLobbyUsers() > 1) {
                int timeRemaining = longCountdown - Sys_Milliseconds();
                if (timeRemaining > WAIT_START_TIME_SHORT) {
                        shortCountdown = Sys_Milliseconds() +
        WAIT_START_TIME_SHORT;
                }
                else {
                        shortCountdown = timeRemaining;
                }
                longCountRemaining = timeRemaining;
                if (timeRemaining < 0) {
                        timeRemaining = 0;
                }
                ms = (int)ceilf(timeRemaining / 1000.0f);
                handler->SetTimeRemaining(timeRemaining);
        }
        else {
                ms = 0;
                longCountdown = Sys_Milliseconds() + WAIT_START_TIME_LONG;
                longCountRemaining = longCountdown;
                shortCountdown = Sys_Milliseconds() + WAIT_START_TIME_SHORT;
                handler->SetTimeRemaining(longCountRemaining);
        }*/
      }
    } else {
      if (spMenuData) {
        std::shared_ptr<idMenuHandler_Shell> handler =
            std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
        if (handler) {
          // ms = (int)ceilf(handler->GetTimeRemaining() / 1000.0f);
        }
      }
    }

    /*idSWFScriptObject& root = GetSWFObject()->GetRootObject();
    if (BindSprite(root)) {
            idSWFTextInstance* waitTime =
    GetSprite()->GetScriptObject()->GetNestedText("waitTime", "txtVal"); if
    (waitTime != NULL) { idStr status; if (ms == 1) { status =
    idLocalization::GetString("#str_online_game_starts_in_second");
                            status.Replace("<DNT_VAL>", idStr(ms));
                            waitTime->SetText(status);
                    }
                    else if (ms > 0 && ms < 30) {
                            status =
    idLocalization::GetString("#str_online_game_starts_in_seconds");
                            status.Replace("<DNT_VAL>", idStr(ms));
                            waitTime->SetText(status);
                    }
                    else {
                            waitTime->SetText("");
                    }
                    waitTime->SetStrokeInfo(true, 0.75f, 2.0f);
            }
    }*/
    Update();

  } else {
    if (isPeer) {
      Update();
    }
  }

  if (session->GetState() == idSession::sessionState_t::GAME_LOBBY) {
    if (options) {
      if (options->GetFocusIndex() >= options->GetTotalNumberOfOptions() &&
          options->GetTotalNumberOfOptions() > 0) {
        options->SetViewIndex(options->GetTotalNumberOfOptions() - 1);
        options->SetFocusIndex(options->GetTotalNumberOfOptions() - 1);
      }
    }

    idMatchParameters matchParameters =
        session->GetActivePlatformLobbyBase().GetMatchParms();

    std::shared_ptr<idSWFTextInstance> mapName =
        GetSprite()->GetScriptObject()->GetNestedText("matchInfo",
                                                      "txtMapName");
    // idSWFTextInstance* modeName =
    // GetSprite()->GetScriptObject()->GetNestedText("matchInfo",
    // "txtModeName");

    if (mapName) {
      const auto& maps = common->GetMapList();
      std::string name =
          maps[idMath::ClampInt(0, maps.size() - 1, matchParameters.gameMap)]
              .mapName;
      mapName->SetText(name);
    }

    // if (modeName != NULL) {
    //	const idStrList& modes = common->GetModeDisplayList();
    //	idStr mode = idLocalization::GetString(modes[idMath::ClampInt(0,
    // modes.Num() - 1, matchParameters.gameMode)]); 	modeName->SetText(mode);
    //	modeName->SetStrokeInfo(true);
    // }

    // idSWFTextInstance* privacy =
    // GetSprite()->GetScriptObject()->GetNestedText("matchInfo", "txtPrivacy");
    // if (privacy != NULL) {
    //	if (isPeer || !privateGameLobby) {
    //		privacy->SetText("");
    //	}
    //	else {
    //		int bitSet = (matchParameters.matchFlags & MATCH_INVITE_ONLY);
    //		bool privacySet = (bitSet != 0 ? true : false);
    //		if (privacySet) {
    //			privacy->SetText("#str_swf_privacy_closed");
    //			privacy->SetStrokeInfo(true);
    //		}
    //		else if (!privacySet) {
    //			privacy->SetText("#str_swf_privacy_open");
    //			privacy->SetStrokeInfo(true);
    //		}
    //	}
    // }

    // idLocalUser* user = session->GetSignInManager().GetMasterLocalUser();
    // if (user != NULL && options != NULL) {
    //	if (user->IsInParty() && user->GetPartyCount() > 1 &&
    //! session->IsPlatformPartyInLobby() && menuOptions.Num() > 0) { if
    //(menuOptions[menuOptions.Num() - 1][0] !=
    //"#str_swf_invite_xbox_live_party") {
    // menuOptions[menuOptions.Num() - 1][0] =
    // "#str_swf_invite_xbox_live_party";
    //// invite Xbox LIVE party
    ///options->SetListData(menuOptions);
    //			options->Update();
    //		}
    //	}
    //	else if (menuOptions.Num() > 0) {
    //		if (menuOptions[menuOptions.Num() - 1][0] !=
    //"#str_swf_invite_friends") {
    // menuOptions[menuOptions.Num() - 1][0] =
    //"#str_swf_invite_friends";	// invite Xbox LIVE party
    //			options->SetListData(menuOptions);
    //			options->Update();
    //		}
    //	}
    // }
  }

  // setup names for lobby;
  if (lobby) {
    /*idMenuHandler_Shell* mgr = dynamic_cast<idMenuHandler_Shell*>(menuData);
    if (mgr != NULL) {
            mgr->UpdateLobby(lobby);
            lobby->Update();
    }

    if (lobby->GetNumEntries() > 0 && lobby->GetFocusIndex() >=
    lobby->GetNumEntries()) { lobby->SetFocusIndex(lobby->GetNumEntries() - 1);
            lobby->SetViewIndex(lobby->GetNumEntries() - 1);
    }*/
  }
}
