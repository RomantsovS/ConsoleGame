#include "idlib/precompiled.h"

/*
================================================
idPacketProcessor::QueueReliableAck
================================================
*/
void idPacketProcessor::QueueReliableAck(int lastReliable) {
	// NOTE - Even if it was the last known sequence, go ahead and ack it, in case our last ack for this sequence got dropped
	if (lastReliable >= reliableSequenceRecv) {
		queuedReliableAck = lastReliable;
		reliableSequenceRecv = lastReliable;
	}
}

/*
================================================
idPacketProcessor::FinalizeRead
================================================
*/
int idPacketProcessor::FinalizeRead(idBitMsg& inMsg, idBitMsg& outMsg, int& userValue) {
	userValue = 0;

	idInnerPacketHeader header;
	header.ReadFromMsg(inMsg);

	if (!idverify(header.Type() != PACKET_TYPE_FRAGMENTED)) {		// We shouldn't be fragmented at this point
		idLib::Printf("Received invalid fragmented packet.\n");
		return RETURN_TYPE_NONE;
	}

	if (header.Type() == PACKET_TYPE_RELIABLE_ACK) {
		// Handle reliable ack
		int reliableSequence = inMsg.ReadLong();
		reliable.RemoveOlderThan(reliableSequence + 1);
		header.ReadFromMsg(inMsg);								// Read the new header, since the reliable ack sits on top the actual header of the message
	}

	if (header.Type() == PACKET_TYPE_OOB) {
		// out-of-band packet
		userValue = header.Value();
	}
	else {
		// At this point, this MUST be an in-band packet
		if (!idverify(header.Type() == PACKET_TYPE_INBAND)) {
			idLib::Printf("In-band packet expected, received type %i instead.\n", header.Type());
			return RETURN_TYPE_NONE;
		}

		// Reset number of reliables received (NOTE - This means you MUST unload all reliables as they are received)
		numReliable = 0;

		// Handle reliable portion of in-band packets
		int numReliableRecv = header.Value();
		int bufferPos = 0;

		if (numReliableRecv > 0) {
			int reliableSequence = inMsg.ReadLong();

			idLib::Printf("NET: got %d msgs reliableSequence %d / reliableSequenceRecv %d\n", numReliableRecv, reliableSequence, reliableSequenceRecv);

			for (int r = 0; r < numReliableRecv; r++) {
				uint16_t reliableDataLength = inMsg.ReadLong();

				if (reliableSequence + r > reliableSequenceRecv) {		// Only accept newer reliable msg's than we've currently already received
					if (!idverify(bufferPos + reliableDataLength <= reliableBuffer.size())) {
						idLib::Printf("Reliable msg size overflow.\n");
						return RETURN_TYPE_NONE;
					}
					if (!idverify(numReliable < MAX_RELIABLE_QUEUE)) {
						idLib::Printf("Reliable msg count overflow.\n");
						return RETURN_TYPE_NONE;
					}
					for (size_t i = 0; i < reliableDataLength; ++i)
						*(reliableBuffer.data() + bufferPos + i) = static_cast<std::byte>(inMsg.ReadByte());

					reliableMsgSize[numReliable] = reliableDataLength;
					reliableMsgPtrs[numReliable++] = &reliableBuffer[bufferPos];
					bufferPos += reliableDataLength;
				}
			}

			//inMsg.SetReadCount(inMsg.GetReadCount() + compressedSize);

			QueueReliableAck(reliableSequence + numReliableRecv - 1);
		}
	}

	// Load actual msg
	outMsg.BeginWriting();
	outMsg.WriteData(inMsg.GetReadData() + inMsg.GetReadCount(), inMsg.GetRemainingData());
	outMsg.SetSize(inMsg.GetRemainingData());

	return (header.Type() == PACKET_TYPE_OOB) ? RETURN_TYPE_OOB : RETURN_TYPE_INBAND;
}

