#include "idlib/precompiled.h"
#include "sys_lobby_backend.h"
#include "sys_lobby_backend_direct.h"

extern idCVar net_port;
idCVar host_net_ip("host_net_ip", "localhost", 0, "local IP address");

extern idLobbyToSessionCB* lobbyToSessionCB;

/*
========================
idLobbyBackendDirect::idLobbyBackendDirect
========================
*/
idLobbyBackendDirect::idLobbyBackendDirect() {
	state = lobbyBackendState_t::STATE_INVALID;
}

/*
========================
idLobbyBackendDirect::StartHosting
========================
*/
void idLobbyBackendDirect::StartHosting(const idMatchParameters& p, float skillLevel, lobbyBackendType_t type) {
	NET_VERBOSE_PRINT("idLobbyBackendDirect::StartHosting\n");

	isLocal = true;// MatchTypeIsLocal(p.matchFlags);
	isHost = true;

	state = lobbyBackendState_t ::STATE_READY;
	isLocal = true;
}

/*
========================
idLobbyBackendDirect::StartFinding
========================
*/
void idLobbyBackendDirect::StartFinding(const idMatchParameters& p, int numPartyUsers, float skillLevel) {
	isLocal = true;
	isHost = false;

	//if (lobbyToSessionCB->CanJoinLocalHost()) {
		state = lobbyBackendState_t::STATE_READY;
	/*}
	else {
		state = lobbyBackendState_t::STATE_FAILED;
	}*/
}

/*
========================
idLobbyBackendDirect::GetSearchResults
========================
*/
void idLobbyBackendDirect::GetSearchResults(std::vector<lobbyConnectInfo_t>& searchResults) {
	lobbyConnectInfo_t fakeResult;

	boost::asio::ip::udp::endpoint ep(boost::asio::ip::make_address(host_net_ip.GetString()), net_port.GetInteger());
	fakeResult.netAddr.address = ep.address();
	fakeResult.netAddr.port = ep.port();
	fakeResult.netAddr.type = netadrtype_t::NA_IP;

	searchResults.clear();
	searchResults.push_back(fakeResult);
}

/*
========================
idLobbyBackendDirect::JoinFromConnectInfo
========================
*/
void idLobbyBackendDirect::JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo) {
	if (lobbyToSessionCB->CanJoinLocalHost()) {
		boost::asio::ip::udp::endpoint ep(boost::asio::ip::make_address("127.0.0.1"), net_port.GetInteger());
		address.address = ep.address();
		address.port = ep.port();
		address.type = netadrtype_t::NA_IP;
	}
	else {
		address = connectInfo.netAddr;
	}

	state = lobbyBackendState_t::STATE_READY;
	isLocal = false;
	isHost = false;
}

/*
========================
idLobbyBackendDirect::Shutdown
========================
*/
void idLobbyBackendDirect::Shutdown() {
	state = lobbyBackendState_t::STATE_SHUTDOWN;
}

/*
========================
idLobbyBackendDirect::GetOwnerAddress
========================
*/
void idLobbyBackendDirect::GetOwnerAddress(lobbyAddress_t& outAddr) {
	outAddr.netAddr = address;
	state = lobbyBackendState_t::STATE_READY;
}

/*
========================
idLobbyBackendDirect::GetConnectInfo
========================
*/
lobbyConnectInfo_t idLobbyBackendDirect::GetConnectInfo() {
	lobbyConnectInfo_t connectInfo;

	// If we aren't the host, this lobby should have been joined through JoinFromConnectInfo
	if (IsHost()) {
		boost::asio::ip::udp::endpoint ep(boost::asio::ip::make_address("127.0.0.1"), net_port.GetInteger());
		address.address = ep.address();
		address.port = ep.port();
		address.type = netadrtype_t::NA_IP;
	}

	connectInfo.netAddr = address;

	return connectInfo;
}