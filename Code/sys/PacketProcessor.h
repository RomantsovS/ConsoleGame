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

  idPacketProcessor() { Reset(); }

  void Reset() {
    msgWritePos = 0;
    fragmentSequence = 0;
    droppedFrags = 0;
    fragmentedSend = false;

    reliableSequenceSend = 1;
    reliableSequenceRecv = 0;

    numReliable = 0;

    queuedReliableAck = -1;

    unsentMsg = idBitMsg();
  }

  static const int MAX_MSG_SIZE =
      8000;  // This is the max size you can pass into ProcessOutgoing
  static const int MAX_FINAL_PACKET_SIZE =
      1200;  // Lowest/safe MTU across all our platforms to avoid fragmentation
             // at the transport layer (which is poorly supported by consumer
             // hardware and may cause nasty latency side effects)
  static const int MAX_RELIABLE_QUEUE = 64;

  // TypeInfo doesn't like sizeof( sessionId_t )?? and then fails to understand
  // the #ifdef/#else/#endif
  static const int MAX_PACKET_SIZE =
      MAX_FINAL_PACKET_SIZE - 4 - 5 -
      2;  // Largest possible packet before headers and such applied (subtract
          // some for various internal header data, and session id)
  static const int MAX_OOB_MSG_SIZE =
      MAX_PACKET_SIZE - 1;  // We don't allow fragmentation for out-of-band
                            // msg's, and we need a byte for the header

 private:
  void QueueReliableAck(int lastReliable);
  int FinalizeRead(idBitMsg& inMsg, idBitMsg& outMsg, int& userValue);

 public:
  // Used to queue reliable msg's, to be sent on the next ProcessOutgoing
  bool QueueReliableMessage(std::byte type, const std::byte* data, int dataLen);
  // Used to process a msg ready to be sent, could get fragmented into multiple
  // fragments
  bool ProcessOutgoing(const int time, const idBitMsg& msg, bool isOOB,
                       int userData);
  // Used to get each fragment for sending through the actual net connection
  bool GetSendFragment(const int time, sessionId_t sessionID, idBitMsg& outMsg);
  // Used to process a fragment received.  Returns true when msg was
  // reconstructed.
  int ProcessIncoming(int time, sessionId_t expectedSessionID, idBitMsg& msg,
                      idBitMsg& out, int& userData, const int peerNum);

  // Returns true if there are more fragments to send
  bool HasMoreFragments() const { return (unsentMsg.GetRemainingData() > 0); }

  // Num reliables not ack'd
  int NumQueuedReliables() { return reliable.Num(); }
  // True if we need to send a reliable ack
  int NeedToSendReliableAck() { return queuedReliableAck >= 0 ? true : false; }

  int GetNumReliables() const { return numReliable; }
  const std::byte* GetReliable(int i) const { return reliableMsgPtrs[i]; }
  int GetReliableSize(int i) const { return reliableMsgSize[i]; }

  // Used for out-of-band non connected peers
  // This doesn't actually support fragmentation, it is just simply here to hide
  // the header structure, so the caller doesn't have to skip over the header
  // data.
  static bool ProcessConnectionlessOutgoing(idBitMsg& msg, idBitMsg& out,
                                            int lobbyType, int userData);
  static bool ProcessConnectionlessIncoming(idBitMsg& msg, idBitMsg& out,
                                            int& userData);

  // Used to "peek" at a session id of a message fragment
  static sessionId_t GetSessionID(idBitMsg& msg);

  int GetReliableDataSize() const { return reliable.GetDataLength(); }

 private:
  // Packet header types
  static const int PACKET_TYPE_INBAND =
      0;  // In-band. Number of reliable msg's stored in userData portion of
          // header
  static const int PACKET_TYPE_OOB =
      1;  // Out-of-band. userData free to use by the caller. Cannot fragment.
  static const int PACKET_TYPE_RELIABLE_ACK =
      2;  // Header type used to piggy-back on top of msgs to ack reliable msg's
  static const int PACKET_TYPE_FRAGMENTED =
      3;  // The msg is fragmented, fragment type stored in the userData portion
          // of header

  // PACKET_TYPE_FRAGMENTED userData values
  static const int FRAGMENT_START = 0;
  static const int FRAGMENT_MIDDLE = 1;
  static const int FRAGMENT_END = 2;

  class idOuterPacketHeader {
   public:
    idOuterPacketHeader() : sessionID(SESSION_ID_INVALID) {}
    idOuterPacketHeader(sessionId_t sessionID_) : sessionID(sessionID_) {}

    void WriteToMsg(idBitMsg& msg) { msg.WriteUShort(sessionID); }

    void ReadFromMsg(idBitMsg& msg) { sessionID = msg.ReadUShort(); }

    sessionId_t GetSessionID() { return sessionID; }

   private:
    sessionId_t sessionID;
  };

  class idInnerPacketHeader {
   public:
    idInnerPacketHeader() : type(0), userData(0) {}
    idInnerPacketHeader(int inType, int inData)
        : type(inType), userData(inData) {}

    void WriteToMsg(idBitMsg& msg) {
      msg.WriteBytes(type, 1);
      msg.WriteLong(userData);
    }

    void ReadFromMsg(idBitMsg& msg) {
      type = msg.ReadBytes(1);
      userData = msg.ReadLong();
    }

    int Type() { return type; }
    int Value() { return userData; }

   private:
    int type;
    int userData;
  };

  std::array<std::byte, MAX_MSG_SIZE>
      msgBuffer;         // Buffer used to reconstruct the msg
  int msgWritePos;       // Write position into the msg reconstruction buffer
  int fragmentSequence;  // Fragment sequence number
  int droppedFrags;      // Number of dropped fragments
  bool fragmentedSend;   // Used to determine if the current send requires
                         // fragmenting

  idDataQueue<MAX_RELIABLE_QUEUE, MAX_MSG_SIZE>
      reliable;  // list of unacknowledged reliable messages

  int reliableSequenceSend;  // sequence number of the next reliable packet
                             // we're going to send to this peer
  int reliableSequenceRecv;  // sequence number of the last reliable packet we
                             // received from this peer

  // These are for receiving reliables, you need to get these before the next
  // process call or they will get cleared
  int numReliable;
  std::array<std::byte, MAX_MSG_SIZE>
      reliableBuffer;  // We shouldn't have to hold more than this
  std::array<const std::byte*, MAX_RELIABLE_QUEUE> reliableMsgPtrs;
  std::array<int, MAX_RELIABLE_QUEUE> reliableMsgSize;

  int queuedReliableAck;  // Used to piggy back on the next send to ack
                          // reliables

  idBitMsg unsentMsg;
  std::array<std::byte, MAX_MSG_SIZE>
      unsentBuffer;  // Buffer used hold the current msg until it's all sent
};

#endif