#include "idlib/precompiled.h"
#include "sys_lobby.h"

/*
========================
idLobby::AllocUser
========================
*/
lobbyUser_t* idLobby::AllocUser(const lobbyUser_t& defaults) {
  if (!idverify(freeUsers.size() > 0)) {
    idLib::Error("Out of session users");  // This shouldn't be possible
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
  if (!idverify(user)) {
    return;
  }

  if (!VerifyUser(user)) {
    return;
  }

  // SaveDisconnectedUser(*user);

  userList.erase(std::remove(userList.begin(), userList.end(), user),
                 userList.end());

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
  if (!idverify(find(userList.begin(), userList.end(), lobbyUser) !=
                userList.end())) {
    return false;
  }

  return true;
}

bool idLobby::IsSessionUserLocal(const lobbyUser_t* lobbyUser) const {
  if (!idverify(lobbyUser != NULL)) {
    return false;
  }

  if (!VerifyUser(lobbyUser)) {
    return false;
  }

  // This user is local if the peerIndex matches what our peerIndex is on the
  // host
  return (lobbyUser->peerIndex == peerIndexOnHost);
}

bool idLobby::IsSessionUserIndexLocal(int i) const {
  return IsSessionUserLocal(GetLobbyUser(i));
}

int idLobby::GetLobbyUserIndexByID(lobbyUserID_t lobbyUserId,
                                   bool ignoreLobbyType) const {
  if (!lobbyUserId.IsValid()) {
    return -1;
  }
  idassert(lobbyUserId.GetLobbyType() == static_cast<std::byte>(lobbyType) ||
           ignoreLobbyType);

  for (int i = 0; i < GetNumLobbyUsers(); ++i) {
    /*if (ignoreLobbyType) {
            if
    (GetLobbyUser(i)->lobbyUserID.CompareIgnoreLobbyType(lobbyUserId)) { return
    i;
            }
            continue;
    }*/
    if (GetLobbyUser(i)->lobbyUserID == lobbyUserId) {
      return i;
    }
  }
  return -1;
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
  lobbyUser.lobbyUserID =
      lobbyUserID_t(Sys_GetClockTicks(),
                    static_cast<std::byte>(
                        lobbyType));  // Generate the lobby using a combination
                                      // of local user id, and lobby type
  // lobbyUser.disconnecting = false;

  NET_VERBOSE_PRINT("NET: CreateLobbyUserFromLocalUser: (%s)\n",
                    GetLobbyName().c_str());

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
Go through each user, and remove the ones that have a peer marked as
disconnected NOTE - This should only be called from the host.  The host will
call RemoveSessionUsersByIDList, which will forward the action to the connected
peers.
========================
*/
void idLobby::RemoveUsersWithDisconnectedPeers() {
  if (!idverify(IsHost())) {
    // We're not allowed to do this unless we are the host of this session type
    // If we are the host, RemoveSessionUsersByIDList will forward the call to
    // peers.
    return;
  }
}

void idLobby::SendNewUsersToPeers(int skipPeer, int userStart, int numUsers) {
  if (!IsHost()) {
    return;
  }

  assert(GetNumLobbyUsers() - userStart == numUsers);

  // std::byte buffer[idPacketProcessor::MAX_PACKET_SIZE];
  // idBitMsg outmsg(buffer, sizeof(buffer));

  //// Write number of users
  // outmsg.WriteBytes(numUsers, 1);

  //// Fill up the msg with all the users
  // for (int u = userStart; u < GetNumLobbyUsers(); u++) {
  //	GetLobbyUser(u)->WriteToMsg(outmsg);
  // }

  //// Send the msg to all peers (except the skipPeer, or peers not connected to
  /// this session type)
  // for (int p = 0; p < peers.size(); p++) {
  //	if (p == skipPeer || peers[p].GetConnectionState() !=
  // connectionState_t::CONNECTION_ESTABLISHED) { 		continue;
  // // If they are not connected in this session type, don't send anything to
  // them.
  //	}
  //	QueueReliableMessage(p, RELIABLE_USER_CONNECTED, outmsg.GetReadData(),
  // outmsg.GetSize());
  // }
}

void idLobby::AddUsersFromMsg(idBitMsg& msg, int fromPeer) {
  int userStart = GetNumLobbyUsers();
  int numNewUsers = msg.ReadByte();

  assert(lobbyBackend);

  // Add the new users to our own list
  for (int u = 0; u < numNewUsers; u++) {
    lobbyUser_t newUser;

    // Read in the new user
    newUser.ReadFromMsg(msg);

    // Initialize their peerIndex and userID if we are the host
    // (we'll send these back to them in the initial connect)
    if (IsHost()) {
      if (fromPeer != -1) {  // -1 means this is the host adding his own users,
                             // and this stuff is already computed
        // local users will already have this information filled out.
        newUser.address = peers[fromPeer].address;
        newUser.peerIndex = fromPeer;
        /*if (lobbyType == TYPE_PARTY) {
                newUser.partyToken = GetPartyTokenAsHost();
        }*/
      }
    } else {
      assert(fromPeer == host);
      // The host sends us all user addresses, except his local users, so we
      // compute that here
      if (newUser.peerIndex == -1) {
        newUser.address = peers[fromPeer].address;
      }
    }

    idLib::Printf("NET: user joined (%s).\n", GetLobbyName().c_str());

    lobbyUser_t* appendedUser = NULL;

    // First, try to replace a disconnected user
    for (int i = 0; i < GetNumLobbyUsers(); i++) {
      lobbyUser_t* user = GetLobbyUser(i);

      if (user->IsDisconnected()) {
        userStart = i;
        *user = newUser;
        appendedUser = user;
        break;
      }
    }

    // Add them to our list
    if (appendedUser == NULL) {
      appendedUser = AllocUser(newUser);
    }

    // Run platform-specific handler after adding
    assert(appendedUser->peerIndex == newUser.peerIndex);      // paranoia
    assert(appendedUser->lobbyUserID == newUser.lobbyUserID);  // paranoia
    // RegisterUser(appendedUser);
  }

  // Forward list of the new users to all other peers
  if (IsHost()) {
    SendNewUsersToPeers(fromPeer, userStart, numNewUsers);

    // Set the lobbies skill level
    // lobbyBackend->UpdateLobbySkill(GetAverageSessionLevel());
  }

  idLib::Printf("---------------- %s --------------------\n",
                GetLobbyName().c_str());
  for (int userIndex = 0; userIndex < GetNumLobbyUsers(); ++userIndex) {
    lobbyUser_t* user = GetLobbyUser(userIndex);
    idLib::Printf("party user \n");
  }
  idLib::Printf("---------------- %s --------------------\n",
                GetLobbyName().c_str());
}

bool idLobby::ValidateConnectedUser(const lobbyUser_t* user) const {
  if (!user) {
    return false;
  }

  if (user->IsDisconnected()) {
    return false;
  }

  if (user->peerIndex == -1) {
    return true;  // Host
  }

  if (IsHost()) {
    if (user->peerIndex < 0 || user->peerIndex >= peers.size()) {
      return false;
    }

    if (!peers[user->peerIndex].IsConnected()) {
      return false;
    }
  }

  return true;
}

bool idLobby::IsLobbyUserValid(lobbyUserID_t lobbyUserID) const {
  if (!lobbyUserID.IsValid()) {
    return false;
  }

  if (GetLobbyUserIndexByID(lobbyUserID) == -1) {
    return false;
  }

  return true;
}

bool idLobby::IsLobbyUserLoaded(lobbyUserID_t lobbyUserID) const {
  assert(lobbyType == GetActingGameStateLobbyType());

  int userIndex = GetLobbyUserIndexByID(lobbyUserID);

  if (!idverify(userIndex != -1)) {
    return false;
  }

  const lobbyUser_t* user = GetLobbyUser(userIndex);

  if (user == NULL) {
    return false;
  }

  if (!ValidateConnectedUser(user)) {
    return false;
  }

  if (IsSessionUserLocal(user)) {
    return loaded;  // If this is a local user, check the local loaded flag
  }

  if (!idverify(user->peerIndex >= 0 && user->peerIndex < peers.size())) {
    return false;
  }

  return peers[user->peerIndex].loaded;
}

lobbyUserID_t idLobby::GetLobbyUserIdByOrdinal(int userIndex) const {
  const lobbyUser_t* user = GetLobbyUser(userIndex);
  if (!user) {
    return lobbyUserID_t();
  }

  if (!ValidateConnectedUser(user)) {
    return lobbyUserID_t();
  }

  return user->lobbyUserID;
}