/*
================================================
idPacketProcessor::QueueReliableMessage
================================================
*/
bool idPacketProcessor::QueueReliableMessage(std::byte type, const std::byte* data, int dataLen) {
	idLib::Printf("NET: peer queued reliable type %d reliableSequenceSend %d\n", static_cast<int>(type), reliableSequenceSend);
	return reliable.Append(reliableSequenceSend++, &type, 1, data, dataLen);
}

/*
========================
idPacketProcessor::CanSendMoreData
========================
*/
//bool idPacketProcessor::CanSendMoreData() const {
//	if (net_maxRate.GetInteger() == 0) {
//		return true;
//	}
//
//	return (outgoingRateBytes <= net_maxRate.GetInteger() * 1024);
//}

/*
================================================
idPacketProcessor::ProcessOutgoing
NOTE - We only compress reliables because we assume everything else has already been compressed.
================================================
*/
bool idPacketProcessor::ProcessOutgoing(const int time, const idBitMsg& msg, bool isOOB, int userData) {
	// We can only do ONE ProcessOutgoing call, then we need to do GetSendFragment to
	// COMPLETELY empty unsentMsg before calling ProcessOutgoing again.
	/*if (!verify(fragmentedSend == false)) {
		idLib::Warning("ProcessOutgoing: fragmentedSend == true!");
		return false;
	}*/

	if (!idverify(unsentMsg.GetRemainingData() == 0)) {
		idLib::Warning("ProcessOutgoing: unsentMsg.GetRemainingData() > 0!");
		return false;
	}

	// Build the full msg to send, which could include reliable data
	unsentMsg.InitWrite(unsentBuffer.data(), sizeof(unsentBuffer));
	unsentMsg.BeginWriting();

	// Ack reliables if we need to (NOTE - We will send this ack on both the in-band and out-of-band channels)
	if (queuedReliableAck >= 0) {
		idInnerPacketHeader header(PACKET_TYPE_RELIABLE_ACK, 0);
		header.WriteToMsg(unsentMsg);
		unsentMsg.WriteLong(queuedReliableAck);
		queuedReliableAck = -1;
	}

	if (isOOB) {
		if (msg.GetSize() + unsentMsg.GetSize() > MAX_OOB_MSG_SIZE) {		// Fragmentation not allowed for out-of-band msg's
			idLib::Printf("Out-of-band packet too large %i\n", unsentMsg.GetSize());
			idassert(0);
			return false;
		}
		// We don't need to worry about reliable for out of band packets
		idInnerPacketHeader header(PACKET_TYPE_OOB, userData);
		header.WriteToMsg(unsentMsg);
	}
	else {
		// Add reliable msg's here if this is an in-band packet
		idInnerPacketHeader header(PACKET_TYPE_INBAND, reliable.Num());
		header.WriteToMsg(unsentMsg);
		if (reliable.Num() > 0) {
			unsentMsg.WriteLong(reliable.ItemSequence(0));
			for (int i = 0; i < reliable.Num(); i++) {
				unsentMsg.WriteLong(reliable.ItemLength(i));
				unsentMsg.WriteData(reliable.ItemData(i), reliable.ItemLength(i));
			}
		}
	}

	// Fill up with actual msg
	unsentMsg.WriteData(msg.GetReadData(), msg.GetSize());

	if (unsentMsg.GetSize() > MAX_PACKET_SIZE) {
		if (isOOB) {
			idLib::Error("oob msg's cannot fragment");
		}
		//fragmentedSend = true;
	}

	return true;
}

