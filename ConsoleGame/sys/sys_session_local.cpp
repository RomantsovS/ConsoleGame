#include "sys_session_local.h"
#include "../framework/Common.h"

/*
========================
idSessionLocal::idSessionLocal
========================
*/
idSessionLocal::idSessionLocal() {
	InitBaseState();
}

/*
========================
idSessionLocal::idSessionLocal
========================
*/
idSessionLocal::~idSessionLocal() {
}

/*
========================
idSessionLocal::InitBaseState
========================
*/
void idSessionLocal::InitBaseState() {
	localState = state_t::STATE_PRESS_START;
}

/*
========================
idSessionLocal::~idSession
========================
*/
idSession::~idSession() {
}

/*
========================
idSessionLocal::Initialize
========================
*/
void idSessionLocal::Initialize() {
}

/*
========================
idSessionLocal::Shutdown
========================
*/
void idSessionLocal::Shutdown() {
}

/*
========================
idSessionLocal::GetState
========================
*/
idSessionLocal::sessionState_t idSessionLocal::GetState() const {
	// Convert our internal state to one of the external states
	switch (localState) {
	case state_t::STATE_PRESS_START:						return sessionState_t::PRESS_START;
	case state_t::STATE_IDLE:								return sessionState_t::IDLE;
	case state_t::STATE_PARTY_LOBBY_HOST:					return sessionState_t::PARTY_LOBBY;
	case state_t::STATE_PARTY_LOBBY_PEER:					return sessionState_t::PARTY_LOBBY;
	case state_t::STATE_GAME_LOBBY_HOST:					return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_LOBBY_PEER:					return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_STATE_LOBBY_HOST:				return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_STATE_LOBBY_PEER:				return sessionState_t::GAME_LOBBY;
	case state_t::STATE_LOADING:							return sessionState_t::LOADING;
	case state_t::STATE_INGAME:								return sessionState_t::INGAME;
	case state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY:		return sessionState_t::CONNECTING;
	case state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY:		return sessionState_t::CONNECTING;
	case state_t::STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY:return sessionState_t::CONNECTING;
	case state_t::STATE_FIND_OR_CREATE_MATCH:				return sessionState_t::SEARCHING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_PARTY:			return sessionState_t::CONNECTING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_GAME:			return sessionState_t::CONNECTING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE:		return sessionState_t::CONNECTING;
	case state_t::STATE_BUSY:								return sessionState_t::BUSY;
	default: {
		common->Error("GetState: Unknown state in idSessionLocal");
	}
	};

	return sessionState_t::MAX_STATES;
}

// idSession interface

/*
========================
idSessionLocal::LoadingFinished

Only called by idCommonLocal::FinalizeMapChange
========================
*/
void idSessionLocal::LoadingFinished() {
	//assert(GetState() == idSession::LOADING);

	SetState(state_t::STATE_INGAME);
}

/*
========================
idSessionLocal::SetState
========================
*/
void idSessionLocal::SetState(state_t newState) {
	//assert(newState < NUM_STATES);
	//assert(localState < NUM_STATES);

	localState = newState;
}

