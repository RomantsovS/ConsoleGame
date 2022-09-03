#include "idLib/precompiled.h"

/*
================================================
idPacketProcessor::ProcessConnectionlessOutgoing
================================================
*/
bool idPacketProcessor::ProcessConnectionlessOutgoing(idBitMsg& msg, int lobbyType) {
	sessionId_t sessionID = lobbyType + 1;

	msg.WriteLong(sessionID);

	msg.WriteLong(PACKET_TYPE_OOB);

	return true;
}

/*
================================================
idPacketProcessor::ProcessConnectionlessIncoming
================================================
*/
bool idPacketProcessor::ProcessConnectionlessIncoming(idBitMsg& msg, sessionId_t sessionID, idBitMsg& out, int& userData) {

	if (sessionID != SESSION_ID_CONNECTIONLESS_PARTY && sessionID != SESSION_ID_CONNECTIONLESS_GAME && sessionID != SESSION_ID_CONNECTIONLESS_GAME_STATE) {
		// Not a connectionless msg (this can happen if a previously connected peer keeps sending data for whatever reason)
		idLib::Printf("ProcessConnectionlessIncoming: Invalid session ID - %d\n", sessionID);
		return false;
	}

	int header = msg.ReadLong();

	if (header != PACKET_TYPE_OOB) {
		idLib::Printf("ProcessConnectionlessIncoming: header.Type() != PACKET_TYPE_OOB\n");
		return false;		// Only out-of-band packets supported for connectionless
	}

	return true;
}

/*
================================================
idPacketProcessor::GetSessionID
================================================
*/
idPacketProcessor::sessionId_t idPacketProcessor::GetSessionID(idBitMsg& msg) {
	sessionId_t sessionID;
	
	sessionID = msg.ReadLong();

	return sessionID;
}