/*
================================================
idPacketProcessor::GetSendFragment
================================================
*/
bool idPacketProcessor::GetSendFragment(const int time, sessionId_t sessionID, idBitMsg& outMsg) {
	//lastSendTime = time;

	if (unsentMsg.GetRemainingData() <= 0) {
		return false;	// Nothing to send
	}

	outMsg.BeginWriting();

	idOuterPacketHeader	outerHeader(sessionID);

	// Write outer packet header to the msg
	outerHeader.WriteToMsg(outMsg);

	//if (!fragmentedSend) {
		// Simple case, no fragments to sent
		outMsg.WriteData(unsentMsg.GetReadData(), unsentMsg.GetSize());
		unsentMsg.SetSize(0);
	//}
	//else {
	//	int currentSize = idMath::ClampInt(0, MAX_PACKET_SIZE, unsentMsg.GetRemainingData());
	//	idassert(currentSize > 0);
	//	idassert(unsentMsg.GetRemainingData() - currentSize >= 0);

	//	// See if we'll have more fragments once we subtract off how much we're about to write
	//	bool moreFragments = (unsentMsg.GetRemainingData() - currentSize > 0) ? true : false;

	//	if (!unsentMsg.GetReadCount()) {		// If this is the first read, then we know it's the first fragment
	//		idassert(moreFragments);			// If we have a first, we must have more or something went wrong
	//		idInnerPacketHeader header(PACKET_TYPE_FRAGMENTED, FRAGMENT_START);
	//		header.WriteToMsg(outMsg);
	//	}
	//	else {
	//		idInnerPacketHeader header(PACKET_TYPE_FRAGMENTED, moreFragments ? FRAGMENT_MIDDLE : FRAGMENT_END);
	//		header.WriteToMsg(outMsg);
	//	}

	//	outMsg.WriteLong(fragmentSequence);
	//	outMsg.WriteData(unsentMsg.GetReadData() + unsentMsg.GetReadCount(), currentSize);
	//	unsentMsg.ReadData(NULL, currentSize);

	//	idassert(moreFragments == unsentMsg.GetRemainingData() > 0);
	//	fragmentedSend = moreFragments;

	//	fragmentSequence++;				// Advance sequence

	//	fragmentAccumulator++;			// update the counter for the net debug hud
	//}


	// The caller needs to send this packet, so assume he did, and update rates
	//UpdateOutgoingRate(time, outMsg.GetSize());

	return true;
}

/*
================================================
idPacketProcessor::ProcessIncoming
================================================
*/
int idPacketProcessor::ProcessIncoming(int time, sessionId_t expectedSessionID, idBitMsg& msg, idBitMsg& out, int& userData, const int peerNum) {
	idassert(msg.GetSize() <= MAX_FINAL_PACKET_SIZE);

	//UpdateIncomingRate(time, msg.GetSize());

	idOuterPacketHeader outerHeader;
	outerHeader.ReadFromMsg(msg);

	sessionId_t sessionID = outerHeader.GetSessionID();
	idassert(sessionID == expectedSessionID);

	if (!idverify(sessionID != SESSION_ID_CONNECTIONLESS_PARTY && sessionID != SESSION_ID_CONNECTIONLESS_GAME && sessionID != SESSION_ID_CONNECTIONLESS_GAME_STATE)) {
		idLib::Printf("Expected non connectionless ID, but got a connectionless one\n");
		return RETURN_TYPE_NONE;
	}

	if (sessionID != expectedSessionID) {
		idLib::Printf("Expected session id: %8x but got %8x instead\n", expectedSessionID, sessionID);
		return RETURN_TYPE_NONE;
	}

	int c, b;
	msg.SaveReadState(c, b);

	idInnerPacketHeader header;
	header.ReadFromMsg(msg);

	if (header.Type() != PACKET_TYPE_FRAGMENTED) {
		// Non fragmented
		msg.RestoreReadState(c, b);		// Reset since we took a byte to check the type
		return FinalizeRead(msg, out, userData);
	}

	// Decode fragmented packet
	int readSequence = msg.ReadLong();	// Read sequence of fragment

	//if (header.Value() == FRAGMENT_START) {
	//	msgWritePos = 0;				// Reset msg reconstruction write pos
	//}
	//else if (fragmentSequence == -1 || readSequence != fragmentSequence + 1) {
	//	droppedFrags++;
	//	idLib::Printf("Dropped Fragments - PeerNum: %i FragmentSeq: %i, ReadSeq: %i, Total: %i\n", peerNum, fragmentSequence, readSequence, droppedFrags);

	//	// If this is the middle or end, make sure we are reading in fragmentSequence
	//	fragmentSequence = -1;
	//	return RETURN_TYPE_NONE;		// Out of sequence
	//}
	//fragmentSequence = readSequence;
	//idassert(msg.GetRemainingData() > 0);

	//if (!verify(msgWritePos + msg.GetRemainingData() < sizeof(msgBuffer))) {
	//	idLib::Error("ProcessIncoming: Fragmented msg buffer overflow.");
	//}

	//memcpy(msgBuffer + msgWritePos, msg.GetReadData() + msg.GetReadCount(), msg.GetRemainingData());
	//msgWritePos += msg.GetRemainingData();

	//if (header.Value() == FRAGMENT_END) {
	//	// Done reconstructing the msg
	//	idBitMsg msg(msgBuffer, sizeof(msgBuffer));
	//	msg.SetSize(msgWritePos);
	//	return FinalizeRead(msg, out, userData);
	//}

	/*if (!verify(header.Value() == FRAGMENT_START || header.Value() == FRAGMENT_MIDDLE)) {
		idLib::Printf("ProcessIncoming: Invalid packet.\n");
	}*/

	// If we get here, this is part (either beginning or end) of a fragmented packet.
	// We return RETURN_TYPE_NONE to let the caller know they don't need to do anything yet.
	return RETURN_TYPE_NONE;
}

