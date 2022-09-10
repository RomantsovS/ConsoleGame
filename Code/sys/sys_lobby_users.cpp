#include "idlib/precompiled.h"
#include "sys_lobby.h"

/*
========================
idLobby::AllocUser
========================
*/
lobbyUser_t* idLobby::AllocUser(const lobbyUser_t& defaults) {
	if (!verify(freeUsers.size() > 0)) {
		idLib::Error("Out of session users");		// This shouldn't be possible
	}

	lobbyUser_t* user = freeUsers.back();
	freeUsers.pop_back();

	// Set defaults
	*user = defaults;

	userList.push_back(user);

	idassert(userList.size() == userPool.size() - freeUsers.size());

	return user;
}

/*
========================
idLobby::FreeUser
========================
*/
void idLobby::FreeUser(lobbyUser_t* user) {
	if (!verify(user)) {
		return;
	}

	if (!VerifyUser(user)) {
		return;
	}

	//SaveDisconnectedUser(*user);

	userList.erase(std::remove(userList.begin(), userList.end(), user), userList.end());

	freeUsers.push_back(user);
}

/*
========================
idLobby::FreeAllUsers
========================
*/
void idLobby::FreeAllUsers() {
	for (int i = userList.size() - 1; i >= 0; i--) {
		FreeUser(userList[i]);
	}

	idassert(userList.empty());
	idassert(freeUsers.size() == userPool.size());
}

/*
========================
idLobby::VerifyUser
========================
*/
bool idLobby::VerifyUser(const lobbyUser_t* lobbyUser) const {
	if (!verify(find(userList.begin(), userList.end(), lobbyUser) != userList.end())) {
		return false;
	}

	return true;
}

/*
========================
idLobby::CreateLobbyUserFromLocalUser
This functions just defaults the session users to the signin manager local users
========================
*/
lobbyUser_t idLobby::CreateLobbyUserFromLocalUser() {

	lobbyUser_t lobbyUser;
	lobbyUser.peerIndex = -1;
	//lobbyUser.lobbyUserID = lobbyUserID_t(localUser->GetLocalUserHandle(), lobbyType);	// Generate the lobby using a combination of local user id, and lobby type
	//lobbyUser.disconnecting = false;

	NET_VERBOSE_PRINT("NET: CreateLobbyUserFromLocalUser: (%s)\n", GetLobbyName().c_str());

	return lobbyUser;
}

/*
========================
idLobby::InitSessionUsersFromLocalUsers
This functions just defaults the session users to the signin manager local users
========================
*/
void idLobby::InitSessionUsersFromLocalUsers(bool onlineMatch) {
	idassert(lobbyBackend);

	// First, clear all session users of this session type
	FreeAllUsers();

	lobbyUser_t lobbyUser = CreateLobbyUserFromLocalUser();

	// Append this new session user to the session user list
	lobbyUser_t* createdUser = AllocUser(lobbyUser);
}

/*
========================
idLobby::RemoveUsersWithDisconnectedPeers
Go through each user, and remove the ones that have a peer marked as disconnected
NOTE - This should only be called from the host.  The host will call RemoveSessionUsersByIDList,
which will forward the action to the connected peers.
========================
*/
void idLobby::RemoveUsersWithDisconnectedPeers() {
	if (!verify(IsHost())) {
		// We're not allowed to do this unless we are the host of this session type
		// If we are the host, RemoveSessionUsersByIDList will forward the call to peers.
		return;
	}
}