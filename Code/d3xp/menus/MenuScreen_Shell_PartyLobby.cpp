#include "idlib/precompiled.h"
#include "../Game_local.h"

const static int NUM_LOBBY_OPTIONS = 8;

extern idCVar si_map;

enum class partyLobbyCmds_t {
	PARTY_CMD_QUICK,
	PARTY_CMD_FIND,
	PARTY_CMD_CREATE,
	PARTY_CMD_PWF,
	PARTY_CMD_INVITE,
	PARTY_CMD_LEADERBOARDS,
	PARTY_CMD_TOGGLE_PRIVACY,
	PARTY_CMD_SHOW_PARTY_GAMES,
};

/*
========================
idMenuScreen_Shell_PartyLobby::Initialize
========================
*/
void idMenuScreen_Shell_PartyLobby::Initialize(std::shared_ptr<idMenuHandler> data) noexcept {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuPartyLobby");

	options = std::make_shared<idMenuWidget_DynamicList>();
	options->SetNumVisibleOptions(NUM_LOBBY_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);
	AddChild(options);

	while (options->GetChildren().size() < NUM_LOBBY_OPTIONS) {
		std::shared_ptr<idMenuWidget_Button> buttonWidget = std::make_shared<idMenuWidget_Button>();
		buttonWidget->Initialize(data);
		//buttonWidget->RegisterEventObserver(helpWidget);
		options->AddChild(buttonWidget);
	}
	options->Initialize(data);


	btnBack = std::make_shared<idMenuWidget_Button>();
	btnBack->Initialize(data);
	btnBack->SetLabel("MAIN MENU");
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_GO_BACK);
	AddChild(btnBack);

	lobby = std::make_unique<idMenuWidget_LobbyList>();
	lobby->SetNumVisibleOptions(8);
	lobby->SetSpritePath(GetSpritePath(), "options");
	lobby->SetWrappingAllowed(true);
	lobby->Initialize(data);
	/*while (lobby->GetChildren().size() < 8) {
		std::shared_ptr<idMenuWidget_LobbyButton> buttonWidget = std::make_shared<idMenuWidget_LobbyButton>();
		buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_SELECT_GAMERTAG, lobby->GetChildren().Num());
		buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_COMMAND).Set(WIDGET_ACTION_MUTE_PLAYER, lobby->GetChildren().Num());
		buttonWidget->Initialize(data);
		lobby->AddChild(buttonWidget);
	}
	AddChild(lobby);*/

	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_UP));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE));
}

/*
========================
idMenuScreen_Shell_PartyLobby::Update
========================
*/
void idMenuScreen_Shell_PartyLobby::Update() noexcept {

	/*idLobbyBase& activeLobby = session->GetPartyLobbyBase();
	if (lobby != NULL) {
		if (activeLobby.GetNumActiveLobbyUsers() != 0) {
			if (lobby->GetFocusIndex() >= activeLobby.GetNumActiveLobbyUsers()) {
				lobby->SetFocusIndex(activeLobby.GetNumActiveLobbyUsers() - 1);
				lobby->SetViewIndex(lobby->GetViewOffset() + lobby->GetFocusIndex());
			}
		}
	}*/

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();
	if (BindSprite(root.get())) {
		std::shared_ptr<idSWFTextInstance> heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading) {
			heading->SetText("MULTIPLAYER");	// MULTIPLAYER
		}
	}

	UpdateOptions();

	if (auto spMenuData = menuData.lock()) {
		if (spMenuData->NextScreen() == static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY)) {
			std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
			if (cmdBar) {
				cmdBar->ClearAllButtons();
				idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
				buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
				buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_GO_BACK);

				buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
				buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);
			}
		}
	}

	if (btnBack) {
		btnBack->BindSprite(root.get());
	}

	idMenuScreen::Update();
}

