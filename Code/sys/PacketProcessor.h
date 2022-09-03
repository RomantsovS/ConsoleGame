#ifndef __PACKET_PROCESSOR_H__
#define __PACKET_PROCESSOR_H__

#include <google/protobuf/message.h>

/*
================================================
idPacketProcessor
================================================
*/
class idPacketProcessor {
public:
	static const int RETURN_TYPE_NONE = 0;
	static const int RETURN_TYPE_OOB = 1;
	static const int RETURN_TYPE_INBAND = 2;

	using sessionId_t = uint16_t;

	static const int NUM_LOBBY_TYPE_BITS = 2;
	static const int LOBBY_TYPE_MASK = (1 << NUM_LOBBY_TYPE_BITS) - 1;

	static const sessionId_t SESSION_ID_INVALID = 0;
	static const sessionId_t SESSION_ID_CONNECTIONLESS_PARTY = 1;
	static const sessionId_t SESSION_ID_CONNECTIONLESS_GAME = 2;
	static const sessionId_t SESSION_ID_CONNECTIONLESS_GAME_STATE = 3;

	static const int MAX_MSG_SIZE = 8000;							// This is the max size you can pass into ProcessOutgoing
	static const int MAX_FINAL_PACKET_SIZE = 1200;							// Lowest/safe MTU across all our platforms to avoid fragmentation at the transport layer (which is poorly supported by consumer hardware and may cause nasty latency side effects)
	static const int MAX_RELIABLE_QUEUE = 64;

	// TypeInfo doesn't like sizeof( sessionId_t )?? and then fails to understand the #ifdef/#else/#endif
	//static const int MAX_PACKET_SIZE		= MAX_FINAL_PACKET_SIZE - 6 - sizeof( sessionId_t );	// Largest possible packet before headers and such applied (subtract some for various internal header data, and session id)
	static const int MAX_PACKET_SIZE = MAX_FINAL_PACKET_SIZE - 6 - 2;			// Largest possible packet before headers and such applied (subtract some for various internal header data, and session id)
	static const int MAX_OOB_MSG_SIZE = MAX_PACKET_SIZE - 1;			// We don't allow fragmentation for out-of-band msg's, and we need a byte for the header

	// Used for out-of-band non connected peers
	// This doesn't actually support fragmentation, it is just simply here to hide the
	// header structure, so the caller doesn't have to skip over the header data.
	static bool ProcessConnectionlessOutgoing(idBitMsg& msg, int lobbyType);
	static bool ProcessConnectionlessIncoming(idBitMsg& msg, sessionId_t sessionID, idBitMsg& out, int& userData);

	// Used to "peek" at a session id of a message fragment
	static sessionId_t GetSessionID(idBitMsg& msg);
private:

	// Packet header types
	static const int PACKET_TYPE_INBAND = 0;	// In-band. Number of reliable msg's stored in userData portion of header
	static const int PACKET_TYPE_OOB = 1;	// Out-of-band. userData free to use by the caller. Cannot fragment.
	static const int PACKET_TYPE_RELIABLE_ACK = 2;	// Header type used to piggy-back on top of msgs to ack reliable msg's
	static const int PACKET_TYPE_FRAGMENTED = 3;	// The msg is fragmented, fragment type stored in the userData portion of header
};

#endif