/*
================================================
idPacketProcessor::ProcessConnectionlessOutgoing
================================================
*/
bool idPacketProcessor::ProcessConnectionlessOutgoing(idBitMsg& msg, idBitMsg& out, int lobbyType, int userData) {
	sessionId_t sessionID = lobbyType + 1;


	// Write outer header
	idOuterPacketHeader outerHeader(sessionID);
	outerHeader.WriteToMsg(out);

	// Write inner header
	idInnerPacketHeader header(PACKET_TYPE_OOB, userData);
	header.WriteToMsg(out);

	// Write msg
	out.WriteData(msg.GetReadData(), msg.GetSize());


	return true;
}

/*
================================================
idPacketProcessor::ProcessConnectionlessIncoming
================================================
*/
bool idPacketProcessor::ProcessConnectionlessIncoming(idBitMsg& msg, idBitMsg& out, int& userData) {

	idOuterPacketHeader outerHeader;
	outerHeader.ReadFromMsg(msg);

	sessionId_t sessionID = outerHeader.GetSessionID();

	if (sessionID != SESSION_ID_CONNECTIONLESS_PARTY && sessionID != SESSION_ID_CONNECTIONLESS_GAME && sessionID != SESSION_ID_CONNECTIONLESS_GAME_STATE) {
		// Not a connectionless msg (this can happen if a previously connected peer keeps sending data for whatever reason)
		idLib::Printf("ProcessConnectionlessIncoming: Invalid session ID - %d\n", sessionID);
		return false;
	}

	idInnerPacketHeader header;
	header.ReadFromMsg(msg);

	if (header.Type() != PACKET_TYPE_OOB) {
		idLib::Printf("ProcessConnectionlessIncoming: header.Type() != PACKET_TYPE_OOB\n");
		return false;		// Only out-of-band packets supported for connectionless
	}

	userData = header.Value();

	out.BeginWriting();
	out.WriteData(msg.GetReadData() + msg.GetReadCount(), msg.GetRemainingData());
	out.SetSize(msg.GetRemainingData());

	return true;
}

/*
================================================
idPacketProcessor::GetSessionID
================================================
*/
idPacketProcessor::sessionId_t idPacketProcessor::GetSessionID(idBitMsg& msg) {
	sessionId_t sessionID;
	int c, b;
	msg.SaveReadState(c, b);
	// Read outer header
	idOuterPacketHeader outerHeader;
	outerHeader.ReadFromMsg(msg);

	// Get session ID
	sessionID = outerHeader.GetSessionID();

	msg.RestoreReadState(c, b);
	return sessionID;
}