void idMenuScreen_Shell_PartyLobby::UpdateOptions() {

	bool forceUpdate = false;


	if (/*(session->GetPartyLobbyBase().IsHost() && (!isHost || forceUpdate)) &&*/ options) {

		menuOptions.clear();
		std::vector<std::string> option;

		isHost = true;
		isPeer = false;

		option.push_back("Find Match");	// Find Match
		menuOptions.push_back(option);
		option.clear();
		option.push_back("CreateMatch");	// Create Match
		menuOptions.push_back(option);
		option.clear();

		int index = 0;
		options->GetChildByIndex(index)->ClearEventActions();
		options->GetChildByIndex(index)->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND,
			static_cast<int>(partyLobbyCmds_t::PARTY_CMD_FIND), index);
		auto buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));
		if (buttonWidget) {
			//buttonWidget->SetDescription("find match");
		}
		index++;
		options->GetChildByIndex(index)->ClearEventActions();
		options->GetChildByIndex(index)->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND,
			static_cast<int>(partyLobbyCmds_t::PARTY_CMD_CREATE), index);
		buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));
		if (buttonWidget) {
			//buttonWidget->SetDescription("#str_swf_create_desc");
		}

		options->SetListData(menuOptions);

	}

	if (forceUpdate) {
		options->Update();
	}

}

/*
========================
idMenuScreen_Shell_PartyLobby::ShowScreen
========================
*/
void idMenuScreen_Shell_PartyLobby::ShowScreen() {

	isPeer = false;
	isHost = false;

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();
	if (BindSprite(root.get())) {
		/*idSWFSpriteInstance* waitTime = GetSprite()->GetScriptObject()->GetNestedSprite("waitTime");
		if (waitTime != NULL) {
			waitTime->SetVisible(false);
		}*/
	}

	if (session->GetPartyLobbyBase().IsHost()) {
		/*idMatchParameters matchParameters = session->GetPartyLobbyBase().GetMatchParms();
		if (net_inviteOnly.GetBool()) {
			matchParameters.matchFlags |= MATCH_INVITE_ONLY;
		}
		else {
			matchParameters.matchFlags &= ~MATCH_INVITE_ONLY;
		}

		matchParameters.numSlots = session->GetTitleStorageInt("MAX_PLAYERS_ALLOWED", 4);

		session->UpdatePartyParms(matchParameters);*/
	}

	idMenuScreen::ShowScreen();
	if (lobby) {
		lobby->SetFocusIndex(0);
	}
}

/*
========================
idMenuScreen_Shell_PartyLobby::HideScreen
========================
*/
void idMenuScreen_Shell_PartyLobby::HideScreen() {
	idMenuScreen::HideScreen();
}

/*
========================
idMenuScreen_Shell_PartyLobby::HandleAction
========================
*/
bool idMenuScreen_Shell_PartyLobby::HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled) {

	auto spMenuData = menuData.lock();
	if (!spMenuData) {
		return false;
	}

	if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY)) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
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

		switch (static_cast<partyLobbyCmds_t>(parms[0]->ToInteger())) {
		case partyLobbyCmds_t::PARTY_CMD_FIND: {
			idMatchParameters matchParameters = idMatchParameters(session->GetPartyLobbyBase().GetMatchParms());
			matchParameters.gameMap = GAME_MAP_RANDOM;

			//session->UpdatePartyParms(matchParameters);

			// Update flags for game lobby.
			matchParameters.matchFlags = DefaultPartyFlags | DefaultPublicGameFlags;

			session->FindOrCreateMatch(matchParameters);

			break;
		}
		case partyLobbyCmds_t::PARTY_CMD_CREATE: {
			idMatchParameters matchParameters = idMatchParameters(session->GetPartyLobbyBase().GetMatchParms());

			//const bool isInviteOnly = MatchTypeInviteOnly(matchParameters.matchFlags);

			matchParameters.matchFlags = DefaultPartyFlags | DefaultPrivateGameFlags;

			//int mode = idMath::ClampInt(-1, GAME_COUNT - 1, si_mode.GetInteger());
			const auto& maps = common->GetMapList();
			int map = idMath::ClampInt(-1, maps.size() - 1, si_map.GetInteger());

			matchParameters.gameMap = map;
			//matchParameters.gameMode = mode;
			//cvarSystem->MoveCVarsToDict(CVAR_SERVERINFO, matchParameters.serverInfo);
			session->CreateMatch(matchParameters);
			break;
		}
		}

		return true;
	}
	case widgetAction_t::WIDGET_ACTION_START_REPEATER: {

		if (!options) {
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
idMenuScreen_Shell_PartyLobby::UpdateLobby
========================
*/
void idMenuScreen_Shell_PartyLobby::UpdateLobby() {

	auto spMenuData = menuData.lock();

	if (spMenuData && spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY)) {
		return;
	}

	// Keep this menu in sync with the session host/peer status.
	if (session->GetPartyLobbyBase().IsHost() && !isHost) {
		Update();
	}

	if (session->GetPartyLobbyBase().IsPeer() && !isPeer) {
		Update();
	}

	if (isPeer) {
		Update();
	}

	UpdateOptions();

	// setup names for lobby;
	if (lobby) {
		std::shared_ptr<idMenuHandler_Shell> mgr = std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
		if (mgr) {
			mgr->UpdateLobby(lobby.get());
			lobby->Update();
		}

		if (lobby->GetNumEntries() > 0 && lobby->GetFocusIndex() >= lobby->GetNumEntries()) {
			lobby->SetFocusIndex(lobby->GetNumEntries() - 1);
			lobby->SetViewIndex(lobby->GetNumEntries() - 1);
		}
	}

	if (session->GetState() == idSession::sessionState_t::PARTY_LOBBY) {

		if (options) {
			if (options->GetFocusIndex() >= options->GetTotalNumberOfOptions() && options->GetTotalNumberOfOptions() > 0) {
				options->SetViewIndex(options->GetTotalNumberOfOptions() - 1);
				options->SetFocusIndex(options->GetTotalNumberOfOptions() - 1);
			}
		}

		/*std::shared_ptr<idSWFTextInstance> privacy = GetSprite()->GetScriptObject()->GetNestedText("matchInfo", "txtPrivacy");
		if (privacy) {
			if (isPeer) {
				privacy->SetText("");
			}
			else {

				idMatchParameters matchParameters = session->GetPartyLobbyBase().GetMatchParms();
				int bitSet = (matchParameters.matchFlags & MATCH_INVITE_ONLY);
				bool privacySet = (bitSet != 0 ? true : false);
				if (privacySet) {
					privacy->SetText("#str_swf_privacy_closed");
					privacy->SetStrokeInfo(true);
				}
				else if (!privacySet) {
					privacy->SetText("#str_swf_privacy_open");
					privacy->SetStrokeInfo(true);
				}
			}
		}*/

		//idLocalUser* user = session->GetSignInManager().GetMasterLocalUser();
		//if (user != NULL && options != NULL) {
		//	if (user->IsInParty() && user->GetPartyCount() > 1 && !session->IsPlatformPartyInLobby() && menuOptions.Num() > 0) {
		//		if (menuOptions[menuOptions.Num() - 1][0] != "#str_swf_invite_xbox_live_party") {
		//			menuOptions[menuOptions.Num() - 1][0] = "#str_swf_invite_xbox_live_party";	// invite Xbox LIVE party
		//			options->SetListData(menuOptions);
		//			options->Update();
		//		}
		//	}
		//	else if (menuOptions.Num() > 0) {
		//		if (menuOptions[menuOptions.Num() - 1][0] != "#str_swf_invite_friends") {
		//			menuOptions[menuOptions.Num() - 1][0] = "#str_swf_invite_friends";	// invite Xbox LIVE party
		//			options->SetListData(menuOptions);
		//			options->Update();
		//		}
		//	}
		//}
	}
}